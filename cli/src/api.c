#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "api.h"

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Not enough memory (realloc returned NULL)\n");
        return 0;
    }
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

static int perform_request(Config *config, const char *url, const char *method, 
                          const char *post_data, struct MemoryStruct *chunk) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        return 0;
    }
    
    chunk->memory = malloc(1);
    chunk->size = 0;
    
    char auth_header[MAX_API_KEY_LENGTH + 20];
    snprintf(auth_header, sizeof(auth_header), "x-api-key: %s", config->api_key);
    
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header);
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)chunk);
    
    if (strcmp(method, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    } else if (strcmp(method, "PUT") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    } else if (strcmp(method, "DELETE") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(chunk->memory);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return 0;
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return 1;
}

int api_get_passwords(Config *config, Password **passwords, int *count) {
    struct MemoryStruct chunk;
    char url[MAX_URL_LENGTH + 20];
    snprintf(url, sizeof(url), "%s/passwords", config->server_url);
    
    if (!perform_request(config, url, "GET", NULL, &chunk)) {
        return 0;
    }
    
    json_object *root = json_tokener_parse(chunk.memory);
    if (!root) {
        fprintf(stderr, "Failed to parse JSON response\n");
        free(chunk.memory);
        return 0;
    }
    
    json_object *success_obj, *data_obj;
    if (!json_object_object_get_ex(root, "success", &success_obj) || 
        !json_object_get_boolean(success_obj)) {
        fprintf(stderr, "API request failed\n");
        json_object_put(root);
        free(chunk.memory);
        return 0;
    }
    
    if (!json_object_object_get_ex(root, "data", &data_obj)) {
        fprintf(stderr, "No data in response\n");
        json_object_put(root);
        free(chunk.memory);
        return 0;
    }
    
    int array_len = json_object_array_length(data_obj);
    *count = array_len;
    *passwords = (Password *)malloc(sizeof(Password) * array_len);
    
    for (int i = 0; i < array_len; i++) {
        json_object *item = json_object_array_get_idx(data_obj, i);
        json_object *id_obj, *title_obj, *username_obj, *password_obj, *url_obj, *notes_obj;
        
        json_object_object_get_ex(item, "id", &id_obj);
        json_object_object_get_ex(item, "title", &title_obj);
        json_object_object_get_ex(item, "username", &username_obj);
        json_object_object_get_ex(item, "password", &password_obj);
        json_object_object_get_ex(item, "url", &url_obj);
        json_object_object_get_ex(item, "notes", &notes_obj);
        
        (*passwords)[i].id = json_object_get_int(id_obj);
        strncpy((*passwords)[i].title, json_object_get_string(title_obj), 255);
        strncpy((*passwords)[i].username, json_object_get_string(username_obj), 255);
        strncpy((*passwords)[i].password, json_object_get_string(password_obj), 1023);
        
        if (url_obj && !json_object_is_type(url_obj, json_type_null)) {
            strncpy((*passwords)[i].url, json_object_get_string(url_obj), 511);
        } else {
            (*passwords)[i].url[0] = '\0';
        }
        
        if (notes_obj && !json_object_is_type(notes_obj, json_type_null)) {
            strncpy((*passwords)[i].notes, json_object_get_string(notes_obj), 2047);
        } else {
            (*passwords)[i].notes[0] = '\0';
        }
    }
    
    json_object_put(root);
    free(chunk.memory);
    return 1;
}

int api_get_password(Config *config, int id, Password *password) {
    struct MemoryStruct chunk;
    char url[MAX_URL_LENGTH + 30];
    snprintf(url, sizeof(url), "%s/passwords/%d", config->server_url, id);
    
    if (!perform_request(config, url, "GET", NULL, &chunk)) {
        return 0;
    }
    
    json_object *root = json_tokener_parse(chunk.memory);
    if (!root) {
        fprintf(stderr, "Failed to parse JSON response\n");
        free(chunk.memory);
        return 0;
    }
    
    json_object *success_obj, *data_obj;
    if (!json_object_object_get_ex(root, "success", &success_obj) || 
        !json_object_get_boolean(success_obj)) {
        fprintf(stderr, "API request failed\n");
        json_object_put(root);
        free(chunk.memory);
        return 0;
    }
    
    if (!json_object_object_get_ex(root, "data", &data_obj)) {
        fprintf(stderr, "No data in response\n");
        json_object_put(root);
        free(chunk.memory);
        return 0;
    }
    
    json_object *id_obj, *title_obj, *username_obj, *password_obj, *url_obj, *notes_obj;
    
    json_object_object_get_ex(data_obj, "id", &id_obj);
    json_object_object_get_ex(data_obj, "title", &title_obj);
    json_object_object_get_ex(data_obj, "username", &username_obj);
    json_object_object_get_ex(data_obj, "password", &password_obj);
    json_object_object_get_ex(data_obj, "url", &url_obj);
    json_object_object_get_ex(data_obj, "notes", &notes_obj);
    
    password->id = json_object_get_int(id_obj);
    strncpy(password->title, json_object_get_string(title_obj), 255);
    strncpy(password->username, json_object_get_string(username_obj), 255);
    strncpy(password->password, json_object_get_string(password_obj), 1023);
    
    if (url_obj && !json_object_is_type(url_obj, json_type_null)) {
        strncpy(password->url, json_object_get_string(url_obj), 511);
    } else {
        password->url[0] = '\0';
    }
    
    if (notes_obj && !json_object_is_type(notes_obj, json_type_null)) {
        strncpy(password->notes, json_object_get_string(notes_obj), 2047);
    } else {
        password->notes[0] = '\0';
    }
    
    json_object_put(root);
    free(chunk.memory);
    return 1;
}

int api_add_password(Config *config, Password *password) {
    struct MemoryStruct chunk;
    char url[MAX_URL_LENGTH + 20];
    snprintf(url, sizeof(url), "%s/passwords", config->server_url);
    
    json_object *json = json_object_new_object();
    json_object_object_add(json, "title", json_object_new_string(password->title));
    json_object_object_add(json, "username", json_object_new_string(password->username));
    json_object_object_add(json, "password", json_object_new_string(password->password));
    
    if (strlen(password->url) > 0) {
        json_object_object_add(json, "url", json_object_new_string(password->url));
    }
    
    if (strlen(password->notes) > 0) {
        json_object_object_add(json, "notes", json_object_new_string(password->notes));
    }
    
    const char *json_str = json_object_to_json_string(json);
    
    int result = perform_request(config, url, "POST", json_str, &chunk);
    
    json_object_put(json);
    
    if (!result) {
        return 0;
    }
    
    json_object *root = json_tokener_parse(chunk.memory);
    if (!root) {
        fprintf(stderr, "Failed to parse JSON response\n");
        free(chunk.memory);
        return 0;
    }
    
    json_object *success_obj, *data_obj, *id_obj;
    if (!json_object_object_get_ex(root, "success", &success_obj) || 
        !json_object_get_boolean(success_obj)) {
        fprintf(stderr, "API request failed\n");
        json_object_put(root);
        free(chunk.memory);
        return 0;
    }
    
    if (json_object_object_get_ex(root, "data", &data_obj) &&
        json_object_object_get_ex(data_obj, "id", &id_obj)) {
        password->id = json_object_get_int(id_obj);
    }
    
    json_object_put(root);
    free(chunk.memory);
    return 1;
}

int api_update_password(Config *config, Password *password) {
    struct MemoryStruct chunk;
    char url[MAX_URL_LENGTH + 30];
    snprintf(url, sizeof(url), "%s/passwords/%d", config->server_url, password->id);
    
    json_object *json = json_object_new_object();
    json_object_object_add(json, "title", json_object_new_string(password->title));
    json_object_object_add(json, "username", json_object_new_string(password->username));
    json_object_object_add(json, "password", json_object_new_string(password->password));
    
    if (strlen(password->url) > 0) {
        json_object_object_add(json, "url", json_object_new_string(password->url));
    }
    
    if (strlen(password->notes) > 0) {
        json_object_object_add(json, "notes", json_object_new_string(password->notes));
    }
    
    const char *json_str = json_object_to_json_string(json);
    
    int result = perform_request(config, url, "PUT", json_str, &chunk);
    
    json_object_put(json);
    
    if (!result) {
        return 0;
    }
    
    json_object *root = json_tokener_parse(chunk.memory);
    if (!root) {
        fprintf(stderr, "Failed to parse JSON response\n");
        free(chunk.memory);
        return 0;
    }
    
    json_object *success_obj;
    if (!json_object_object_get_ex(root, "success", &success_obj) || 
        !json_object_get_boolean(success_obj)) {
        fprintf(stderr, "API request failed\n");
        json_object_put(root);
        free(chunk.memory);
        return 0;
    }
    
    json_object_put(root);
    free(chunk.memory);
    return 1;
}

int api_delete_password(Config *config, int id) {
    struct MemoryStruct chunk;
    char url[MAX_URL_LENGTH + 30];
    snprintf(url, sizeof(url), "%s/passwords/%d", config->server_url, id);
    
    if (!perform_request(config, url, "DELETE", NULL, &chunk)) {
        return 0;
    }
    
    json_object *root = json_tokener_parse(chunk.memory);
    if (!root) {
        fprintf(stderr, "Failed to parse JSON response\n");
        free(chunk.memory);
        return 0;
    }
    
    json_object *success_obj;
    if (!json_object_object_get_ex(root, "success", &success_obj) || 
        !json_object_get_boolean(success_obj)) {
        fprintf(stderr, "API request failed\n");
        json_object_put(root);
        free(chunk.memory);
        return 0;
    }
    
    json_object_put(root);
    free(chunk.memory);
    return 1;
}
