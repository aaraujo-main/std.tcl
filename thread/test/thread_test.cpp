#include <tcl.h>

#include <gtest/gtest.h>

#include "stdthread/stdthread.hpp"

namespace {

class StdThreadTest : public ::testing::Test {
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

} // namespace
