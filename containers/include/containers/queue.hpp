#pragma once

#include "containers/detail.hpp"

#include <deque>
#include <queue>
#include <stdexcept>

namespace stdcontainers {

using QueueStorage = std::deque<Tcl_Obj*, TclAlloc<Tcl_Obj*>>;
using Queue = std::queue<Tcl_Obj*, QueueStorage>;

inline int queue_size(const Queue* value) {
    return static_cast<int>(value->size());
}

inline bool queue_empty(const Queue* value) {
    return value->empty();
}

inline void queue_push(Queue* value, Tcl_Obj* item) {
    value->push(detail::retain(item));
}

inline Tcl_Obj* queue_front(const Queue* value) {
    if (value->empty()) {
        throw std::runtime_error("queue is empty");
    }
    return value->front();
}

inline Tcl_Obj* queue_pop(Queue* value) {
    if (value->empty()) {
        throw std::runtime_error("queue is empty");
    }
    Tcl_Obj* item = value->front();
    value->pop();
    return item;
}

inline Tcl_Obj* queue_to_list(const Queue* value) {
    Tcl_Obj* out = Tcl_NewListObj(0, nullptr);
    Queue copy = *value;
    while (!copy.empty()) {
        Tcl_ListObjAppendElement(nullptr, out, copy.front());
        copy.pop();
    }
    return out;
}

inline void queue_clear(Queue* value) {
    while (!value->empty()) {
        detail::release(value->front());
        value->pop();
    }
}

int InitQueuePackage(Tcl_Interp* interp);

} // namespace stdcontainers

extern "C" int Stdcontainersqueue_Init(Tcl_Interp* interp);