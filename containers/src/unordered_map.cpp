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

    TCLXX_CMD_NEW0(interp, "::std::unordered_map::new", stdcontainers::UnorderedMap);
    TCLXX_CMD_GETTER(interp, "::std::unordered_map::size", &stdcontainers::unordered_map_size);
    TCLXX_CMD_GETTER(interp, "::std::unordered_map::empty", &stdcontainers::unordered_map_empty);
    TCLXX_CMD_GETTER(interp, "::std::unordered_map::exists", &stdcontainers::unordered_map_exists);
    TCLXX_CMD_GETTER(interp, "::std::unordered_map::get", &stdcontainers::unordered_map_get);
    TCLXX_CMD_GETTER(interp, "::std::unordered_map::keys", &stdcontainers::unordered_map_keys);
    TCLXX_CMD_GETTER(interp, "::std::unordered_map::dict", &stdcontainers::unordered_map_to_dict);
    TCLXX_CMD_SETTER(interp, "::std::unordered_map::reserve", &stdcontainers::unordered_map_reserve);
    TCLXX_CMD_SETTER(interp, "::std::unordered_map::put", &stdcontainers::unordered_map_put);
    TCLXX_CMD_SETTER(interp, "::std::unordered_map::erase", &stdcontainers::unordered_map_erase);
    TCLXX_CMD_SETTER(interp, "::std::unordered_map::clear", &stdcontainers::unordered_map_clear);

    return TCL_OK;
}

} // namespace

namespace tclxx {

template <>
std::string ObjType<stdcontainers::UnorderedMap>::ToString(
    const stdcontainers::UnorderedMap& v) {
    return to_tcl_string(stdcontainers::unordered_map_to_dict(&v));
}

template <>
void ObjType<stdcontainers::UnorderedMap>::Startup(stdcontainers::UnorderedMap* value) noexcept {
    if (value) {
        for (const auto& kv : *value) {
            stdcontainers::detail::retain(kv.first);
            stdcontainers::detail::retain(kv.second);
        }
    }
}

template <>
void ObjType<stdcontainers::UnorderedMap>::Cleanup(stdcontainers::UnorderedMap* value) noexcept {
    if (value) {
        stdcontainers::unordered_map_clear(value);
    }
}

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
