#ifndef REGISTRY_H
#define REGISTRY_H

/* The registry manager serves as a hierachy of environment variables where
 * drivers can place their own registers as applications can too
 *
 * The purpouse is to redirect writes/reads to an enviroment variable by doing a
 * remote call to an application or driver */

#include <stddef.h>
#include <mutex.h>

#define MAX_REGISTRY_PATH 255

#define MAX_REGISTRY_NAME 16
#define MAX_REGISTRY_KEY 16

struct registry_group {
    char *name;

    struct registry_group *groups;
    size_t n_groups;

    struct registry_key *keys;
    size_t n_keys;
};

struct registry_key {
    char *name;

    void *data;
    size_t n_data;

    void (*read)(void *buffer, size_t len);
    void (*write)(const void *buffer, size_t len);
};

void KeInitRegistry(void);
#define KeGetRegistryRootGroup _Zrmgrgrp
struct registry_group *KeGetRegistryRootGroup(void);
#define KeCreateRegistryGroup _Zrmcrgrp
struct registry_group *KeCreateRegistryGroup(struct registry_group *root, const char *name);
#define KeResolveRegistryPath _Zrmrrp
struct registry_key *KeResolveRegistryPath(struct registry_group *root, const char *path);
#define KeAddRegistryGroupToGroup _Zrmargt
struct registry_group *KeAddRegistryGroupToGroup(struct registry_group *root, struct registry_group *subgroup);
#define KeAddRegistryKeyToGroup _Zrmarkg
struct registry_key *KeAddRegistryKeyToGroup(struct registry_group *root, struct registry_key *key);
#define KeFindRegistryKeyInGroup _Zrmfkig
struct registry_key *KeFindRegistryKeyInGroup(const struct registry_group *root, const char *name);
#define KeFindRegistryGroupInGroup _Zrmfgig
struct registry_group *KeFindRegistryGroupInGroup(const struct registry_group *root, const char *name);
#define KeDestroyRegistryGroup _Zrmdrg
void KeDestroyRegistryGroup(struct registry_group *group);
#define KeCreateRegistryKey _Zrmcrk
struct registry_key *KeCreateRegistryKey(struct registry_group *root, const char *name);
#define KeDestroyRegistryKey _Zrmdrk
void KeDestroyRegistryKey(struct registry_key *key);

#if defined(DEBUG)
#define KeDumpRegistryKey _Zdrmdrk
void KeDumpRegistryKey(const struct registry_key *key, int level);
#define KeDumpRegistryGroup _Zdrmdrg
void KeDumpRegistryGroup(const struct registry_group *root, int level);
#endif

#endif
