/* scheduler.c
 *
 * Implements the scheduling algorithms for the scheduler
 * 
 * Note that this file does not handle context switching, this is done by the
 * architecture dependent context.c and context.h
 */

#include <scheduler.h>
#include <mm/mm.h>
#include <debug/panic.h>
#include <debug/assert.h>

static struct {
    struct scheduler_job *jobs;
    size_t n_jobs;
    size_t current_job;
}g_scheduler = {0};

struct scheduler_job *KeCreateJob(
    const char *name,
    signed char priority,
    size_t max_mem)
{
    struct scheduler_job *job;

    DEBUG_ASSERT(name != NULL);

    g_scheduler.jobs = MmReallocateArray(g_scheduler.jobs, g_scheduler.n_jobs + 1,
        sizeof(struct scheduler_job));
    if(g_scheduler.jobs == NULL) {
        KePanic("Out of memory");
    }
    job = &g_scheduler.jobs[g_scheduler.n_jobs++];
    KeSetMemory(job, 0, sizeof(struct scheduler_job));

    job->name = MmAllocate(KeStringLength(name) + 1);
    if(job->name == NULL) {
        KePanic("Out of memory");
    }
    KeCopyString(job->name, name);

    job->priority = priority;
    job->max_mem = max_mem;
    return job;
}

struct scheduler_task *KeCreateTask(
    struct scheduler_job *job,
    const char *name)
{
    struct scheduler_task *task;

    DEBUG_ASSERT(job != NULL && name != NULL);

    job->tasks = MmReallocateArray(job->tasks, job->n_tasks + 1,
        sizeof(struct scheduler_task));
    if(job->tasks == NULL) {
        KePanic("Out of memory");
    }
    task = &job->tasks[job->n_tasks++];
    KeSetMemory(task, 0, sizeof(struct scheduler_task));

    task->name = MmAllocate(KeStringLength(name) + 1);
    if(task->name == NULL) {
        KePanic("Out of memory");
    }
    KeCopyString(task->name, name);
    return task;
}

struct scheduler_thread *KeCreateThread(
    struct scheduler_job *job,
    struct scheduler_task *task,
    size_t stack_size)
{
    struct scheduler_thread *thread;

    DEBUG_ASSERT(job != NULL && task != NULL);

    task->threads = MmReallocateArray(task->threads, task->n_threads + 1,
        sizeof(struct scheduler_thread));
    if(task->threads == NULL) {
        KePanic("Out of memory");
    }
    thread = &task->threads[task->n_threads++];
    KeSetMemory(thread, 0, sizeof(struct scheduler_thread));

    /* Allocate stack for this thread (the stack is local to each thread) */
    thread->stack = MmAllocateZero(stack_size);
    if(thread->stack == NULL) {
        KePanic("Out of memory");
    }

    thread->context.r15 = (unsigned int)thread->stack + stack_size;
    return thread;
}

job_t KeGetCurrentJobId(
    void)
{
    return (job_t)g_scheduler.current_job;
}

void KeSchedule(
    void)
{
    struct scheduler_job *job;
    struct scheduler_task *task;
    struct scheduler_thread *old_thread, *new_thread;

    if(g_scheduler.current_job >= g_scheduler.n_jobs) {
        g_scheduler.current_job = 0;
    }
    job = &g_scheduler.jobs[g_scheduler.current_job++];

    if(job->current_task >= job->n_tasks) {
        job->current_task = 0;
    }
    task = &job->tasks[job->current_task++];

    if(task->current_thread >= task->n_threads) {
        task->current_thread = 0;
    }
    old_thread = &task->threads[task->current_thread++];

    if(task->current_thread >= task->n_threads) {
        task->current_thread = 0;
    }
    new_thread = &task->threads[task->current_thread++];

    HwSwitchThreadContext(old_thread, new_thread);
    return;
}