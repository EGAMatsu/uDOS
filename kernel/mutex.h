#ifndef MUTEX_H
#define MUTEX_H

typedef volatile int mutex_t;

int mutex_try_lock(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);

#endif