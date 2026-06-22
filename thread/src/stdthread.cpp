#include "stdthread/stdthread.hpp"

#include "tclxx.hpp"

#include <tcl.h>

#include <chrono>
#include <cstdio>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

constexpr const char* kPackageName = "std::thread";
constexpr const char* kPackageVersion = "0.1.0";

using stdthread::model::Thread;
using stdthread::model::ThisThread;

std::string pointer_to_string(const void* pointer) {
    return std::to_string(static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(pointer)));
}

struct CapturedThreadArg {
    enum class Kind {
        tclobj_value,
        int_value,
        double_value,
        string_value,
        bool_value
    };

    Kind kind = Kind::tclobj_value;
    std::string name;
    Tcl_Obj tclObjValue;
    int intValue = 0;
    double doubleValue = 0.0;
    std::string stringValue;
    bool boolValue = false;
};

CapturedThreadArg::Kind parse_arg_type(const std::string& typeName) {
    if (typeName == "int") {
        return CapturedThreadArg::Kind::int_value;
    }
    if (typeName == "double") {
        return CapturedThreadArg::Kind::double_value;
    }
    if (typeName == "string") {
        return CapturedThreadArg::Kind::string_value;
    }
    if (typeName == "bool") {
        return CapturedThreadArg::Kind::bool_value;
    }
    return CapturedThreadArg::Kind::tclobj_value;
}

Tcl_Obj* make_thread_arg_obj(CapturedThreadArg& arg) {
    switch (arg.kind) {
    case CapturedThreadArg::Kind::tclobj_value: {
        return Tcl_DuplicateObj(&arg.tclObjValue);
    }
    case CapturedThreadArg::Kind::int_value:
        return Tcl_NewIntObj(arg.intValue);
    case CapturedThreadArg::Kind::double_value:
        return Tcl_NewDoubleObj(arg.doubleValue);
    case CapturedThreadArg::Kind::string_value:
        return Tcl_NewStringObj(arg.stringValue.c_str(), -1);
    case CapturedThreadArg::Kind::bool_value:
        return Tcl_NewBooleanObj(arg.boolValue ? 1 : 0);
    }

    throw std::runtime_error("unsupported argument kind");
}

int thread_new_cmd(ClientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) {

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "func ?arg1 arg2 ...?");
        return TCL_ERROR;
    }
    try {
        int funcElementCount = 0;
        Tcl_Obj** funcElements = nullptr;
        if (Tcl_ListObjGetElements(interp, objv[1], &funcElementCount, &funcElements) != TCL_OK) {
            throw std::runtime_error("func must be a list: {t_args body}");
        }
        if (funcElementCount != 2) {
            throw std::runtime_error("func must be exactly: {t_args body}");
        }

        Tcl_Obj* argsSpecObj = funcElements[0];
        Tcl_Obj* bodyObj = funcElements[1];

        int argSpecCount = 0;
        Tcl_Obj** argSpecs = nullptr;
        if (Tcl_ListObjGetElements(interp, argsSpecObj, &argSpecCount, &argSpecs) != TCL_OK) {
            throw std::runtime_error("t_args must be a proper list");
        }

        const int providedArgCount = objc - 2;
        if (argSpecCount != providedArgCount) {
            throw std::runtime_error(
                "argument count mismatch: func expects " + std::to_string(argSpecCount) +
                ", got " + std::to_string(providedArgCount));
        }

        std::vector<CapturedThreadArg> capturedArgs;
        capturedArgs.reserve(static_cast<std::size_t>(argSpecCount));

        for (int index = 0; index < argSpecCount; ++index) {
            int specElementCount = 0;
            Tcl_Obj** specElements = nullptr;
            if (Tcl_ListObjGetElements(interp, argSpecs[index], &specElementCount, &specElements) != TCL_OK) {
                throw std::runtime_error("each t_arg entry must be {name} or {type name}");
            }

            std::string argName;
            CapturedThreadArg::Kind argKind = CapturedThreadArg::Kind::tclobj_value;

            if (specElementCount == 1) {
                argName = Tcl_GetString(specElements[0]);
            } else if (specElementCount == 2) {
                const std::string typeName = Tcl_GetString(specElements[0]);
                argKind = parse_arg_type(typeName);
                argName = Tcl_GetString(specElements[1]);
            } else {
                throw std::runtime_error("each t_arg entry must be {name} or {type name}");
            }

            if (argName.empty()) {
                throw std::runtime_error("thread argument name cannot be empty");
            }

            CapturedThreadArg captured;
            captured.name = argName;
            captured.kind = argKind;

            Tcl_Obj* providedArgObj = objv[index + 2];
            switch (argKind) {
            case CapturedThreadArg::Kind::tclobj_value: {
                // copy value to heap so other interpreter has access
                captured.tclObjValue = *providedArgObj;
                break;
            }
            case CapturedThreadArg::Kind::int_value:
                captured.intValue = tclxx::obj_cast::to<int>(interp, providedArgObj);
                break;
            case CapturedThreadArg::Kind::double_value:
                captured.doubleValue = tclxx::obj_cast::to<double>(interp, providedArgObj);
                break;
            case CapturedThreadArg::Kind::string_value:
                captured.stringValue = tclxx::obj_cast::to<std::string>(interp, providedArgObj);
                break;
            case CapturedThreadArg::Kind::bool_value:
                captured.boolValue = tclxx::obj_cast::to<bool>(interp, providedArgObj);
                break;
            }

            capturedArgs.push_back(std::move(captured));
        }

        const std::string bodyScript = Tcl_GetString(bodyObj);
        std::thread worker([bodyScript, capturedArgs = std::move(capturedArgs)]() {
            Tcl_Interp* threadInterp = Tcl_CreateInterp();
            if (threadInterp == nullptr) {
                return;
            }

            struct InterpGuard {
                Tcl_Interp* interp = nullptr;

                ~InterpGuard() {
                    if (interp != nullptr) {
                        Tcl_DeleteInterp(interp);
                    }
                }
            } interpGuard{threadInterp};

            if (Tcl_Init(threadInterp) != TCL_OK) {
                return;
            }

            if (Stdthread_Init(threadInterp) != TCL_OK) {
                return;
            }

            for (const auto& arg : capturedArgs) {
                Tcl_Obj* valueObj = nullptr;
                try {
                    valueObj = make_thread_arg_obj(const_cast<CapturedThreadArg&>(arg));
                } catch (...) {
                    return;
                }

                if (Tcl_ObjSetVar2(
                        threadInterp,
                        Tcl_NewStringObj(arg.name.c_str(), -1),
                        nullptr,
                        valueObj,
                        TCL_LEAVE_ERR_MSG) == nullptr) {
                    return;
                }
            }

            Tcl_Obj* evalScriptObj = Tcl_NewStringObj(bodyScript.c_str(), -1);
            if (evalScriptObj != nullptr) {
                Tcl_IncrRefCount(evalScriptObj);
                Tcl_EvalObjEx(threadInterp, evalScriptObj, 0);
                Tcl_DecrRefCount(evalScriptObj);
            }
        });

        auto* threadHandle = new stdthread::model::Thread(std::move(worker));
        Tcl_SetObjResult(interp, tclxx::ObjType<stdthread::model::Thread>::New(threadHandle));
        return TCL_OK;
    } catch (const std::exception& ex) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ex.what(), -1));
        return TCL_ERROR;
    }
}

int thread_join_cmd(ClientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) {
    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "thread");
        return TCL_ERROR;
    }

    try {
        auto* threadHandle = tclxx::obj_cast::to<Thread*>(interp, objv[1]);
        if (threadHandle == nullptr) {
            throw std::runtime_error("invalid std::thread handle");
        }
        threadHandle->join();
        return TCL_OK;
    } catch (const std::exception& ex) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ex.what(), -1));
        return TCL_ERROR;
    }
}

int thread_swap_cmd(ClientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) {
    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "threadVar1 threadVar2");
        return TCL_ERROR;
    }

    Tcl_Obj* firstValue = Tcl_ObjGetVar2(interp, objv[1], nullptr, TCL_LEAVE_ERR_MSG);
    if (firstValue == nullptr) {
        return TCL_ERROR;
    }

    Tcl_Obj* secondValue = Tcl_ObjGetVar2(interp, objv[2], nullptr, TCL_LEAVE_ERR_MSG);
    if (secondValue == nullptr) {
        return TCL_ERROR;
    }

    try {
        auto* firstThread = tclxx::obj_cast::to<Thread*>(interp, firstValue);
        auto* secondThread = tclxx::obj_cast::to<Thread*>(interp, secondValue);
        if (firstThread == nullptr || secondThread == nullptr) {
            throw std::runtime_error("invalid std::thread handle");
        }

        firstThread->swap(secondThread);
        return TCL_OK;
    } catch (const std::exception& ex) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ex.what(), -1));
        return TCL_ERROR;
    }
}

int thread_invalidate_string_rep_cmd(ClientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) {
    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "objVar");
        return TCL_ERROR;
    }

    Tcl_Obj* value = Tcl_ObjGetVar2(interp, objv[1], nullptr, TCL_LEAVE_ERR_MSG);
    if (value == nullptr) {
        return TCL_ERROR;
    }

    Tcl_InvalidateStringRep(value);
    return TCL_OK;
}

int register_thread_commands(Tcl_Interp* interp) {
    tclxx::ObjType<stdthread::model::Thread>::GetType();

    Tcl_CreateNamespace(interp, "::std", nullptr, nullptr);
    Tcl_CreateNamespace(interp, "::std::thread", nullptr, nullptr);
    Tcl_CreateNamespace(interp, "::std::this_thread", nullptr, nullptr);

    Tcl_CreateObjCommand(interp, "::std::thread::new", thread_new_cmd, nullptr, nullptr);
    Tcl_CreateObjCommand(interp, "::std::thread::join", thread_join_cmd, nullptr, nullptr);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::thread::detach", &stdthread::model::Thread::detach);
    Tcl_CreateObjCommand(interp, "::std::thread::swap", thread_swap_cmd, nullptr, nullptr);
    Tcl_CreateObjCommand(interp, "::std::thread::invalidateStringRep", thread_invalidate_string_rep_cmd, nullptr, nullptr);

    TCLXX_CMD_STATIC(interp, "::std::this_thread::get_id", &stdthread::model::ThisThread::get_id);
    TCLXX_CMD_STATIC(interp, "::std::this_thread::sleep_for", &stdthread::model::ThisThread::sleep_for);

    return TCL_OK;
}

} // namespace

namespace stdthread {
namespace model {

Thread::Thread(std::thread&& thread) : thread_(std::make_shared<std::thread>(std::move(thread))) {}

Thread::Thread(std::shared_ptr<std::thread> thread) : thread_(std::move(thread)) {
    if (!thread_) {
        throw std::runtime_error("invalid std::thread handle");
    }
}

void Thread::join() {
    if (!thread_) {
        throw std::runtime_error("invalid std::thread handle");
    }
    if (!thread_->joinable()) {
        throw std::runtime_error("thread is not joinable");
    }
    thread_->join();
}

void Thread::detach() {
    if (!thread_) {
        throw std::runtime_error("invalid std::thread handle");
    }
    if (!thread_->joinable()) {
        throw std::runtime_error("thread is not joinable");
    }
    thread_->detach();
}

void Thread::swap(Thread* other) {
    if (!thread_ || other == nullptr || !other->thread_) {
        throw std::runtime_error("invalid std::thread handle");
    }
    thread_->swap(*(other->thread_));
}

bool Thread::joinable() const {
    return thread_ && thread_->joinable();
}

std::shared_ptr<std::thread> Thread::native() const {
    return thread_;
}

std::thread* Thread::address() const {
    return thread_.get();
}

std::string ThisThread::get_id() {
    std::ostringstream stream;
    stream << std::this_thread::get_id();
    return stream.str();
}

std::string ThisThread::sleep_for(long long milliseconds) {
    if (milliseconds < 0) {
        throw std::runtime_error("milliseconds must be non-negative");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    return get_id();
}

} // namespace model
} // namespace stdthread

namespace tclxx {

template <>
std::string ObjType<stdthread::model::Thread>::ToString(const stdthread::model::Thread& value) {
    return pointer_to_string(value.address());
}

template <>
stdthread::model::Thread ObjType<stdthread::model::Thread>::FromAny(Tcl_Interp* interp, Tcl_Obj* const obj) {
    (void)interp;
    (void)obj;
    return stdthread::model::Thread(std::thread());
}

} // namespace tclxx

extern "C" int Stdthread_Init(Tcl_Interp* interp) {
    if (interp == nullptr) {
        return TCL_ERROR;
    }
    
    if (Stdmutex_Init(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (Stdcond_Init(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    if (register_thread_commands(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    return Tcl_PkgProvideEx(interp, kPackageName, kPackageVersion, nullptr);
}
