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

static tivx_target tivxTargetAllocHandle(vx_enum target_id);
static void tivxTargetFreeHandle(tivx_target *target_handle);
static vx_status tivxTargetDequeueObjDesc(tivx_target target, uint16_t *obj_desc_id, uint32_t timeout);
static tivx_target tivxTargetGetHandle(vx_enum target_id);
static void tivxTargetNodeDescSendComplete(const tivx_obj_desc_node_t *node_obj_desc);
static vx_bool tivxTargetNodeDescCanNodeExecute(const tivx_obj_desc_node_t *node_obj_desc);
static void tivxTargetNodeDescTriggerNextNodes(const tivx_obj_desc_node_t *node_obj_desc);
static void tivxTargetNodeDescNodeExecuteTargetKernel(tivx_obj_desc_node_t *node_obj_desc, uint16_t prm_obj_desc_id[]);
static void tivxTargetNodeDescNodeExecuteUserKernel(tivx_obj_desc_node_t *node_obj_desc, uint16_t prm_obj_desc_id[]);
static void tivxTargetNodeDescNodeExecute(tivx_target target, tivx_obj_desc_node_t *node_obj_desc);
static vx_status tivxTargetNodeDescNodeCreate(tivx_obj_desc_node_t *node_obj_desc);
static vx_status tivxTargetNodeDescNodeDelete(const tivx_obj_desc_node_t *node_obj_desc);
static vx_status tivxTargetNodeDescNodeControl(
    tivx_obj_desc_cmd_t *cmd_obj_desc,
    const tivx_obj_desc_node_t *node_obj_desc);
static void tivxTargetCmdDescHandleAck(tivx_obj_desc_cmd_t *cmd_obj_desc);
static vx_action tivxTargetCmdDescHandleUserCallback(tivx_obj_desc_node_t *node_obj_desc, uint64_t timestamp);
static void tivxTargetSetGraphStateAbandon(
    const tivx_obj_desc_node_t *node_obj_desc);
static void tivxTargetCmdDescSendAck(tivx_obj_desc_cmd_t *cmd_obj_desc, vx_status status);
static void tivxTargetCmdDescHandler(tivx_obj_desc_cmd_t *cmd_obj_desc);
static void VX_CALLBACK tivxTargetTaskMain(void *app_var);


static tivx_target tivxTargetAllocHandle(vx_enum target_id)
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
            tivxLogResourceAlloc("TIVX_TARGET_MAX_TARGETS_IN_CPU", 1);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxTargetAllocHandle: Exceeded max targets in CPU. Modify TIVX_TARGET_MAX_TARGETS_IN_CPU value in tivx_config.h\n");
        VX_PRINT(VX_ZONE_ERROR, "tivxTargetAllocHandle: May need to increase the value of TIVX_TARGET_MAX_TARGETS_IN_CPU in tiovx/include/tivx_config.h\n");
    }

    return target;
}

static void tivxTargetFreeHandle(tivx_target *target_handle)
{
    if((NULL != target_handle) && (*target_handle!=NULL))
    {
        /* mark target handle as free */
        (*target_handle)->target_id = TIVX_TARGET_ID_INVALID;

        *target_handle = NULL;
    }
}

static vx_status tivxTargetDequeueObjDesc(tivx_target target, uint16_t *obj_desc_id, uint32_t timeout)
{
    vx_status status;
    uint32_t value = TIVX_OBJ_DESC_INVALID;

    *obj_desc_id = TIVX_OBJ_DESC_INVALID;

    status = tivxQueueGet(&target->job_queue_handle,
                &value, timeout);

    if(status == VX_SUCCESS)
    {
        *obj_desc_id = (uint16_t)value;
    }

    return status;
}

static tivx_target tivxTargetGetHandle(vx_enum target_id)
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

static void tivxTargetNodeDescSendComplete(
    const tivx_obj_desc_node_t *node_obj_desc)
{
    uint16_t cmd_obj_desc_id;

    if( (tivxFlagIsBitSet(node_obj_desc->flags, TIVX_NODE_FLAG_IS_USER_CALLBACK))
            ||
        (node_obj_desc->num_out_nodes == 0)
        )
    {
        cmd_obj_desc_id = node_obj_desc->node_complete_cmd_obj_desc_id;

        if( cmd_obj_desc_id != TIVX_OBJ_DESC_INVALID)
        {
            tivx_obj_desc_cmd_t *cmd_obj_desc = (tivx_obj_desc_cmd_t *)tivxObjDescGet(cmd_obj_desc_id);

            if( tivxObjDescIsValidType( (tivx_obj_desc_t*)cmd_obj_desc, TIVX_OBJ_DESC_CMD) )
            {
                uint64_t timestamp = tivxPlatformGetTimeInUsecs()*1000;

                tivx_uint64_to_uint32(
                    timestamp,
                    &cmd_obj_desc->timestamp_h,
                    &cmd_obj_desc->timestamp_l
                );

                /* users wants a notification of node complete or this is leaf node
                 * so send node complete command to host
                 */
                tivxObjDescSend( cmd_obj_desc->dst_target_id, cmd_obj_desc_id);
            }
        }
    }
}

static vx_bool tivxTargetNodeDescCanNodeExecute(
    const tivx_obj_desc_node_t *node_obj_desc)
{
    tivx_obj_desc_node_t *prev_node_obj_desc;
    uint16_t prev_node_obj_desc_id;
    uint16_t i;
    vx_bool can_execute = vx_true_e;

    for(i=0; i<node_obj_desc->num_in_nodes; i++)
    {
        prev_node_obj_desc_id = node_obj_desc->in_node_id[i];
        prev_node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(prev_node_obj_desc_id);

        if( tivxObjDescIsValidType( (tivx_obj_desc_t*)prev_node_obj_desc, TIVX_OBJ_DESC_NODE) )
        {
            if( tivxFlagIsBitSet(prev_node_obj_desc->flags,
                        TIVX_NODE_FLAG_IS_EXECUTED) == vx_false_e)
            {
                can_execute = vx_false_e;
            }
        }
    }

    return can_execute;
}

static void tivxTargetNodeDescTriggerNextNodes(
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
        next_node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(next_node_obj_desc_id);

        if( tivxObjDescIsValidType( (tivx_obj_desc_t*)next_node_obj_desc, TIVX_OBJ_DESC_NODE) )
        {
            can_execute = tivxTargetNodeDescCanNodeExecute(next_node_obj_desc);

            if(can_execute == vx_true_e)
            {
                tivxObjDescSend( next_node_obj_desc->target_id, next_node_obj_desc_id);
            }
        }
    }
}

static void tivxTargetNodeDescNodeExecuteTargetKernel(
    tivx_obj_desc_node_t *node_obj_desc, uint16_t prm_obj_desc_id[])
{
    tivx_target_kernel_instance target_kernel_instance;
    tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS];
    uint32_t i, cnt, loop_max = 1;
    int32_t exe_status = 0;
    uint32_t is_prm_replicated = node_obj_desc->is_prm_replicated;
    tivx_obj_desc_t *parent_obj_desc[TIVX_KERNEL_MAX_PARAMS];
    tivx_obj_desc_t *prm_obj_desc;

    if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) ==
        vx_true_e)
    {
        loop_max = node_obj_desc->num_of_replicas;
    }

    for (cnt = 0; cnt < loop_max; cnt ++)
    {
        target_kernel_instance = tivxTargetKernelInstanceGet(
            node_obj_desc->target_kernel_index[cnt], node_obj_desc->kernel_id);

        for(i=0; i<node_obj_desc->num_params ; i++)
        {
            parent_obj_desc[i] = NULL;

            if(is_prm_replicated & (1U<<i))
            {
                prm_obj_desc = tivxObjDescGet(prm_obj_desc_id[i]);
                if(prm_obj_desc)
                {
                    parent_obj_desc[i] = tivxObjDescGet(
                        prm_obj_desc->scope_obj_desc_id);
                }
            }
        }


        for(i=0; i<node_obj_desc->num_params ; i++)
        {
            params[i] = NULL;
            if(is_prm_replicated & (1U<<i))
            {
                if(parent_obj_desc[i])
                {
                    if(parent_obj_desc[i]->type==TIVX_OBJ_DESC_OBJARRAY)
                    {
                        params[i] = tivxObjDescGet(
                            ((tivx_obj_desc_object_array_t*)parent_obj_desc[i])->
                                obj_desc_id[cnt]);
                    }
                    else
                    if(parent_obj_desc[i]->type==TIVX_OBJ_DESC_PYRAMID)
                    {
                        params[i] = tivxObjDescGet(
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
                params[i] = tivxObjDescGet(prm_obj_desc_id[i]);
            }
        }

        exe_status |= tivxTargetKernelExecute(target_kernel_instance, params,
            node_obj_desc->num_params);
    }
}

static void tivxTargetNodeDescNodeExecuteUserKernel(tivx_obj_desc_node_t *node_obj_desc, uint16_t prm_obj_desc_id[])
{
    vx_reference prm_ref[TIVX_KERNEL_MAX_PARAMS];
    uint32_t i;

    for(i=0; i<node_obj_desc->num_params; i++)
    {
        prm_ref[i] = ownReferenceGetHandleFromObjDescId(prm_obj_desc_id[i]);
    }
    node_obj_desc->exe_status = ownNodeUserKernelExecute((vx_node)(uintptr_t)node_obj_desc->base.host_ref, prm_ref);
}

vx_bool tivxTargetNodeDescIsPrevPipeNodeBlocked(tivx_obj_desc_node_t *node_obj_desc)
{
    tivx_obj_desc_node_t *prev_node_obj_desc;
    vx_bool is_prev_node_blocked = vx_false_e;

    prev_node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(node_obj_desc->prev_pipe_node_id);
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
                is_prev_node_blocked = vx_true_e;
            }
        }
        else
        if(node_obj_desc->state == TIVX_NODE_OBJ_DESC_STATE_BLOCKED)
        {
            /* this is trigger from prev node or due to resource being released so proceed with execution */
            node_obj_desc->state = TIVX_NODE_OBJ_DESC_STATE_IDLE;
        }
    }
    return is_prev_node_blocked;
}


static void tivxTargetNodeDescNodeMarkComplete(tivx_obj_desc_node_t *node_obj_desc, uint16_t *blocked_node_id)
{
    /* check if any node is blocked on this node to get unblocked and complete execution
     * This will be a node from next pipeline instance
     */
    *blocked_node_id = node_obj_desc->blocked_node_id;
    node_obj_desc->blocked_node_id = TIVX_OBJ_DESC_INVALID;
    node_obj_desc->state = TIVX_NODE_OBJ_DESC_STATE_IDLE;
    tivxFlagBitSet(&node_obj_desc->flags, TIVX_NODE_FLAG_IS_EXECUTED);
}

static void tivxTargetNodeDescNodeExecute(tivx_target target, tivx_obj_desc_node_t *node_obj_desc)
{
    uint64_t beg_time, end_time;
    uint16_t blocked_node_id = TIVX_OBJ_DESC_INVALID;
    uint16_t prm_obj_desc_id[TIVX_KERNEL_MAX_PARAMS];

    /* if node is already executed do nothing */
    if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_EXECUTED) == vx_false_e )
    {
        /* check if same node in previous pipeline instance is blocked, if yes then
         * dont acquire parameters for this node
         */
        if(tivxTargetNodeDescIsPrevPipeNodeBlocked(node_obj_desc)==vx_false_e)
        {
            vx_bool is_node_blocked;

            is_node_blocked = vx_false_e;

            VX_PRINT(VX_ZONE_INFO,"Node (node=%d, pipe=%d) acquiring parameters on target %08x\n",
                                 node_obj_desc->base.obj_desc_id,
                                 node_obj_desc->pipeline_id,
                                 target->target_id
                           );

            tivxTargetNodeDescAcquireAllParameters(node_obj_desc, prm_obj_desc_id, &is_node_blocked);

            if(is_node_blocked==vx_false_e)
            {
                VX_PRINT(VX_ZONE_INFO,"Node (node=%d, pipe=%d) executing on target %08x\n",
                                 node_obj_desc->base.obj_desc_id,
                                 node_obj_desc->pipeline_id,
                                 target->target_id
                           );

                beg_time = tivxPlatformGetTimeInUsecs();

                tivxLogRtTraceNodeExeStart(beg_time, node_obj_desc);

                if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_TARGET_KERNEL) )
                {
                    tivxTargetNodeDescNodeExecuteTargetKernel(node_obj_desc, prm_obj_desc_id);
                }
                else
                {
                    tivxTargetNodeDescNodeExecuteUserKernel(node_obj_desc, prm_obj_desc_id);
                }

                end_time = tivxPlatformGetTimeInUsecs();

                tivxLogRtTraceNodeExeEnd(end_time, node_obj_desc);

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

                tivxTargetNodeDescNodeMarkComplete(node_obj_desc, &blocked_node_id);
                tivxTargetNodeDescReleaseAllParameters(node_obj_desc, prm_obj_desc_id);
                tivxTargetNodeDescSendComplete(node_obj_desc);
                tivxTargetNodeDescTriggerNextNodes(node_obj_desc);

                if(blocked_node_id!=TIVX_OBJ_DESC_INVALID)
                {
                    /* this will be same node in next pipeline to trigger it last */
                    VX_PRINT(VX_ZONE_INFO,"Re-triggering (node=%d)\n",
                             blocked_node_id
                    );
                    tivxTargetTriggerNode(blocked_node_id);
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_INFO,"Node (node=%d, pipe=%d) ... BLOCKED for resrouces on target %08x\n",
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

static vx_status tivxTargetNodeDescNodeCreate(tivx_obj_desc_node_t *node_obj_desc)
{
    tivx_target_kernel_instance target_kernel_instance;
    vx_status status = VX_SUCCESS;
    uint16_t i, cnt, loop_max = 1;
    tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS];
    uint32_t is_prm_replicated = node_obj_desc->is_prm_replicated;
    tivx_obj_desc_t *parent_obj_desc[TIVX_KERNEL_MAX_PARAMS];
    tivx_obj_desc_t *prm_obj_desc;
    tivx_obj_desc_kernel_name_t *kernel_name_obj_desc;
    char *kernel_name = NULL;

    if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) ==
        vx_true_e)
    {
        loop_max = node_obj_desc->num_of_replicas;
    }

    kernel_name_obj_desc = (tivx_obj_desc_kernel_name_t*)tivxObjDescGet(node_obj_desc->kernel_name_obj_desc_id);
    if(kernel_name_obj_desc!=NULL)
    {
        kernel_name = kernel_name_obj_desc->kernel_name;
    }
    for (cnt = 0; cnt < loop_max; cnt ++)
    {
        target_kernel_instance = tivxTargetKernelInstanceAlloc(
            node_obj_desc->kernel_id, kernel_name, node_obj_desc->target_id);

        if(target_kernel_instance == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxTargetNodeDescNodeCreate: target_kernel_instance is NULL\n");
            status = VX_ERROR_NO_RESOURCES;
            break;
        }
        else
        {
            /* save index key for fast retrival of handle during run-time */
            node_obj_desc->target_kernel_index[cnt] =
                tivxTargetKernelInstanceGetIndex(target_kernel_instance);

            for(i=0; i<node_obj_desc->num_params ; i++)
            {
                parent_obj_desc[i] = NULL;

                if(is_prm_replicated & (1U<<i))
                {
                    prm_obj_desc = tivxObjDescGet(node_obj_desc->data_id[i]);
                    if(prm_obj_desc)
                    {
                        parent_obj_desc[i] = tivxObjDescGet(
                            prm_obj_desc->scope_obj_desc_id);
                    }
                }
            }


            for(i=0; i<node_obj_desc->num_params ; i++)
            {
                params[i] = NULL;
                if(is_prm_replicated & (1U<<i))
                {
                    if(parent_obj_desc[i])
                    {
                        if(parent_obj_desc[i]->type==TIVX_OBJ_DESC_OBJARRAY)
                        {
                            params[i] = tivxObjDescGet(
                                ((tivx_obj_desc_object_array_t*)parent_obj_desc[i])->
                                    obj_desc_id[cnt]);
                        }
                        else
                        if(parent_obj_desc[i]->type==TIVX_OBJ_DESC_PYRAMID)
                        {
                            params[i] = tivxObjDescGet(
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
                    params[i] = tivxObjDescGet(node_obj_desc->data_id[i]);
                }
            }
            /* copy border mode also in the target_kernel_instance */
            target_kernel_instance->border_mode =
                node_obj_desc->border_mode;

            status = tivxTargetKernelCreate(target_kernel_instance,
                params, node_obj_desc->num_params);

            if(status!=VX_SUCCESS)
            {
                tivxTargetKernelInstanceFree(&target_kernel_instance);
                break;
            }
        }
    }

    if (VX_SUCCESS != status)
    {
        for (i = 0; i < cnt; i ++)
        {
            target_kernel_instance = tivxTargetKernelInstanceGet(
                node_obj_desc->target_kernel_index[i], node_obj_desc->kernel_id);

            if (NULL != target_kernel_instance)
            {
                tivxTargetKernelInstanceFree(&target_kernel_instance);
            }
        }
    }

    return status;
}

static vx_status tivxTargetNodeDescNodeDelete(const tivx_obj_desc_node_t *node_obj_desc)
{
    tivx_target_kernel_instance target_kernel_instance;
    vx_status status = VX_SUCCESS;
    uint16_t i, cnt, loop_max = 1;
    tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS];

    if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) ==
        vx_true_e)
    {
        loop_max = node_obj_desc->num_of_replicas;
    }

    for (cnt = 0; cnt < loop_max; cnt ++)
    {
        target_kernel_instance = tivxTargetKernelInstanceGet(
            node_obj_desc->target_kernel_index[cnt], node_obj_desc->kernel_id);

        if(target_kernel_instance == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxTargetNodeDescNodeDelete: target_kernel_instance is NULL\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
        else
        {
            {
                /* NOTE: nothing special for replicated node during
                         create/delete */
                for(i=0; i<node_obj_desc->num_params ; i++)
                {
                    params[i] = tivxObjDescGet(node_obj_desc->data_id[i]);
                }

                status |= tivxTargetKernelDelete(target_kernel_instance,
                    params, node_obj_desc->num_params);
            }

            tivxTargetKernelInstanceFree(&target_kernel_instance);
        }
    }

    return status;
}

static vx_status tivxTargetNodeDescNodeControl(
    tivx_obj_desc_cmd_t *cmd_obj_desc,
    const tivx_obj_desc_node_t *node_obj_desc)
{
    tivx_target_kernel_instance target_kernel_instance;
    vx_status status = VX_SUCCESS;
    uint16_t i, cnt, loop_max = 1;
    tivx_obj_desc_t *params[TIVX_CMD_MAX_OBJ_DESCS];

    if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) ==
        vx_true_e)
    {
        loop_max = node_obj_desc->num_of_replicas;
    }

    for (cnt = 0; cnt < loop_max; cnt ++)
    {
        target_kernel_instance = tivxTargetKernelInstanceGet(
            node_obj_desc->target_kernel_index[cnt], node_obj_desc->kernel_id);

        if(target_kernel_instance == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxTargetNodeDescNodeControl: target_kernel_instance is NULL\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
        else
        {
            for(i=0; i<cmd_obj_desc->num_obj_desc; i++)
            {
                params[i] = tivxObjDescGet(node_obj_desc->data_id[i]);
            }

            status = tivxTargetKernelControl(target_kernel_instance, params,
                cmd_obj_desc->num_obj_desc);
        }
    }

    return status;
}

static void tivxTargetCmdDescHandleAck(tivx_obj_desc_cmd_t *cmd_obj_desc)
{
    if( tivxFlagIsBitSet( cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK))
    {
        tivxFlagBitClear( &cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK);

        tivxEventPost((tivx_event)(uintptr_t)cmd_obj_desc->ack_event_handle);
    }
}

static vx_action tivxTargetCmdDescHandleUserCallback(tivx_obj_desc_node_t *node_obj_desc, uint64_t timestamp)
{
    vx_action action;
    vx_node node = (vx_node)(uintptr_t)node_obj_desc->base.host_ref;
    vx_bool is_send_graph_complete_event = vx_false_e;

    /* return action is ignored */
    action = ownNodeExecuteUserCallback(node);

    /* if this is leaf node, check if graph is completed */
    if(ownNodeGetNumOutNodes(node)==0)
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

    /* first we let any node events to go thru before sending graph events */
    if(is_send_graph_complete_event)
    {
        ownSendGraphCompletedEvent(node->graph);
    }

    return (action);
}

static void tivxTargetSetGraphStateAbandon(
    const tivx_obj_desc_node_t *node_obj_desc)
{
    vx_node node = (vx_node)(uintptr_t)node_obj_desc->base.host_ref;

    ownSetGraphState(node->graph, node_obj_desc->pipeline_id, VX_GRAPH_STATE_ABANDONED);
}

static void tivxTargetCmdDescSendAck(tivx_obj_desc_cmd_t *cmd_obj_desc, vx_status status)
{
    if( tivxFlagIsBitSet( cmd_obj_desc->flags, TIVX_CMD_FLAG_SEND_ACK))
    {
        tivxFlagBitSet( &cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK);

        cmd_obj_desc->cmd_status = (uint32_t)status;

        tivxObjDescSend(cmd_obj_desc->src_target_id, cmd_obj_desc->base.obj_desc_id);
    }
}

static void tivxTargetCmdDescHandler(tivx_obj_desc_cmd_t *cmd_obj_desc)
{
    uint16_t node_obj_desc_id;
    tivx_obj_desc_node_t *node_obj_desc;
    vx_action action;
    vx_status status = VX_SUCCESS;

    switch(cmd_obj_desc->cmd_id)
    {
        case TIVX_CMD_NODE_CREATE:
        case TIVX_CMD_NODE_DELETE:
        case TIVX_CMD_NODE_CONTROL:
            if( tivxFlagIsBitSet( cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK) == vx_false_e )
            {
                node_obj_desc_id = cmd_obj_desc->obj_desc_id[0];
                node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(node_obj_desc_id);

                if( tivxObjDescIsValidType( (tivx_obj_desc_t*)node_obj_desc, TIVX_OBJ_DESC_NODE) )
                {
                    if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_TARGET_KERNEL) )
                    {
                        if(cmd_obj_desc->cmd_id == TIVX_CMD_NODE_CREATE)
                        {
                            status = tivxTargetNodeDescNodeCreate(node_obj_desc);
                        }
                        else
                        if(cmd_obj_desc->cmd_id == TIVX_CMD_NODE_DELETE)
                        {
                            status = tivxTargetNodeDescNodeDelete(node_obj_desc);
                        }
                        else
                        if(cmd_obj_desc->cmd_id == TIVX_CMD_NODE_CONTROL)
                        {
                            status = tivxTargetNodeDescNodeControl(cmd_obj_desc, node_obj_desc);
                        }
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "tivxTargetCmdDescHandler: object descriptor type is invalid\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                tivxTargetCmdDescSendAck(cmd_obj_desc, status);
            }
            else
            {
                /* this is ACK for a previously sent command */
                tivxTargetCmdDescHandleAck(cmd_obj_desc);
            }
            break;

        case TIVX_CMD_NODE_USER_CALLBACK:
            node_obj_desc_id = cmd_obj_desc->obj_desc_id[0];
            node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(node_obj_desc_id);

            if( tivxObjDescIsValidType( (tivx_obj_desc_t*)node_obj_desc, TIVX_OBJ_DESC_NODE) )
            {
                uint64_t timestamp;

                tivx_uint32_to_uint64(&timestamp, cmd_obj_desc->timestamp_h, cmd_obj_desc->timestamp_l);

                action = tivxTargetCmdDescHandleUserCallback(node_obj_desc, timestamp);

                if (action == VX_ACTION_ABANDON)
                {
                    tivxTargetSetGraphStateAbandon(node_obj_desc);
                }
            }
            /* No ack for user callback command */
            break;
        case TIVX_CMD_DATA_REF_CONSUMED:
        {
            tivx_data_ref_queue data_ref_q;

            data_ref_q = (tivx_data_ref_queue)ownReferenceGetHandleFromObjDescId(cmd_obj_desc->obj_desc_id[0]);

            if( data_ref_q != NULL )
            {
                uint64_t timestamp;

                tivx_uint32_to_uint64(&timestamp, cmd_obj_desc->timestamp_h, cmd_obj_desc->timestamp_l);

                tivxDataRefQueueSendRefConsumedEvent(data_ref_q, timestamp);
            }
            /* No ack for this command */
        }
            break;
        default:

            break;
    }

}

static void VX_CALLBACK tivxTargetTaskMain(void *app_var)
{
    tivx_target target = (tivx_target)app_var;
    tivx_obj_desc_t *obj_desc;
    uint16_t obj_desc_id;
    vx_status status = VX_SUCCESS;

    while(target->targetExitRequest == vx_false_e)
    {
        status = tivxTargetDequeueObjDesc(target,
                    &obj_desc_id, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

        if(    (status != VX_SUCCESS)
            || (obj_desc_id == TIVX_OBJ_DESC_INVALID) )
        {
            /* in case of error, do nothing,
             * if target exit was requested, while(...) condition check with
             * check and exit
             */
        }
        else
        {
            obj_desc = tivxObjDescGet(obj_desc_id);
            if(obj_desc == NULL)
            {
                /* in valid obj_desc_id received */
            }
            else
            {
                tivxLogRtTraceTargetExeStart(target, obj_desc);

                switch(obj_desc->type)
                {
                    case TIVX_OBJ_DESC_CMD:
                        if( tivxObjDescIsValidType( obj_desc, TIVX_OBJ_DESC_CMD) )
                        {
                            tivxTargetCmdDescHandler((tivx_obj_desc_cmd_t*)obj_desc);
                        }
                        break;
                    case TIVX_OBJ_DESC_NODE:
                        if( tivxObjDescIsValidType( obj_desc, TIVX_OBJ_DESC_NODE) )
                        {
                            tivxTargetNodeDescNodeExecute(target, (tivx_obj_desc_node_t*)obj_desc);
                        }
                        break;
                    default:
                        /* unsupported obj_desc received at target */
                        break;
                }

                tivxLogRtTraceTargetExeEnd(target, obj_desc);
            }
        }
    }

    target->targetExitDone = vx_true_e;
}

vx_status tivxTargetCreate(vx_enum target_id, tivx_target_create_params_t *params)
{
    vx_status status = VX_SUCCESS;
    tivx_target target;

    target = tivxTargetAllocHandle(target_id);

    if(target == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxTargetCreate: target is NULL\n");
        status = VX_ERROR_NO_RESOURCES;
    }
    else
    {
        target->target_id = target_id;

        tivxTaskSetDefaultCreateParams(&target->task_params);
        target->task_params.stack_ptr = params->task_stack_ptr;
        target->task_params.stack_size = params->task_stack_size;
        target->task_params.core_affinity = params->task_core_affinity;
        target->task_params.priority = params->task_priority;
        target->task_params.task_main = &tivxTargetTaskMain;
        target->task_params.app_var = target;

        target->targetExitRequest = vx_false_e;
        target->targetExitDone = vx_false_e;


        /* create job queue */
        status = tivxQueueCreate(&target->job_queue_handle,
                        TIVX_TARGET_MAX_JOB_QUEUE_DEPTH,
                        target->job_queue_memory,
                        TIVX_QUEUE_FLAG_BLOCK_ON_GET
                );
        if(status == VX_SUCCESS)
        {
            /* create and start target task */
            status = tivxTaskCreate(&target->task_handle, &target->task_params);
            if(status != VX_SUCCESS)
            {
                /* delete job queue due to task create error */
                tivxQueueDelete(&target->job_queue_handle);
            }
        }
        if(status!=VX_SUCCESS)
        {
            tivxTargetFreeHandle(&target);
        }
    }
    return status;
}

vx_status tivxTargetDelete(vx_enum target_id)
{
    vx_status status = VX_SUCCESS;
    tivx_target target;

    target = tivxTargetGetHandle(target_id);

    /* delete task */

    if (NULL != target)
    {
        /* set flag to break target from main loop */
        target->targetExitRequest = vx_true_e;

        /* queue a invalid object descriptor to unblock queue wait */
        tivxTargetQueueObjDesc(target_id, TIVX_OBJ_DESC_INVALID);

        /* wait until target exit is done */
        while(target->targetExitDone==vx_false_e)
        {
            tivxTaskWaitMsecs(1);
        }
        tivxTaskDelete(&target->task_handle);

        /* delete job queue */
        tivxQueueDelete(&target->job_queue_handle);
    }

    return status;
}

void tivxTargetTriggerNode(uint16_t node_obj_desc_id)
{
    tivx_obj_desc_node_t *node_obj_desc;

    node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(node_obj_desc_id);

    if( tivxObjDescIsValidType( (tivx_obj_desc_t*)node_obj_desc, TIVX_OBJ_DESC_NODE) )
    {
        tivxObjDescSend( node_obj_desc->target_id, node_obj_desc_id);
    }
}

vx_status tivxTargetQueueObjDesc(vx_enum target_id, uint16_t obj_desc_id)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    tivx_target target = tivxTargetGetHandle(target_id);

    if(target!=NULL)
    {
        status = tivxQueuePut(&target->job_queue_handle,
                obj_desc_id, TIVX_EVENT_TIMEOUT_NO_WAIT);
    }

    return status;
}

void tivxTargetSetDefaultCreateParams(tivx_target_create_params_t *params)
{
    params->task_stack_ptr = NULL;
    params->task_stack_size = 0;
    params->task_core_affinity = 0;
    params->task_priority = TIVX_TASK_PRI_LOWEST;
}

vx_enum tivxTargetGetCpuId(vx_enum target_id)
{
    return TIVX_GET_CPU_ID(target_id);
}

void tivxTargetInit(void)
{
    uint16_t i;

    for(i=0; i<dimof(g_target_table); i++)
    {
        g_target_table[i].target_id = TIVX_TARGET_ID_INVALID;
    }

    tivxTargetKernelInit();
    tivxTargetKernelInstanceInit();
}

void tivxTargetDeInit(void)
{
    tivxTargetKernelInstanceDeInit();
    tivxTargetKernelDeInit();

}
