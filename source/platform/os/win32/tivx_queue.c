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




vx_status tivxQueueCreate(
    tivx_queue *queue, uint32_t max_elements, uint32_t *queue_memory,
    uint32_t flags)
{
    vx_status status = VX_FAILURE;

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

        if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET)
        {
            /*
             * user requested block on que get
             */

            /*
             * create semaphore for it
             */
            status = tivxEventCreate(&queue->block_rd);
        }

        if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT)
        {
            /*
             * user requested block on que put
             */

            /*
             * create semaphore for it
             */
            status = tivxEventCreate(&queue->block_wr);
        }

        if (VX_SUCCESS == status)
        {
            status = tivxMutexCreate(&queue->lock);
        }

        if (VX_SUCCESS == status)
        {
            queue->blockedOnGet = vx_false_e;
            queue->blockedOnPut = vx_false_e;
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
    vx_status status = VX_FAILURE;

    if (NULL != queue)
    {
        if ((queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET) &&
            (NULL != queue->block_rd))
        {
            tivxEventDelete(&queue->block_rd);
        }
        if ((queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT) &&
            (NULL != queue->block_wr))
        {
            tivxEventDelete(&queue->block_wr);
        }
        if ((NULL != queue->lock))
        {
            tivxMutexDelete(&queue->lock);
        }

        status = VX_SUCCESS;
    }

    return (status);
}

vx_status tivxQueuePut(tivx_queue *queue, uint32_t data, uint32_t timeout)
{
    vx_status status = VX_FAILURE;
    volatile vx_bool do_break = vx_false_e;

    do
    {
        /* lock queue */
        tivxMutexLock(queue->lock);

        if (queue->count < queue->max_ele)
        {
            /* insert element */
            queue->queue[queue->cur_wr] = data;

            /* increment put pointer */
            queue->cur_wr = (queue->cur_wr + 1) % queue->max_ele;

            /* increment count of number element in que */
            queue->count++;

            /* restore interrupts */
            tivxMutexUnlock(queue->lock);

            /* mark status as success */
            status = VX_SUCCESS;

            if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET)
            {
                /* blocking on que get enabled */

                /* post semaphore to unblock, blocked tasks */
                tivxEventPost(queue->block_rd);
            }

            /* exit, with success */
            do_break = vx_true_e;

        }
        else
        {
            /* que is full */

            /* restore interrupts */
            tivxMutexUnlock(queue->lock);

            if (timeout == TIVX_EVENT_TIMEOUT_NO_WAIT)
            {
                do_break = vx_true_e; /* non-blocking, so exit with error */
            }
            else if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET)
            {
                vx_status wait_status;

                /* blocking on que put enabled */

                /* take semaphore and block until timeout occurs or
                 * semaphore is posted */
                queue->blockedOnPut = vx_true_e;
                wait_status = tivxEventWait(queue->block_wr, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);
                queue->blockedOnPut = vx_false_e;
                if (VX_SUCCESS != wait_status)
                {
                    do_break = vx_true_e;
                    /* error, exit with error */
                }
                else
                {
                    do_break = vx_false_e;
                }
                /* received semaphore, recheck for available space in the que */
            }
            else
            {
                /* blocking on que put disabled */

                /* exit with error */
                do_break = vx_true_e;
            }
        }

        if (vx_true_e == do_break)
        {
            break;
        }
    }
    while (1);

    return (status);
}

vx_status tivxQueueGet(tivx_queue *queue, uint32_t *data, uint32_t timeout)
{
    vx_status status = VX_FAILURE;/* init status to error */
    volatile vx_bool do_break = vx_false_e;

    do
    {
        /* disable interrupts */
        tivxMutexLock(queue->lock);

        if (queue->count > 0)
        {
            /* extract the element */
            *data = queue->queue[queue->cur_rd];

            /* increment get pointer */
            queue->cur_rd = (queue->cur_rd + 1) % queue->max_ele;

            /* decrmeent number of elements in que */
            queue->count--;

            /* restore interrupts */
            tivxMutexUnlock(queue->lock);

            /* set status as success */
            status = VX_SUCCESS;

            if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_PUT)
            {
                /* blocking on que put enabled,
                 * post semaphore to unblock, blocked tasks
                 */
                tivxEventPost(queue->block_wr);
            }

            /* exit with success */
            do_break = vx_true_e;
        }
        else
        {
            /* no elements or not enough element in que to extract */

            /* restore interrupts */
            tivxMutexUnlock(queue->lock);

            if (timeout == TIVX_EVENT_TIMEOUT_NO_WAIT)
            {
                do_break = vx_true_e; /* non-blocking, exit with error */
            }
            else
            if (queue->flags & TIVX_QUEUE_FLAG_BLOCK_ON_GET)
            {
                vx_status wait_status;

                /* blocking on que get enabled */

                /* take semaphore and block until timeout occurs or
                 * semaphore is posted
                 */

                queue->blockedOnGet = vx_true_e;
                wait_status = tivxEventWait(queue->block_rd, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);
                queue->blockedOnGet = vx_false_e;
                if (VX_SUCCESS != wait_status)
                {
                    do_break = vx_true_e; /* exit with error */
                }
                else
                {
                    do_break = vx_false_e;
                }
                /* received semaphore, check que again */
            }
            else
            {
                /* blocking on que get disabled */

                /* exit with error */
                do_break = vx_true_e;
            }
        }

        if (vx_true_e == do_break)
        {
            break;
        }
    }
    while (1);

    return (status);
}

