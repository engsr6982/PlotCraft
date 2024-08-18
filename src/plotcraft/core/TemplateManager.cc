#include "TemplateManager.h"
#include "mc/deps/core/utility/buffer_span_mut.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockLegacy.h"
#include "mc/world/level/block/BlockVolume.h"
#include "mc/world/level/block/registry/BlockTypeRegistry.h"
#include "nlohmann/json.hpp"
#include "plotcraft/Config.h"
#include "plotcraft/utils/JsonHelper.h"
#include <algorithm>
#include <filesystem>


namespace plo::core {
namespace fs = std::filesystem;
using json   = nlohmann::json;

// static member definitions
TemplateData                                          TemplateManager::mTemplateData;
std::unordered_map<string, Block const*>              TemplateManager::mBlockMap;
std::unordered_map<string, std::vector<Block const*>> TemplateManager::mBlockBuffer;
std::unordered_map<string, BlockVolume>               TemplateManager::mBlockVolume;
TemplateData                                          TemplateManager::mRecordTemplateData;
bool                                                  TemplateManager::mIsRecordTemplateing;


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
                auto bl                       = mBlockMap[data.block_map[templateBlocks[y * r]]]; // 获取方块
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
    utils::JsonHelper::jsonToStructNoMerge(jdata, mTemplateData);

    return _parseTemplate();
}

// Tools
int TemplateManager::getChunkNum() {
    return mTemplateData.lock_chunk_num ? mTemplateData.template_chunk_num : config::cfg.generator.cuPlotChunkNum;
}
string TemplateManager::calculateChunkID(const ChunkPos& pos) {
    int n = getChunkNum();
    return fmt::format("({},{})", (pos.x * 16 / n), (pos.z * 16 / n));
}


// Record

} // namespace plo::core