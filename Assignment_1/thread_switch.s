# Context switching function:
# thread_switch(old,new)
.globl thread_switch

thread_switch:
    pushq %rbx # Push
    pushq %rbp # all
    pushq %r12 # callee-save
    pushq %r13 # registers
    pushq %r14 # onto the
    pushq %r15 # stack

    # Save current stack pointer in old's TCB
    movq %rsp,(%rdi)

    # Load stack pointer from new's TCB into %rsp
    movq (%rsi),%rsp

    popq %r15 # Pop
    popq %r14 # all
    popq %r13 # callee-save
    popq %r12 # registers
    popq %rbp # from the
    popq %rbx # new stack (in reverse)

    ret

