#pragma once

#include "containers/detail.hpp"

#include <set>

namespace stdcontainers {

class SetContainer {
public:
    using storage_type = std::set<Tcl_Obj*, detail::TclObjStringLess, TclAlloc<Tcl_Obj*>>;

    SetContainer() = default;

    SetContainer(const SetContainer& other) {
        for (Tcl_Obj* key : other.data_) {
            data_.insert(detail::retain(key));
        }
    }

    SetContainer& operator=(const SetContainer& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        for (Tcl_Obj* key : other.data_) {
            data_.insert(detail::retain(key));
        }
        return *this;
    }

    SetContainer(SetContainer&&) noexcept = default;
    SetContainer& operator=(SetContainer&&) noexcept = default;

    ~SetContainer() {
        clear();
    }

    int size() const {
        return static_cast<int>(data_.size());
    }

    bool empty() const {
        return data_.empty();
    }

    bool contains(Tcl_Obj* key) const {
        return data_.find(key) != data_.end();
    }

    bool insert_one(Tcl_Obj* key) {
        Tcl_Obj* held = detail::retain(key);
        auto [it, inserted] = data_.insert(held);
        if (!inserted) {
            detail::release(held);
        }
        return inserted;
    }

    int insert_many(int count, Tcl_Obj* const* keys) {
        int inserted_count = 0;
        for (int i = 0; i < count; ++i) {
            if (insert_one(keys[i])) {
                ++inserted_count;
            }
        }
        return inserted_count;
    }

    bool erase_key(Tcl_Obj* key) {
        auto it = data_.find(key);
        if (it == data_.end()) {
            return false;
        }
        detail::release(*it);
        data_.erase(it);
        return true;
    }

    void clear() {
        for (Tcl_Obj* key : data_) {
            detail::release(key);
        }
        data_.clear();
    }

    Tcl_Obj* to_list() const {
        Tcl_Obj* out = Tcl_NewListObj(0, nullptr);
        for (Tcl_Obj* key : data_) {
            Tcl_ListObjAppendElement(nullptr, out, key);
        }
        return out;
    }

private:
    storage_type data_;
};

int InitSetPackage(Tcl_Interp* interp);

} // namespace stdcontainers

extern "C" int Stdcontainersset_Init(Tcl_Interp* interp);
