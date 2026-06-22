#pragma once

#include "containers/detail.hpp"

#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace stdcontainers {

class UnorderedMapContainer {
public:
    using value_type = std::pair<Tcl_Obj* const, Tcl_Obj*>;
    using storage_type = std::unordered_map<
        Tcl_Obj*,
        Tcl_Obj*,
        detail::TclObjStringHash,
        detail::TclObjStringEqual,
        TclAlloc<value_type>>;

    UnorderedMapContainer() = default;

    UnorderedMapContainer(const UnorderedMapContainer& other) {
        for (const auto& kv : other.data_) {
            data_.emplace(detail::retain(kv.first), detail::retain(kv.second));
        }
    }

    UnorderedMapContainer& operator=(const UnorderedMapContainer& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        for (const auto& kv : other.data_) {
            data_.emplace(detail::retain(kv.first), detail::retain(kv.second));
        }
        return *this;
    }

    UnorderedMapContainer(UnorderedMapContainer&&) noexcept = default;
    UnorderedMapContainer& operator=(UnorderedMapContainer&&) noexcept = default;

    ~UnorderedMapContainer() {
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
            throw std::runtime_error("unordered_map reserve requires non-negative capacity");
        }
        data_.reserve(static_cast<std::size_t>(capacity));
    }

    bool exists(Tcl_Obj* key) const {
        return data_.find(key) != data_.end();
    }

    Tcl_Obj* get(Tcl_Obj* key) const {
        auto it = data_.find(key);
        if (it == data_.end()) {
            throw std::runtime_error("unordered_map key not found");
        }
        return it->second;
    }

    void put(Tcl_Obj* key, Tcl_Obj* value) {
        Tcl_Obj* held_key = detail::retain(key);
        Tcl_Obj* held_value = detail::retain(value);
        auto [it, inserted] = data_.try_emplace(held_key, held_value);
        if (inserted) {
            return;
        }

        detail::release(held_key);
        detail::release(it->second);
        it->second = held_value;
    }

    bool erase_key(Tcl_Obj* key) {
        auto it = data_.find(key);
        if (it == data_.end()) {
            return false;
        }
        detail::release(it->first);
        detail::release(it->second);
        data_.erase(it);
        return true;
    }

    void clear() {
        for (auto& kv : data_) {
            detail::release(kv.first);
            detail::release(kv.second);
        }
        data_.clear();
    }

    Tcl_Obj* keys() const {
        Tcl_Obj* out = Tcl_NewListObj(0, nullptr);
        for (const auto& kv : data_) {
            Tcl_ListObjAppendElement(nullptr, out, kv.first);
        }
        return out;
    }

    Tcl_Obj* to_dict() const {
        Tcl_Obj* out = Tcl_NewDictObj();
        for (const auto& kv : data_) {
            if (Tcl_DictObjPut(nullptr, out, kv.first, kv.second) != TCL_OK) {
                throw std::runtime_error("unordered_map to_dict failed");
            }
        }
        return out;
    }

private:
    storage_type data_;
};

int InitUnorderedMapPackage(Tcl_Interp* interp);

} // namespace stdcontainers

extern "C" int Stdcontainersunordered_map_Init(Tcl_Interp* interp);
