#include <tcl.h>

#include <gtest/gtest.h>

#include "stdthread/stdthread.hpp"

namespace {

class StdMutexTest : public ::testing::Test {
protected:
    void SetUp() override {
        interp_ = Tcl_CreateInterp();
        ASSERT_NE(interp_, nullptr);

        ASSERT_EQ(Tcl_Init(interp_), TCL_OK)
            << "Tcl_Init failed: " << Tcl_GetStringResult(interp_);
        ASSERT_EQ(Stdthread_Init(interp_), TCL_OK)
            << "Stdthread_Init failed: " << Tcl_GetStringResult(interp_);
    }

    void TearDown() override {
        if (interp_ != nullptr) {
            Tcl_DeleteInterp(interp_);
            interp_ = nullptr;
        }
    }

    void EvalOk(const char* script) {
        ASSERT_EQ(Tcl_Eval(interp_, script), TCL_OK)
            << "Tcl script failed: " << script << "\n"
            << Tcl_GetStringResult(interp_);
    }

    void EvalError(const char* script) {
        const int code = Tcl_Eval(interp_, script);
        EXPECT_NE(code, TCL_OK)
            << "Tcl script unexpectedly succeeded: " << script;
    }

    void ExpectBooleanTrue(const char* message) {
        int value = 0;
        ASSERT_EQ(Tcl_GetBooleanFromObj(interp_, Tcl_GetObjResult(interp_), &value), TCL_OK)
            << "Expected boolean result";
        EXPECT_NE(value, 0) << message;
    }

    Tcl_Interp* interp_ = nullptr;
};

TEST_F(StdMutexTest, NewAndAddressString) {
    EvalOk("set m [::std::mutex::new]");
    EvalOk("expr {[string is wideinteger -strict $m]}");
    ExpectBooleanTrue("mutex handle should stringify as integer address");
}

TEST_F(StdMutexTest, LockTryUnlockFlow) {
    EvalOk("set m [::std::mutex::new]");
    EvalOk("::std::mutex::lock $m");
    EvalOk("expr {[::std::mutex::try_lock $m] == 0}");
    ExpectBooleanTrue("try_lock should fail when already locked by this thread");
    EvalOk("::std::mutex::unlock $m");

    EvalOk("expr {[::std::mutex::try_lock $m] == 1}");
    ExpectBooleanTrue("try_lock should succeed after unlock");
    EvalOk("::std::mutex::unlock $m");
}

TEST_F(StdMutexTest, GuardAndScopedLock) {
    EvalOk("set m [::std::mutex::new]");
    EvalOk("set g [::std::mutex::guard::new $m]");
    EvalOk("expr {[::std::mutex::guard::release $g] == 1}");
    ExpectBooleanTrue("guard release should unlock mutex");

    EvalOk("set a [::std::mutex::new]");
    EvalOk("set b [::std::mutex::new]");
    EvalOk("set sl [::std::scoped_lock::new $a $b]");
    EvalOk("unset sl");

    EvalOk("expr {[::std::mutex::try_lock $a] == 1}");
    ExpectBooleanTrue("scoped lock should release first mutex on destruction");
    EvalOk("::std::mutex::unlock $a");

    EvalOk("expr {[::std::mutex::try_lock $b] == 1}");
    ExpectBooleanTrue("scoped lock should release second mutex on destruction");
    EvalOk("::std::mutex::unlock $b");
}

TEST_F(StdMutexTest, WrongArgCountsFail) {
    EvalError("::std::mutex::new x");
    EvalError("::std::mutex::lock");
    EvalError("::std::mutex::try_lock");
    EvalError("::std::mutex::unlock");
    EvalError("::std::mutex::guard::new");
    EvalError("::std::mutex::guard::release");
    EvalError("::std::scoped_lock::new");
}

} // namespace
