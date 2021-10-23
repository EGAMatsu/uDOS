#ifndef USER_H
#define USER_H

#include <stddef.h>

#define MAX_USERS 64
typedef int8_t user_t;
struct user {
    struct group *groups;
    size_t n_groups;
    char *name;
};

user_t KeCreateUser(const char *name);
void KeSetCurrentUser(user_t uid);
user_t KeGetCurrentUser(void);
user_t KeGetUserByName(const char *name);
struct user *KeGetUserById(user_t uid);

#define MAX_GROUPS 8
typedef int8_t group_t;
struct group {
    user_t *users;
    size_t n_users;
    char *name;
};

group_t KeCreateUserGroup(const char *name);
group_t KeGetUserGroupByName(const char *name);
struct group *KeGetUserGroupById(group_t gid);

#define MAX_SESSIONS 128
typedef int8_t session_t;
struct session {
    struct user *user;
};

#endif