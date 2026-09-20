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

    TCLXX_CMD_NEW0(interp, "::std::set::new", stdcontainers::Set);
    TCLXX_CMD_GETTER(interp, "::std::set::size", &stdcontainers::set_size);
    TCLXX_CMD_GETTER(interp, "::std::set::empty", &stdcontainers::set_empty);
    TCLXX_CMD_GETTER(interp, "::std::set::contains", &stdcontainers::set_contains);
    TCLXX_CMD_GETTER(interp, "::std::set::list", &stdcontainers::set_to_list);
    TCLXX_CMD_SETTER(interp, "::std::set::erase", &stdcontainers::set_erase);
    TCLXX_CMD_SETTER(interp, "::std::set::clear", &stdcontainers::set_clear);
    Tcl_CreateObjCommand(
        interp,
        "::std::set::insert",
        stdcontainers::cmd_helpers::variadic_set_insert_cmd<
            stdcontainers::Set,
            &stdcontainers::set_insert>,
        nullptr,
        nullptr);

    return TCL_OK;
}

} // namespace

namespace tclxx {

template <>
std::string ObjType<stdcontainers::Set>::ToString(const stdcontainers::Set& v) {
    return to_tcl_string(stdcontainers::set_to_list(&v));
}

template <>
void ObjType<stdcontainers::Set>::Startup(stdcontainers::Set* value) noexcept {
    if (value) {
        for (Tcl_Obj* key : *value) {
            stdcontainers::detail::retain(key);
        }
    }
}

template <>
void ObjType<stdcontainers::Set>::Cleanup(stdcontainers::Set* value) noexcept {
    if (value) {
        stdcontainers::set_clear(value);
    }
}

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
