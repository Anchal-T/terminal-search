#ifndef ENV_LOADER_H
#define ENV_LOADER_H

int load_env_file(const char *filename);
char* get_env_value(const char *key);

#endif // ENV_LOADER_H
