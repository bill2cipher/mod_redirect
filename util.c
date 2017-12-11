#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void init_rand() {
  srand((int)time(0));
}

bool check_rand() {
  float next = n * rand()/(RAND_MAX+1.0);
  if (next < probability) {
    return true;
  }
  return false;
}
