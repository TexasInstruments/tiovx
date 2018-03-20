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


vx_status tivxDataRefQueueEnqueueReadyRef(tivx_data_ref_queue data_ref_q, vx_reference ref)
{
    vx_status status = VX_SUCCESS;

    if(data_ref_q == NULL || ref == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "'data_ref_q' is invalid or 'ref' is invalid \n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        uint16_t queue_obj_desc_id, ref_obj_desc_id;
        tivx_obj_desc_queue_blocked_nodes_t blocked_nodes;

        /* get queue object descriptor */
        queue_obj_desc_id = data_ref_q->ready_q_obj_desc_id;
        /* get reference object descriptor */
        ref_obj_desc_id = ref->obj_desc->obj_desc_id;

        VX_PRINT(VX_ZONE_INFO,"Q (queue=%d, ref=%d)\n",
                            queue_obj_desc_id, ref_obj_desc_id
                       );

        tivxPlatformSystemLock(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);

        status = tivxObjDescQueueEnqueue(queue_obj_desc_id, ref_obj_desc_id);
        if(status==VX_SUCCESS)
        {
            blocked_nodes.num_nodes = 0;

            /* if any node is blocked on ref enqueued to this queue, then get the list of blocked nodes */
            tivxObjDescQueueExtractBlockedNodes(queue_obj_desc_id,
                &blocked_nodes);
        }

        tivxPlatformSystemUnlock(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);

        if(status==VX_SUCCESS)
        {
            uint32_t node_id;

            /* re-trigger blocked nodes */
            for(node_id=0; node_id<blocked_nodes.num_nodes; node_id++)
            {
                VX_PRINT(VX_ZONE_INFO,"Re-triggering (node=%d)\n",
                                 blocked_nodes.node_id[node_id]
                           );

                tivxTargetTriggerNode(blocked_nodes.node_id[node_id]);
            }
        }
    }

    return status;
}

vx_status tivxDataRefQueueDequeueDoneRef(tivx_data_ref_queue data_ref_q, vx_reference *ref)
{
    vx_status status = VX_SUCCESS;

    if(data_ref_q == NULL || ref == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "'data_ref_q' is invalid or 'ref' is invalid \n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        uint16_t queue_obj_desc_id, ref_obj_desc_id;

        /* get queue object descriptor */
        queue_obj_desc_id = data_ref_q->done_q_obj_desc_id;

        tivxPlatformSystemLock(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);

        status = tivxObjDescQueueDequeue(queue_obj_desc_id, &ref_obj_desc_id);

        tivxPlatformSystemUnlock(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);

        if(status==VX_SUCCESS && ref_obj_desc_id != TIVX_OBJ_DESC_INVALID)
        {
            VX_PRINT(VX_ZONE_INFO,"DQ (queue=%d, ref=%d)\n",
                             queue_obj_desc_id,
                             ref_obj_desc_id
                       );

            status = VX_SUCCESS;

            *ref = ownReferenceGetHandleFromObjDescId(ref_obj_desc_id);
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO,"DQ (queue=%d) .. NO BUFFER\n",
                             queue_obj_desc_id
                       );
        }
    }

    return status;
}

vx_status tivxDataRefQueueWaitDoneRef(tivx_data_ref_queue data_ref_q, vx_uint32 timeout)
{
    vx_status status = VX_SUCCESS;

    if(data_ref_q == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "'data_ref_q' is invalid or 'queue_id' is invalid\n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        status = tivxEventWait(data_ref_q->wait_done_ref_available_event, timeout);
    }

    return status;
}

vx_status tivxDataRefQueueGetDoneQueueCount(tivx_data_ref_queue data_ref_q, vx_uint32 *count)
{
    vx_status status = VX_SUCCESS;

    if(data_ref_q == NULL || count == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "'data_ref_q' is invalid or 'count' is invalid or 'queue_id' is invalid\n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        uint16_t queue_obj_desc_id;

        /* get queue object descriptor */
        queue_obj_desc_id = data_ref_q->done_q_obj_desc_id;

        tivxPlatformSystemLock(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);

        status = tivxObjDescQueueGetCount(queue_obj_desc_id, count);

        tivxPlatformSystemUnlock(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);
    }
    return status;
}

vx_status tivxDataRefQueueGetReadyQueueCount(tivx_data_ref_queue data_ref_q, vx_uint32 *count)
{
    vx_status status = VX_SUCCESS;

    if(data_ref_q == NULL || count == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "'data_ref_q' is invalid or 'count' is invalid or 'queue_id' is invalid\n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        uint16_t queue_obj_desc_id;

        /* get queue object descriptor */
        queue_obj_desc_id = data_ref_q->ready_q_obj_desc_id;

        tivxPlatformSystemLock(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);

        status = tivxObjDescQueueGetCount(queue_obj_desc_id, count);

        tivxPlatformSystemUnlock(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);
    }
    return status;
}

uint16_t tivxDataRefQueueGetObjDescId(tivx_data_ref_queue ref, uint32_t pipeline_id)
{
    uint16_t obj_desc_id = TIVX_OBJ_DESC_INVALID;

    if(ref!=NULL && pipeline_id < ref->pipeline_depth)
    {
        obj_desc_id = ref->obj_desc[pipeline_id]->base.obj_desc_id;
    }
    return obj_desc_id;
}

vx_status tivxDataRefQueueSendRefConsumedEvent(tivx_data_ref_queue ref, uint64_t timestamp)
{
    vx_status status = VX_SUCCESS;

    if(ref!=NULL)
    {
        if(ref->wait_done_ref_available_event)
        {
            tivxEventPost(ref->wait_done_ref_available_event);
        }
        if(ref->is_enable_send_ref_consumed_event)
        {
            tivxEventQueueAddEvent(&ref->base.context->event_queue,
                        VX_EVENT_GRAPH_PARAMETER_CONSUMED, timestamp,
                        (uintptr_t)ref->graph, (uintptr_t)ref->graph_parameter_index);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
    }
    return status;
}

static vx_status tivxDataRefQueueDestruct(vx_reference ref)
{
    uint32_t i;

    if(ref->type == TIVX_TYPE_DATA_REF_Q)
    {
        tivx_data_ref_queue data_ref_q = (tivx_data_ref_queue)ref;

        if(data_ref_q->wait_done_ref_available_event!=NULL)
        {
            tivxEventDelete(&data_ref_q->wait_done_ref_available_event);
        }

        for(i=0; i<data_ref_q->pipeline_depth; i++)
        {
            if(data_ref_q->obj_desc[i])
            {
                tivxObjDescFree((tivx_obj_desc_t**)&data_ref_q->obj_desc[i]);
            }
            if(data_ref_q->obj_desc_cmd[i])
            {
                tivxObjDescFree((tivx_obj_desc_t**)&data_ref_q->obj_desc_cmd[i]);
            }
        }
        if(data_ref_q->acquire_q_obj_desc_id!=TIVX_OBJ_DESC_INVALID)
        {
            tivxObjDescQueueRelease(&data_ref_q->acquire_q_obj_desc_id);
        }
        if(data_ref_q->release_q_obj_desc_id!=TIVX_OBJ_DESC_INVALID)
        {
            tivxObjDescQueueRelease(&data_ref_q->release_q_obj_desc_id);
        }
    }
    return VX_SUCCESS;
}

tivx_data_ref_queue tivxDataRefQueueCreate(vx_graph graph, tivx_data_ref_queue_create_params_t *prms)
{
    tivx_data_ref_queue ref = NULL;
    vx_status status = VX_SUCCESS;

    if(graph!=NULL && prms != NULL)
    {
        ref = (tivx_data_ref_queue)ownCreateReference(graph->base.context,
            TIVX_TYPE_DATA_REF_Q, VX_INTERNAL, &graph->base);

        if ((vxGetStatus((vx_reference)ref) == VX_SUCCESS) &&
            (ref->base.type == TIVX_TYPE_DATA_REF_Q))
        {
            uint32_t pipe_id;

            ref->base.destructor_callback = &tivxDataRefQueueDestruct;
            ref->base.mem_alloc_callback = NULL;
            ref->base.release_callback =
                (tivx_reference_release_callback_f)&tivxDataRefQueueRelease;

            ref->pipeline_depth = prms->pipeline_depth;

            ref->ready_q_obj_desc_id = TIVX_OBJ_DESC_INVALID;
            ref->done_q_obj_desc_id = TIVX_OBJ_DESC_INVALID;
            ref->wait_done_ref_available_event = NULL;
            for(pipe_id=0; pipe_id<TIVX_GRAPH_MAX_PIPELINE_DEPTH; pipe_id++)
            {
                ref->obj_desc_cmd[pipe_id] = NULL;
                ref->obj_desc[pipe_id] = NULL;
            }
            ref->acquire_q_obj_desc_id = TIVX_OBJ_DESC_INVALID;
            ref->release_q_obj_desc_id = TIVX_OBJ_DESC_INVALID;

            ref->is_enable_send_ref_consumed_event = prms->is_enable_send_ref_consumed_event;
            ref->enable_user_queueing = prms->enable_user_queueing;
            ref->graph = graph;
            ref->graph_parameter_index = prms->graph_parameter_index;

            for(pipe_id=0; pipe_id<ref->pipeline_depth; pipe_id++)
            {
                ref->obj_desc[pipe_id] = (tivx_obj_desc_data_ref_q_t*)tivxObjDescAlloc(TIVX_OBJ_DESC_DATA_REF_Q, (vx_reference)ref);
                if(ref->obj_desc[pipe_id]==NULL)
                {
                    status = VX_ERROR_NO_RESOURCES;
                }
                if(status==VX_SUCCESS)
                {
                    /* if user queueing is enabled, need to send response back to host */
                    if(prms->enable_user_queueing)
                    {
                        ref->obj_desc_cmd[pipe_id] = (tivx_obj_desc_cmd_t*)tivxObjDescAlloc(TIVX_OBJ_DESC_CMD, NULL);
                        if(ref->obj_desc_cmd[pipe_id]==NULL)
                        {
                            status = VX_ERROR_NO_RESOURCES;
                        }
                    }
                }
            }
            if(status==VX_SUCCESS)
            {
                if(prms->enable_user_queueing)
                {
                    status = tivxEventCreate(&ref->wait_done_ref_available_event);
                }
            }
            if(status==VX_SUCCESS)
            {
                status = tivxObjDescQueueCreate(&ref->acquire_q_obj_desc_id);
            }
            if(status==VX_SUCCESS)
            {
                if(prms->enable_user_queueing)
                {
                    status = tivxObjDescQueueCreate(&ref->release_q_obj_desc_id);
                }
            }
            if(status==VX_SUCCESS)
            {
                /* all resource acquired now set the data structure fields */
                if(prms->enable_user_queueing)
                {
                    ref->ready_q_obj_desc_id = ref->acquire_q_obj_desc_id;
                    ref->done_q_obj_desc_id = ref->release_q_obj_desc_id;
                }
                for(pipe_id=0; pipe_id<ref->pipeline_depth; pipe_id++)
                {
                    tivx_obj_desc_data_ref_q_t *obj_desc;
                    tivx_obj_desc_cmd_t *obj_desc_cmd;

                    obj_desc = ref->obj_desc[pipe_id];
                    obj_desc_cmd = ref->obj_desc_cmd[pipe_id];

                    obj_desc->flags = 0;
                    obj_desc->ref_consumed_cmd_obj_desc_id = TIVX_OBJ_DESC_INVALID;
                    obj_desc->release_q_obj_desc_id = TIVX_OBJ_DESC_INVALID;

                    if(prms->enable_user_queueing)
                    {
                        tivxFlagBitSet(&obj_desc->flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_ENABLE_REF_CONSUMED_EVENT);
                        obj_desc->ref_consumed_cmd_obj_desc_id = obj_desc_cmd->base.obj_desc_id;
                    }
                    obj_desc->acquire_q_obj_desc_id = ref->acquire_q_obj_desc_id;
                    if(prms->enable_user_queueing)
                    {
                        obj_desc->release_q_obj_desc_id = ref->release_q_obj_desc_id;
                    }
                    else
                    {
                        obj_desc->release_q_obj_desc_id = ref->acquire_q_obj_desc_id;
                    }
                    obj_desc->ref_obj_desc_id = TIVX_OBJ_DESC_INVALID;
                    obj_desc->in_node_done_cnt = 0;
                    obj_desc->num_in_nodes = prms->num_in_nodes;
                    obj_desc->next_obj_desc_id_in_delay = TIVX_OBJ_DESC_INVALID;
                    obj_desc->delay_slot_index = 0;
                    obj_desc->delay_slots = 0;

                    if(prms->enable_user_queueing)
                    {
                        obj_desc_cmd->cmd_id = TIVX_CMD_DATA_REF_CONSUMED;

                        /* No ACK needed */
                        obj_desc_cmd->flags = 0;

                        /* this command is sent by the target node to HOST hence dst_target_id is HOST */
                        obj_desc_cmd->dst_target_id = tivxPlatformGetTargetId(TIVX_TARGET_HOST);

                        /* source is node target which is not known at this moment, however
                         * since ACK is not required for this command, this can be set to INVALID
                         */
                        obj_desc_cmd->src_target_id = TIVX_TARGET_ID_INVALID;

                        /* parameter is node object descriptor ID */
                        obj_desc_cmd->num_obj_desc = 1;
                        obj_desc_cmd->obj_desc_id[0] = obj_desc->base.obj_desc_id;
                    }
                }
            }

            if(status!=VX_SUCCESS)
            {
                ref = NULL;
                VX_PRINT(VX_ZONE_ERROR, "Unable to alloc resources for data ref queue\n");
            }
        }
        else
        {
            ref = NULL;
            VX_PRINT(VX_ZONE_ERROR, "Unable to create data ref queue\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters\n");
    }
    return ref;
}

vx_status tivxDataRefQueueRelease(tivx_data_ref_queue *ref)
{
    return ownReleaseReferenceInt((vx_reference *)ref, TIVX_TYPE_DATA_REF_Q, VX_INTERNAL, NULL);
}

vx_status tivxDataRefQueueLinkDelayDataRefQueues(
            tivx_data_ref_queue delay_data_ref_q_list[],
            vx_bool auto_age_delay_slot[],
            uint32_t delay_slots)
{
    vx_status status = VX_SUCCESS;
    uint32_t pipe_id, i;
    tivx_data_ref_queue cur_data_ref_q, next_data_ref_q;

    for(i=0; i<delay_slots; i++)
    {
        cur_data_ref_q = delay_data_ref_q_list[i];
        next_data_ref_q = delay_data_ref_q_list[(i+1) % delay_slots];

        if(next_data_ref_q->enable_user_queueing)
        {
            cur_data_ref_q->release_q_obj_desc_id = next_data_ref_q->done_q_obj_desc_id;
        }
        else
        {
            cur_data_ref_q->release_q_obj_desc_id = TIVX_OBJ_DESC_INVALID;
        }

        for(pipe_id=0; pipe_id<cur_data_ref_q->pipeline_depth; pipe_id++)
        {
            tivx_obj_desc_data_ref_q_t *obj_desc, *next_obj_desc;

            obj_desc = cur_data_ref_q->obj_desc[pipe_id];
            next_obj_desc = next_data_ref_q->obj_desc[pipe_id];

            tivxFlagBitSet(&obj_desc->flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_IN_DELAY);
            obj_desc->next_obj_desc_id_in_delay = next_obj_desc->base.obj_desc_id;
            obj_desc->delay_slot_index = i;
            obj_desc->delay_slots = delay_slots;

            if(auto_age_delay_slot[i])
            {
                tivxFlagBitSet(&obj_desc->flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_DELAY_SLOT_AUTO_AGE);
            }

            if(next_data_ref_q->enable_user_queueing)
            {
                obj_desc->release_q_obj_desc_id = next_data_ref_q->done_q_obj_desc_id;
            }
            else
            {
                obj_desc->release_q_obj_desc_id = next_data_ref_q->acquire_q_obj_desc_id;
            }
            if(next_data_ref_q->enable_user_queueing)
            {
                tivxFlagBitSet(&obj_desc->flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_ENABLE_REF_CONSUMED_EVENT);
                obj_desc->ref_consumed_cmd_obj_desc_id = next_data_ref_q->obj_desc_cmd[pipe_id]->base.obj_desc_id;
            }
            else
            {
                tivxFlagBitClear(&obj_desc->flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_ENABLE_REF_CONSUMED_EVENT);
                obj_desc->ref_consumed_cmd_obj_desc_id = TIVX_OBJ_DESC_INVALID;
            }
        }
    }
    return status;
}
