#pragma once
#include "plotcraft/Global.h"
#include "plotcraft/data/PlotDBStorage.h"
#include <string>


namespace plo::event {


bool registerEventListener();

bool unRegisterEventListener();


bool CheckPerm(data::PlotDBStorage* pdb, PlotID const& id, UUIDs const& uuid, bool ignoreAdmin = false);
bool StringFind(std::string const& str, std::string const& sub);


} // namespace plo::event