#include "containers/containers.hpp"

#include "init_common.hpp"

namespace stdcontainers {

int RegisterCommands(Tcl_Interp* interp) {
    if (!interp) {
        return TCL_ERROR;
    }

    if (InitVectorPackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    if (InitListPackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    if (InitMapPackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    if (InitQueuePackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    if (InitStackPackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    if (InitSetPackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    if (InitUnorderedMapPackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    if (InitUnorderedSetPackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    return TCL_OK;
}

} // namespace stdcontainers

extern "C" int Stdcontainers_Init(Tcl_Interp* interp) {
    if (!interp) {
        return TCL_ERROR;
    }

    if (stdcontainers::RegisterCommands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    return stdcontainers::init_common::provide_package(interp);
}
