# Context switching function:
# thread_start(old,new)
.globl thread_start

thread_start:
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

    jmp thread_wrap

