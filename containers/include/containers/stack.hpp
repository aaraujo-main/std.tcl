#pragma once

#include "containers/detail.hpp"

#include <stdexcept>
#include <vector>

namespace stdcontainers {

class StackContainer {
public:
    using storage_type = std::vector<Tcl_Obj*, TclAlloc<Tcl_Obj*>>;

    StackContainer() = default;

    StackContainer(const StackContainer& other) {
        data_.reserve(other.data_.size());
        for (Tcl_Obj* item : other.data_) {
            data_.push_back(detail::retain(item));
        }
    }

    StackContainer& operator=(const StackContainer& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        data_.reserve(other.data_.size());
        for (Tcl_Obj* item : other.data_) {
            data_.push_back(detail::retain(item));
        }
        return *this;
    }

    StackContainer(StackContainer&&) noexcept = default;
    StackContainer& operator=(StackContainer&&) noexcept = default;

    ~StackContainer() {
        clear();
    }

    int size() const {
        return static_cast<int>(data_.size());
    }

    bool empty() const {
        return data_.empty();
    }

    void push_one(Tcl_Obj* value) {
        data_.push_back(detail::retain(value));
    }

    Tcl_Obj* top() const {
        if (data_.empty()) {
            throw std::runtime_error("stack is empty");
        }
        return data_.back();
    }

    Tcl_Obj* pop_take() {
        if (data_.empty()) {
            throw std::runtime_error("stack is empty");
        }
        Tcl_Obj* value = data_.back();
        data_.pop_back();
        return value;
    }

    Tcl_Obj* to_list() const {
        Tcl_Obj* out = Tcl_NewListObj(0, nullptr);
        for (Tcl_Obj* item : data_) {
            Tcl_ListObjAppendElement(nullptr, out, item);
        }
        return out;
    }

    void clear() {
        for (Tcl_Obj* item : data_) {
            detail::release(item);
        }
        data_.clear();
    }

private:
    storage_type data_;
};

int InitStackPackage(Tcl_Interp* interp);

} // namespace stdcontainers

extern "C" int Stdcontainersstack_Init(Tcl_Interp* interp);