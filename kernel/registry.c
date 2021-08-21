#include <registry.h>
#include <malloc.h>
#include <string.h>
#include <panic.h>

/* TODO: Initialize root group to 0 */
struct reg_group *g_root_group;

void reg_init(void) {
    g_root_group = reg_create_group(NULL, "HKEY");
    return;
}

struct reg_group *reg_get_root_group(void) {
    return g_root_group;
}

struct reg_group *reg_create_group(struct reg_group *root, const char *name) {
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

struct reg_key *reg_resolve_path(struct reg_group *root, const char *path) {
    struct reg_group *group;
    struct reg_key *key;
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
        struct reg_group *old_group;
        char *old_tmpbuf, *name;
        size_t name_len;

        /* Find the next separator, these separators allow for namespaces 
         * which are then parsed into groups -> keys */
        old_tmpbuf = buf;
        buf = strpbrk(buf, "_-:");
        if(buf == NULL) {
            /* If no separators were found it means we are at the key */
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
    return key;
}

struct reg_group *reg_add_subgroup_to_group(struct reg_group *root, struct reg_group *subgroup) {
    root->groups = krealloc_array(root->groups, root->n_groups + 1, sizeof(struct reg_group));
    if(root->groups == NULL) {
        kpanic("Out of memory");
    }
    memcpy(&root->groups[root->n_groups++], subgroup, sizeof(struct reg_group));
    return &root->groups[root->n_groups - 1];
}

struct reg_key *reg_add_key_to_group(struct reg_group *root, struct reg_key *key) {
    root->keys = krealloc_array(root->keys, root->n_keys + 1, sizeof(struct reg_key));
    if(root->keys == NULL) {
        kpanic("Out of memory");
    }
    memcpy(&root->keys[root->n_keys++], key, sizeof(struct reg_key));
    return &root->keys[root->n_keys - 1];
}

struct reg_group *reg_find_subgroup_in_group(struct reg_group *root, const char *name) {
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

struct reg_key *reg_find_key_in_group(struct reg_group *root, const char *name) {
    size_t i, len;

    len = strlen(name);
    if(len >= MAX_REGISTRY_NAME) {
        kpanic("Length exceeds max registry name");
    }

    for(i = 0; i < root->n_keys; i++) {
        struct reg_key *key = &root->keys[i];
        if(!strcmp(key->name, name)) {
            return key;
        }
    }
    return NULL;
}

void reg_destroy_group(struct reg_group *group) {
    kfree(group->name);
    kfree(group);
    return;
}

struct reg_key *reg_create_key(struct reg_group *root, const char *name) {
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

void reg_destroy_key(struct reg_key *key) {
    kfree(key->name);
    kfree(key);
    return;
}

#include <printf.h>
void reg_dump_key(struct reg_key *key, int level) {
    size_t i;
    for(i = 0; i < (size_t)level; i++) {
        kprintf("    ");
    }
    kprintf("[HKEY] %s\n", key->name);
}

void reg_dump_group(struct reg_group *root, int level) {
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