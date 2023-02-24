/*
*
* Copyright (c) 2017-2021 Texas Instruments Incorporated
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

typedef struct _tivx_queue_context *tivx_queue_context;

typedef struct _tivx_queue_context {

  pthread_mutex_t lock;
  pthread_cond_t  condGet;
  pthread_cond_t  condPut;

} tivx_queue_context_t;


vx_status tivxQueueCreate(
    tivx_queue *queue, uint32_t max_elements, uintptr_t *queue_memory,
    uint32_t flags)
{
    vx_status status = (vx_status)VX_FAILURE;
    tivx_queue_context context = NULL;

    if ((NULL != queue) && (NULL != queue_memory) && (0U != max_elements))
    {
        /*
         * init queue to 0's
         */
        memset(queue, 0, sizeof(tivx_queue));

        /*
         * init queue with user parameters
         */
        queue->max_ele = max_elements;
        queue->flags = flags;

        queue->queue = queue_memory;

        queue->context = malloc(sizeof(tivx_queue_context_t));

        context = queue->context;

        if(queue->context==NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "queue memory allocation failed\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
        else
        {
            pthread_mutexattr_t mutex_attr;

            status = (vx_status)VX_SUCCESS;

            status |= pthread_mutexattr_init(&mutex_attr);
            status |= pthread_mutex_init(&context->lock, &mutex_attr);

            pthread_mutexattr_destroy(&mutex_attr);

            if(status==0)
            {
                if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET)
                {
                    pthread_condattr_t cond_attr;

                    /*
                     * user requested block on que get
                     */

                    /*
                     * create cond for it
                     */
                    status |= pthread_condattr_init(&cond_attr);
                    status |= pthread_cond_init(&context->condGet, &cond_attr);

                    pthread_condattr_destroy(&cond_attr);
                }
                if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT)
                {
                    pthread_condattr_t cond_attr;

                    /*
                     * user requested block on que put
                     */

                    /*
                     * create cond for it
                     */
                    status |= pthread_condattr_init(&cond_attr);
                    status |= pthread_cond_init(&context->condPut, &cond_attr);

                    pthread_condattr_destroy(&cond_attr);
                }
            }
            if ((vx_status)VX_SUCCESS == status)
            {
                queue->blockedOnGet = (vx_bool)vx_false_e;
                queue->blockedOnPut = (vx_bool)vx_false_e;
            }
            else
            {
                pthread_mutex_destroy(&context->lock);
                free(queue->context);
                queue->context = NULL;
            }
        }
    }

    return (status);
}

vx_status tivxQueueDelete(tivx_queue *queue)
{
    vx_status status = (vx_status)VX_FAILURE;
    tivx_queue_context context;

    if (NULL != queue && queue->context != NULL)
    {
        context = queue->context;

        VX_PRINT(VX_ZONE_INFO, "if this hangs, please ensure all application threads have been destroyed\n");
        if ((queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET))
        {
            pthread_cond_destroy(&(context)->condGet);
        }
        if ((queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT))
        {
            pthread_cond_destroy(&(context)->condPut);
        }
        pthread_mutex_destroy(&context->lock);

        free(context);
        queue->context = NULL;

        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

vx_status tivxQueuePut(tivx_queue *queue, uintptr_t data, uint32_t timeout)
{
    vx_status status = (vx_status)VX_FAILURE;
    volatile vx_bool do_break = (vx_bool)vx_false_e;
    tivx_queue_context context = NULL;

    if(queue && queue->context)
    {
        context = queue->context;

        status = pthread_mutex_lock(&context->lock);
        if(status==0)
        {
            do
            {
                if (queue->count < queue->max_ele)
                {
                    /* insert element */
                    queue->queue[queue->cur_wr] = data;

                    /* increment put pointer */
                    queue->cur_wr = (queue->cur_wr + 1U) % queue->max_ele;

                    /* increment count of number element in que */
                    queue->count++;

                    /* mark status as success */
                    status = (vx_status)VX_SUCCESS;

                    if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET)
                    {
                        /* blocking on que get enabled */

                        /* post cond to unblock, blocked tasks */
                        pthread_cond_signal(&context->condGet);
                    }

                    /* exit, with success */
                    do_break = (vx_bool)vx_true_e;
                }
                else
                {
                    /* que is full */
                    if (timeout == TIVX_EVENT_TIMEOUT_NO_WAIT)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "queue is full\n");
                        status = (vx_status)VX_FAILURE;
                        do_break = (vx_bool)vx_true_e; /* non-blocking, so exit with error */
                    }
                    else if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT)
                    {
                        /* blocking on que put enabled */
                        queue->blockedOnPut = (vx_bool)vx_true_e;
                        pthread_cond_wait(&context->condPut, &context->lock);
                        queue->blockedOnPut = (vx_bool)vx_false_e;
                    }
                    else
                    {
                        /* blocking on que put disabled */

                        /* exit with error */
                        VX_PRINT(VX_ZONE_ERROR, "blocking on queue is disabled\n");
                        status = (vx_status)VX_FAILURE;
                        do_break = (vx_bool)vx_true_e;
                    }
                }

                if ((vx_bool)vx_true_e == do_break)
                {
                    break;
                }
            }
            while (1);

            status |= pthread_mutex_unlock(&context->lock);
        }
    }

    return (status);
}

vx_status tivxQueueGet(tivx_queue *queue, uintptr_t *data, uint32_t timeout)
{
    vx_status status = (vx_status)VX_FAILURE;/* init status to error */
    volatile vx_bool do_break = (vx_bool)vx_false_e;
    tivx_queue_context context = NULL;

    if(queue && queue->context)
    {
        context = queue->context;

        status = pthread_mutex_lock(&context->lock);
        if(status==0)
        {
            do
            {
                if (queue->count > 0U)
                {
                    /* extract the element */
                    *data = queue->queue[queue->cur_rd];

                    /* increment get pointer */
                    queue->cur_rd = (queue->cur_rd + 1U) % queue->max_ele;

                    /* decrmeent number of elements in que */
                    queue->count--;

                    /* set status as success */
                    status = (vx_status)VX_SUCCESS;

                    if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT)
                    {
                        /* post cond to unblock, blocked tasks */
                        pthread_cond_signal(&context->condPut);
                    }

                    /* exit with success */
                    do_break = (vx_bool)vx_true_e;
                }
                else
                {
                    /* no elements or not enough element in que to extract */
                    if (timeout == TIVX_EVENT_TIMEOUT_NO_WAIT)
                    {
                        /* VX_PRINT(VX_ZONE_ERROR, "no elements found in que\n"); */
                        status = (vx_status)VX_FAILURE;
                        do_break = (vx_bool)vx_true_e; /* non-blocking, exit with error */
                    }
                    else
                    if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET)
                    {
                        /* blocking on que get enabled */

                        queue->blockedOnGet = (vx_bool)vx_true_e;
                        pthread_cond_wait(&context->condGet, &context->lock);
                        queue->blockedOnGet = (vx_bool)vx_false_e;
                        /* received semaphore, check que again */
                    }
                    else
                    {
                        /* blocking on que get disabled */

                        /* exit with error */
                        VX_PRINT(VX_ZONE_ERROR, "blocking on que get disabled\n");
                        status = (vx_status)VX_FAILURE;
                        do_break = (vx_bool)vx_true_e;
                    }
                }

                if ((vx_bool)vx_true_e == do_break)
                {
                    break;
                }
            }
            while (1);
            status |= pthread_mutex_unlock(&context->lock);
        }
    }
    return (status);
}

vx_bool tivxQueueIsEmpty(const tivx_queue *queue)
{
    vx_bool is_empty = (vx_bool)vx_true_e;
    tivx_queue_context context = NULL;
    vx_status status;

    if(queue && queue->context)
    {
        context = queue->context;

        status = pthread_mutex_lock(&context->lock);
        if(status==0)
        {
            if (queue->count == 0U)
            {
                is_empty = (vx_bool)vx_true_e;
            }
            else
            {
                is_empty = (vx_bool)vx_false_e;
            }

            pthread_mutex_unlock(&context->lock);
        }
    }
    return (is_empty);
}

vx_status tivxQueuePeek(const tivx_queue *queue, uintptr_t *data)
{
    vx_status status = (vx_status)VX_FAILURE;/* init status to error */
    tivx_queue_context context = NULL;

    if(queue && queue->context)
    {
        context = queue->context;

        status = pthread_mutex_lock(&context->lock);
        if(status==0)
        {
            if (queue->count > 0U)
            {
                /* 'peek' the element but dont extract it */
                *data = queue->queue[queue->cur_rd];

                /* set status as success */
                status = (vx_status)VX_SUCCESS;
            }

            status = pthread_mutex_unlock(&context->lock);
        }
    }
    return (status);
}
