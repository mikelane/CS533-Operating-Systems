#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scheduler.h"

int shared_count = 0;
int shared_array[50] = {0};

struct mutex m;

void increment_and_print(void * arg) {
  int i;
  for(i = 0; i < 10; ++i) {
    mutex_lock(&m);

    shared_array[shared_count] = shared_count;

    int temp = shared_count;
    yield();
    shared_count = temp + 1;

    printf("%s, %d\n", (char*)arg, shared_count);
    mutex_unlock(&m);
  }
}

int main(int argc, char**argv) {
  int i;
  char * name = "Thread-X";
  char * names[5];
  for(i = 0; i < 5; ++i) {
    names[i] = malloc(9);
    strcpy(names[i], name);
    names[i][7] = i + '1';
  }
  mutex_init(&m);

  scheduler_begin();

  for(i = 0; i < 5; ++i) {
    thread_fork(increment_and_print, names[i]);
  }

  scheduler_end();

  int array_consistent = 1;
  for(i = 0; i < 50; ++i) {
    if(shared_array[i] != i) {
      array_consistent = 0;
      break;
    }
  }

  printf(array_consistent ? "results are consistent.\n" 
                          : "results are inconsistent!\n");

  return 0;
}
