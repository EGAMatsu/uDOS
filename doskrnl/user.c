/* user.c
 *
 * Basic user management system which also has groups, generally it does nothing
 * on it's own but it should be used to implement some form of ACLs on the VFS
 * or to allow certain privileges on a arch dependant basis
 */

#include <mm/mm.h>
#include <debug/panic.h>
#include <memory.h>
#include <user.h>

static struct UserAccount g_user_list[MAX_USERS] = {0};
static struct UserGroup g_group_list[MAX_GROUPS] = {0};
static user_t current_user = 0;

group_t KeCreateUserGroup(
    const char *name)
{
    struct UserGroup *group = NULL;
    size_t i;

    for(i = 0; i < MAX_GROUPS; i++) {
        if(g_group_list[i].name != NULL) {
            continue;
        }

        group = &g_group_list[i];
        group->name = MmAllocate(KeStringLength(name) + 1);
        if(group->name == NULL) {
            KePanic("Out of memory");
        }
        KeCopyString(group->name, name);
        break;
    }

    if(group == NULL) {
        KePanic("Max number of groups reached");
    }
    return (group_t)i;
}

group_t KeGetUserGroupByName(
    const char *name)
{
    size_t i;

    for(i = 0; i < MAX_USERS; i++) {
        if(g_group_list[i].name == NULL) {
            continue;
        }

        if(!KeCompareString(g_group_list[i].name, name)) {
            return (group_t)i;
        }
    }
    return (group_t)-1;
}

struct UserGroup *KeGetUserGroupById(
    group_t gid)
{
    return &g_group_list[gid];
}

user_t KeCreateAccount(
    const char *name)
{
    struct UserAccount *user = NULL;
    size_t i;

    for(i = 0; i < MAX_USERS; i++) {
        if(g_user_list[i].name != NULL) {
            continue;
        }

        user = &g_user_list[i];
        user->name = MmAllocate(KeStringLength(name) + 1);
        if(user->name == NULL) {
            KePanic("Out of memory");
        }
        KeCopyString(user->name, name);
        break;
    }

    if(user == NULL) {
        KePanic("Max number of users reached");
    }
    return (user_t)i;
}

void KeSetCurrentAccount(
    user_t uid)
{
    current_user = uid;
    return;
}

user_t KeGetCurrentAccount(
    void)
{
    return current_user;
}

user_t KeGetAccountByName(
    const char *name)
{
    size_t i;

    for(i = 0; i < MAX_USERS; i++) {
        if(g_user_list[i].name == NULL) {
            continue;
        }

        if(!KeCompareString(g_user_list[i].name, name)) {
            return (user_t)i;
        }
    }
    return (user_t)-1;
}

struct UserAccount *KeGetAccountById(
    user_t uid)
{
    return &g_user_list[uid];
}
