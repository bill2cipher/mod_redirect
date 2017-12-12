#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
const char* cookies = "session_id=f3674771a4106cadce0911d385eaff145c212e13341726347e445f0029628567; "
"xhs_spid.2420=306f90d149dc9124.1512528217.13.1513057023.1512964641.dce9a5ea-6426-4ad5-a592-a06cf396862c;"
" _sp_id.2420=306f90d149dc9124.1512528217.13.1513057023.1512964641.362f98f7-e15b-41f3-a432-fd67e57df870;"
" session=.eJxVjs8OgyAMh1_F9LzD_pkoh532FmMhoHUhc7JAjRrGu0_Aw8bla7-U_uphMU5qMaGy-NCO7ALMQ_ECBp6Dw8YicWAF"
"h_fuw9c377vpemFzqlWdII9lYl1tEz7x0CRUpc1tm3s8mfz5nNCVHHZFzHJOm0GsiZJigYNUPbYxneyIcaYx5qnRRbVe12KPhCLL6G"
"73vIh-nA_hXwpJZLUaadsTAoTwBcliXOs.DREDUg.wCs5TkrJXFOCWOmeFJ005-xT-Sc";
void init_rand() {
  srand((int)time(0));
}

bool conform_rand(float probability) {
  float next = rand()/(RAND_MAX+1.0);
  if (next < probability) {
    return true;
  }
  return false;
}

bool get_cookie(const char* cookies) {
  char* c, k, v;
  c = strtok(cookies, ";");
  while (c != NULL) {
    k = strtok(c, "=");
    if (k != NULL) {
      print("key is %s", k);
    }
    v = strtok(v, "=");
    if (v != NULL) {
      print("value is %s", v);
    }
    c = strtok(NULL, ";");
  }
}

int main() {
  get_cookie(cookies);
  return 0;
}