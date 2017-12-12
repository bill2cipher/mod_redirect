#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "ap_regex.h"
#include "http_log.h"

typedef struct
{
  float probability;
  const char *cookie_key;
  const char *cookie_value;
  const int cookie_op; // 0 unset, 1 contain, 2 not contain, 3 invalid

  const char *refer_value;
  const int refer_op;

  const char *uri_value;
  const int uri_op;

  const char *target;

  const bool enabled;
} RedirectConfig;

void init_rand();

bool check_rand();

int redirect_config_checker(
    apr_pool_t *pconf, apr_pool_t *plog,
    apr_pool_t *ptemp, server_rec *s);

#endif
