#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/env_loader.h"

#define MAX_LINE_LENGTH 256
#define MAX_ENV_VARS 100

typedef struct {
    char *key;
    char *value;
} EnvVar;

static EnvVar env_vars[MAX_ENV_VARS];
static int env_count = 0;

int load_env_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Warning: Could not open .env file: %s\n", filename);
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    env_count = 0;

    while (fgets(line, sizeof(line), file) && env_count < MAX_ENV_VARS) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }

        // Find the = sign
        char *equals = strchr(line, '=');
        if (!equals) {
            continue;
        }

        // Split key and value
        *equals = '\0';
        char *key = line;
        char *value = equals + 1;

        // Trim whitespace
        while (*key == ' ' || *key == '\t') key++;
        while (*value == ' ' || *value == '\t') value++;

        // Store the key-value pair
        env_vars[env_count].key = strdup(key);
        env_vars[env_count].value = strdup(value);
        env_count++;
    }

    fclose(file);
    return 0;
}

char* get_env_value(const char *key) {
    for (int i = 0; i < env_count; i++) {
        if (strcmp(env_vars[i].key, key) == 0) {
            return env_vars[i].value;
        }
    }
    return NULL;
}
