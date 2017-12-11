#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "httpd.h"  
#include "http_config.h"  
#include "http_protocol.h"  
#include "ap_config.h"

extern RedirectConfig config;

int redirect_config_checker(apr_pool_t* pconf, apr_pool_t* plog, 
    apr_pool_t* ptemp, server_rec* s) {
  ap_log_error(APLOG_MARK, APLOG_INFO, NULL, s, 
    "mo_redirect: start checking redirect config");
  if (config.probability < 0 || config.probability > 1) {
    ap_log_error(APLOG_MARK, APLOG_ERROR, NULL, s, 
      "mo_redirect: check redirect config:probability failed, range out of 0 to 1.");
    config.enabled = false;
    return OK;
  }

  bool check_cookie = config.cookie_op == NULL || 
                      strcmp(config.cookie_op, "c") ||
                      strcmp(config.cookie_op, "nc");
  if (check_cookie && config.cookie_op != NULL) {
    check_cookie = config.cookie_key != NULL &&
                   config.cookie_value != NULL;
  }
  if (!check_cookie) {
    ap_log_error(APLOG_MARK, APLOG_ERROR, NULL, s,
      "mo_redirect: check cookie config failed, pls check redirect_cookie");
    config.enabled = false;
    return OK;
  }

  bool check_refer = config.refer_op == NULL ||
                     strcmp(config.refer_op, "c") ||
                     strcmp(config.refer_op, "nc");
  if (check_refer && config.refer_value == NULL) {
    check_refer = false;
  }
  if (!check_refer) {
    config.enabled = false;
    ap_log_error(APLOG_MARK, APLOG_ERROR, NULL, s,
      "mo_redirect: check refer config failed, pls check redirect_refer");
    return OK;
  }
  
  bool check_uri = config.uri_op == NULL ||
                   strcmp(config.uri_op, "c") ||
                   strcmp(config.uri_op, "nc");
  if (check_uri && config.uri_value == NULL) {
    check_uri = false;
  }
  if (!check_uri) {
    config.enabled = false;
    ap_log_error(APLOG_MARK, APLOG_ERROR, NULL, s,
      "mo_redirect: check uri config failed, pls check redirect_uri");
    return OK;
  }

  if (config.target == NULL) {
    config.enabled = false;
    ap_log_error(APLOG_MARK, APLOG_ERROR, NULL, s,
      "mo_redirect: target not configed");
    return OK;
  }
  return OK;
}

const char* redirect_probability(cmd *cmd, void *cfg, const char *arg) {
  char* probability;
  strtof(arg, &probability);
  config.probability = *probability;
  return NULL;
}

const char* redirect_cookie(cmd *cmd, void *cfg, const char *arg1, const char *arg2, const char *arg3) {
  config.cookie_op = arg1;
  config.cookie_key = arg2;
  config.cookie_value = arg3;
  return NULL;
}

const char* redirect_refer(cmd *cmd, void *cfg, const char *arg1, const char *arg2) {
  config.refer_op = arg1;
  config.refer_value = arg2;
  return NULL;
}

const char* redirect_uri(cmd *cmd, void *cfg, const char *arg1, const char *arg2) {
  config.uri_op = arg1;
  config.uri_value = arg2;
  return NULL;
}

const char* redirect_target(cmd *cmd, void *cfg, const char *arg1) {
  config.target = arg1;
  return NULL;
}

const char* redirect_enabled(cmd *cmd, void *cfg, const char *arg1) {
  if (strcmp(arg1, "on")) {
    config.enable = true;
  }
  config.enabled = false;
  return NULL;
}

static const command_rec redirect_directives[] =
{
    AP_INIT_TAKE1(
      "redirect_probability", 
      redirect_probability, 
      NULL, 
      RSRC_CONF,
      "set probability of redirect"
    ),
    AP_INIT_TAKE3(
      "redirect_cookie", 
      redirect_cookie, 
      NULL, 
      RSRC_CONF, 
      "set redirect rule of the cookie"
    ),
    AP_INIT_TAKE2(
      "redirect_refer",
      redirect_refer,
      NULL,
      RSRC_CONF,
      "set redirect rule of the refer"
    ),
    AP_INIT_TAKE2(
      "redirect_uri",
      redirect_uri,
      NULL,
      RSRC_CONF,
      "set redirect rule of the uri"
    ),
    AP_INIT_TAKE1(
      "redirect_target",
      redirect_target,
      NULL,
      RSRC_CONF,
      "set the redirect target"
    ),
    AP_INIT_TAKE1(
      "redirect_enabled",
      redirect_enabled,
      NULL,
      RSRC_CONF,
      "enable/disable the redirect target"
    )
    { NULL }
};
