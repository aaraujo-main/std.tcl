#pragma once

#include <tcl.h>

#include <cstdint>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace stdthread {
namespace model {

class Mutex {
public:
	Mutex();
	explicit Mutex(std::shared_ptr<std::mutex> mutex);

	void lock();
	bool try_lock();
	void unlock();

	std::shared_ptr<std::mutex> native() const;
	std::mutex* address() const;

private:
	std::shared_ptr<std::mutex> mutex_;
};

class MutexGuard {
public:
	explicit MutexGuard(Mutex* mutex);
	explicit MutexGuard(std::shared_ptr<std::unique_lock<std::mutex>> guard);

	bool release();

	std::shared_ptr<std::unique_lock<std::mutex>> native() const;
	std::unique_lock<std::mutex>* native_lock() const;
	std::unique_lock<std::mutex>* address() const;

private:
	std::shared_ptr<std::unique_lock<std::mutex>> guard_;
};

class ConditionalVariable {
public:
	ConditionalVariable();
	explicit ConditionalVariable(std::shared_ptr<std::condition_variable> conditionVariable);

	void wait(MutexGuard* guard);
	bool wait_for(MutexGuard* guard, long long milliseconds);
	void notify_one();
	void notify_all();

	std::shared_ptr<std::condition_variable> native() const;
	std::condition_variable* address() const;

private:
	std::shared_ptr<std::condition_variable> conditionVariable_;
};

class Thread {
public:
	explicit Thread(std::thread&& thread);
	explicit Thread(std::shared_ptr<std::thread> thread);

	void join();
	void detach();
	void swap(Thread* other);

	bool joinable() const;
	std::shared_ptr<std::thread> native() const;
	std::thread* address() const;

private:
	std::shared_ptr<std::thread> thread_;
};

class ThisThread {
public:
	static std::string get_id();
	static std::string sleep_for(long long milliseconds);
};

class ScopeLock {
public:
	class RuntimeScopedLock;
	
	explicit ScopeLock(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
	explicit ScopeLock(std::vector<std::shared_ptr<std::mutex>> mutexes);
	void touch() const;

	std::uintptr_t address() const;

private:
	explicit ScopeLock(std::shared_ptr<RuntimeScopedLock> scopedLock);

	std::shared_ptr<RuntimeScopedLock> scopedLock_;
};

} // namespace model
} // namespace stdthread

extern "C" int Stdmutex_Init(Tcl_Interp* interp);
extern "C" int Stdcond_Init(Tcl_Interp* interp);
extern "C" int Stdthread_Init(Tcl_Interp* interp);
