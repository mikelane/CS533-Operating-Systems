## M:N Cooperative Round-Robin Scheduler

One potential design makes almost no changes to the existing scheduler. There is still just one ready queue (protected with a spinlock), and each new processor has an idle thread which continuously yields, pulling work off the ready queue if it is available. User-level threads are not preempted involuntairly, though their underlying kernel thread might be.

In the animation below, green rectangles represent running threads, and gray rectangles represent ready threads. The infinity symbol (∞) represents an idle thread whose sole job is to `yield` in an infinite loop.

[figure 2](figure2.gif)
