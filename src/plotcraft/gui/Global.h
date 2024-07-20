#include "fmt/format.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/form/CustomForm.h"
#include "ll/api/form/FormBase.h"
#include "ll/api/form/ModalForm.h"
#include "ll/api/form/SimpleForm.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemStackBase.h"
#include "mc/world/level/dimension/VanillaDimensions.h"
#include "plotcraft/Config.h"
#include "plotcraft/EconomyQueue.h"
#include "plotcraft/PlotPos.h"
#include "plotcraft/core/CoreUtils.h"
#include "plotcraft/data/PlayerNameDB.h"
#include "plotcraft/data/PlotBDStorage.h"
#include "plotcraft/data/PlotMetadata.h"
#include "plotcraft/event/PlotEvents.h"
#include "plotcraft/utils/Date.h"
#include "plotcraft/utils/JsonHelper.h"
#include "plotcraft/utils/Mc.h"
#include "plotcraft/utils/Menu.h"
#include "plotcraft/utils/Moneys.h"
#include "plotcraft/utils/Text.h"
#include "plotcraft/utils/Utils.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <regex>
#include <string>


namespace plo::gui {

using namespace plo::data;
using namespace ll::form;
using namespace plo::utils;
using namespace plo::core_utils;
namespace pev = plo::event;


// MainGUI.cc
void MainGUI(Player& player);


// PlayerSettingGUI.cc
void PlayerSettingGUI(Player& player);


// PlotCommentGUI.cc
void PlotCommentGUI(Player& player, std::shared_ptr<PlotMetadata> pt);
void _publishComment(Player& player, std::shared_ptr<PlotMetadata> pt);
void _showCommentOperation(Player& player, std::shared_ptr<PlotMetadata> pt, CommentID id);
void _editComment(Player& player, std::shared_ptr<PlotMetadata> pt, CommentID id);


// PlotGUI.cc
void _selectPlot(Player& player);
void PlotGUI(Player& player, std::shared_ptr<PlotMetadata> plot, bool ret = false);
void _changePlotName(Player& player, std::shared_ptr<PlotMetadata> pt);


// PlotSaleGUI.cc
void PlotSaleGUI(Player& player, std::shared_ptr<PlotMetadata> pt);
void _sellPlotAndEditPrice(Player& player, std::shared_ptr<PlotMetadata> pt, bool edit);


// PlotShareGUI.cc
void PlotShareGUI(Player& player, std::shared_ptr<PlotMetadata> pt);
void _addSharePlayer(Player& player, std::shared_ptr<PlotMetadata> pt);


// PlotShopGUI.cc
void PlotShopGUI(Player& player);
void _plotShopShowPlot(Player& player, std::shared_ptr<PlotMetadata> pt);
void _buyPlot(Player& player, std::shared_ptr<PlotMetadata> pt);


// PluginSettingGUI.cc
void PluginSettingGUI(Player& player);


} // namespace plo::gui