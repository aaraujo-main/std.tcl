#pragma once

#include "containers/detail.hpp"

#include <unordered_set>

namespace stdcontainers {

using UnorderedSet = std::unordered_set<
    Tcl_Obj*,
    detail::TclObjStringHash,
    detail::TclObjStringEqual,
    TclAlloc<Tcl_Obj*>>;

inline int unordered_set_size(const UnorderedSet* value) {
    return static_cast<int>(value->size());
}

inline bool unordered_set_empty(const UnorderedSet* value) {
    return value->empty();
}

inline bool unordered_set_contains(const UnorderedSet* value, Tcl_Obj* key) {
    return value->find(key) != value->end();
}

inline bool unordered_set_insert(UnorderedSet* value, Tcl_Obj* key) {
    Tcl_Obj* held = detail::retain(key);
    auto [it, inserted] = value->insert(held);
    if (!inserted) {
        detail::release(held);
    }
    return inserted;
}

inline bool unordered_set_erase(UnorderedSet* value, Tcl_Obj* key) {
    auto it = value->find(key);
    if (it == value->end()) {
        return false;
    }
    detail::release(*it);
    value->erase(it);
    return true;
}

inline void unordered_set_clear(UnorderedSet* value) {
    for (Tcl_Obj* key : *value) {
        detail::release(key);
    }
    value->clear();
}

inline Tcl_Obj* unordered_set_to_list(const UnorderedSet* value) {
    Tcl_Obj* out = Tcl_NewListObj(0, nullptr);
    for (Tcl_Obj* key : *value) {
        Tcl_ListObjAppendElement(nullptr, out, key);
    }
    return out;
}

int InitUnorderedSetPackage(Tcl_Interp* interp);

} // namespace stdcontainers

extern "C" int Stdcontainersunordered_set_Init(Tcl_Interp* interp);