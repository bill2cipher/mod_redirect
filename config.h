#ifndef config_h
#define config_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_CONFIG_VALUE_LEN 5

typedef struct{
    char* filepath;
    float probability;
    char* cookie_key;
    char* cookie_value[MAX_CONFIG_VALUE_LEN + 1];
    int cookie_op;       // 0 unset, 1 contain, 2 not contain, 3 invalid
    char* refer_value[MAX_CONFIG_VALUE_LEN + 1];
    int refer_op;
    char* uri_value[MAX_CONFIG_VALUE_LEN + 1];
    int uri_op;
    char* target;
    bool enabled;
} RedirectConfig;

static RedirectConfig config = {
    .filepath = NULL,
    .probability = 0,
    .cookie_key = NULL,
    .cookie_value = NULL,
    .cookie_op = 0,
    .refer_value = NULL,
    .refer_op = 0,
    .uri_value = NULL,
    .uri_op = 0,
    .target = NULL,
    .enabled = false,
};

bool read_config(request_rec* r);
#endif /* config_h */
