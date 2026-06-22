#include <tcl.h>

#include <gtest/gtest.h>

#include "stdthread/stdthread.hpp"

namespace {

class StdConditionalVariableTest : public ::testing::Test {
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

    void ExpectBoolean(bool expected, const char* message) {
        int value = 0;
        ASSERT_EQ(Tcl_GetBooleanFromObj(interp_, Tcl_GetObjResult(interp_), &value), TCL_OK)
            << "Expected boolean result";
        EXPECT_EQ(value != 0, expected) << message;
    }

    Tcl_Interp* interp_ = nullptr;
};

TEST_F(StdConditionalVariableTest, NewAndAddressString) {
    EvalOk("set cv [::std::conditional_variable::new]");
    EvalOk("expr {[string is wideinteger -strict $cv]}");
    ExpectBoolean(true, "conditional variable should stringify as integer address");
}

TEST_F(StdConditionalVariableTest, WaitForTimeoutAndNotify) {
    EvalOk("set m [::std::mutex::new]");
    EvalOk("set g [::std::mutex::guard::new $m]");
    EvalOk("set cv [::std::conditional_variable::new]");

    EvalOk("expr {[::std::conditional_variable::wait_for $cv $g 1] == 0}");
    ExpectBoolean(true, "wait_for should time out without notifications");

    EvalOk("::std::conditional_variable::notify_one $cv");
    EvalOk("::std::conditional_variable::notify_all $cv");
}

TEST_F(StdConditionalVariableTest, TypoNamespaceAliasIsAvailable) {
    EvalOk("set cv [::std::coditional_variable::new]");
    EvalOk("set m [::std::mutex::new]");
    EvalOk("set g [::std::mutex::guard::new $m]");
    EvalOk("expr {[::std::coditional_variable::wait_for $cv $g 1] == 0}");
    ExpectBoolean(true, "coditional_variable alias should resolve to same implementation");
}

TEST_F(StdConditionalVariableTest, GuardMustOwnLock) {
    EvalOk("set m [::std::mutex::new]");
    EvalOk("set g [::std::mutex::guard::new $m]");
    EvalOk("::std::mutex::guard::release $g");
    EvalOk("set cv [::std::conditional_variable::new]");

    EvalError("::std::conditional_variable::wait_for $cv $g 1");
}

} // namespace
