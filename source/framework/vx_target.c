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

static tivx_target_t g_target_table[TIVX_TARGET_MAX_TARGETS_IN_CPU];

static tivx_target ownTargetAllocHandle(vx_enum target_id);
static void ownTargetFreeHandle(tivx_target *target_handle);
static vx_status ownTargetDequeueObjDesc(tivx_target target, uint16_t *obj_desc_id, uint32_t timeout);
static tivx_target ownTargetGetHandle(vx_enum target_id);
static void ownTargetNodeDescSendComplete(const tivx_obj_desc_node_t *node_obj_desc);
static vx_bool ownTargetNodeDescCanNodeExecute(const tivx_obj_desc_node_t *node_obj_desc);
static void ownTargetNodeDescTriggerNextNodes(const tivx_obj_desc_node_t *node_obj_desc);
static void ownTargetNodeDescNodeExecuteTargetKernel(tivx_obj_desc_node_t *node_obj_desc, uint16_t prm_obj_desc_id[]);
static void ownTargetNodeDescNodeExecuteUserKernel(tivx_obj_desc_node_t *node_obj_desc, uint16_t prm_obj_desc_id[]);
static void ownTargetNodeDescNodeExecute(tivx_target target, tivx_obj_desc_node_t *node_obj_desc);
static vx_status ownTargetNodeDescNodeCreate(tivx_obj_desc_node_t *node_obj_desc);
static vx_status ownTargetNodeDescNodeDelete(const tivx_obj_desc_node_t *node_obj_desc);
static vx_status ownTargetNodeDescNodeControl(
    const tivx_obj_desc_cmd_t *cmd_obj_desc,
    const tivx_obj_desc_node_t *node_obj_desc);
static void ownTargetCmdDescHandleAck(tivx_obj_desc_cmd_t *cmd_obj_desc);
static vx_action ownTargetCmdDescHandleUserCallback(tivx_obj_desc_node_t *node_obj_desc, uint64_t timestamp);
static void ownTargetSetGraphStateAbandon(
    const tivx_obj_desc_node_t *node_obj_desc);
static void ownTargetCmdDescSendAck(tivx_obj_desc_cmd_t *cmd_obj_desc, vx_status status);
static void ownTargetCmdDescHandler(tivx_obj_desc_cmd_t *cmd_obj_desc);
static void VX_CALLBACK ownTargetTaskMain(void *app_var);
static vx_bool ownTargetNodeDescIsPrevPipeNodeBlocked(tivx_obj_desc_node_t *node_obj_desc);
static void ownTargetNodeDescNodeMarkComplete(tivx_obj_desc_node_t *node_obj_desc, uint16_t *blocked_node_id);
static vx_status ownTargetNodeSendCommand(const tivx_obj_desc_cmd_t *cmd_obj_desc,
    uint32_t node_id, const tivx_obj_desc_node_t *node_obj_desc);

static tivx_target ownTargetAllocHandle(vx_enum target_id)
{
    uint16_t target_inst = TIVX_GET_TARGET_INST(target_id);
    tivx_target tmp_target = NULL, target = NULL;

    if(target_inst < TIVX_TARGET_MAX_TARGETS_IN_CPU)
    {
        tmp_target = &g_target_table[target_inst];

        if(tmp_target->target_id == target_id)
        {
            /* target id already allocated so return null */

            target = NULL;
        }
        else
        {
            /* target ID is not allocated, allocate it */
            tmp_target->target_id = target_id;

            target = tmp_target;
            ownLogResourceAlloc("TIVX_TARGET_MAX_TARGETS_IN_CPU", 1);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Exceeded max targets in CPU. Modify TIVX_TARGET_MAX_TARGETS_IN_CPU value in tiovx/include/TI/tivx_config.h\n");
        VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_TARGET_MAX_TARGETS_IN_CPU in tiovx/include/TI/tivx_config.h\n");
    }

    return target;
}

static void ownTargetFreeHandle(tivx_target *target_handle)
{
    if((NULL != target_handle) && (*target_handle!=NULL))
    {
        /* mark target handle as free */
        (*target_handle)->target_id = (vx_enum)TIVX_TARGET_ID_INVALID;

        *target_handle = NULL;
    }
}

static vx_status ownTargetDequeueObjDesc(tivx_target target, uint16_t *obj_desc_id, uint32_t timeout)
{
    vx_status status;
    uintptr_t value = (vx_enum)TIVX_OBJ_DESC_INVALID;

    *obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

    status = tivxQueueGet(&target->job_queue_handle,
                &value, timeout);

    if(status == (vx_status)VX_SUCCESS)
    {
        *obj_desc_id = (uint16_t)value;
    }

    return status;
}

static tivx_target ownTargetGetHandle(vx_enum target_id)
{
    uint16_t target_inst = TIVX_GET_TARGET_INST(target_id);
    tivx_target tmp_target = NULL, target = NULL;

    if(target_inst < TIVX_TARGET_MAX_TARGETS_IN_CPU)
    {
        tmp_target = &g_target_table[target_inst];

        if(tmp_target->target_id == target_id)
        {
            /* target id matches so return it */

            target = tmp_target;
        }
        else
        {
            /* target ID does not match so return NULL */
            target = NULL;
        }
    }

    return target;
}

static void ownTargetNodeDescSendComplete(
    const tivx_obj_desc_node_t *node_obj_desc)
{
    uint16_t cmd_obj_desc_id;

    if( (tivxFlagIsBitSet(node_obj_desc->flags, TIVX_NODE_FLAG_IS_USER_CALLBACK) != (vx_bool)vx_false_e)
            ||
        (node_obj_desc->num_out_nodes == 0U)
        )
    {
        cmd_obj_desc_id = (uint16_t)node_obj_desc->node_complete_cmd_obj_desc_id;

        if( (vx_enum)cmd_obj_desc_id != (vx_enum)TIVX_OBJ_DESC_INVALID)
        {
            tivx_obj_desc_cmd_t *cmd_obj_desc = (tivx_obj_desc_cmd_t *)ownObjDescGet(cmd_obj_desc_id);

            if( ownObjDescIsValidType( (tivx_obj_desc_t*)cmd_obj_desc, TIVX_OBJ_DESC_CMD) != 0)
            {
                uint64_t timestamp = tivxPlatformGetTimeInUsecs()*1000U;

                tivx_uint64_to_uint32(
                    timestamp,
                    &cmd_obj_desc->timestamp_h,
                    &cmd_obj_desc->timestamp_l
                );

                /* users wants a notification of node complete or this is leaf node
                 * so send node complete command to host
                 */
                ownObjDescSend( cmd_obj_desc->dst_target_id, cmd_obj_desc_id);
            }
        }
    }
}

static vx_bool ownTargetNodeDescCanNodeExecute(
    const tivx_obj_desc_node_t *node_obj_desc)
{
    tivx_obj_desc_node_t *prev_node_obj_desc;
    uint16_t prev_node_obj_desc_id;
    uint16_t i;
    vx_bool can_execute = (vx_bool)vx_true_e;

    for(i=0; i<node_obj_desc->num_in_nodes; i++)
    {
        prev_node_obj_desc_id = node_obj_desc->in_node_id[i];
        prev_node_obj_desc = (tivx_obj_desc_node_t*)ownObjDescGet(prev_node_obj_desc_id);

        if( ownObjDescIsValidType( (tivx_obj_desc_t*)prev_node_obj_desc, TIVX_OBJ_DESC_NODE) != 0)
        {
            if( tivxFlagIsBitSet(prev_node_obj_desc->flags,
                        TIVX_NODE_FLAG_IS_EXECUTED) == (vx_bool)vx_false_e)
            {
                can_execute = (vx_bool)vx_false_e;
            }
        }
    }

    return can_execute;
}

static void ownTargetNodeDescTriggerNextNodes(
    const tivx_obj_desc_node_t *node_obj_desc)
{
    vx_bool can_execute;
    uint16_t i;
    tivx_obj_desc_node_t *next_node_obj_desc;
    uint16_t next_node_obj_desc_id;

    /* check and trigger next set of nodes */
    for(i=0; i<node_obj_desc->num_out_nodes; i++)
    {
        next_node_obj_desc_id = node_obj_desc->out_node_id[i];
        next_node_obj_desc = (tivx_obj_desc_node_t*)ownObjDescGet(next_node_obj_desc_id);

        if( ownObjDescIsValidType( (tivx_obj_desc_t*)next_node_obj_desc, TIVX_OBJ_DESC_NODE) != 0)
        {
            can_execute = ownTargetNodeDescCanNodeExecute(next_node_obj_desc);

            if(can_execute == (vx_bool)vx_true_e)
            {
                ownObjDescSend( next_node_obj_desc->target_id, next_node_obj_desc_id);
            }
        }
    }
}

static void ownTargetNodeDescNodeExecuteTargetKernel(
    tivx_obj_desc_node_t *node_obj_desc, uint16_t prm_obj_desc_id[])
{
    tivx_target_kernel_instance target_kernel_instance;
    tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS] = {NULL};
    uint32_t i, cnt, loop_max = 1;
    uint32_t is_prm_replicated = node_obj_desc->is_prm_replicated;
    uint32_t is_prm_array_element = node_obj_desc->is_prm_array_element;
    uint32_t is_prm_data_ref_q_flag = node_obj_desc->is_prm_data_ref_q;
    tivx_obj_desc_t *parent_obj_desc[TIVX_KERNEL_MAX_PARAMS] = {NULL};
    tivx_obj_desc_t *prm_obj_desc;

    if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) ==
        (vx_bool)vx_true_e)
    {
        loop_max = node_obj_desc->num_of_replicas;
    }

    for (cnt = 0; cnt < loop_max; cnt ++)
    {
        target_kernel_instance = ownTargetKernelInstanceGet(
                (uint16_t)node_obj_desc->target_kernel_index[cnt], (vx_enum)node_obj_desc->kernel_id);

        for(i=0; i<node_obj_desc->num_params ; i++)
        {
            parent_obj_desc[i] = NULL;

            if((is_prm_replicated & ((uint32_t)1U << i)) != 0U)
            {
                prm_obj_desc = ownObjDescGet(prm_obj_desc_id[i]);
                if(prm_obj_desc != NULL)
                {
                    parent_obj_desc[i] = ownObjDescGet(
                        prm_obj_desc->scope_obj_desc_id);

                    /* if parent is NULL, then prm_obj_desc itself is the parent */
                    if(parent_obj_desc[i]==NULL)
                    {
                        parent_obj_desc[i] = prm_obj_desc;
                    }
                }
            }
        }

        for(i=0; i<node_obj_desc->num_params ; i++)
        {
            params[i] = NULL;
            if((is_prm_replicated & ((uint32_t)1U << i)) != 0U)
            {
                if(parent_obj_desc[i] != NULL)
                {
                    if((vx_enum)parent_obj_desc[i]->type==(vx_enum)TIVX_OBJ_DESC_OBJARRAY)
                    {
                        params[i] = ownObjDescGet(
                            ((tivx_obj_desc_object_array_t*)parent_obj_desc[i])->
                                obj_desc_id[cnt]);
                    }
                    else
                    if((vx_enum)parent_obj_desc[i]->type==(vx_enum)TIVX_OBJ_DESC_PYRAMID)
                    {
                        params[i] = ownObjDescGet(
                            ((tivx_obj_desc_pyramid_t*)parent_obj_desc[i])->
                                obj_desc_id[cnt]);
                    }
                    else
                    {
                        params[i] = NULL;
                    }
                }
            }
            else if((is_prm_array_element & ((uint32_t)1U << i)) != 0U)
            {
                if((is_prm_data_ref_q_flag & ((uint32_t)1U << i)) != 0U)
                {
                    /* this is a case of parameter expected by node being a
                     * element within a object array or pyramid
                     *
                     * Here we index into the object array and pass the element
                     * later return parent back to the framework
                     */

                    /* if this parameter is pipelined then it is assumed
                     * that this points to 0th element of object array or pyramid, always
                     */

                    parent_obj_desc[i] = ownObjDescGet(prm_obj_desc_id[i]);
                    if(parent_obj_desc[i] != NULL)
                    {
                        if((vx_enum)parent_obj_desc[i]->type==(vx_enum)TIVX_OBJ_DESC_OBJARRAY)
                        {
                            tivx_obj_desc_t *tmp_node_param;

                            tmp_node_param = ownObjDescGet(node_obj_desc->data_id[i]);

                            if (NULL != tmp_node_param)
                            {
                                params[i] = ownObjDescGet(
                                    ((tivx_obj_desc_object_array_t*)parent_obj_desc[i])->
                                        obj_desc_id[tmp_node_param->element_idx]);
                            }
                        }
                        else
                        if((vx_enum)parent_obj_desc[i]->type==(vx_enum)TIVX_OBJ_DESC_PYRAMID)
                        {
                            tivx_obj_desc_t *tmp_node_param;

                            tmp_node_param = ownObjDescGet(node_obj_desc->data_id[i]);

                            if (NULL != tmp_node_param)
                            {
                                params[i] = ownObjDescGet(
                                    ((tivx_obj_desc_pyramid_t*)parent_obj_desc[i])->
                                        obj_desc_id[tmp_node_param->element_idx]);
                            }
                        }
                        else
                        {
                            /* this is not part of a array element,
                             * pass thru params[] = prm_obj_desc_id[]
                             * set parent_obj_desc as NULL
                             * reset bit in is_prm_array_element
                             * */
                            params[i] = ownObjDescGet(prm_obj_desc_id[i]);
                            parent_obj_desc[i] = NULL;
                            tivxFlagBitClear(&is_prm_array_element, (uint32_t)1<<i);
                        }
                    }
                }
                else
                {
                    params[i] = ownObjDescGet(node_obj_desc->data_id[i]);
                }
            }
            else
            {
                params[i] = ownObjDescGet(prm_obj_desc_id[i]);

                /* Note: this check is needed to solve the condition where a replicated object array/pyramid
                 *       is connected to a node that consumes the full object array/pyramid.  The acquire
                 *       parameter function returns the first obj desc id of that array rather than
                 *       the obj desc id of the array/pyramid.  This logic checks if the obj desc id has a
                 *       parent object.  In the case that it does, it checks the type of the parent against
                 *       the type of the node object.  If these match, then the parent object should be returned.
                 *       If they don't, then the element of the parent should be returned.
                 */
                if((is_prm_data_ref_q_flag & ((uint32_t)1U << i)) != 0U)
                {
                    parent_obj_desc[i] = ownObjDescGet(params[i]->scope_obj_desc_id);

                    if(NULL != parent_obj_desc[i])
                    {
                        tivx_obj_desc_t *tmp_node_param;

                        tmp_node_param = ownObjDescGet(node_obj_desc->data_id[i]);

                        if (NULL != tmp_node_param)
                        {
                            if (parent_obj_desc[i]->type == tmp_node_param->type)
                            {
                                params[i] = parent_obj_desc[i];
                            }
                        }
                    }
                }
            }

            #if 0
            if(params[i])
            {
                VX_PRINT(VX_ZONE_INFO," Param %d is %d of type %d (node=%d, pipe=%d)\n",
                        i,
                        params[i]->obj_desc_id,
                        params[i]->type,
                        node_obj_desc->base.obj_desc_id,
                        node_obj_desc->pipeline_id
                );
            }
            else
            {
                VX_PRINT(VX_ZONE_INFO," Param %d is NULL (node=%d, pipe=%d)\n",
                        i,
                        node_obj_desc->base.obj_desc_id,
                        node_obj_desc->pipeline_id
                );
            }
            #endif
        }

        {
            if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_SUPERNODE) ==
                (vx_bool)vx_true_e)
            {
                params[0] = (tivx_obj_desc_t *) ownObjDescGet( node_obj_desc->base.scope_obj_desc_id );

                node_obj_desc->exe_status |= (uint32_t)ownTargetKernelExecute(target_kernel_instance, params,
                    1);
            }
            else
            {
                ownTargetSetTimestamp(node_obj_desc, params);
                node_obj_desc->exe_status |= (uint32_t)ownTargetKernelExecute(target_kernel_instance, params,
                    (uint16_t)node_obj_desc->num_params);
            }
        }
    }

    /* params[] contain pointer to obj_desc for each parameter,
     * A node could change the obj_desc value of params[i] as part of its execution
     * below logic changes prm_obj_desc_id, based on updated params[i] value.
     * This logic will not take effect for replicated parameters and for non data ref queue parameters
     * i.e it will take effect only for data ref queue parameters and non-replicated parameters
     */
    for(i=0; i<node_obj_desc->num_params ; i++)
    {
        if(    (tivxFlagIsBitSet(is_prm_data_ref_q_flag, ((uint32_t)1<<i)) == (vx_bool)vx_true_e)
            && (tivxFlagIsBitSet(is_prm_replicated, ((uint32_t)1<<i)) == (vx_bool)vx_false_e)
            )
        {
            if(params[i]==NULL)
            {
                prm_obj_desc_id[i] = (vx_enum)TIVX_OBJ_DESC_INVALID;
            }
            else if(tivxFlagIsBitSet(is_prm_array_element, ((uint32_t)1<<i)) == (vx_bool)vx_true_e)
            {
                prm_obj_desc_id[i] = (vx_enum)TIVX_OBJ_DESC_INVALID;

                prm_obj_desc = ownObjDescGet(params[i]->obj_desc_id);

                if (prm_obj_desc != NULL)
                {
                    parent_obj_desc[i] = ownObjDescGet(
                        prm_obj_desc->scope_obj_desc_id);

                    if(parent_obj_desc[i]!=NULL)
                    {
                        prm_obj_desc_id[i] = parent_obj_desc[i]->obj_desc_id;
                    }
                }
            }
            else
            {
                prm_obj_desc_id[i] = params[i]->obj_desc_id;
            }
        }
    }

}

static void ownTargetNodeDescNodeExecuteUserKernel(tivx_obj_desc_node_t *node_obj_desc, uint16_t prm_obj_desc_id[])
{
    vx_reference prm_ref[TIVX_KERNEL_MAX_PARAMS];
    uint32_t i;

    for(i=0; i<node_obj_desc->num_params; i++)
    {
        prm_ref[i] = ownReferenceGetHandleFromObjDescId(prm_obj_desc_id[i]);
    }
    node_obj_desc->exe_status = (uint32_t)ownNodeUserKernelExecute((vx_node)(uintptr_t)node_obj_desc->base.host_ref, prm_ref);
}

static vx_bool ownTargetNodeDescIsPrevPipeNodeBlocked(tivx_obj_desc_node_t *node_obj_desc)
{
    tivx_obj_desc_node_t *prev_node_obj_desc;
    vx_bool is_prev_node_blocked = (vx_bool)vx_false_e;

    prev_node_obj_desc = (tivx_obj_desc_node_t*)ownObjDescGet(node_obj_desc->prev_pipe_node_id);
    if(prev_node_obj_desc!=NULL)
    {
        if(node_obj_desc->state == TIVX_NODE_OBJ_DESC_STATE_IDLE)
        {
            if(prev_node_obj_desc->state != TIVX_NODE_OBJ_DESC_STATE_IDLE)
            {
                /* previous node in pipeline is blocked, so block this pipeline node until previous pipeline
                 * completes
                 */
                node_obj_desc->state = TIVX_NODE_OBJ_DESC_STATE_BLOCKED;
                prev_node_obj_desc->blocked_node_id = node_obj_desc->base.obj_desc_id;
                is_prev_node_blocked = (vx_bool)vx_true_e;
            }
        }
        else
        if(node_obj_desc->state == TIVX_NODE_OBJ_DESC_STATE_BLOCKED)
        {
            /* this is trigger from prev node or due to resource being released so proceed with execution */
            node_obj_desc->state = TIVX_NODE_OBJ_DESC_STATE_IDLE;
        }
        else
        {
            /* do nothing */
        }
    }
    return is_prev_node_blocked;
}


static void ownTargetNodeDescNodeMarkComplete(tivx_obj_desc_node_t *node_obj_desc, uint16_t *blocked_node_id)
{
    /* check if any node is blocked on this node to get unblocked and complete execution
     * This will be a node from next pipeline instance
     */
    *blocked_node_id = node_obj_desc->blocked_node_id;
    node_obj_desc->blocked_node_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
    node_obj_desc->state = TIVX_NODE_OBJ_DESC_STATE_IDLE;
    tivxFlagBitSet(&node_obj_desc->flags, TIVX_NODE_FLAG_IS_EXECUTED);
}

static void ownTargetNodeDescNodeExecute(tivx_target target, tivx_obj_desc_node_t *node_obj_desc)
{
    uint64_t beg_time, end_time;
    uint16_t blocked_node_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
    uint16_t prm_obj_desc_id[TIVX_KERNEL_MAX_PARAMS];

    /* if node is already executed do nothing */
    if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_EXECUTED) == (vx_bool)vx_false_e )
    {
        /* check if same node in previous pipeline instance is blocked, if yes then
         * dont acquire parameters for this node
         */
        if(ownTargetNodeDescIsPrevPipeNodeBlocked(node_obj_desc)==(vx_bool)vx_false_e)
        {
            vx_bool is_node_blocked;
            tivx_target_kernel_instance target_kernel_instance;
            vx_enum kernel_instance_state = (vx_enum)VX_NODE_STATE_STEADY;
            uint32_t num_bufs = 1;

            is_node_blocked = (vx_bool)vx_false_e;

            VX_PRINT(VX_ZONE_INFO,"Node (node=%d, pipe=%d) acquiring parameters on target %08x\n",
                                 node_obj_desc->base.obj_desc_id,
                                 node_obj_desc->pipeline_id,
                                 target->target_id
                           );

            /* Note: not taking into account replicated node */
            target_kernel_instance = ownTargetKernelInstanceGet(
                (uint16_t)node_obj_desc->target_kernel_index[0], (vx_enum)node_obj_desc->kernel_id);

            /* Note: in the case of user kernel, target_kernel instance is NULL */
            if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_TARGET_KERNEL) != 0)
            {
                if (NULL != target_kernel_instance)
                {
                    num_bufs = target_kernel_instance->kernel->num_pipeup_bufs;

                    kernel_instance_state = target_kernel_instance->state;
                }
            }
            else
            {
                kernel_instance_state = (vx_enum)node_obj_desc->source_state;

                num_bufs = node_obj_desc->num_pipeup_bufs;
            }

            if ( ((vx_enum)VX_NODE_STATE_PIPEUP == kernel_instance_state) &&
                 (num_bufs > 1U) )
            {
                int32_t buf_idx;

                for (buf_idx = 0; buf_idx < ((int32_t)num_bufs - 1); buf_idx++)
                {
                    ownTargetNodeDescAcquireAllParametersForPipeup(node_obj_desc, prm_obj_desc_id);

                    if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_TARGET_KERNEL) != 0)
                    {
                        ownTargetNodeDescNodeExecuteTargetKernel(node_obj_desc, prm_obj_desc_id);
                    }
                    else
                    {
                        ownTargetNodeDescNodeExecuteUserKernel(node_obj_desc, prm_obj_desc_id);
                    }
                }

                node_obj_desc->source_state = (vx_enum)VX_NODE_STATE_STEADY;

                if( (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_TARGET_KERNEL) != (vx_bool)vx_false_e) &&
                    (NULL != target_kernel_instance))
                {
                    target_kernel_instance->state = (vx_enum)VX_NODE_STATE_STEADY;
                }
            }

            ownTargetNodeDescAcquireAllParameters(node_obj_desc, prm_obj_desc_id, &is_node_blocked);

            if(is_node_blocked==(vx_bool)vx_false_e)
            {
                VX_PRINT(VX_ZONE_INFO,"Node (node=%d, pipe=%d) executing on target %08x\n",
                                 node_obj_desc->base.obj_desc_id,
                                 node_obj_desc->pipeline_id,
                                 target->target_id
                           );

                beg_time = tivxPlatformGetTimeInUsecs();

                ownLogRtTraceNodeExeStart(beg_time, node_obj_desc);

                if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_TARGET_KERNEL) != 0)
                {
                    ownTargetNodeDescNodeExecuteTargetKernel(node_obj_desc, prm_obj_desc_id);
                }
                else
                {
                    ownTargetNodeDescNodeExecuteUserKernel(node_obj_desc, prm_obj_desc_id);
                }

                end_time = tivxPlatformGetTimeInUsecs();

                ownLogRtTraceNodeExeEnd(end_time, node_obj_desc);

                VX_PRINT(VX_ZONE_INFO,"Node (node=%d, pipe=%d) executing on target %08x ... DONE !!!\n",
                                 node_obj_desc->base.obj_desc_id,
                                 node_obj_desc->pipeline_id,
                                 target->target_id
                           );

                tivx_uint64_to_uint32(
                    beg_time,
                    &node_obj_desc->exe_time_beg_h,
                    &node_obj_desc->exe_time_beg_l
                );

                tivx_uint64_to_uint32(
                    end_time,
                    &node_obj_desc->exe_time_end_h,
                    &node_obj_desc->exe_time_end_l
                );

                ownTargetNodeDescNodeMarkComplete(node_obj_desc, &blocked_node_id);
                ownTargetNodeDescReleaseAllParameters(node_obj_desc, prm_obj_desc_id);
                ownTargetNodeDescSendComplete(node_obj_desc);
                ownTargetNodeDescTriggerNextNodes(node_obj_desc);

                if((vx_enum)blocked_node_id!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                {
                    /* this will be same node in next pipeline to trigger it last */
                    VX_PRINT(VX_ZONE_INFO,"Re-triggering (node=%d)\n",
                             blocked_node_id
                    );
                    ownTargetTriggerNode(blocked_node_id);
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_INFO,"Node (node=%d, pipe=%d) ... BLOCKED for resources on target %08x\n",
                                 node_obj_desc->base.obj_desc_id,
                                 node_obj_desc->pipeline_id,
                                 target->target_id
                           );
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO,"Node (node=%d, pipe=%d) ... BLOCKED for previous pipe instance node (node=%d) to complete !!!\n",
                    node_obj_desc->base.obj_desc_id,
                    node_obj_desc->pipeline_id,
                    node_obj_desc->prev_pipe_node_id
            );
        }
    }
}

static vx_status ownTargetNodeDescNodeCreate(tivx_obj_desc_node_t *node_obj_desc)
{
    tivx_target_kernel_instance target_kernel_instance;
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t i, cnt, loop_max = 1;
    tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS] = {NULL};
    uint32_t is_prm_replicated = node_obj_desc->is_prm_replicated;
    tivx_obj_desc_t *parent_obj_desc[TIVX_KERNEL_MAX_PARAMS] = {NULL};
    tivx_obj_desc_t *prm_obj_desc;
    tivx_obj_desc_kernel_name_t *kernel_name_obj_desc;
    volatile char *kernel_name = NULL;

    if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) ==
        (vx_bool)vx_true_e)
    {
        loop_max = (uint16_t)node_obj_desc->num_of_replicas;
    }

    kernel_name_obj_desc = (tivx_obj_desc_kernel_name_t*)ownObjDescGet((uint16_t)node_obj_desc->kernel_name_obj_desc_id);
    if(kernel_name_obj_desc!=NULL)
    {
        kernel_name = kernel_name_obj_desc->kernel_name;
    }
    for (cnt = 0; (cnt < loop_max) && (status == (vx_status)VX_SUCCESS); cnt ++)
    {
        target_kernel_instance = ownTargetKernelInstanceAlloc(
            (vx_enum)node_obj_desc->kernel_id, kernel_name, (vx_enum)node_obj_desc->target_id);

        if(target_kernel_instance == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "target_kernel_instance is NULL\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
        else
        {
            /* This target_kernel_instance is newly allocated in this create function.  The "kernel"
             * in this case is a target_kernel which has not had num_pipeup_bufs set either
             * and thus needs to be set from the node_obj_desc which received this value from
             * the host side vx_kernel. */
            target_kernel_instance->kernel->num_pipeup_bufs = node_obj_desc->num_pipeup_bufs;

            if (target_kernel_instance->kernel->num_pipeup_bufs > 1U)
            {
                target_kernel_instance->state = (vx_enum)VX_NODE_STATE_PIPEUP;
            }
            else
            {
                target_kernel_instance->state = (vx_enum)VX_NODE_STATE_STEADY;
            }

            /* setting the tile size for each node */
            target_kernel_instance->block_width = node_obj_desc->block_width;
            target_kernel_instance->block_height = node_obj_desc->block_height;

            if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) ==
                (vx_bool)vx_true_e)
            {
                target_kernel_instance->is_kernel_instance_replicated = (vx_bool)vx_true_e;
            }

            /* save index key for fast retrival of handle during run-time */
            node_obj_desc->target_kernel_index[cnt] =
                ownTargetKernelInstanceGetIndex(target_kernel_instance);

            for(i=0; i<node_obj_desc->num_params ; i++)
            {
                parent_obj_desc[i] = NULL;

                if((is_prm_replicated & ((uint32_t)1U << i)) != 0U)
                {
                    prm_obj_desc = ownObjDescGet(node_obj_desc->data_id[i]);
                    if(prm_obj_desc != NULL)
                    {
                        parent_obj_desc[i] = ownObjDescGet(
                            prm_obj_desc->scope_obj_desc_id);
                    }
                }
            }

            for(i=0; i<node_obj_desc->num_params ; i++)
            {
                params[i] = NULL;
                if((is_prm_replicated & ((uint32_t)1U << i)) != 0U)
                {
                    if(parent_obj_desc[i] != NULL)
                    {
                        if((vx_enum)parent_obj_desc[i]->type==(vx_enum)TIVX_OBJ_DESC_OBJARRAY)
                        {
                            params[i] = ownObjDescGet(
                                ((tivx_obj_desc_object_array_t*)parent_obj_desc[i])->
                                    obj_desc_id[cnt]);
                        }
                        else
                        if((vx_enum)parent_obj_desc[i]->type==(vx_enum)TIVX_OBJ_DESC_PYRAMID)
                        {
                            params[i] = ownObjDescGet(
                                ((tivx_obj_desc_pyramid_t*)parent_obj_desc[i])->
                                    obj_desc_id[cnt]);
                        }
                        else
                        {
                            params[i] = NULL;
                        }
                    }
                }
                else
                {
                    params[i] = ownObjDescGet(node_obj_desc->data_id[i]);
                }
            }

            /* Linking the reference to the node object descriptor */
            target_kernel_instance->node_obj_desc = node_obj_desc;

            /* copy border mode also in the target_kernel_instance */
            tivx_obj_desc_memcpy(&target_kernel_instance->border_mode, &node_obj_desc->border_mode, (uint32_t)sizeof(vx_border_t));

            if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_SUPERNODE) ==
                (vx_bool)vx_true_e)
            {
                params[0] = (tivx_obj_desc_t *) ownObjDescGet( node_obj_desc->base.scope_obj_desc_id );

                status = ownTargetKernelCreate(target_kernel_instance,
                    params, 1);
            }
            else
            {
                status = ownTargetKernelCreate(target_kernel_instance,
                    params, (uint16_t)node_obj_desc->num_params);
            }

            if(status!=(vx_status)VX_SUCCESS)
            {
                ownTargetKernelInstanceFree(&target_kernel_instance);
            }
        }
    }

    if ((vx_status)VX_SUCCESS != status)
    {
        for (i = 0; i < cnt; i ++)
        {
            target_kernel_instance = ownTargetKernelInstanceGet(
                (uint16_t)node_obj_desc->target_kernel_index[i], (vx_enum)node_obj_desc->kernel_id);

            if (NULL != target_kernel_instance)
            {
                ownTargetKernelInstanceFree(&target_kernel_instance);
            }
        }
    }

    return status;
}

static vx_status ownTargetNodeDescNodeDelete(const tivx_obj_desc_node_t *node_obj_desc)
{
    tivx_target_kernel_instance target_kernel_instance;
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t i, cnt, loop_max = 1;
    tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS];

    if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) ==
        (vx_bool)vx_true_e)
    {
        loop_max = (uint16_t)node_obj_desc->num_of_replicas;
    }

    for (cnt = 0; cnt < loop_max; cnt ++)
    {
        target_kernel_instance = ownTargetKernelInstanceGet(
            (uint16_t)node_obj_desc->target_kernel_index[cnt], (vx_enum)node_obj_desc->kernel_id);

        if(target_kernel_instance == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "target_kernel_instance is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        else
        {
            {
                /* NOTE: nothing special for replicated node during
                         create/delete */
                for(i=0; i<node_obj_desc->num_params ; i++)
                {
                    params[i] = ownObjDescGet(node_obj_desc->data_id[i]);
                }

                if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_SUPERNODE) ==
                    (vx_bool)vx_true_e)
                {
                    params[0] = (tivx_obj_desc_t *) ownObjDescGet( node_obj_desc->base.scope_obj_desc_id );

                    tivxCheckStatus(&status, ownTargetKernelDelete(target_kernel_instance, params, 1));
                }
                else
                {
                    tivxCheckStatus(&status, ownTargetKernelDelete(target_kernel_instance,
                        params, (uint16_t)node_obj_desc->num_params));
                }
            }

            ownTargetKernelInstanceFree(&target_kernel_instance);
        }
    }

    return status;
}

static vx_status ownTargetNodeSendCommand(const tivx_obj_desc_cmd_t *cmd_obj_desc,
    uint32_t node_id, const tivx_obj_desc_node_t *node_obj_desc)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int16_t i;
    tivx_target_kernel_instance target_kernel_instance;
    tivx_obj_desc_t *params[TIVX_CMD_MAX_OBJ_DESCS];

    target_kernel_instance = ownTargetKernelInstanceGet(
        (uint16_t)node_obj_desc->target_kernel_index[node_id], (vx_enum)node_obj_desc->kernel_id);

    if(target_kernel_instance == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "target_kernel_instance is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        for(i=0; i<(int32_t)cmd_obj_desc->num_cmd_params; i++)
        {
            params[i] = ownObjDescGet(cmd_obj_desc->cmd_params_desc_id[i]);
        }

        status = ownTargetKernelControl(target_kernel_instance,
            cmd_obj_desc->node_cmd_id, params,
            (uint16_t)cmd_obj_desc->num_cmd_params);
    }

    return (status);
}

static vx_status ownTargetNodeDescNodeControl(
    const tivx_obj_desc_cmd_t *cmd_obj_desc,
    const tivx_obj_desc_node_t *node_obj_desc)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t cnt, loop_max = 1;

    if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) ==
        (vx_bool)vx_true_e)
    {
        loop_max = (uint16_t)node_obj_desc->num_of_replicas;

        if ((vx_enum)TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES ==
            cmd_obj_desc->replicated_node_idx)
        {
            for (cnt = 0; cnt < loop_max; cnt ++)
            {
                status = ownTargetNodeSendCommand(cmd_obj_desc, cnt,
                    node_obj_desc);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "SendCommand Failed\n");
                    break;
                }
            }
        }
        else
        {
            /* Replicated node idx must be less than total replicated nodes. */
            if (cmd_obj_desc->replicated_node_idx < (int32_t)loop_max)
            {
                status = ownTargetNodeSendCommand(cmd_obj_desc,
                    (uint32_t)cmd_obj_desc->replicated_node_idx,
                    node_obj_desc);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "SendCommand Failed\n");
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Incorrect node id\n");
                status = (vx_status)VX_FAILURE;
            }
        }
    }
    else
    {
        /* For non-replicated node, ignore node-id field */
        status = ownTargetNodeSendCommand(cmd_obj_desc, 0U, node_obj_desc);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "SendCommand Failed\n");
        }
    }

    return status;
}

static void ownTargetCmdDescHandleAck(tivx_obj_desc_cmd_t *cmd_obj_desc)
{
    if( tivxFlagIsBitSet( cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK) != 0)
    {
        tivxFlagBitClear( &cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK);

        tivxEventPost((tivx_event)(uintptr_t)cmd_obj_desc->ack_event_handle);
    }
}

static vx_action ownTargetCmdDescHandleUserCallback(tivx_obj_desc_node_t *node_obj_desc, uint64_t timestamp)
{
    vx_action action;
    vx_node node = (vx_node)(uintptr_t)node_obj_desc->base.host_ref;
    vx_bool is_send_graph_complete_event = (vx_bool)vx_false_e;

    /* return action is ignored */
    action = ownNodeExecuteUserCallback(node);

    /* if this is leaf node, check if graph is completed */
    if(ownNodeGetNumOutNodes(node)==0U)
    {
        /* check if graph represetned by this node at this pipeline_id
         * is completed and do graph
         * completion handling
         */
        is_send_graph_complete_event = ownCheckGraphCompleted(node->graph, node_obj_desc->pipeline_id);

    }

    /* first do all booking keeping and then send event q events */
    /* send completion event if enabled */
    ownNodeCheckAndSendCompletionEvent(node_obj_desc, timestamp);

    /* if an error occurred within the node, then send an error completion event */
    if ((vx_status)VX_SUCCESS != (vx_status)node_obj_desc->exe_status)
    {
        ownNodeCheckAndSendErrorEvent(node_obj_desc, timestamp, (vx_status)node_obj_desc->exe_status);
    }

    /* Clearing node status now that it has been sent as an error event */
    node_obj_desc->exe_status = 0;

    /* first we let any node events to go thru before sending graph events */
    if(is_send_graph_complete_event!= 0)
    {
        ownSendGraphCompletedEvent(node->graph);
    }

    return (action);
}

static void ownTargetSetGraphStateAbandon(
    const tivx_obj_desc_node_t *node_obj_desc)
{
    vx_node node = (vx_node)(uintptr_t)node_obj_desc->base.host_ref;

    ownSetGraphState(node->graph, node_obj_desc->pipeline_id, (vx_enum)VX_GRAPH_STATE_ABANDONED);
}

static void ownTargetCmdDescSendAck(tivx_obj_desc_cmd_t *cmd_obj_desc, vx_status status)
{
    if( tivxFlagIsBitSet( cmd_obj_desc->flags, TIVX_CMD_FLAG_SEND_ACK) != 0)
    {
        tivxFlagBitSet( &cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK);

        cmd_obj_desc->cmd_status = (uint32_t)status;

        ownObjDescSend(cmd_obj_desc->src_target_id, cmd_obj_desc->base.obj_desc_id);
    }
}

static void ownTargetCmdDescHandler(tivx_obj_desc_cmd_t *cmd_obj_desc)
{
    uint16_t node_obj_desc_id;
    tivx_obj_desc_node_t *node_obj_desc;
    vx_action action;
    vx_status status = (vx_status)VX_SUCCESS;

    switch(cmd_obj_desc->cmd_id)
    {
        case (vx_enum)TIVX_CMD_NODE_CREATE:
        case (vx_enum)TIVX_CMD_NODE_DELETE:
        case (vx_enum)TIVX_CMD_NODE_CONTROL:
            if( tivxFlagIsBitSet( cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK) == (vx_bool)vx_false_e )
            {
                node_obj_desc_id = cmd_obj_desc->obj_desc_id[0];
                node_obj_desc = (tivx_obj_desc_node_t*)ownObjDescGet(node_obj_desc_id);

                if( ownObjDescIsValidType( (tivx_obj_desc_t*)node_obj_desc, TIVX_OBJ_DESC_NODE) != 0)
                {
                    if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_TARGET_KERNEL) != 0)
                    {
                        if((vx_enum)cmd_obj_desc->cmd_id == (vx_enum)TIVX_CMD_NODE_CREATE)
                        {
                            status = ownTargetNodeDescNodeCreate(node_obj_desc);
                        }
                        else
                        if((vx_enum)cmd_obj_desc->cmd_id == (vx_enum)TIVX_CMD_NODE_DELETE)
                        {
                            status = ownTargetNodeDescNodeDelete(node_obj_desc);
                        }
                        else
                        if((vx_enum)cmd_obj_desc->cmd_id == (vx_enum)TIVX_CMD_NODE_CONTROL)
                        {
                            status = ownTargetNodeDescNodeControl(cmd_obj_desc, node_obj_desc);
                        }
                        else
                        {
                            /* do nothing */
                        }
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "object descriptor type is invalid\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                ownTargetCmdDescSendAck(cmd_obj_desc, status);
            }
            else
            {
                /* this is ACK for a previously sent command */
                ownTargetCmdDescHandleAck(cmd_obj_desc);
            }
            break;

        case (vx_enum)TIVX_CMD_NODE_USER_CALLBACK:
            node_obj_desc_id = cmd_obj_desc->obj_desc_id[0];
            node_obj_desc = (tivx_obj_desc_node_t*)ownObjDescGet(node_obj_desc_id);

            if( ownObjDescIsValidType( (tivx_obj_desc_t*)node_obj_desc, TIVX_OBJ_DESC_NODE) != 0)
            {
                uint64_t timestamp;

                tivx_uint32_to_uint64(&timestamp, cmd_obj_desc->timestamp_h, cmd_obj_desc->timestamp_l);

                action = ownTargetCmdDescHandleUserCallback(node_obj_desc, timestamp);

                if (action == (vx_enum)VX_ACTION_ABANDON)
                {
                    ownTargetSetGraphStateAbandon(node_obj_desc);
                }
            }
            /* No ack for user callback command */
            break;
        case (vx_enum)TIVX_CMD_DATA_REF_CONSUMED:
        {
            tivx_data_ref_queue data_ref_q;

            data_ref_q = (tivx_data_ref_queue)ownReferenceGetHandleFromObjDescId(cmd_obj_desc->obj_desc_id[0]);

            if( data_ref_q != NULL )
            {
                uint64_t timestamp;

                tivx_uint32_to_uint64(&timestamp, cmd_obj_desc->timestamp_h, cmd_obj_desc->timestamp_l);

                ownDataRefQueueSendRefConsumedEvent(data_ref_q, timestamp);
            }
            /* No ack for this command */
        }
            break;
        default:

            break;
    }

}

static void VX_CALLBACK ownTargetTaskMain(void *app_var)
{
    tivx_target target = (tivx_target)app_var;
    tivx_obj_desc_t *obj_desc;
    uint16_t obj_desc_id;
    vx_status status = (vx_status)VX_SUCCESS;

    /* Adding OS-specific task init functions */
    ownPlatformTaskInit();

    while(target->targetExitRequest == (vx_bool)vx_false_e)
    {
        status = ownTargetDequeueObjDesc(target,
                    &obj_desc_id, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

        if(    (status != (vx_status)VX_SUCCESS)
            || ((vx_enum)obj_desc_id == (vx_enum)TIVX_OBJ_DESC_INVALID) )
        {
            /* in case of error, do nothing,
             * if target exit was requested, while(...) condition check with
             * check and exit
             */
        }
        else
        {
            obj_desc = ownObjDescGet(obj_desc_id);
            if(obj_desc == NULL)
            {
                /* in valid obj_desc_id received */
            }
            else
            {
                ownLogRtTraceTargetExeStart(target, obj_desc);

                switch(obj_desc->type)
                {
                    case (vx_enum)TIVX_OBJ_DESC_CMD:
                        if( ownObjDescIsValidType( obj_desc, TIVX_OBJ_DESC_CMD) != 0)
                        {
                            ownTargetCmdDescHandler((tivx_obj_desc_cmd_t*)obj_desc);
                        }
                        break;
                    case (vx_enum)TIVX_OBJ_DESC_NODE:
                        if( ownObjDescIsValidType( obj_desc, TIVX_OBJ_DESC_NODE) != 0)
                        {
                            ownTargetNodeDescNodeExecute(target, (tivx_obj_desc_node_t*)obj_desc);
                        }
                        break;
                    default:
                        /* unsupported obj_desc received at target */
                        break;
                }

                ownLogRtTraceTargetExeEnd(target, obj_desc);
            }
        }
    }

    target->targetExitDone = (vx_bool)vx_true_e;
}

vx_status ownTargetCreate(vx_enum target_id, const tivx_target_create_params_t *params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_target target;

    target = ownTargetAllocHandle(target_id);

    if(target == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "target is NULL\n");
        status = (vx_status)VX_ERROR_NO_RESOURCES;
    }
    else
    {
        target->target_id = target_id;

        tivxTaskSetDefaultCreateParams(&target->task_params);
        target->task_params.stack_ptr = params->task_stack_ptr;
        target->task_params.stack_size = params->task_stack_size;
        target->task_params.core_affinity = params->task_core_affinity;
        target->task_params.priority = params->task_priority;
        target->task_params.task_main = &ownTargetTaskMain;
        target->task_params.app_var = target;
        strncpy(target->task_params.task_name, params->task_name, TIVX_MAX_TASK_NAME);
        target->task_params.task_name[TIVX_MAX_TASK_NAME-1U] = '\0';

        target->targetExitRequest = (vx_bool)vx_false_e;
        target->targetExitDone = (vx_bool)vx_false_e;

        /* create job queue */
        status = tivxQueueCreate(&target->job_queue_handle,
                        TIVX_TARGET_MAX_JOB_QUEUE_DEPTH,
                        target->job_queue_memory,
                        TIVX_QUEUE_FLAG_BLOCK_ON_GET);

        if(status == (vx_status)VX_SUCCESS)
        {
            /* create and start target task */
            status = tivxTaskCreate(&target->task_handle, &target->task_params);
            if(status != (vx_status)VX_SUCCESS)
            {
                tivxQueueDelete(&target->job_queue_handle);
            }
        }

        if (status != (vx_status)VX_SUCCESS)
        {
            ownTargetFreeHandle(&target);
        }
    }
    return status;
}

vx_status ownTargetDelete(vx_enum target_id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_target target;

    target = ownTargetGetHandle(target_id);

    /* delete task */

    if (NULL != target)
    {
        /* set flag to break target from main loop */
        target->targetExitRequest = (vx_bool)vx_true_e;

        /* queue a invalid object descriptor to unblock queue wait */
        ownTargetQueueObjDesc(target_id, (vx_enum)TIVX_OBJ_DESC_INVALID);

        /* wait until target exit is done */
        while(target->targetExitDone==(vx_bool)vx_false_e)
        {
            tivxTaskWaitMsecs(1);
        }
        tivxTaskDelete(&target->task_handle);

        /* delete job queue */
        tivxQueueDelete(&target->job_queue_handle);
    }

    return status;
}

void ownTargetTriggerNode(uint16_t node_obj_desc_id)
{
    tivx_obj_desc_node_t *node_obj_desc;

    node_obj_desc = (tivx_obj_desc_node_t*)ownObjDescGet(node_obj_desc_id);

    if( ownObjDescIsValidType( (tivx_obj_desc_t*)node_obj_desc, TIVX_OBJ_DESC_NODE) != 0)
    {
        ownObjDescSend( node_obj_desc->target_id, node_obj_desc_id);
    }
}

vx_status ownTargetQueueObjDesc(vx_enum target_id, uint16_t obj_desc_id)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    tivx_target target = ownTargetGetHandle(target_id);

    if(target!=NULL)
    {
        status = tivxQueuePut(&target->job_queue_handle,
                (uintptr_t)obj_desc_id, TIVX_EVENT_TIMEOUT_NO_WAIT);

        if((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"***************************************************************************************************\n");
            VX_PRINT(VX_ZONE_ERROR,"FATAL ERROR: tivxQueuePut failed\n");
            VX_PRINT(VX_ZONE_ERROR,"May need to increase the value of TIVX_TARGET_MAX_JOB_QUEUE_DEPTH in tiovx/include/TI/tivx_config.h\n");
            VX_PRINT(VX_ZONE_ERROR,"***************************************************************************************************\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "target is NULL\n");
    }

    return status;
}

void ownTargetSetDefaultCreateParams(tivx_target_create_params_t *params)
{
    params->task_stack_ptr = NULL;
    params->task_stack_size = 0;
    params->task_core_affinity = 0;
    params->task_priority = TIVX_TASK_PRI_LOWEST;
    strncpy(params->task_name, "TIVX_TARGET", TIVX_MAX_TASK_NAME);
    params->task_name[TIVX_MAX_TASK_NAME-1U] = '\0';
}

vx_enum ownTargetGetCpuId(vx_enum target_id)
{
    vx_uint32 returnVal = TIVX_GET_CPU_ID(target_id);
    return ((vx_enum)returnVal);
}

void ownTargetInit(void)
{
    uint16_t i;

    for(i=0; i<dimof(g_target_table); i++)
    {
        g_target_table[i].target_id = (vx_enum)TIVX_TARGET_ID_INVALID;
    }

    ownTargetKernelInit();
    ownTargetKernelInstanceInit();
}

void ownTargetDeInit(void)
{
    ownTargetKernelInstanceDeInit();
    ownTargetKernelDeInit();

}

void ownTargetSetTimestamp(
    const tivx_obj_desc_node_t *node_obj_desc, tivx_obj_desc_t *obj_desc[])
{
    uint16_t prm_id;
    uint64_t timestamp = 0, obj_timestamp = 0;
    uint32_t is_prm_input_flag;
    tivx_obj_desc_t *parent_obj_desc;

    is_prm_input_flag = node_obj_desc->is_prm_input;

    /* Reading all input timestamps, taking the most recent of the timestamps to pass along */
    for (prm_id = 0U; prm_id < node_obj_desc->num_params; prm_id++)
    {
        if (NULL != obj_desc[prm_id])
        {
            if (tivxFlagIsBitSet(is_prm_input_flag, ((uint32_t)1U<<prm_id)))
            {
                obj_timestamp = obj_desc[prm_id]->timestamp;

                if (obj_timestamp > timestamp)
                {
                    timestamp = obj_timestamp;
                }
            }
        }
    }

    /* Setting all outputs to use most recent of the timestamps */
    for (prm_id = 0U; prm_id < node_obj_desc->num_params; prm_id++)
    {
        if (NULL != obj_desc[prm_id])
        {
            if (!tivxFlagIsBitSet(is_prm_input_flag, ((uint32_t)1U<<prm_id)))
            {
                obj_desc[prm_id]->timestamp = timestamp;

                /* Handle case of parent objects */
                parent_obj_desc = ownObjDescGet(
                        obj_desc[prm_id]->scope_obj_desc_id);

                if(parent_obj_desc!=NULL)
                {
                    parent_obj_desc->timestamp = timestamp;
                }
            }
        }
    }
}

