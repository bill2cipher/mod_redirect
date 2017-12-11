#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "httpd.h"  
#include "http_config.h"  
#include "http_protocol.h"  
#include "ap_config.h"
#include "ap_regex.h"
#include "http_log.h"

#include "common.h"


static RedirectConfig config = {
  .probability=0,
  .cookie_key=NULL,
  .cookie_value=NULL,
  .cookie_op=NULL,
  .refer_value=NULL,
  .refer_op=NULL,
  .uri_value=NULL,
  .uri_op=NULL,
  .target=NULL,
  .enabled=false,
};

extern command_rec redirect_directives[];

static int redirect_handler(request_rec *r) {  
  if (strcmp(r->handler, "redirect")) {  
    return DECLINED;  
  } else if (!config.enabled) {
    return DECLINED;
  } else if (!check_rand()) {
    return DECLINED;
  }
  r->content_type = "text/html";        
  
  apr_status_t* cookie_pass = ap_cookie_check_string("")
  apr_status_t* refer_pass = 
  
  if (!r->header_only)  
    ap_rputs("The sample page from mod_helloworld.c\n", r);  
  return DONE;  
}

static bool check_cookie(request_rec *r) {
  cookies = r.headers_in;
  if (config.cookie_op == NULL) {
    return true;
  }
  ap_cookie_check_string(config.cookie_value);
}

static void redirect_register_hooks(apr_pool_t *p)  {  
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
