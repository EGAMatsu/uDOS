#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stddef.h>
#include <mutex.h>
#include <arch/context.h>
#include <arch/mmu.h>

typedef unsigned short thread_t;
struct SchedulerThread {
    uintptr_t pc;
    void *stack;
    arch_context_t context;
};

typedef unsigned short task_t;
struct SchedulerTask {
    char *name;
    struct SchedulerThread *threads;
    size_t n_threads;
    size_t current_thread;
};

typedef unsigned short job_t;
struct SchedulerJob {
    char *name;
    struct SchedulerTask *tasks;
    size_t n_tasks;
    size_t current_task;

    /* Storage space (virtual) */
    virtual_space_t vspace;

    /* Priority of the job */
    signed char priority;
    
    /* Max memory to be used by job */
    size_t max_mem;
};

struct SchedulerJob *KeCreateJob(const char *name, signed char priority,
    size_t max_mem);
struct SchedulerTask *KeCreateTask(struct SchedulerJob *job,
    const char *name);
struct SchedulerThread *KeCreateThread(struct SchedulerJob *job,
    struct SchedulerTask *task,size_t stack_size);
job_t KeGetCurrentJobId(void);
void KeSchedule(void);

#endif