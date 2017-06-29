Fib:
cpyargs 4
fget 0
push 3
more
jumpifnot L0
push L1
fget 0
push 1
sub
finc 4
jump Fib
L1:
fdec 4
push L2
fget 0
push 2
sub
finc 4
jump Fib
L2:
fdec 4
add
ireturn
L0:
push 1
ireturn
return
Main:
push L3
push 30
jump Fib
L3:
set 510
get 510
ccall 1
return
