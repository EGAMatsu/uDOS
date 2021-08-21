#include <mutex.h>

int mutex_try_lock(mutex_t *mutex) {
    return (int)__sync_bool_compare_and_swap(mutex, 0, 1);
}

void mutex_lock(mutex_t *mutex) {
    while(!mutex_try_lock(mutex)) {
        /* Loop until mutex is aquired */
    }
    return;
}

void mutex_unlock(mutex_t *mutex) {
    *mutex = 0;
    return;
}