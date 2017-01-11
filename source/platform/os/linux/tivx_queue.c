/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */
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

typedef struct _tivx_queue_context *tivx_queue_context;

typedef struct _tivx_queue_context {

  pthread_mutex_t lock;
  pthread_cond_t  condGet;
  pthread_cond_t  condPut;

} tivx_queue_context_t;


vx_status tivxQueueCreate(
    tivx_queue *queue, uint32_t max_elements, uint32_t *queue_memory,
    uint32_t flags)
{
    vx_status status = VX_FAILURE;
    tivx_queue_context context = NULL;

    if ((NULL != queue) && (NULL != queue_memory) && (0 != max_elements))
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
            status = VX_ERROR_NO_MEMORY;
        }
        else
        {
            pthread_mutexattr_t mutex_attr;

            status = VX_SUCCESS;

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
            if (VX_SUCCESS == status)
            {
                queue->blockedOnGet = vx_false_e;
                queue->blockedOnPut = vx_false_e;
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
    vx_status status = VX_FAILURE;
    tivx_queue_context context;

    if (NULL != queue && queue->context != NULL)
    {
        context = queue->context;

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

        status = VX_SUCCESS;
    }

    return (status);
}

vx_status tivxQueuePut(tivx_queue *queue, uint32_t data, uint32_t timeout)
{
    vx_status status = VX_FAILURE;
    volatile vx_bool do_break = vx_false_e;
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
                    queue->cur_wr = (queue->cur_wr + 1) % queue->max_ele;

                    /* increment count of number element in que */
                    queue->count++;

                    /* mark status as success */
                    status = VX_SUCCESS;

                    if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET)
                    {
                        /* blocking on que get enabled */

                        /* post cond to unblock, blocked tasks */
                        pthread_cond_signal(&context->condGet);
                    }

                    /* exit, with success */
                    do_break = vx_true_e;
                }
                else
                {
                    /* que is full */
                    if (timeout == TIVX_EVENT_TIMEOUT_NO_WAIT)
                    {
                        status = VX_FAILURE;
                        do_break = vx_true_e; /* non-blocking, so exit with error */
                    }
                    else if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET)
                    {
                        /* blocking on que put enabled */
                        queue->blockedOnPut = vx_true_e;
                        pthread_cond_wait(&context->condPut, &context->lock);
                        queue->blockedOnPut = vx_false_e;
                    }
                    else
                    {
                        /* blocking on que put disabled */

                        /* exit with error */
                        status = VX_FAILURE;
                        do_break = vx_true_e;
                    }
                }

                if (vx_true_e == do_break)
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

vx_status tivxQueueGet(tivx_queue *queue, uint32_t *data, uint32_t timeout)
{
    vx_status status = VX_FAILURE;/* init status to error */
    volatile vx_bool do_break = vx_false_e;
    tivx_queue_context context = NULL;

    if(queue && queue->context)
    {
        context = queue->context;

        status = pthread_mutex_lock(&context->lock);
        if(status==0)
        {
            do
            {
                if (queue->count > 0)
                {
                    /* extract the element */
                    *data = queue->queue[queue->cur_rd];

                    /* increment get pointer */
                    queue->cur_rd = (queue->cur_rd + 1) % queue->max_ele;

                    /* decrmeent number of elements in que */
                    queue->count--;

                    /* set status as success */
                    status = VX_SUCCESS;

                    if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT)
                    {
                        /* post cond to unblock, blocked tasks */
                        pthread_cond_signal(&context->condPut);
                    }

                    /* exit with success */
                    do_break = vx_true_e;
                }
                else
                {
                    /* no elements or not enough element in que to extract */
                    if (timeout == TIVX_EVENT_TIMEOUT_NO_WAIT)
                    {
                        status = VX_FAILURE;
                        do_break = vx_true_e; /* non-blocking, exit with error */
                    }
                    else
                    if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET)
                    {
                        /* blocking on que get enabled */

                        queue->blockedOnGet = vx_true_e;
                        pthread_cond_wait(&context->condGet, &context->lock);
                        queue->blockedOnGet = vx_false_e;
                        /* received semaphore, check que again */
                    }
                    else
                    {
                        /* blocking on que get disabled */

                        /* exit with error */
                        status = VX_FAILURE;
                        do_break = vx_true_e;
                    }
                }

                if (vx_true_e == do_break)
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

