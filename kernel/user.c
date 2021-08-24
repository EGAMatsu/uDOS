#include <malloc.h>
#include <panic.h>
#include <string.h>
#include <user.h>

#define MAX_USERS 255
#define MAX_GROUPS 16

static struct user g_user_list[MAX_USERS] = {0};
static struct group g_group_list[MAX_GROUPS] = {0};
static user_t current_user = 0;

group_t user_group_create(const char *name) {
  struct group *group = NULL;
  size_t i;

  for (i = 0; i < MAX_GROUPS; i++) {
    if (g_group_list[i].name != NULL) {
      continue;
    }

    group = &g_group_list[i];
    group->name = kmalloc(strlen(name) + 1);
    if (group->name == NULL) {
      kpanic("Out of memory");
    }
    strcpy(group->name, name);
    break;
  }

  if (group == NULL) {
    kpanic("Max number of groups reached");
  }
  return (group_t)i;
}

group_t user_group_get_by_name(const char *name) {
  size_t i;

  for (i = 0; i < MAX_USERS; i++) {
    if (g_group_list[i].name == NULL) {
      continue;
    }

    if (!strcmp(g_group_list[i].name, name)) {
      return (group_t)i;
    }
  }
  return (group_t)-1;
}

struct group *user_group_from_gid(group_t gid) {
  return &g_group_list[gid];
}

user_t user_create(const char *name) {
  struct user *user = NULL;
  size_t i;

  for (i = 0; i < MAX_USERS; i++) {
    if (g_user_list[i].name != NULL) {
      continue;
    }

    user = &g_user_list[i];
    user->name = kmalloc(strlen(name) + 1);
    if (user->name == NULL) {
      kpanic("Out of memory");
    }
    strcpy(user->name, name);
    break;
  }

  if (user == NULL) {
    kpanic("Max number of users reached");
  }
  return (user_t)i;
}

void user_set_current(user_t uid) {
  current_user = uid;
  return;
}

user_t user_get_current(void) { return current_user; }

user_t user_get_by_name(const char *name) {
  size_t i;

  for (i = 0; i < MAX_USERS; i++) {
    if (g_user_list[i].name == NULL) {
      continue;
    }

    if (!strcmp(g_user_list[i].name, name)) {
      return (user_t)i;
    }
  }
  return (user_t)-1;
}

struct user *user_from_uid(user_t uid) {
  return &g_user_list[uid];
}