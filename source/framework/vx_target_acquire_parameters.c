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

/*
 * this file has APIs to acquire and release parameters when graph pipelining
 * is enabled.
 * This is critical piece of logic and to keep it readable from rest of the logic
 * is placed in a file of its own
 */

#include <vx_internal.h>

static void ownTargetNodeDescAcquireParameter(
                    tivx_obj_desc_node_t *node_obj_desc,
                    tivx_obj_desc_data_ref_q_t *data_ref_q_obj_desc, /* data ref q obj desc */
                    uint16_t *prm_obj_desc_id, /* extracted parameter ref */
                    vx_bool *is_node_blocked);
static void ownTargetObjDescSendRefConsumed(
    const tivx_obj_desc_data_ref_q_t *obj_desc);
static void ownTargetNodeDescAcquireParameterForPipeup(
                    tivx_obj_desc_node_t *node_obj_desc,
                    tivx_obj_desc_data_ref_q_t *data_ref_q_obj_desc,
                    uint16_t *prm_obj_desc_id);
static void ownTargetNodeDescReleaseParameterInDelay(
                        tivx_obj_desc_data_ref_q_t *data_ref_q_obj_desc,
                        tivx_obj_desc_queue_blocked_nodes_t *blocked_nodes);
static void ownTargetNodeDescReleaseParameter(
                    tivx_obj_desc_node_t *node_obj_desc,
                    tivx_obj_desc_data_ref_q_t *data_ref_q_obj_desc,
                    uint16_t ref_obj_desc_id,
                    vx_bool is_prm_input,
                    vx_bool *is_prm_released);

static void ownTargetObjDescSendRefConsumed(
    const tivx_obj_desc_data_ref_q_t *obj_desc)
{
    uint16_t cmd_obj_desc_id;

    if(0 != tivxFlagIsBitSet(obj_desc->flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_ENABLE_REF_CONSUMED_EVENT))
    {
        cmd_obj_desc_id = obj_desc->ref_consumed_cmd_obj_desc_id;

        if((vx_enum)cmd_obj_desc_id != (vx_enum)TIVX_OBJ_DESC_INVALID)
        {
            tivx_obj_desc_cmd_t *cmd_obj_desc = (tivx_obj_desc_cmd_t *)ownObjDescGet(cmd_obj_desc_id);

            if(0 != ownObjDescIsValidType((tivx_obj_desc_t*)cmd_obj_desc, TIVX_OBJ_DESC_CMD))
            {
                uint64_t timestamp = tivxPlatformGetTimeInUsecs()*1000U;

                tivx_uint64_to_uint32(
                    timestamp,
                    &cmd_obj_desc->timestamp_h,
                    &cmd_obj_desc->timestamp_l
                );

                VX_PRINT(VX_ZONE_INFO, "Sending reference consumed event (data_ref_q=%d) !!!\n", obj_desc->base.obj_desc_id);

                /* users wants a notification of data reference consumed
                 * so send command to host
                 */
                ownObjDescSend( cmd_obj_desc->dst_target_id, cmd_obj_desc_id);
            }
        }
    }
}

static void ownTargetNodeDescAcquireParameter(
                    tivx_obj_desc_node_t *node_obj_desc,
                    tivx_obj_desc_data_ref_q_t *data_ref_q_obj_desc, /* data ref q obj desc */
                    uint16_t *prm_obj_desc_id, /* extracted parameter ref */
                    vx_bool *is_node_blocked)
{
    uint32_t flags;

    *prm_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

    ownPlatformSystemLock((vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);

    flags = data_ref_q_obj_desc->flags;

    if(tivxFlagIsBitSet(flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_REF_ACQUIRED)==(vx_bool)vx_false_e)
    {
        uint16_t ref_obj_desc_id, obj_desc_q_id;

        obj_desc_q_id = data_ref_q_obj_desc->acquire_q_obj_desc_id;

        ownObjDescQueueDequeue(
            obj_desc_q_id,
            &ref_obj_desc_id
            );

        if((vx_enum)ref_obj_desc_id==(vx_enum)TIVX_OBJ_DESC_INVALID) /* did not get a ref */
        {
            /* add node to list of blocked nodes */
            ownObjDescQueueAddBlockedNode(
                obj_desc_q_id,
                node_obj_desc->base.obj_desc_id
                );
            *is_node_blocked = (vx_bool)vx_true_e;

            /* mark current node as blocked, blocked on parameter acquire */
            node_obj_desc->state = TIVX_NODE_OBJ_DESC_STATE_BLOCKED;

            VX_PRINT(VX_ZONE_INFO,"Parameter acquire failed ... BLOCKING (node=%d, pipe=%d, data_ref_q=%d, queue=%d)\n",
                             node_obj_desc->base.obj_desc_id,
                             node_obj_desc->pipeline_id,
                             data_ref_q_obj_desc->base.obj_desc_id,
                             obj_desc_q_id
                       );
        }
        else
        {
            tivx_obj_desc_t *obj_desc;

            tivxFlagBitSet(&flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_REF_ACQUIRED);

            data_ref_q_obj_desc->ref_obj_desc_id = ref_obj_desc_id;
            data_ref_q_obj_desc->in_node_done_cnt = 0;
            data_ref_q_obj_desc->flags = flags;

            obj_desc = ownObjDescGet(ref_obj_desc_id);
            if(obj_desc != NULL)
            {
                obj_desc->in_node_done_cnt = 0;
            }

            *prm_obj_desc_id = ref_obj_desc_id;

            VX_PRINT(VX_ZONE_INFO,"Parameter acquired (node=%d, pipe=%d, data_ref_q=%d, queue=%d, ref=%d)\n",
                                node_obj_desc->base.obj_desc_id,
                                node_obj_desc->pipeline_id,
                                data_ref_q_obj_desc->base.obj_desc_id,
                                obj_desc_q_id,
                                ref_obj_desc_id
                           );
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_INFO,"Parameter ALREADY acquired (node=%d, pipe=%d, data_ref_q=%d, ref=%d)\n",
                                node_obj_desc->base.obj_desc_id,
                                node_obj_desc->pipeline_id,
                                data_ref_q_obj_desc->base.obj_desc_id,
                                data_ref_q_obj_desc->ref_obj_desc_id
                            );

        *prm_obj_desc_id = data_ref_q_obj_desc->ref_obj_desc_id;
    }

    ownPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);
}

static void ownTargetNodeDescAcquireParameterForPipeup(
                    tivx_obj_desc_node_t *node_obj_desc,
                    tivx_obj_desc_data_ref_q_t *data_ref_q_obj_desc, /* data ref q obj desc */
                    uint16_t *prm_obj_desc_id /* extracted parameter ref */
                    )
{
    uint32_t flags;

    *prm_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

    ownPlatformSystemLock((vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);

    flags = data_ref_q_obj_desc->flags;

    if(tivxFlagIsBitSet(flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_REF_ACQUIRED)==(vx_bool)vx_false_e)
    {
        uint16_t ref_obj_desc_id, obj_desc_q_id;

        obj_desc_q_id = data_ref_q_obj_desc->acquire_q_obj_desc_id;

        ownObjDescQueueDequeue(
            obj_desc_q_id,
            &ref_obj_desc_id
            );

        if((vx_enum)ref_obj_desc_id==(vx_enum)TIVX_OBJ_DESC_INVALID) /* did not get a ref */
        {
            VX_PRINT(VX_ZONE_INFO,"Parameter acquire for pipe up failed ... (node=%d, pipe=%d, data_ref_q=%d, queue=%d)\n",
                             node_obj_desc->base.obj_desc_id,
                             node_obj_desc->pipeline_id,
                             data_ref_q_obj_desc->base.obj_desc_id,
                             obj_desc_q_id
                       );
        }
        else
        {
            tivx_obj_desc_t *obj_desc;

            /* dont set acquired flag, since this is pipe up phase, just extract the data obj desc */
            obj_desc = ownObjDescGet(ref_obj_desc_id);
            if(obj_desc != NULL)
            {
                obj_desc->in_node_done_cnt = 0;
            }

            *prm_obj_desc_id = ref_obj_desc_id;

            VX_PRINT(VX_ZONE_INFO,"Parameter acquired for pipe up (node=%d, pipe=%d, data_ref_q=%d, queue=%d, ref=%d)\n",
                                node_obj_desc->base.obj_desc_id,
                                node_obj_desc->pipeline_id,
                                data_ref_q_obj_desc->base.obj_desc_id,
                                obj_desc_q_id,
                                ref_obj_desc_id
                           );
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_INFO,"Parameter ALREADY acquired for pipe up (node=%d, pipe=%d, data_ref_q=%d, ref=%d)\n",
                                node_obj_desc->base.obj_desc_id,
                                node_obj_desc->pipeline_id,
                                data_ref_q_obj_desc->base.obj_desc_id,
                                data_ref_q_obj_desc->ref_obj_desc_id
                            );

        *prm_obj_desc_id = data_ref_q_obj_desc->ref_obj_desc_id;
    }

    ownPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);
}

static void ownTargetNodeDescReleaseParameterInDelay(
                        tivx_obj_desc_data_ref_q_t *data_ref_q_obj_desc,
                        tivx_obj_desc_queue_blocked_nodes_t *blocked_nodes)
{
    tivx_obj_desc_data_ref_q_t *next_data_ref_q, *cur_data_ref_q;
    uint16_t obj_desc_q_id, ref_obj_desc_id;
    uint16_t delay_slots, i;

    delay_slots = data_ref_q_obj_desc->delay_slots;

    cur_data_ref_q = data_ref_q_obj_desc;

    for(i=0; i<(delay_slots-1U); i++)
    {
        next_data_ref_q = (tivx_obj_desc_data_ref_q_t*)ownObjDescGet(cur_data_ref_q->next_obj_desc_id_in_delay);

        if(next_data_ref_q!=NULL)
        {
            if(0 != tivxFlagIsBitSet(next_data_ref_q->flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_DELAY_SLOT_AUTO_AGE))
            {
                /* acquire a ref and release it immediately to rotate the ref at this slot */
                obj_desc_q_id = next_data_ref_q->acquire_q_obj_desc_id;

                ownObjDescQueueDequeue(
                    obj_desc_q_id,
                    &ref_obj_desc_id
                    );

                if((vx_enum)ref_obj_desc_id!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                {
                    obj_desc_q_id = next_data_ref_q->release_q_obj_desc_id;

                    ownObjDescQueueEnqueue(
                        obj_desc_q_id,
                        ref_obj_desc_id
                        );
                    /* check if anyone was blocked on this buffer being available
                     * if yes then get the node IDs and trigger them
                     */
                    ownObjDescQueueExtractBlockedNodes(
                        obj_desc_q_id,
                        blocked_nodes
                        );
                }
            }
            cur_data_ref_q = next_data_ref_q;
        }
        else
        {
            /* invalid descriptor found */
            break;
        }
    }
}

static void ownTargetNodeDescReleaseParameter(
                    tivx_obj_desc_node_t *node_obj_desc,
                    tivx_obj_desc_data_ref_q_t *data_ref_q_obj_desc, /* data ref q obj desc */
                    uint16_t ref_obj_desc_id, /* parameter ref to release */
                    vx_bool is_prm_input,
                    vx_bool *is_prm_released)
{
    uint32_t flags;
    uint16_t node_id, num_in_nodes;
    tivx_obj_desc_queue_blocked_nodes_t blocked_nodes;
    vx_bool do_release_ref;
    vx_bool do_release_ref_to_queue;
    tivx_obj_desc_t *obj_desc;
    tivx_obj_desc_t *parent_obj_desc;

    *is_prm_released = (vx_bool)vx_false_e;
    blocked_nodes.num_nodes = 0;
    blocked_nodes.node_id[0] = 0;
    do_release_ref = (vx_bool)vx_false_e;
    do_release_ref_to_queue = (vx_bool)vx_false_e;
    obj_desc = ownObjDescGet(ref_obj_desc_id);

    ownPlatformSystemLock((vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);

    flags = data_ref_q_obj_desc->flags;

    if(is_prm_input == (vx_bool)vx_true_e)
    {
        data_ref_q_obj_desc->in_node_done_cnt++;
        if(data_ref_q_obj_desc->in_node_done_cnt==data_ref_q_obj_desc->num_in_nodes)
        {
            do_release_ref = (vx_bool)vx_true_e;
        }
        if(obj_desc!=NULL)
        {
            obj_desc->in_node_done_cnt++;

            /* In order to fix TIOVX-956 and TIOVX-1151, the bracketed logic was added to handle multiple obj arr
             * situations. The bug found that when a replicated node output was consumed by both another
             * replicated node as well as a node consuming the full object array, the reference was never
             * released. This was because the do_release_ref_to_queue was not set to true because the two consumed
             * descriptors were different, i.e., the replicated node input was an element of an obj array
             * and the full obj array was the obj array itself.  Therefore, instead of checking the
             * in_node_done_cnt of the obj_desc, we need to also query the parent_obj_desc to see if this obj desc
             * has also been released and check this against the check the data_ref_q_obj_desc->num_in_nodes. */
            {
                parent_obj_desc = ownObjDescGet(obj_desc->scope_obj_desc_id);

                if(parent_obj_desc!=NULL)
                {
                    num_in_nodes = obj_desc->in_node_done_cnt + parent_obj_desc->in_node_done_cnt;
                }
                else
                {
                    num_in_nodes = obj_desc->in_node_done_cnt;
                }
            }

            if(num_in_nodes==data_ref_q_obj_desc->num_in_nodes)
            {
                do_release_ref_to_queue = (vx_bool)vx_true_e;
                /* Note: This is needed because the delay obj_desc does not get re-acquired for each slot.
                 *       Therefore, each in_node_done_cnt must be reset for each delay slot. */
                obj_desc->in_node_done_cnt = 0;
            }
        }
    }
    else
    if( (is_prm_input == (vx_bool)vx_false_e)
        && (data_ref_q_obj_desc->num_in_nodes == 0U) /* i.e this node is not consumed by the graph */
        )
    {
        do_release_ref = (vx_bool)vx_true_e;
        if(obj_desc!=NULL)
        {
            do_release_ref_to_queue = (vx_bool)vx_true_e;
        }
    }
    else
    {
        /* this is a output and is used as input but some other node */
        data_ref_q_obj_desc->ref_obj_desc_id = ref_obj_desc_id;
    }
    if((do_release_ref != (vx_bool)vx_false_e) || (do_release_ref_to_queue != (vx_bool)vx_false_e))
    {
        if(do_release_ref_to_queue != (vx_bool)vx_false_e)
        {
            uint16_t obj_desc_q_id;

            /* all input nodes have released this input
             */
            obj_desc_q_id = data_ref_q_obj_desc->release_q_obj_desc_id;

            ownObjDescQueueEnqueue(
                obj_desc_q_id,
                ref_obj_desc_id
                );
            /* check if anyone was blocked on this buffer being available
             * if yes then get the node IDs and trigger them
             */
            ownObjDescQueueExtractBlockedNodes(
                obj_desc_q_id,
                &blocked_nodes
                );

            *is_prm_released = (vx_bool)vx_true_e;

            /* handle ref auto age for delay
             * if delay is connected to some input node then acquire/release
             * at the node takes care of ref ageing at the delay slot
             * However if a delay slot is not connected to any node then
             * ageing must be done explicitly here.
             */
            if(0 != tivxFlagIsBitSet(data_ref_q_obj_desc->flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_IN_DELAY))
            {
                ownTargetNodeDescReleaseParameterInDelay(data_ref_q_obj_desc, &blocked_nodes);
            }

            VX_PRINT(VX_ZONE_INFO,"Parameter released (node=%d, pipe=%d, data_ref_q=%d, queue=%d, ref=%d)\n",
                                 node_obj_desc->base.obj_desc_id,
                                 node_obj_desc->pipeline_id,
                                 data_ref_q_obj_desc->base.obj_desc_id,
                                 obj_desc_q_id,
                                 ref_obj_desc_id
                           );
        }
        if(do_release_ref != (vx_bool)vx_false_e)
        {
            tivxFlagBitClear(&flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_REF_ACQUIRED);
            data_ref_q_obj_desc->flags = flags;
        }
    }
    else
    {
        if(obj_desc!=NULL)
        {
            VX_PRINT(VX_ZONE_INFO,"Parameter NOT released (node=%d, pipe=%d, data_ref_q=%d, ref=%d, users=%d)\n",
                             node_obj_desc->base.obj_desc_id,
                             node_obj_desc->pipeline_id,
                             data_ref_q_obj_desc->base.obj_desc_id,
                             ref_obj_desc_id,
                             data_ref_q_obj_desc->num_in_nodes - obj_desc->in_node_done_cnt
                       );
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO,"Parameter NOT released (node=%d, pipe=%d, data_ref_q=%d, ref=%d, max_users=%d)\n",
                             node_obj_desc->base.obj_desc_id,
                             node_obj_desc->pipeline_id,
                             data_ref_q_obj_desc->base.obj_desc_id,
                             ref_obj_desc_id,
                             data_ref_q_obj_desc->num_in_nodes
                       );
        }
    }
    ownPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);

    for(node_id=0; node_id<blocked_nodes.num_nodes; node_id++)
    {
        VX_PRINT(VX_ZONE_INFO,"Re-triggering (node=%d)\n",
                                 blocked_nodes.node_id[node_id]
                           );

        ownTargetTriggerNode(blocked_nodes.node_id[node_id]);
    }
}

void ownTargetNodeDescAcquireAllParameters(tivx_obj_desc_node_t *node_obj_desc,
            uint16_t prm_obj_desc_id[], vx_bool *is_node_blocked)
{
    uint32_t prm_id;
    vx_bool is_prm_data_ref_q_flag;

    *is_node_blocked = (vx_bool)vx_false_e;

    is_prm_data_ref_q_flag = (int32_t)node_obj_desc->is_prm_data_ref_q;

    for(prm_id=0; prm_id<node_obj_desc->num_params; prm_id++)
    {
        if(tivxFlagIsBitSet((uint32_t)is_prm_data_ref_q_flag, ((uint32_t)1U<<prm_id))==(vx_bool)vx_false_e)
        {
            prm_obj_desc_id[prm_id] = node_obj_desc->data_id[prm_id];
        }
        else
        {
            tivx_obj_desc_data_ref_q_t *data_ref_q_obj_desc;

            data_ref_q_obj_desc = (tivx_obj_desc_data_ref_q_t*)ownObjDescGet(node_obj_desc->data_ref_q_id[prm_id]);

            if(0 != ownObjDescIsValidType((tivx_obj_desc_t*)data_ref_q_obj_desc, TIVX_OBJ_DESC_DATA_REF_Q))
            {
                ownTargetNodeDescAcquireParameter(
                    node_obj_desc,
                    data_ref_q_obj_desc, /* data ref q obj desc */
                    &prm_obj_desc_id[prm_id], /* extracted parameter ref */
                    is_node_blocked
                    );
            }
            else
            {
                prm_obj_desc_id[prm_id] = (vx_enum)TIVX_OBJ_DESC_INVALID;
            }
        }
        #if 1
        if(0 != *is_node_blocked)
        {
            /* node is blocked on some resource, dont acquire more resources */
            VX_PRINT(VX_ZONE_INFO,"Parameter acquire ... ABORTING (node=%d, pipe=%d)\n",
                        node_obj_desc->base.obj_desc_id,
                        node_obj_desc->pipeline_id
                );

            break;
        }
        #endif
    }
}

void ownTargetNodeDescAcquireAllParametersForPipeup(tivx_obj_desc_node_t *node_obj_desc,
            uint16_t prm_obj_desc_id[])
{
    uint32_t prm_id;
    vx_bool is_prm_data_ref_q_flag;

    is_prm_data_ref_q_flag = (int32_t)node_obj_desc->is_prm_data_ref_q;

    for(prm_id=0; prm_id<node_obj_desc->num_params; prm_id++)
    {
        if(tivxFlagIsBitSet((uint32_t)is_prm_data_ref_q_flag, ((uint32_t)1U<<prm_id))==(vx_bool)vx_false_e)
        {
            prm_obj_desc_id[prm_id] = node_obj_desc->data_id[prm_id];
        }
        else
        {
            tivx_obj_desc_data_ref_q_t *data_ref_q_obj_desc;

            data_ref_q_obj_desc = (tivx_obj_desc_data_ref_q_t*)ownObjDescGet(node_obj_desc->data_ref_q_id[prm_id]);

            if(0 != ownObjDescIsValidType((tivx_obj_desc_t*)data_ref_q_obj_desc, TIVX_OBJ_DESC_DATA_REF_Q))
            {
                ownTargetNodeDescAcquireParameterForPipeup(
                    node_obj_desc,
                    data_ref_q_obj_desc, /* data ref q obj desc */
                    &prm_obj_desc_id[prm_id] /* extracted parameter ref */
                    );
            }
            else
            {
                prm_obj_desc_id[prm_id] = (vx_enum)TIVX_OBJ_DESC_INVALID;
            }
        }
    }
}

void ownTargetNodeDescReleaseAllParameters(tivx_obj_desc_node_t *node_obj_desc, uint16_t prm_obj_desc_id[])
{
    uint32_t is_prm_input_flag, is_prm_data_ref_q_flag, prm_id;
    vx_bool is_prm_released, is_prm_input;

    is_prm_data_ref_q_flag = node_obj_desc->is_prm_data_ref_q;
    is_prm_input_flag = node_obj_desc->is_prm_input;

    for(prm_id=0; prm_id<node_obj_desc->num_params; prm_id++)
    {
        if(tivxFlagIsBitSet(is_prm_data_ref_q_flag, ((uint32_t)1U<<prm_id))==(vx_bool)vx_false_e)
        {
            /* not a data ref q, nothing to release */
        }
        else
        {
            tivx_obj_desc_data_ref_q_t *data_ref_q_obj_desc;

            data_ref_q_obj_desc = (tivx_obj_desc_data_ref_q_t*)ownObjDescGet(node_obj_desc->data_ref_q_id[prm_id]);

            if(0 != ownObjDescIsValidType((tivx_obj_desc_t*)data_ref_q_obj_desc, TIVX_OBJ_DESC_DATA_REF_Q))
            {
                is_prm_input = tivxFlagIsBitSet(is_prm_input_flag, ((uint32_t)1U<<prm_id));

                is_prm_released = (vx_bool)vx_false_e;

                ownTargetNodeDescReleaseParameter(
                    node_obj_desc,
                    data_ref_q_obj_desc, /* data ref q obj desc */
                    prm_obj_desc_id[prm_id], /* parameter ref to release */
                    is_prm_input,
                    &is_prm_released
                    );

                if(0 != is_prm_released)
                {
                    ownTargetObjDescSendRefConsumed(data_ref_q_obj_desc);
                }
            }
        }
    }
}
