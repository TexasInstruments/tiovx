/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <vx_internal.h>

#include <ti/osal/HwiP.h>

vx_status tivxQueueCreate(
    tivx_queue *queue, uint32_t max_elements, uintptr_t *queue_memory,
    uint32_t flags)
{
    vx_status status = (vx_status)VX_FAILURE;

    if ((NULL != queue) && (NULL != queue_memory) && (0U != max_elements))
    {
        status = (vx_status)VX_SUCCESS;

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

        if ((queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET) != 0U)
        {
            /*
             * user requested block on que get
             */

            /*
             * create semaphore for it
             */
            status = tivxEventCreate(&queue->block_rd);
        }

        if ((queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT) != 0U)
        {
            /*
             * user requested block on que put
             */

            /*
             * create semaphore for it
             */
            status = tivxEventCreate(&queue->block_wr);
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            queue->blockedOnGet = (vx_bool)vx_false_e;
            queue->blockedOnPut = (vx_bool)vx_false_e;
        }
        else
        {
            tivxQueueDelete(queue);
        }
    }

    return (status);
}

vx_status tivxQueueDelete(tivx_queue *queue)
{
    vx_status status = (vx_status)VX_FAILURE;

    if (NULL != queue)
    {
        if (((queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET) ==
                TIVX_QUEUE_FLAG_BLOCK_ON_GET) &&
            (NULL != queue->block_rd))
        {
            tivxEventDelete(&queue->block_rd);
        }
        if (((queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT) ==
                TIVX_QUEUE_FLAG_BLOCK_ON_PUT) &&
            (NULL != queue->block_wr))
        {
            tivxEventDelete(&queue->block_wr);
        }

        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

vx_status tivxQueuePut(tivx_queue *queue, uintptr_t data, uint32_t timeout)
{
    vx_status status = (vx_status)VX_FAILURE;
    uint32_t cookie;
    volatile vx_bool do_break = (vx_bool)vx_false_e;

    do
    {
        /* disable interrupts */
        cookie = (uint32_t)HwiP_disable();

        if (queue->count < queue->max_ele)
        {
            /* insert element */
            queue->queue[queue->cur_wr] = data;

            /* increment put pointer */
            queue->cur_wr = (queue->cur_wr + 1U) % queue->max_ele;

            /* increment count of number element in que */
            queue->count++;

            /* restore interrupts */
            HwiP_restore(cookie);

            /* mark status as success */
            status = (vx_status)VX_SUCCESS;

            if ((queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET) != 0U)
            {
                /* blocking on que get enabled */

                /* post semaphore to unblock, blocked tasks */
                tivxEventPost(queue->block_rd);
            }

            /* exit, with success */
            do_break = (vx_bool)vx_true_e;

        }
        else
        {
            /* que is full */

            /* restore interrupts */
            HwiP_restore(cookie);

            if (timeout == TIVX_EVENT_TIMEOUT_NO_WAIT)
            {
                do_break = (vx_bool)vx_true_e; /* non-blocking, so exit with error */
            }
            else if ((queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT) != 0U)
            {
                vx_status wait_status;

                /* blocking on que put enabled */

                /* take semaphore and block until timeout occurs or
                 * semaphore is posted */
                queue->blockedOnPut = (vx_bool)vx_true_e;
                wait_status = tivxEventWait(queue->block_wr, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);
                queue->blockedOnPut = (vx_bool)vx_false_e;
                if ((vx_status)VX_SUCCESS != wait_status)
                {
                    do_break = (vx_bool)vx_true_e;
                    /* error, exit with error */
                }
                else
                {
                    do_break = (vx_bool)vx_false_e;
                }
                /* received semaphore, recheck for available space in the que */
            }
            else
            {
                /* blocking on que put disabled */

                /* exit with error */
                do_break = (vx_bool)vx_true_e;
            }
        }

        if ((vx_bool)vx_true_e == do_break)
        {
            break;
        }
    }
    while (1 == 1);

    return (status);
}

vx_status tivxQueueGet(tivx_queue *queue, uintptr_t *data, uint32_t timeout)
{
    vx_status status = (vx_status)VX_FAILURE;/* init status to error */
    uint32_t cookie;
    volatile vx_bool do_break = (vx_bool)vx_false_e;

    do
    {
        /* disable interrupts */
        cookie = (uint32_t)HwiP_disable();

        if (queue->count > 0U)
        {
            /* extract the element */
            *data = queue->queue[queue->cur_rd];

            /* increment get pointer */
            queue->cur_rd = (queue->cur_rd + 1U) % queue->max_ele;

            /* decrmeent number of elements in que */
            queue->count--;

            /* restore interrupts */
            HwiP_restore(cookie);

            /* set status as success */
            status = (vx_status)VX_SUCCESS;

            if ((queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT) != 0U)
            {
                /* blocking on que put enabled,
                 * post semaphore to unblock, blocked tasks
                 */
                tivxEventPost(queue->block_wr);
            }

            /* exit with success */
            do_break = (vx_bool)vx_true_e;
        }
        else
        {
            /* no elements or not enough element in que to extract */

            /* restore interrupts */
            HwiP_restore(cookie);

            if (timeout == TIVX_EVENT_TIMEOUT_NO_WAIT)
            {
                do_break = (vx_bool)vx_true_e; /* non-blocking, exit with error */
            }
            else
            if ((queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET) != 0U)
            {
                vx_status wait_status;

                /* blocking on que get enabled */

                /* take semaphore and block until timeout occurs or
                 * semaphore is posted
                 */

                queue->blockedOnGet = (vx_bool)vx_true_e;
                wait_status = tivxEventWait(queue->block_rd, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);
                queue->blockedOnGet = (vx_bool)vx_false_e;
                if ((vx_status)VX_SUCCESS != wait_status)
                {
                    do_break = (vx_bool)vx_true_e; /* exit with error */
                }
                else
                {
                    do_break = (vx_bool)vx_false_e;
                }
                /* received semaphore, check que again */
            }
            else
            {
                /* blocking on que get disabled */

                /* exit with error */
                do_break = (vx_bool)vx_true_e;
            }
        }

        if ((vx_bool)vx_true_e == do_break)
        {
            break;
        }
    }
    while (1 == 1);

    return (status);
}

vx_bool tivxQueueIsEmpty(const tivx_queue *queue)
{
    vx_bool is_empty = (vx_bool)vx_false_e;
    uint32_t cookie;

    /* disable interrupts */
    cookie = (uint32_t)HwiP_disable();

    if (queue->count == 0U)
    {
        is_empty = (vx_bool)vx_true_e;
    }

    /* restore interrupts */
    HwiP_restore(cookie);

    return is_empty;
}

vx_status tivxQueuePeek(const tivx_queue *queue, uintptr_t *data)
{
    vx_status status = (vx_status)VX_FAILURE;/* init status to error */

    uint32_t cookie;

    /* disable interrupts */
    cookie = (uint32_t)HwiP_disable();

    if (queue->count > 0U)
    {
        /* 'peek' the element but dont extract it */
        *data = queue->queue[queue->cur_rd];

        /* set status as success */
        status = (vx_status)VX_SUCCESS;
    }

    /* restore interrupts */
    HwiP_restore(cookie);

    return status;
}
