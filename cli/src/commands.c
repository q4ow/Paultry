#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include "commands.h"
#include "api.h"

static char* get_password(const char *prompt) {
    static char password[1024];
    struct termios old, new;
    
    // Turn off echo
    tcgetattr(STDIN_FILENO, &old);
    new = old;
    new.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
    
    printf("%s", prompt);
    fgets(password, sizeof(password), stdin);
    
    // Remove newline
    password[strcspn(password, "\n")] = 0;
    
    // Restore terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    printf("\n");
    
    return password;
}

int cmd_configure(Config *config, int argc, char **argv) {
    char input[MAX_URL_LENGTH];
    
    printf("Server URL [%s]: ", config->server_url);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    
    if (strlen(input) > 0) {
        strncpy(config->server_url, input, MAX_URL_LENGTH - 1);
    }
    
    printf("API Key: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    
    if (strlen(input) > 0) {
        strncpy(config->api_key, input, MAX_API_KEY_LENGTH - 1);
    }
    
    if (save_config(config)) {
        printf("Configuration saved successfully.\n");
        return 1;
    } else {
        fprintf(stderr, "Failed to save configuration.\n");
        return 0;
    }
}

int cmd_list(Config *config, int argc, char **argv) {
    Password *passwords;
    int count;
    
    if (!api_get_passwords(config, &passwords, &count)) {
        fprintf(stderr, "Failed to retrieve passwords.\n");
        return 0;
    }
    
    printf("ID  | Title                 | Username               \n");
    printf("----+-----------------------+-----------------------\n");
    
    for (int i = 0; i < count; i++) {
        printf("%-3d | %-21s | %-21s\n", 
               passwords[i].id, 
               passwords[i].title, 
               passwords[i].username);
    }
    
    free(passwords);
    return 1;
}

int cmd_get(Config *config, int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: vault get <id>\n");
        return 0;
    }
    
    int id = atoi(argv[2]);
    Password password;
    
    if (!api_get_password(config, id, &password)) {
        fprintf(stderr, "Failed to retrieve password.\n");
        return 0;
    }
    
    printf("Title: %s\n", password.title);
    printf("Username: %s\n", password.username);
    printf("Password: %s\n", password.password);
    
    if (strlen(password.url) > 0) {
        printf("URL: %s\n", password.url);
    }
    
    if (strlen(password.notes) > 0) {
        printf("Notes: %s\n", password.notes);
    }
    
    return 1;
}

int cmd_add(Config *config, int argc, char **argv) {
    Password password;
    char input[2048];
    
    printf("Title: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    strncpy(password.title, input, 255);
    
    printf("Username: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    strncpy(password.username, input, 255);
    
    strncpy(password.password, get_password("Password: "), 1023);
    
    printf("URL (optional): ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    strncpy(password.url, input, 511);
    
    printf("Notes (optional): ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    strncpy(password.notes, input, 2047);
    
    if (api_add_password(config, &password)) {
        printf("Password added successfully with ID: %d\n", password.id);
        return 1;
    } else {
        fprintf(stderr, "Failed to add password.\n");
        return 0;
    }
}

int cmd_update(Config *config, int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: vault update <id>\n");
        return 0;
    }
    
    int id = atoi(argv[2]);
    Password password;
    
    if (!api_get_password(config, id, &password)) {
        fprintf(stderr, "Failed to retrieve password.\n");
        return 0;
    }
    
    char input[2048];
    
    printf("Title [%s]: ", password.title);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    if (strlen(input) > 0) {
        strncpy(password.title, input, 255);
    }
    
    printf("Username [%s]: ", password.username);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    if (strlen(input) > 0) {
        strncpy(password.username, input, 255);
    }
    
    printf("Password (leave empty to keep current): ");
    char *new_password = get_password("");
    if (strlen(new_password) > 0) {
        strncpy(password.password, new_password, 1023);
    }
    
    printf("URL [%s]: ", password.url);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    if (strlen(input) > 0) {
        strncpy(password.url, input, 511);
    }
    
    printf("Notes [%s]: ", password.notes);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    if (strlen(input) > 0) {
        strncpy(password.notes, input, 2047);
    }
    
    if (api_update_password(config, &password)) {
        printf("Password updated successfully.\n");
        return 1;
    } else {
        fprintf(stderr, "Failed to update password.\n");
        return 0;
    }
}

int cmd_delete(Config *config, int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: vault delete <id>\n");
        return 0;
    }
    
    int id = atoi(argv[2]);
    
    printf("Are you sure you want to delete password with ID %d? (y/n): ", id);
    char confirm;
    scanf(" %c", &confirm);
    
    if (confirm != 'y' && confirm != 'Y') {
        printf("Deletion cancelled.\n");
        return 1;
    }
    
    if (api_delete_password(config, id)) {
        printf("Password deleted successfully.\n");
        return 1;
    } else {
        fprintf(stderr, "Failed to delete password.\n");
        return 0;
    }
}

void print_help() {
    printf("Usage: vault <command> [options]\n\n");
    printf("Commands:\n");
    printf("  configure      Configure the vault client\n");
    printf("  list           List all passwords\n");
    printf("  get <id>       Get a specific password\n");
    printf("  add            Add a new password\n");
    printf("  update <id>    Update an existing password\n");
    printf("  delete <id>    Delete a password\n");
    printf("  help           Show this help message\n");
}
