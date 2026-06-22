#pragma once

#include "containers/detail.hpp"

#include <list>
#include <stdexcept>

namespace stdcontainers {

class ListContainer {
public:
    using storage_type = std::list<Tcl_Obj*, TclAlloc<Tcl_Obj*>>;

    ListContainer() = default;

    ListContainer(const ListContainer& other) {
        for (Tcl_Obj* item : other.data_) {
            data_.push_back(detail::retain(item));
        }
    }

    ListContainer& operator=(const ListContainer& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        for (Tcl_Obj* item : other.data_) {
            data_.push_back(detail::retain(item));
        }
        return *this;
    }

    ListContainer(ListContainer&&) noexcept = default;
    ListContainer& operator=(ListContainer&&) noexcept = default;

    ~ListContainer() {
        clear();
    }

    int size() const {
        return static_cast<int>(data_.size());
    }

    bool empty() const {
        return data_.empty();
    }

    void push_back_one(Tcl_Obj* value) {
        data_.push_back(detail::retain(value));
    }

    Tcl_Obj* at(int index) const {
        if (index < 0 || index >= static_cast<int>(data_.size())) {
            throw std::out_of_range("list index out of range");
        }
        auto it = data_.begin();
        std::advance(it, index);
        return *it;
    }

    Tcl_Obj* pop_back_take() {
        if (data_.empty()) {
            throw std::runtime_error("list is empty");
        }
        Tcl_Obj* value = data_.back();
        data_.pop_back();
        return value;
    }

    void clear() {
        for (Tcl_Obj* item : data_) {
            detail::release(item);
        }
        data_.clear();
    }

    Tcl_Obj* to_list() const {
        Tcl_Obj* out = Tcl_NewListObj(0, nullptr);
        for (Tcl_Obj* item : data_) {
            Tcl_ListObjAppendElement(nullptr, out, item);
        }
        return out;
    }

private:
    storage_type data_;
};

int InitListPackage(Tcl_Interp* interp);

} // namespace stdcontainers

extern "C" int Stdcontainerslist_Init(Tcl_Interp* interp);
