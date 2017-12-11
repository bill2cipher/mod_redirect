#ifndef __COMMON_H__
#define __COMMON_H__

#include "ap_config.h"

typedef struct
{
  float probability;
  const char* cookie_key;
  const char* cookie_value;
  const char* cookie_op;

  const char* refer_value;
  const char* refer_op;

  const char* uri_value;
  const char* uri_op;

  const char* target;

  const bool enabled;
} RedirectConfig;

void init_rand()；

bool check_rand();

int redirect_config_checker(
  apr_pool_t* pconf, apr_pool_t* plog, 
  apr_pool_t* ptemp, server_rec* s);
#endif