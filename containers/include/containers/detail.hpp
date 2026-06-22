#pragma once

#include <tcl.h>
#include "tclxx.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>

namespace stdcontainers {

template <class T>
struct TclAlloc {
    using value_type = T;

    TclAlloc() noexcept = default;

    template <class U>
    TclAlloc(const TclAlloc<U>&) noexcept {}

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

    template <class U>
    bool operator==(const TclAlloc<U>&) const noexcept {
        return true;
    }

    template <class U>
    bool operator!=(const TclAlloc<U>&) const noexcept {
        return false;
    }
};

namespace detail {

inline Tcl_Obj* retain(Tcl_Obj* obj) {
    if (obj != nullptr) {
        Tcl_IncrRefCount(obj);
    }
    return obj;
}

inline void release(Tcl_Obj* obj) {
    if (obj != nullptr) {
        Tcl_DecrRefCount(obj);
    }
}

inline std::string key_string(Tcl_Obj* obj) {
    if (!obj) {
        return std::string();
    }
    int len = 0;
    const char* str = Tcl_GetStringFromObj(obj, &len);
    return std::string(str, static_cast<std::size_t>(len));
}

struct TclObjStringHash {
    std::size_t operator()(Tcl_Obj* obj) const {
        int len = 0;
        const char* s = Tcl_GetStringFromObj(obj, &len);
        return std::hash<std::string_view>{}(
            std::string_view(s, static_cast<std::size_t>(len)));
    }
};

struct TclObjStringEqual {
    bool operator()(Tcl_Obj* lhs, Tcl_Obj* rhs) const {
        if (lhs == rhs) {
            return true;
        }
        int llen = 0;
        int rlen = 0;
        const char* l = Tcl_GetStringFromObj(lhs, &llen);
        const char* r = Tcl_GetStringFromObj(rhs, &rlen);
        if (llen != rlen) {
            return false;
        }
        return std::memcmp(l, r, static_cast<std::size_t>(llen)) == 0;
    }
};

struct TclObjStringLess {
    bool operator()(Tcl_Obj* lhs, Tcl_Obj* rhs) const {
        int llen = 0;
        int rlen = 0;
        const char* l = Tcl_GetStringFromObj(lhs, &llen);
        const char* r = Tcl_GetStringFromObj(rhs, &rlen);
        const int min_len = std::min(llen, rlen);
        const int cmp = std::memcmp(l, r, static_cast<std::size_t>(min_len));
        if (cmp < 0) {
            return true;
        }
        if (cmp > 0) {
            return false;
        }
        return llen < rlen;
    }
};

} // namespace detail

} // namespace stdcontainers
