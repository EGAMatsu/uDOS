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

#include <alloc.h>
#include <panic.h>
#include <registry.h>
#include <string.h>

/* TODO: Initialize root group to 0 */
struct reg_group *g_root_group;

void reg_init(
    void)
{
    g_root_group = reg_create_group(NULL, "HKEY");
    return;
}

struct reg_group *reg_get_root_group(
    void)
{
    return g_root_group;
}

struct reg_group *reg_create_group(
    struct reg_group *root,
    const char *name)
{
    struct reg_group *group;
    size_t len;

    len = strlen(name);
    if(len >= MAX_REGISTRY_NAME) {
        kpanic("Length exceeds max registry name");
    }

    group = kzalloc(sizeof(struct reg_group));
    if(group == NULL) {
        kpanic("Out of memory");
    }

    group->name = kmalloc(len + 1);
    if(group->name == NULL) {
        kpanic("Out of memory");
    }
    memcpy(group->name, name, len + 1);

    if(root != NULL) {
        group = reg_add_subgroup_to_group(root, group);
    }
    return group;
}

struct reg_key *reg_resolve_path(
    struct reg_group *root,
    const char *path)
{
    const struct reg_group *group;
    const struct reg_key *key;
    char *tmpbuf, *buf;
    size_t len;

    len = strlen(path);
    if(len >= MAX_REGISTRY_PATH) {
        kpanic("Length exceeds max registry path");
    }

    tmpbuf = kmalloc(len + 1);
    if(tmpbuf == NULL) {
        kpanic("Out of memory");
    }
    strcpy(tmpbuf, path);

    group = root;
    key = NULL;
    buf = tmpbuf;
    while(*buf != '\0') {
        const struct reg_group *old_group;
        const char *old_tmpbuf;
        char *name;
        size_t name_len;

        /* Find the next separator, these separators allow for namespaces
     * which are then parsed into groups -> keys */
        old_tmpbuf = buf;
        buf = strpbrk(buf, "_-:");
        if(buf == NULL) {
            /* ifno separators were found it means we are at the key */
            name_len = strlen(old_tmpbuf);
        } else {
            name_len = (size_t)((ptrdiff_t)buf - (ptrdiff_t)old_tmpbuf);
            if(!name_len) {
                kpanic("Zero-lenght name");
            }
        }

        /* Skip the separator itself */
        buf++;

        name = kmalloc(name_len + 1);
        if(name == NULL) {
            kpanic("Ouf of memory");
        }
        memcpy(name, old_tmpbuf, name_len);
        old_group = group;
        group = reg_find_subgroup_in_group(group, name);
        if(group == NULL) {
            group = old_group;
            key = reg_find_key_in_group(group, name);
            if(key == NULL) {
                kpanic("Registry %s not found", name);
            }
            kfree(name);
            break;
        }
        kfree(name);
    }
    kfree(tmpbuf);
    return (struct reg_key *)key;
}

struct reg_group *reg_add_subgroup_to_group(
    struct reg_group *root,
    struct reg_group *subgroup)
{
    root->groups = krealloc_array(root->groups, root->n_groups + 1,
                                  sizeof(struct reg_group));
    if(root->groups == NULL) {
        kpanic("Out of memory");
    }
    memcpy(&root->groups[root->n_groups++], subgroup, sizeof(struct reg_group));
    return &root->groups[root->n_groups - 1];
}

struct reg_key *reg_add_key_to_group(
    struct reg_group *root,
    struct reg_key *key)
{
    root->keys =
        krealloc_array(root->keys, root->n_keys + 1, sizeof(struct reg_key));
    if(root->keys == NULL) {
        kpanic("Out of memory");
    }
    memcpy(&root->keys[root->n_keys++], key, sizeof(struct reg_key));
    return &root->keys[root->n_keys - 1];
}

struct reg_group *reg_find_subgroup_in_group(
    const struct reg_group *root,
    const char *name)
{
    size_t i, len;

    len = strlen(name);
    if(len >= MAX_REGISTRY_KEY) {
        kpanic("Length exceeds max registry key");
    }

    for(i = 0; i < root->n_groups; i++) {
        struct reg_group *subgroup = &root->groups[i];
        if(!strcmp(subgroup->name, name)) {
            return subgroup;
        }
    }
    return NULL;
}

struct reg_key *reg_find_key_in_group(
    const struct reg_group *root,
    const char *name)
{
    size_t i, len;

    len = strlen(name);
    if(len >= MAX_REGISTRY_NAME) {
        kpanic("Length exceeds max registry name");
    }

    for(i = 0; i < root->n_keys; i++) {
        const struct reg_key *key = &root->keys[i];
        if(!strcmp(key->name, name)) {
            return (struct reg_key *)key;
        }
    }
    return NULL;
}

void reg_destroy_group(
    struct reg_group *group)
{
    kfree(group->name);
    kfree(group);
    return;
}

struct reg_key *reg_create_key(
    struct reg_group *root,
    const char *name)
{
    struct reg_key *key;
    size_t len;

    len = strlen(name);
    if(len >= MAX_REGISTRY_KEY) {
        kpanic("Length exceeds max registry key");
    }

    key = kzalloc(sizeof(struct reg_key));
    if(key == NULL) {
        kpanic("Out of memory");
    }

    key->name = kmalloc(len + 1);
    if(key->name == NULL) {
        kpanic("Out of memory");
    }
    memcpy(key->name, name, len + 1);

    if(root != NULL) {
        key = reg_add_key_to_group(root, key);
    }
    return key;
}

void reg_destroy_key(
    struct reg_key *key)
{
    kfree(key->name);
    kfree(key);
    return;
}

#include <printf.h>
void reg_dump_key(
    const struct reg_key *key,
    int level)
{
    size_t i;

    for(i = 0; i < (size_t)level; i++) {
        kprintf("    ");
    }
    kprintf("[HKEY] %s\n", key->name);
}

void reg_dump_group(
    const struct reg_group *root,
    int level)
{
    size_t i;

    for(i = 0; i < (size_t)level; i++) {
        kprintf("    ");
    }
    kprintf("[HGROUP] %s\n", root->name);

    for(i = 0; i < root->n_groups; i++) {
        reg_dump_group(&root->groups[i], level + 1);
    }
    for(i = 0; i < root->n_keys; i++) {
        reg_dump_key(&root->keys[i], level + 1);
    }
}