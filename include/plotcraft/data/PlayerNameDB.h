#pragma once
#include "PlotMetadata.h"
#include "ll/api/data/KeyValueDB.h"
#include "mc/deps/core/mce/UUID.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/Global.h"


using string = std::string;

namespace plot::data {


class PlayerNameDB {
private:
    std::unique_ptr<ll::data::KeyValueDB> mPlayerNameDB;

    PlayerNameDB()                               = default;
    PlayerNameDB(const PlayerNameDB&)            = delete;
    PlayerNameDB& operator=(const PlayerNameDB&) = delete;

public:
    PLAPI static PlayerNameDB& getInstance();
    PLAPI bool                 initPlayerNameDB();

    PLAPI bool hasPlayer(string const& realName);
    PLAPI bool hasPlayer(UUIDm const& uuid); // mce::UUID

    PLAPI string getPlayerName(UUIDs const& uuid);
    PLAPI UUIDs  getPlayerUUID(string const& realName);

    PLAPI bool insertPlayer(Player& player);
};


} // namespace plot::data
