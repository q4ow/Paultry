#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FILE_PATH ".password-vault-config"
#define MAX_API_KEY_LENGTH 256
#define MAX_URL_LENGTH 256

typedef struct {
    char api_key[MAX_API_KEY_LENGTH];
    char server_url[MAX_URL_LENGTH];
} Config;

int load_config(Config *config);
int save_config(Config *config);
void init_config(Config *config);

#endif
