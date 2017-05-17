/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */
#include <vx_internal.h>
#include <pthread.h>
#include <time.h>

int nanosleep (const struct timespec *requested_time, struct timespec *remaining);

#define PRI_MAX  sched_get_priority_max(SCHED_FIFO)
#define PRI_MIN  sched_get_priority_min(SCHED_FIFO)

typedef struct _tivx_task_context *tivx_task_context;

typedef struct _tivx_task_context
{
    pthread_t hndl;

} tivx_task_context_t;

void tivxTaskSetDefaultCreateParams(tivx_task_create_params_t *params)
{
    if (NULL != params)
    {
        memset(params, 0, sizeof(tivx_task_create_params_t));

        params->core_affinity = TIVX_TASK_AFFINITY_ANY;
        params->priority = TIVX_TASK_PRI_LOWEST;
    }
}

static void *tivxTaskMain(void *arg)
{
    tivx_task *task = (tivx_task*)arg;

    if( task && task->task_func)
    {
        task->task_func(task->app_var);
    }

    return NULL;
}

vx_status tivxTaskCreate(tivx_task *task, tivx_task_create_params_t *params)
{
    vx_status status = VX_SUCCESS;

    if ((NULL != task) && (NULL != params))
    {
        tivx_task_context context;

        task->tsk_handle = NULL;

        context = malloc(sizeof(tivx_task_context_t));
        if(context == NULL)
        {
            status = VX_ERROR_NO_MEMORY;
        }
        else
        {
            pthread_attr_t thread_attr;
            struct sched_param schedprm;
            uint32_t pri;

            task->stack_ptr = params->stack_ptr;
            task->stack_size = params->stack_size;
            task->core_affinity = params->core_affinity;
            task->priority = params->priority;
            task->task_func = params->task_main;
            task->app_var = params->app_var;

            status = pthread_attr_init(&thread_attr);

            if(status==0)
            {
                if(task->stack_size>0)
                {
                    status |= pthread_attr_setstacksize(&thread_attr, task->stack_size);
                }
                status |= pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);

                pri = PRI_MIN;
                if(task->priority==TIVX_TASK_PRI_HIGHEST)
                {
                    pri = PRI_MAX;
                }
                else
                if(task->priority==TIVX_TASK_PRI_LOWEST)
                {
                    pri = PRI_MIN;
                }

                schedprm.sched_priority = pri;
                status |= pthread_attr_setschedparam(&thread_attr, &schedprm);

                if(status==0)
                {
                    status = pthread_create(&context->hndl, &thread_attr, tivxTaskMain, task);
                }
                pthread_attr_destroy(&thread_attr);
            }
            if (status == 0)
            {
                task->tsk_handle = (void *)context;
            }
        }
    }
    else
    {
        status = VX_FAILURE;
    }

    return (status);
}

vx_status tivxTaskDelete(tivx_task *task)
{
    vx_status status = VX_SUCCESS;

    if(task && task->tsk_handle)
    {
        tivx_task_context context;
        void *ret_val;

        context = (tivx_task_context)task->tsk_handle;

        pthread_cancel(context->hndl);
        pthread_join(context->hndl, &ret_val);

        free(context);
        task->tsk_handle = NULL;
    }

    return (status);
}

void tivxTaskWaitMsecs(uint32_t msec)
{
    struct timespec delay_time, remain_time;
    int ret;

    delay_time.tv_sec  = msec/1000;
    delay_time.tv_nsec = (msec%1000)*1000000;

    do
    {
        ret = nanosleep(&delay_time, &remain_time);
        if(ret < 0 && remain_time.tv_sec > 0 && remain_time.tv_nsec > 0)
        {
            /* restart for remaining time */
            delay_time = remain_time;
        }
        else
        {
            break;
        }
    } while(1);
}

