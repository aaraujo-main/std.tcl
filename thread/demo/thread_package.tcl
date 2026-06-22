
set init {
    package require std 0.1.0
    proc multi10Vec {vVar start end} {
        upvar $vVar v
        for {set i $start} {$i < $end} {incr i} {
            std::vector<int>::set v $i [expr {$i * 10}]
        }
    }
}

eval $init

proc demo {N init numThreads} {
    puts "demo numThreads=$numThreads"
    set v [std::vector<int>::new.shared]
    for {set i 0} {$i < $N} {incr i} {
        std::vector<int>::push v $i
    }
    set t_total [time {
        if {$numThreads == 1} {
            set length [std::vector<int>::size $v]
            set t_thread [time {multi10Vec v 0 $length}]
        } else {
            set threads {}
            for {set id 0} {$id < $numThreads} {incr id} {
                set t [::std::thread::new {{{string init} {int numThreads} {int id} v} {
                    eval $init
                    set length [std::vector<int>::size $v]
                    set start [expr {int($id * $length / $numThreads)}]
                    if {$id == $numThreads - 1} {
                        set end $length
                    } else {
                        set end [expr {int(($id + 1) * $length / $numThreads)}]
                    }
                    multi10Vec v $start $end
                }} $init $numThreads $id $v]
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
}

demo 10000000 $init 1
demo 10000000 $init 4
