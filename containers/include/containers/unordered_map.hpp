#pragma once

#include "containers/detail.hpp"

#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace stdcontainers {

using UnorderedMap = std::unordered_map<
    Tcl_Obj*,
    Tcl_Obj*,
    detail::TclObjStringHash,
    detail::TclObjStringEqual,
    TclAlloc<std::pair<Tcl_Obj* const, Tcl_Obj*>>>;

inline int unordered_map_size(const UnorderedMap* value) {
    return static_cast<int>(value->size());
}

inline bool unordered_map_empty(const UnorderedMap* value) {
    return value->empty();
}

inline void unordered_map_reserve(UnorderedMap* value, int capacity) {
    if (capacity < 0) {
        throw std::runtime_error("unordered_map reserve requires non-negative capacity");
    }
    value->reserve(static_cast<std::size_t>(capacity));
}

inline bool unordered_map_exists(const UnorderedMap* value, Tcl_Obj* key) {
    return value->find(key) != value->end();
}

inline Tcl_Obj* unordered_map_get(const UnorderedMap* value, Tcl_Obj* key) {
    auto it = value->find(key);
    if (it == value->end()) {
        throw std::runtime_error("unordered_map key not found");
    }
    return it->second;
}

inline void unordered_map_put(UnorderedMap* value, Tcl_Obj* key, Tcl_Obj* item) {
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

inline bool unordered_map_erase(UnorderedMap* value, Tcl_Obj* key) {
    auto it = value->find(key);
    if (it == value->end()) {
        return false;
    }
    detail::release(it->first);
    detail::release(it->second);
    value->erase(it);
    return true;
}

inline void unordered_map_clear(UnorderedMap* value) {
    for (auto& kv : *value) {
        detail::release(kv.first);
        detail::release(kv.second);
    }
    value->clear();
}

inline Tcl_Obj* unordered_map_keys(const UnorderedMap* value) {
    Tcl_Obj* out = Tcl_NewListObj(0, nullptr);
    for (const auto& kv : *value) {
        Tcl_ListObjAppendElement(nullptr, out, kv.first);
    }
    return out;
}

inline Tcl_Obj* unordered_map_to_dict(const UnorderedMap* value) {
    Tcl_Obj* out = Tcl_NewDictObj();
    for (const auto& kv : *value) {
        if (Tcl_DictObjPut(nullptr, out, kv.first, kv.second) != TCL_OK) {
            throw std::runtime_error("unordered_map to_dict failed");
        }
    }
    return out;
}

int InitUnorderedMapPackage(Tcl_Interp* interp);

} // namespace stdcontainers

extern "C" int Stdcontainersunordered_map_Init(Tcl_Interp* interp);
