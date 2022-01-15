#ifndef MUTEX_H
#define MUTEX_H

typedef volatile int mutex_t;

int KeTryLockMutex(mutex_t *mutex);
void KeLockMutex(mutex_t *mutex);
void KeUnlockMutex(mutex_t *mutex);

#endif
