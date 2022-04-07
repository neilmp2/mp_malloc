#include <stdlib.h>
#include <stdio.h>

int main() {
  void *a = malloc(256);  // 256 == 0x 100 bytes
  void *b = malloc(256);
  void *c = malloc(128);
  void *d = malloc(256);
  void *e = malloc(128);
  /* Line 6 */
  free(a);
  free(b);
  free(d);
   
  void *r1 = malloc(10);
  void *r2 = malloc(10);
  void *r3 = malloc(300);
  void *r4 = malloc(250);

}