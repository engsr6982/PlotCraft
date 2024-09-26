#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/BlockVolume.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include <string>
#include <unordered_map>
#include <vector>


using string = std::string;

namespace plo::core {

typedef string ints; // string int
struct TemplateData {
    int    version{2};                     // 模板版本
    int    template_chunk_num{2};          // 模板区块数量
    int    template_road_width{2};         // 模板道路宽度
    int    template_height{16};            // 模板高度
    int    template_offset{0};             // 模板偏移
    bool   fill_bedrock{true};             // 是否填充基岩
    string default_block{"minecraft:air"}; // 默认方块

    std::unordered_map<ints, string>             block_map;     // 方块映射 key: hashID
    std::unordered_map<string, std::vector<int>> template_data; // 模板数据 key: chunkID
};


class TemplateManager {
public:
    // Template
    static TemplateData                                          mTemplateData; // 模板数据
    static std::unordered_map<string, Block const*>              mBlockMap;     // 方块映射     key: typeName
    static std::unordered_map<string, std::vector<Block const*>> mBlockBuffer;  // 方块缓冲区   key: chunkID
    static std::unordered_map<string, BlockVolume>               mBlockVolume;  // 方块体积     key: chunkID

    static bool loadTemplate(string const& path);
    static bool _parseTemplate();

    static bool generatorBlockVolume(BlockVolume& volume);

    static bool isUseTemplate();
    static int  getCurrentTemplateChunkNum();
    static int  getCurrentTemplateVersion();
    static int  getCurrentTemplateRoadWidth();

    // Record Template
    static TemplateData                    mRecordData;       // 记录模板数据
    static bool                            mIsRecording;      // 是否正在记录模板
    static bool                            mCanRecord;        // 是否可以记录模板
    static ChunkPos                        mRecordStart;      // 记录模板开始
    static ChunkPos                        mRecordEnd;        // 记录模板结束
    static std::unordered_map<string, int> mRecordBlockIDMap; // 记录方块ID映射 key: typeName

    static bool canRecordTemplate();   // 是否可以记录模板
    static bool isRecordTemplateing(); // 是否正在记录模板

    static bool prepareRecordTemplate( // 准备记录模板
        int           stratY,          // = offset
        int           endY,            // = offset + height
        int           roadWidth,       // = roadWidth
        bool          fillBedrock,
        string const& defaultBlock
    );

    static bool postRecordTemplateStart(ChunkPos const& start); // 记录模板开始
    static bool postRecordTemplateEnd(ChunkPos const& end);     // 记录模板结束

    static bool postRecordAndSaveTemplate(string const& fileName, Player& player); // 记录并保存模板

    static bool resetRecordTemplate(); // 重置记录模板

    static bool _processChunk(LevelChunk const& chunk); // 处理区块


    // Tool functions
    static void   toPositive(int& num);                  // 将数字转换为正数
    static string calculateChunkID(ChunkPos const& pos); // 计算区块ID

private:
    TemplateManager()                                  = delete;
    ~TemplateManager()                                 = delete;
    TemplateManager(TemplateManager const&)            = delete;
    TemplateManager& operator=(TemplateManager const&) = delete;
};

} // namespace plo::core
