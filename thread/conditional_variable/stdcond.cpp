#include "stdthread/stdthread.hpp"

#include "tclxx.hpp"

#include <tcl.h>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

constexpr const char* kPackageName = "std::conditional_variable";
constexpr const char* kPackageVersion = "0.1.0";

using stdthread::model::ConditionalVariable;



std::string pointer_to_string(const void* pointer) {
    return std::to_string(static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(pointer)));
}

} // namespace

namespace stdthread {
namespace model {

ConditionalVariable::ConditionalVariable() : conditionVariable_(std::make_shared<std::condition_variable>()) {}

ConditionalVariable::ConditionalVariable(std::shared_ptr<std::condition_variable> conditionVariable)
    : conditionVariable_(std::move(conditionVariable)) {
    if (!conditionVariable_) {
        throw std::runtime_error("invalid std::conditional_variable handle");
    }
}

void ConditionalVariable::wait(MutexGuard* guard) {
    if (!conditionVariable_) {
        throw std::runtime_error("invalid std::conditional_variable handle");
    }
    if (guard == nullptr || guard->native_lock() == nullptr) {
        throw std::runtime_error("invalid std::mutex_guard handle");
    }
    if (!guard->native_lock()->owns_lock()) {
        throw std::runtime_error("std::mutex_guard does not own the mutex lock");
    }

    conditionVariable_->wait(*(guard->native_lock()));
}

bool ConditionalVariable::wait_for(MutexGuard* guard, long long milliseconds) {
    if (!conditionVariable_) {
        throw std::runtime_error("invalid std::conditional_variable handle");
    }
    if (guard == nullptr || guard->native_lock() == nullptr) {
        throw std::runtime_error("invalid std::mutex_guard handle");
    }
    if (!guard->native_lock()->owns_lock()) {
        throw std::runtime_error("std::mutex_guard does not own the mutex lock");
    }
    if (milliseconds < 0) {
        throw std::runtime_error("milliseconds must be non-negative");
    }

    const auto status = conditionVariable_->wait_for(*(guard->native_lock()), std::chrono::milliseconds(milliseconds));
    return status == std::cv_status::no_timeout;
}

void ConditionalVariable::notify_one() {
    if (!conditionVariable_) {
        throw std::runtime_error("invalid std::conditional_variable handle");
    }
    conditionVariable_->notify_one();
}

void ConditionalVariable::notify_all() {
    if (!conditionVariable_) {
        throw std::runtime_error("invalid std::conditional_variable handle");
    }
    conditionVariable_->notify_all();
}

std::shared_ptr<std::condition_variable> ConditionalVariable::native() const {
    return conditionVariable_;
}

std::condition_variable* ConditionalVariable::address() const {
    return conditionVariable_.get();
}

} // namespace model
} // namespace stdthread

namespace tclxx {

template <>
std::string ObjType<stdthread::model::ConditionalVariable>::ToString(const stdthread::model::ConditionalVariable& value) {
    return pointer_to_string(value.address());
}

template <>
stdthread::model::ConditionalVariable ObjType<stdthread::model::ConditionalVariable>::FromAny(Tcl_Interp* interp,
                                                                                             Tcl_Obj* const obj) {
    (void)interp;
    (void)obj;
    return stdthread::model::ConditionalVariable();
}

} // namespace tclxx

extern "C" int Stdcond_Init(Tcl_Interp* interp) {
    if (interp == nullptr) {
        return TCL_ERROR;
    }

    tclxx::ObjType<stdthread::model::ConditionalVariable>::GetType();

    Tcl_CreateNamespace(interp, "::std", nullptr, nullptr);
    Tcl_CreateNamespace(interp, "::std::conditional_variable", nullptr, nullptr);
    Tcl_CreateNamespace(interp, "::std::coditional_variable", nullptr, nullptr);

    TCLXX_CMD_NEW0_SHARED(interp, "::std::conditional_variable::new.shared", stdthread::model::ConditionalVariable);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::conditional_variable::wait", &stdthread::model::ConditionalVariable::wait);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::conditional_variable::wait_for", &stdthread::model::ConditionalVariable::wait_for);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::conditional_variable::notify_one", &stdthread::model::ConditionalVariable::notify_one);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::conditional_variable::notify_all", &stdthread::model::ConditionalVariable::notify_all);

    Tcl_CreateObjCommand(interp,
                         "::std::coditional_variable::new",
                         ::tclxx::cmd::create<stdthread::model::ConditionalVariable>,
                         nullptr,
                         nullptr);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::coditional_variable::wait", &stdthread::model::ConditionalVariable::wait);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::coditional_variable::wait_for", &stdthread::model::ConditionalVariable::wait_for);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::coditional_variable::notify_one", &stdthread::model::ConditionalVariable::notify_one);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::coditional_variable::notify_all", &stdthread::model::ConditionalVariable::notify_all);

    return Tcl_PkgProvideEx(interp, kPackageName, kPackageVersion, nullptr);
}
