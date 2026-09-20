#include "containers/stack.hpp"

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

int register_stack_commands(Tcl_Interp* interp) {
    Tcl_CreateNamespace(interp, "::std::stack", nullptr, nullptr);

    TCLXX_CMD_NEW0(interp, "::std::stack::new", stdcontainers::Stack);
    TCLXX_CMD_GETTER(interp, "::std::stack::size", &stdcontainers::stack_size);
    TCLXX_CMD_GETTER(interp, "::std::stack::empty", &stdcontainers::stack_empty);
    TCLXX_CMD_GETTER(interp, "::std::stack::top", &stdcontainers::stack_top);
    TCLXX_CMD_GETTER(interp, "::std::stack::list", &stdcontainers::stack_to_list);
    TCLXX_CMD_SETTER(interp, "::std::stack::clear", &stdcontainers::stack_clear);
    TCLXX_CMD_SETTER(interp, "::std::stack::pop", &stdcontainers::stack_pop);
    Tcl_CreateObjCommand(
        interp,
        "::std::stack::push",
        stdcontainers::cmd_helpers::variadic_push_cmd<
            stdcontainers::Stack,
            &stdcontainers::stack_push>,
        nullptr,
        nullptr);

    return TCL_OK;
}

} // namespace

namespace tclxx {

template <>
std::string ObjType<stdcontainers::Stack>::ToString(const stdcontainers::Stack& v) {
    return to_tcl_string(stdcontainers::stack_to_list(&v));
}

template <>
void ObjType<stdcontainers::Stack>::Startup(stdcontainers::Stack* value) noexcept {
    if (value) {
        for (Tcl_Obj* item : *value) {
            stdcontainers::detail::retain(item);
        }
    }
}

template <>
void ObjType<stdcontainers::Stack>::Cleanup(stdcontainers::Stack* value) noexcept {
    if (value) {
        stdcontainers::stack_clear(value);
    }
}

} // namespace tclxx

namespace stdcontainers {

int InitStackPackage(Tcl_Interp* interp) {
    if (!interp) {
        return TCL_ERROR;
    }

    if (init_common::ensure_base_namespaces(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_stack_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    return TCL_OK;
}

} // namespace stdcontainers

extern "C" int Stdcontainersstack_Init(Tcl_Interp* interp) {
    if (stdcontainers::InitStackPackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    return stdcontainers::init_common::provide_package(interp);
}
