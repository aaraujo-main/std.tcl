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

    TCLXX_CMD_NEW0(interp, "::std::list::new", stdcontainers::List);
    TCLXX_CMD_GETTER(interp, "::std::list::size", &stdcontainers::list_size);
    TCLXX_CMD_GETTER(interp, "::std::list::empty", &stdcontainers::list_empty);
    TCLXX_CMD_GETTER(interp, "::std::list::at", &stdcontainers::list_at);
    TCLXX_CMD_GETTER(interp, "::std::list::list", &stdcontainers::list_to_list);
    TCLXX_CMD_SETTER(interp, "::std::list::clear", &stdcontainers::list_clear);
    TCLXX_CMD_SETTER(interp, "::std::list::pop", &stdcontainers::list_pop);
    Tcl_CreateObjCommand(
        interp,
        "::std::list::push",
        stdcontainers::cmd_helpers::variadic_push_cmd<
            stdcontainers::List,
            &stdcontainers::list_push>,
        nullptr,
        nullptr);

    return TCL_OK;
}

} // namespace

namespace tclxx {

template <>
std::string ObjType<stdcontainers::List>::ToString(const stdcontainers::List& v) {
    return to_tcl_string(stdcontainers::list_to_list(&v));
}

template <>
void ObjType<stdcontainers::List>::Startup(stdcontainers::List* value) noexcept {
    if (value) {
        for (Tcl_Obj* item : *value) {
            stdcontainers::detail::retain(item);
        }
    }
}

template <>
void ObjType<stdcontainers::List>::Cleanup(stdcontainers::List* value) noexcept {
    if (value) {
        stdcontainers::list_clear(value);
    }
}

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
