#pragma once

#include "containers/detail.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

#include "tclxx.hpp"

namespace stdcontainers {

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

/// VectorContainerHeap<T> — same interface as VectorContainer but stores
/// plain value types (int, double, bool, …) using standard heap allocation
/// (no TclAlloc, no retain/release).
template <typename T>
class VectorContainerHeap {
public:
    using storage_type = std::vector<T>;

    VectorContainerHeap() = default;

    VectorContainerHeap(const VectorContainerHeap& other) = default;
    VectorContainerHeap& operator=(const VectorContainerHeap& other) = default;

    VectorContainerHeap(VectorContainerHeap&&) noexcept = default;
    VectorContainerHeap& operator=(VectorContainerHeap&&) noexcept = default;

    ~VectorContainerHeap() = default;

    int size() const {
        return static_cast<int>(data_.size());
    }

    bool empty() const {
        return data_.empty();
    }

    void reserve(int capacity) {
        if (capacity < 0) {
            throw std::runtime_error("vector reserve requires non-negative capacity");
        }
        data_.reserve(static_cast<std::size_t>(capacity));
    }

    void push(const T& value) {
        data_.push_back(value);
    }

    T at(int index) const {
        if (index < 0 || index >= static_cast<int>(data_.size())) {
            throw std::out_of_range("vector index out of range");
        }
        return data_[static_cast<std::size_t>(index)];
    }

    void set_at(int index, T value) {
        if (index < 0 || index >= static_cast<int>(data_.size())) {
            throw std::out_of_range("vector index out of range");
        }
        data_[static_cast<std::size_t>(index)] = value;
    }

    T pop_back_take() {
        if (data_.empty()) {
            throw std::runtime_error("vector is empty");
        }
        T value = data_.back();
        data_.pop_back();
        return value;
    }

    void clear() {
        data_.clear();
    }

private:
    storage_type data_;
};

/// VectorContainerShared — stores Tcl_Obj* using internal representation duplication.
/// Each stored value has only internal rep (no string rep shared).
/// Getting returns Tcl_DuplicateObj() copy to caller.
/// Uses standard allocator (not TclAlloc).
class VectorContainerShared {
public:
    using storage_type = std::vector<Tcl_Obj>;

    VectorContainerShared() = default;

    VectorContainerShared(ClientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) {
        data_.reserve(static_cast<std::size_t>(objc - 1));
        for (int i = 1; i < objc; ++i) {
            push_one(interp, objv[i]);
        }
    }

    VectorContainerShared(const VectorContainerShared& other) {
        data_.reserve(other.data_.size());
        for (const Tcl_Obj& item : other.data_) {
            data_.push_back(clone_heap_value(item));
        }
    }

    VectorContainerShared& operator=(const VectorContainerShared& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        data_.reserve(other.data_.size());
        for (const Tcl_Obj& item : other.data_) {
            data_.push_back(clone_heap_value(item));
        }
        return *this;
    }

    VectorContainerShared(VectorContainerShared&& other) noexcept : data_(std::move(other.data_)) {
        other.data_.clear();
    }

    VectorContainerShared& operator=(VectorContainerShared&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        data_ = std::move(other.data_);
        other.data_.clear();
        return *this;
    }

    ~VectorContainerShared() {
        clear();
    }

    int size() const {
        return static_cast<int>(data_.size());
    }

    bool empty() const {
        return data_.empty();
    }

    void reserve(int capacity) {
        if (capacity < 0) {
            throw std::runtime_error("vector reserve requires non-negative capacity");
        }
        data_.reserve(static_cast<std::size_t>(capacity));
    }

    void push_one(Tcl_Interp* interp, Tcl_Obj* value) {
        data_.push_back(clone_interp_value(interp, value));
    }

    Tcl_Obj* at(int index) const {
        if (index < 0 || index >= static_cast<int>(data_.size())) {
            throw std::out_of_range("vector index out of range");
        }
        Tcl_Obj* heap_obj = const_cast<Tcl_Obj*>(&data_[static_cast<std::size_t>(index)]);
        return Tcl_DuplicateObj(heap_obj);
    }

    void set_at(Tcl_Interp* interp, int index, Tcl_Obj* value) {
        if (index < 0 || index >= static_cast<int>(data_.size())) {
            throw std::out_of_range("vector index out of range");
        }
        const std::size_t pos = static_cast<std::size_t>(index);
        destroy_heap_value(data_[pos]);
        data_[pos] = clone_interp_value(interp, value);
    }

    Tcl_Obj* pop_back_take() {
        if (data_.empty()) {
            throw std::runtime_error("vector is empty");
        }
        Tcl_Obj* heap_obj = &data_.back();
        Tcl_Obj* value = Tcl_DuplicateObj(heap_obj);
        destroy_heap_value(*heap_obj);
        data_.pop_back();
        return value;
    }

    void clear() {
        for (Tcl_Obj& heap_obj : data_) {
            destroy_heap_value(heap_obj);
        }
        data_.clear();
    }

    Tcl_Obj* to_list() const {
        Tcl_Obj* out = Tcl_NewListObj(0, nullptr);
        for (const Tcl_Obj& heap_obj : data_) {
            Tcl_Obj* dup = Tcl_DuplicateObj(const_cast<Tcl_Obj*>(&heap_obj));
            Tcl_ListObjAppendElement(nullptr, out, dup);
        }
        return out;
    }

private:
    storage_type data_;

    static Tcl_Obj clone_interp_value(Tcl_Interp* interp, Tcl_Obj* interp_obj) {
        Tcl_Obj heap_obj{};
        if (!interp_obj || !interp_obj->typePtr || !interp_obj->typePtr->dupIntRepProc) {
            double myDoubleValue;
            if (Tcl_GetDoubleFromObj(interp, interp_obj, &myDoubleValue) != TCL_OK) {
                throw std::runtime_error("Cannot store Tcl_Obj without internal representation which can't be cast into double");
            }
            tclxx::ObjGuard guard(Tcl_NewDoubleObj(myDoubleValue));
            heap_obj = *(guard.get()); // copy string rep if no internal rep
        } else {
            interp_obj->typePtr->dupIntRepProc(interp_obj, &heap_obj);
        }
        heap_obj.refCount = 1;
        // clear string on heap object since it can't be shared
        if (heap_obj.bytes) {
            heap_obj.bytes = nullptr; // prevent freeing string rep in destructor
            heap_obj.length = 0;
        }
        return heap_obj;
    }

    static Tcl_Obj clone_heap_value(const Tcl_Obj& src) {
        Tcl_Obj heap_obj{};
        if (src.typePtr && src.typePtr->dupIntRepProc) {
            src.typePtr->dupIntRepProc(const_cast<Tcl_Obj*>(&src), &heap_obj);
        } else {
            heap_obj = src; // copy string rep if no internal rep
        }
        heap_obj.refCount = 1;
        // clear string on heap object since it can't be shared
        if (heap_obj.bytes) {
            heap_obj.bytes = nullptr; // prevent freeing string rep in destructor
            heap_obj.length = 0;
        }
        return heap_obj;
    }

    static void destroy_heap_value(Tcl_Obj& heap_obj) {
        if (heap_obj.typePtr && heap_obj.typePtr->freeIntRepProc) {
            heap_obj.typePtr->freeIntRepProc(&heap_obj);
        }
    }
};

int InitVectorPackage(Tcl_Interp* interp);

} // namespace stdcontainers

extern "C" int Stdcontainersvector_Init(Tcl_Interp* interp);
