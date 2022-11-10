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

vx_status ownEventQueueCreate(tivx_event_queue_t *event_q)
{
    vx_status status = (vx_status)VX_SUCCESS;

    event_q->enable = (vx_bool)vx_true_e;

    status = tivxQueueCreate(&event_q->free_queue,
                TIVX_EVENT_QUEUE_MAX_SIZE, event_q->free_queue_memory, 0 /* non-blocking */
        );
    if(status==(vx_status)VX_SUCCESS)
    {
        status = tivxQueueCreate(&event_q->ready_queue,
                TIVX_EVENT_QUEUE_MAX_SIZE, event_q->ready_queue_memory,
                TIVX_QUEUE_FLAG_BLOCK_ON_GET);
        if(status!=(vx_status)VX_SUCCESS)
        {
            tivxQueueDelete(&event_q->free_queue);
        }
    }
    if(status!=(vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to create queues\n");
    }
    if(status==(vx_status)VX_SUCCESS)
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

void ownEventQueueDelete(tivx_event_queue_t *event_q)
{
    event_q->enable = (vx_bool)vx_false_e;

    tivxQueueDelete(&event_q->free_queue);
    tivxQueueDelete(&event_q->ready_queue);
}

void ownEventQueueEnableEvents(tivx_event_queue_t *event_q, vx_bool enable)
{
    event_q->enable = enable;
}

vx_status ownEventQueueAddEvent(tivx_event_queue_t *event_q,
        vx_enum event_id, uint64_t timestamp, uint32_t app_value, uintptr_t param1, uintptr_t param2, uintptr_t param3)
{
    vx_status status = (vx_status)VX_FAILURE;

    if((event_q != NULL) && (event_q->enable == (vx_bool)vx_true_e))
    {
        uintptr_t index;

        status = tivxQueueGet(&event_q->free_queue, &index, TIVX_EVENT_TIMEOUT_NO_WAIT);
        if((status == (vx_status)VX_SUCCESS) && (index < TIVX_EVENT_QUEUE_MAX_SIZE))
        {
            tivx_event_queue_elem_t *elem;

            elem = &event_q->event_list[index];

            elem->event_id = event_id;
            elem->timestamp = timestamp;
            elem->app_value = app_value;
            elem->param1 = param1;
            elem->param2 = param2;
            elem->param3 = param3;

            status = tivxQueuePut(&event_q->ready_queue, index, TIVX_EVENT_TIMEOUT_NO_WAIT);

            if ((vx_status)VX_SUCCESS == status)
            {
                ownLogSetResourceUsedValue("TIVX_EVENT_QUEUE_MAX_SIZE", (vx_uint16)index+1U);
            }
        }
        if(status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to add event, dropping it\n");
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxEnableEvents(vx_context context)
{
    vx_status status;

    if (ownIsValidContext(context) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"context is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        ownEventQueueEnableEvents(&context->event_queue, (vx_bool)vx_true_e);
        status = (vx_status)VX_SUCCESS;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxDisableEvents(vx_context context)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidContext(context) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"context is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        ownEventQueueEnableEvents(&context->event_queue, (vx_bool)vx_false_e);
        status = (vx_status)VX_SUCCESS;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSendUserEvent(vx_context context, vx_uint32 app_value, const void *parameter)
{
    vx_status status;

    if (ownIsValidContext(context) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"context is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        uint64_t timestamp = tivxPlatformGetTimeInUsecs()*1000U;

        status = ownEventQueueAddEvent(
                &context->event_queue,
                (vx_enum)VX_EVENT_USER,
                timestamp, app_value,
                (uintptr_t)app_value, (uintptr_t)parameter, (uintptr_t)0);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxWaitEvent(
                    vx_context context, vx_event_t *event,
                    vx_bool do_not_block)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidContext(context) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"context is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        /* Call general wait function */
        status = vxWaitEventQueue(&context->event_queue, event, do_not_block);
    }

    return status;
}

vx_status vxWaitEventQueue(
                    tivx_event_queue_t *event_q, vx_event_t *event,
                    vx_bool do_not_block)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uintptr_t index;
    uint32_t timeout;

    if((vx_bool)vx_true_e == do_not_block)
    {
        timeout = 0;
    }
    else
    {
        timeout = TIVX_EVENT_TIMEOUT_WAIT_FOREVER;
    }

    status = tivxQueueGet(&event_q->ready_queue, &index, timeout);

    if((status == (vx_status)VX_SUCCESS) && (index < TIVX_EVENT_QUEUE_MAX_SIZE))
    {
        tivx_event_queue_elem_t *elem;
        elem = &event_q->event_list[index];

        /* copy internal event info to user event structure */
        if(event!=NULL)
        {
            event->type = elem->event_id;
            event->timestamp = elem->timestamp;
            event->app_value = elem->app_value;

            if(elem->event_id==(vx_enum)VX_EVENT_GRAPH_PARAMETER_CONSUMED)
            {
                event->event_info.graph_parameter_consumed.graph = (vx_graph)elem->param1;
                event->event_info.graph_parameter_consumed.graph_parameter_index = (uint32_t)elem->param2;
            }
            else
            if(elem->event_id==(vx_enum)VX_EVENT_GRAPH_COMPLETED)
            {
                event->event_info.graph_completed.graph = (vx_graph)elem->param1;
            }
            else
            if(elem->event_id==(vx_enum)VX_EVENT_NODE_COMPLETED)
            {
                event->event_info.node_completed.graph = (vx_graph)elem->param1;
                event->event_info.node_completed.node = (vx_node)elem->param2;
            }
            else
            if(elem->event_id==(vx_enum)VX_EVENT_NODE_ERROR)
            {
                event->event_info.node_error.graph = (vx_graph)elem->param1;
                event->event_info.node_error.node = (vx_node)elem->param2;
                event->event_info.node_error.status = (vx_status)elem->param3;
            }
            else
            if(elem->event_id==(vx_enum)VX_EVENT_USER)
            {
                event->app_value = (uint32_t)elem->param1;
                event->event_info.user_event.user_event_parameter = (void*)elem->param2;
            }
            else
            {
                /* do nothing */
            }
        }

        /* release index into free queue,
         * this wont fail since the index was dequeued from free queue to begin with */
        tivxQueuePut(&event_q->free_queue, index, TIVX_EVENT_TIMEOUT_NO_WAIT);
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxRegisterEvent(vx_reference ref,
                enum vx_event_type_e type, vx_uint32 param, vx_uint32 app_value)
{
    vx_status status = (vx_status)VX_ERROR_NOT_SUPPORTED;

    status = ownRegisterEvent(ref, TIVX_EVENT_CONTEXT_QUEUE, type, param, app_value);

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL ownRegisterEvent(vx_reference ref,
                enum tivx_queue_type_e queue_type, enum vx_event_type_e type,
                vx_uint32 param, vx_uint32 app_value)
{
    vx_status status = (vx_status)VX_ERROR_NOT_SUPPORTED;

    if (ownIsValidSpecificReference(ref, (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        if( ((vx_enum)type==(vx_enum)VX_EVENT_NODE_COMPLETED) ||
            ((vx_enum)type==(vx_enum)VX_EVENT_NODE_ERROR) )
        {
            vx_node node = (vx_node)ref;

            if ((vx_enum)TIVX_EVENT_GRAPH_QUEUE == (vx_enum)queue_type)
            {
                node->is_graph_event = (vx_bool)vx_true_e;
                status = (vx_status)VX_SUCCESS;
            }
            else if ((vx_enum)TIVX_EVENT_CONTEXT_QUEUE == (vx_enum)queue_type)
            {
                node->is_context_event = (vx_bool)vx_true_e;
                status = (vx_status)VX_SUCCESS;
            }
            else
            {
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                VX_PRINT(VX_ZONE_ERROR, "Invalid queue type given\n");
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                status = ownNodeRegisterEvent((vx_node)ref, (vx_enum)type, app_value);
            }
        }
    }
    else
    if (ownIsValidSpecificReference(ref, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        if((vx_enum)type==(vx_enum)VX_EVENT_GRAPH_COMPLETED)
        {
            status = ownGraphRegisterCompletionEvent((vx_graph)ref, app_value);
        }
        else
        if((vx_enum)type==(vx_enum)VX_EVENT_GRAPH_PARAMETER_CONSUMED)
        {
            status = ownGraphRegisterParameterConsumedEvent((vx_graph)ref, param, app_value);
        }
        else
        {
            /* do nothing */
        }
    }
    else
    {
        /* do nothing */
    }

    return status;
}
