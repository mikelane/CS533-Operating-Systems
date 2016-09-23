/* CS533 Assignment 5
 * threadmap.c: Kernel thread -> user thread lookup
 * 
 * To use this file, add the following lines to your scheduler.h:
 *   extern struct thread * get_current_thread();
 *   extern void set_current_thread(struct thread*);

 * And optionally:
 *   #define current_thread (get_current_thread())
 * 
 */

/*********** uncomment this line once you have completed part 2! **************/
// #define PART2COMPLETE
/******************************************************************************/

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdlib.h>

#ifdef PART2COMPLETE
#include <atomic_ops.h>
#endif

#include "scheduler.h"

#define TABLE_SIZE 7

struct mapping {
  pid_t kernel_tid;
  struct thread * t;

  struct mapping * next;
};

struct mapping * table[TABLE_SIZE];

#ifdef PART2COMPLETE
AO_TS_t table_lock = AO_TS_INITIALIZER;
#endif

void set_current_thread(struct thread * t) {
  #ifdef PART2COMPLETE
  spinlock_lock(&table_lock);
  #endif

  // get current kernel thread id
  pid_t kernel_tid = syscall(SYS_gettid);

  // hash kernel tid
  int idx = kernel_tid % TABLE_SIZE;

  // search for existing mapping
  struct mapping * temp = table[idx];

  while(temp) {
    // if one exists, update it and return
    if(temp->kernel_tid == kernel_tid) {
      temp->t = t;

      #ifdef PART2COMPLETE
      spinlock_unlock(&table_lock);
      #endif

      return;
    }
    temp = temp->next;
  }

  // no mapping exists; create a new mapping
  temp = malloc(sizeof(struct mapping));

  temp->kernel_tid = kernel_tid;
  temp->t = t;

  temp->next = table[idx];
  table[idx] = temp;

  #ifdef PART2COMPLETE
  spinlock_unlock(&table_lock);
  #endif
}


struct thread * get_current_thread() {
  #ifdef PART2COMPLETE
  spinlock_lock(&table_lock);
  #endif
  
  pid_t kernel_tid = syscall(SYS_gettid);

  int idx = kernel_tid % TABLE_SIZE;

  struct thread * ret = NULL;
  struct mapping * temp = table[idx];

  while(temp) {
    if(temp->kernel_tid == kernel_tid) {
      ret = temp->t;
      break;
    }
    temp = temp->next;
  }

  #ifdef PART2COMPLETE
  spinlock_unlock(&table_lock);
  #endif
  return ret; 

}
