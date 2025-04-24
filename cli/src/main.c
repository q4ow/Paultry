#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "config.h"
#include "commands.h"

int main(int argc, char **argv) {
    // Initialize curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Load or initialize config
    Config config;
    init_config(&config);
    load_config(&config);
    
    int result = 0;
    
    if (argc < 2) {
        print_help();
        result = 1;
    } else {
        if (strcmp(argv[1], "configure") == 0) {
            result = cmd_configure(&config, argc, argv);
        } else if (strcmp(argv[1], "list") == 0) {
            result = cmd_list(&config, argc, argv);
        } else if (strcmp(argv[1], "get") == 0) {
            result = cmd_get(&config, argc, argv);
        } else if (strcmp(argv[1], "add") == 0) {
            result = cmd_add(&config, argc, argv);
        } else if (strcmp(argv[1], "update") == 0) {
            result = cmd_update(&config, argc, argv);
        } else if (strcmp(argv[1], "delete") == 0) {
            result = cmd_delete(&config, argc, argv);
        } else if (strcmp(argv[1], "help") == 0) {
            print_help();
            result = 1;
        } else {
            fprintf(stderr, "Unknown command: %s\n", argv[1]);
            print_help();
            result = 0;
        }
    }
    
    // Cleanup curl
    curl_global_cleanup();
    
    return result ? 0 : 1;
}
