#include "index.h"
#include "ll/api/form/SimpleForm.h"


using namespace ll::form;
using namespace plo::utils;
using namespace plo::database;

namespace plo::gui {

void index(Player& player) {}


void plot(Player& player, PlotPos plotPos) {
    auto& db   = PlotDB::getInstance();
    auto& impl = db.getImpl();
    auto  p    = db.getCached(plotPos.getPlotID());
    if (!p.has_value()) {
        p = impl.getPlot(plotPos.getPlotID()); // 尝试从数据库中获取
    }

    if (!p.has_value()) {
        // 无主地皮，跳转购买页面
    }
}


} // namespace plo::gui