#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "scheduler.h"

#define BUF_SIZE 200000

char buf[BUF_SIZE];

void read_file(void * arg) {
  int fd = open((char*)arg, O_RDONLY);
  read_wrap(fd, buf, BUF_SIZE);
  close(fd);
}

void delay(int n) {
  int i;
  for(i = 0; i < n; i+=2) {
    i--;
  }
}

void zero_buf(void) {
  int i;
  for(i = 0; i < BUF_SIZE; ++i) {
    buf[i] = 0;
    delay(25);
  }
}

int main(void) {
  scheduler_begin();

  /* fork a thread to read from file to buffer */
  thread_fork(read_file, "/dev/urandom");

  /* zero out buffer */
  zero_buf();

  /* wait for threads to terminate */  
  scheduler_end();

  /* print contents of buffer */
  int i, allzeroes = 1;
  for(i = 0; i < BUF_SIZE; ++i) {
    printf("%d ", buf[i]);
    if(buf[i] != 0) { allzeroes = 0; }
  }
  printf("\n\n%s\n", allzeroes ? "all zeroes!" : "some nonzero!");

  return 0;
}

