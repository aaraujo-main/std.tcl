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

    TCLXX_CMD_NEW0(interp, "::std::stack::new", stdcontainers::StackContainer);
    TCLXX_CMD_NEW0_SHARED(interp, "::std::stack::new.shared", stdcontainers::StackContainer);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::stack::size", &stdcontainers::StackContainer::size);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::stack::empty", &stdcontainers::StackContainer::empty);
    Tcl_CreateObjCommand(
        interp,
        "::std::stack::top",
        stdcontainers::cmd_helpers::getter_obj0_cmd<
            stdcontainers::StackContainer,
            &stdcontainers::StackContainer::top>,
        nullptr,
        nullptr);
    Tcl_CreateObjCommand(
        interp,
        "::std::stack::list",
        stdcontainers::cmd_helpers::getter_obj0_cmd<
            stdcontainers::StackContainer,
            &stdcontainers::StackContainer::to_list>,
        nullptr,
        nullptr);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::stack::clear", &stdcontainers::StackContainer::clear);
    Tcl_CreateObjCommand(
        interp,
        "::std::stack::pop",
        stdcontainers::cmd_helpers::setter_obj0_cmd<
            stdcontainers::StackContainer,
            &stdcontainers::StackContainer::pop_take>,
        nullptr,
        nullptr);
    Tcl_CreateObjCommand(
        interp,
        "::std::stack::push",
        stdcontainers::cmd_helpers::variadic_push_cmd<
            stdcontainers::StackContainer,
            &stdcontainers::StackContainer::push_one>,
        nullptr,
        nullptr);

    return TCL_OK;
}

} // namespace

namespace tclxx {

template <>
std::string ObjType<stdcontainers::StackContainer>::ToString(const stdcontainers::StackContainer& v) {
    return to_tcl_string(v.to_list());
}

template <>
void ObjType<stdcontainers::StackContainer>::Startup(stdcontainers::StackContainer*) noexcept {}

template <>
void ObjType<stdcontainers::StackContainer>::Cleanup(stdcontainers::StackContainer*) noexcept {}

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
