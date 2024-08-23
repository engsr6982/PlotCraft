#include "TemplateManager.h"
#include "mc/deps/core/utility/buffer_span_mut.h"
#include "mc/enums/LogLevel.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockVolume.h"
#include "mc/world/level/block/registry/BlockTypeRegistry.h"
#include "mc/world/level/chunk/ChunkSource.h"
#include "nlohmann/json.hpp"
#include "plotcraft/Config.h"
#include "plotcraft/utils/JsonHelper.h"
#include "plotcraft/utils/Mc.h"
#include "plugin/MyPlugin.h"
#include <filesystem>
#include <fstream>
#include <ostream>


namespace plo::core {
namespace fs = std::filesystem;
using json   = nlohmann::json;
using namespace plo::mc;

// static member definitions
TemplateData                                          TemplateManager::mTemplateData;
std::unordered_map<string, Block const*>              TemplateManager::mBlockMap;
std::unordered_map<string, std::vector<Block const*>> TemplateManager::mBlockBuffer;
std::unordered_map<string, BlockVolume>               TemplateManager::mBlockVolume;

TemplateData                    TemplateManager::mRecordData; // Record
bool                            TemplateManager::mCanRecord;
bool                            TemplateManager::mIsRecording;
ChunkPos                        TemplateManager::mRecordStart;
ChunkPos                        TemplateManager::mRecordEnd;
std::unordered_map<string, int> TemplateManager::mRecordBlockIDMap; // key: typeName


// functions
bool TemplateManager::_parseTemplate() {
    auto& data = mTemplateData;

    // 解析 block_map
    auto defBlock     = &BlockTypeRegistry::getDefaultBlockState(data.default_block.c_str());
    auto bedrockBlock = &BlockTypeRegistry::getDefaultBlockState("minecraft:bedrock");
    auto airBlock     = &BlockTypeRegistry::getDefaultBlockState("minecraft:air");
    mBlockMap.emplace(defBlock->getTypeName(), defBlock);
    mBlockMap.emplace(bedrockBlock->getTypeName(), bedrockBlock);
    mBlockMap.emplace(airBlock->getTypeName(), airBlock);
    for (auto& i : data.block_map) {
        auto bl = &BlockTypeRegistry::getDefaultBlockState(i.second.c_str());
        mBlockMap.emplace(bl->getTypeName(), bl);
    }


    // 解析 template_data
    int templateStartHeight = data.template_offset;         // 模板起始高度(此高度以下的为defBlock)
    if (templateStartHeight < 0) templateStartHeight += 64; // 起始高度 < 0，则+64，指向正确的buffer位置

    int templateTotalHeight = templateStartHeight + data.template_height; // 起始高度 + 模板数据高度

    // 计算最小的 buffer 高度
    int remainder = templateTotalHeight % 16; // 取余数
    int fixedHeight =
        remainder != 0 ? (templateTotalHeight + 16 - remainder) : templateTotalHeight; // 最小的BlockVolume.y为16

    int totalVolume = fixedHeight * 16 * 16; // 16 * 16 * fixedHeight

    // 遍历区块数据
    for (auto& [key, templateBlocks] : data.template_data) {
        auto& buffer = mBlockBuffer[string(key)];
        buffer.resize(totalVolume, airBlock); // 调整大小并填充默认方块

        // 遍历区块的 256 个Y轴
        for (int y = 0; y < 256; y++) {
            if (data.fill_bedrock) buffer[y * fixedHeight] = bedrockBlock; // 每个y轴的起点设置为基岩

            // 读 template_height 个方块数据
            int mapIndex    = y * data.template_height;                // block_map 的索引
            int bufferIndex = (y * fixedHeight) + templateStartHeight; // buffer 的索引
            int defIndex    = (y * fixedHeight) + 1;                   // 默认方块下标(+1避开基岩)
            for (int r = 0; r < fixedHeight; r++) {
                if (r < data.template_height) {
                    Block const* bl         = mBlockMap[data.block_map[std::to_string(templateBlocks[mapIndex + r])]];
                    buffer[bufferIndex + r] = bl;
                }
                if (r < templateStartHeight - 1) {
                    buffer[defIndex + r] = defBlock;
                }
            }
        }
    }

    return true;
}
bool TemplateManager::loadTemplate(const string& path) {
    if (!fs::exists(path)) return false;

    std::ifstream file(path);
    if (!file.is_open()) return false;

    auto jdata = json::parse(file);
    utils::JsonHelper::jsonToStructNoMerge(jdata, mTemplateData);

    return _parseTemplate();
}
bool TemplateManager::generatorBlockVolume(BlockVolume& volume) {
    auto& data        = mTemplateData;
    int   startHeight = data.template_offset;
    if (startHeight < 0) startHeight += 64;               // 64 为世界底部高度, +64为了正确指向buffer
    int totalHeight = startHeight + data.template_height; // 起点 + buffer 高度

    for (auto& [key, buffer] : mBlockBuffer) {
        mBlockVolume[string(key)] = BlockVolume(volume);

        auto& v = mBlockVolume[key];

        v.mHeight        = totalHeight;
        v.mBlocks.mBegin = &*buffer.begin();
        v.mBlocks.mEnd   = &*buffer.end();
    }
    return true;
}

int  TemplateManager::getCurrentTemplateVersion() { return mTemplateData.version; }
int  TemplateManager::getCurrentTemplateChunkNum() { return mTemplateData.template_chunk_num; }
int  TemplateManager::getCurrentTemplateRoadWidth() { return mTemplateData.template_road_width; }
bool TemplateManager::isUseTemplate() { return config::cfg.generator.type == config::PlotGeneratorType::Template; }

// Tools
void TemplateManager::toPositive(int& num) {
    if (num < 0) num = -num;
}
string TemplateManager::calculateChunkID(const ChunkPos& pos) {
    int n = getCurrentTemplateChunkNum();
    int x = ((pos.x % n) + n) % n;
    int z = ((pos.z % n) + n) % n;
    return fmt::format("({},{})", x, z);
}


// Record
bool TemplateManager::isRecordTemplateing() { return mIsRecording; }
bool TemplateManager::canRecordTemplate() { return !mIsRecording; }


bool TemplateManager::prepareRecordTemplate(
    int           stratY,    // = offset
    int           endY,      // = offset + height
    int           roadWidth, // = roadWidth
    bool          fillBedrock,
    string const& defaultBlock
) {
    if (mIsRecording) return false;
    if (stratY >= endY) return false;

    mRecordData.template_offset     = stratY;
    mRecordData.template_height     = endY - stratY;
    mRecordData.template_road_width = roadWidth;
    mRecordData.fill_bedrock        = fillBedrock;
    mRecordData.default_block       = string{defaultBlock};

    mIsRecording = true;
    return true;
}
bool TemplateManager::postRecordTemplateStart(const ChunkPos& start) {
    if (!mIsRecording) return false;
    if (start.x != 0 && start.z != 0) return false; // 限定 (0,0) 开始

    mRecordStart = ChunkPos{start};

    return true;
}
bool TemplateManager::postRecordTemplateEnd(const ChunkPos& end) {
    if (!mIsRecording) return false;
    if (end.x != end.z) return false;           // 限定正方形
    if (end.x <= 0 || end.z <= 0) return false; // 禁止2、3、4象限

    mRecordEnd                     = ChunkPos{end};
    mRecordData.template_chunk_num = end.x + 1; // +1 因为 (0,0) 开始

    mCanRecord = true;

    return true;
}


bool TemplateManager::_processChunk(const LevelChunk& chunk) {
    auto const& min = chunk.getMin();
    auto const& bs  = chunk.getDimension().getBlockSourceFromMainChunkSource();

    // 获取当前区块的 buffer
    auto& map    = mRecordBlockIDMap;
    auto& data   = mRecordData;
    auto& buffer = data.template_data[calculateChunkID(chunk.getPosition())];


    int totalHeight = data.template_offset + data.template_height + 64; // y 偏移量 + buffer 高度
    if (data.template_offset > 0) totalHeight += 64;                    // + 原版-64
    int const totalVolume = totalHeight * 16 * 16;                      // buffer 体积  16 * 16 * totalHeight

    buffer.reserve(totalVolume); // 预分配空间


    int const height = data.template_offset + data.template_height; // -64 + 16 = 48

    BlockPos cur(min);
    cur.y = mRecordData.template_offset; // 初始 y 轴
    for (int _x = 0; _x < 16; _x++) {
        for (int _z = 0; _z < 16; _z++) {
            for (int _y = mRecordData.template_offset; _y < height; _y++) {
                cur.y = _y;
                // printf("cur: (%d,%d,%d)\n", cur.x, cur.y, cur.z);
                auto& bl   = bs.getBlock(cur).getTypeName();
                auto  iter = map.find(bl);
                if (iter == map.end()) {
                    map[bl] = map.size() + 1;
                    buffer.push_back(map[bl]);
                } else {
                    buffer.push_back(iter->second);
                }
            }
            cur.z++;
        }
        cur.x++;
        cur.z = min.z; // 重置 z 轴
    }

    return true;
}
bool TemplateManager::postRecordAndSaveTemplate(const string& fileName, Player& player) {
    if (!mCanRecord) return false;

    mCanRecord = false;
    mRecordData.block_map.clear();
    mRecordData.template_data.clear();

    auto const& bs = player.getDimensionBlockSourceConst();

    // 处理区块
    for (int x = mRecordStart.x; x <= mRecordEnd.x; x++) {
        for (int z = mRecordStart.z; z <= mRecordEnd.z; z++) {
            // printf("Processing chunk (%d,%d)\n", x, z);
            auto ch = bs.getChunk(x, z);
            if (!ch) {
                sendText<LogLevel::Error>(player, "[TemplateManager] Can't get chunk ({},{}).", x, z);
                break;
            }
            if (!ch->isFullyLoaded()) {
                sendText<LogLevel::Error>(player, "[TemplateManager] Chunk ({},{}) is not fully loaded.", x, z);
                break;
            }
            if (_processChunk(*ch)) {
                sendText<LogLevel::Success>(player, "[TemplateManager] Process chunk ({},{}).", x, z);
            } else {
                sendText<LogLevel::Error>(player, "[TemplateManager] Can't process chunk ({},{}).", x, z);
            }
        }
    }

    auto& idMap = mRecordData.block_map;
    idMap.reserve(mRecordBlockIDMap.size());
    for (auto& [name, id] : mRecordBlockIDMap) {
        idMap.emplace(std::to_string(id), name); // id -> name
    }

    auto& data = mRecordData;
    auto  out  = my_plugin::MyPlugin::getInstance().getSelf().getConfigDir() / fileName;

    std::ofstream ofs(out.string());
    if (ofs.is_open()) {
        ofs << utils::JsonHelper::structToJsonString(data);
        ofs.close();
    }
    sendText<LogLevel::Success>(player, "[TemplateManager] Template saved to {}", out.string());

    return true;
}
bool TemplateManager::resetRecordTemplate() {
    mRecordData  = {};
    mIsRecording = false;
    mCanRecord   = false;
    mRecordStart = {};
    mRecordEnd   = {};
    mRecordBlockIDMap.clear();
    return true;
}

} // namespace plo::core