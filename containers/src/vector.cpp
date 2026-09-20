#include "containers/vector.hpp"

#include <memory>
#include <sstream>
#include <string>

#include "command_helpers.hpp"
#include "init_common.hpp"
#include "tclxx.hpp"

namespace {

std::string to_tcl_string(Tcl_Obj* obj) {
    Tcl_IncrRefCount(obj);
    int length = 0;
    const char* bytes = Tcl_GetStringFromObj(obj, &length);
    std::string out(bytes, static_cast<std::size_t>(length));
    Tcl_DecrRefCount(obj);
    return out;
}

using Vector = stdcontainers::TclVector;

int vector_new_args_cmd(ClientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) {
    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "?arg1 arg2 ...?");
        return TCL_ERROR;
    }
    auto value = std::make_unique<Vector>();
    try {
        value->reserve(static_cast<std::size_t>(objc - 1));
        for (int i = 1; i < objc; ++i) {
            value->push_back(objv[i]);
        }
        Tcl_SetObjResult(interp, tclxx::obj_cast::from_owned(value.release()));
        return TCL_OK;
    } catch (const std::exception& e) {
        value->clear();
        Tcl_SetObjResult(interp, Tcl_NewStringObj(e.what(), -1));
        return TCL_ERROR;
    }
}

int register_vector_commands(Tcl_Interp* interp) {
    Tcl_CreateNamespace(interp, "::std::vector", nullptr, nullptr);
    TCLXX_CMD_NEW0(interp, "::std::vector::new", Vector);
    Tcl_CreateObjCommand(interp, "::std::vector::new(args)", vector_new_args_cmd, nullptr, nullptr);
    TCLXX_CMD_GETTER(interp, "::std::vector::size", &stdcontainers::vector_size);
    TCLXX_CMD_GETTER(interp, "::std::vector::empty", &stdcontainers::vector_empty);
    TCLXX_CMD_SETTER(interp, "::std::vector::set", &stdcontainers::vector_set);
    TCLXX_CMD_GETTER(interp, "::std::vector::at", &stdcontainers::vector_at);
    TCLXX_CMD_GETTER(interp, "::std::vector::list", &stdcontainers::vector_to_list);
    TCLXX_CMD_SETTER(interp, "::std::vector::reserve", &stdcontainers::vector_reserve);
    TCLXX_CMD_SETTER(interp, "::std::vector::clear", &stdcontainers::vector_clear);
    TCLXX_CMD_SETTER(interp, "::std::vector::pop", &stdcontainers::vector_pop);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector::push",
        stdcontainers::cmd_helpers::variadic_push_cmd<Vector, &stdcontainers::vector_push>,
        nullptr,
        nullptr);

    return TCL_OK;
}

// VectorContainerHeap registrations for int, double, string
int register_vector_heap_int_commands(Tcl_Interp* interp) {
    using HeapIntVec = stdcontainers::VectorContainerHeap<int>;

    Tcl_CreateNamespace(interp, "::std::vector<int>", nullptr, nullptr);
    TCLXX_CMD_NEW0(interp, "::std::vector<int>::new", HeapIntVec);
    TCLXX_CMD_NEW0_SHARED(interp, "::std::vector<int>::new.shared", HeapIntVec);
    TCLXX_CMD_NEW_MAKE_SHARED(interp, "::std::vector<int>::make_shared", HeapIntVec);
    TCLXX_CMD_NEW_FROM_SHARED(interp, "::std::vector<int>::from_shared", HeapIntVec);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::vector<int>::size", &HeapIntVec::size);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::vector<int>::empty", &HeapIntVec::empty);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::vector<int>::set", &HeapIntVec::set_at);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<int>::at",
        stdcontainers::cmd_helpers::getter_obj_int_cmd<HeapIntVec, &HeapIntVec::at>,
        nullptr,
        nullptr);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::vector<int>::reserve", &HeapIntVec::reserve);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::vector<int>::clear", &HeapIntVec::clear);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<int>::pop",
        stdcontainers::cmd_helpers::setter_obj0_cmd<HeapIntVec, &HeapIntVec::pop_back_take>,
        nullptr,
        nullptr);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<int>::push",
        stdcontainers::cmd_helpers::variadic_push_cmd<HeapIntVec, &HeapIntVec::push>,
        nullptr,
        nullptr);

    return TCL_OK;
}

int register_vector_heap_double_commands(Tcl_Interp* interp) {
    using HeapDoubleVec = stdcontainers::VectorContainerHeap<double>;

    Tcl_CreateNamespace(interp, "::std::vector<double>", nullptr, nullptr);
    TCLXX_CMD_NEW0(interp, "::std::vector<double>::new", HeapDoubleVec);
    TCLXX_CMD_NEW0_SHARED(interp, "::std::vector<double>::new.shared", HeapDoubleVec);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::vector<double>::size", &HeapDoubleVec::size);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::vector<double>::empty", &HeapDoubleVec::empty);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::vector<double>::set", &HeapDoubleVec::set_at);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<double>::at",
        stdcontainers::cmd_helpers::getter_obj_int_cmd<HeapDoubleVec, &HeapDoubleVec::at>,
        nullptr,
        nullptr);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::vector<double>::reserve", &HeapDoubleVec::reserve);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::vector<double>::clear", &HeapDoubleVec::clear);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<double>::pop",
        stdcontainers::cmd_helpers::setter_obj0_cmd<HeapDoubleVec, &HeapDoubleVec::pop_back_take>,
        nullptr,
        nullptr);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<double>::push",
        stdcontainers::cmd_helpers::variadic_push_cmd<HeapDoubleVec, &HeapDoubleVec::push>,
        nullptr,
        nullptr);

    return TCL_OK;
}

int register_vector_heap_string_commands(Tcl_Interp* interp) {
    using HeapStringVec = stdcontainers::VectorContainerHeap<std::string>;

    Tcl_CreateNamespace(interp, "::std::vector<string>", nullptr, nullptr);
    TCLXX_CMD_NEW0(interp, "::std::vector<string>::new", HeapStringVec);
    TCLXX_CMD_NEW0_SHARED(interp, "::std::vector<string>::new.shared", HeapStringVec);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::vector<string>::size", &HeapStringVec::size);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::vector<string>::empty", &HeapStringVec::empty);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::vector<string>::set", &HeapStringVec::set_at);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<string>::at",
        stdcontainers::cmd_helpers::getter_obj_int_cmd<HeapStringVec, &HeapStringVec::at>,
        nullptr,
        nullptr);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::vector<string>::reserve", &HeapStringVec::reserve);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::vector<string>::clear", &HeapStringVec::clear);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<string>::pop",
        stdcontainers::cmd_helpers::setter_obj0_cmd<HeapStringVec, &HeapStringVec::pop_back_take>,
        nullptr,
        nullptr);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<string>::push",
        stdcontainers::cmd_helpers::variadic_push_cmd<HeapStringVec, &HeapStringVec::push>,
        nullptr,
        nullptr);

    return TCL_OK;
}

int register_vector_shared_commands(Tcl_Interp* interp) {
    using SharedVec = stdcontainers::VectorContainerShared;

    Tcl_CreateNamespace(interp, "::std::vector<shared>", nullptr, nullptr);
    TCLXX_CMD_NEW0(interp, "::std::vector<shared>::new", SharedVec);
    TCLXX_CMD_NEW0_SHARED(interp, "::std::vector<shared>::new.shared", SharedVec);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::vector<shared>::size", &SharedVec::size);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::vector<shared>::empty", &SharedVec::empty);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::vector<shared>::set", &SharedVec::set_at);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<shared>::at",
        stdcontainers::cmd_helpers::getter_obj_int_cmd<
            SharedVec,
            &SharedVec::at>,
        nullptr,
        nullptr);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<shared>::list",
        stdcontainers::cmd_helpers::getter_obj0_cmd<
            SharedVec,
            &SharedVec::to_list>,
        nullptr,
        nullptr);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::vector<shared>::reserve", &SharedVec::reserve);
    TCLXX_CMD_SETTER_METHOD(interp, "::std::vector<shared>::clear", &SharedVec::clear);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<shared>::pop",
        stdcontainers::cmd_helpers::setter_obj0_cmd<
            SharedVec,
            &SharedVec::pop_back_take>,
        nullptr,
        nullptr);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<shared>::push",
        stdcontainers::cmd_helpers::variadic_push_cmd<SharedVec, &SharedVec::push_one>,
        nullptr,
        nullptr);

    return TCL_OK;
}

} // namespace

namespace tclxx {

template <>
std::string ObjType<stdcontainers::TclVector>::ToString(const stdcontainers::TclVector& v) {
    return to_tcl_string(stdcontainers::vector_to_list(&v));
}

template <>
void ObjType<stdcontainers::TclVector>::Startup(stdcontainers::TclVector* value) noexcept {
    if (!value) {
        return;
    }
    for (Tcl_Obj* item : *value) {
        stdcontainers::detail::retain(item);
    }
}

template <>
void ObjType<stdcontainers::TclVector>::Cleanup(stdcontainers::TclVector* value) noexcept {
    if (value) {
        stdcontainers::vector_clear(value);
    }
}

// VectorContainerHeap<int> toString
template <>
std::string ObjType<stdcontainers::VectorContainerHeap<int>>::ToString(const stdcontainers::VectorContainerHeap<int>& v) {
    std::ostringstream oss;
    for (int i = 0; i < v.size(); ++i) {
        if (i > 0) oss << " ";
        oss << v.at(i);
    }
    return oss.str();
}

template <>
void ObjType<stdcontainers::VectorContainerHeap<int>>::Startup(stdcontainers::VectorContainerHeap<int>*) noexcept {}

template <>
void ObjType<stdcontainers::VectorContainerHeap<int>>::Cleanup(stdcontainers::VectorContainerHeap<int>*) noexcept {}

// VectorContainerHeap<double> toString
template <>
std::string ObjType<stdcontainers::VectorContainerHeap<double>>::ToString(const stdcontainers::VectorContainerHeap<double>& v) {
    std::ostringstream oss;
    for (int i = 0; i < v.size(); ++i) {
        if (i > 0) oss << " ";
        oss << v.at(i);
    }
    return oss.str();
}

template <>
void ObjType<stdcontainers::VectorContainerHeap<double>>::Startup(stdcontainers::VectorContainerHeap<double>*) noexcept {}

template <>
void ObjType<stdcontainers::VectorContainerHeap<double>>::Cleanup(stdcontainers::VectorContainerHeap<double>*) noexcept {}

// VectorContainerHeap<std::string> toString
template <>
std::string ObjType<stdcontainers::VectorContainerHeap<std::string>>::ToString(const stdcontainers::VectorContainerHeap<std::string>& v) {
    std::ostringstream oss;
    for (int i = 0; i < v.size(); ++i) {
        if (i > 0) oss << " ";
        std::string str = v.at(i);
        bool has_space = (str.find(' ') != std::string::npos);
        if (has_space) {
            oss << '{' << str << '}';
        } else {
            oss << str;
        }
    }
    return oss.str();
}

template <>
void ObjType<stdcontainers::VectorContainerHeap<std::string>>::Startup(stdcontainers::VectorContainerHeap<std::string>*) noexcept {}

template <>
void ObjType<stdcontainers::VectorContainerHeap<std::string>>::Cleanup(stdcontainers::VectorContainerHeap<std::string>*) noexcept {}

// VectorContainerShared toString
template <>
std::string ObjType<stdcontainers::VectorContainerShared>::ToString(const stdcontainers::VectorContainerShared& v) {
    return to_tcl_string(v.to_list());
}

template <>
void ObjType<stdcontainers::VectorContainerShared>::Startup(stdcontainers::VectorContainerShared*) noexcept {}

template <>
void ObjType<stdcontainers::VectorContainerShared>::Cleanup(stdcontainers::VectorContainerShared*) noexcept {}

} // namespace tclxx

namespace stdcontainers {

int InitVectorPackage(Tcl_Interp* interp) {
    if (!interp) {
        return TCL_ERROR;
    }

    if (init_common::ensure_base_namespaces(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_vector_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_vector_heap_int_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_vector_heap_double_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_vector_heap_string_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_vector_shared_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    return TCL_OK;
}

} // namespace stdcontainers

extern "C" int Stdcontainersvector_Init(Tcl_Interp* interp) {
    if (stdcontainers::InitVectorPackage(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    return stdcontainers::init_common::provide_package(interp);
}
