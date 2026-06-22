#include "containers/set.hpp"

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

int register_set_commands(Tcl_Interp* interp) {
    Tcl_CreateNamespace(interp, "::std::set", nullptr, nullptr);

    TCLXX_CMD_NEW0(interp, "::std::set::new", stdcontainers::SetContainer);
    TCLXX_CMD_NEW0_SHARED(interp, "::std::set::new.shared", stdcontainers::SetContainer);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::set::size", &stdcontainers::SetContainer::size);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::set::empty", &stdcontainers::SetContainer::empty);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::set::contains", &stdcontainers::SetContainer::contains);
    Tcl_CreateObjCommand(
        interp,
        "::std::set::list",
        stdcontainers::cmd_helpers::getter_obj0_cmd<
            stdcontainers::SetContainer,
            &stdcontainers::SetContainer::to_list>,
        nullptr,
        nullptr);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::set::erase", &stdcontainers::SetContainer::erase_key);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::set::clear", &stdcontainers::SetContainer::clear);
    Tcl_CreateObjCommand(
        interp,
        "::std::set::insert",
        stdcontainers::cmd_helpers::variadic_set_insert_cmd<
            stdcontainers::SetContainer,
            &stdcontainers::SetContainer::insert_one>,
        nullptr,
        nullptr);

    return TCL_OK;
}

} // namespace

namespace tclxx {

template <>
std::string ObjType<stdcontainers::SetContainer>::ToString(const stdcontainers::SetContainer& v) {
    return to_tcl_string(v.to_list());
}

template <>
void ObjType<stdcontainers::SetContainer>::Startup(stdcontainers::SetContainer*) noexcept {}

template <>
void ObjType<stdcontainers::SetContainer>::Cleanup(stdcontainers::SetContainer*) noexcept {}

} // namespace tclxx

namespace stdcontainers {

int InitSetPackage(Tcl_Interp* interp) {
    if (!interp) {
        return TCL_ERROR;
    }

    if (init_common::ensure_base_namespaces(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_set_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    return TCL_OK;
}

} // namespace stdcontainers

extern "C" int Stdcontainersset_Init(Tcl_Interp* interp) {
    if (stdcontainers::InitSetPackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    return stdcontainers::init_common::provide_package(interp);
}
