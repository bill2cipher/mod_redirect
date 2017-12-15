#ifndef util_h
#define util_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define log_header "mod_redirect: "
#define error(...) ap_log_error(APLOG_MARK, APLOG_ERR, 0, __VA_ARGS__)
#define debug(...) ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, __VA_ARGS__)
#define info(...)  ap_log_error(APLOG_MARK, APLOG_INFO, 0, __VA_ARGS__)

#define SEP_CHAR ="\t\n\v\f\r "

void init_rand(void);
bool conform_rand(float probability);
bool start_with(const char *src, const char *start);
char* trim_space(char* str);

#endif /* util_h */
