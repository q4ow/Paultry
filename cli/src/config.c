#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "config.h"

static char* get_config_path() {
    static char config_path[1024];
    const char *homedir;
    
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    
    snprintf(config_path, sizeof(config_path), "%s/%s", homedir, CONFIG_FILE_PATH);
    return config_path;
}

int load_config(Config *config) {
    FILE *file = fopen(get_config_path(), "r");
    if (!file) {
        return 0;
    }
    
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "\n");
        
        if (key && value) {
            if (strcmp(key, "api_key") == 0) {
                strncpy(config->api_key, value, MAX_API_KEY_LENGTH - 1);
            } else if (strcmp(key, "server_url") == 0) {
                strncpy(config->server_url, value, MAX_URL_LENGTH - 1);
            }
        }
    }
    
    fclose(file);
    return 1;
}

int save_config(Config *config) {
    FILE *file = fopen(get_config_path(), "w");
    if (!file) {
        return 0;
    }
    
    fprintf(file, "api_key=%s\n", config->api_key);
    fprintf(file, "server_url=%s\n", config->server_url);
    
    fclose(file);
    return 1;
}

void init_config(Config *config) {
    memset(config, 0, sizeof(Config));
    strncpy(config->server_url, "http://localhost:3000", MAX_URL_LENGTH - 1);
}
