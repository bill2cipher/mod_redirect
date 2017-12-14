#include "util.h"

void init_rand(void) {
    srand((int)time(0));
}

bool conform_rand(float probability) {
    float next = rand()/(RAND_MAX+1.0);
    if (next < probability) {
        return true;
    }
    return false;
}

bool start_with(const char *src, const char *start) {
    int i = 0;
    if(strlen(src) < strlen(start))
        return false;
    i = (int)strlen(start) - 1;
    while(i >= 0) {
        if(src[i] != start[i])
            return false;
        i--;
    }
    return true;
}
