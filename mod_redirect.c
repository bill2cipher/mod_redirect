#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ap_provider.h"
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "ap_regex.h"
#include "http_log.h"
#include "http_request.h"
#include "apr.h"
#include "apr_strings.h"


typedef struct{
  float probability;
  char* cookie_key;
  char* cookie_value;
  int cookie_op; // 0 unset, 1 contain, 2 not contain, 3 invalid

  char* refer_value;
  int refer_op;

  char* uri_value;
  int uri_op;

  char* target;

  bool enabled;
} RedirectConfig;

static RedirectConfig config = {
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

void init_rand() {
  srand((int)time(0));
}

bool conform_rand(float probability) {
  return true;
  float next = rand()/(RAND_MAX+1.0);
  if (next < probability) {
    return true;
  }
  return false;
}

int redirect_config_checker(apr_pool_t* pconf, apr_pool_t* plog, apr_pool_t* ptemp, server_rec* s) {
  ap_log_error(APLOG_MARK, APLOG_INFO, NULL, s, 
    "mod_redirect: start checking redirect config");
  if (config.probability < 0 || config.probability > 1) {
    ap_log_error(APLOG_MARK, APLOG_ERR, NULL, s, 
      "mod_redirect: check redirect config:probability failed, range out of 0 to 1.");
    config.enabled = false;
    return OK;
  }

  bool check_cookie = config.cookie_op != 3;
  if (config.cookie_op == 1 || config.cookie_op == 2) {
    check_cookie &= (config.cookie_key != NULL) &&
                    (config.cookie_value != NULL);
  }

  if (!check_cookie) {
    ap_log_error(APLOG_MARK, APLOG_ERR, NULL, s,
      "mod_redirect: check cookie config failed, pls check redirect_cookie");
    config.enabled = false;
    return OK;
  }

  bool check_refer = config.refer_op != 3;
  if (config.refer_op == 1 || config.refer_op == 2) {
    check_refer &= config.refer_value != NULL;
  }
  if (!check_refer) {
    config.enabled = false;
    ap_log_error(APLOG_MARK, APLOG_ERR, NULL, s,
      "mod_redirect: check refer config failed, pls check redirect_refer");
    return OK;
  }
  
  bool check_uri = config.uri_op != 3;
  if (config.uri_op == 1 || config.uri_op == 2) {
    check_uri &= config.uri_value != NULL;
  }
  if (!check_uri) {
    config.enabled = false;
    ap_log_error(APLOG_MARK, APLOG_ERR, NULL, s,
      "mod_redirect: check uri config failed, pls check redirect_uri");
    return OK;
  }

  if (config.target == NULL) {
    config.enabled = false;
    ap_log_error(APLOG_MARK, APLOG_ERR, NULL, s,
      "mod_redirect: target not configed");
    return OK;
  }

  if ((config.refer_op == 0) && (config.uri_op == 0) && (config.cookie_op == 0)) {
    config.enabled = false;
    ap_log_error(APLOG_MARK, APLOG_ERR, NULL, s,
      "mod_redirect: none of the three rule set, disable redirect");
    return OK;
  }

  ap_log_error(APLOG_MARK, APLOG_INFO, NULL, s,
    "mod_redirect: check config success. probability(%f); enabled(%d); target(%s);", 
    config.probability, config.enabled, config.target);
  return OK;
}

const char* redirect_probability(cmd_parms *cmd, void *cfg, const char *arg) {
  config.probability = strtof(arg, NULL);
  return NULL;
}

const char* redirect_cookie(cmd_parms *cmd, void *cfg, const char *arg1, const char *arg2, const char *arg3) {
  if (!strcmp(arg1, "c")) {
    config.cookie_op = 1;
  } else if (!strcmp(arg1, "nc")) {
    config.cookie_op = 2;
  } else {
    ap_log_error(APLOG_MARK, APLOG_ERR, NULL, cmd->server,
      "mod_redirect: check cookie op failed, pls check redirect_cookie");
    config.cookie_op = 3;
  }
  config.cookie_key = arg2;
  config.cookie_value = arg3;
  return NULL;
}

const char* redirect_refer(cmd_parms *cmd, void *cfg, const char *arg1, const char *arg2) {
  if (!strcmp(arg1, "c")) {
    config.refer_op = 1;
  } else if (!strcmp(arg1, "nc")) {
    config.refer_op = 2;
  } else {
    ap_log_error(APLOG_MARK, APLOG_ERR, NULL, cmd->server,
      "mod_redirect: check refer op failed, pls check redirect_refer");
    config.refer_op = 3;
  }
  config.refer_value = arg2;
  return NULL;
}

const char* redirect_uri(cmd_parms *cmd, void *cfg, const char *arg1, const char *arg2) {
  if (!strcmp(arg1, "c")) {
    config.uri_op = 1;
  } else if (!strcmp(arg1, "nc")) {
    config.uri_op = 2;
  } else {
    ap_log_error(APLOG_MARK, APLOG_ERR, NULL, cmd->server,
      "mod_redirect: check uri op failed, pls check redirect_uri");
    config.uri_op = 3;
  }
  config.uri_value = arg2;
  return NULL;
}

const char* redirect_target(cmd_parms *cmd, void *cfg, const char *arg1) {
  config.target = arg1;
  return NULL;
}

const char* redirect_enabled(cmd_parms *cmd, void *cfg, const char *arg1) {
  if (strcmp(arg1, "on") == 0) {
    config.enabled = true;
  } else {
    config.enabled = false;
  }
  return NULL;
}

static const command_rec redirect_directives[] = {
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
    ),
    { NULL }
};


static bool conform_cookie(const char* cookies) {
  if (config.cookie_op == NULL) {
    return true;
  }
  ap_cookie_check_string(config.cookie_value);
  return false;
}

static bool conform_refer(const char* value) {
  bool contain = false;
  if (config.refer_op == 0 || config.refer_op == 3) {
    return false;
  }

  if (strstr(value, config.refer_value) != NULL) {
    contain = true;
  }
  if (config.refer_op == 1) {
    return contain;
  } else {
    return !contain;
  }
}

static bool conform_uri(const char* value) {
  bool contain = false;
  if (config.uri_op == 0 || config.uri_op == 3) {
    return false;
  }

  if (strstr(value, config.uri_value) != NULL) {
    contain = true;
  }

  if (config.uri_op == 1) {
    return contain;
  } else {
    return !contain;
  }
}

static bool conform_rule(request_rec *r){
  const apr_array_header_t *fields;
  int i;
  apr_table_entry_t *e = 0;
  bool refer_check = false, cookie_check = false, uri_check = false, result = true, rule_set = false;
  char* request_path = NULL;
  fields = apr_table_elts(r->headers_in);
  e = (apr_table_entry_t *)fields->elts;
  for (i = 0; i < fields->nelts; i++) {
    if (strcmp(e[i].key, "Referer") == 0) {
      refer_check = conform_refer(e[i].val);
      ap_log_error(APLOG_MARK, APLOG_DEBUG, NULL, r->server, 
        "mod_redirect: check refer %s/%s with conform %d", e[i].val, config.refer_value, refer_check);
    } else if (strcmp(e[i].key, "Cookie") == 0) {
      ap_log_error(APLOG_MARK, APLOG_DEBUG, NULL, r->server, 
        "mod_redirect: before cookie check %d", e[i].val, config.refer_value, refer_check);
       cookie_check = conform_cookie(e[i].val);
    }
  }

  ap_log_error(APLOG_MARK, APLOG_DEBUG, NULL, r->server, 
    "before uri check", e[i].val, config.refer_value, refer_check);
  request_path = apr_pstrcat(r->pool, r->hostname, r->unparsed_uri);
  uri_check = conform_uri(request_path);
  ap_log_error(APLOG_MARK, APLOG_DEBUG, NULL, r->server,
     "mod_redirect: check uri %s/%s with conform %d", request_path, config.uri_value, uri_check);
     
  if ((config.uri_op != 0) && (config.uri_op != 3)) {
    rule_set = true;
    result = result && uri_check;
  }
  if ((config.refer_op != 0) && (config.uri_op != 3)) {
    rule_set = true;
    result = result && refer_check;
  }
  if ((config.cookie_op != 0) && (config.cookie_op != 3)) {
    rule_set = true;
    result = result && cookie_check;
  }
  if (rule_set && result) {
    return true;
  }
  return false;
}

static int redirect_handler(request_rec *r) {
  ap_log_error(APLOG_MARK, APLOG_DEBUG, NULL, r->server,
    "mod_redirect: enter redirect handler");
  if (!strcmp(r->handler, "redirect")) {
    ap_log_error(APLOG_MARK, APLOG_DEBUG, NULL, r->server,
      "mod_redirect: decline for handler not match");
    return DECLINED;
  }
  else if (!config.enabled) {
    ap_log_error(APLOG_MARK, APLOG_DEBUG, NULL, r->server,
      "mod_redirect: decline for not enabled");
    return DECLINED;
  }
  else if (!conform_rand(config.probability)) {
    ap_log_error(APLOG_MARK, APLOG_DEBUG, NULL, r->server,
      "mod_redirect: decline for rand not conform");
    return DECLINED;
  } else if (!conform_rule(r)) {
    ap_log_error(APLOG_MARK, APLOG_DEBUG, NULL, r->server,
      "mod_redirect: decline for rule not conform");
    return DECLINED;
  }

  r->content_type = "text/html";
  ap_rputs("The sample page from mod_helloworld.c\n", r);
  return DONE;
}


static void redirect_register_hooks(apr_pool_t *p) {
  ap_hook_post_config(redirect_config_checker, NULL, NULL, APR_HOOK_MIDDLE);
  ap_hook_handler(redirect_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA redirect_module = {
    STANDARD20_MODULE_STUFF,
    NULL,
    NULL,
    NULL,
    NULL,
    redirect_directives,
    redirect_register_hooks
};
