#ifndef REGISTRY_H
#define REGISTRY_H
#ifdef __cplusplus
extern "C" {
#endif

#include <mutex.h>
#include <stddef.h>

#define MAX_REGISTRY_NAME 16
#define MAX_REGISTRY_KEY 16
#define MAX_REGISTRY_PATH 255

struct reg_group {
    char *name;

    struct reg_group *groups;
    size_t n_groups;

    struct reg_key *keys;
    size_t n_keys;
};

struct reg_key {
    char *name;

    void *data;
    size_t n_data;

    void (*read)(void *buffer, size_t len);
    void (*write)(const void *buffer, size_t len);
};

void reg_init(void);
struct reg_group *reg_get_root_group(void);
struct reg_group *reg_create_group(struct reg_group *root, const char *name);
struct reg_key *reg_resolve_path(struct reg_group *root, const char *path);
struct reg_group *reg_add_subgroup_to_group(
    struct reg_group *root, struct reg_group *subgroup);
struct reg_key *reg_add_key_to_group(
    struct reg_group *root, struct reg_key *key);
struct reg_key *reg_find_key_in_group(struct reg_group *root, const char *name);
struct reg_group *reg_find_subgroup_in_group(
    struct reg_group *root, const char *name);
void reg_destroy_group(struct reg_group *group);
struct reg_key *reg_create_key(struct reg_group *root, const char *name);
void reg_destroy_key(struct reg_key *key);

void reg_dump_key(struct reg_key *key, int level);
void reg_dump_group(struct reg_group *root, int level);

#ifdef __cplusplus
}
#endif
#endif