/* mutex.c
 * 
 * Implements a basic locking mutex
 */

#include <mutex.h>

int KeTryLockMutex(
    mutex_t *mutex)
{
    /*return (int)__sync_bool_compare_and_swap(mutex, 0, 1);*/
    return 0;
}

void KeLockMutex(
    mutex_t *mutex)
{
    while(!KeTryLockMutex(mutex)) {
        /* Loop until mutex is aquired */
    }
    return;
}

void KeUnlockMutex(
    mutex_t *mutex)
{
    *mutex = 0;
    return;
}
