# x86-64 Assembly Tips

*   Assembly code files can be compiled and linked with C code quite easily, e.g.: `gcc my_asm.s my_code.c`
*   `pushq %reg` pushes the value of `%reg` onto the stack.
*   `popq %reg` pops a value off the stack and stores it in `%reg`.
*   `%rdi` and `%rsi` store the first and second arguments to a function call, respectively.
*   `movq %reg1 %reg2` stores the value of `%reg1` in `%reg2`.
*   If `%reg2` holds a memory address (e.g. a pointer), then `movq %reg1 (%reg2)` stores the 8-byte value of `%reg1` at the memory address in `%reg2`.
*   Likewise, `movq (%reg2) %reg1` loads the 8-byte value at the memory address in `%reg2` into `%reg1`.
*   If `%reg` holds a pointer to a C struct, whose fields are 8 bytes wide, the first field's value is at `(%reg)`, the second field's value is at `8(%reg)`, and so forth.
*   If `%reg` holds a C function pointer, `jmp *%reg` transfers control to the beginning of that function.
*   If `f` is the defined name of a C function, `jmp f` will transfer control to the beginning of that function.
*   `%rax` can safely be used as a scratch (temporary) register if needed.
    Other registers may need to be saved before using, and restored afterwards.

## Example: Swap Routine

    swap.s:

        # Inline comment
        /* Block comment */

        # void swap(long * x, long * y);

        .globl swap

        swap:
          pushq %rbx           # callee-save
          movq (%rdi), %rax    # %rax = *x
          movq (%rsi), %rbx    # %rbx = *y
          movq  %rbx, (%rdi)   # *x = %rbx
          movq  %rax, (%rsi)   # *y = %rax
          popq  %rbx           # callee-restore
          ret                  # return

    test.c:

        #include <stdio.h>

        void swap(long * x, long * y);

        int main(void) {
          long x = 5;
          long y = 6;
          printf("x = %ld, y = %ld\n", x, y); // x = 5, y = 6

          swap(&x, &y);
          printf("x = %ld, y = %ld\n", x, y); // x = 6, y = 5

          return 0;
        }

If you're interested in learning more about x86-64 assembly, check out these resources:

*   [Primer from Univ. of Chicago](http://www.classes.cs.uchicago.edu/archive/2009/spring/22620-1/docs/handout-03.pdf)
*   [x86-64 ABI](http://refspecs.linuxfoundation.org/elf/x86_64-abi-0.95.pdf)
