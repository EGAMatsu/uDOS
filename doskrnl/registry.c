/* registry.c
 * 
 * Implements a BTree+ registry manager. A registry is basically a big control
 * panel which has a key and a value, when a value is written the call is piped
 * to a driver in the kernel space in order to perform an action.
 * 
 * A practical usage would be a NVMe driver which has a SubsystemId in the
 * registry HLOCAL_DRIVES_NVME0_SSID, if the hypotetical NVMe driver could
 * change the SubsystemId of a drive then a write operation on the registry
 * would make the NVMe drive evaluate the new SSID and reflect the changes
 * via hardware procedures.
 * 
 * In short, the registry manager is an alternative form of a virtual filesystem
 * designed specifically for dynamic assignment (values can be strings, numbers,
 * etcetera). Something that is commonly ignored on VFS implementations which
 * can be useful for driver developers.
 */

#include <mm/mm.h>
#include <debug/panic.h>
#include <registry.h>
#include <memory.h>

/* TODO: Initialize root group to 0 */
struct RegistryGroup *g_root_group;

void KeInitRegistry(
    void)
{
    g_root_group = KeCreateRegistryGroup(NULL, "HKEY");
    return;
}

struct RegistryGroup *KeGetRegistryRootGroup(
    void)
{
    return g_root_group;
}

struct RegistryGroup *KeCreateRegistryGroup(
    struct RegistryGroup *root,
    const char *name)
{
    struct RegistryGroup *group;
    size_t len;

    len = KeStringLength(name);
    if(len >= MAX_REGISTRY_NAME) {
        KePanic("Length exceeds max registry name");
    }

    group = MmAllocateZero(sizeof(struct RegistryGroup));
    if(group == NULL) {
        KePanic("Out of memory");
    }

    group->name = MmAllocate(len + 1);
    if(group->name == NULL) {
        KePanic("Out of memory");
    }
    KeCopyMemory(group->name, name, len + 1);

    if(root != NULL) {
        group = KeAddRegistryGroupToGroup(root, group);
    }
    return group;
}

struct RegistryKey *KeResolveRegistryPath(
    struct RegistryGroup *root,
    const char *path)
{
    const struct RegistryGroup *group;
    const struct RegistryKey *key;
    char *tmpbuf, *buf;
    size_t len;

    len = KeStringLength(path);
    if(len >= MAX_REGISTRY_PATH) {
        KePanic("Length exceeds max registry path");
    }

    tmpbuf = MmAllocate(len + 1);
    if(tmpbuf == NULL) {
        KePanic("Out of memory");
    }
    KeCopyString(tmpbuf, path);

    group = root;
    key = NULL;
    buf = tmpbuf;
    while(*buf != '\0') {
        const struct RegistryGroup *old_group;
        const char *old_tmpbuf;
        char *name;
        size_t name_len;

        /* Find the next separator, these separators allow for namespaces
     * which are then parsed into groups -> keys */
        old_tmpbuf = buf;
        buf = KeBreakCharPtrString(buf, "_-:");
        if(buf == NULL) {
            /* ifno separators were found it means we are at the key */
            name_len = KeStringLength(old_tmpbuf);
        } else {
            name_len = (size_t)((ptrdiff_t)buf - (ptrdiff_t)old_tmpbuf);
            if(!name_len) {
                KePanic("Zero-lenght name");
            }
        }

        /* Skip the separator itself */
        buf++;

        name = MmAllocate(name_len + 1);
        if(name == NULL) {
            KePanic("Ouf of memory");
        }
        KeCopyMemory(name, old_tmpbuf, name_len);
        old_group = group;
        group = KeFindRegistryGroupInGroup(group, name);
        if(group == NULL) {
            group = old_group;
            key = KeFindRegistryKeyInGroup(group, name);
            if(key == NULL) {
                KePanic("Registry %s not found", name);
            }
            MmFree(name);
            break;
        }
        MmFree(name);
    }
    MmFree(tmpbuf);
    return (struct RegistryKey *)key;
}

struct RegistryGroup *KeAddRegistryGroupToGroup(
    struct RegistryGroup *root,
    struct RegistryGroup *subgroup)
{
    root->groups = MmReallocateArray(root->groups, root->n_groups + 1,
                                  sizeof(struct RegistryGroup));
    if(root->groups == NULL) {
        KePanic("Out of memory");
    }
    KeCopyMemory(&root->groups[root->n_groups++], subgroup, sizeof(struct RegistryGroup));
    return &root->groups[root->n_groups - 1];
}

struct RegistryKey *KeAddRegistryKeyToGroup(
    struct RegistryGroup *root,
    struct RegistryKey *key)
{
    root->keys =
        MmReallocateArray(root->keys, root->n_keys + 1, sizeof(struct RegistryKey));
    if(root->keys == NULL) {
        KePanic("Out of memory");
    }
    KeCopyMemory(&root->keys[root->n_keys++], key, sizeof(struct RegistryKey));
    return &root->keys[root->n_keys - 1];
}

struct RegistryGroup *KeFindRegistryGroupInGroup(
    const struct RegistryGroup *root,
    const char *name)
{
    size_t i, len;

    len = KeStringLength(name);
    if(len >= MAX_REGISTRY_KEY) {
        KePanic("Length exceeds max registry key");
    }

    for(i = 0; i < root->n_groups; i++) {
        struct RegistryGroup *subgroup = &root->groups[i];
        if(!KeCompareString(subgroup->name, name)) {
            return subgroup;
        }
    }
    return NULL;
}

struct RegistryKey *KeFindRegistryKeyInGroup(
    const struct RegistryGroup *root,
    const char *name)
{
    size_t i, len;

    len = KeStringLength(name);
    if(len >= MAX_REGISTRY_NAME) {
        KePanic("Length exceeds max registry name");
    }

    for(i = 0; i < root->n_keys; i++) {
        const struct RegistryKey *key = &root->keys[i];
        if(!KeCompareString(key->name, name)) {
            return (struct RegistryKey *)key;
        }
    }
    return NULL;
}

void KeDestroyRegistryGroup(
    struct RegistryGroup *group)
{
    MmFree(group->name);
    MmFree(group);
    return;
}

struct RegistryKey *KeCreateRegistryKey(
    struct RegistryGroup *root,
    const char *name)
{
    struct RegistryKey *key;
    size_t len;

    len = KeStringLength(name);
    if(len >= MAX_REGISTRY_KEY) {
        KePanic("Length exceeds max registry key");
    }

    key = MmAllocateZero(sizeof(struct RegistryKey));
    if(key == NULL) {
        KePanic("Out of memory");
    }

    key->name = MmAllocate(len + 1);
    if(key->name == NULL) {
        KePanic("Out of memory");
    }
    KeCopyMemory(key->name, name, len + 1);

    if(root != NULL) {
        key = KeAddRegistryKeyToGroup(root, key);
    }
    return key;
}

void KeDestroyRegistryKey(
    struct RegistryKey *key)
{
    MmFree(key->name);
    MmFree(key);
    return;
}

#if defined(DEBUG)
#include <debug/printf.h>

void KeDumpRegistryKey(
    const struct RegistryKey *key,
    int level)
{
    size_t i;

    for(i = 0; i < (size_t)level; i++) {
        KeDebugPrint("    ");
    }
    KeDebugPrint("[HKEY] %s\r\n", key->name);
}

void KeDumpRegistryGroup(
    const struct RegistryGroup *root,
    int level)
{
    size_t i;

    for(i = 0; i < (size_t)level; i++) {
        KeDebugPrint("    ");
    }
    KeDebugPrint("[HGROUP] %s\r\n", root->name);

    for(i = 0; i < root->n_groups; i++) {
        KeDumpRegistryGroup(&root->groups[i], level + 1);
    }
    for(i = 0; i < root->n_keys; i++) {
        KeDumpRegistryKey(&root->keys[i], level + 1);
    }
}
#endif
