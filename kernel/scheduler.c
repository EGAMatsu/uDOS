/* scheduler.c
 *
 * Implements the scheduling algorithms for the scheduler
 * 
 * Note that this file does not handle context switching, this is done by the
 * architecture dependent context.c and context.h
 */

#include <scheduler.h>
#include <alloc.h>
#include <panic.h>

static struct {
    struct scheduler_job *jobs;
    size_t n_jobs;
    size_t current_job;
}g_scheduler = {0};

struct scheduler_job *scheduler_new_job(
    const char *name,
    signed char priority,
    size_t max_mem)
{
    struct scheduler_job *job;

    g_scheduler.jobs = krealloc_array(g_scheduler.jobs, g_scheduler.n_jobs + 1,
        sizeof(struct scheduler_job));
    if(g_scheduler.jobs == NULL) {
        kpanic("Out of memory");
    }
    job = &g_scheduler.jobs[g_scheduler.n_jobs++];
    memset(job, 0, sizeof(struct scheduler_job));

    job->name = kmalloc(strlen(name) + 1);
    if(job->name == NULL) {
        kpanic("Out of memory");
    }
    strcpy(job->name, name);

    job->priority = priority;
    job->max_mem = max_mem;
    return job;
}

struct scheduler_task *scheduler_new_task(
    struct scheduler_job *job,
    const char *name)
{
    struct scheduler_task *task;

    job->tasks = krealloc_array(job->tasks, job->n_tasks + 1,
        sizeof(struct scheduler_task));
    if(job->tasks == NULL) {
        kpanic("Out of memory");
    }
    task = &job->tasks[job->n_tasks++];
    memset(task, 0, sizeof(struct scheduler_task));

    task->name = kmalloc(strlen(name) + 1);
    if(task->name == NULL) {
        kpanic("Out of memory");
    }
    strcpy(task->name, name);
    return task;
}

struct scheduler_thread *scheduler_new_thead(
    struct scheduler_job *job,
    struct scheduler_task *task,
    size_t stack_size)
{
    struct scheduler_thread *thread;

    task->threads = krealloc_array(task->threads, task->n_threads + 1,
        sizeof(struct scheduler_thread));
    if(task->threads == NULL) {
        kpanic("Out of memory");
    }
    thread = &task->threads[task->n_threads++];

    /* Check if we are below our memory quota (if any) */
    if(job->max_mem != 0 && stack_size > job->max_mem) {
        kpanic("Out of stack for job %s", job->name);
    }

    thread->stack = kmalloc(stack_size);
    if(thread->stack == NULL) {
        kpanic("Out of memory");
    }
    job->max_mem -= stack_size;
    return thread;
}

void scheduler_schedule(
    void)
{
    struct scheduler_job *job;
    struct scheduler_task *task;
    struct scheduler_thread *thread;

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

    /* Save context to current thread (obtained from the scratch frame) */
    thread = &task->threads[task->current_thread];
    memcpy(&thread->context, context_scratch_frame(), sizeof(arch_context_t));
    task->current_thread++;

    /* Load new thread context into the scratch frame */
    thread = &task->threads[task->current_thread];
    memcpy(context_scratch_frame(), &thread->context, sizeof(arch_context_t));
    return;
}