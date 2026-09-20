#include "containers/unordered_set.hpp"

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

int register_unordered_set_commands(Tcl_Interp* interp) {
    Tcl_CreateNamespace(interp, "::std::unordered_set", nullptr, nullptr);

    TCLXX_CMD_NEW0(interp, "::std::unordered_set::new", stdcontainers::UnorderedSet);
    TCLXX_CMD_GETTER(interp, "::std::unordered_set::size", &stdcontainers::unordered_set_size);
    TCLXX_CMD_GETTER(interp, "::std::unordered_set::empty", &stdcontainers::unordered_set_empty);
    TCLXX_CMD_GETTER(interp, "::std::unordered_set::contains", &stdcontainers::unordered_set_contains);
    TCLXX_CMD_GETTER(interp, "::std::unordered_set::list", &stdcontainers::unordered_set_to_list);
    TCLXX_CMD_SETTER(interp, "::std::unordered_set::erase", &stdcontainers::unordered_set_erase);
    TCLXX_CMD_SETTER(interp, "::std::unordered_set::clear", &stdcontainers::unordered_set_clear);
    Tcl_CreateObjCommand(
        interp,
        "::std::unordered_set::insert",
        stdcontainers::cmd_helpers::variadic_set_insert_cmd<
            stdcontainers::UnorderedSet,
            &stdcontainers::unordered_set_insert>,
        nullptr,
        nullptr);

    return TCL_OK;
}

} // namespace

namespace tclxx {

template <>
std::string ObjType<stdcontainers::UnorderedSet>::ToString(
    const stdcontainers::UnorderedSet& v) {
    return to_tcl_string(stdcontainers::unordered_set_to_list(&v));
}

template <>
void ObjType<stdcontainers::UnorderedSet>::Startup(stdcontainers::UnorderedSet* value) noexcept {
    if (value) {
        for (Tcl_Obj* key : *value) {
            stdcontainers::detail::retain(key);
        }
    }
}

template <>
void ObjType<stdcontainers::UnorderedSet>::Cleanup(
    stdcontainers::UnorderedSet* value) noexcept {
    if (value) {
        stdcontainers::unordered_set_clear(value);
    }
}

} // namespace tclxx

namespace stdcontainers {

int InitUnorderedSetPackage(Tcl_Interp* interp) {
    if (!interp) {
        return TCL_ERROR;
    }

    if (init_common::ensure_base_namespaces(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_unordered_set_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    return TCL_OK;
}

} // namespace stdcontainers

extern "C" int Stdcontainersunordered_set_Init(Tcl_Interp* interp) {
    if (stdcontainers::InitUnorderedSetPackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    return stdcontainers::init_common::provide_package(interp);
}