/* scheduler.c
 *
 * Implements the scheduling algorithms for the scheduler
 * 
 * Note that this file does not handle context switching, this is done by the
 * architecture dependent context.c and context.h
 */

#include <scheduler.h>
#include <mm.h>
#include <panic.h>
#include <assert.h>

static struct {
    struct scheduler_job *jobs;
    size_t n_jobs;
    size_t current_job;
}g_scheduler = {0};

struct scheduler_job *KeCreateJob(const char *name, signed char priority, size_t max_mem)
{
    struct scheduler_job *job;

    DEBUG_ASSERT(name != NULL);

    g_scheduler.jobs = MmReallocateArray(g_scheduler.jobs, g_scheduler.n_jobs + 1, sizeof(struct scheduler_job));
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

struct scheduler_task *KeCreateTask(struct scheduler_job *job, const char *name)
{
    struct scheduler_task *task;

    DEBUG_ASSERT(job != NULL && name != NULL);

    job->tasks = MmReallocateArray(job->tasks, job->n_tasks + 1, sizeof(struct scheduler_task));
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

struct scheduler_thread *KeCreateThread(struct scheduler_job *job, struct scheduler_task *task, size_t stack_size)
{
    struct scheduler_thread *thread;

    DEBUG_ASSERT(job != NULL && task != NULL);

    task->threads = MmReallocateArray(task->threads, task->n_threads + 1, sizeof(struct scheduler_thread));
    if(task->threads == NULL) {
        KePanic("Out of memory");
    }
    thread = &task->threads[task->n_threads++];
    KeSetMemory(thread, 0, sizeof(struct scheduler_thread));

    /* Allocate stack for this thread (the stack is local to each thread) */
    thread->stack = MmAllocatePhysical(stack_size, 8);
    if(thread->stack == NULL) {
        KePanic("Out of memory");
    }
    KeSetMemory(thread->stack, 0, stack_size);

    /* R13 is used as a stack pointer, now we have to setup a few things up */
    thread->context.r13 = (unsigned int)thread->stack + stack_size;

    /* stack+76 should point to stack+180 (because this would be the next frame!) */
    *((uint32_t *)(&((uint8_t *)thread->stack)[76])) = (&((uint8_t *)thread->stack)[180]);
    *((uint32_t *)(&((uint8_t *)thread->stack)[8])) = (&((uint8_t *)thread->stack)[180]);

    /* Set backchain to 0 for stack unwinding */
    *((uint32_t *)(&((uint8_t *)thread->stack)[4])) = NULL;
    *((uint32_t *)(&((uint8_t *)thread->stack)[8])) = NULL;

    /* Set entry point on R15 */
    return thread;
}

job_t KeGetCurrentJobId(void)
{
    DEBUG_ASSERT(!(g_scheduler.current_job > g_scheduler.n_jobs));
    return (job_t)g_scheduler.current_job;
}

void KeSchedule(void)
{
    struct scheduler_job *job;
    struct scheduler_task *task;
    struct scheduler_thread *old_thread, *new_thread;

    DEBUG_ASSERT(!(g_scheduler.current_job > g_scheduler.n_jobs));
    if(g_scheduler.current_job >= g_scheduler.n_jobs) {
        g_scheduler.current_job = 0;
    }
    job = &g_scheduler.jobs[g_scheduler.current_job++];

    DEBUG_ASSERT(!(job->current_task > job->n_tasks));
    if(job->current_task >= job->n_tasks) {
        job->current_task = 0;
    }
    task = &job->tasks[job->current_task++];

    DEBUG_ASSERT(!(task->current_thread > task->n_threads));
    if(task->current_thread >= task->n_threads) {
        task->current_thread = 0;
    }
    old_thread = &task->threads[task->current_thread++];

    DEBUG_ASSERT(!(task->current_thread > task->n_threads));
    if(task->current_thread >= task->n_threads) {
        task->current_thread = 0;
    }
    new_thread = &task->threads[task->current_thread++];

    HwSwitchThreadContext(old_thread, new_thread);
    return;
}
