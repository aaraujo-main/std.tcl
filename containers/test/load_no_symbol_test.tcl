#!/usr/bin/env tclsh

if {$argc != 1} {
    puts stderr "usage: load_no_symbol_test.tcl /absolute/path/to/stdcontainers.so"
    exit 2
}

set lib [lindex $argv 0]

if {[catch {load $lib} err]} {
    puts stderr "load without symbol failed: $err"
    exit 1
}

if {[catch {package require std::containers} pkgErr]} {
    puts stderr "package require std::containers failed after load: $pkgErr"
    exit 1
}

if {[llength [info commands ::std::vector::new]] != 1} {
    puts stderr "vector constructor was not registered"
    exit 1
}

set v [::std::vector::new]
::std::vector::push v a b
if {[::std::vector::at $v 1] ne "b"} {
    puts stderr "vector roundtrip failed"
    exit 1
}

set v2 [::std::vector::new.shared]
::std::vector::push v2 c d
if {[::std::vector::at $v2 1] ne "d"} {
    puts stderr "shared vector roundtrip failed"
    exit 1
}
puts "v2: $v2"
exit 0
