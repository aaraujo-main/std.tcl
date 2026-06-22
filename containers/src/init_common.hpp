#pragma once

#include <tcl.h>

namespace stdcontainers::init_common {

inline int ensure_base_namespaces(Tcl_Interp* interp) {
    if (!interp) {
        return TCL_ERROR;
    }
    Tcl_CreateNamespace(interp, "::std", nullptr, nullptr);
    return TCL_OK;
}

inline int provide_package(Tcl_Interp* interp) {
    return Tcl_PkgProvideEx(interp, "std::containers", "0.1.0", nullptr);
}

} // namespace stdcontainers::init_common
