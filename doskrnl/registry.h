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

struct RegistryGroup {
    char *name;

    struct RegistryGroup *groups;
    size_t n_groups;

    struct RegistryKey *keys;
    size_t n_keys;
};

struct RegistryKey {
    char *name;

    void *data;
    size_t n_data;

    void (*read)(void *buffer, size_t len);
    void (*write)(const void *buffer, size_t len);
};

void KeInitRegistry(void);
#define KeGetRegistryRootGroup _Zrmgrgrp
struct RegistryGroup *KeGetRegistryRootGroup(void);
#define KeCreateRegistryGroup _Zrmcrgrp
struct RegistryGroup *KeCreateRegistryGroup(struct RegistryGroup *root, const char *name);
#define KeResolveRegistryPath _Zrmrrp
struct RegistryKey *KeResolveRegistryPath(struct RegistryGroup *root, const char *path);
#define KeAddRegistryGroupToGroup _Zrmargt
struct RegistryGroup *KeAddRegistryGroupToGroup(struct RegistryGroup *root,
    struct RegistryGroup *subgroup);
#define KeAddRegistryKeyToGroup _Zrmarkg
struct RegistryKey *KeAddRegistryKeyToGroup(struct RegistryGroup *root,
    struct RegistryKey *key);
#define KeFindRegistryKeyInGroup _Zrmfkig
struct RegistryKey *KeFindRegistryKeyInGroup(const struct RegistryGroup *root,
    const char *name);
#define KeFindRegistryGroupInGroup _Zrmfgig
struct RegistryGroup *KeFindRegistryGroupInGroup(const struct RegistryGroup *root,
    const char *name);
#define KeDestroyRegistryGroup _Zrmdrg
void KeDestroyRegistryGroup(struct RegistryGroup *group);
#define KeCreateRegistryKey _Zrmcrk
struct RegistryKey *KeCreateRegistryKey(struct RegistryGroup *root, const char *name);
#define KeDestroyRegistryKey _Zrmdrk
void KeDestroyRegistryKey(struct RegistryKey *key);

#if defined(DEBUG)
#define KeDumpRegistryKey _Zrmdrk
void KeDumpRegistryKey(const struct RegistryKey *key, int level);
#define KeDumpRegistryGroup _Zrmdrg
void KeDumpRegistryGroup(const struct RegistryGroup *root, int level);
#endif

#endif
