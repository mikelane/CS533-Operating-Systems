/* 
 * CS533 Assignment 5
 * Spinlock tester
 * Author: Kendall Stewart <kstew2@cs.pdx.edu>
 * 
 * Put your spinlock implementation in the functions below.
 *
 * This program hammers on a shared counter and array with five kernel threads,
 * and runs the test 100 times. If any test fails, it will output the contents
 * of the shared array after the failed test.  If all tests succeed, "success!"
 * will be printed.
 *
 * NOTE 1:
 * Due to the non-deterministic nature of parallel programs, there is a
 * not-insignificant chance of a false positive result. I suggest running the
 * program a few times to verify the result if it outputs "success!"
 * 
 * NOTE 2:
 * This is a standalone file that does not depend on your scheduler. It should
 * be compiled independently and linked against the libatomic_ops library using
 * the instructions on the course website.
 */
 
#define _GNU_SOURCE
#include <sched.h>
#include <atomic_ops.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void spinlock_lock(AO_TS_t * lock) {
  // your implementation here
}
void spinlock_unlock(AO_TS_t * lock) {
  // your implementation here
}

int shared_counter;
int shared_array[50];
AO_TS_t lock = AO_TS_INITIALIZER;

int increment(void * arg) {
  int i;
  for(i = 0; i < 10; ++i) {
    spinlock_lock(&lock);
    shared_counter++;
    shared_array[shared_counter] = shared_counter;
    spinlock_unlock(&lock);
  }

  *(int*)arg = 1;

  while(1) {}
}

int main(void) {
  int num_tests = 100;
  int test_no;
  int success = 1;

  for(test_no = 0; test_no < num_tests; ++test_no) {
    // initialize shared data
    shared_counter = 0;
    memset(shared_array, 0, 50 * sizeof(int));

    int i;
    void * child_stacks[5];
    int finished_flags[5] = {0};

    // create thread stacks
    for(i = 0; i < 5; ++i) {
      child_stacks[i] = malloc(4096) + 4096;
    }

    // clone threads
    for(i = 0; i < 5; ++i) {
      clone(increment, child_stacks[i], 
            CLONE_THREAD | CLONE_VM | CLONE_SIGHAND, &finished_flags[i]);
    }

    // wait for all threads to complete
    // (this method of constructing a barrier is similar to Dekker's Algorithm)
    int all_finished;
    do {
      all_finished = 1;
      for(i = 0; i < 5; ++i) {
        all_finished = all_finished && finished_flags[i];
      } 
    } while(!all_finished);

    // free thread stacks
    for(i = 0; i < 5; ++i) {
      free(child_stacks[i] - 4096);
    }
    
    // check result of this test
    for(i = 0; i < 50; ++i) {
      if(shared_array[i] != i) {
        success = 0;
        break;
      }
    }
    if(success) { 
      printf("o");
      fflush(stdout);
    } else {
      printf("x");
      break;
    }
  }

  // check result of all tests
  if(success) {
    printf("\nsuccess!\n");
  } else {
    printf("\nfailure on test %d of %d. contents are:\n", test_no, num_tests);
    int i;
    for(i = 0; i < 50; ++i) {
      printf("%d\n", shared_array[i]);
    }
  }

  return 0;
}

