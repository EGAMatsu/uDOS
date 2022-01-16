#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stddef.h>
#include <mutex.h>
#include <mmu.h>
#include <cpu.h>

typedef unsigned short job_t;
struct scheduler_job {
    char *name;
    struct scheduler_task *tasks;
    size_t n_tasks;
    size_t current_task;

    /* Storage space (virtual) */
    isovirt_space vspace;

    /* Priority of the job */
    signed char priority;
    
    /* Max memory to be used by job */
    size_t max_mem;
};

typedef unsigned short task_t;
struct scheduler_task {
    char *name;
    struct scheduler_thread *threads;
    size_t n_threads;
    size_t current_thread;
};

typedef unsigned short thread_t;
struct scheduler_thread {
    unsigned int pc;
    void *stack;
    cpu_context context;
};

#define KeCreateJob _Zshcj
struct scheduler_job *KeCreateJob(const char *name, signed char priority, size_t max_mem);
#define KeCreateTask _Zshctk
struct scheduler_task *KeCreateTask(struct scheduler_job *job, const char *name);
#define KeCreateThread _Zshcth
struct scheduler_thread *KeCreateThread(struct scheduler_job *job, struct scheduler_task *task,size_t stack_size);
job_t KeGetCurrentJobId(void);
void KeSchedule(void);

#endif
