#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

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

#include "config.h"
#include "util.h"


extern RedirectConfig config;

#define MAX_LINE_LEN 1024

void reset_array(char* arr[], int len) {
    int i = 0;
    for(i = 0; i < len; i++) {
        arr[i] = NULL;
    }
}

void reset_config() {
    config.probability = 0;
    config.cookie_key = NULL;
    config.cookie_op = 0;
    config.refer_op = 0;
    config.uri_op = 0;
    config.target = NULL;
    config.enabled = false;
    reset_array(config.cookie_value, MAX_CONFIG_VALUE_LEN);
    reset_array(config.uri_value, MAX_CONFIG_VALUE_LEN);
    reset_array(config.refer_value, MAX_CONFIG_VALUE_LEN);
}

bool parse_probability(request_rec* r, char *line) {
    char* last = NULL;
    char* cmd = apr_strtok(line, " ", &last);
    if (!cmd || apr_strnatcmp(cmd, "probability") != 0) {
        config.probability = 0;
        error(r->server, log_header"disable probability for unknown error %s.", cmd);
        return false;
    }
    char* value = apr_strtok(NULL, " ", &last);
    if (!value) {
        error(r->server, log_header"probability cmd value not set");
        config.probability = 0;
        return false;
    }
    config.probability = strtof(value, NULL);
    debug(r->server, log_header"parse probability cmd success: %f", config.probability);
    return true;
}

bool parse_cookie(request_rec* r, char* line) {
    char* last = NULL;
    char* value = NULL;
    char* key = NULL;
    char* cmd = apr_strtok(line, " ", &last);
    int i = 0, counter = 0;
    if (!cmd || apr_strnatcmp(cmd, "cookie") != 0) {
        config.cookie_op = 3;
        error(r->server, log_header"disable cookie for unknown error %s.", cmd);
        return false;
    }
    
    key = apr_strtok(NULL, " ", &last);
    if (!key) {
        config.cookie_op = 3;
        error(r->server, log_header"disable cookie for key not set");
        return false;
    }
    for(i = 0; i < MAX_CONFIG_VALUE_LEN; i++) {
        value = apr_strtok(NULL, " ", &last);
        if (!value) {
            break;
        }
        counter += 1;
        config.cookie_value[i] = value;
        config.cookie_value[i + 1] = NULL;
    }
    if (counter <= 0) {
        config.cookie_op = 3;
        info(r->server, log_header"no cookie value found");
        return false;
    } else {
        config.cookie_op = 1;
        debug(r->server, log_header"found %d cookie value", counter);
        return true;
    }
}

bool parse_refer(request_rec* r, char* line) {
    char* last = NULL;
    char* value = NULL;
    char* cmd = apr_strtok(line, "", &last);
    int i = 0, counter = 0;
    if (!cmd || apr_strnatcmp(cmd, "refer") != 0) {
        config.refer_op = 3;
        error(r->server, log_header"disable refer for unknown error %s", cmd);
        return false;
    }
    for(i = 0; i < MAX_CONFIG_VALUE_LEN; i++) {
        value = apr_strtok(NULL, " ", &last);
        if (!value) {
            break;
        }
        counter += 1;
        config.refer_value[i] = value;
        config.refer_value[i + 1] = NULL;
    }
    if (counter <= 0) {
        config.refer_op = 3;
        info(r->server, log_header"no refer value found");
        return false;
    } else {
        config.refer_op = 1;
        debug(r->server, log_header"found %d refer value", counter);
        return true;
    }
}

bool parse_uri(request_rec* r, char* line) {
    char* last = NULL;
    char* value = NULL;
    char* cmd = apr_strtok(line, "", &last);
    int i = 0, counter = 0;
    if (!cmd || apr_strnatcmp(cmd, "uri") != 0) {
        config.uri_op = 3;
        error(r->server, log_header"disable uri for unknown error %s", cmd);
        return false;
    }
    for(i = 0; i < MAX_CONFIG_VALUE_LEN; i++) {
        value = apr_strtok(NULL, " ", &last);
        if (!value) {
            break;
        }
        counter += 1;
        config.uri_value[i] = value;
        config.uri_value[i + 1] = NULL;
    }
    if (counter <= 0) {
        config.uri_op = 3;
        info(r->server, log_header"no uri value found");
        return false;
    } else {
        config.uri_op = 1;
        debug(r->server, log_header"found %d uri value", counter);
        return true;
    }
}

bool parse_target(request_rec* r, char* line) {
    char* last = NULL;
    char* cmd = apr_strtok(line, " ", &last);
    if (!cmd || apr_strnatcmp(cmd, "target") != 0) {
        config.target = NULL;
        error(r->server, log_header"disable redirect for parse target cmd error %s.", cmd);
        return false;
    }
    char* value = apr_strtok(NULL, " ", &last);
    if (!value) {
        error(r->server, log_header"target value not set");
        config.target = NULL;
        return false;
    }
    config.target = value;
    debug(r->server, log_header"parse target success: %s", config.target);
    return true;
}

bool parse_enabled(request_rec* r, char* line) {
    char* last = NULL;
    char* value = NULL;
    char* cmd = apr_strtok(line, " ", &last);
    if (!cmd || apr_strnatcmp(cmd, "enabled") != 0) {
        config.enabled = false;
        error(r->server, log_header"disable redirect for parse enabled cmd error %s.", cmd);
        return false;
    }
    
    value = apr_strtok(NULL, " ", &last);
    if (!value) {
        debug(r->server, log_header"disable redirect for value not set");
        config.enabled = false;
        return false;
    } else if (apr_strnatcmp(value, "on") == 0) {
        config.enabled = true;
        return true;
    } else {
        debug(r->server, log_header"disable redirect for enabled not set on %s", value);
        config.enabled = false;
        return false;
    }
}

bool parse_config(request_rec* r, ap_configfile_t* f) {
    char line[MAX_LINE_LEN];
    char* copy = NULL;
    bool valid = true, op_set = false;
    
    // first reset all config values
    reset_config();

    while(!(ap_cfg_getline(line, MAX_LINE_LEN, f))) {
        copy = apr_pstrdup(r->pool, line);
        if (start_with(copy, "probability")) {
            valid = parse_probability(r, copy);
        } else if (start_with(copy, "target")) {
            valid = parse_target(r, copy);
        } else if (start_with(copy, "uri")) {
            op_set = true;
            valid = parse_uri(r, copy);
        } else if (start_with(copy, "cookie")) {
            op_set = true;
            valid = parse_cookie(r, copy);
        } else if (start_with(copy, "refer")) {
            op_set = true;
            valid = parse_refer(r, copy);
        } else if (start_with(line, "enabled")) {
            valid = parse_enabled(r, copy);
        } else {
            error(r->server, log_header"meet unknown command %s", copy);
            valid = false;
        }
        if (!valid) {
            config.enabled = false;
            break;
        }
    }
    if (!valid) {
        info(r->server, log_header"disable for meet invalid cmd");
        return false;
    } else if (!config.enabled) {
        info(r->server, log_header"disalbe for enabled not set");
        return false;
    } else if (!op_set) {
        info(r->server, log_header"disable for no operation set");
        config.enabled = false;
        return false;
    } else if (config.target == NULL) {
        info(r->server, log_header"disable for no target set");
        config.enabled = false;
        return false;
    } else if (config.probability <= 0) {
        info(r->server, log_header"disable for probability(%f) less or equal to zero", config.probability);
        config.enabled = false;
        return false;
    }
    debug(r->server, "enable redirect with probability(%f), target(%s)", config.probability, config.target);
    return true;
}

bool read_config(request_rec* r) {
    apr_status_t status;
    ap_configfile_t* f = NULL;
    bool parse_result;
    if (config.filepath == NULL) {
        debug(r->server, log_header"config file not set %s", config.filepath);
        return false;
    }
    status = ap_pcfg_openfile(&f, r->pool, config.filepath);
    if (status != APR_SUCCESS) {
        debug(r->server, log_header"read config file failed %d", status);
        return false;
    }
    parse_result = parse_config(r, f);
    ap_cfg_closefile(f);
    return parse_result;
}
