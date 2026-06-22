#include "std.hpp"

#include "containers/containers.hpp"
#include "stdthread/stdthread.hpp"

namespace {

constexpr const char* kPackageName = "std";
constexpr const char* kPackageVersion = "0.1.0";

} // namespace

extern "C" int Std_Init(Tcl_Interp* interp) {
    if (interp == nullptr) {
        return TCL_ERROR;
    }

    if (Tcl_InitStubs(interp, "8.6", 0) == nullptr) {
        return TCL_ERROR;
    }

    if (Stdcontainers_Init(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (Stdthread_Init(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    return Tcl_PkgProvideEx(interp, kPackageName, kPackageVersion, nullptr);
}
