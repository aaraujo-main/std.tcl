#include <gtest/gtest.h>
#include <tcl.h>

#include <string>

#include "containers/containers.hpp"

namespace {

class ContainersTest : public ::testing::Test {
protected:
    Tcl_Interp* interp;

    void SetUp() override {
        interp = Tcl_CreateInterp();
        ASSERT_NE(interp, nullptr);
        ASSERT_EQ(Tcl_Init(interp), TCL_OK);
        ASSERT_EQ(Stdcontainers_Init(interp), TCL_OK);
    }

    void TearDown() override {
        if (interp) {
            Tcl_DeleteInterp(interp);
        }
    }

    bool eval_ok(const char* script) {
        if (Tcl_Eval(interp, script) != TCL_OK) {
            ADD_FAILURE() << "Tcl error for script: " << script << "\n"
                         << Tcl_GetStringResult(interp);
            return false;
        }
        return true;
    }

    bool eval_error(const char* script) {
        if (Tcl_Eval(interp, script) == TCL_OK) {
            ADD_FAILURE() << "script unexpectedly succeeded: " << script;
            return false;
        }
        return true;
    }

    bool expect_result_true() {
        int value = 0;
        if (Tcl_GetBooleanFromObj(interp, Tcl_GetObjResult(interp), &value) != TCL_OK) {
            ADD_FAILURE() << "result is not boolean";
            return false;
        }
        return value != 0;
    }
};

TEST_F(ContainersTest, VectorCreate) {
    EXPECT_TRUE(eval_ok("set v [::std::vector::new]"));
}

TEST_F(ContainersTest, VectorReserve) {
    EXPECT_TRUE(eval_ok("set v [::std::vector::new]"));
    EXPECT_TRUE(eval_ok("::std::vector::reserve v 128"));
}

TEST_F(ContainersTest, VectorPushVariadic) {
    EXPECT_TRUE(eval_ok("set v [::std::vector::new]"));
    EXPECT_TRUE(eval_ok("::std::vector::push v a b c"));
}

TEST_F(ContainersTest, VectorSize) {
    EXPECT_TRUE(eval_ok("set v [::std::vector::new]"));
    EXPECT_TRUE(eval_ok("::std::vector::push v a b c"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector::size $v] == 3}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorAt) {
    EXPECT_TRUE(eval_ok("set v [::std::vector::new]"));
    EXPECT_TRUE(eval_ok("::std::vector::push v a b c"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector::at $v 1] eq \"b\"}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorSet) {
    EXPECT_TRUE(eval_ok("set v [::std::vector::new]"));
    EXPECT_TRUE(eval_ok("::std::vector::push v a b c"));
    EXPECT_TRUE(eval_ok("::std::vector::set v 1 x"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector::at $v 1] eq \"x\"}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorPop) {
    EXPECT_TRUE(eval_ok("set v [::std::vector::new]"));
    EXPECT_TRUE(eval_ok("::std::vector::push v a b c"));
    EXPECT_TRUE(eval_ok("set p [::std::vector::pop v]"));
    EXPECT_TRUE(eval_ok("expr {$p eq \"c\"}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorToString) {
    EXPECT_TRUE(eval_ok("set v [::std::vector::new]"));
    EXPECT_TRUE(eval_ok("::std::vector::clear v"));
    EXPECT_TRUE(eval_ok("::std::vector::push v {a b} c"));
    EXPECT_TRUE(eval_ok("set v_str [format %s $v]"));
    EXPECT_TRUE(eval_ok("expr {$v_str eq {{a b} c}}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorSetOutOfRange) {
    EXPECT_TRUE(eval_ok("set v [::std::vector::new]"));
    EXPECT_TRUE(eval_ok("::std::vector::push v a b c"));
    EXPECT_TRUE(eval_error("::std::vector::set v 9 z"));
}

TEST_F(ContainersTest, VectorAtOutOfRange) {
    EXPECT_TRUE(eval_ok("set v [::std::vector::new]"));
    EXPECT_TRUE(eval_ok("::std::vector::push v a b c"));
    EXPECT_TRUE(eval_error("::std::vector::at $v 9"));
}

TEST_F(ContainersTest, ListCreate) {
    EXPECT_TRUE(eval_ok("set l [::std::list::new]"));
}

TEST_F(ContainersTest, ListSharedCreateRemoved) {
    EXPECT_TRUE(eval_ok("expr {[llength [info commands ::std::list::new.shared]] == 0}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, ListPushVariadic) {
    EXPECT_TRUE(eval_ok("set l [::std::list::new]"));
    EXPECT_TRUE(eval_ok("::std::list::push l x y z"));
}

TEST_F(ContainersTest, ListSize) {
    EXPECT_TRUE(eval_ok("set l [::std::list::new]"));
    EXPECT_TRUE(eval_ok("::std::list::push l x y z"));
    EXPECT_TRUE(eval_ok("expr {[::std::list::size $l] == 3}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, ListAt) {
    EXPECT_TRUE(eval_ok("set l [::std::list::new]"));
    EXPECT_TRUE(eval_ok("::std::list::push l x y z"));
    EXPECT_TRUE(eval_ok("expr {[::std::list::at $l 2] eq \"z\"}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, ListPop) {
    EXPECT_TRUE(eval_ok("set l [::std::list::new]"));
    EXPECT_TRUE(eval_ok("::std::list::push l x y z"));
    EXPECT_TRUE(eval_ok("set lp [::std::list::pop l]"));
    EXPECT_TRUE(eval_ok("expr {$lp eq \"z\"}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, ListToString) {
    EXPECT_TRUE(eval_ok("set l [::std::list::new]"));
    EXPECT_TRUE(eval_ok("::std::list::clear l"));
    EXPECT_TRUE(eval_ok("::std::list::push l {left right} tail"));
    EXPECT_TRUE(eval_ok("set l_str [format %s $l]"));
    EXPECT_TRUE(eval_ok("expr {$l_str eq {{left right} tail}}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, StackCreate) {
    EXPECT_TRUE(eval_ok("set s [::std::stack::new]"));
}

TEST_F(ContainersTest, StackSharedCreateRemoved) {
    EXPECT_TRUE(eval_ok("expr {[llength [info commands ::std::stack::new.shared]] == 0}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, StackPushVariadic) {
    EXPECT_TRUE(eval_ok("set s [::std::stack::new]"));
    EXPECT_TRUE(eval_ok("::std::stack::push s k1 k2"));
}

TEST_F(ContainersTest, StackList) {
    EXPECT_TRUE(eval_ok("set s [::std::stack::new]"));
    EXPECT_TRUE(eval_ok("::std::stack::push s k1 k2"));
    EXPECT_TRUE(eval_ok("expr {[::std::stack::list $s] eq {k1 k2}}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, StackTop) {
    EXPECT_TRUE(eval_ok("set s [::std::stack::new]"));
    EXPECT_TRUE(eval_ok("::std::stack::push s k1 k2"));
    EXPECT_TRUE(eval_ok("expr {[::std::stack::top $s] eq \"k2\"}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, StackPop) {
    EXPECT_TRUE(eval_ok("set s [::std::stack::new]"));
    EXPECT_TRUE(eval_ok("::std::stack::push s k1 k2"));
    EXPECT_TRUE(eval_ok("set sp [::std::stack::pop s]"));
    EXPECT_TRUE(eval_ok("expr {$sp eq \"k2\"}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, StackToString) {
    EXPECT_TRUE(eval_ok("set s [::std::stack::new]"));
    EXPECT_TRUE(eval_ok("::std::stack::clear s"));
    EXPECT_TRUE(eval_ok("::std::stack::push s {top value} tail"));
    EXPECT_TRUE(eval_ok("set s_str [format %s $s]"));
    EXPECT_TRUE(eval_ok("expr {$s_str eq {{top value} tail}}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, StackClear) {
    EXPECT_TRUE(eval_ok("set s [::std::stack::new]"));
    EXPECT_TRUE(eval_ok("::std::stack::push s k1 k2"));
    EXPECT_TRUE(eval_ok("::std::stack::clear s"));
}

TEST_F(ContainersTest, StackTopEmpty) {
    EXPECT_TRUE(eval_ok("set s [::std::stack::new]"));
    EXPECT_TRUE(eval_error("::std::stack::top $s"));
}

TEST_F(ContainersTest, SetCreate) {
    EXPECT_TRUE(eval_ok("set st [::std::set::new]"));
}

TEST_F(ContainersTest, SetSharedCreateRemoved) {
    EXPECT_TRUE(eval_ok("expr {[llength [info commands ::std::set::new.shared]] == 0}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, SetInsertVariadic) {
    EXPECT_TRUE(eval_ok("set st [::std::set::new]"));
    EXPECT_TRUE(eval_ok("set inserted [::std::set::insert st a {b c} a]"));
    EXPECT_TRUE(eval_ok("expr {$inserted eq {}}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, SetContains) {
    EXPECT_TRUE(eval_ok("set st [::std::set::new]"));
    EXPECT_TRUE(eval_ok("::std::set::insert st a {b c} a"));
    EXPECT_TRUE(eval_ok("expr {[::std::set::contains $st {b c}]}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, SetErase) {
    EXPECT_TRUE(eval_ok("set st [::std::set::new]"));
    EXPECT_TRUE(eval_ok("::std::set::insert st a {b c} a"));
    EXPECT_TRUE(eval_ok("::std::set::erase st {b c}"));
    EXPECT_TRUE(eval_ok("expr {![::std::set::contains $st {b c}]}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, SetToString) {
    EXPECT_TRUE(eval_ok("set st [::std::set::new]"));
    EXPECT_TRUE(eval_ok("::std::set::insert st a {b c}"));
    EXPECT_TRUE(eval_ok("set st_str [format %s $st]"));
    EXPECT_TRUE(eval_ok("expr {[llength $st_str] == 2 && [lsearch -exact $st_str a] >= 0}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, UnorderedMapCreate) {
    EXPECT_TRUE(eval_ok("set m [::std::unordered_map::new]"));
}

TEST_F(ContainersTest, UnorderedMapSharedCreateRemoved) {
    EXPECT_TRUE(eval_ok("expr {[llength [info commands ::std::unordered_map::new.shared]] == 0}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, UnorderedMapReserve) {
    EXPECT_TRUE(eval_ok("set m [::std::unordered_map::new]"));
    EXPECT_TRUE(eval_ok("::std::unordered_map::reserve m 128"));
}

TEST_F(ContainersTest, UnorderedMapPut) {
    EXPECT_TRUE(eval_ok("set m [::std::unordered_map::new]"));
    EXPECT_TRUE(eval_ok("::std::unordered_map::put m key1 value1"));
    EXPECT_TRUE(eval_ok("::std::unordered_map::put m key2 {value 2}"));
}

TEST_F(ContainersTest, UnorderedMapExists) {
    EXPECT_TRUE(eval_ok("set m [::std::unordered_map::new]"));
    EXPECT_TRUE(eval_ok("::std::unordered_map::put m key1 value1"));
    EXPECT_TRUE(eval_ok("expr {[::std::unordered_map::exists $m key1]}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, UnorderedMapGet) {
    EXPECT_TRUE(eval_ok("set m [::std::unordered_map::new]"));
    EXPECT_TRUE(eval_ok("::std::unordered_map::put m key2 {value 2}"));
    EXPECT_TRUE(eval_ok("expr {[::std::unordered_map::get $m key2] eq {value 2}}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, UnorderedMapDict) {
    EXPECT_TRUE(eval_ok("set m [::std::unordered_map::new]"));
    EXPECT_TRUE(eval_ok("::std::unordered_map::put m key1 value1"));
    EXPECT_TRUE(eval_ok("set md [::std::unordered_map::dict $m]"));
    EXPECT_TRUE(eval_ok("expr {[dict get $md key1] eq \"value1\"}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, UnorderedMapErase) {
    EXPECT_TRUE(eval_ok("set m [::std::unordered_map::new]"));
    EXPECT_TRUE(eval_ok("::std::unordered_map::put m key1 value1"));
    EXPECT_TRUE(eval_ok("::std::unordered_map::erase m key1"));
    EXPECT_TRUE(eval_ok("expr {![::std::unordered_map::exists $m key1]}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, UnorderedMapToString) {
    EXPECT_TRUE(eval_ok("set m [::std::unordered_map::new]"));
    EXPECT_TRUE(eval_ok("::std::unordered_map::put m key2 {value 2}"));
    EXPECT_TRUE(eval_ok("set m_str [format %s $m]"));
    EXPECT_TRUE(eval_ok("expr {[dict size $m_str] == 1 && [dict get $m_str key2] eq {value 2}}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, UnorderedMapMissingKey) {
    EXPECT_TRUE(eval_ok("set m [::std::unordered_map::new]"));
    EXPECT_TRUE(eval_ok("::std::unordered_map::put m key2 value2"));
    EXPECT_TRUE(eval_error("::std::unordered_map::get $m missing"));
}

class PerTypeInitTest : public ::testing::Test {
protected:
    Tcl_Interp* interp;

    void SetUp() override {
        interp = Tcl_CreateInterp();
        ASSERT_NE(interp, nullptr);
        ASSERT_EQ(Tcl_Init(interp), TCL_OK);
    }

    void TearDown() override {
        if (interp) {
            Tcl_DeleteInterp(interp);
        }
    }

    bool eval_ok(const char* script) {
        if (Tcl_Eval(interp, script) != TCL_OK) {
            ADD_FAILURE() << "Tcl error for script: " << script << "\n"
                         << Tcl_GetStringResult(interp);
            return false;
        }
        return true;
    }

    bool expect_result_true() {
        int value = 0;
        if (Tcl_GetBooleanFromObj(interp, Tcl_GetObjResult(interp), &value) != TCL_OK) {
            ADD_FAILURE() << "result is not boolean";
            return false;
        }
        return value != 0;
    }
};

TEST_F(PerTypeInitTest, ListInitSucceeds) {
    EXPECT_EQ(Stdcontainerslist_Init(interp), TCL_OK);
}

TEST_F(PerTypeInitTest, ListCommandRegistered) {
    EXPECT_EQ(Stdcontainerslist_Init(interp), TCL_OK);
    EXPECT_TRUE(eval_ok("expr {[llength [info commands ::std::list::new]] == 1}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(PerTypeInitTest, VectorNotRegisteredByListInit) {
    EXPECT_EQ(Stdcontainerslist_Init(interp), TCL_OK);
    EXPECT_TRUE(eval_ok("expr {[llength [info commands ::std::vector::new]] == 0}"));
    EXPECT_TRUE(expect_result_true());
}

// VectorHeap<int> tests
TEST_F(ContainersTest, VectorIntCreate) {
    EXPECT_TRUE(eval_ok("set vi [::std::vector<int>::new]"));
}

TEST_F(ContainersTest, VectorIntPush) {
    EXPECT_TRUE(eval_ok("set vi [::std::vector<int>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::push vi 1 2 3"));
}

TEST_F(ContainersTest, VectorIntSize) {
    EXPECT_TRUE(eval_ok("set vi [::std::vector<int>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::push vi 10 20 30"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<int>::size $vi] == 3}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorIntAt) {
    EXPECT_TRUE(eval_ok("set vi [::std::vector<int>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::push vi 100 200 300"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<int>::at $vi 1] == 200}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorIntSet) {
    EXPECT_TRUE(eval_ok("set vi [::std::vector<int>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::push vi 100 200 300"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::set vi 1 250"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<int>::at $vi 1] == 250}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorIntSharedConstructors) {
    EXPECT_TRUE(eval_ok("set vi [::std::vector<int>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::push vi 1 2"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::make_shared vi"));
    EXPECT_TRUE(eval_ok("set vi2 $vi"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::from_shared vi2"));
    EXPECT_TRUE(eval_ok("set vi3 [::std::vector<int>::new.shared]"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<int>::size $vi] == 2 && [::std::vector<int>::size $vi2] == 2 && [::std::vector<int>::size $vi3] == 0}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorIntPop) {
    EXPECT_TRUE(eval_ok("set vi [::std::vector<int>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::push vi 5 10 15"));
    EXPECT_TRUE(eval_ok("set vip [::std::vector<int>::pop vi]"));
    EXPECT_TRUE(eval_ok("expr {$vip == 15}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorIntClear) {
    EXPECT_TRUE(eval_ok("set vi [::std::vector<int>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::push vi 1 2 3"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::clear vi"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<int>::size $vi] == 0}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorIntReserve) {
    EXPECT_TRUE(eval_ok("set vi [::std::vector<int>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::reserve vi 256"));
}

TEST_F(ContainersTest, VectorIntToString) {
    EXPECT_TRUE(eval_ok("set vi [::std::vector<int>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::clear vi"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::push vi 1 2 3"));
    EXPECT_TRUE(eval_ok("set vi_str [format %s $vi]"));
    EXPECT_TRUE(eval_ok("expr {$vi_str eq {1 2 3}}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorIntAtOutOfRange) {
    EXPECT_TRUE(eval_ok("set vi [::std::vector<int>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<int>::push vi 1 2"));
    EXPECT_TRUE(eval_error("::std::vector<int>::at $vi 99"));
}

TEST_F(ContainersTest, VectorIntEmpty) {
    EXPECT_TRUE(eval_ok("set vi [::std::vector<int>::new]"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<int>::empty $vi]}"));
    EXPECT_TRUE(expect_result_true());
}

// VectorHeap<double> tests
TEST_F(ContainersTest, VectorDoubleCreate) {
    EXPECT_TRUE(eval_ok("set vd [::std::vector<double>::new]"));
}

TEST_F(ContainersTest, VectorDoublePush) {
    EXPECT_TRUE(eval_ok("set vd [::std::vector<double>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<double>::push vd 1.5 2.5 3.5"));
}

TEST_F(ContainersTest, VectorDoubleSize) {
    EXPECT_TRUE(eval_ok("set vd [::std::vector<double>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<double>::push vd 1.1 2.2 3.3"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<double>::size $vd] == 3}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorDoubleAt) {
    EXPECT_TRUE(eval_ok("set vd [::std::vector<double>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<double>::push vd 10.5 20.5 30.5"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<double>::at $vd 0] == 10.5}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorDoubleSet) {
    EXPECT_TRUE(eval_ok("set vd [::std::vector<double>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<double>::push vd 1.5 2.5 3.5"));
    EXPECT_TRUE(eval_ok("::std::vector<double>::set vd 1 4.5"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<double>::at $vd 1] == 4.5}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorDoublePop) {
    EXPECT_TRUE(eval_ok("set vd [::std::vector<double>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<double>::push vd 1.1 2.2 3.3"));
    EXPECT_TRUE(eval_ok("set vdp [::std::vector<double>::pop vd]"));
    EXPECT_TRUE(eval_ok("expr {$vdp == 3.3}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorDoubleClear) {
    EXPECT_TRUE(eval_ok("set vd [::std::vector<double>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<double>::push vd 1.5 2.5"));
    EXPECT_TRUE(eval_ok("::std::vector<double>::clear vd"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<double>::size $vd] == 0}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorDoubleToString) {
    EXPECT_TRUE(eval_ok("set vd [::std::vector<double>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<double>::clear vd"));
    EXPECT_TRUE(eval_ok("::std::vector<double>::push vd 1.5 2.5 3.5"));
    EXPECT_TRUE(eval_ok("set vd_str [format %s $vd]"));
    EXPECT_TRUE(eval_ok("expr {$vd_str eq {1.5 2.5 3.5}}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorDoubleEmpty) {
    EXPECT_TRUE(eval_ok("set vd [::std::vector<double>::new]"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<double>::empty $vd]}"));
    EXPECT_TRUE(expect_result_true());
}

// VectorHeap<string> tests
TEST_F(ContainersTest, VectorStringCreate) {
    EXPECT_TRUE(eval_ok("set vs [::std::vector<string>::new]"));
}

TEST_F(ContainersTest, VectorStringPush) {
    EXPECT_TRUE(eval_ok("set vs [::std::vector<string>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<string>::push vs hello world test"));
}

TEST_F(ContainersTest, VectorStringSize) {
    EXPECT_TRUE(eval_ok("set vs [::std::vector<string>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<string>::push vs foo bar baz"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<string>::size $vs] == 3}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorStringAt) {
    EXPECT_TRUE(eval_ok("set vs [::std::vector<string>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<string>::push vs apple banana cherry"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<string>::at $vs 1] eq \"banana\"}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorStringSet) {
    EXPECT_TRUE(eval_ok("set vs [::std::vector<string>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<string>::push vs apple banana cherry"));
    EXPECT_TRUE(eval_ok("::std::vector<string>::set vs 1 changed"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<string>::at $vs 1] eq \"changed\"}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorStringPop) {
    EXPECT_TRUE(eval_ok("set vs [::std::vector<string>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<string>::push vs alpha beta gamma"));
    EXPECT_TRUE(eval_ok("set vsp [::std::vector<string>::pop vs]"));
    EXPECT_TRUE(eval_ok("expr {$vsp eq \"gamma\"}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorStringClear) {
    EXPECT_TRUE(eval_ok("set vs [::std::vector<string>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<string>::push vs one two"));
    EXPECT_TRUE(eval_ok("::std::vector<string>::clear vs"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<string>::size $vs] == 0}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorStringToString) {
    EXPECT_TRUE(eval_ok("set vs [::std::vector<string>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<string>::clear vs"));
    EXPECT_TRUE(eval_ok("::std::vector<string>::push vs hello world"));
    EXPECT_TRUE(eval_ok("set vs_str [format %s $vs]"));
    EXPECT_TRUE(eval_ok("expr {$vs_str eq {hello world}}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorStringAtOutOfRange) {
    EXPECT_TRUE(eval_ok("set vs [::std::vector<string>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<string>::push vs test"));
    EXPECT_TRUE(eval_error("::std::vector<string>::at $vs 10"));
}

TEST_F(ContainersTest, VectorStringEmpty) {
    EXPECT_TRUE(eval_ok("set vs [::std::vector<string>::new]"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<string>::empty $vs]}"));
    EXPECT_TRUE(expect_result_true());
}

// VectorShared tests
TEST_F(ContainersTest, VectorSharedCreate) {
    EXPECT_TRUE(eval_ok("set vsh [::std::vector<shared>::new]"));
}

TEST_F(ContainersTest, VectorSharedPush) {
    EXPECT_TRUE(eval_ok("set vsh [::std::vector<shared>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<shared>::push vsh 1 2"));
}

TEST_F(ContainersTest, VectorSharedSize) {
    EXPECT_TRUE(eval_ok("set vsh [::std::vector<shared>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<shared>::push vsh 1 2 3"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<shared>::size $vsh] == 3}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorSharedAt) {
    EXPECT_TRUE(eval_ok("set vsh [::std::vector<shared>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<shared>::push vsh 1 2 3"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<shared>::at $vsh 2] == 3}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorSharedPop) {
    EXPECT_TRUE(eval_ok("set vsh [::std::vector<shared>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<shared>::push vsh 1 2 3"));
    EXPECT_TRUE(eval_ok("set vshp [::std::vector<shared>::pop vsh]"));
    EXPECT_TRUE(eval_ok("expr {$vshp == 3}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorSharedList) {
    EXPECT_TRUE(eval_ok("set vsh [::std::vector<shared>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<shared>::push vsh 1 2"));
    EXPECT_TRUE(eval_ok("set vshl [::std::vector<shared>::list $vsh]"));
    EXPECT_TRUE(eval_ok("expr {[llength $vshl] == 2}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorSharedClear) {
    EXPECT_TRUE(eval_ok("set vsh [::std::vector<shared>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<shared>::push vsh 1 2 3"));
    EXPECT_TRUE(eval_ok("::std::vector<shared>::clear vsh"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<shared>::size $vsh] == 0}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorSharedReserve) {
    EXPECT_TRUE(eval_ok("set vsh [::std::vector<shared>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<shared>::reserve vsh 512"));
}

TEST_F(ContainersTest, VectorSharedToString) {
    EXPECT_TRUE(eval_ok("set vsh [::std::vector<shared>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<shared>::clear vsh"));
    EXPECT_TRUE(eval_ok("::std::vector<shared>::push vsh 1 2"));
    EXPECT_TRUE(eval_ok("set vsh_str [format %s $vsh]"));
    EXPECT_TRUE(eval_ok("expr {$vsh_str eq {1.0 2.0}}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorSharedEmpty) {
    EXPECT_TRUE(eval_ok("set vsh [::std::vector<shared>::new]"));
    EXPECT_TRUE(eval_ok("expr {[::std::vector<shared>::empty $vsh]}"));
    EXPECT_TRUE(expect_result_true());
}

TEST_F(ContainersTest, VectorSharedCopyIsolation) {
    EXPECT_TRUE(eval_ok("set vsh [::std::vector<shared>::new]"));
    EXPECT_TRUE(eval_ok("::std::vector<shared>::push vsh 1 2"));
    EXPECT_TRUE(eval_ok("set vsh_copy $vsh"));
    EXPECT_TRUE(eval_ok("::std::vector<shared>::push vsh 3"));
    EXPECT_TRUE(eval_ok(
        "expr {[::std::vector<shared>::list $vsh] eq {1.0 2.0 3.0} && "
        "[::std::vector<shared>::list $vsh_copy] eq {1.0 2.0}}"));
    EXPECT_TRUE(expect_result_true());
}

}  // namespace

GTEST_API_ int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
