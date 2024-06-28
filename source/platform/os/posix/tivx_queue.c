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

#include <tivx_platform_posix.h>

void init_queue(tivx_queue ** const queue);

void init_queue(tivx_queue ** const queue)
{
    (*queue)->cur_rd = (uint32_t)0;
    (*queue)->cur_wr = (uint32_t)0;
    (*queue)->count = (uint32_t)0;
    (*queue)->max_ele = (uint32_t)0;
    (*queue)->queue = NULL;
    (*queue)->block_rd = NULL;
    (*queue)->block_wr = NULL;
    (*queue)->lock = NULL;
    (*queue)->context = NULL;
    (*queue)->flags = (uint32_t)0;
    (*queue)->blockedOnGet = (vx_bool)0;
    (*queue)->blockedOnPut = (vx_bool)0;

    return;
}

vx_status tivxQueueCreate(
    tivx_queue *queue, uint32_t max_elements, uintptr_t *queue_memory,
    uint32_t flags)
{
    vx_status status = (vx_status)VX_FAILURE;
    uint32_t temp_status;
    tivx_queue_context context = NULL;

    if ((NULL != queue) && (NULL != queue_memory) && (0U != max_elements))
    {
        /*
         * init queue to 0's
         */
        init_queue(&queue);

        /*
         * init queue with user parameters
         */
        queue->max_ele = max_elements;
        queue->flags = flags;

        queue->queue = queue_memory;

        queue->context = ownPosixObjectAlloc((vx_enum)TIVX_POSIX_TYPE_QUEUE);

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

            temp_status = (uint32_t)status | (uint32_t)pthread_mutexattr_init(&mutex_attr);
            status = (vx_status)temp_status;
            temp_status = (uint32_t)status | (uint32_t)pthread_mutex_init(&context->lock, &mutex_attr);
            status = (vx_status)temp_status;

            (void)pthread_mutexattr_destroy(&mutex_attr);

            if(status==0)
            {
                if ((uint32_t)(queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET) != (uint32_t)0)
                {
                    pthread_condattr_t cond_attr;

                    /*
                     * user requested block on queue get
                     */

                    /*
                     * create cond for it
                     */
                    temp_status = (uint32_t)status | (uint32_t)pthread_condattr_init(&cond_attr);
                    status = (vx_status)temp_status;
                    temp_status = (uint32_t)status | (uint32_t)pthread_cond_init(&context->condGet, &cond_attr);
                    status = (vx_status)temp_status;

                    (void)pthread_condattr_destroy(&cond_attr);
                }
                if ((uint32_t)(queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT) != (uint32_t)0)
                {
                    pthread_condattr_t cond_attr;

                    /*
                     * user requested block on queue put
                     */

                    /*
                     * create cond for it
                     */
                    temp_status = (uint32_t)status | (uint32_t)pthread_condattr_init(&cond_attr);
                    status = (vx_status)temp_status;
                    temp_status = (uint32_t)status | (uint32_t)pthread_cond_init(&context->condPut, &cond_attr);
                    status = (vx_status)temp_status;

                    (void)pthread_condattr_destroy(&cond_attr);
                }
            }
            if ((vx_status)VX_SUCCESS == status)
            {
                queue->blockedOnGet = (vx_bool)vx_false_e;
                queue->blockedOnPut = (vx_bool)vx_false_e;
            }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1727- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TIVX_QUEUE_UM001 */
            else
            {
                (void)pthread_mutex_destroy(&context->lock);
                temp_status = ownPosixObjectFree(queue->context, (vx_enum)TIVX_POSIX_TYPE_QUEUE);
                if ((vx_status)VX_SUCCESS != temp_status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Queue free failed\n");
                }
                queue->context = NULL;
            }
#endif
        }
    }

    return (status);
}

vx_status tivxQueueDelete(tivx_queue *queue)
{
    vx_status status = (vx_status)VX_FAILURE;
    tivx_queue_context context;

    if ((NULL != queue) && (queue->context != NULL))
    {
        context = queue->context;

        VX_PRINT(VX_ZONE_INFO, "if this hangs, please ensure all application threads have been destroyed\n");
        if ((uint32_t)(queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET) != (uint32_t)0)
        {
            (void)pthread_cond_destroy(&(context)->condGet);
        }
        if ((uint32_t)(queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT) != (uint32_t)0)
        {
            (void)pthread_cond_destroy(&(context)->condPut);
        }
        (void)pthread_mutex_destroy(&context->lock);

        (void)ownPosixObjectFree((uint8_t*)context, (vx_enum)TIVX_POSIX_TYPE_QUEUE);
        queue->context = NULL;

        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

vx_status tivxQueuePut(tivx_queue *queue, uintptr_t data, uint32_t timeout)
{
    vx_status status = (vx_status)VX_FAILURE;
    uint32_t temp_status;
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

                    /* increment count of number element in queue */
                    queue->count++;

                    /* mark status as success */
                    status = (vx_status)VX_SUCCESS;

                    if ((uint32_t)(queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET) != (uint32_t)0)
                    {
                        /* blocking on queue get enabled */

                        /* post cond to unblock, blocked tasks */
                        (void)pthread_cond_signal(&context->condGet);
                    }

                    /* exit, with success */
                    do_break = (vx_bool)vx_true_e;
                }
                else
                {
                    /* queue is full */
                    if (timeout == TIVX_EVENT_TIMEOUT_NO_WAIT)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "queue is full\n");
                        status = (vx_status)VX_FAILURE;
                        do_break = (vx_bool)vx_true_e; /* non-blocking, so exit with error */
                    }
                    else if ((uint32_t)(queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT) != (uint32_t)0)
                    {
                        /* blocking on queue put enabled */
                        queue->blockedOnPut = (vx_bool)vx_true_e;
                        (void)pthread_cond_wait(&context->condPut, &context->lock);
                        queue->blockedOnPut = (vx_bool)vx_false_e;
                    }
                    else
                    {
                        /* blocking on queue put disabled */

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
            while (true);

            temp_status = (uint32_t)status | (uint32_t)pthread_mutex_unlock(&context->lock);
            status = (vx_status)temp_status;
        }
    }

    return (status);
}

vx_status tivxQueueGet(tivx_queue *queue, uintptr_t *data, uint32_t timeout)
{
    vx_status status = (vx_status)VX_FAILURE;/* init status to error */
    uint32_t temp_status;
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

                    /* decrement number of elements in queue */
                    queue->count--;

                    /* set status as success */
                    status = (vx_status)VX_SUCCESS;

                    if ((uint32_t)(queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT) != (uint32_t)0)
                    {
                        /* post cond to unblock, blocked tasks */
                        (void)pthread_cond_signal(&context->condPut);
                    }

                    /* exit with success */
                    do_break = (vx_bool)vx_true_e;
                }
                else
                {
                    /* no elements or not enough element in queue to extract */
                    if (timeout == TIVX_EVENT_TIMEOUT_NO_WAIT)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "no elements found in que\n");
                        status = (vx_status)VX_FAILURE;
                        do_break = (vx_bool)vx_true_e; /* non-blocking, exit with error */
                    }
                    else
                    if ((uint32_t)(queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET) != (uint32_t)0)
                    {
                        /* blocking on queue get enabled */

                        queue->blockedOnGet = (vx_bool)vx_true_e;
                        (void)pthread_cond_wait(&context->condGet, &context->lock);
                        queue->blockedOnGet = (vx_bool)vx_false_e;
                        /* received semaphore, check queue again */
                    }
                    else
                    {
                        /* blocking on queue get disabled */

                        /* exit with error */
                        VX_PRINT(VX_ZONE_ERROR, "blocking on queue get disabled\n");
                        status = (vx_status)VX_FAILURE;
                        do_break = (vx_bool)vx_true_e;
                    }
                }

                if ((vx_bool)vx_true_e == do_break)
                {
                    break;
                }
            }
            while (true);
          
            temp_status = (uint32_t)status | (uint32_t)pthread_mutex_unlock(&context->lock);
            status = (vx_status)temp_status;
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

            (void)pthread_mutex_unlock(&context->lock);
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

        (void)pthread_mutex_lock(&context->lock);
        
        if (queue->count > 0U)
        {
            /* 'peek' the element but dont extract it */
            *data = queue->queue[queue->cur_rd];
            /*Set status as success*/
            status = (vx_status)VX_SUCCESS;
        }

        (void)pthread_mutex_unlock(&context->lock);
        
    }
    return (status);
}
