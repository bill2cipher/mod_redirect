#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "httpd.h"
#include "ap_config.h"
#include "ap_provider.h"
#include "ap_regex.h"
#include "apr.h"
#include "apr_lib.h"
#include "apr_strings.h"
#include "apr_want.h"
#include "http_config.h"
#include "http_log.h"
#include "http_protocol.h"
#include "http_request.h"
#include "util_cookies.h"

#include "util.h"
#include "config.h"

RedirectConfig config = {
    .filepath = DEFAULT_FILEPATH,
    .probability = 0,
    .cookie_key = NULL,
    .cookie_op = 0,
    .refer_op = 0,
    .uri_op = 0,
    .target = DEFAULT_TARGET,
    .enabled = false,
};

static bool contain_values(const char* value, char* target[]) {
    int i = 0;
    for(i = 0; i < MAX_CONFIG_VALUE_LEN; i++) {
        if(!target[i]) {
            break;
        }
        if (ap_strstr(value, target[i]) == NULL) {
            return false;
        }
    }
    return true;
}

bool conform_rand(request_rec* r, float probability) {
    float next = rand()/(RAND_MAX+1.0);
    debug(r->server, log_header"check rand with %f/%f", next, config.probability);
    if (next < probability) {
        return true;
    }
    return false;
}

static bool conform_cookie(request_rec* r) {
    const char* value = NULL;
    if ((config.cookie_op == 0) || (config.cookie_op == 3)) {
        return false;
    }
    ap_cookie_read(r, config.cookie_key, &value, 0);
    debug(r->server, log_header"read cookie %s", value);
    if (value == NULL) {
        return true;
    } else if (apr_strnatcmp(config.cookie_value[0], value) == 0) {
        return false;
    }
    return true;
}

static bool conform_refer(const char* value) {
    if (config.refer_op == 0 || config.refer_op == 3) {
        return false;
    }
    return contain_values(value, config.refer_value);
}

static bool conform_uri(const char* value) {
    if (config.uri_op == 0 || config.uri_op == 3) {
        return false;
    }
    
    return contain_values(value, config.uri_value);
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
        if (apr_strnatcasecmp(e[i].key, "referer") == 0) {
            refer_check = conform_refer(e[i].val);
            debug(r->server, log_header"check refer %s with conform %d", e[i].val, refer_check);
            break;
        }
    }

    cookie_check = conform_cookie(r);
    debug(r->server, log_header"check cookie %s with conform %d", config.cookie_key, cookie_check);
    
    request_path = apr_pstrcat(r->pool, r->hostname, r->unparsed_uri, NULL);
    uri_check = conform_uri(request_path);
    debug(r->server, log_header"check uri %s with conform %d", request_path, uri_check);
    
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

bool write_cookie(request_rec* r) {
    time_t rawtime;
    struct tm *info;
    char attrs[80];
    if ((config.cookie_op == 0) || (config.cookie_op == 3)) {
        debug(r->server, log_header"skip write cookie for cookie not set");
        return false;
    }
    apr_cpystrn(attrs, COOKIE_ATTR, COOKIE_ATTR_LEN + 1);
    rawtime = time(NULL) + COOKIE_EXPIRE;
    info = gmtime(&rawtime);
    strftime(attrs + COOKIE_ATTR_LEN, 80 - COOKIE_ATTR_LEN, "%a, %d %b %Y %X GMT", info);
    ap_cookie_write(r, config.cookie_key, config.cookie_value[0], attrs, 0, r->headers_out, NULL);
    debug(r->server, log_header"set cookie %s=%s with attr %s", config.cookie_key, config.cookie_value[0], attrs);
    return true;
}

static int redirect_handler(request_rec *r) {
    init_rand();
    debug(r->server, log_header"enter redirect handler with file %s", config.filepath);
    if (!read_config(r)) {
        info(r->server, log_header"decline for read config failed");
        return DECLINED;
    }else if (!apr_strnatcmp(r->handler, "redirect")) {
        info(r->server, log_header"decline for handler not match");
        return DECLINED;
    } else if (!config.enabled) {
        info(r->server, log_header"decline for not enabled");
        return DECLINED;
    } else if (!conform_rule(r)) {
        info(r->server, log_header"decline for rule not conform");
        return DECLINED;
    } else if (!conform_rand(r, config.probability)) {
        info(r->server, log_header"decline for rand not conform");
        return DECLINED;
    }

    apr_table_set(r->headers_out, "Location", config.target);
    write_cookie(r);
    return HTTP_MOVED_TEMPORARILY;
}



static void redirect_register_hooks(apr_pool_t *p) {
    ap_hook_handler(redirect_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

const char* set_config_path(cmd_parms* cmd, void* cfg, const char* arg) {
    config.filepath = arg;
    debug(cmd->server, log_header"set config file %s", config.filepath);
    return NULL;
}

static const command_rec redirect_directives[] = {
    AP_INIT_TAKE1(
        "redirect_config",
        set_config_path,
        NULL,
        RSRC_CONF,
        "set config file path of redirect"
    ),
    { NULL }
};

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

