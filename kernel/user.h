#ifndef USER_H
#define USER_H

#include <stddef.h>

#define MAX_USERS 32
#define MAX_GROUPS 8

typedef int user_t;
typedef int group_t;

struct group {
    char *name;
    user_t *users;
    size_t n_users;
};

group_t user_group_create(const char *name);
group_t user_group_get_by_name(const char *name);
struct group *user_group_from_gid(group_t gid);

struct user {
    char *name;
};

user_t user_create(const char *name);
void user_set_current(user_t uid);
user_t user_get_current(void);
user_t user_get_by_name(const char *name);
struct user *user_from_uid(user_t uid);

#endif