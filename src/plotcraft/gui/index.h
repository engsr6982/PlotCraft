#include "ll/api/form/CustomForm.h"
#include "ll/api/form/ModalForm.h"
#include "ll/api/form/SimpleForm.h"
#include "mc/world/actor/player/Player.h"
#include "plotcraft/Config.h"
#include "plotcraft/DataBase.h"
#include "plotcraft/PlotPos.h"
#include "plotcraft/utils/Date.h"
#include "plotcraft/utils/Mc.h"
#include "plotcraft/utils/Menu.h"
#include "plotcraft/utils/Moneys.h"
#include "plotcraft/utils/Text.h"
#include "plotcraft/utils/Utils.h"


namespace plo::gui {

void index(Player& player);

void plot(Player& player, PlotPos plotPos);

} // namespace plo::gui