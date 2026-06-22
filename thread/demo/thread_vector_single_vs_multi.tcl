set scriptDir [file dirname [file normalize [info script]]]
set packages {}
lappend packages [file join $scriptDir .. build stdthread[info sharedlibextension]]
lappend packages [file join $scriptDir ../.. containers build stdcontainers.so]

set init {
    foreach pkg $packages {
        load $pkg
    }
    proc multi10Vec {vVar start end} {
        upvar $vVar v
        for {set i $start} {$i < $end} {incr i} {
            std::vector<int>::set v $i [expr {$i * 10}]
        }
    }
}

eval $init

proc demo {N init packages numThreads} {
    puts "demo numThreads=$numThreads"
    set v [std::vector<int>::new.shared]
    for {set i 0} {$i < $N} {incr i} {
        std::vector<int>::push v $i
    }
    set t_total [time {
        if {$numThreads == 1} {
            set length [std::vector<int>::size $v]
            set t_thread [time {multi10Vec v 0 $length}]
            puts "t_thread main: $t_thread"
        } else {
            set threads {}
            for {set id 0} {$id < $numThreads} {incr id} {
                puts "\[DEBUG\] Spawning thread $id"
                set t [::std::thread::new {{{string init} {string packages} {int numThreads} {int id} v} {
                    eval $init
                    set length [std::vector<int>::size $v]
                    set start [expr {int($id * $length / $numThreads)}]
                    if {$id == $numThreads - 1} {
                        set end $length
                    } else {
                        set end [expr {int(($id + 1) * $length / $numThreads)}]
                    }
                    set t_thread [time {
                        multi10Vec v $start $end
                    }]
                    puts "t_thread $id: $t_thread"
                }} $init $packages $numThreads $id $v]
                lappend threads $t
            }
            foreach t $threads {
                ::std::thread::join $t
            }
            ::std::thread::invalidateStringRep v
        }
    }]
    puts "t_total: $t_total"
    puts [std::vector<int>::at $v 1]
    puts [std::vector<int>::at $v [expr {$N / 2}]]
    puts [std::vector<int>::at $v [expr {$N - 1}]]
    return $t_total
}

demo 10000000 $init $packages 1
demo 10000000 $init $packages 4
