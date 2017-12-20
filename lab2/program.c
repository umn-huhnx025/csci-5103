/*
Do not modify this file.
Make all of your changes to main.c instead.
*/

#include "program.h"

#include <stdio.h>
#include <stdlib.h>

static int compare_bytes(const void *pa, const void *pb) {
  int a = *(char *)pa;
  int b = *(char *)pb;

  if (a < b) {
    return -1;
  } else if (a == b) {
    return 0;
  } else {
    return 1;
  }
}

void focus_program(char *data, int length) {
  int total = 0;
  int i, j;

  srand(38290);

  for (i = 0; i < length; i++) {
    data[i] = 0;
  }

  for (j = 0; j < 100; j++) {
    int start = rand() % length;
    int size = 25;
    for (i = 0; i < 100; i++) {
      data[(start + rand() % size) % length] = rand();
    }
  }

  for (i = 0; i < length; i++) {
    total += data[i];
  }

#ifndef BENCHMARK
  printf("focus result is %d\n", total);
#endif
}

void sort_program(char *data, int length) {
  int total = 0;
  int i;

  srand(4856);

  for (i = 0; i < length; i++) {
    data[i] = rand();
  }

  qsort(data, length, 1, compare_bytes);

  for (i = 0; i < length; i++) {
    total += data[i];
  }
#ifndef BENCHMARK
  printf("sort result is %d\n", total);
#endif
}

void scan_program(char *cdata, int length) {
  unsigned i, j;
  unsigned char *data = (unsigned char *)cdata;
  unsigned total = 0;

  for (i = 0; i < length; i++) {
    data[i] = i % 256;
  }

  for (j = 0; j < 10; j++) {
    for (i = 0; i < length; i++) {
      total += data[i];
    }
  }

#ifndef BENCHMARK
  printf("scan result is %d\n", total);
#endif
}
