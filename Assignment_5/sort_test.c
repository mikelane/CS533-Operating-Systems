#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "scheduler.h"

static int seq_threshold;

struct array {
  int * arr;
  int len;
};

void selection_sort(struct array * A) {
  int * arr = A->arr;
  int length = A->len;

  int i,j,min,temp;
  for(i = 0; i < length-1; ++i) {

    min = i;

    for(j = i+1; j < length; ++j) {
      if(arr[j] < arr[min]) {
        min = j;
      }
    }

    temp = arr[i];
    arr[i] = arr[min];
    arr[min] = temp;
  }
}

void merge(struct array * A, struct array * B) {
  int * arr1 = A->arr;
  int l1     = A->len;

  int * arr2 = B->arr;
  int l2     = B->len;

  int * result = malloc(sizeof(int) * (l1 + l2));

  int i = 0, j = 0, k = 0;

  while(i < l1 && j < l2) {
    if(arr1[i] < arr2[j]) {
      result[k++] = arr1[i++];
    } else {
      result[k++] = arr2[j++];
    }

  }

  if(i >= l1) {
    memcpy(result+k, arr2+j, sizeof(int) * (l2-j));
  } else if(j >= l2) {
    memcpy(result+k, arr1+i, sizeof(int) * (l1-i));
  }

  memcpy(arr1, result, sizeof(int) * (l1+l2));

  free(result);
}


void par_mergesort(void * arg) {
  struct array * A = (struct array*)arg;
  
  if(A->len <= seq_threshold) {
    selection_sort(A);
  }

  else {
    struct array left_half, right_half;

    if (A->len % 2 == 0) {
      left_half.len  = right_half.len = A->len/2;
    } else {
      left_half.len  = A->len/2;
      right_half.len = A->len/2 + 1;
    }

    left_half.arr  = A->arr;
    right_half.arr = A->arr + left_half.len;

    struct thread * left_t  = thread_fork(par_mergesort, &left_half);
    struct thread * right_t = thread_fork(par_mergesort, &right_half);

    thread_join(left_t);
    thread_join(right_t);    

    merge(&left_half, &right_half);
  }
}

struct array * rand_array(int size) {
  struct array * result = malloc(sizeof(struct array));
  result->arr = malloc(sizeof(int) * size);
  result->len = size;

  int i;
  srand(time(NULL));
  for(i = 0; i < size; ++i) {
    result->arr[i] = rand() % size;
  }
  return result;
}

const char* check_sort(struct array * A) {
  int i, is_sorted = 1;
  for(i = 0; i < A->len-1; ++i) {
    if(A->arr[i] > A->arr[i+1]) {
      is_sorted = 0;
      break;
    }
  }
  return is_sorted ? "sorted!" : "not sorted!";
}

int main(int argc, char ** argv) {
  if (argc < 4) {
    fprintf(stderr, "usage: %s num_kthreads array_size seq_threshold\n", argv[0]);
    exit(1);
  }

  int num_kthreads = atoi(argv[1]);
  int array_size   = atoi(argv[2]);
  seq_threshold    = atoi(argv[3]);

  struct array * A = rand_array(array_size);

  scheduler_begin(num_kthreads);

  printf("before sort: %s\n", check_sort(A));
  par_mergesort(A);
  printf("after sort: %s\n", check_sort(A));

  scheduler_end();
  return 0;
}
