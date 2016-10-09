#include <stdio.h>

#ifndef P1_THREADS_H
#define P1_THREADS_H

typedef unsigned char byte;

typedef struct thread {
  byte * stack_pointer;
  void (*initial_function) (void *);
  void * initial_argument;
} thread;

enum {STACK_SIZE = 1024 * 1024};

thread * current_thread;
thread * inactive_thread;

int factorial (int);
void fun_with_threads(void *);

void thread_switch(thread * old, thread * new);
void thread_start(thread * old, thread * new);
void thread_wrap();
void yield();

#endif //P1_THREADS_H
