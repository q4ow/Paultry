#ifndef API_H
#define API_H

#include "config.h"

typedef struct {
    int id;
    char title[256];
    char username[256];
    char password[1024];
    char url[512];
    char notes[2048];
} Password;

int api_get_passwords(Config *config, Password **passwords, int *count);
int api_get_password(Config *config, int id, Password *password);
int api_add_password(Config *config, Password *password);
int api_update_password(Config *config, Password *password);
int api_delete_password(Config *config, int id);

#endif
