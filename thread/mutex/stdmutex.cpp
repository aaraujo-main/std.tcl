#include "stdthread/stdthread.hpp"

#include "tclxx.hpp"

#include <tcl.h>

#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr const char* kPackageName = "std::mutex";
constexpr const char* kPackageVersion = "0.1.0";

using stdthread::model::Mutex;
using stdthread::model::MutexGuard;
using stdthread::model::ScopeLock;

std::string pointer_to_string(const void* pointer) {
    return std::to_string(static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(pointer)));
}

} // namespace

namespace stdthread {
namespace model {

class ScopeLock::RuntimeScopedLock {
public:
    explicit RuntimeScopedLock(std::vector<std::shared_ptr<std::mutex>> mutexes) : mutexes_(std::move(mutexes)) {
        locks_.reserve(mutexes_.size());
        for (const auto& mutexRep : mutexes_) {
            if (!mutexRep) {
                throw std::runtime_error("invalid std::mutex handle");
            }
            locks_.emplace_back(*mutexRep);
        }
    }

private:
    std::vector<std::shared_ptr<std::mutex>> mutexes_;
    std::vector<std::unique_lock<std::mutex>> locks_;
};



Mutex::Mutex() : mutex_(std::make_shared<std::mutex>()) {}

Mutex::Mutex(std::shared_ptr<std::mutex> mutex) : mutex_(std::move(mutex)) {
    if (!mutex_) {
        throw std::runtime_error("invalid std::mutex handle");
    }
}

void Mutex::lock() {
    if (!mutex_) {
        throw std::runtime_error("invalid std::mutex handle");
    }
    mutex_->lock();
}

bool Mutex::try_lock() {
    if (!mutex_) {
        throw std::runtime_error("invalid std::mutex handle");
    }
    return mutex_->try_lock();
}

void Mutex::unlock() {
    if (!mutex_) {
        throw std::runtime_error("invalid std::mutex handle");
    }
    mutex_->unlock();
}

std::shared_ptr<std::mutex> Mutex::native() const {
    return mutex_;
}

std::mutex* Mutex::address() const {
    return mutex_.get();
}

MutexGuard::MutexGuard(Mutex* mutex) {
    if (mutex == nullptr || !mutex->native()) {
        throw std::runtime_error("invalid std::mutex handle");
    }
    guard_ = std::make_shared<std::unique_lock<std::mutex>>(*mutex->native());
}

MutexGuard::MutexGuard(std::shared_ptr<std::unique_lock<std::mutex>> guard) : guard_(std::move(guard)) {
    if (!guard_) {
        throw std::runtime_error("invalid std::mutex_guard handle");
    }
}

bool MutexGuard::release() {
    if (!guard_) {
        throw std::runtime_error("invalid std::mutex_guard handle");
    }

    const bool hadLock = guard_->owns_lock();
    if (hadLock) {
        guard_->unlock();
    }
    return hadLock;
}

std::shared_ptr<std::unique_lock<std::mutex>> MutexGuard::native() const {
    return guard_;
}

std::unique_lock<std::mutex>* MutexGuard::native_lock() const {
    return guard_.get();
}

std::unique_lock<std::mutex>* MutexGuard::address() const {
    return guard_.get();
}

ScopeLock::ScopeLock(std::vector<std::shared_ptr<std::mutex>> mutexes)
    : scopedLock_(std::make_shared<RuntimeScopedLock>(std::move(mutexes))) {}

ScopeLock::ScopeLock(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) {
    (void)clientData;
    std::vector<std::shared_ptr<std::mutex>> mutexes;
    mutexes.reserve(static_cast<std::size_t>(objc - 1));
    for (int i = 1; i < objc; ++i) {
        auto* mutexHandle = tclxx::obj_cast::to<Mutex*>(interp, objv[i]);
        if (mutexHandle == nullptr) {
            throw std::runtime_error("invalid std::mutex handle");
        }
        mutexes.push_back(mutexHandle->native());
    }
    scopedLock_ = std::make_shared<RuntimeScopedLock>(std::move(mutexes));
}

ScopeLock::ScopeLock(std::shared_ptr<RuntimeScopedLock> scopedLock) : scopedLock_(std::move(scopedLock)) {
    if (!scopedLock_) {
        throw std::runtime_error("invalid std::scoped_lock handle");
    }
}

void ScopeLock::touch() const {
    if (!scopedLock_) {
        throw std::runtime_error("invalid std::scoped_lock handle");
    }
}

std::uintptr_t ScopeLock::address() const {
    return reinterpret_cast<std::uintptr_t>(scopedLock_.get());
}


} // namespace model
} // namespace stdthread

namespace tclxx {

template <>
std::string ObjType<stdthread::model::Mutex>::ToString(const stdthread::model::Mutex& value) {
    return pointer_to_string(value.address());
}

template <>
stdthread::model::Mutex ObjType<stdthread::model::Mutex>::FromAny(Tcl_Interp* interp, Tcl_Obj* const obj) {
    (void)interp;
    (void)obj;
    return stdthread::model::Mutex();
}

template <>
std::string ObjType<stdthread::model::MutexGuard>::ToString(const stdthread::model::MutexGuard& value) {
    return pointer_to_string(value.address());
}

template <>
stdthread::model::MutexGuard ObjType<stdthread::model::MutexGuard>::FromAny(Tcl_Interp* interp, Tcl_Obj* const obj) {
    (void)interp;
    (void)obj;

    auto backingMutex = std::make_shared<std::mutex>();
    auto guard = std::shared_ptr<std::unique_lock<std::mutex>>(
        new std::unique_lock<std::mutex>(*backingMutex),
        [backingMutex](std::unique_lock<std::mutex>* lockPtr) {
            delete lockPtr;
        });
    return stdthread::model::MutexGuard(std::move(guard));
}

template <>
std::string ObjType<stdthread::model::ScopeLock>::ToString(const stdthread::model::ScopeLock& value) {
    return std::to_string(static_cast<unsigned long long>(value.address()));
}

template <>
stdthread::model::ScopeLock ObjType<stdthread::model::ScopeLock>::FromAny(Tcl_Interp* interp, Tcl_Obj* const obj) {
    (void)interp;
    (void)obj;
    return stdthread::model::ScopeLock(std::vector<std::shared_ptr<std::mutex>>{});
}

} // namespace tclxx

extern "C" int Stdmutex_Init(Tcl_Interp* interp) {
    if (interp == nullptr) {
        return TCL_ERROR;
    }

    tclxx::ObjType<stdthread::model::Mutex>::GetType();
    tclxx::ObjType<stdthread::model::MutexGuard>::GetType();
    tclxx::ObjType<stdthread::model::ScopeLock>::GetType();

    Tcl_CreateNamespace(interp, "::std", nullptr, nullptr);
    Tcl_CreateNamespace(interp, "::std::mutex", nullptr, nullptr);
    Tcl_CreateNamespace(interp, "::std::mutex::guard", nullptr, nullptr);
    Tcl_CreateNamespace(interp, "::std::scoped_lock", nullptr, nullptr);

    TCLXX_CMD_NEW0_SHARED(interp, "::std::mutex::new.shared", stdthread::model::Mutex);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::mutex::lock", &stdthread::model::Mutex::lock);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::mutex::try_lock", &stdthread::model::Mutex::try_lock);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::mutex::unlock", &stdthread::model::Mutex::unlock);

    TCLXX_CMD_NEW(interp, "::std::mutex::guard::new", stdthread::model::MutexGuard, Mutex*);
    TCLXX_CMD_GETTER_METHOD(interp, "::std::mutex::guard::release", &stdthread::model::MutexGuard::release);

    TCLXX_CMD_NEWARGS_SHARED(interp, "::std::scoped_lock::new.shared(args)", stdthread::model::ScopeLock);

    return Tcl_PkgProvideEx(interp, kPackageName, kPackageVersion, nullptr);
}
