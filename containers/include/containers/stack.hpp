#pragma once

#include "containers/detail.hpp"

#include <stdexcept>
#include <vector>

namespace stdcontainers {

template <typename T>
struct StackAlloc {
    using value_type = T;

    StackAlloc() noexcept = default;

    template <typename U>
    StackAlloc(const StackAlloc<U>&) noexcept {}

    T* allocate(std::size_t n) {
        if (n == 0) {
            return nullptr;
        }
        void* p = Tcl_Alloc(static_cast<unsigned int>(n * sizeof(T)));
        if (!p) {
            throw std::bad_alloc();
        }
        return static_cast<T*>(p);
    }

    void deallocate(T* p, std::size_t) noexcept {
        Tcl_Free(reinterpret_cast<char*>(p));
    }

    template <typename U>
    bool operator==(const StackAlloc<U>&) const noexcept {
        return true;
    }

    template <typename U>
    bool operator!=(const StackAlloc<U>&) const noexcept {
        return false;
    }
};

using Stack = std::vector<Tcl_Obj*, StackAlloc<Tcl_Obj*>>;

inline int stack_size(const Stack* value) {
    return static_cast<int>(value->size());
}

inline bool stack_empty(const Stack* value) {
    return value->empty();
}

inline void stack_push(Stack* value, Tcl_Obj* item) {
    value->push_back(detail::retain(item));
}

inline Tcl_Obj* stack_top(const Stack* value) {
    if (value->empty()) {
        throw std::runtime_error("stack is empty");
    }
    return value->back();
}

inline Tcl_Obj* stack_pop(Stack* value) {
    if (value->empty()) {
        throw std::runtime_error("stack is empty");
    }
    Tcl_Obj* item = value->back();
    value->pop_back();
    return item;
}

inline Tcl_Obj* stack_to_list(const Stack* value) {
    Tcl_Obj* out = Tcl_NewListObj(0, nullptr);
    for (Tcl_Obj* item : *value) {
        Tcl_ListObjAppendElement(nullptr, out, item);
    }
    return out;
}

inline void stack_clear(Stack* value) {
    for (Tcl_Obj* item : *value) {
        detail::release(item);
    }
    value->clear();
}

int InitStackPackage(Tcl_Interp* interp);

} // namespace stdcontainers

extern "C" int Stdcontainersstack_Init(Tcl_Interp* interp);