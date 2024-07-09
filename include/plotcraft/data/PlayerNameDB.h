#include "ll/api/data/KeyValueDB.h"
#include "mc/deps/core/mce/UUID.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/Macro.h"

using string = std::string;

namespace plo::database {


class PlayerNameDB {
private:
    std::unique_ptr<ll::data::KeyValueDB> mPlayerNameDB;
    bool                                  isInit = false;

    PlayerNameDB()                               = default;
    PlayerNameDB(const PlayerNameDB&)            = delete;
    PlayerNameDB& operator=(const PlayerNameDB&) = delete;

public:
    PLAPI static PlayerNameDB& getInstance();
    PLAPI bool                 initPlayerNameDB();

    PLAPI bool hasPlayer(string const& realName);
    PLAPI bool hasPlayer(mce::UUID const& uuid);

    PLAPI string getPlayerName(mce::UUID const& uuid);
    PLAPI std::optional<mce::UUID> getPlayerUUID(string const& realName);

    PLAPI bool insertPlayer(Player& player);
};


} // namespace plo::database
