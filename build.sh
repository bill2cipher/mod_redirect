#!/bin/bash
apxs -n mod_redirect -i -c mod_redirect.c util.h config.h util.c config.c   && systemctl restart httpd

