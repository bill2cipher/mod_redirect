#include "util.h"

void init_rand(void) {
    srand((int)time(0));
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

char *trim_space(char *str) {
  if (str == NULL) {
      return NULL;
  }
  char *end;

  // trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)
      return str;

  // trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // write new null terminator
  *(end+1) = 0;

  return str;
}
