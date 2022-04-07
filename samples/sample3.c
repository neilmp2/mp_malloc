#include <stdio.h>
#include <stdlib.h>

int main() {
  void *a = malloc(1280);
  void *b = malloc(1280);
  void *c = malloc(1280);
  void *d = malloc(1280);
  void *e = malloc(1280);

  free(b);
  free(c);
  free(d);

  void *f = malloc(2600);
  void *g = malloc(1);
}