#!/usr/bin/env tclsh

# Benchmark harness for comparing std::containers against Tcl built-ins.
# Compares:
# - ::std::vector operations vs Tcl list operations
# - ::std::unordered_map operations vs Tcl dict operations
# Includes reserve-based cases to isolate reallocation cost.
#
# Usage:
#   tclsh benchmarks/becnhmarks.tcl --lib /absolute/path/to/stdcontainers.so
#
# If --lib is omitted, script tries package require std::containers.

proc parse_args {argv} {
    set lib ""
    set i 0
    while {$i < [llength $argv]} {
        set a [lindex $argv $i]
        if {$a eq "--lib"} {
            incr i
            if {$i >= [llength $argv]} {
                error "missing value for --lib"
            }
            set lib [lindex $argv $i]
        } else {
            error "unknown argument: $a"
        }
        incr i
    }
    return $lib
}

proc ensure_loaded {lib} {
    if {$lib ne ""} {
        load $lib
        return
    }
    if {[catch {package require std::containers} err]} {
        error "failed to load std::containers: $err"
    }
}

proc bench {name script iterations} {
    set micros [lindex [time $script $iterations] 0]
    set per_op [expr {$micros / double($iterations)}]
    puts [format "%-32s %12.3f us/op (%d iters)" $name $per_op $iterations]
    return $per_op
}

proc compare {name_a value_a name_b value_b} {
    if {$value_b == 0.0} {
        puts [format "  %-18s vs %-18s ratio: n/a" $name_a $name_b]
        return
    }
    set ratio [expr {$value_a / $value_b}]
    puts [format "  %-18s vs %-18s ratio: %.3fx" $name_a $name_b $ratio]
}

set lib [parse_args $argv]
ensure_loaded $lib

puts "== std::containers benchmark: std containers vs Tcl built-ins =="
puts ""
puts "Notes:"
puts "- std container commands cross Tcl->C++ boundaries for each operation."
puts "- list/dict are native Tcl internals and avoid command-call overhead."
puts "- reserve() cases isolate growth/reallocation costs from dispatch costs."
puts ""

set outerIters 200
set itemCount 500

set ITEMS {}
set KEYS {}
set VALUES {}
for {set i 0} {$i < $itemCount} {incr i} {
    lappend ITEMS $i
    lappend KEYS "k$i"
    lappend VALUES "v$i"
}
set MID_KEY [lindex $KEYS [expr {$itemCount / 2}]]

puts "-- Vector vs Tcl list --"
set vector_push_script {
    set v [::std::vector::new]
    foreach item $::ITEMS {
        ::std::vector::push v $item
    }
}

set vector_push_reserved_script {
    set v [::std::vector::new]
    ::std::vector::reserve v $::itemCount
    foreach item $::ITEMS {
        ::std::vector::push v $item
    }
}

set list_push_script {
    set l {}
    foreach item $::ITEMS {
        lappend l $item
    }
}

set vector_read_script {
    set v [::std::vector::new]
    ::std::vector::reserve v $::itemCount
    foreach item $::ITEMS {
        ::std::vector::push v $item
    }
    set x [::std::vector::at $v [expr {$::itemCount / 2}]]
}

set list_read_script {
    set l {}
    foreach item $::ITEMS {
        lappend l $item
    }
    set x [lindex $l [expr {$::itemCount / 2}]]
}

set vector_push [bench "std::vector push" [list apply [list {} $vector_push_script]] $outerIters]
set vector_push_reserved [bench "std::vector push(reserve)" [list apply [list {} $vector_push_reserved_script]] $outerIters]
set list_push [bench "tcl list lappend" [list apply [list {} $list_push_script]] $outerIters]
set vector_read [bench "std::vector at(read)" [list apply [list {} $vector_read_script]] $outerIters]
set list_read [bench "tcl list lindex(read)" [list apply [list {} $list_read_script]] $outerIters]

compare "std::vector push" $vector_push "tcl list lappend" $list_push
compare "std::vector push(reserve)" $vector_push_reserved "tcl list lappend" $list_push
compare "std::vector at" $vector_read "tcl list lindex" $list_read
puts ""

puts "-- Unordered map vs Tcl dict --"
set map_put_script {
    set m [::std::unordered_map::new]
    foreach k $::KEYS v $::VALUES {
        ::std::unordered_map::put m $k $v
    }
}

set map_put_reserved_script {
    set m [::std::unordered_map::new]
    ::std::unordered_map::reserve m $::itemCount
    foreach k $::KEYS v $::VALUES {
        ::std::unordered_map::put m $k $v
    }
}

set dict_put_script {
    set d [dict create]
    foreach k $::KEYS v $::VALUES {
        dict set d $k $v
    }
}

set map_get_script {
    set m [::std::unordered_map::new]
    ::std::unordered_map::reserve m $::itemCount
    foreach k $::KEYS v $::VALUES {
        ::std::unordered_map::put m $k $v
    }
    set y [::std::unordered_map::get $m $::MID_KEY]
}

set dict_get_script {
    set d [dict create]
    foreach k $::KEYS v $::VALUES {
        dict set d $k $v
    }
    set y [dict get $d $::MID_KEY]
}

set map_put [bench "std::unordered_map put" [list apply [list {} $map_put_script]] $outerIters]
set map_put_reserved [bench "std::unordered_map put(reserve)" [list apply [list {} $map_put_reserved_script]] $outerIters]
set dict_put [bench "tcl dict set" [list apply [list {} $dict_put_script]] $outerIters]
set map_get [bench "std::unordered_map get" [list apply [list {} $map_get_script]] $outerIters]
set dict_get [bench "tcl dict get" [list apply [list {} $dict_get_script]] $outerIters]

compare "std::unordered_map put" $map_put "tcl dict set" $dict_put
compare "std::unordered_map put(reserve)" $map_put_reserved "tcl dict set" $dict_put
compare "std::unordered_map get" $map_get "tcl dict get" $dict_get
puts ""

puts "Done. Lower us/op is faster. Ratio > 1.0 means std container path is slower than Tcl baseline."
