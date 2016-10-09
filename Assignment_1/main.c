#include <stdio.h>
#include <stdlib.h>
#include "threads.h"

int main() {
  printf("-- Initializing variables ... ");
  int * p = malloc(sizeof(int));
  *p = 5;
  int j = 0;
  printf("DONE\n");

  printf("-- Allocating space on the heap for current and inactive threads ... ");
  current_thread = malloc(sizeof(thread));
  inactive_thread = malloc(sizeof(thread));
  printf("DONE\n");

  printf("-- Initializing the current_thread TCB ... ");
  current_thread->initial_function = fun_with_threads;
  current_thread->initial_argument = p;
  // Hang on to location of malloc'd stack pointer to make freeing easier
  byte * temp_sp = malloc(STACK_SIZE);
  // Set the thread's stack pointer to the top of the stack
  current_thread->stack_pointer = temp_sp + STACK_SIZE;
  printf("DONE\n\n");

  printf("* Calling thread_start from main() *\n");
  thread_start(inactive_thread, current_thread);
  for(; j < 10; ++j) {
    printf("* Yielding from main() *\n");
    yield();
    printf("* Back in main function's for loop. j: %d *\n", j);
  }

  printf("* Back in main *\n");

  printf("\n-- Cleaning up ... ");
  free(p);
  free(temp_sp);
  free(inactive_thread);
  free(current_thread);
  printf("DONE\n");

  return 0;
}
