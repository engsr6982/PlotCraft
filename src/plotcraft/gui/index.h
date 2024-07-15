#include "fmt/format.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/form/CustomForm.h"
#include "ll/api/form/FormBase.h"
#include "ll/api/form/ModalForm.h"
#include "ll/api/form/SimpleForm.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemStackBase.h"
#include "mc/world/level/dimension/VanillaDimensions.h"
#include "plotcraft/Config.h"
#include "plotcraft/PlotPos.h"
#include "plotcraft/data/PlayerNameDB.h"
#include "plotcraft/data/PlotBDStorage.h"
#include "plotcraft/data/PlotMetadata.h"
#include "plotcraft/event/PlotEvents.h"
#include "plotcraft/utils/Date.h"
#include "plotcraft/utils/Mc.h"
#include "plotcraft/utils/Menu.h"
#include "plotcraft/utils/Moneys.h"
#include "plotcraft/utils/Text.h"
#include "plotcraft/utils/Utils.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <string>


namespace plo::gui {

using namespace plo::data;

void index(Player& player);

void _selectPlot(Player& player);


void plot(Player& player, PlotPos plotPos);
void plot(Player& player, std::shared_ptr<PlotMetadata> plot, bool ret = false);

void _pluginSetting(Player& player);

void _playerSetting(Player& player);

void _plotShop(Player& player);
void _plotShopShowPlot(Player& player, std::shared_ptr<PlotMetadata> pt);

void _addSharePlayer(Player& player, std::shared_ptr<PlotMetadata> pt);

void _sellMyPlot(Player& player, std::shared_ptr<PlotMetadata> pt);
void _sellPlotAndEditPrice(Player& player, std::shared_ptr<PlotMetadata> pt, bool edit);

void _plotShareManage(Player& player, std::shared_ptr<PlotMetadata> pt);

void _changePlotName(Player& player, std::shared_ptr<PlotMetadata> pt);

void _buyPlot(Player& player, std::shared_ptr<PlotMetadata> pt);

void _plotcomment(Player& player, std::shared_ptr<PlotMetadata> pt);

void _publishComment(Player& player, std::shared_ptr<PlotMetadata> pt);

void _showCommentOperation(Player& player, std::shared_ptr<PlotMetadata> pt, CommentID id);

void _editComment(Player& player, std::shared_ptr<PlotMetadata> pt, CommentID id);


} // namespace plo::gui