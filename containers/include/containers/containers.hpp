#pragma once

#include <tcl.h>

#include "containers/list.hpp"
#include "containers/set.hpp"
#include "containers/stack.hpp"
#include "containers/unordered_map.hpp"
#include "containers/vector.hpp"

namespace stdcontainers {

int RegisterCommands(Tcl_Interp* interp);

} // namespace stdcontainers

extern "C" int Stdcontainers_Init(Tcl_Interp* interp);
