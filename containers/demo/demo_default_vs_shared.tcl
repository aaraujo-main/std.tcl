set here [file dirname [file normalize [info script]]]
load [file join $here .. build stdcontainers[info sharedlibextension]]

proc demo {isShared useShared} {
    puts "Demo: isShared=$isShared, useShared=$useShared"
    if {$isShared} {
        set vi1 [::std::vector<int>::new.shared]
        if {!$useShared} {
            ::std::vector<int>::from_shared vi1
        }
    } else {
        set vi1 [::std::vector<int>::new]
        if {$useShared} {
            ::std::vector<int>::make_shared vi1
        }
    }
    
    ::std::vector<int>::clear vi1
    ::std::vector<int>::push vi1 1 2 3
    set vi1_str [format %s $vi1]
    puts "$vi1_str"
    puts $vi1
    puts [expr {$vi1_str eq "1 2 3"}]

    set vi2 $vi1
    ::std::vector<int>::push vi1 4 5
    ::std::vector<int>::push vi2 6 7
    puts $vi1
    puts $vi2
}

demo 1 1
demo 1 0
demo 0 1
demo 0 0
