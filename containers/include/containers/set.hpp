#pragma once

#include "containers/detail.hpp"

#include <set>

namespace stdcontainers {

using Set = std::set<Tcl_Obj*, detail::TclObjStringLess, TclAlloc<Tcl_Obj*>>;

inline int set_size(const Set* value) {
    return static_cast<int>(value->size());
}

inline bool set_empty(const Set* value) {
    return value->empty();
}

inline bool set_contains(const Set* value, Tcl_Obj* key) {
    return value->find(key) != value->end();
}

inline bool set_insert(Set* value, Tcl_Obj* key) {
    Tcl_Obj* held = detail::retain(key);
    auto [it, inserted] = value->insert(held);
    if (!inserted) {
        detail::release(held);
    }
    return inserted;
}

inline bool set_erase(Set* value, Tcl_Obj* key) {
    auto it = value->find(key);
    if (it == value->end()) {
        return false;
    }
    detail::release(*it);
    value->erase(it);
    return true;
}

inline void set_clear(Set* value) {
    for (Tcl_Obj* key : *value) {
        detail::release(key);
    }
    value->clear();
}

inline Tcl_Obj* set_to_list(const Set* value) {
    Tcl_Obj* out = Tcl_NewListObj(0, nullptr);
    for (Tcl_Obj* key : *value) {
        Tcl_ListObjAppendElement(nullptr, out, key);
    }
    return out;
}

int InitSetPackage(Tcl_Interp* interp);

} // namespace stdcontainers

extern "C" int Stdcontainersset_Init(Tcl_Interp* interp);
