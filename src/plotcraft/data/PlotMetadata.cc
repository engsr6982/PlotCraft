#include "plotcraft/data/PlotMetadata.h"
#include "plotcraft/data/PlotBDStorage.h"
#include "plotcraft/utils/Date.h"
#include "plotcraft/utils/JsonHelper.h"
#include <algorithm>
#include <memory>
#include <optional>
#include <vector>


using namespace plo::utils;

namespace plo::data {

PlotMetadataPtr PlotMetadata::make(PlotID const& id, UUID const& owner, string const& name, int x, int z) {
    auto ptr        = std::make_shared<PlotMetadata>();
    ptr->mPlotID    = string(id); // 拷贝
    ptr->mPlotName  = string(name);
    ptr->mPlotX     = int(x);
    ptr->mPlotZ     = int(z);
    ptr->mPlotOwner = UUID(owner);
    return ptr;
}
PlotMetadataPtr PlotMetadata::make(PlotID const& id, UUID const& owner, int x, int z) {
    return make(id, owner, "", x, z);
}
PlotMetadataPtr PlotMetadata::make(PlotID const& id, int x, int z) { return make(id, UUID{}, "", x, z); }
PlotMetadataPtr PlotMetadata::make() { return make(PlotID{}, UUID{}, "", 0, 0); }


bool PlotMetadata::isOwner(UUID const& uuid) const { return mPlotOwner == uuid; }

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

bool PlotMetadata::setPlotOwner(UUID const& uuid) {
    if (uuid.empty()) return false;
    mPlotOwner = UUID(uuid);
    return true;
}

bool PlotMetadata::isSharedPlayer(UUID const& uuid) const {
    return std::find_if(
               mSharedPlayers.begin(),
               mSharedPlayers.end(),
               [uuid](auto const& p) { return p.mSharedPlayer == uuid; }
           )
        != mSharedPlayers.end();
}

bool PlotMetadata::addSharedPlayer(UUID const& uuid) {
    if (isSharedPlayer(uuid)) return false;
    mSharedPlayers.emplace_back(PlotShareItem{uuid, Date{}.toString()});
    return true;
}

bool PlotMetadata::delSharedPlayer(UUID const& uuid) {
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

bool PlotMetadata::isCommentOwner(CommentID const& id, UUID const& uuid) const {
    auto opt = getComment(id);
    if (!opt.has_value()) return false;

    return opt->mCommentPlayer == uuid;
}

bool PlotMetadata::addComment(UUID const& uuid, string const& text) {
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

std::vector<PlotCommentItem> PlotMetadata::getComments(UUID const& uuid) const {
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

UUID PlotMetadata::getPlotOwner() const { return mPlotOwner; }

int PlotMetadata::getX() const { return mPlotX; }

int PlotMetadata::getZ() const { return mPlotZ; }

PlotPermission PlotMetadata::getPlayerInThisPlotPermission(UUID const& uuid) const {
    if (mPlotOwner == uuid) return PlotPermission::Owner;
    if (isSharedPlayer(uuid)) return PlotPermission::Shared;
    return PlotPermission::None;
}


PlotPermissionTable&       PlotMetadata::getPermissionTable() { return mPermissionTable; }
PlotPermissionTable const& PlotMetadata::getPermissionTable() const { return mPermissionTable; }


void PlotMetadata::save() { PlotBDStorage::getInstance().save(*this); }

string PlotMetadata::toString() const { return JsonHelper::structToJson(*this).dump(); }


} // namespace plo::data