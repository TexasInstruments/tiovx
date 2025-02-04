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

extern own_execute_user_kernel_f      g_executeUserKernel_f;
extern own_target_cmd_desc_handler_f  g_target_cmd_desc_handler_for_host_f;

/* These two are assigned to the function pointers above */
static void ownTargetNodeDescNodeExecuteUserKernel(tivx_obj_desc_node_t *node_obj_desc, uint16_t prm_obj_desc_id[]);
static void ownTargetCmdDescHandlerHost(const tivx_obj_desc_cmd_t *cmd_obj_desc);

/* These two are called by \ref ownTargetCmdDescHandlerHost */
static vx_action ownTargetCmdDescHandleUserCallback(tivx_obj_desc_node_t *node_obj_desc, uint64_t timestamp);
static void ownTargetSetGraphStateAbandon(const tivx_obj_desc_node_t *node_obj_desc);


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

static void ownTargetCmdDescHandlerHost(const tivx_obj_desc_cmd_t *cmd_obj_desc)
{
    uint16_t node_obj_desc_id;
    tivx_obj_desc_node_t *node_obj_desc;

    switch(cmd_obj_desc->cmd_id) /* TIOVX-1937- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_ON_HOST_CPU_UBR003 */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_ON_HOST_CPU_UBR003
<justification end>*/
    {
/* LDRA_JUSTIFY_END */
        case (vx_enum)TIVX_CMD_NODE_USER_CALLBACK:
            node_obj_desc_id = cmd_obj_desc->obj_desc_id[0];
            node_obj_desc = (tivx_obj_desc_node_t*)ownObjDescGet(node_obj_desc_id);

            if( ownObjDescIsValidType( (tivx_obj_desc_t*)node_obj_desc, TIVX_OBJ_DESC_NODE) != 0) /* TIOVX-1937- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_ON_HOST_CPU_UBR001 */
            {
                uint64_t timestamp;
                vx_action action;

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
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_ON_HOST_CPU_UBR002
<justification end>*/
            if( data_ref_q != NULL ) /* TIOVX-1937- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_ON_HOST_CPU_UBR002 */
            {
                uint64_t timestamp;

                tivx_uint32_to_uint64(&timestamp, cmd_obj_desc->timestamp_h, cmd_obj_desc->timestamp_l);

                (void)ownDataRefQueueSendRefConsumedEvent(data_ref_q, timestamp);
            }
/* LDRA_JUSTIFY_END */
            /* No ack for this command */
        }
            break;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_ON_HOST_CPU_UM001
<justification end>*/
/* TIOVX-1746- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_ON_HOST_CPU_UM001 */
        default:
            break;
/* LDRA_JUSTIY_END */
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


void ownRegisterFunctionsForHost(void)
{
    g_executeUserKernel_f = (own_execute_user_kernel_f)ownTargetNodeDescNodeExecuteUserKernel;
    g_target_cmd_desc_handler_for_host_f = (own_target_cmd_desc_handler_f)ownTargetCmdDescHandlerHost;
}
