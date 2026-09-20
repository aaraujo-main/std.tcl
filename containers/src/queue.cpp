#include "containers/queue.hpp"

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

int register_queue_commands(Tcl_Interp* interp) {
    Tcl_CreateNamespace(interp, "::std::queue", nullptr, nullptr);

    TCLXX_CMD_NEW0(interp, "::std::queue::new", stdcontainers::Queue);
    TCLXX_CMD_GETTER(interp, "::std::queue::size", &stdcontainers::queue_size);
    TCLXX_CMD_GETTER(interp, "::std::queue::empty", &stdcontainers::queue_empty);
    TCLXX_CMD_GETTER(interp, "::std::queue::front", &stdcontainers::queue_front);
    TCLXX_CMD_GETTER(interp, "::std::queue::list", &stdcontainers::queue_to_list);
    TCLXX_CMD_SETTER(interp, "::std::queue::pop", &stdcontainers::queue_pop);
    TCLXX_CMD_SETTER(interp, "::std::queue::clear", &stdcontainers::queue_clear);
    Tcl_CreateObjCommand(
        interp,
        "::std::queue::push",
        stdcontainers::cmd_helpers::variadic_push_cmd<
            stdcontainers::Queue,
            &stdcontainers::queue_push>,
        nullptr,
        nullptr);

    return TCL_OK;
}

} // namespace

namespace tclxx {

template <>
std::string ObjType<stdcontainers::Queue>::ToString(const stdcontainers::Queue& v) {
    return to_tcl_string(stdcontainers::queue_to_list(&v));
}

template <>
void ObjType<stdcontainers::Queue>::Startup(stdcontainers::Queue* value) noexcept {
    if (value) {
        stdcontainers::Queue copy = *value;
        while (!copy.empty()) {
            stdcontainers::detail::retain(copy.front());
            copy.pop();
        }
    }
}

template <>
void ObjType<stdcontainers::Queue>::Cleanup(stdcontainers::Queue* value) noexcept {
    if (value) {
        stdcontainers::queue_clear(value);
    }
}

} // namespace tclxx

namespace stdcontainers {

int InitQueuePackage(Tcl_Interp* interp) {
    if (!interp) {
        return TCL_ERROR;
    }

    if (init_common::ensure_base_namespaces(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_queue_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    return TCL_OK;
}

} // namespace stdcontainers

extern "C" int Stdcontainersqueue_Init(Tcl_Interp* interp) {
    if (stdcontainers::InitQueuePackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    return stdcontainers::init_common::provide_package(interp);
}