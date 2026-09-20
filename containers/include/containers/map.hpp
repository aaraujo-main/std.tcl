#pragma once

#include "containers/detail.hpp"

#include <map>
#include <stdexcept>
#include <utility>

namespace stdcontainers {

using Map = std::map<
    Tcl_Obj*,
    Tcl_Obj*,
    detail::TclObjStringLess,
    TclAlloc<std::pair<Tcl_Obj* const, Tcl_Obj*>>>;

inline int map_size(const Map* value) {
    return static_cast<int>(value->size());
}

inline bool map_empty(const Map* value) {
    return value->empty();
}

inline bool map_exists(const Map* value, Tcl_Obj* key) {
    return value->find(key) != value->end();
}

inline Tcl_Obj* map_get(const Map* value, Tcl_Obj* key) {
    auto it = value->find(key);
    if (it == value->end()) {
        throw std::runtime_error("map key not found");
    }
    return it->second;
}

inline void map_put(Map* value, Tcl_Obj* key, Tcl_Obj* item) {
    Tcl_Obj* held_key = detail::retain(key);
    Tcl_Obj* held_value = detail::retain(item);
    auto [it, inserted] = value->try_emplace(held_key, held_value);
    if (inserted) {
        return;
    }

    detail::release(held_key);
    detail::release(it->second);
    it->second = held_value;
}

inline bool map_erase(Map* value, Tcl_Obj* key) {
    auto it = value->find(key);
    if (it == value->end()) {
        return false;
    }
    detail::release(it->first);
    detail::release(it->second);
    value->erase(it);
    return true;
}

inline void map_clear(Map* value) {
    for (auto& kv : *value) {
        detail::release(kv.first);
        detail::release(kv.second);
    }
    value->clear();
}

inline Tcl_Obj* map_keys(const Map* value) {
    Tcl_Obj* out = Tcl_NewListObj(0, nullptr);
    for (const auto& kv : *value) {
        Tcl_ListObjAppendElement(nullptr, out, kv.first);
    }
    return out;
}

inline Tcl_Obj* map_to_dict(const Map* value) {
    Tcl_Obj* out = Tcl_NewDictObj();
    for (const auto& kv : *value) {
        if (Tcl_DictObjPut(nullptr, out, kv.first, kv.second) != TCL_OK) {
            throw std::runtime_error("map to_dict failed");
        }
    }
    return out;
}

int InitMapPackage(Tcl_Interp* interp);

} // namespace stdcontainers

extern "C" int Stdcontainersmap_Init(Tcl_Interp* interp);