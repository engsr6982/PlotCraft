#include "Command.h"
#include "ll/api/command/CommandRegistrar.h"


namespace plo::command {


enum class OperationOP : int {

};


bool registerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand("plo", "PlotCraft");


    return true;
}


} // namespace plo::command