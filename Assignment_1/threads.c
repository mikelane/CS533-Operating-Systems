#include "threads.h"

int factorial(int n) {
  return n == 0 ? 1 : n * factorial(n-1);
}

void fun_with_threads(void * arg) {
  printf("** In fun_with_threads() function **\n");
  int i = 0;
  int n = *(int *) arg;
  for(;i < 10; ++i) {
    printf("** Loop %d **\n", i);
    printf("** %d! = %d **\n", n+i, factorial(n+i));
    printf("** Yielding from fun_with_threads() **\n");
    yield();
  }
}

void thread_wrap() {
  printf("*** Starting thread_wrap ***\n");
  current_thread->initial_function(current_thread->initial_argument);
  printf("*** Yeilding from thread_wrap() ***\n");
  yield();
}

void yield() {
  printf("\n**** In yield function ****\n");
  thread * temp = current_thread;
  current_thread = inactive_thread;
  inactive_thread = temp;
  printf("**** Calling thread_switch ****\n");
  thread_switch(inactive_thread, current_thread);
}
