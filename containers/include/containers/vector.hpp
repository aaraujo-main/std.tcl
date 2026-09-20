#pragma once

#include "containers/detail.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

#include "tclxx.hpp"

namespace stdcontainers {
// ********************************************
// TclVector: retained Tcl_Obj* storage.
// ********************************************
using TclVector = std::vector<Tcl_Obj*, TclAlloc<Tcl_Obj*>>;

inline int vector_size(const TclVector* value) {
    return static_cast<int>(value->size());
}

inline bool vector_empty(const TclVector* value) {
    return value->empty();
}

inline void vector_reserve(TclVector* value, int capacity) {
    if (capacity < 0) {
        throw std::runtime_error("vector reserve requires non-negative capacity");
    }
    value->reserve(static_cast<std::size_t>(capacity));
}

inline void vector_push(TclVector* value, Tcl_Obj* item) {
    // Vector retains item; vector_clear releases each stored object.
    value->push_back(detail::retain(item));
}

inline Tcl_Obj* vector_at(const TclVector* value, int index) {
    if (index < 0 || index >= static_cast<int>(value->size())) {
        throw std::out_of_range("vector index out of range");
    }
    return (*value)[static_cast<std::size_t>(index)];
}

inline void vector_set(TclVector* value, int index, Tcl_Obj* item) {
    if (index < 0 || index >= static_cast<int>(value->size())) {
        throw std::out_of_range("vector index out of range");
    }
    const std::size_t position = static_cast<std::size_t>(index);
    Tcl_Obj* replacement = detail::retain(item);
    Tcl_Obj* old = (*value)[position];
    (*value)[position] = replacement;
    detail::release(old);
}

inline Tcl_Obj* vector_pop(TclVector* value) {
    if (value->empty()) {
        throw std::runtime_error("vector is empty");
    }
    Tcl_Obj* item = value->back();
    value->pop_back();
    return item;
}

inline void vector_clear(TclVector* value) {
    for (Tcl_Obj* item : *value) {
        detail::release(item);
    }
    value->clear();
}

inline Tcl_Obj* vector_to_list(const TclVector* value) {
    Tcl_Obj* out = Tcl_NewListObj(0, nullptr);
    for (Tcl_Obj* item : *value) {
        Tcl_ListObjAppendElement(nullptr, out, item);
    }
    return out;
}

// ********************************************
// std::vector<T>: ordinary C++ value storage.
// ********************************************
template <typename T>
int value_vector_size(const std::vector<T>* value) {
    return static_cast<int>(value->size());
}

template <typename T>
bool value_vector_empty(const std::vector<T>* value) {
    return value->empty();
}

template <typename T>
void value_vector_reserve(std::vector<T>* value, int capacity) {
    if (capacity < 0) {
        throw std::runtime_error("vector reserve requires non-negative capacity");
    }
    value->reserve(static_cast<std::size_t>(capacity));
}

template <typename T>
void value_vector_push(std::vector<T>* value, T item) {
    value->push_back(std::move(item));
}

template <typename T>
T value_vector_at(const std::vector<T>* value, int index) {
    if (index < 0 || index >= static_cast<int>(value->size())) {
        throw std::out_of_range("vector index out of range");
    }
    return (*value)[static_cast<std::size_t>(index)];
}

template <typename T>
void value_vector_set(std::vector<T>* value, int index, T item) {
    if (index < 0 || index >= static_cast<int>(value->size())) {
        throw std::out_of_range("vector index out of range");
    }
    (*value)[static_cast<std::size_t>(index)] = std::move(item);
}

template <typename T>
T value_vector_pop(std::vector<T>* value) {
    if (value->empty()) {
        throw std::runtime_error("vector is empty");
    }
    T item = std::move(value->back());
    value->pop_back();
    return item;
}

template <typename T>
void value_vector_clear(std::vector<T>* value) {
    value->clear();
}

// ********************************************
// SharedVector: deep-copied Tcl_Obj storage.
// ********************************************
using SharedVector = std::vector<Tcl_Obj>;

// Internal helpers for SharedVector ownership.
inline Tcl_Obj shared_vector_clone(Tcl_Interp* interp, Tcl_Obj* source) {
    Tcl_Obj value{};
    if (!source || !source->typePtr || !source->typePtr->dupIntRepProc) {
        double number = 0.0;
        if (Tcl_GetDoubleFromObj(interp, source, &number) != TCL_OK) {
            throw std::runtime_error(
                "Cannot store Tcl_Obj without internal representation which can't be cast into double");
        }
        tclxx::ObjGuard guard(Tcl_NewDoubleObj(number));
        value = *guard.get();
    } else {
        source->typePtr->dupIntRepProc(source, &value);
    }
    value.refCount = 1;
    value.bytes = nullptr;
    value.length = 0;
    return value;
}

inline void shared_vector_destroy(Tcl_Obj& value) {
    if (value.typePtr && value.typePtr->freeIntRepProc) {
        value.typePtr->freeIntRepProc(&value);
    }
}

// SharedVector operations.
inline int shared_vector_size(const SharedVector* value) {
    return static_cast<int>(value->size());
}

inline bool shared_vector_empty(const SharedVector* value) {
    return value->empty();
}

inline void shared_vector_reserve(SharedVector* value, int capacity) {
    if (capacity < 0) {
        throw std::runtime_error("vector reserve requires non-negative capacity");
    }
    value->reserve(static_cast<std::size_t>(capacity));
}

inline void shared_vector_push(SharedVector* value, Tcl_Obj* item) {
    // Vector stores an independent Tcl_Obj internal representation.
    value->push_back(shared_vector_clone(nullptr, item));
}

inline Tcl_Obj* shared_vector_at(const SharedVector* value, int index) {
    if (index < 0 || index >= static_cast<int>(value->size())) {
        throw std::out_of_range("vector index out of range");
    }
    return Tcl_DuplicateObj(const_cast<Tcl_Obj*>(&(*value)[static_cast<std::size_t>(index)]));
}

inline void shared_vector_set(SharedVector* value, int index, Tcl_Obj* item) {
    if (index < 0 || index >= static_cast<int>(value->size())) {
        throw std::out_of_range("vector index out of range");
    }
    const std::size_t position = static_cast<std::size_t>(index);
    Tcl_Obj replacement = shared_vector_clone(nullptr, item);
    shared_vector_destroy((*value)[position]);
    (*value)[position] = replacement;
}

inline Tcl_Obj* shared_vector_pop(SharedVector* value) {
    if (value->empty()) {
        throw std::runtime_error("vector is empty");
    }
    Tcl_Obj& item = value->back();
    Tcl_Obj* result = Tcl_DuplicateObj(&item);
    shared_vector_destroy(item);
    value->pop_back();
    return result;
}

inline void shared_vector_clear(SharedVector* value) {
    for (Tcl_Obj& item : *value) {
        shared_vector_destroy(item);
    }
    value->clear();
}

inline Tcl_Obj* shared_vector_to_list(const SharedVector* value) {
    Tcl_Obj* out = Tcl_NewListObj(0, nullptr);
    for (const Tcl_Obj& item : *value) {
        Tcl_ListObjAppendElement(nullptr, out, Tcl_DuplicateObj(const_cast<Tcl_Obj*>(&item)));
    }
    return out;
}

int InitVectorPackage(Tcl_Interp* interp);

} // namespace stdcontainers

extern "C" int Stdcontainersvector_Init(Tcl_Interp* interp);
