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

// Plain std::vector registrations for int, double, string
int register_value_vector_int_commands(Tcl_Interp* interp) {
    using IntVec = std::vector<int>;

    Tcl_CreateNamespace(interp, "::std::vector<int>", nullptr, nullptr);
    TCLXX_CMD_NEW0(interp, "::std::vector<int>::new", IntVec);
    TCLXX_CMD_NEW0_SHARED(interp, "::std::vector<int>::new.shared", IntVec);
    TCLXX_CMD_NEW_MAKE_SHARED(interp, "::std::vector<int>::make_shared", IntVec);
    TCLXX_CMD_NEW_FROM_SHARED(interp, "::std::vector<int>::from_shared", IntVec);
    TCLXX_CMD_GETTER(interp, "::std::vector<int>::size", &stdcontainers::value_vector_size<int>);
    TCLXX_CMD_GETTER(interp, "::std::vector<int>::empty", &stdcontainers::value_vector_empty<int>);
    TCLXX_CMD_SETTER(interp, "::std::vector<int>::set", &stdcontainers::value_vector_set<int>);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<int>::at",
        tclxx::cmd::getter<&stdcontainers::value_vector_at<int>>,
        nullptr,
        nullptr);
    TCLXX_CMD_SETTER(interp, "::std::vector<int>::reserve", &stdcontainers::value_vector_reserve<int>);
    TCLXX_CMD_SETTER(interp, "::std::vector<int>::clear", &stdcontainers::value_vector_clear<int>);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<int>::pop",
        tclxx::cmd::setter<&stdcontainers::value_vector_pop<int>>,
        nullptr,
        nullptr);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<int>::push",
        stdcontainers::cmd_helpers::variadic_push_cmd<IntVec, &stdcontainers::value_vector_push<int>>,
        nullptr,
        nullptr);

    return TCL_OK;
}

int register_value_vector_double_commands(Tcl_Interp* interp) {
    using DoubleVec = std::vector<double>;

    Tcl_CreateNamespace(interp, "::std::vector<double>", nullptr, nullptr);
    TCLXX_CMD_NEW0(interp, "::std::vector<double>::new", DoubleVec);
    TCLXX_CMD_NEW0_SHARED(interp, "::std::vector<double>::new.shared", DoubleVec);
    TCLXX_CMD_GETTER(interp, "::std::vector<double>::size", &stdcontainers::value_vector_size<double>);
    TCLXX_CMD_GETTER(interp, "::std::vector<double>::empty", &stdcontainers::value_vector_empty<double>);
    TCLXX_CMD_SETTER(interp, "::std::vector<double>::set", &stdcontainers::value_vector_set<double>);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<double>::at",
        tclxx::cmd::getter<&stdcontainers::value_vector_at<double>>,
        nullptr,
        nullptr);
    TCLXX_CMD_SETTER(interp, "::std::vector<double>::reserve", &stdcontainers::value_vector_reserve<double>);
    TCLXX_CMD_SETTER(interp, "::std::vector<double>::clear", &stdcontainers::value_vector_clear<double>);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<double>::pop",
        tclxx::cmd::setter<&stdcontainers::value_vector_pop<double>>,
        nullptr,
        nullptr);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<double>::push",
        stdcontainers::cmd_helpers::variadic_push_cmd<DoubleVec, &stdcontainers::value_vector_push<double>>,
        nullptr,
        nullptr);

    return TCL_OK;
}

int register_value_vector_string_commands(Tcl_Interp* interp) {
    using StringVec = std::vector<std::string>;

    Tcl_CreateNamespace(interp, "::std::vector<string>", nullptr, nullptr);
    TCLXX_CMD_NEW0(interp, "::std::vector<string>::new", StringVec);
    TCLXX_CMD_NEW0_SHARED(interp, "::std::vector<string>::new.shared", StringVec);
    TCLXX_CMD_GETTER(interp, "::std::vector<string>::size", &stdcontainers::value_vector_size<std::string>);
    TCLXX_CMD_GETTER(interp, "::std::vector<string>::empty", &stdcontainers::value_vector_empty<std::string>);
    TCLXX_CMD_SETTER(interp, "::std::vector<string>::set", &stdcontainers::value_vector_set<std::string>);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<string>::at",
        tclxx::cmd::getter<&stdcontainers::value_vector_at<std::string>>,
        nullptr,
        nullptr);
    TCLXX_CMD_SETTER(interp, "::std::vector<string>::reserve", &stdcontainers::value_vector_reserve<std::string>);
    TCLXX_CMD_SETTER(interp, "::std::vector<string>::clear", &stdcontainers::value_vector_clear<std::string>);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<string>::pop",
        tclxx::cmd::setter<&stdcontainers::value_vector_pop<std::string>>,
        nullptr,
        nullptr);
    Tcl_CreateObjCommand(
        interp,
        "::std::vector<string>::push",
        stdcontainers::cmd_helpers::variadic_push_cmd<StringVec, &stdcontainers::value_vector_push<std::string>>,
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

// std::vector<int> toString
template <>
std::string ObjType<std::vector<int>>::ToString(const std::vector<int>& v) {
    std::ostringstream oss;
    for (int i = 0; i < v.size(); ++i) {
        if (i > 0) oss << " ";
        oss << v[static_cast<std::size_t>(i)];
    }
    return oss.str();
}

// std::vector<double> toString
template <>
std::string ObjType<std::vector<double>>::ToString(const std::vector<double>& v) {
    std::ostringstream oss;
    for (int i = 0; i < v.size(); ++i) {
        if (i > 0) oss << " ";
        oss << v[static_cast<std::size_t>(i)];
    }
    return oss.str();
}

// std::vector<std::string> toString
template <>
std::string ObjType<std::vector<std::string>>::ToString(const std::vector<std::string>& v) {
    std::ostringstream oss;
    for (int i = 0; i < v.size(); ++i) {
        if (i > 0) oss << " ";
        const std::string& str = v[static_cast<std::size_t>(i)];
        bool has_space = (str.find(' ') != std::string::npos);
        if (has_space) {
            oss << '{' << str << '}';
        } else {
            oss << str;
        }
    }
    return oss.str();
}

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

    if (register_value_vector_int_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_value_vector_double_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_value_vector_string_commands(interp) != TCL_OK) {
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
