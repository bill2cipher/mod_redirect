#!/bin/bash
apxs -i -c mod_redirect.c
systemctl restart httpd

