#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stddef.h>
#include <Mutex.h>
#include <Arch/Mmu.h>

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

typedef unsigned short task_t;
struct SchedulerTask {
    char *name;
    struct SchedulerThread *threads;
    size_t n_threads;
    size_t current_thread;
};

#include <Arch/Context.h>
typedef unsigned short thread_t;
struct SchedulerThread {
    unsigned int pc;
    void *stack;
    arch_context_t context;
};

#define KeCreateJob _Zshcj
struct SchedulerJob *KeCreateJob(const char *name, signed char priority,
    size_t max_mem);
#define KeCreateTask _Zshctk
struct SchedulerTask *KeCreateTask(struct SchedulerJob *job,
    const char *name);
#define KeCreateThread _Zshcth
struct SchedulerThread *KeCreateThread(struct SchedulerJob *job,
    struct SchedulerTask *task,size_t stack_size);
job_t KeGetCurrentJobId(void);
void KeSchedule(void);

#endif
