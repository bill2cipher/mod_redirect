main: util.o directive.o mod_redirect.o
    apxs -c redirect_mod.c -o mod_redirect