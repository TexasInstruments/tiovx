/*
*
* Copyright (c) 2018 Texas Instruments Incorporated
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

vx_status tivxEventQueueCreate(tivx_event_queue_t *event_q)
{
    vx_status status = VX_SUCCESS;

    event_q->enable = vx_true_e;

    status = tivxQueueCreate(&event_q->free_queue,
                TIVX_EVENT_QUEUE_MAX_SIZE, event_q->free_queue_memory, 0 /* non-blocking */
        );
    if(status==VX_SUCCESS)
    {
        status = tivxQueueCreate(&event_q->ready_queue,
                TIVX_EVENT_QUEUE_MAX_SIZE, event_q->ready_queue_memory,
                TIVX_QUEUE_FLAG_BLOCK_ON_GET);
        if(status!=VX_SUCCESS)
        {
            tivxQueueDelete(&event_q->free_queue);
        }
    }
    if(status!=VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxEventQueueCreate: Unable to create queues\n");
    }
    if(status==VX_SUCCESS)
    {
        uint32_t i;

        /* enqueue all event element index's to free queue */
        for(i=0; i<TIVX_EVENT_QUEUE_MAX_SIZE; i++)
        {
            /* this call wont fail since number of elements being inserted are equal to
             * queue depth, hence not doing any error checks
             */
            tivxQueuePut(&event_q->free_queue, i, TIVX_EVENT_TIMEOUT_NO_WAIT);
        }
    }


    return status;
}

void tivxEventQueueDelete(tivx_event_queue_t *event_q)
{
    event_q->enable = vx_false_e;

    tivxQueueDelete(&event_q->free_queue);
    tivxQueueDelete(&event_q->ready_queue);
}

static void tivxEventQueueEnableEvents(tivx_event_queue_t *event_q, vx_bool enable)
{
    event_q->enable = enable;
}

vx_status tivxEventQueueAddEvent(tivx_event_queue_t *event_q,
        vx_enum event_id, uint64_t timestamp, uintptr_t param1, uintptr_t param2)
{
    vx_status status = VX_FAILURE;

    if(event_q != NULL && event_q->enable==vx_true_e)
    {
        uint32_t index;

        status = tivxQueueGet(&event_q->free_queue, &index, TIVX_EVENT_TIMEOUT_NO_WAIT);
        if(status==VX_SUCCESS && index < TIVX_EVENT_QUEUE_MAX_SIZE)
        {
            tivx_event_queue_elem_t *elem;

            elem = &event_q->event_list[index];

            elem->event_id = event_id;
            elem->timestamp = timestamp;
            elem->param1 = param1;
            elem->param2 = param2;

            status = tivxQueuePut(&event_q->ready_queue, index, TIVX_EVENT_TIMEOUT_NO_WAIT);
        }
        if(status!=VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxEventQueueAddEvent: Unable to add event, dropping it\n");
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxEnableEvents(vx_context context)
{
    vx_status status;

    if (ownIsValidContext(context) == vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"context is invalid\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        tivxEventQueueEnableEvents(&context->event_queue, vx_true_e);
        status = VX_SUCCESS;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxDisableEvents(vx_context context)
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidContext(context) == vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"context is invalid\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        tivxEventQueueEnableEvents(&context->event_queue, vx_false_e);
        status = VX_SUCCESS;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSendUserEvent(vx_context context, vx_uint32 id, void *parameter)
{
    vx_status status;

    if (ownIsValidContext(context) == vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"context is invalid\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        uint64_t timestamp = tivxPlatformGetTimeInUsecs()*1000;

        status = tivxEventQueueAddEvent(
                &context->event_queue,
                VX_EVENT_USER,
                timestamp,
                (uintptr_t)id, (uintptr_t)parameter);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxWaitEvent(
                    vx_context context, vx_event_t *event,
                    vx_bool do_not_block)
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidContext(context) == vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"context is invalid\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        uint32_t index, timeout;
        tivx_event_queue_t *event_q = &context->event_queue;

        if(do_not_block)
        {
            timeout = 0;
        }
        else
        {
            timeout = TIVX_EVENT_TIMEOUT_WAIT_FOREVER;
        }

        status = tivxQueueGet(&event_q->ready_queue, &index, timeout);
        if(status==VX_SUCCESS && index < TIVX_EVENT_QUEUE_MAX_SIZE)
        {
            tivx_event_queue_elem_t *elem;

            elem = &event_q->event_list[index];

            /* copy internal event info to user event structure */
            if(event!=NULL)
            {
                event->type = elem->event_id;
                event->timestamp = elem->timestamp;

                if(elem->event_id==VX_EVENT_GRAPH_PARAMETER_CONSUMED)
                {
                    event->event_info.graph_parameter_consumed.graph = (vx_graph)elem->param1;
                    event->event_info.graph_parameter_consumed.graph_parameter_index = (uint32_t)elem->param2;
                }
                else
                if(elem->event_id==VX_EVENT_GRAPH_COMPLETED)
                {
                    event->event_info.graph_completed.graph = (vx_graph)elem->param1;
                }
                else
                if(elem->event_id==VX_EVENT_NODE_COMPLETED)
                {
                    event->event_info.node_completed.graph = (vx_graph)elem->param1;
                    event->event_info.node_completed.node = (vx_node)elem->param2;
                }
                else
                if(elem->event_id==VX_EVENT_USER)
                {
                    event->event_info.user_event.user_event_id = (uint32_t)elem->param1;
                    event->event_info.user_event.user_event_parameter = (void*)elem->param2;
                }
            }

            /* release index into free queue,
             * this wont fail since the index was dequeued from free queue to begin with */
            tivxQueuePut(&event_q->free_queue, index, TIVX_EVENT_TIMEOUT_NO_WAIT);
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxRegisterEvent(vx_reference ref,
                enum vx_event_type_e type, vx_uint32 param)
{
    vx_status status = VX_ERROR_NOT_SUPPORTED;

    if (ownIsValidSpecificReference(ref, VX_TYPE_NODE) == vx_true_e)
    {
        if(type==VX_EVENT_NODE_COMPLETED)
        {
            status = ownNodeRegisterEvent((vx_node)ref, type);
        }
    }
    else
    if (ownIsValidSpecificReference(ref, VX_TYPE_GRAPH) == vx_true_e)
    {
        if(type==VX_EVENT_GRAPH_COMPLETED)
        {
            status = ownGraphRegisterCompletionEvent((vx_graph)ref);
        }
        else
        if(type==VX_EVENT_GRAPH_PARAMETER_CONSUMED)
        {
            status = ownGraphRegisterParameterConsumedEvent((vx_graph)ref, param);
        }
    }

    return status;
}
