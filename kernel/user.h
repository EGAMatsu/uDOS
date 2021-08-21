#ifndef USER_H
#define USER_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef int user_t;
typedef int group_t;

struct user {
    char *name;
};

struct group {
    char *name;
    user_t *users;
    size_t n_users;
};

group_t user_group_create(const char *name);
group_t user_group_get_by_name(const char *name);
struct group *user_group_from_gid(group_t gid);

user_t user_create(const char *name);
void user_set_current(user_t uid);
user_t user_get_current(void);
user_t user_get_by_name(const char *name);
struct user *user_from_uid(user_t uid);

#ifdef __cplusplus
}
#endif
#endif