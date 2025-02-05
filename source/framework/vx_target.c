/*
*
* Copyright (c) 2017-2023 Texas Instruments Incorporated
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

#define VX_PRINT_OBJECT(zone, object, message, ...) do { tivx_print_object(((vx_enum)zone), object->debug_zonemask, "[%s:%u] " message, __FUNCTION__, __LINE__, ## __VA_ARGS__); } while (1 == 0)

#ifndef PC
#define TIVX_TARGET_MAX_CPUS_IN_EXE (1u)
#else
#define TIVX_TARGET_MAX_CPUS_IN_EXE (TIVX_CPU_ID_MAX)
#endif

/* When building for PC, all targets in system need to be accounted for in a single table.
   When building for SoC, each executable running on different CPUs will have their own individual target tables.
   This design allows for both builds to use the same target_ids and target id creation functions. */
static tivx_target_t g_target_table[TIVX_TARGET_MAX_CPUS_IN_EXE][TIVX_TARGET_MAX_TARGETS_IN_CPU];

own_execute_user_kernel_f      g_executeUserKernel_f = (own_execute_user_kernel_f)NULL;
own_target_cmd_desc_handler_f  g_target_cmd_desc_handler_for_host_f = (own_target_cmd_desc_handler_f)NULL;

static tivx_target ownTargetAllocHandle(vx_enum target_id);

static void ownTargetFreeHandle(tivx_target *target_handle);

static vx_status ownTargetDequeueObjDesc(tivx_target target, uint16_t *obj_desc_id, uint32_t timeout);
static tivx_target ownTargetGetHandle(vx_enum target_id);
static void ownTargetNodeDescSendComplete(const tivx_obj_desc_node_t *node_obj_desc);
static vx_bool ownTargetNodeDescCanNodeExecute(const tivx_obj_desc_node_t *node_obj_desc);
static void ownTargetNodeDescTriggerNextNodes(const tivx_obj_desc_node_t *node_obj_desc);
static void ownTargetNodeDescNodeExecuteKernel(tivx_obj_desc_node_t *node_obj_desc, uint16_t prm_obj_desc_id[]);
static void ownTargetNodeDescNodeExecuteTargetKernel(tivx_obj_desc_node_t *node_obj_desc, uint16_t prm_obj_desc_id[]);
static void ownTargetNodeDescNodeExecute(tivx_target target, tivx_obj_desc_node_t *node_obj_desc);
static vx_status ownTargetNodeDescNodeCreate(tivx_obj_desc_node_t *node_obj_desc);
static vx_status ownTargetNodeDescNodeDelete(const tivx_obj_desc_node_t *node_obj_desc);
static vx_status ownTargetNodeDescNodeControl(
    const tivx_obj_desc_cmd_t *cmd_obj_desc,
    const tivx_obj_desc_node_t *node_obj_desc);

/* TIOVX-1671- Host only Id: TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM001 */
static void ownTargetCmdDescHandleAck(tivx_obj_desc_cmd_t *cmd_obj_desc);

static void ownTargetCmdDescSendAck(tivx_obj_desc_cmd_t *cmd_obj_desc, vx_status status);
static void ownTargetCmdDescHandler(tivx_obj_desc_cmd_t *cmd_obj_desc);
static void VX_CALLBACK ownTargetTaskMain(void *app_var);
static vx_bool ownTargetNodeDescIsPrevPipeNodeBlocked(tivx_obj_desc_node_t *node_obj_desc);
static void ownTargetNodeDescNodeMarkComplete(tivx_obj_desc_node_t *node_obj_desc, uint16_t *blocked_node_id);
static vx_status ownTargetNodeSendCommand(const tivx_obj_desc_cmd_t *cmd_obj_desc,
    uint32_t node_id, const tivx_obj_desc_node_t *node_obj_desc);
static uint16_t ownTargetGetSubTableId(vx_enum target_id);

static uint16_t ownTargetGetSubTableId(vx_enum target_id)
{
    uint16_t id;

    #ifndef PC
        id = 0;
    #else
        id = TIVX_GET_CPU_ID(target_id);
    #endif

    return id;
}

static tivx_target ownTargetAllocHandle(vx_enum target_id)
{
    uint16_t target_inst = TIVX_GET_TARGET_INST(target_id);
    uint16_t sub_table_id = ownTargetGetSubTableId(target_id);

    tivx_target tmp_target = NULL, target = NULL;

    if(target_inst < TIVX_TARGET_MAX_TARGETS_IN_CPU)
    {
        tmp_target = &g_target_table[sub_table_id][target_inst];

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

#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM007
<justification end> */
#endif
/* TIOVX-1671- Host only Id: TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM007 */
static void ownTargetFreeHandle(tivx_target *target_handle)
{
    if((NULL != target_handle) && /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR001 */
    (*target_handle!=NULL)) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR002 */
    {
        /* mark target handle as free */
        (*target_handle)->target_id = (vx_enum)TIVX_TARGET_ID_INVALID;

        *target_handle = NULL;
        ownLogResourceFree("TIVX_TARGET_MAX_TARGETS_IN_CPU", 1);
    }
}
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif

static vx_status ownTargetDequeueObjDesc(tivx_target target, uint16_t *obj_desc_id, uint32_t timeout)
{
    vx_status status;
    uintptr_t value = (vx_enum)TIVX_OBJ_DESC_INVALID;

    *obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

    status = tivxQueueGet(&target->job_queue_handle,
                &value, timeout);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR003
<justification end> */
    if(status == (vx_status)VX_SUCCESS) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR003 */
    {
        *obj_desc_id = (uint16_t)value;
    }
/* LDRA_JUSTIFY_END */
    return status;
}

static tivx_target ownTargetGetHandle(vx_enum target_id)
{
    uint16_t target_inst = TIVX_GET_TARGET_INST(target_id);
    uint16_t sub_table_id = ownTargetGetSubTableId(target_id);
    tivx_target tmp_target = NULL, target = NULL;

    if(target_inst < TIVX_TARGET_MAX_TARGETS_IN_CPU)
    {
        tmp_target = &g_target_table[sub_table_id][target_inst];

        /* Target initialized with NULL, no need to reassign Target to NULL */
        if(tmp_target->target_id == target_id)
        {
            /* target id matches so return it */
            target = tmp_target;
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

        if( (vx_enum)cmd_obj_desc_id != (vx_enum)TIVX_OBJ_DESC_INVALID) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR004 */
        {
            tivx_obj_desc_cmd_t *cmd_obj_desc = (tivx_obj_desc_cmd_t *)ownObjDescGet(cmd_obj_desc_id);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR005
<justification end> */
            if( ownObjDescIsValidType( (tivx_obj_desc_t*)cmd_obj_desc, TIVX_OBJ_DESC_CMD) != 0) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR005 */
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
                /* error status check is not required
                 * as the return status is not used further
                 * in ownTargetNodeDescSendComplete
                 */
                (void)ownObjDescSend( cmd_obj_desc->dst_target_id, cmd_obj_desc_id);
            }
/* LDRA_JUSTIFY_END */
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

        if( ownObjDescIsValidType( (tivx_obj_desc_t*)prev_node_obj_desc, TIVX_OBJ_DESC_NODE) != 0) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR006 */
        {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM019
<justification end> */
/* TIOVX-1671- Host only Id: TIOVX_CODE_COVERAGE_TARGET_UM019 */
            if( tivxFlagIsBitSet(prev_node_obj_desc->flags,
                        TIVX_NODE_FLAG_IS_EXECUTED) == (vx_bool)vx_false_e)
            {
                can_execute = (vx_bool)vx_false_e;
                break;
            }
/* LDRA_JUSTIFY_END */
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

        if( ownObjDescIsValidType( (tivx_obj_desc_t*)next_node_obj_desc, TIVX_OBJ_DESC_NODE) != 0) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR007 */
        {
            can_execute = ownTargetNodeDescCanNodeExecute(next_node_obj_desc);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR008
<justification end> */
            if(can_execute == (vx_bool)vx_true_e) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR008 */
            {
                /* error status check is not done
                 * as the return status is not used further
                 * in ownTargetNodeDescTriggerNextNodes
                 */
                (void)ownObjDescSend( next_node_obj_desc->target_id, next_node_obj_desc_id);
            }
/* LDRA_JUSTIFY_END */
        }
    }
}

static void ownTargetNodeDescNodeExecuteKernel(
    tivx_obj_desc_node_t *node_obj_desc, uint16_t prm_obj_desc_id[])
{
    if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_TARGET_KERNEL) != 0) /* TIOVX-1967- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR001 */
    {
        ownTargetNodeDescNodeExecuteTargetKernel(node_obj_desc, prm_obj_desc_id);
    }
#ifdef HOST_ONLY
/* TIOVX-1671- Host only Id: TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM002 */
    else
    {
#if defined(A72) || defined(A53)
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR009
<justification end> */
#endif
        if(NULL != g_executeUserKernel_f) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR009 */
        {
            g_executeUserKernel_f(node_obj_desc, prm_obj_desc_id);
        }
#if defined(A72) || defined(A53)
/* LDRA_JUSTIFY_END */
#endif
    }
#endif
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
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR010
<justification end> */
                if(prm_obj_desc != NULL) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR010 */
                {
                    parent_obj_desc[i] = ownObjDescGet(
                        prm_obj_desc->scope_obj_desc_id);

                    /* if parent is NULL, then prm_obj_desc itself is the parent */
                    if(parent_obj_desc[i]==NULL)
                    {
                        parent_obj_desc[i] = prm_obj_desc;
                    }
                }
/* LDRA_JUSTIFY_END */
            }
        }

        for(i=0; i<node_obj_desc->num_params ; i++)
        {
            params[i] = NULL;
            if((is_prm_replicated & ((uint32_t)1U << i)) != 0U)
            {
                if(parent_obj_desc[i] != NULL) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR011 */
                {
                    if((vx_enum)parent_obj_desc[i]->type==(vx_enum)TIVX_OBJ_DESC_OBJARRAY)
                    {
                        params[i] = ownObjDescGet(
                            ((tivx_obj_desc_object_array_t*)parent_obj_desc[i])->
                                obj_desc_id[cnt]);
                    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR012
<justification end> */
                    else
                    if((vx_enum)parent_obj_desc[i]->type==(vx_enum)TIVX_OBJ_DESC_PYRAMID) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR012 */
/* LDRA_JUSTIFY_END */
                    {
                        params[i] = ownObjDescGet(
                            ((tivx_obj_desc_pyramid_t*)parent_obj_desc[i])->
                                obj_desc_id[cnt]);
                    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM016
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM016 */
                    else
                    {
                        params[i] = NULL;
                    }
/* LDRA_JUSTIFY_END */
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
                    if(parent_obj_desc[i] != NULL)  /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR013 */
                    {
                        if((vx_enum)parent_obj_desc[i]->type==(vx_enum)TIVX_OBJ_DESC_OBJARRAY)
                        {
                            tivx_obj_desc_t *tmp_node_param;

                            tmp_node_param = ownObjDescGet(node_obj_desc->data_id[i]);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR014
<justification end> */
                            if (NULL != tmp_node_param)  /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR014 */
                            {
                                params[i] = ownObjDescGet(
                                    ((tivx_obj_desc_object_array_t*)parent_obj_desc[i])->
                                        obj_desc_id[tmp_node_param->element_idx]);
                            }
/* LDRA_JUSTIFY_END */
                        }
                        else
                        if((vx_enum)parent_obj_desc[i]->type==(vx_enum)TIVX_OBJ_DESC_PYRAMID)
                        {
                            tivx_obj_desc_t *tmp_node_param;

                            tmp_node_param = ownObjDescGet(node_obj_desc->data_id[i]);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR015
<justification end> */
                            if (NULL != tmp_node_param)  /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR015 */
                            {
                                params[i] = ownObjDescGet(
                                    ((tivx_obj_desc_pyramid_t*)parent_obj_desc[i])->
                                        obj_desc_id[tmp_node_param->element_idx]);
                            }
/* LDRA_JUSTIFY_END */
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

                        if (NULL != tmp_node_param)  /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR016 */
                        {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR017
<justification end> */
                            if (parent_obj_desc[i]->type == tmp_node_param->type)  /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR017 */
                            {
                                params[i] = parent_obj_desc[i];
                            }
/* LDRA_JUSTIFY_END */
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
            #if defined(BUILD_BAM)
            if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_SUPERNODE) ==
                (vx_bool)vx_true_e)
            {
                params[0] = (tivx_obj_desc_t *) ownObjDescGet( node_obj_desc->base.scope_obj_desc_id );

                node_obj_desc->exe_status |= (uint32_t)ownTargetKernelExecute(target_kernel_instance, params,
                    1);
            }
            else
            #endif
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

                if (prm_obj_desc != NULL)  /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR018 */
                {
                    parent_obj_desc[i] = ownObjDescGet(
                        prm_obj_desc->scope_obj_desc_id);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR019
<justification end> */
                    if(parent_obj_desc[i]!=NULL)  /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR019 */
                    {
                        prm_obj_desc_id[i] = parent_obj_desc[i]->obj_desc_id;
                    }
/* LDRA_JUSTIFY_END */
                }
            }
            else
            {
                prm_obj_desc_id[i] = params[i]->obj_desc_id;
            }
        }
    }

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
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR020
<justification end> */
        else
        if(node_obj_desc->state == TIVX_NODE_OBJ_DESC_STATE_BLOCKED) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR020 */
/* LDRA_JUSTIFY_END */
        {
            /* this is trigger from prev node or due to resource being released so proceed with execution */
            node_obj_desc->state = TIVX_NODE_OBJ_DESC_STATE_IDLE;
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM006
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM006 */
        else
        {
            /* do nothing */
        }
/* LDRA_JUSTIFY_END */
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
    if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_EXECUTED) == (vx_bool)vx_false_e ) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR021 */
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
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR042
<justification end> */
            VX_PRINT_OBJECT(VX_ZONE_INFO, node_obj_desc, "Node (node=%d, pipe=%d) acquiring parameters on target %08x\n",
                                 node_obj_desc->base.obj_desc_id,
                                 node_obj_desc->pipeline_id,
                                 target->target_id
                           ); /* TIOVX-1967- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR042 */
/* LDRA_JUSTIFY_END */
            /* Note: not taking into account replicated node */
            target_kernel_instance = ownTargetKernelInstanceGet(
                (uint16_t)node_obj_desc->target_kernel_index[0], (vx_enum)node_obj_desc->kernel_id);

            /* Note: in the case of user kernel, target_kernel instance is NULL */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR002
<justification end> */
            if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_TARGET_KERNEL) != 0) /* TIOVX-1967- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR002 */
/* LDRA_JUSTIFY_END */
            {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR022
<justification end> */
                if (NULL != target_kernel_instance) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR022 */
                {
                    num_bufs = target_kernel_instance->kernel->num_pipeup_bufs;

                    kernel_instance_state = target_kernel_instance->state;
                }
/* LDRA_JUSTIFY_END */
            }
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM005
<justification end> */
#endif
/* TIOVX-1671- Host only Id: TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM005 */
            else
            {
                /* This is applicable for the USER KERNEL*/
                kernel_instance_state = (vx_enum)node_obj_desc->source_state;

                num_bufs = node_obj_desc->num_pipeup_bufs;
            }
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif
            if ( ((vx_enum)VX_NODE_STATE_PIPEUP == kernel_instance_state) &&
                 (num_bufs > 1U) )
            {
                int32_t buf_idx;

                for (buf_idx = 0; buf_idx < ((int32_t)num_bufs - 1); buf_idx++)
                {
                    ownTargetNodeDescAcquireAllParameters(node_obj_desc, prm_obj_desc_id, &is_node_blocked, (vx_bool)vx_true_e);
                    ownTargetNodeDescNodeExecuteKernel(node_obj_desc, prm_obj_desc_id);
                }

                node_obj_desc->source_state = (vx_enum)VX_NODE_STATE_STEADY;

                if( (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_TARGET_KERNEL) != (vx_bool)vx_false_e) && /* TIOVX-1967- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR003 */
                    (NULL != target_kernel_instance)) /* TIOVX-1967- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR004 */
                {
                    target_kernel_instance->state = (vx_enum)VX_NODE_STATE_STEADY;
                }
            }

            ownTargetNodeDescAcquireAllParameters(node_obj_desc, prm_obj_desc_id, &is_node_blocked, (vx_bool)vx_false_e);

            if(is_node_blocked==(vx_bool)vx_false_e)
            {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR043
<justification end> */
                VX_PRINT_OBJECT(VX_ZONE_INFO, node_obj_desc, "Node (node=%d, pipe=%d) executing on target %08x\n",
                                 node_obj_desc->base.obj_desc_id,
                                 node_obj_desc->pipeline_id,
                                 target->target_id
                           ); /* TIOVX-1967- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR043 */
/* LDRA_JUSTIFY_END */
                beg_time = tivxPlatformGetTimeInUsecs();

                ownLogRtTraceNodeExeStart(beg_time, node_obj_desc);

                ownTargetNodeDescNodeExecuteKernel(node_obj_desc, prm_obj_desc_id);

                end_time = tivxPlatformGetTimeInUsecs();

                ownLogRtTraceNodeExeEnd(end_time, node_obj_desc);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR044
<justification end> */
                VX_PRINT_OBJECT(VX_ZONE_INFO, node_obj_desc, "Node (node=%d, pipe=%d) executing on target %08x ... DONE !!!\n",
                                 node_obj_desc->base.obj_desc_id,
                                 node_obj_desc->pipeline_id,
                                 target->target_id
                           ); /* TIOVX-1967- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR044 */
/* LDRA_JUSTIFY_END */
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
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR045
<justification end> */
                    VX_PRINT_OBJECT(VX_ZONE_INFO, node_obj_desc, "Re-triggering (node=%d)\n",
                             blocked_node_id
                    ); /* TIOVX-1967- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR045 */
/* LDRA_JUSTIFY_END */
                    ownTargetTriggerNode(blocked_node_id);
                }
            }
            else
            {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR046
<justification end> */
                VX_PRINT_OBJECT(VX_ZONE_INFO, node_obj_desc, "Node (node=%d, pipe=%d) ... BLOCKED for resources on target %08x\n",
                                 node_obj_desc->base.obj_desc_id,
                                 node_obj_desc->pipeline_id,
                                 target->target_id
                           ); /* TIOVX-1967- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR046 */
/* LDRA_JUSTIFY_END */
            }
        }
        else
        {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR047
<justification end> */
            VX_PRINT_OBJECT(VX_ZONE_INFO, node_obj_desc, "Node (node=%d, pipe=%d) ... BLOCKED for previous pipe instance node (node=%d) to complete !!!\n",
                    node_obj_desc->base.obj_desc_id,
                    node_obj_desc->pipeline_id,
                    node_obj_desc->prev_pipe_node_id
            ); /* TIOVX-1967- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR047 */
/* LDRA_JUSTIFY_END */
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
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR023
<justification end> */
    if(kernel_name_obj_desc!=NULL) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR023 */
    {
        kernel_name = kernel_name_obj_desc->kernel_name;
    }
/* LDRA_JUSTIFY_END */
    for (cnt = 0; (cnt < loop_max) && (status == (vx_status)VX_SUCCESS); cnt ++)
    {
        target_kernel_instance = ownTargetKernelInstanceAlloc(
            (vx_enum)node_obj_desc->kernel_id, kernel_name, (vx_enum)node_obj_desc->target_id);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM011
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM011 */
        if(target_kernel_instance == NULL)
        {
            VX_PRINT_OBJECT(VX_ZONE_ERROR, node_obj_desc, "target_kernel_instance is NULL\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM011
<justification end> */
        else
/* LDRA_JUSTIFY_END */
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
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR024
<justification end> */
                    if(prm_obj_desc != NULL) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR024 */
                    {
                        parent_obj_desc[i] = ownObjDescGet(
                            prm_obj_desc->scope_obj_desc_id);
                    }
/* LDRA_JUSTIFY_END */
                }
            }

            for(i=0; i<node_obj_desc->num_params ; i++)
            {
                params[i] = NULL;
                if((is_prm_replicated & ((uint32_t)1U << i)) != 0U)
                {
                    if(parent_obj_desc[i] != NULL) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR025 */
                    {
                        if((vx_enum)parent_obj_desc[i]->type==(vx_enum)TIVX_OBJ_DESC_OBJARRAY)
                        {
                            params[i] = ownObjDescGet(
                                ((tivx_obj_desc_object_array_t*)parent_obj_desc[i])->
                                    obj_desc_id[cnt]);
                        }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR026
<justification end> */
                        else
                        if((vx_enum)parent_obj_desc[i]->type==(vx_enum)TIVX_OBJ_DESC_PYRAMID) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR026 */
/* LDRA_JUSTIFY_END */
                        {
                            params[i] = ownObjDescGet(
                                ((tivx_obj_desc_pyramid_t*)parent_obj_desc[i])->
                                    obj_desc_id[cnt]);
                        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM012
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM012 */
                        else
                        {
                            params[i] = NULL;
                        }
/* LDRA_JUSTIFY_END */
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

            #if defined(BUILD_BAM)
            if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_SUPERNODE) ==
                (vx_bool)vx_true_e)
            {
                params[0] = (tivx_obj_desc_t *) ownObjDescGet( node_obj_desc->base.scope_obj_desc_id );

                status = ownTargetKernelCreate(target_kernel_instance,
                    params, 1);
            }
            else
            #endif
            {
                status = ownTargetKernelCreate(target_kernel_instance,
                    params, (uint16_t)node_obj_desc->num_params);
            }

            if(status!=(vx_status)VX_SUCCESS)
            {
                /* error status check is not required
                 * as it is already done in the previous status check
                 * of ownTargetKernelCreate
                 */
                (void)ownTargetKernelInstanceFree(&target_kernel_instance);
            }
        }
    }

    if ((vx_status)VX_SUCCESS != status)
    {
        for (i = 0; i < cnt; i ++)
        {
            target_kernel_instance = ownTargetKernelInstanceGet(
                (uint16_t)node_obj_desc->target_kernel_index[i], (vx_enum)node_obj_desc->kernel_id);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM013
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM013 */
            if (NULL != target_kernel_instance)
            {
                /* status check is not required
                 * as the NULL check of target kernel instance
                 * is already confirmed in previous check
                 */
                (void)ownTargetKernelInstanceFree(&target_kernel_instance);
            }
/* LDRA_JUSTIFY_END */
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

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM014
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM014 */
        if(target_kernel_instance == NULL)
        {
            VX_PRINT_OBJECT(VX_ZONE_ERROR, node_obj_desc, "target_kernel_instance is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM014
<justification end> */
        else
/* LDRA_JUSTIFY_END */
        {
            {
                /* NOTE: nothing special for replicated node during
                         create/delete */
                for(i=0; i<node_obj_desc->num_params ; i++)
                {
                    params[i] = ownObjDescGet(node_obj_desc->data_id[i]);
                }

                #if defined(BUILD_BAM)
                if (tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_SUPERNODE) ==
                    (vx_bool)vx_true_e)
                {
                    params[0] = (tivx_obj_desc_t *) ownObjDescGet( node_obj_desc->base.scope_obj_desc_id );

                    tivxCheckStatus(&status, ownTargetKernelDelete(target_kernel_instance, params, 1));
                }
                else
                #endif
                {
                    tivxCheckStatus(&status, ownTargetKernelDelete(target_kernel_instance,
                        params, (uint16_t)node_obj_desc->num_params));
                }
            }
            /* error status check is not required here
             * as it is checked before in tivxCheckStatus
             */
            (void)ownTargetKernelInstanceFree(&target_kernel_instance);
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

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM001
<justification end>*/
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM001 */
    if(target_kernel_instance == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "target_kernel_instance is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM001
<justification end>*/
    else
/* LDRA_JUSTIFY_END */
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
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR027
<justification end>*/
        if ((vx_enum)TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES ==
            cmd_obj_desc->replicated_node_idx) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR027 */
/* LDRA_JUSTIFY_END */
        {
            for (cnt = 0; cnt < loop_max; cnt ++)
            {
                status = ownTargetNodeSendCommand(cmd_obj_desc, cnt,
                    node_obj_desc);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM017
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM017 */
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT_OBJECT(VX_ZONE_ERROR, node_obj_desc, "SendCommand Failed\n");
                    break;
                }
/* LDRA_JUSTIFY_END */
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM007
<justification end>*/
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM007 */
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
                    VX_PRINT_OBJECT(VX_ZONE_ERROR, node_obj_desc, "SendCommand Failed\n");
                }
            }
            else
            {
                VX_PRINT_OBJECT(VX_ZONE_ERROR, node_obj_desc, "Incorrect node id\n");
                status = (vx_status)VX_FAILURE;
            }
        }
/* LDRA_JUSTIFY_END */
    }
    else
    {
        /* For non-replicated node, ignore node-id field */
        status = ownTargetNodeSendCommand(cmd_obj_desc, 0U, node_obj_desc);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM018
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM018 */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT_OBJECT(VX_ZONE_ERROR, node_obj_desc, "SendCommand Failed\n");
        }
/* LDRA_JUSTIFY_END */
    }

    return status;
}

#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> static void ownTargetCmdDescHandleAck.* <function end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM001
<justification end> */
#endif
/* TIOVX-1671- Host only Id: TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM001 */
static void ownTargetCmdDescHandleAck(tivx_obj_desc_cmd_t *cmd_obj_desc)
{
#if defined(A72) || defined(A53)
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR028
<justification end> */
#endif
    if( tivxFlagIsBitSet( cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK) != 0) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR028 */
    {
        tivxFlagBitClear( &cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK);

       /* error status check is not done
        * as the return status value is not used
        * ownTargetCmdDescHandleAck further
        */
       (void)tivxEventPost((tivx_event)(uintptr_t)cmd_obj_desc->ack_event_handle);
    }
#if defined(A72) || defined(A53)
/* LDRA_JUSTIFY_END */
#endif
}

static void ownTargetCmdDescSendAck(tivx_obj_desc_cmd_t *cmd_obj_desc, vx_status status)
{
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR029
<justification end> */
    if( tivxFlagIsBitSet( cmd_obj_desc->flags, TIVX_CMD_FLAG_SEND_ACK) != 0) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR029 */
    {
        tivxFlagBitSet( &cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK);

        cmd_obj_desc->cmd_status = (uint32_t)status;

       /* error status check is not done
        * as the returned status value is not used
        * in ownTargetCmdDescSendAck further
        */
       (void)ownObjDescSend(cmd_obj_desc->src_target_id, cmd_obj_desc->base.obj_desc_id);
    }
/* LDRA_JUSTIFY_END */
}

static void ownTargetCmdDescHandler(tivx_obj_desc_cmd_t *cmd_obj_desc)
{
    uint16_t node_obj_desc_id;
    tivx_obj_desc_node_t *node_obj_desc;
    vx_status status = (vx_status)VX_SUCCESS;

    switch(cmd_obj_desc->cmd_id) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR030 */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR030
<justification end> */
    {
/* LDRA_JUSTIFY_END */
        case (vx_enum)TIVX_CMD_NODE_CREATE:
        case (vx_enum)TIVX_CMD_NODE_DELETE:
        case (vx_enum)TIVX_CMD_NODE_CONTROL:
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR005
<justification end> */
            if( tivxFlagIsBitSet( cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK) == (vx_bool)vx_false_e ) /* TIOVX-1967- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR005 */
/* LDRA_JUSTIFY_END */
            {
                node_obj_desc_id = cmd_obj_desc->obj_desc_id[0];
                node_obj_desc = (tivx_obj_desc_node_t*)ownObjDescGet(node_obj_desc_id);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR031
<justification end> */
                if( ownObjDescIsValidType( (tivx_obj_desc_t*)node_obj_desc, TIVX_OBJ_DESC_NODE) != 0) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR031 */
/* LDRA_JUSTIFY_END */
                {
                    if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_TARGET_KERNEL) != 0) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR032 */
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
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR033
<justification end> */
                        else
                        if((vx_enum)cmd_obj_desc->cmd_id == (vx_enum)TIVX_CMD_NODE_CONTROL) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR033 */
/* LDRA_JUSTIFY_END */
                        {
                            status = ownTargetNodeDescNodeControl(cmd_obj_desc, node_obj_desc);
                        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM008
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM008 */
                        else
                        {
                            /* do nothing */
                        }
/* LDRA_JUSTIFY_END */
                    }
                }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM009
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM009 */
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "object descriptor type is invalid\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
/* LDRA_JUSTIFY_END */
                ownTargetCmdDescSendAck(cmd_obj_desc, status);
            }
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM001
<justification end> */
#endif
/* TIOVX-1671- Host only Id: TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM001 */
            else
            {
                /* this is ACK for a previously sent command */
                ownTargetCmdDescHandleAck(cmd_obj_desc);
            }
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif
            break;
#ifdef HOST_ONLY
/* TIOVX-1671- Host only Id: TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM008 */
        /* TIVX_CMD_NODE_USER_CALLBACK command is initated by the command sent from target via: \ref ownTargetNodeDescSendComplete()
           It is always executed on HOST target to check if there is any user callback, and if so executes it, otherwise, does nothing */
        case (vx_enum)TIVX_CMD_NODE_USER_CALLBACK:
        /* TIVX_CMD_DATA_REF_CONSUMED command is initated by the command sent from target via: \ref ownTargetNodeDescReleaseAllParameters()
           if the data reference consumed event flag is enabled for a given data reference. It is always executed on HOST target */
        case (vx_enum)TIVX_CMD_DATA_REF_CONSUMED:
            /* These 2 commands are only executed on the "HOST" target, therefore using function pointer that is registered
               via ownRegisterFunctionsForHost(), which is called from tivxHostInit function, to avoid linking of symbols not needed
               on non-host CPUs */
#if defined(A72) || defined(A53)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR034
<justification end> */
#endif
            if(NULL != g_target_cmd_desc_handler_for_host_f) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR034 */
            {
                g_target_cmd_desc_handler_for_host_f(cmd_obj_desc);
            }
#if defined(A72) || defined(A53)
/* LDRA_JUSTIFY_END */
#endif
            break;
#endif
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM015
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM015 */
        default:
            break;
/* LDRA_JUSTIFY_END */
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

    while(
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR006
<justification end> */
        target->targetExitRequest == (vx_bool)vx_false_e) /* TIOVX-1967- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR006 */
/* LDRA_JUSTIFY_END */
    {
        status = ownTargetDequeueObjDesc(target,
                    &obj_desc_id, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);
#ifdef HOST_ONLY
/* TIOVX-1671- Host only Id: TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM009 */
        if(    (status != (vx_status)VX_SUCCESS) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR035 */
            || ((vx_enum)obj_desc_id == (vx_enum)TIVX_OBJ_DESC_INVALID) )
        {
            /* in case of error, do nothing,
             * if target exit was requested, while(...) condition check with
             * check and exit
             */
        }
        else
#endif
        {
            obj_desc = ownObjDescGet(obj_desc_id);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM010
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM010 */
            if(obj_desc == NULL)
            {
                /* in valid obj_desc_id received */
            }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM010
<justification end> */
            else
/* LDRA_JUSTIFY_END */
            {
                ownLogRtTraceTargetExeStart(target, obj_desc);

                switch(obj_desc->type) /* TIOVX-1967- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR007 */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_RTOS_TIVX_TARGET_UBR007
<justification end> */
                {
/* LDRA_JUSTIFY_END */
                    case (vx_enum)TIVX_OBJ_DESC_CMD:
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR036
<justification end> */
                        if( ownObjDescIsValidType( obj_desc, TIVX_OBJ_DESC_CMD) != 0) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR036 */
                        {
                            ownTargetCmdDescHandler((tivx_obj_desc_cmd_t*)obj_desc);
                        }
/* LDRA_JUSTIFY_END */
                        break;
                    case (vx_enum)TIVX_OBJ_DESC_NODE:
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR037
<justification end> */
                        if( ownObjDescIsValidType( obj_desc, TIVX_OBJ_DESC_NODE) != 0) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR037 */
                        {
                            ownTargetNodeDescNodeExecute(target, (tivx_obj_desc_node_t*)obj_desc);
                        }
/* LDRA_JUSTIFY_END */
                        break;
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM006
<justification end> */
#endif
/* TIOVX-1671- Host only Id: TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM006 */
                    default:
                        /* unsupported obj_desc received at target */
                        break;
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif
                }

                ownLogRtTraceTargetExeEnd(target, obj_desc);
            }
        }
    }

#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> statement <metric end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM010
<justification end> */
#endif
/* TIOVX-1671- Host only Id: TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM010 */
    target->targetExitDone = (vx_bool)vx_true_e;
}
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif

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
        (void)snprintf(target->task_params.task_name, TIVX_TARGET_MAX_TASK_NAME, "%s", params->task_name);
        target->task_params.task_name[TIVX_TARGET_MAX_TASK_NAME-1U] = '\0';

        target->targetExitRequest = (vx_bool)vx_false_e;
        target->targetExitDone = (vx_bool)vx_false_e;

        /* create job queue */
        status = tivxQueueCreate(&target->job_queue_handle,
                        TIVX_TARGET_MAX_JOB_QUEUE_DEPTH,
                        target->job_queue_memory,
                        TIVX_QUEUE_FLAG_BLOCK_ON_GET);

        if(status == (vx_status)VX_SUCCESS) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR038 */
        {
            /* create and start target task */
            status = tivxTaskCreate(&target->task_handle, &target->task_params);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM002
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM002 */
            if(status != (vx_status)VX_SUCCESS)
            {
                /* error status check is not done
                 * as the status check is covered in previous check
                 * in tivxQueueCreate
                 */
                (void)tivxQueueDelete(&target->job_queue_handle);
            }
/* LDRA_JUSTIFY_END */
        }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM003
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM003 */
        if (status != (vx_status)VX_SUCCESS)
        {
#ifdef HOST_ONLY
            ownTargetFreeHandle(&target);
#endif
        }
/* LDRA_JUSTIFY_END */
    }
    return status;
}

vx_status ownTargetDelete(vx_enum target_id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_target target;

    target = ownTargetGetHandle(target_id);

    /* delete task */
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM003
<justification end> */
#endif
/* TIOVX-1671- Host only Id: TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM003 */
    if (NULL != target)
    {
        /* set flag to break target from main loop */
        target->targetExitRequest = (vx_bool)vx_true_e;

        /* queue a invalid object descriptor to unblock queue wait */
        status = ownTargetQueueObjDesc(target_id, (vx_enum)TIVX_OBJ_DESC_INVALID);

        if((vx_status)VX_SUCCESS == status) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR039 */
        {
            /* wait until target exit is done */
            while(target->targetExitDone==(vx_bool)vx_false_e)
            {
                    tivxTaskWaitMsecs(1);
            }
            /* error status is not required
             * as it found to return always true
             */
            (void)tivxTaskDelete(&target->task_handle);

            /* delete job queue */
            /* error check not required as it is already
             * handled in ownTargetQueueObjDesc status check
             */
            (void)tivxQueueDelete(&target->job_queue_handle);

            ownTargetFreeHandle(&target);
        }
    }
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM003
<justification end> */
#endif
    else
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif
    {
        VX_PRINT(VX_ZONE_ERROR, "Target delete failed due to invalid target ID\n");
        status = (vx_status)VX_ERROR_INVALID_VALUE;
    }

    return status;
}

void ownTargetTriggerNode(uint16_t node_obj_desc_id)
{
    tivx_obj_desc_node_t *node_obj_desc;

    node_obj_desc = (tivx_obj_desc_node_t*)ownObjDescGet(node_obj_desc_id);

    if( ownObjDescIsValidType( (tivx_obj_desc_t*)node_obj_desc, TIVX_OBJ_DESC_NODE) != 0)
    {
       /* status check is not required
        * as the return status is not used here
        */
       (void)ownObjDescSend( node_obj_desc->target_id, node_obj_desc_id);
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

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_UM004
<justification end> */
/* TIOVX-1671- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_UM004 */
        if((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"***************************************************************************************************\n");
            VX_PRINT(VX_ZONE_ERROR,"FATAL ERROR: tivxQueuePut failed\n");
            VX_PRINT(VX_ZONE_ERROR,"May need to increase the value of TIVX_TARGET_MAX_JOB_QUEUE_DEPTH in tiovx/include/TI/tivx_config.h\n");
            VX_PRINT(VX_ZONE_ERROR,"***************************************************************************************************\n");
        }
/* LDRA_JUSTIFY_END */
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
    (void)strncpy(params->task_name, "TIVX_TARGET", TIVX_MAX_TASK_NAME);
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
    tivx_target_t *tmp_target_table = &g_target_table[0][0];

    for(i=0; i<dimof(g_target_table); i++)
    {
        tmp_target_table->target_id = (vx_enum)TIVX_TARGET_ID_INVALID;
        tmp_target_table++;
    }
    /* error status check is not done
     * as the returned status value is not used
     * in ownTargetInit further
     */
    (void)ownTargetKernelInit();
    /* error status check is not done
     * as the returned status value is not used
     * in ownTargetInit further
     */
    (void)ownTargetKernelInstanceInit();
}

#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> statement <metric end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM004
<justification end> */
/* TIOVX-1671- Host only Id: TIOVX_CODE_COVERAGE_HOST_ONLY_TARGET_UM004 */
#endif
void ownTargetDeInit(void)
{
    ownTargetKernelInstanceDeInit();
    ownTargetKernelDeInit();

}
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif

void ownTargetSetTimestamp(
    const tivx_obj_desc_node_t *node_obj_desc, tivx_obj_desc_t *obj_desc[])
{
    uint16_t prm_id;
    uint64_t timestamp = 0, obj_timestamp = 0;
    uint32_t is_prm_input_flag, is_prm_bidi_flag;
    tivx_obj_desc_t *parent_obj_desc;

    is_prm_input_flag = node_obj_desc->is_prm_input;
    is_prm_bidi_flag  = node_obj_desc->is_prm_bidi;

    /* Reading all input timestamps, taking the most recent of the timestamps to pass along */
    for (prm_id = 0U; prm_id < node_obj_desc->num_params; prm_id++)
    {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR040
<justification end> */
        if (NULL != obj_desc[prm_id]) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR040 */
        {
           if ((tivxFlagIsBitSet(is_prm_input_flag, ((uint32_t)1U<<prm_id))==(vx_bool)vx_true_e) ||
                (tivxFlagIsBitSet(is_prm_bidi_flag, ((uint32_t)1U<<prm_id))==(vx_bool)vx_true_e))
            {
                obj_timestamp = obj_desc[prm_id]->timestamp;

                if (obj_timestamp > timestamp)
                {
                    timestamp = obj_timestamp;
                }
            }
        }
/* LDRA_JUSTIFY_END */
    }

    /* Setting all outputs to use most recent of the timestamps */
    for (prm_id = 0U; prm_id < node_obj_desc->num_params; prm_id++)
    {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR041
<justification end> */
        if (NULL != obj_desc[prm_id]) /* TIOVX-1930- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TARGET_UBR041 */
        {
            if (tivxFlagIsBitSet(is_prm_input_flag, ((uint32_t)1U<<prm_id)) == (vx_bool)vx_false_e)
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
/* LDRA_JUSTIFY_END */
    }
}
