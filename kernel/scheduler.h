#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <mutex.h>
#include <stddef.h>
#include <s390/context.h>

typedef unsigned short thread_t;
struct scheduler_thread {
    uintptr_t pc;
    void *stack;
    arch_context_t context;
};

typedef unsigned short task_t;
struct scheduler_task {
    char *name;
    struct scheduler_thread *threads;
    size_t n_threads;
};

typedef unsigned short job_t;
struct scheduler_job {
    char *name;
    struct scheduler_task *tasks;
    size_t n_tasks;
    size_t current_task;

    /* Priority of the job */
    signed char priority;
    
    /* Max memory to be used by job */
    size_t max_mem;
};

struct scheduler_job *scheduler_new_job(const char *name, signed char priority,
    size_t max_mem);
struct scheduler_task *scheduler_new_task(struct scheduler_job *job,
    const char *name);
struct scheduler_thread *scheduler_new_thead(struct scheduler_job *job,
    struct scheduler_task *task,size_t stack_size);
void scheduler_schedule(void);

#endif