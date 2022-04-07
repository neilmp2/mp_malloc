#include <stdio.h>
#include <stdlib.h>

#define NUM_TIMES 3

int main() {
  for (int i = 0; i < NUM_TIMES; i++) {
    malloc(100);    
  }
  void *ptr = malloc(100);
  for (int i = 0; i < NUM_TIMES; i++) {
    malloc(100);
  }

  for (int i = 0; i < NUM_TIMES; i++) {
    free(ptr);
    ptr = malloc(100);
  }
}