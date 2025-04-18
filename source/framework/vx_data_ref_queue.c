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

static vx_status ownDataRefQueueDestruct(vx_reference ref);

vx_status ownDataRefQueueWaitDoneRef(tivx_data_ref_queue data_ref_q, vx_uint32 timeout)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(data_ref_q == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "'data_ref_q' is invalid or 'queue_id' is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        status = tivxEventWait(data_ref_q->wait_done_ref_available_event, timeout);
    }

    return status;
}

vx_status ownDataRefQueueGetDoneQueueCount(tivx_data_ref_queue data_ref_q, vx_uint32 *count)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if((data_ref_q == NULL) || (count == NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,
            "'data_ref_q' is invalid or 'count' is invalid or 'queue_id' is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        uint16_t queue_obj_desc_id;

        /* get queue object descriptor */
        queue_obj_desc_id = data_ref_q->done_q_obj_desc_id;

        status = ownObjDescQueueGetCount(queue_obj_desc_id, count);
    }
    return status;
}

vx_status ownDataRefQueueGetReadyQueueCount(tivx_data_ref_queue data_ref_q, vx_uint32 *count)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if((data_ref_q == NULL) || (count == NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,
            "'data_ref_q' is invalid or 'count' is invalid or 'queue_id' is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        uint16_t queue_obj_desc_id;

        /* get queue object descriptor */
        queue_obj_desc_id = data_ref_q->ready_q_obj_desc_id;

        status = ownObjDescQueueGetCount(queue_obj_desc_id, count);
    }
    return status;
}

uint16_t ownDataRefQueueGetObjDescId(tivx_data_ref_queue ref, uint32_t pipeline_id)
{
    uint16_t obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

    if((ref != NULL) && (pipeline_id < ref->pipeline_depth))
    {
        obj_desc_id = ref->obj_desc[pipeline_id]->base.obj_desc_id;
    }
    return obj_desc_id;
}

vx_status ownDataRefQueueSendRefConsumedEvent(tivx_data_ref_queue ref, uint64_t timestamp)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(ref!=NULL)
    {
        if(ref->wait_done_ref_available_event != NULL)
        {
            status = tivxEventPost(ref->wait_done_ref_available_event);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_DATA_REF_QUEUE_UM001
<justification end> */
            if((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,"Failed to add post event\n");
            }
/* LDRA_JUSTIFY_END */
        }
        if(  ((vx_bool)vx_true_e == ref->base.context->event_queue.enable) &&
             ((vx_bool)vx_true_e == ref->is_enable_send_ref_consumed_event) )
        {
            status = ownEventQueueAddEvent(&ref->base.context->event_queue,
                        (vx_enum)VX_EVENT_GRAPH_PARAMETER_CONSUMED, timestamp, ref->graph->parameters[ref->graph_parameter_index].graph_consumed_context_app_value,
                        (uintptr_t)ref->graph, (uintptr_t)ref->graph_parameter_index, (uintptr_t)0);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_DATA_REF_QUEUE_UM002
<justification end> */
            if((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,"Failed to add event to context event queue\n");
            }
/* LDRA_JUSTIFY_END */
        }
        if(  ((vx_bool)vx_true_e == ref->graph->event_queue.enable) &&
             ((vx_bool)vx_true_e == ref->is_enable_send_ref_consumed_graph_event) )
        {
            status = ownEventQueueAddEvent(&ref->graph->event_queue,
                        (vx_enum)VX_EVENT_GRAPH_PARAMETER_CONSUMED, timestamp, ref->graph->parameters[ref->graph_parameter_index].graph_consumed_graph_app_value,
                        (uintptr_t)ref->graph, (uintptr_t)ref->graph_parameter_index, (uintptr_t)0);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_DATA_REF_QUEUE_UM002
<justification end> */
            if((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,"Failed to add event to graph event queue\n");
            }
/* LDRA_JUSTIFY_END */
        }
    }
    else
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
    }

    return status;
}

static vx_status ownDataRefQueueDestruct(vx_reference ref)
{
    uint32_t i;
    vx_status status=(vx_status)VX_SUCCESS;
    vx_bool do_break = (vx_bool)vx_false_e;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_DATA_REF_QUEUE_UBR003
<justification end> */
    if(ref->type == (vx_enum)TIVX_TYPE_DATA_REF_Q)
/* LDRA_JUSTIFY_END */
    {
        tivx_data_ref_queue data_ref_q = (tivx_data_ref_queue)ref;

        if(data_ref_q->wait_done_ref_available_event!=NULL)
        {
           /* Error status check not done since
            * there is previous NULL check
            */
           (void)tivxEventDelete(&data_ref_q->wait_done_ref_available_event);
        }
        for(i=0; i<data_ref_q->pipeline_depth; i++)
        {
            if(data_ref_q->obj_desc[i] != NULL)
            {
                status = ownObjDescFree((tivx_obj_desc_t**)&data_ref_q->obj_desc[i]);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_DATA_REF_QUEUE_UM003
<justification end> */
                if((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Failed to release object descriptor\n");
                    do_break = (vx_bool)vx_true_e;
                }
/* LDRA_JUSTIFY_END */
            }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_DATA_REF_QUEUE_UBR004
<justification end> */
            if((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
            {
                if(data_ref_q->obj_desc_cmd[i] != NULL)
                {
                    status = ownObjDescFree((tivx_obj_desc_t**)&data_ref_q->obj_desc_cmd[i]);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_DATA_REF_QUEUE_UM004
<justification end> */
                    if((vx_status)VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Failed to release object descriptor\n");
                        do_break = (vx_bool)vx_true_e;
                    }
/* LDRA_JUSTIFY_END */
                }
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_DATA_REF_QUEUE_UM005
<justification end> */
            if((vx_bool)vx_true_e == do_break)
            {
                break;
            }
/* LDRA_JUSTIFY_END */
        }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_DATA_REF_QUEUE_UBR005
<justification end> */
        if(status == (vx_status)VX_SUCCESS)
/* LDRA_JUSTIFY_END */
        {
            if((vx_enum)data_ref_q->acquire_q_obj_desc_id!=(vx_enum)TIVX_OBJ_DESC_INVALID)
            {
                status = ownObjDescQueueRelease(&data_ref_q->acquire_q_obj_desc_id);
            }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_DATA_REF_QUEUE_UBR006
<justification end> */
            if(status == (vx_status)VX_SUCCESS)
            {
                if((vx_enum)data_ref_q->release_q_obj_desc_id!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                {
                    status = ownObjDescQueueRelease(&data_ref_q->release_q_obj_desc_id);
                }
            }
/* LDRA_JUSTIFY_END */
        }
    }
    return status;
}

tivx_data_ref_queue tivxDataRefQueueCreate(vx_graph graph, const tivx_data_ref_queue_create_params_t *prms)
{
    tivx_data_ref_queue ref = NULL;
    vx_status status = (vx_status)VX_SUCCESS;

    if((graph != NULL) && (prms != NULL))
    {
        ref = (tivx_data_ref_queue)ownCreateReference(graph->base.context,
            (vx_enum)TIVX_TYPE_DATA_REF_Q, (vx_enum)VX_INTERNAL, &graph->base);
        if ((vxGetStatus((vx_reference)ref) == (vx_status)VX_SUCCESS) && /* TIOVX-1934- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_DATA_REF_QUEUE_UBR007 */
            (ref->base.type == (vx_enum)TIVX_TYPE_DATA_REF_Q)) /* TIOVX-1934- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_DATA_REF_QUEUE_UBR008 */
        {
            uint32_t pipe_id;

            ref->base.destructor_callback = &ownDataRefQueueDestruct;
            ref->base.mem_alloc_callback = NULL;
            ref->base.release_callback =
                &ownReleaseReferenceBufferGeneric;

            ref->pipeline_depth = prms->pipeline_depth;

            ref->ready_q_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
            ref->done_q_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
            ref->wait_done_ref_available_event = NULL;
            for(pipe_id=0; pipe_id<TIVX_GRAPH_MAX_PIPELINE_DEPTH; pipe_id++)
            {
                ref->obj_desc_cmd[pipe_id] = NULL;
                ref->obj_desc[pipe_id] = NULL;
            }
            ref->acquire_q_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
            ref->release_q_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

            ref->is_enable_send_ref_consumed_event = prms->is_enable_send_ref_consumed_event;
            ref->is_enable_send_ref_consumed_graph_event = prms->is_enable_send_ref_consumed_graph_event;
            ref->enable_user_queueing = prms->enable_user_queueing;
            ref->graph = graph;
            ref->graph_parameter_index = prms->graph_parameter_index;

            for(pipe_id=0; pipe_id<ref->pipeline_depth; pipe_id++)
            {
                ref->obj_desc[pipe_id] = (tivx_obj_desc_data_ref_q_t*)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_DATA_REF_Q, (vx_reference)ref);
                if(ref->obj_desc[pipe_id]==NULL)
                {
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate data reference queue object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in include/TI/soc/tivx_config_<soc>.h\n");
                }
                if(status==(vx_status)VX_SUCCESS)
                {
                    /* if user queueing is enabled, need to send response back to host */
                    if(prms->enable_user_queueing != 0)
                    {
                        ref->obj_desc_cmd[pipe_id] = (tivx_obj_desc_cmd_t*)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_CMD, NULL);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_DATA_REF_QUEUE_UTJT001
<justification end> */
                        if(ref->obj_desc_cmd[pipe_id]==NULL)
                        {
                            status = (vx_status)VX_ERROR_NO_RESOURCES;
                            VX_PRINT(VX_ZONE_ERROR, "Could not allocate object descriptor\n");
                            VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                            VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in include/TI/soc/tivx_config_<soc>.h\n");
                        }
/* LDRA_JUSTIFY_END */
                    }
                }
            }
            if(status==(vx_status)VX_SUCCESS)
            {
                if(prms->enable_user_queueing != 0)
                {
                    status = tivxEventCreate(&ref->wait_done_ref_available_event);
                }
            }
            if(status==(vx_status)VX_SUCCESS)
            {
                status = ownObjDescQueueCreate(&ref->acquire_q_obj_desc_id);
            }
            if(status==(vx_status)VX_SUCCESS)
            {
                if(prms->enable_user_queueing != 0)
                {
                    status = ownObjDescQueueCreate(&ref->release_q_obj_desc_id);
                }
            }
            if(status==(vx_status)VX_SUCCESS)
            {
                /* all resource acquired now set the data structure fields */
                if(prms->enable_user_queueing != 0)
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
                    obj_desc->ref_consumed_cmd_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
                    obj_desc->release_q_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

                    if(prms->enable_user_queueing != 0)
                    {
                        tivxFlagBitSet(&obj_desc->flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_ENABLE_REF_CONSUMED_EVENT);
                        obj_desc->ref_consumed_cmd_obj_desc_id = obj_desc_cmd->base.obj_desc_id;
                    }
                    obj_desc->acquire_q_obj_desc_id = ref->acquire_q_obj_desc_id;
                    if(prms->enable_user_queueing != 0)
                    {
                        obj_desc->release_q_obj_desc_id = ref->release_q_obj_desc_id;
                    }
                    else
                    {
                        obj_desc->release_q_obj_desc_id = ref->acquire_q_obj_desc_id;
                    }
                    obj_desc->ref_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
                    obj_desc->in_node_done_cnt = 0;
                    obj_desc->num_in_nodes = (vx_uint16)prms->num_in_nodes;
                    obj_desc->next_obj_desc_id_in_delay = (vx_enum)TIVX_OBJ_DESC_INVALID;
                    obj_desc->delay_slot_index = 0;
                    obj_desc->delay_slots = 0;

                    if(prms->enable_user_queueing != 0)
                    {
                        obj_desc_cmd->cmd_id = (vx_enum)TIVX_CMD_DATA_REF_CONSUMED;

                        /* No ACK needed */
                        obj_desc_cmd->flags = 0;

                        /* this command is sent by the target node to HOST hence dst_target_id is HOST */
                        obj_desc_cmd->dst_target_id = (vx_uint32)ownPlatformGetTargetId(TIVX_TARGET_HOST);

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

            if(status!=(vx_status)VX_SUCCESS)
            {
                (void)ownDataRefQueueRelease(&ref);
                ref = NULL;
                VX_PRINT(VX_ZONE_ERROR, "Unable to alloc resources for data ref queue\n");
            }
        }
        else /* TIOVX_CODE_COVERAGE_DATA_REF_QUEUE_UTJT002 */
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

vx_status ownDataRefQueueRelease(tivx_data_ref_queue *ref)
{
    return ownReleaseReferenceInt((vx_reference *)ref, (vx_enum)TIVX_TYPE_DATA_REF_Q, (vx_enum)VX_INTERNAL, NULL);
}

vx_status ownDataRefQueueLinkDelayDataRefQueues(
            tivx_data_ref_queue delay_data_ref_q_list[],
            vx_bool auto_age_delay_slot[],
            uint32_t delay_slots)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t pipe_id, i;
    tivx_data_ref_queue cur_data_ref_q, next_data_ref_q;

    for(i=0; i<delay_slots; i++)
    {
        cur_data_ref_q = delay_data_ref_q_list[i];
        next_data_ref_q = delay_data_ref_q_list[(i+1U) % delay_slots];

        if(next_data_ref_q->enable_user_queueing != 0)
        {
            cur_data_ref_q->release_q_obj_desc_id = next_data_ref_q->done_q_obj_desc_id;
        }
        else
        {
            cur_data_ref_q->release_q_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
        }

        for(pipe_id=0; pipe_id<cur_data_ref_q->pipeline_depth; pipe_id++)
        {
            tivx_obj_desc_data_ref_q_t *obj_desc, *next_obj_desc;

            obj_desc = cur_data_ref_q->obj_desc[pipe_id];
            next_obj_desc = next_data_ref_q->obj_desc[pipe_id];

            tivxFlagBitSet(&obj_desc->flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_IN_DELAY);
            obj_desc->next_obj_desc_id_in_delay = next_obj_desc->base.obj_desc_id;
            obj_desc->delay_slot_index = (vx_uint16)i;
            obj_desc->delay_slots = (vx_uint16)delay_slots;

            if(auto_age_delay_slot[i] != 0)
            {
                tivxFlagBitSet(&obj_desc->flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_DELAY_SLOT_AUTO_AGE);
            }

            if(next_data_ref_q->enable_user_queueing != 0)
            {
                obj_desc->release_q_obj_desc_id = next_data_ref_q->done_q_obj_desc_id;
            }
            else
            {
                obj_desc->release_q_obj_desc_id = next_data_ref_q->acquire_q_obj_desc_id;
            }
            if(next_data_ref_q->enable_user_queueing != 0)
            {
                tivxFlagBitSet(&obj_desc->flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_ENABLE_REF_CONSUMED_EVENT);
                obj_desc->ref_consumed_cmd_obj_desc_id = next_data_ref_q->obj_desc_cmd[pipe_id]->base.obj_desc_id;
            }
            else
            {
                tivxFlagBitClear(&obj_desc->flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_ENABLE_REF_CONSUMED_EVENT);
                obj_desc->ref_consumed_cmd_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
            }
        }
    }
    return status;
}
