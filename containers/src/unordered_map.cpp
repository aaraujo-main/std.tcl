#include "containers/unordered_map.hpp"

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

int register_unordered_map_commands(Tcl_Interp* interp) {
    Tcl_CreateNamespace(interp, "::std::unordered_map", nullptr, nullptr);

    TCLXX_CMD_NEW0(interp, "::std::unordered_map::new", stdcontainers::UnorderedMapContainer);
    TCLXX_CMD_NEW0_SHARED(interp, "::std::unordered_map::new.shared", stdcontainers::UnorderedMapContainer);
    TCLXX_CMD_GETTER_METHOD(
        interp,
        "::std::unordered_map::size",
        &stdcontainers::UnorderedMapContainer::size);
    TCLXX_CMD_GETTER_METHOD(
        interp,
        "::std::unordered_map::empty",
        &stdcontainers::UnorderedMapContainer::empty);
    TCLXX_CMD_GETTER_METHOD(
        interp,
        "::std::unordered_map::exists",
        &stdcontainers::UnorderedMapContainer::exists);
    Tcl_CreateObjCommand(
        interp,
        "::std::unordered_map::get",
        stdcontainers::cmd_helpers::getter_obj_key_cmd<
            stdcontainers::UnorderedMapContainer,
            &stdcontainers::UnorderedMapContainer::get>,
        nullptr,
        nullptr);
    Tcl_CreateObjCommand(
        interp,
        "::std::unordered_map::keys",
        stdcontainers::cmd_helpers::getter_obj0_cmd<
            stdcontainers::UnorderedMapContainer,
            &stdcontainers::UnorderedMapContainer::keys>,
        nullptr,
        nullptr);
    Tcl_CreateObjCommand(
        interp,
        "::std::unordered_map::dict",
        stdcontainers::cmd_helpers::getter_obj0_cmd<
            stdcontainers::UnorderedMapContainer,
            &stdcontainers::UnorderedMapContainer::to_dict>,
        nullptr,
        nullptr);
    TCLXX_CMD_SETTER_METHOD(
        interp,
        "::std::unordered_map::reserve",
        &stdcontainers::UnorderedMapContainer::reserve);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::unordered_map::put", &stdcontainers::UnorderedMapContainer::put);
    TCLXX_CMD_SETTER_METHOD(
        interp,
        "::std::unordered_map::erase",
        &stdcontainers::UnorderedMapContainer::erase_key);
    TCLXX_CMD_SETTER_METHOD(
        interp,
        "::std::unordered_map::clear",
        &stdcontainers::UnorderedMapContainer::clear);

    return TCL_OK;
}

} // namespace

namespace tclxx {

template <>
std::string ObjType<stdcontainers::UnorderedMapContainer>::ToString(
    const stdcontainers::UnorderedMapContainer& v) {
    return to_tcl_string(v.to_dict());
}

template <>
void ObjType<stdcontainers::UnorderedMapContainer>::Startup(stdcontainers::UnorderedMapContainer*) noexcept {}

template <>
void ObjType<stdcontainers::UnorderedMapContainer>::Cleanup(stdcontainers::UnorderedMapContainer*) noexcept {}

} // namespace tclxx

namespace stdcontainers {

int InitUnorderedMapPackage(Tcl_Interp* interp) {
    if (!interp) {
        return TCL_ERROR;
    }

    if (init_common::ensure_base_namespaces(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_unordered_map_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    return TCL_OK;
}

} // namespace stdcontainers

extern "C" int Stdcontainersunordered_map_Init(Tcl_Interp* interp) {
    if (stdcontainers::InitUnorderedMapPackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    return stdcontainers::init_common::provide_package(interp);
}
