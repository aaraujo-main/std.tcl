#pragma once

#include <tcl.h>

#include <stdexcept>

#include "tclxx.hpp"

namespace stdcontainers::cmd_helpers {

inline Tcl_Obj* resolve_mutable_var(Tcl_Interp* interp, Tcl_Obj* name_obj) {
    Tcl_Obj* obj = Tcl_ObjGetVar2(interp, name_obj, nullptr, TCL_LEAVE_ERR_MSG);
    if (!obj) {
        return nullptr;
    }
    if (Tcl_IsShared(obj)) {
        obj = Tcl_DuplicateObj(obj);
        if (!Tcl_ObjSetVar2(interp, name_obj, nullptr, obj, TCL_LEAVE_ERR_MSG)) {
            return nullptr;
        }
    }
    return obj;
}

template <typename Container, auto Method>
int variadic_push_cmd(ClientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) {
    if (objc < 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "varName value ?value ...?");
        return TCL_ERROR;
    }

    Tcl_Obj* this_obj = resolve_mutable_var(interp, objv[1]);
    if (!this_obj) {
        return TCL_ERROR;
    }
    
    try {
        Container* c = tclxx::obj_cast::to<Container*>(interp, this_obj);
        for (int i = 2; i < objc; ++i) {
            if constexpr (std::is_member_function_pointer_v<decltype(Method)>) {
                using traits = tclxx::cmd::detail::member_function_traits<decltype(Method)>;
                if constexpr (traits::arity == 1) {
                    using Arg = std::tuple_element_t<0, typename traits::args>;
                    using RawArg = std::remove_cv_t<std::remove_reference_t<Arg>>;
                    (c->*Method)(tclxx::obj_cast::to<RawArg>(interp, objv[i]));
                } else if constexpr (traits::arity == 2) {
                    using Arg = std::tuple_element_t<1, typename traits::args>;
                    using RawArg = std::remove_cv_t<std::remove_reference_t<Arg>>;
                    (c->*Method)(interp, tclxx::obj_cast::to<RawArg>(interp, objv[i]));
                }
            } else {
                using traits = tclxx::cmd::detail::function_traits<decltype(Method)>;
                static_assert(traits::arity == 2, "variadic push function must accept container and value");
                using Arg = std::tuple_element_t<1, typename traits::args>;
                using RawArg = std::remove_cv_t<std::remove_reference_t<Arg>>;
                Method(c, tclxx::obj_cast::to<RawArg>(interp, objv[i]));
            }
        }
        Tcl_InvalidateStringRep(this_obj);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(c->size()));
        return TCL_OK;
    } catch (const std::exception& e) {
        Tcl_InvalidateStringRep(this_obj);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(e.what(), -1));
        return TCL_ERROR;
    } catch (...) {
        Tcl_InvalidateStringRep(this_obj);
        Tcl_SetObjResult(interp, Tcl_NewStringObj("unknown C++ exception", -1));
        return TCL_ERROR;
    }
}

template <typename Container, auto Method>
int variadic_set_insert_cmd(ClientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) {
    if (objc < 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "varName key ?key ...?");
        return TCL_ERROR;
    }

    Tcl_Obj* this_obj = resolve_mutable_var(interp, objv[1]);
    if (!this_obj) {
        return TCL_ERROR;
    }

    try {
        auto* c = tclxx::obj_cast::to<Container*>(interp, this_obj);
        using traits = tclxx::cmd::detail::member_function_traits<decltype(Method)>;
        for (int i = 2; i < objc; ++i) {
            if constexpr (traits::arity == 1) {
                using Arg = std::tuple_element_t<0, typename traits::args>;
                using RawArg = std::remove_cv_t<std::remove_reference_t<Arg>>;
                (c->*Method)(tclxx::obj_cast::to<RawArg>(interp, objv[i]));
            } else if constexpr (traits::arity == 2) {
                using Arg = std::tuple_element_t<1, typename traits::args>;
                using RawArg = std::remove_cv_t<std::remove_reference_t<Arg>>;
                (c->*Method)(interp, tclxx::obj_cast::to<RawArg>(interp, objv[i]));
            }
        }
        Tcl_InvalidateStringRep(this_obj);
        Tcl_ResetResult(interp);
        return TCL_OK;
    } catch (const std::exception& e) {
        Tcl_InvalidateStringRep(this_obj);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(e.what(), -1));
        return TCL_ERROR;
    } catch (...) {
        Tcl_InvalidateStringRep(this_obj);
        Tcl_SetObjResult(interp, Tcl_NewStringObj("unknown C++ exception", -1));
        return TCL_ERROR;
    }
}

template <typename Container, auto Method>
int getter_obj0_cmd(ClientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) {
    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "object");
        return TCL_ERROR;
    }

    try {
        Container* c = tclxx::obj_cast::to<Container*>(interp, objv[1]);
        using traits = tclxx::cmd::detail::member_function_traits<decltype(Method)>;
        auto val = (c->*Method)();
        if constexpr (std::is_same_v<typename traits::return_type, Tcl_Obj*>) {
            Tcl_SetObjResult(interp, val);
        } else {
            Tcl_SetObjResult(interp, tclxx::obj_cast::from(val));
        }
        return TCL_OK;
    } catch (const std::exception& e) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(e.what(), -1));
        return TCL_ERROR;
    } catch (...) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("unknown C++ exception", -1));
        return TCL_ERROR;
    }
}

template <typename Container, auto Method>
int getter_obj_int_cmd(ClientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) {
    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "object index");
        return TCL_ERROR;
    }

    try {
        Container* c = tclxx::obj_cast::to<Container*>(interp, objv[1]);
        const int index = tclxx::obj_cast::to<int>(interp, objv[2]);
        using traits = tclxx::cmd::detail::member_function_traits<decltype(Method)>;
        auto val = (c->*Method)(index);
        if constexpr (std::is_same_v<typename traits::return_type, Tcl_Obj*>) {
            Tcl_SetObjResult(interp, val);
        } else {
            Tcl_SetObjResult(interp, tclxx::obj_cast::from(val));
        }
        return TCL_OK;
    } catch (const std::exception& e) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(e.what(), -1));
        return TCL_ERROR;
    } catch (...) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("unknown C++ exception", -1));
        return TCL_ERROR;
    }
}

template <typename Container, auto Method>
int getter_obj_key_cmd(ClientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) {
    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "object key");
        return TCL_ERROR;
    }

    try {
        Container* c = tclxx::obj_cast::to<Container*>(interp, objv[1]);
        using traits = tclxx::cmd::detail::member_function_traits<decltype(Method)>;
        if constexpr (traits::arity == 1) {
            using Arg = std::tuple_element_t<0, typename traits::args>;
            auto val = (c->*Method)(tclxx::obj_cast::to<Arg>(interp, objv[2]));
            if constexpr (std::is_same_v<typename traits::return_type, Tcl_Obj*>) {
                Tcl_SetObjResult(interp, val);
            } else {
                Tcl_SetObjResult(interp, tclxx::obj_cast::from(val));
            }
        } else {
            static_assert(traits::arity == 1, "getter_obj_key_cmd expecting 1 argument");
        }
        return TCL_OK;
    } catch (const std::exception& e) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(e.what(), -1));
        return TCL_ERROR;
    } catch (...) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("unknown C++ exception", -1));
        return TCL_ERROR;
    }
}

template <typename Container, auto Method>
int setter_obj0_cmd(ClientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) {
    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "varName");
        return TCL_ERROR;
    }

    Tcl_Obj* this_obj = resolve_mutable_var(interp, objv[1]);
    if (!this_obj) {
        return TCL_ERROR;
    }

    try {
        Container* c = tclxx::obj_cast::to<Container*>(interp, this_obj);
        using traits = tclxx::cmd::detail::member_function_traits<decltype(Method)>;
        auto val = (c->*Method)();
        Tcl_InvalidateStringRep(this_obj);
        if constexpr (std::is_same_v<typename traits::return_type, Tcl_Obj*>) {
            Tcl_SetObjResult(interp, val);
        } else if constexpr (!std::is_void_v<typename traits::return_type>) {
            Tcl_SetObjResult(interp, tclxx::obj_cast::from(val));
        }
        return TCL_OK;
    } catch (const std::exception& e) {
        Tcl_InvalidateStringRep(this_obj);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(e.what(), -1));
        return TCL_ERROR;
    } catch (...) {
        Tcl_InvalidateStringRep(this_obj);
        Tcl_SetObjResult(interp, Tcl_NewStringObj("unknown C++ exception", -1));
        return TCL_ERROR;
    }
}

} // namespace stdcontainers::cmd_helpers
