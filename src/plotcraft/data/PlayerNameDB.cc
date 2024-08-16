#include "plotcraft/data/PlayerNameDB.h"
#include "plugin/MyPlugin.h"

namespace plo::data {

PlayerNameDB& PlayerNameDB::getInstance() {
    static PlayerNameDB instance;
    return instance;
}
bool PlayerNameDB::initPlayerNameDB() {
    if (mPlayerNameDB != nullptr) return true;
    mPlayerNameDB = std::make_unique<ll::data::KeyValueDB>(
        my_plugin::MyPlugin::getInstance().getSelf().getDataDir() / "PlayerNameDB"
    );
    return true;
}

bool PlayerNameDB::hasPlayer(string const& realName) { return mPlayerNameDB->has(realName); }
bool PlayerNameDB::hasPlayer(mce::UUID const& uuid) { return mPlayerNameDB->has(uuid.asString()); }

string PlayerNameDB::getPlayerName(UUIDs const& uuid) {
    auto fn = mPlayerNameDB->get(uuid);
    if (fn) return *fn;
    return "";
}
UUIDs PlayerNameDB::getPlayerUUID(string const& realName) {
    if (hasPlayer(realName)) return *mPlayerNameDB->get(realName);
    return "";
}

bool PlayerNameDB::insertPlayer(Player& player) {
    if (hasPlayer(player.getUuid()) || hasPlayer(player.getRealName())) return false;
    string uuidStr = player.getUuid().asString();
    string name    = player.getRealName();
    mPlayerNameDB->set(uuidStr, name);
    mPlayerNameDB->set(name, uuidStr);
    return true;
}


} // namespace plo::data