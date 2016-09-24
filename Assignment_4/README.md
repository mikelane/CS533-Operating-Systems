# Assignment 4: Synchronization

**Due Friday, November 20th**

## Overview

### Mutual Exclusion

Have a look at the following [demo program](demo/demo1.c). Does this program require explicit synchronization? There's a shared variable, `buf`, accessed both by the thread executing `read_file` and the main thread. However, there is no danger of any concurrent accesses, since our system is running cooperatively on a single kernel thread, and neither thread calls `yield` during its operation.

Thus `buf` is implicitly protected by mutual exclusion. So, what output can we expect from this program?

Since our implementation of `thread_fork` causes the child thread to run immediately, `read_file` will read random bytes into the buffer, finish, and `yield`s back to main, which will then calls `zero_buf`. This fills the buffer with all zeroes. When the contents of the buffer are printed, it will be all zeroes.

Now have a look at this [identical version](demo/demo2.c), where the call to `read` is replaced with [`read_wrap`](/Assignment_3).

Assuming again that `thread_fork` causes the child thread to run immediately, then `read_wrap` will run first and `yield` after starting the asynchronous read. Then `zero_buf` will run. `zero_buf` doesn't yield, so it should run to completion, and after it completes we should expect that the buffer is filled with all zeroes.

Next, `scheduler_end` will run, which will `yield` back and forth with `read_wrap` until its read is complete. Once the read is complete, the buffer should be filled with all random bytes. So we should expect the program to print out a buffer filled with entirely random bytes.

Go ahead and compile and link `demo2.c` with your scheduler. What do you notice? Here is an [section of output](demo/demo2output) from a sample run of `demo2.c`. Contrary to our expectations, the zeroes are overlapped with the random bytes! Somehow, the execution of both threads became concurrent, and the data was corrupted as a result.

How did our system achieve concurrency with only a single kernel thread? Well, the asynchronous I/O call at the heart of `read_wrap` requires that work is done in the background to service the I/O request; this work may be done on the same CPU by switching out the current kernel thread, or on another CPU. The result is the same: overlapping accesses to the same buffer.

It's worth noting that this behavior depends on an implementation of `read_wrap` that passes `buf` directly to `aio_read`. An implementation of `read_wrap` that creates a local buffer and then copies into `buf` after the read is complete would not have this issue. However, this would require performing a potentially expensive array copy.

It's better to optimize for the common case, where there are no concurrent accesses, and pass in `buf` directly. However, this invalidates our guarantee that we can achieve mutual exclusion simply by not calling `yield` within a critical section. Thus, our system needs some explicit mechanism for mutual exclusion. In this assignment, we'll add blocking mutex locks.

### Conditional Waiting

Another thing missing from our system so far has been a way for threads to wait until some condition becomes true, such as some quantity of a shared resource becoming available. Without concurrency, this can be done at the application level by using shared global variables and busy-waiting, but it would be more practical to give the user a structured interface, that incorporates blocking, such as condition variables (see [this slide set](http://web.cecs.pdx.edu/~walpole/class/cs533/fall2015/slides/2.pdf) from the start of the term).

## Design Choices

### Blocking vs. Busy-waiting (revisited)

Recall in the [last assignment](/Assignment_3#managing-notification-signals-vs-polling) that we decided in favor of busy-waiting instead of blocking while waiting for I/O completion. This is because I/O completion is an external event whose completion time is unknown, and a combination of polling and busy-waiting was the simplest solution for our purposes.

However, a mutex becoming free is _not_ an external event; it is a synchronous operation that is performed by a thread. Busy-waiting for a mutex to become free is always wasted work, because there's no chance of progress: the only thing that can wake up the spinning thread is another user-level thread, who will not be making any progress while the spinner is running.

The same issue applies to condition variables. Thus, in this assignment, we'll create true blocking mutex locks and condition variables.

### Consequences of Blocking

In the absence of true blocking, all previous versions of our scheduler have maintained the invariant that there is always at least one runnable thread. Thus, as long as the current thread enqueues itself on the ready list before dequeuing a thread, the ready list will never run empty. An empty ready list is a very undesirable situation, because control flow has nowhere sane to go!

Once we introduce true blocking, we also introduce the possibility that all threads might become blocked, which could result in the ready list becoming empty. Consider the case where there is one runnable thread, and it blocks -- it would not enqueue itself on the ready list, thus there would be nothing to dequeue (and nothing to switch to). In our implementation, this would cause our program to crash.

You might notice, however, that this situation is a fatal error with a precise cause: since our system does not feature asynchronous software interrupts (signals), if a thread blocks and there are no other runnable threads, then there are no events that can come to that thread's "rescue" and cause it to unblock. This is an unrecoverable deadlock.

If our system did use signals (for instance to deal with I/O completion events instead of polling), then we would need to introduce an "idle thread". While there was still work to do, the idle thread would simply call `yield`. If the ready list became empty, the idle thread would wait for a signal to arrive (see [`pause(2)`](http://linux.die.net/man/2/pause)).

## Implementation

1.  Modify your `yield` routine to prevent a thread whose status is `BLOCKED` from enqueuing itself on the ready list. Since our system does not use signals, your implementation should reflect the fact that it is a fatal error to block when the ready list is empty.

2.  Design and implement mutex lock that provides mutual exclusion between the lock and unlock primitives by blocking any thread that attempts to lock a mutex that is currently held.

    I suggest the following data structure:

           struct mutex {
             int held;
             struct queue waiting_threads;
           };

    And the following operations:

    *   `void mutex_init(struct mutex *)`
    *   `void mutex_lock(struct mutex *)`
    *   `void mutex_unlock(struct mutex *)`

    `mutex_init` should initialize all fields of `struct mutex`. `mutex_lock` should attempt to acquire the lock, and block the calling thread if the lock is already held. `mutex_unlock` should wake up a thread waiting for the lock by putting it back on the ready list, or free the lock if no threads are waiting.

3.  Design and implement a condition variable that has MESA semantics. I suggest the following data structure:

           struct condition {
             struct queue waiting_threads;
           };

    And the following operations:

    *   `void condition_init(struct condition *)`
    *   `void condition_wait(struct condition *, struct mutex *)`
    *   `void condition_signal(struct condition *)`
    *   `void condition_broadcast(struct condition *)`

    `condition_init` should initialize all fields of `struct condition`. `condition_wait` should unlock the supplied mutex and cause the thread to block. The mutex should be re-locked after the thread wakes up. `condition_signal` should wake up a waiting thread by adding it back on to the ready list. `condition_broadcast` should `signal` all waiting threads.

## Testing

1.  [This test program](counter_test.c) is designed to verify the semantics of your mutex lock, namely that a thread holding the lock has exclusive access to the critical section protected by the lock, and that all blocked threads eventually wake up and have a chance to run in the critical section.

    It creates 5 threads to each increment a shared counter 10 times, calling `yield` at a very inopportune time. If your mutex is working correctly, you will see the values 1 through 50, consecutively numbered. If your mutex does not work properly, the values printed out will be inconsistent.

2.  Test your condition variables by implementing `thread_join` using condition variables. This will require you to add a mutex and condition variable to the thread control block.

    `thread_join` should have the following prototype:

           void thread_join(struct thread*);

    To allow `thread_join` to be used in practice, you should modify `thread_fork` to return a pointer to the newly created thread's thread control block.

    [This test program](sort_test.c) will test your implementation of `thread_join` with a "parallel" mergesort procedure.

3.  Feel free to write any other tests you see fit!

## Discussion

Our mutex locks works because the lock variables themselves are implicitly protected by mutual exclusion -- the `lock` and `unlock` operations are "atomic" because they do not yield and should not be affected by asynchronous reading operations.

Your next assignment will be to add a second kernel thread to your user-level thread scheduler, to allow two user-level threads to run in parallel. How will this affect the operation of your blocking mutex lock? What additional mechanisms might we need in order to ensure that our programs are safe?

## What To Hand In

You should submit:

1.  All scheduler code so far. This should include:

    *   `switch.s`
    *   `queue.h`
    *   `queue.c`
    *   `scheduler.h`
    *   `scheduler.c`
    *   `async.c`
2.  If you modified `counter_test.c` or `sort_test.c`, turn in the modified versions. Also turn in any tests you wrote yourself.

3.  A brief written report, including:

    1.  A description of what you did and how you tested it.
    2.  Your response to the question in "Discussion", above.

Please submit your code files _as-is_; do not copy them into a Word document or PDF.  
Plain text is also preferred for your write-up.  
You may wrap your files in an archive such as a .tar file.

Email your submission to the TA at <u>kstew2 at cs.pdx.edu</u> on or before the due date. The subject line should be "CS533 Assignment 4".

## Need Help?

If you have any questions or concerns, or want clarification, feel free to [contact the TA](https://mikelane.github.io/CS533-Operating-Systems/) by coming to office hours or sending an email.

You may also send an email to the [class mailing list](https://mailhost.cecs.pdx.edu/mailman/listinfo/cs533). Your peers will see these emails, as will the TA and professor.
