# Assignment 1: Context Switching

**Due Date: Friday, October 7th**

## Overview

Throughout this project, we will be developing various parts of a user-level interface for programming with threads, similar to that described in Birrell's [An Introduction to Programming with Threads](http://web.cecs.pdx.edu/~walpole/class/cs533/papers/thread_intro.pdf), with some important differences that will become clear as the project progresses.

The first part of each assignment will be a bit of overview, followed by a discussion of the design choices one is faced with when designing and implementing a particular part of the threading interface. In this assignment, we will begin to implement the data structures that store information about the threads themselves, as well as some basic mechanisms for switching control flow between two different threads.

## Design Choices

### Data Structures

Since all threads in a particular program share an address space, we must decide where in that address space each part of the thread will live. Concretely, we need a data structure representing the thread's vital information (commonly called a "thread control block"), which the threading library will use to maintain and manage the state of each thread. We also need space for the thread's activation stack, which will be used by the compiler and hardware to manage local variables, function calls, and so forth.

In this project, the activation for each thread after the main thread (where the `main()` function starts) will live in the heap, while the main thread's stack will live in the stack area of the address space, as it usually does:

    Address space:
    |======================| High Addresses
    | Main Thread Stack  | |
    |                    v |
    |======================|
    | Heap:                |
    |----------------------|
    | Thread 1 Stack     | |
    |                    v |
    |----------------------|
    | Thread 2 Stack     | |
    |                    v |
    |----------------------|
    | etc ...              |
    |======================|
    | Data                 |
    |======================|
    | Text                 |
    |======================| Low Addresses

Something to think about: can you see any potential problems with this design? What happens if the main thread runs out of stack space? What about the subordinate threads? What could you do to prevent a stack overflow in either case?

Allowing the thread stacks to live in the heap allows the number of threads in the system to be flexible and unknown at compilation time. This implies that the thread control blocks must also live in the heap, so they can be dynamically created and destroyed.

### Context Switching

context switching refers to the control-flow jump that occurs when one thread gives up the CPU and another thread takes over. Like a function call, it involves pushing values on the system stack and manipulating CPU registers. Unlike a function call, which has explicit entry and exit points, a context switch can happen at any time, without warning. If a stack and set of registers represents everything you need to know about a control flow (its context), then the entire context must be saved before switching to another one.

There is no explicit way to do this at the C language level. Therefore, context switching must be directly implemented at the assembly language level. The core assembly mechanism we will use to switch between thread stacks is to change the value of the system stack pointer register (`%rsp` on the x86-64 machines we will be using).

## Implementation

### Step 1: Data Structures

Write a C data structure, `struct thread`, to represent a thread control block. For now, it should have at least these fields:

*   A stack pointer: `unsigned char* stack_pointer`
    *   Note: an `unsigned char` is a single byte, thus a variable of type `unsigned char*` is a pointer to an array of single bytes. However, I find it clearer to use a type without another meaning. If you want to be as clear as possible, you can add a type synonym with `typedef unsigned char byte`, and then make the type `byte*`.
*   A pointer to an initial function: `void (*initial_function)(void*)`
    *   Note: this creates a variable named `initial_function` that is a pointer to functions that take a single `void*` parameter and return no values. For more on function pointer syntax, [see here](http://www.cprogramming.com/tutorial/function-pointers.html).
*   A pointer to an initial argument: `void* initial_argument`
    *   `void*` variables (aka void pointers) are used to allow pointers to values of _any_ type to be passed in to a function. We use them here because it gives us the most general interface for defining initial functions for threads. For more on void pointers, [see here](http://theory.uwinnipeg.ca/programming/node87.html).

Write a short function that takes an argument of type `void*` and returns nothing. It can do whatever you like! Here is an example that shows off how the `void*` parameter might get used:

      int factorial(int n) {
        return n == 0 ? 1 : n * factorial(n-1);
      }

      void fun_with_threads(void * arg) {
        int n = *(int*) arg;
        printf("%d! = %d\n", n, factorial(n));
      }

Create a global pointer to a `struct thread` called `current_thread`, and write a short `main` program to allocate space for `current_thread` and for the thread's stack, then initialize each field of `current_thread`. Initializing the initial function is easy: just give the name of the function, e.g.:

      current_thread->initial_function = fun_with_threads;

The initial argument is a little more complicated, since you should allocate some data on the heap and then use a pointer to it, like so:

      int * p = malloc(sizeof(int));
      *p = 5;
      current_thread->initial_argument = p;

Notice that you don't have to explicitly cast `p` (which is type `int*`) to type `void*`. This is called "upcasting": going from a more specific type to a less specific type. However, in `fun_with_threads` above, `arg` must be explicitly cast _back_ to type `int*`. We know that in this case, `arg` is really a pointer to an integer, but the compiler doesn't. This is called "downcasting": going from a less specific type to a more specific type.

When you initialize the thread's stack pointer, recall that it needs to point to the _end_ of the allocated region, since stacks grow towards lower addresses. For example:

      current_thread->stack_pointer = malloc(STACK_SIZE) + STACK_SIZE;

What should the value of `STACK_SIZE` be? This is a tough question that depends on the application. Thankfully, modern machines have memory to burn, so let's make our stacks be 1 megabyte (1024 * 1024 bytes).

So far, we've crafted a really ugly way to call a function:

      current_thread->initial_function(current_thread->inital_argument);

We don't quite have threading yet. For that, we'll need to implement some more machinery.

### Step 2: Context Switching

The basic way to switch between two established contexts is relatively straightforward: save all of the registers on the current stack, switch stacks, and restore all the registers off the new stack. We'll call this operation `thread_switch`, with the following prototype (in your C file):

      void thread_switch(struct thread * old, struct thread * new);

And the following implementation (in a separate assembly file; [see here](asm_hints.md) for some assembly writing tips):

1.  Push all callee-save registers (`%rbx, %rbp, %r12-15`) onto the current stack.
2.  Save the current stack pointer (`%rsp`) in `old`'s thread control block.
3.  Load the stack pointer from `new`'s thread control block into `%rsp`.
4.  Pop all callee-save registers from the new stack.
5.  Return.

Note that this method switches between two established threads. What if we want to switch to a new thread? Presumably, this thread would have an empty stack, so step 4 would make no sense. Additionally, we need a way to start executing the initial function of the new thread. Let's call this other operation `thread_start`. It has a very similar prototype and implementation to `thread_switch`:

      void thread_start(struct thread * old, struct thread * new);

1.  Push all callee-save registers onto the current stack.
2.  Save the current stack pointer in `old`'s thread control block.
3.  Load the stack pointer from `new`'s thread control block into `%rsp`.
4.  Call the initial function of `new`.

Step 4 above is a little open ended, as there are a few ways to do it. We could load the new thread's initial argument into the first argument register (`%rdi`) and jump to the initial function. Or, we could jump to a C function that calls the initial function using C syntax. This is more desirable, for reasons that will become clear later. Let's call this C wrapper function `thread_wrap`. Right now, it only needs one line:

      void thread_wrap() {
        current_thread->initial_function(current_thread->inital_argument);
      }

We're almost ready to start up a new thread. You should have a `main` function that creates and sets up a `current_thread` TCB (thread control block). Keep in mind that at the time this data structure is allocated, it's not referring to the real current thread. This can be a little mind-bending, but it will become the current thread soon. Meanwhile, we will need a place to park the context of the (currently running) main thread, which will become inactive.

Create another global pointer to a `struct thread` called `inactive_thread`. Allocate space for this thread control block. You don't have to allocate space for its stack, or initialize its initial function or initial argument. Why not?

Okay, now that both the (soon to be) `current_thread` and the (soon to be) `inactive_thread` are set up, you're ready to call `thread_start` in `main`:

      thread_start(inactive_thread, current_thread);

You should see the output of your initial function... and then the program should crash with a `Segmentation fault`. This is the expected behavior at this stage! But why? We'll get to this in a moment, but first let's develop a way to use `thread_switch` to go back to the main thread before our subordinate thread finishes.

### Step 3: Yielding

Before we can call `thread_switch`, we'll need to swap the `current_thread` and `inactive_thread` pointers, so we maintain the condition that outside of setup and switching code, `current_thread` always refers to the thread control block of the thread that's currently running. We'll use the following function, a primitive form of `yield`:

      void yield() {
        struct thread * temp = current_thread;
        current_thread = inactive_thread;
        inactive_thread = temp;
        thread_switch(inactive_thread, current_thread);
      }

Experiment with calling `yield` to switch back and forth between the subordinate thread and the main thread. You might try putting a loop in each that allows you to go back and forth a fixed number of times. Notice that if the loop in the subordinate thread finishes first, the program crashes with a segmentation fault, as before. What happens if the loop in the main thread finishes first? Why?

### Step 4: Thread Termination

Back to our segmentation fault issue. What's going on? Well, when the initial function ends, it returns back to `thread_wrap`, which ends and then tries to return back to the function that called it. **But nothing called `thread_wrap`**. It was the first function executed on an empty stack, and you can't unwind an empty stack any further. What our thread needs is an exit point: a way to clean up the thread's state and switch back to some other task.

One thing we can do is to put a call to `yield` at the end of `thread_wrap`:

      void thread_wrap() {
        current_thread->initial_function(current_thread->inital_argument);
        yield();
      }

Now, when the thread's initial function finishes, no matter what it was, it will return to `thread_wrap`, which will call `yield`. The inactive main thread will be switched in, and execution will continue at the point of the last `thread_switch` or `thread_start` called by the main thread.

One other thing we might be tempted to do in `thread_wrap` is free memory associated with the thread, e.g. its activation stack and thread control block. Be careful, however! We cannot free memory associated with the thread while it is still running. Think about what we could do to delay the deallocation until it is safe.

## Discussion

Think about the answers to the following questions, and discuss them with your peers if you'd like.

1.  What would be an elegant way to create and manage an arbitrary number of threads?

2.  Can you think of any uses for even the very simple form of threading we've developed in this assignment? What extra features could we add to make it more useful?

3.  Memory management can be a challenge in threading systems. When is it safe to free a thread's stack? What about its thread control block? How would this change if we wanted to return results from our threads, or implement a `join` procedure? (see the [Birrell article](http://web.cecs.pdx.edu/~walpole/class/cs533/papers/thread_intro.pdf) for a description of `join`)

## What To Hand In

You should submit:

1.  All code you'd like to submit for this assignment.

2.  A brief written report, including:

    1.  A description of what you did and how you chose to test it.

    2.  Your responses to the discussion questions posed above. If feel like you got a good idea from a peer, make sure to cite them for it and try to give your own thoughts on their idea as well.

Please submit your code files _as-is_; do not copy them into a Word document or PDF.
Plain text is also preferred for your write-up.

Email your submission to [the TA](https://mikelane.github.io/CS533-Operating-Systems/) on or before the due date. The subject line should be "CS533 Assignment 1".

## Need Help?

If you have any questions or concerns, or want clarification, feel free to [contact the TA](https://mikelane.github.io/CS533-Operating-Systems/) by coming to office hours or sending an email.

You may also send an email to the [class mailing list](https://mailhost.cecs.pdx.edu/mailman/listinfo/cs533). Your peers will see these emails, as will the TA and professor.
