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

    // 解析 方块映射
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


    int totalHeight = data.template_offset + data.template_height + 64; // y 偏移量 + buffer 高度
    if (data.template_offset < 0) totalHeight += 64;                    // + 原版-64

    int startHeight = data.template_offset < 0 ? data.template_offset + 64 : data.template_offset; // 起始高度

    int totalVolume = totalHeight * 16 * 16; // buffer 体积  16 * 16 * totalHeight

    for (auto& [key, templateBlocks] : data.template_data) {
        auto& buffer = mBlockBuffer[key];
        buffer.resize(totalVolume, defBlock); // 直接调整大小并填充默认方块

        for (int y = 0; y < 256; y++) { // 遍历 256 个Y轴
            if (data.fill_bedrock) buffer[y * totalHeight] = bedrockBlock;

            // 读 template_height 个方块
            for (size_t r = 0; r < templateBlocks.size() && r < static_cast<size_t>(data.template_height); r++) {
                auto bl = mBlockMap[data.block_map[std::to_string(templateBlocks[y * r])]]; // 获取方块
                buffer[(y * startHeight) + r] = bl;
            }
        }

        // 生成 BlockVolume
        buffer_span_mut<Block const*> span;
        span.mBegin       = &*buffer.begin();
        span.mEnd         = &*buffer.end();
        mBlockVolume[key] = BlockVolume(span, 16, totalHeight, 16, *airBlock, 0);
    }

    return true;
}
bool TemplateManager::loadTemplate(const string& path) {
    if (!fs::exists(path)) return false;

    std::ifstream file(path);
    if (!file.is_open()) return false;

    auto jdata = json::parse(file);
    utils::JsonHelper::jsonToStruct(jdata, mTemplateData);

    return _parseTemplate();
}

// Tools
int    TemplateManager::getChunkNum() { return mTemplateData.template_chunk_num; }
string TemplateManager::calculateChunkID(const ChunkPos& pos) {
    int n = getChunkNum();
    return fmt::format("({},{})", (pos.x * 16 / n), (pos.z * 16 / n));
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

    mRecordData.template_offset     = stratY;
    mRecordData.template_height     = endY - stratY;
    mRecordData.template_road_width = roadWidth;
    mRecordData.fill_bedrock        = fillBedrock;
    mRecordData.default_block       = defaultBlock;

    mIsRecording = true;
    return true;
}
bool TemplateManager::postRecordTemplateStart(const ChunkPos& start) {
    if (!mIsRecording) return false;
    if (start.x != 0 && start.z != 0) return false; // 限定 (0,0) 开始

    mRecordStart = start;

    return true;
}
bool TemplateManager::postRecordTemplateEnd(const ChunkPos& end) {
    if (!mIsRecording) return false;
    if (end.x != end.z) return false; // 限定正方形

    mRecordEnd                     = end;
    mRecordData.template_chunk_num = end.x + 1; // +1 因为 (0,0) 开始

    mCanRecord = true;

    return true;
}


bool TemplateManager::_processChunk(const LevelChunk& chunk) {
    if (!mCanRecord) return false;
    auto const& min = chunk.getMin();
    auto const& bs  = chunk.getDimension().getBlockSourceFromMainChunkSource();

    // 获取当前区块的 buffer
    auto& map    = mRecordBlockIDMap;
    auto& data   = mRecordData;
    auto& buffer = data.template_data[calculateChunkID(chunk.getPosition())];


    int totalHeight = data.template_offset + data.template_height + 64; // y 偏移量 + buffer 高度
    if (data.template_offset < 0) totalHeight += 64;                    // + 原版-64
    int const totalVolume = totalHeight * 16 * 16;                      // buffer 体积  16 * 16 * totalHeight

    buffer.reserve(totalVolume); // 预分配空间

    BlockPos cur(min);
    for (int x = 0; x < 16; x++) {
        cur.x += x;
        for (int z = 0; z < 16; z++) {
            cur.z += z;
            cur.y  = mRecordData.template_offset; // 重置 y 轴
            for (int y = mRecordData.template_offset; y < mRecordData.template_height; y++) {
                cur.y      += y;
                auto& bl    = bs.getBlock(cur).getTypeName();
                auto  iter  = map.find(bl);
                if (iter == map.end()) {
                    map[bl] = map.size() + 1;
                    buffer.push_back(map[bl]);
                } else {
                    buffer.push_back(iter->second);
                }
            }
        }
    }

    return true;
}
bool TemplateManager::postRecordAndSaveTemplate(const string& fileName, Player& player) {
    if (!mCanRecord) return false;

    mCanRecord = false;
    mRecordData.block_map.clear();
    mRecordData.template_data.clear();

    auto const& bs = player.getDimensionBlockSourceConst();

    for (int x = mRecordStart.x; x <= mRecordEnd.x; x++) {
        for (int z = mRecordStart.z; z <= mRecordEnd.z; z++) {
            auto ch = bs.getChunk(x, z);
            if (!ch) {
                sendText<LogLevel::Error>(player, "[TemplateManager] Can't get chunk ({},{}).", x, z);
                continue;
            }
            if (!ch->isFullyLoaded()) {
                sendText<LogLevel::Error>(player, "[TemplateManager] Chunk ({},{}) is not fully loaded.", x, z);
                continue;
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