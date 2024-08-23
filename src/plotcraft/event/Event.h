#pragma once
#include "plotcraft/data/PlotDBStorage.h"
#include "plotcraft/data/PlotMetadata.h"
#include <string>


namespace plo::event {


bool registerEventListener();

bool unRegisterEventListener();


bool CheckPerm(data::PlotDBStorage* pdb, data::PlotID const& id, data::UUIDs const& uuid, bool ignoreAdmin = false);
bool StringFind(std::string const& str, std::string const& sub);


} // namespace plo::event