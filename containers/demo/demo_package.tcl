package require std 0.1.0

set v [::std::vector::new]
::std::vector::push v 1 2 3
puts [::std::vector::at $v 0]  ;# 1
puts [::std::vector::size $v] ;# 3
