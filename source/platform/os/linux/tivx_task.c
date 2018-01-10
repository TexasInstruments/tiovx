/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#include <vx_internal.h>
#include <pthread.h>

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>   /* for nanosleep */

int nanosleep(const struct timespec *req, struct timespec *rem);

#else
#include <unistd.h> /* for usleep */

extern int usleep (__useconds_t __useconds);

#endif

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
            VX_PRINT(VX_ZONE_ERROR, "tivxTaskCreate: Context memory allocation failed\n");
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
            else
            {
                free(context);
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxTaskCreate: Task or params arguments are NULL\n");
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
#if _POSIX_C_SOURCE >= 199309L
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
#else
    usleep(msec * 1000);
#endif
}

