#include "plotcraft/data/PlotMetadata.h"
#include "plotcraft/data/PlotDBStorage.h"
#include "plotcraft/utils/Date.h"
#include "plotcraft/utils/JsonHelper.h"
#include <algorithm>
#include <memory>
#include <optional>
#include <vector>


using namespace plot::utils;

namespace plot::data {


// 构造
PlotMetadataPtr PlotMetadata::make(PlotID const& id, UUIDs const& owner, string const& name, int x, int z) {
    auto ptr        = std::make_shared<PlotMetadata>();
    ptr->mPlotID    = string(id); // 拷贝
    ptr->mPlotName  = string(name);
    ptr->mPlotX     = int(x);
    ptr->mPlotZ     = int(z);
    ptr->mPlotOwner = UUIDs(owner);
    return ptr;
}
PlotMetadataPtr PlotMetadata::make(PlotID const& id, UUIDs const& owner, int x, int z) {
    return make(id, owner, "", x, z);
}
PlotMetadataPtr PlotMetadata::make(PlotID const& id, int x, int z) { return make(id, UUIDs{}, "", x, z); }
PlotMetadataPtr PlotMetadata::make() { return make(PlotID{}, UUIDs{}, "", 0, 0); }


// API
bool PlotMetadata::isMerged() const { return mMerged; }

bool PlotMetadata::isOwner(UUIDs const& uuid) const { return mPlotOwner == uuid; }

bool PlotMetadata::setPlotName(string const& name) {
    mPlotName = string(name);
    return true;
}

bool PlotMetadata::setPlotID(const PlotID& id) {
    if (id.empty()) return false;
    mPlotID = PlotID(id);
    return true;
}

bool PlotMetadata::setX(int x) {
    mPlotX = x;
    return true;
}

bool PlotMetadata::setZ(int z) {
    mPlotZ = z;
    return true;
}

bool PlotMetadata::setPlotOwner(UUIDs const& uuid) {
    if (uuid.empty()) return false;
    mPlotOwner = UUIDs(uuid);
    return true;
}

bool PlotMetadata::isSharedPlayer(UUIDs const& uuid) const {
    return std::find_if(
               mSharedPlayers.begin(),
               mSharedPlayers.end(),
               [uuid](auto const& p) { return p.mSharedPlayer == uuid; }
           )
        != mSharedPlayers.end();
}

bool PlotMetadata::addSharedPlayer(UUIDs const& uuid) {
    if (isSharedPlayer(uuid)) return false;
    mSharedPlayers.emplace_back(PlotShareItem{uuid, Date{}.toString()});
    return true;
}

bool PlotMetadata::delSharedPlayer(UUIDs const& uuid) {
    auto it = std::find_if(mSharedPlayers.begin(), mSharedPlayers.end(), [uuid](auto const& p) {
        return p.mSharedPlayer == uuid;
    });
    if (it == mSharedPlayers.end()) return false;
    mSharedPlayers.erase(it);
    return true;
}

bool PlotMetadata::resetSharedPlayers() {
    mSharedPlayers.clear();
    return true;
}

std::vector<PlotShareItem> PlotMetadata::getSharedPlayers() const { return mSharedPlayers; }


bool PlotMetadata::hasComment(CommentID const& id) const {
    return std::find_if(mComments.begin(), mComments.end(), [id](auto const& c) { return c.mCommentID == id; })
        != mComments.end();
}

bool PlotMetadata::isCommentOwner(CommentID const& id, UUIDs const& uuid) const {
    auto opt = getComment(id);
    if (!opt.has_value()) return false;

    return opt->mCommentPlayer == uuid;
}

bool PlotMetadata::addComment(UUIDs const& uuid, string const& text) {
    if (text.empty()) return false;
    mComments.emplace_back(PlotCommentItem{static_cast<int>(mComments.size()) + 1, uuid, Date{}.toString(), text});
    return true;
}

bool PlotMetadata::delComment(CommentID const& id) {
    auto it = std::find_if(mComments.begin(), mComments.end(), [id](auto const& c) { return c.mCommentID == id; });
    if (it == mComments.end()) return false;
    mComments.erase(it);
    return true;
}

bool PlotMetadata::resetComments() {
    mComments.clear();
    return true;
}

bool PlotMetadata::setCommentContent(CommentID const& id, string const& text) {
    auto it = std::find_if(mComments.begin(), mComments.end(), [id](auto const& c) { return c.mCommentID == id; });
    if (it == mComments.end()) return false;
    it->mContent = text;
    return true;
}

std::optional<PlotCommentItem> PlotMetadata::getComment(CommentID const& id) const {
    auto it = std::find_if(mComments.begin(), mComments.end(), [id](auto const& c) { return c.mCommentID == id; });
    if (it == mComments.end()) return std::nullopt;
    return *it;
}

std::vector<PlotCommentItem> PlotMetadata::getComments() const { return mComments; }

std::vector<PlotCommentItem> PlotMetadata::getComments(UUIDs const& uuid) const {
    std::vector<PlotCommentItem> res;
    std::copy_if(mComments.begin(), mComments.end(), std::back_inserter(res), [uuid](auto const& c) {
        return c.mCommentPlayer == uuid;
    });
    return res;
}

bool PlotMetadata::setSaleStatus(bool isSale) {
    mIsSale = isSale;
    return true;
}

bool PlotMetadata::setSaleStatus(bool isSale, int price) {
    mIsSale = isSale;
    mPrice  = price;
    return true;
}

bool PlotMetadata::setSalePrice(int price) {
    mPrice = price;
    return true;
}

bool PlotMetadata::isSale() const { return mIsSale; }

int PlotMetadata::getSalePrice() const { return mPrice; }

PlotID PlotMetadata::getPlotID() const { return mPlotID; }

string PlotMetadata::getPlotName() const { return mPlotName; }

UUIDs PlotMetadata::getPlotOwner() const { return mPlotOwner; }

int PlotMetadata::getX() const { return mPlotX; }

int PlotMetadata::getZ() const { return mPlotZ; }

PlotPermission PlotMetadata::getPlayerInThisPlotPermission(UUIDs const& uuid) const {
    if (isOwner(uuid)) return PlotPermission::Owner;
    if (isSharedPlayer(uuid)) return PlotPermission::Shared;
    return PlotPermission::None;
}


PlotPermissionTable&       PlotMetadata::getPermissionTable() { return mPermissionTable; }
PlotPermissionTable const& PlotMetadata::getPermissionTableConst() const { return mPermissionTable; }


void PlotMetadata::save() { PlotDBStorage::getInstance().save(*this); }

string PlotMetadata::toString() const { return JsonHelper::structToJson(*this).dump(); }


void PlotMetadata::mergeData(PlotMetadataPtr const other, bool mergeComment, bool mergeSharedPlayer) {
    if (!other) {
        return;
    }

    if (mergeComment) {
        for (auto const& i : other->mComments) {
            this->mComments.push_back(PlotCommentItem{i}); // copy
        }
    }
    if (mergeSharedPlayer) {
        for (auto const& i : other->mSharedPlayers) {
            if (this->isSharedPlayer(i.mSharedPlayer)) {
                continue;
            }
            this->mSharedPlayers.push_back(PlotShareItem{i});
        }
    }
}
void PlotMetadata::updateMergeData(std::unique_ptr<PlotPos> const& pos) {
    if (!pos->isValid()) {
        return;
    }
    // fmt::print("updateMergeData: pos 地址 {}\n", fmt::ptr(std::addressof(*pos)));

    this->mMerged = true;
    auto& data    = this->mMergedData;

    // 更新顶点
    data.mCurrentVertexs.clear();
    data.mCurrentVertexs.reserve(pos->mVertexs.size());
    for (auto const& i : pos->mVertexs) {
        data.mCurrentVertexs.push_back(VertexPos::fromBlockPos(i));
    }

    // 记录被合并的PlotID
    auto plots = std::addressof(*pos)->getRangedPlots();
    data.mMergedPlotIDs.clear();
    data.mMergedPlotIDs.reserve(plots.size());
    for (auto const& i : plots) {
        if (i.getPlotID() == this->mPlotID) continue; // skip self
        data.mMergedPlotIDs.push_back(i.getPlotID());
    }

    // 记录被合并的CrossID
    auto corsses = std::addressof(*pos)->getRangedCrosses();
    data.mMergedCrossIDs.clear();
    data.mMergedCrossIDs.reserve(corsses.size());
    for (auto const& i : corsses) {
        data.mMergedCrossIDs.push_back(i.getCrossID());
    }

    // 记录被合并的RoadID
    auto roads = std::addressof(*pos)->getRangedRoads();
    data.mMergedRoadIDs.clear();
    data.mMergedRoadIDs.reserve(roads.size());
    for (auto const& i : roads) {
        data.mMergedRoadIDs.push_back(i.getRoadID());
    }
}
bool PlotMetadata::setMergeCount(int count) {
    this->mMergedData.mMergeCount = count;
    return true;
}


} // namespace plot::data