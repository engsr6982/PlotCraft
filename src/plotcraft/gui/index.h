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
#include "plotcraft/DataBase.h"
#include "plotcraft/PlotPos.h"
#include "plotcraft/event/PlotEvents.h"
#include "plotcraft/utils/Date.h"
#include "plotcraft/utils/Mc.h"
#include "plotcraft/utils/Menu.h"
#include "plotcraft/utils/Moneys.h"
#include "plotcraft/utils/Text.h"
#include "plotcraft/utils/Utils.h"
#include <algorithm>
#include <functional>
#include <string>


namespace plo::gui {

using namespace plo::database;

void index(Player& player);

void _selectPlot(Player& player);


void plot(Player& player, PlotPos plotPos);
void plot(Player& player, Plot plot, bool ret = false);

void _pluginSetting(Player& player);

void _plotShop(Player& player);
void _plotShopShowPlot(Player& player, Plot pt, PlotSale sl);

void _addSharePlayer(Player& player, Plot pt);

void _sellMyPlot(Player& player, Plot pt);
void _sellPlotAndEditPrice(Player& player, Plot pt, bool edit);

void _plotShareManage(Player& player, Plot pt);

void _changePlotName(Player& player, Plot pt);

void _buyPlot(Player& player, Plot pt);

void _plotcomment(Player& player, Plot pt);

void _publishComment(Player& player, Plot pt);

void _showCommentOperation(Player& player, Plot pt, PlotComment ct);

void _editComment(Player& player, Plot pt, PlotComment ct);


} // namespace plo::gui