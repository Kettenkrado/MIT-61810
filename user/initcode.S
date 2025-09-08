# Initial process that execs /init.
# This code runs in user space.

#include "syscall.h"

# exec(init, argv)
.globl start
start:
        la a0, init
        la a1, argv
        li a7, SYS_exec
        ecall
# this calls init, it will start the "shell" program for user.
# if exec fails, we just exit.
# init and argv are defined below.

# for(;;) exit();
exit:
        li a7, SYS_exit
        ecall
        jal exit
# in fact we do not care the return value of exit, 
# and we do not hope it to return.
# this only happens when some error occurs.

# char init[] = "/init\0";
init:
  .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
  .quad init
  .quad 0
