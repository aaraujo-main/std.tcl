#include "containers/list.hpp"

#include <string>

#include "command_helpers.hpp"
#include "init_common.hpp"
#include "tclxx.hpp"

namespace {

std::string to_tcl_string(Tcl_Obj* obj) {
    Tcl_IncrRefCount(obj);
    int length = 0;
    const char* bytes = Tcl_GetStringFromObj(obj, &length);
    std::string out(bytes, static_cast<std::size_t>(length));
    Tcl_DecrRefCount(obj);
    return out;
}

int register_list_commands(Tcl_Interp* interp) {
    Tcl_CreateNamespace(interp, "::std::list", nullptr, nullptr);

    TCLXX_CMD_NEW0(interp, "::std::list::new", stdcontainers::ListContainer);
    TCLXX_CMD_NEW0_SHARED(interp, "::std::list::new.shared", stdcontainers::ListContainer);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::list::size", &stdcontainers::ListContainer::size);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::list::empty", &stdcontainers::ListContainer::empty);
    Tcl_CreateObjCommand(
        interp,
        "::std::list::at",
        stdcontainers::cmd_helpers::getter_obj_int_cmd<
            stdcontainers::ListContainer,
            &stdcontainers::ListContainer::at>,
        nullptr,
        nullptr);
    Tcl_CreateObjCommand(
        interp,
        "::std::list::list",
        stdcontainers::cmd_helpers::getter_obj0_cmd<
            stdcontainers::ListContainer,
            &stdcontainers::ListContainer::to_list>,
        nullptr,
        nullptr);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::list::clear", &stdcontainers::ListContainer::clear);
    Tcl_CreateObjCommand(
        interp,
        "::std::list::pop",
        stdcontainers::cmd_helpers::setter_obj0_cmd<
            stdcontainers::ListContainer,
            &stdcontainers::ListContainer::pop_back_take>,
        nullptr,
        nullptr);
    Tcl_CreateObjCommand(
        interp,
        "::std::list::push",
        stdcontainers::cmd_helpers::variadic_push_cmd<
            stdcontainers::ListContainer,
            &stdcontainers::ListContainer::push_back_one>,
        nullptr,
        nullptr);

    return TCL_OK;
}

} // namespace

namespace tclxx {

template <>
std::string ObjType<stdcontainers::ListContainer>::ToString(const stdcontainers::ListContainer& v) {
    return to_tcl_string(v.to_list());
}

template <>
void ObjType<stdcontainers::ListContainer>::Startup(stdcontainers::ListContainer*) noexcept {}

template <>
void ObjType<stdcontainers::ListContainer>::Cleanup(stdcontainers::ListContainer*) noexcept {}

} // namespace tclxx

namespace stdcontainers {

int InitListPackage(Tcl_Interp* interp) {
    if (!interp) {
        return TCL_ERROR;
    }

    if (init_common::ensure_base_namespaces(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_list_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    return TCL_OK;
}

} // namespace stdcontainers

extern "C" int Stdcontainerslist_Init(Tcl_Interp* interp) {
    if (stdcontainers::InitListPackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    return stdcontainers::init_common::provide_package(interp);
}
