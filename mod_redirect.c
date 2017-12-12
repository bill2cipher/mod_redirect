#include "common.h"

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

extern command_rec redirect_directives[];

static int redirect_handler(request_rec *r) {
  if (strcmp(r->handler, "redirect")) {
    return DECLINED;
  }
  else if (!config.enabled) {
    return DECLINED;
  }
  else if (!conform_rand()) {
    return DECLINED;
  } else if (!conform_rule()) {
    return DECLINED;
  }

  r->content_type = "text/html";
  ap_rputs("The sample page from mod_helloworld.c\n", r);
  return DONE;
}

static bool conform_cookie(request_rec *r) {
  cookies = r.headers_in;
  if (config.cookie_op == NULL)
  {
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

  if (strchr(value, config.refer_value) != NULL) {
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

  if (strchr(value, config.uri_value) != NULL) {
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
  bool refer_check = false, cookie_check=false, uri_check=false;
  fields = apr_table_elts(r->headers_in);
  e = (apr_table_entry_t *)fields->elts;
  for (i = 0; i < fields->nelts; i++) {
    if (strcmp(e[i].key, "Referer") == 0) {
      refer_check = conform_refer(e[i].value)
    } else if (strcmp(e[i].key, "Cookie") == 0) {
      cookie_check = conform_cookie(e[i].value);
    }
  }
  uri_check = conform_uri(r.parsed_uri)
  if (refer_check && cookie_check && uri_check) {
    return true;
  }
  return false;
}

static void redirect_register_hooks(apr_pool_t *p) {
  ap_hook_post_config(redirect_config_checker, NULL, NULL, APR_HOOK_MIDDLE);
  ap_hook_handler(redirect_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA helloworld_module = {
    STANDARD20_MODULE_STUFF,
    NULL,
    NULL,
    NULL,
    NULL,
    redirect_directives,
    redirect_register_hooks
};
