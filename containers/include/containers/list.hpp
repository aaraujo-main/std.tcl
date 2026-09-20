#pragma once

#include "containers/detail.hpp"

#include <list>
#include <stdexcept>

namespace stdcontainers {

using List = std::list<Tcl_Obj*, TclAlloc<Tcl_Obj*>>;

inline int list_size(const List* value) {
    return static_cast<int>(value->size());
}

inline bool list_empty(const List* value) {
    return value->empty();
}

inline void list_push(List* value, Tcl_Obj* item) {
    value->push_back(detail::retain(item));
}

inline Tcl_Obj* list_at(const List* value, int index) {
    if (index < 0 || index >= static_cast<int>(value->size())) {
        throw std::out_of_range("list index out of range");
    }
    auto it = value->begin();
    std::advance(it, index);
    return *it;
}

inline Tcl_Obj* list_pop(List* value) {
    if (value->empty()) {
        throw std::runtime_error("list is empty");
    }
    Tcl_Obj* item = value->back();
    value->pop_back();
    return item;
}

inline void list_clear(List* value) {
    for (Tcl_Obj* item : *value) {
        detail::release(item);
    }
    value->clear();
}

inline Tcl_Obj* list_to_list(const List* value) {
    Tcl_Obj* out = Tcl_NewListObj(0, nullptr);
    for (Tcl_Obj* item : *value) {
        Tcl_ListObjAppendElement(nullptr, out, item);
    }
    return out;
}

int InitListPackage(Tcl_Interp* interp);

} // namespace stdcontainers

extern "C" int Stdcontainerslist_Init(Tcl_Interp* interp);
