#ifndef USER_H
#define USER_H

#include <stddef.h>

#define MAX_USERS 64
typedef int8_t user_t;
struct UserAccount {
    struct UserGroup *groups;
    size_t n_groups;
    char *name;
};

#define KeCreateAccount _Zacrta
user_t KeCreateAccount(const char *name);
#define KeSetCurrentAccount _Zasca
void KeSetCurrentAccount(user_t uid);
#define KeGetCurrentAccount _Zagca
user_t KeGetCurrentAccount(void);
#define KeGetAccountByName _Zagabn
user_t KeGetAccountByName(const char *name);
#define KeGetAccountById _Zagabi
struct UserAccount *KeGetAccountById(user_t uid);

#define MAX_GROUPS 8
typedef int8_t group_t;
struct UserGroup {
    user_t *users;
    size_t n_users;
    char *name;
};

#define KeCreateUserGroup _Zacg
group_t KeCreateUserGroup(const char *name);
#define KeGetUserGroupByName _Zaggbn
group_t KeGetUserGroupByName(const char *name);
#define KeGetUserGroupById _Zaggbi
struct UserGroup *KeGetUserGroupById(group_t gid);

#define MAX_SESSIONS 128
typedef int8_t session_t;
struct ClientSession {
    struct UserAccount *user;
};

#endif
