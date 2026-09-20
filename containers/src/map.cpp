#include "containers/map.hpp"

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

int register_map_commands(Tcl_Interp* interp) {
    Tcl_CreateNamespace(interp, "::std::map", nullptr, nullptr);

    TCLXX_CMD_NEW0(interp, "::std::map::new", stdcontainers::Map);
    TCLXX_CMD_GETTER(interp, "::std::map::size", &stdcontainers::map_size);
    TCLXX_CMD_GETTER(interp, "::std::map::empty", &stdcontainers::map_empty);
    TCLXX_CMD_GETTER(interp, "::std::map::exists", &stdcontainers::map_exists);
    TCLXX_CMD_GETTER(interp, "::std::map::get", &stdcontainers::map_get);
    TCLXX_CMD_GETTER(interp, "::std::map::keys", &stdcontainers::map_keys);
    TCLXX_CMD_GETTER(interp, "::std::map::dict", &stdcontainers::map_to_dict);
    TCLXX_CMD_SETTER(interp, "::std::map::put", &stdcontainers::map_put);
    TCLXX_CMD_SETTER(interp, "::std::map::erase", &stdcontainers::map_erase);
    TCLXX_CMD_SETTER(interp, "::std::map::clear", &stdcontainers::map_clear);

    return TCL_OK;
}

} // namespace

namespace tclxx {

template <>
std::string ObjType<stdcontainers::Map>::ToString(const stdcontainers::Map& v) {
    return to_tcl_string(stdcontainers::map_to_dict(&v));
}

template <>
void ObjType<stdcontainers::Map>::Startup(stdcontainers::Map* value) noexcept {
    if (value) {
        for (const auto& kv : *value) {
            stdcontainers::detail::retain(kv.first);
            stdcontainers::detail::retain(kv.second);
        }
    }
}

template <>
void ObjType<stdcontainers::Map>::Cleanup(stdcontainers::Map* value) noexcept {
    if (value) {
        stdcontainers::map_clear(value);
    }
}

} // namespace tclxx

namespace stdcontainers {

int InitMapPackage(Tcl_Interp* interp) {
    if (!interp) {
        return TCL_ERROR;
    }

    if (init_common::ensure_base_namespaces(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_map_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    return TCL_OK;
}

} // namespace stdcontainers

extern "C" int Stdcontainersmap_Init(Tcl_Interp* interp) {
    if (stdcontainers::InitMapPackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    return stdcontainers::init_common::provide_package(interp);
}