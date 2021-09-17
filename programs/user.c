#include <printf.h>
#include <user.h>

int main(int argc, char **argv) {
    if (argc <= 1) {
        const struct user *c_user;
        c_user = user_from_uid(user_get_current());
        kprintf("%s\n", c_user->name);
    } else {
        user_t uid = (user_t)-1;
        if (argv[1][0] >= '0' && argv[1][0] <= '9') {
            uid = user_get_by_name(user_from_uid(atoi(argv[1]))->name);
        } else {
            uid = user_get_by_name(argv[1]);
        }

        if (uid == (user_t)-1) {
            kprintf("User %s does not exist\n", argv[1]);
            return -1;
        } else {
            user_set_current(uid);
        }
    }
    return 0;
}