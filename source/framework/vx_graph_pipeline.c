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


static tivx_obj_desc_graph_t *ownGraphDequeueFreeObjDesc(vx_graph graph);
static void ownGraphEnqueueFreeObjDesc(vx_graph graph,
                        const tivx_obj_desc_graph_t *obj_desc);
static tivx_obj_desc_graph_t *ownGraphGetObjDesc(vx_graph graph, uint32_t pipeline_id);
static vx_status ownGraphPipelineValidateRefsList(
                     const vx_graph_parameter_queue_params_t graph_parameters_queue_param);
static vx_status ownGraphParameterEnqueueReadyRef(vx_graph graph,
                vx_uint32 graph_parameter_index,
                vx_reference *refs,
                vx_uint32 num_refs);

static vx_status ownDecrementEnqueueCount(vx_reference ref);

static vx_status ownGraphPipelineValidateRefsList(
    const vx_graph_parameter_queue_params_t graph_parameters_queue_param)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status status1 = (vx_status)VX_SUCCESS;
    vx_meta_format meta_base = NULL, meta = NULL;
    vx_uint32 i;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR001
<justification end> */
    if (NULL != graph_parameters_queue_param.refs_list[0]) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR001 */
    {
        meta_base = ownCreateMetaFormat(graph_parameters_queue_param.refs_list[0]->context);
        status = vxSetMetaFormatFromReference(meta_base, graph_parameters_queue_param.refs_list[0]);
    }

/* LDRA_JUSTIFY_END */
    if ( ((vx_status)VX_SUCCESS == status) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR002 */
    && (NULL != meta_base) ) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR003 */
    {
        for (i = 1; i < graph_parameters_queue_param.refs_list_size; i++)
        {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR004
<justification end> */
            if (NULL != graph_parameters_queue_param.refs_list[i]) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR004 */
/* LDRA_JUSTIFY_END */
            {
                meta = ownCreateMetaFormat(graph_parameters_queue_param.refs_list[i]->context);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR005
<justification end> */
                if (NULL != meta) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR005 */
/* LDRA_JUSTIFY_END */
                {
                    status = vxSetMetaFormatFromReference(meta, graph_parameters_queue_param.refs_list[i]);
                }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PIPELINE_UM001
<justification end> */
/* TIOVX-1720- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_PIPELINE_UM001 */
                else
                {
                    status = (vx_status)VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR, "Meta Format is NULL\n");
                }
/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR006
<justification end> */
                if ((vx_status)VX_SUCCESS == status) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR006 */
/* LDRA_JUSTIFY_END */
                {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR007
<justification end> */
                    if (graph_parameters_queue_param.refs_list[0]->type ==
                        graph_parameters_queue_param.refs_list[i]->type) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR007 */
                    {
                        if ((vx_bool)vx_true_e != ownIsMetaFormatEqual(meta_base, meta, graph_parameters_queue_param.refs_list[0]->type))
                        {
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                            VX_PRINT(VX_ZONE_ERROR, "Invalid meta data of reference list!\n");
                        }
                    }
/* LDRA_JUSTIFY_END */
                }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PIPELINE_UM002
<justification end> */
/* TIOVX-1720- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_PIPELINE_UM002 */
                else
                {
                    break;
                }
/* LDRA_JUSTIFY_END */

                if (ownIsValidSpecificReference(vxCastRefFromMetaFormat(meta), (vx_enum)VX_TYPE_META_FORMAT) == 
                                                (vx_bool)vx_true_e) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR008 */
                {
                    status1 = ownReleaseMetaFormat(&meta);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PIPELINE_UM003
<justification end> */
/* TIOVX-1720- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_PIPELINE_UM003 */
                    if((vx_status)VX_SUCCESS != status1)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Failed to release meta format object \n");
                        status = status1;
                    }
/* LDRA_JUSTIFY_END */
                }
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PIPELINE_UM004
<justification end> */
/* TIOVX-1720- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_PIPELINE_UM004 */
            else
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Invalid graph parameter ref list!\n");
            }
/* LDRA_JUSTIFY_END */
        }
    }

    if (ownIsValidSpecificReference(vxCastRefFromMetaFormat(meta_base), (vx_enum)VX_TYPE_META_FORMAT) == 
                                    (vx_bool)vx_true_e) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR009 */
    {
        status1 = ownReleaseMetaFormat(&meta_base);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PIPELINE_UM005
<justification end> */
/* TIOVX-1720- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_PIPELINE_UM005 */
        if((vx_status)VX_SUCCESS != status1)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to release meta format object \n");
            status = status1;
        }
/* LDRA_JUSTIFY_END */
    }

    return status;
}

static vx_status ownDecrementEnqueueCount(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    /* we don't need to check as the reference has previously been checked for NULL*/
    ref->obj_desc->flags &= ~TIVX_OBJ_DESC_DATA_REF_GRAPH_PARAM_ENQUEUED;
    if (ref->obj_desc->num_enqueues > 0U)
    {
        ref->obj_desc->num_enqueues --;
        status = (vx_status)VX_SUCCESS;
    }
    else if (NULL == ref->delay)
    {
        VX_PRINT(VX_ZONE_ERROR, "Reference enqueue count underflow!, ref=%p, type=%d, name=%s \n", ref, ref->type, ref->name);
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        /* for a delay, ignore the count if it was already zero, this is the case for pipelining */
        status = (vx_status)VX_SUCCESS;
    }
    return status;
}

VX_API_ENTRY vx_status vxSetGraphScheduleConfig(
    vx_graph graph,
    vx_enum graph_schedule_mode,
    vx_uint32 graph_parameters_list_size,
    const vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[]
    )
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        if (graph->verified == (vx_bool)vx_true_e)
        {
            VX_PRINT(VX_ZONE_ERROR, "Not supported on verified graph\n");
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            if(graph_schedule_mode==(vx_enum)VX_GRAPH_SCHEDULE_MODE_NORMAL)
            {
                graph->schedule_mode = graph_schedule_mode;
            }
            else
            if( (   (graph_schedule_mode==(vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO)
                 || (graph_schedule_mode==(vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL)
                )
              && (graph_parameters_list_size <= graph->num_params)
              )
            {
                uint32_t i;

                graph->schedule_mode = graph_schedule_mode;

                /* Pipelining is enabled */
                graph->is_pipelining_enabled = (vx_bool)vx_true_e;

                for(i=0; (i<graph_parameters_list_size) && (status == (vx_status)VX_SUCCESS); i++)
                {
                    if(graph_parameters_queue_params_list[i].refs_list!=NULL)
                    {
                        if((graph_parameters_queue_params_list[i].graph_parameter_index
                            >= graph->num_params)
                            ||
                            (graph_parameters_queue_params_list[i].refs_list_size >= TIVX_OBJ_DESC_QUEUE_MAX_DEPTH)
                            )
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Invalid parameters\n");
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        }
                        else
                        {
                            graph->parameters[i].queue_enable = (vx_bool)vx_true_e;
                            graph->parameters[i].num_buf = graph_parameters_queue_params_list[i].refs_list_size;
                            graph->parameters[i].type = graph_parameters_queue_params_list[i].refs_list[0]->type;

                            status = ownGraphPipelineValidateRefsList(graph_parameters_queue_params_list[i]);

                            if ((vx_status)VX_SUCCESS == status)
                            {
                                uint32_t buf_id;

                                for(buf_id=0; buf_id<graph->parameters[i].num_buf; buf_id++)
                                {
                                    graph->parameters[i].refs_list[buf_id] = graph_parameters_queue_params_list[i].refs_list[buf_id];
                                }
                            }
                            else
                            {
                                VX_PRINT(VX_ZONE_ERROR,
                                    "Graph parameter refs list at index %d contains inconsistent meta data. Please ensure that all buffers in list contain the same meta data\n", i);
                            }
                        }
                    }
                    else
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters: graph_parameters_queue_params_list at index %d is NULL\n", i);
                        break;
                    }
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "user parameter list (%d) > number of graph parameters (%d)\n",
                    graph_parameters_list_size,
                    graph->num_params
                    );
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownGraphParameterEnqueueReadyRef(vx_graph graph,
                vx_uint32 graph_parameter_index,
                vx_reference *refs,
                vx_uint32 num_refs)
{
    tivx_data_ref_queue data_ref_q = NULL;
    vx_status status = (vx_status)VX_SUCCESS;

    /* get data ref queue associated with a graph parameter
     * if this graph parameter is not enabled in queuing mode,
     * then a NULL data_ref_q is returned
     */
    data_ref_q = ownGraphGetParameterDataRefQueue(graph, graph_parameter_index);
    if(data_ref_q == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "Reference enqueue not supported at graph parameter index %d\n", graph_parameter_index);
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        vx_uint32 ref_id;
        vx_uint32 num_enqueue = 0;

        for(ref_id=0; (ref_id < num_refs) && ((vx_status)VX_SUCCESS == status); ref_id++)
        {
            vx_reference ref = refs[ref_id];
            status = (vx_status)VX_FAILURE;
            if (NULL != ref)
            {
                const tivx_obj_desc_node_t *nobj = graph->parameters[graph_parameter_index].node->obj_desc[0];
                vx_bool is_input = tivxFlagIsBitSet(nobj->is_prm_input, ((uint32_t)1U<<graph->parameters[graph_parameter_index].index));
                vx_bool is_replicated = tivxFlagIsBitSet(nobj->is_prm_replicated, ((uint32_t)1U<<graph->parameters[graph_parameter_index].index));
                vx_uint32 num_replicas = ((vx_bool)vx_true_e == is_replicated) ? nobj->num_of_replicas : 0U;
                    /* Reference is valid if all these conditions are true:
                    - the reference is on the list of valid references for this parameter
                    - the parameter is an input and the reference isn't queued on an output or bidirectional somewhere
                    - the parameter is an output or bidirectional and the reference isn't queued anywhere else
                    We keep track of the number of times a reference is queued using ref->obj_desc->num_enqueues,
                    and set TIVX_OBJ_DESC_DATA_REF_GRAPH_PARAM_ENQUEUED when a reference is enqueued on an output or bidirectional.

                    Notice that for replicated parameters we have to check *all* the references in case part of an object array or
                    pyramid has been queued elsewhere; and then we have to increment the counter and set the flag for *all*
                    references in case part of the object may be queued elsewhere. Thus for an object array where the first element
                    is used for a replicated parameter, all elements *and* the the object array itself must be marked.
                */
                uint32_t buf_id;
                for(buf_id=0; buf_id < graph->parameters[graph_parameter_index].num_buf; buf_id++)
                {
                    if (ref == graph->parameters[graph_parameter_index].refs_list[buf_id])
                    {
                        ownPlatformSystemLock((vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);
                        tivx_obj_desc_t *objd = ref->obj_desc;
                        vx_reference const * ref_list = NULL;
                        vx_bool can_be_queued = (vx_bool)vx_true_e;
                        if (is_replicated == (vx_bool)vx_true_e)
                        {
                            objd = ref->scope->obj_desc;
                           if (ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
                            {
                                vx_object_array object_array = vxCastRefAsObjectArray(ref->scope, NULL);
                                ref_list = object_array->ref;
                            }
                            else if (ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
                            {
                                vx_pyramid pyramid = vxCastRefAsPyramid(ref->scope, NULL);
                                ref_list = (vx_reference *)(uintptr_t)(pyramid->img);                            
                            }
                            else
                            {
                                /* Coverage: Should not happen because replication is only possible with obj_arr,
                                concerning the pyramid: This is the only possible in combination with obj_arr 
                                but if it does, complain! */
                                can_be_queued = (vx_bool)vx_false_e;
                                VX_PRINT(VX_ZONE_ERROR, "Found a scope (%d) that was not object array or pyramid!\n", ref->scope->type);
                            }
                        }
                        if ((tivxFlagIsBitSet(objd->flags, TIVX_OBJ_DESC_DATA_REF_GRAPH_PARAM_ENQUEUED) == (vx_bool)vx_true_e) ||
                            ( ((vx_bool)vx_false_e == is_input) && (objd->num_enqueues > 0U)))
                        {
                            can_be_queued = (vx_bool)vx_false_e;
                        }
                        else if (NULL != ref_list)
                        {
                            vx_uint32 i;
                            for (i = 0; i < num_replicas; ++i)
                            {
                                /* loop over the replicats object descriptor */
                                tivx_obj_desc_t const * odi = ref_list[i]->obj_desc;
                                if (NULL == odi)
                                {
                                    VX_PRINT(VX_ZONE_ERROR, "Could not get object descriptor from list!\n");
                                    can_be_queued = (vx_bool)vx_false_e;
                                }
                                else if ((tivxFlagIsBitSet(odi->flags, TIVX_OBJ_DESC_DATA_REF_GRAPH_PARAM_ENQUEUED) == (vx_bool)vx_true_e) ||
                                    (((vx_bool)vx_false_e == is_input) && (odi->num_enqueues > 0U)))
                                {
                                    can_be_queued = (vx_bool)vx_false_e;
                                    break;
                                } else { /* do nothing */ }
                            }
                        } else { /* do nothing */ }
                        if ((vx_bool)vx_false_e == can_be_queued)
                        {
                            ownPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);
                            VX_PRINT(VX_ZONE_ERROR, "ref %p can't be queued. obj_desc_id=%d, param=%d(%s), #enqueues=%d\n",
                                                    ref, ref->obj_desc->obj_desc_id, graph_parameter_index, is_input? "input":"output", objd->num_enqueues);
                        }
                        else
                        {
                            tivx_obj_desc_queue_blocked_nodes_t blocked_nodes;

                            /* get queue object descriptor */
                            uint16_t queue_obj_desc_id = data_ref_q->ready_q_obj_desc_id;
                            /* get reference object descriptor */
                            uint16_t ref_obj_desc_id = ref->obj_desc->obj_desc_id;

                            status = ownObjDescQueueEnqueue(queue_obj_desc_id, ref_obj_desc_id);
                            /* coverage: Similar to TI deviation, negative use case cannot be tested */
                            if(status==(vx_status)VX_SUCCESS)
                            {
                                blocked_nodes.num_nodes = 0;
                                /* if any node is blocked on ref enqueued to this queue, then get the list of blocked nodes */
                                (void)ownObjDescQueueExtractBlockedNodes(queue_obj_desc_id, &blocked_nodes);
                                /* if this parameter is on a replicated node, we need to increment the queue counter
                                   and set the enqueued flag for all elements of the corresponding container */
                                /* Increment the queue counter */
                                objd->num_enqueues ++;
                                if ((vx_bool)vx_false_e == is_input)
                                {
                                    /* not an input, it's output or bidirectional so set the exclusive use flag */
                                    objd->flags |= TIVX_OBJ_DESC_DATA_REF_GRAPH_PARAM_ENQUEUED;
                                }
                                if (NULL != ref_list)
                                {
                                    vx_uint32 i;
                                    for (i = 0; i < num_replicas; ++i)
                                    {
                                        /* loop over the replicats object descriptor */
                                        tivx_obj_desc_t *odi = ref_list[i]->obj_desc;
                                        /* Increment the queue counter */
                                        odi->num_enqueues ++;
                                        if ((vx_bool)vx_false_e == is_input)
                                        {
                                            /* not an input, it's output or bidirectional so set the exclusive use flag */
                                            odi->flags |= TIVX_OBJ_DESC_DATA_REF_GRAPH_PARAM_ENQUEUED;
                                        }
                                    }
                                }
                                num_enqueue++;
                            }

                            ownPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);

                            VX_PRINT(VX_ZONE_INFO, "Q (queue=%d, ref=%d)\n", queue_obj_desc_id, ref_obj_desc_id);
                            
                            if(status==(vx_status)VX_SUCCESS) /* TIOVX-1813- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_PIPELINE_UTJT001 */
                            {
                                uint32_t node_id;
                                /* re-trigger blocked nodes */
                                for(node_id=0; node_id<blocked_nodes.num_nodes; node_id++)
                                {
                                    VX_PRINT(VX_ZONE_INFO,"Re-triggering (node=%d)\n", blocked_nodes.node_id[node_id]);
                                    ownTargetTriggerNode(blocked_nodes.node_id[node_id]);
                                }
                            }
                            else
                            {
                                VX_PRINT(VX_ZONE_ERROR, "Unable to enqueue ref\n");
                            }
                        }
                        break;
                    }
                    else
                    { /* do nothing*/ }
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Unable to enqueue a reference because it is NULL\n");
            }
        }

        if(((vx_status)VX_SUCCESS == status) && (num_enqueue > 0U))
        {
            /* Note: keeping compatibility with deprecated API */
            if (graph->parameters[graph_parameter_index].node->obj_desc[0]->pipeup_buf_idx > 1U)
            {
                graph->parameters[graph_parameter_index].node->obj_desc[0]->pipeup_buf_idx = graph->parameters[graph_parameter_index].node->obj_desc[0]->pipeup_buf_idx - 1U;
            }
            else
            {
                /* if graph mode is 'VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO' and
                 * enqueue of a reference at this parameter should trigger
                 * a graph schedule then schedule the graph */
                if(ownGraphDoScheduleGraphAfterEnqueue(graph, graph_parameter_index)==(vx_bool)vx_true_e)
                {
                    status = ownGraphScheduleGraph(graph, num_enqueue);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_PIPELINE_UTJT004
<justification end> */
/* TIOVX-1813- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_PIPELINE_UTJT004 */
                    if((vx_status)VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Failed to schedule graph \n");
                    }
/* LDRA_JUSTIFY_END */
                }
            }
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxGraphParameterEnqueueReadyRef(vx_graph graph,
                vx_uint32 graph_parameter_index,
                vx_reference *refs,
                vx_uint32 num_refs)
{
    /* flags is set to 0, i.e no special handling during enqueue */
    return ownGraphParameterEnqueueReadyRef(
                graph, graph_parameter_index, refs, num_refs);
}

VX_API_ENTRY vx_status VX_API_CALL vxGraphParameterDequeueDoneRef(vx_graph graph,
            vx_uint32 graph_parameter_index,
            vx_reference *refs,
            vx_uint32 max_refs,
            vx_uint32 *num_refs)
{
    tivx_data_ref_queue data_ref_q = NULL;
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 l_num_refs = 0;

    /* get data ref queue associated with a graph parameter
     * if this graph parameter is not enabled in queuing mode,
     * then a NULL data_ref_q is returned
     */
    data_ref_q = ownGraphGetParameterDataRefQueue(graph, graph_parameter_index);
    if(data_ref_q == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "Reference dequeue not supported at graph parameter index %d\n", graph_parameter_index);
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        tivx_obj_desc_node_t const * nobj = graph->parameters[graph_parameter_index].node->obj_desc[0];
        vx_bool is_replicated = tivxFlagIsBitSet(nobj->is_prm_replicated, ((uint32_t)1U<<graph->parameters[graph_parameter_index].index));
        vx_uint32 num_replicas = is_replicated ? nobj->num_of_replicas : 0U;
        vx_uint32 ref_id;
        vx_bool exit_loop = vx_false_e;
        for(ref_id = 0; ref_id < max_refs; ref_id++)
        {
            vx_reference ref;
            /* wait until a reference is dequeued */
            do
            {
                ref = NULL;
                uint16_t queue_obj_desc_id, ref_obj_desc_id;
                /* get queue object descriptor */
                queue_obj_desc_id = data_ref_q->done_q_obj_desc_id;
                ownPlatformSystemLock((vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);
                status = ownObjDescQueueDequeue(queue_obj_desc_id, &ref_obj_desc_id);
                if(((vx_status)VX_SUCCESS == status) &&
                    ((uint16_t)TIVX_OBJ_DESC_INVALID != ref_obj_desc_id))
                {
                    /* Decrement the queue counter, make reference accessible again */
                    ref = ownReferenceGetHandleFromObjDescId(ref_obj_desc_id);
                    if ((vx_bool)vx_true_e == is_replicated)
                    {
                        vx_uint32 i;
                        vx_reference const * ref_list = NULL;
                        /* We may get here with either the reference of the container,
                           or the reference of the first element of the container.
                           Notice that because we can't have object arrays of object
                           arrays, we can distinguish with the following test.
                           Note we only call ownDecrementEnqueueCount when we know
                           we have a valid container, otherwise we might be calling it
                           on a context or graph.
                           For a valid dequeued reference on a replicated node, we decrement
                           the counts on the container and all the references in it. */
                        if (ref->type == graph->parameters[graph_parameter_index].type)
                        {
                            ref = ref->scope;
                        }
                        if (ownIsValidSpecificReference(ref, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
                        {
                            vx_object_array object_array = vxCastRefAsObjectArray(ref, NULL);
                            ref_list = object_array->ref;
                            status = ownDecrementEnqueueCount(ref);
                        }
                        else if (ownIsValidSpecificReference(ref, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
                        {
                            vx_pyramid pyramid = vxCastRefAsPyramid(ref, NULL);
                            ref_list = (vx_reference *)(uintptr_t)(pyramid->img);
                            status = ownDecrementEnqueueCount(ref);
                        }
                        else 
                        {
                            /* do nothing */
                        }
                        if (NULL != ref_list)
                        {
                            ref = ref_list[0];
                            for (i = 0; (i < num_replicas) &&
                                 ((vx_status)VX_SUCCESS == status); ++i)
                            {
                                status = ownDecrementEnqueueCount(ref_list[i]);
                            }
                        }
                        else
                        {
                            /* There is no other container, so this suggests we have the wrong type
                            of parameter, or that the container didn't have ref_list initialised */
                            status = (vx_status)VX_FAILURE;
                        }
                    }
                    else
                    {
                        /* Node not replicated */
                        status = ownDecrementEnqueueCount(ref);
                    }
                    ownPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);
                    VX_PRINT(VX_ZONE_INFO,"DQ (queue=%d, ref=%d, num enqueues left = %d)\n", queue_obj_desc_id, ref_obj_desc_id, ref->obj_desc->num_enqueues);
                    exit_loop = (vx_bool)vx_true_e;
                    ++l_num_refs;
                    if((vx_status)VX_SUCCESS == status)
                    {
                        /* If the ref type matches the graph parameter type, return graph parameter */
                        if (ref->type == graph->parameters[graph_parameter_index].type)
                        {
                            refs[ref_id] = ref;
                        }
                        /* If the ref type doesn't match graph parameter type, throw an error */
                        else
                        {
                            VX_PRINT(VX_ZONE_ERROR,
                                "Returned reference does not match the expected reference at graph parameter %d\n", graph_parameter_index);
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        }
                    }
                }
                else if (0U == l_num_refs)
                {
                    ownPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);
                    VX_PRINT(VX_ZONE_INFO,"DQ (queue=%d) .. NO BUFFER\n", queue_obj_desc_id);
                    /* wait for "ref available for dequeue" event */
                    status = ownDataRefQueueWaitDoneRef(data_ref_q,
                            graph->timeout_val);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PIPELINE_UM006
<justification end> */
/* TIOVX-1720- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_PIPELINE_UM006 */
                    if(status!=(vx_status)VX_SUCCESS)
                    {
                        /* some error in waiting for event, break loop with error */
                        exit_loop = (vx_bool)vx_true_e;
                    }
                }
/* LDRA_JUSTIFY_END */
/* TIOVX-1720- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_PIPELINE_UM007 */
                else
                {
                    ownPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE);
                    status = (vx_status)VX_SUCCESS;
                    exit_loop = (vx_bool)vx_true_e;
                }
            } while(exit_loop == (vx_bool)vx_false_e);
            if ((vx_status)VX_SUCCESS != status)
            {
                /* some error in dequeue, dont try to dequeue further,
                 * break from loop with error */
                break;
            }
        }
        *num_refs = l_num_refs;
    }
    return status;
}


VX_API_ENTRY vx_status VX_API_CALL vxGraphParameterCheckDoneRef(vx_graph graph,
            vx_uint32 graph_parameter_index,
            vx_uint32 *num_refs)
{
    tivx_data_ref_queue data_ref_q = NULL;
    vx_status status = (vx_status)VX_SUCCESS;

    /* get data ref queue associated with a graph parameter
     * if this graph parameter is not enabled in queuing mode,
     * then a NULL data_ref_q is returned
     */
    data_ref_q = ownGraphGetParameterDataRefQueue(graph, graph_parameter_index);
    if(data_ref_q == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "Reference dequeue not supported at graph parameter index %d\n", graph_parameter_index);
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        *num_refs = 0;
        status = ownDataRefQueueGetDoneQueueCount(data_ref_q, num_refs);
    }
    return status;
}

static tivx_obj_desc_graph_t *ownGraphDequeueFreeObjDesc(vx_graph graph)
{
    vx_status status;
    tivx_obj_desc_graph_t *obj_desc = NULL;
    uintptr_t pipeline_id;
    vx_bool is_q_empty;

    is_q_empty = tivxQueueIsEmpty(&graph->free_q);

    if( (vx_bool)vx_false_e == is_q_empty )
    {
        status = tivxQueueGet(&graph->free_q, &pipeline_id, 0);
        if((status == (vx_status)VX_SUCCESS) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR017 */
        && (pipeline_id < graph->pipeline_depth)) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR018 */
        {
            obj_desc = graph->obj_desc[pipeline_id];
        }
    }
    return obj_desc;
}

static void ownGraphEnqueueFreeObjDesc(vx_graph graph, const tivx_obj_desc_graph_t *obj_desc)
{
    if((obj_desc != NULL) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR019 */
    && (obj_desc->pipeline_id < graph->pipeline_depth)) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR020 */
    {
        (void)tivxQueuePut(&graph->free_q, obj_desc->pipeline_id, TIVX_EVENT_TIMEOUT_NO_WAIT);
    }
}

static tivx_obj_desc_graph_t *ownGraphGetObjDesc(vx_graph graph, uint32_t pipeline_id)
{
    tivx_obj_desc_graph_t *obj_desc = NULL;

    if(pipeline_id < graph->pipeline_depth)
    {
        obj_desc = graph->obj_desc[pipeline_id];
    }
    return obj_desc;
}

vx_status ownGraphCreateQueues(vx_graph graph)
{
    vx_status status;

    status = tivxQueueCreate(&graph->free_q, TIVX_GRAPH_MAX_PIPELINE_DEPTH, graph->free_q_mem, 0);
    if ((vx_status)VX_SUCCESS == status)
    {
        status = ownEventQueueCreate(&graph->event_queue);
    }
    return status;
}


vx_status ownGraphDeleteQueues(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status status1 = (vx_status)VX_SUCCESS;
    status = tivxQueueDelete(&graph->free_q);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_PIPELINE_UTJT005
<justification end> */
/* TIOVX-1813- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_PIPELINE_UTJT005 */
    if((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to delete a queue\n");
    }
/* LDRA_JUSTIFY_END */
    status1 = ownEventQueueDelete(&graph->event_queue);
    if(status1 != (vx_status)VX_SUCCESS)
    {
        status = status1;
    }    
    return status;
}

/* called during graph verify after pipeline_depth is calculated and set */
vx_status ownGraphAllocAndEnqueueObjDescForPipeline(vx_graph graph)
{
    uint32_t i;
    vx_status status = (vx_status)VX_SUCCESS;

    for(i=0; i<TIVX_GRAPH_MAX_PIPELINE_DEPTH; i++)
    {
        graph->obj_desc[i] = NULL;
    }
    if(graph->pipeline_depth >= TIVX_GRAPH_MAX_PIPELINE_DEPTH)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid graph pipeline depth\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for(i=0; i<graph->pipeline_depth; i++)
        {
            graph->obj_desc[i] = (tivx_obj_desc_graph_t*)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_GRAPH, vxCastRefFromGraph(graph));
            if(graph->obj_desc[i]==NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "Unable to alloc graph obj desc\n");
                status = (vx_status)VX_ERROR_NO_RESOURCES;
                VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in include/TI/soc/tivx_config_<soc>.h\n");
                break;
            }
        }
        if(status==(vx_status)VX_SUCCESS)
        {
            for(i=0; i<graph->pipeline_depth; i++)
            {
                graph->obj_desc[i]->pipeline_id = i;
                /* initial state of graph descriptor is verified since, this function is called during verify graph */
                graph->obj_desc[i]->state = (vx_enum)VX_GRAPH_STATE_VERIFIED;
                graph->obj_desc[i]->complete_leaf_nodes = 0;
                graph->obj_desc[i]->exe_time_beg_h = 0;
                graph->obj_desc[i]->exe_time_beg_l = 0;
                graph->obj_desc[i]->exe_time_end_h = 0;
                graph->obj_desc[i]->exe_time_end_l = 0;

                (void)ownGraphEnqueueFreeObjDesc(graph, graph->obj_desc[i]);
            }
        }
    }
    return status;
}

/* called during graph release */
vx_status ownGraphFreeObjDesc(vx_graph graph)
{
    uint32_t i;
    vx_status status = (vx_status)VX_SUCCESS;
    for(i=0; i<graph->pipeline_depth; i++)
    {
        if(graph->obj_desc[i]!=NULL)
        {
            status = ownObjDescFree((tivx_obj_desc_t**)&graph->obj_desc[i]);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PIPELINE_UM009
<justification end>*/
/* TIOVX-1720- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_PIPELINE_UM009 */
            if(status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,"Failed to free the object descriptor\n");
                status = (vx_status)VX_FAILURE;
            }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PIPELINE_UM009
<justification end>*/
            else
/* LDRA_JUSTIFY_END */
            {
                graph->obj_desc[i] = NULL;
            }
        }
    }
    return status;
}

vx_bool ownCheckGraphCompleted(vx_graph graph, uint32_t pipeline_id)
{
    vx_bool is_send_graph_complete_event = (vx_bool)vx_false_e;
    tivx_obj_desc_graph_t *graph_obj_desc;
    uint32_t schedule_count = 0;

    if((vx_status)VX_SUCCESS == ownReferenceLock(&graph->base))
    {
        graph_obj_desc = ownGraphGetObjDesc(graph, pipeline_id);
        if (graph_obj_desc != NULL)
        {
            vx_bool is_completed = (vx_bool)vx_false_e;

            /* a leaf node completed so increment 'complete_leaf_nodes' */
            graph_obj_desc->complete_leaf_nodes = graph_obj_desc->complete_leaf_nodes + 1U;

            /* if all leaf nodes completed, then graph is completed */
            if(graph_obj_desc->complete_leaf_nodes==graph->num_leaf_nodes)
            {
                /* reset value to 0 for next graph run */
                graph_obj_desc->complete_leaf_nodes = 0;

                is_completed = (vx_bool)vx_true_e;
            }

            /* all leaf nodes completed, threfore graph is completed */
            if(is_completed==(vx_bool)vx_true_e)
            {
                uint64_t end_time;
                uint32_t i;

                is_send_graph_complete_event = (vx_bool)vx_true_e;

                /* a submitted graph is completed so decrement this field */

                graph->submitted_count--;

                /* update node performance */
                for(i=0; i<graph->num_nodes; i++)
                {
                    (void)ownUpdateNodePerf(graph->nodes[i], graph_obj_desc->pipeline_id);
                }

                if(graph->schedule_mode == (vx_enum)VX_GRAPH_SCHEDULE_MODE_NORMAL)
                {
                    /* delays need aging only if pipelining is not used */
                    for(i=0;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR021
<justification end>*/
                    i<TIVX_GRAPH_MAX_DELAYS;
/* LDRA_JUSTIFY_END */
                    i++) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR021 */
                    {
                        if(graph->delays[i] != NULL)
                        {
                            (void)vxAgeDelay(graph->delays[i]);
                        }
                        else
                        {
                            /* no more delays registered */
                            break;
                        }
                    }
                }
                if ((vx_enum)graph_obj_desc->state == (vx_enum)VX_GRAPH_STATE_RUNNING)
                {
                    graph_obj_desc->state = (vx_enum)VX_GRAPH_STATE_COMPLETED;
                }

                end_time = tivxPlatformGetTimeInUsecs();

                tivx_uint64_to_uint32(
                   end_time,
                   &graph_obj_desc->exe_time_end_h,
                   &graph_obj_desc->exe_time_end_l
                  );

                ownLogRtTraceGraphExeEnd(end_time, graph_obj_desc);

                (void)ownUpdateGraphPerf(graph, graph_obj_desc->pipeline_id);

                /* if submitted queue is empty then state of graph object
                 * is state on current completed graph pipeline instance
                 * else dont change the state of graph object
                 */
                if ( graph->submitted_count == 0U )
                {
                    graph->state = (int32_t)graph_obj_desc->state;
                }

                (void)ownGraphEnqueueFreeObjDesc(graph, graph_obj_desc);

                VX_PRINT(VX_ZONE_INFO,"Graph Completed (graph=%d, pipe=%d)\n",
                    graph_obj_desc->base.obj_desc_id,
                    graph_obj_desc->pipeline_id
                    );

                /* if there are any pending graph desc to be scehdule
                 * attempt to schedule them.
                 * graph->schedule_pending_count is copied to a local variable
                 * (schedule_count) and graph->schedule_pending_count is reset to 0.
                 *
                 * ownScheduleGraph, attempts to schedule the graph
                 * schedule_count times, internally it reinits graph->schedule_pending_count
                 * based on how many graph desc were scheduled
                 * and how many were left pending.
                 */
                if(graph->schedule_pending_count > 0U)
                {
                    schedule_count = graph->schedule_pending_count;

                    graph->schedule_pending_count = 0;
                }

                /* if all submitted graphs completed and no more pending to be scheduled
                 * then post all graph completed event
                 */
                if ( ( graph->submitted_count == 0U )
                        && (schedule_count == 0U))
                {
                    VX_PRINT(VX_ZONE_INFO,"All Graphs Completed\n");

                    /* there are no more pending graphs to set event to indicate no more pending graphs */
                    (void)tivxEventPost(graph->all_graph_completed_event);
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Graph object descriptor is NULL\n");
        }
        (void)ownReferenceUnlock(&graph->base);
    }
    if(schedule_count > 0U)
    {
        (void)ownGraphScheduleGraph(graph, schedule_count);
    }

    return is_send_graph_complete_event;
}

vx_status ownGraphScheduleGraph(vx_graph graph, uint32_t num_schedule)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status tmp_status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_graph_t *graph_obj_desc;
    uint32_t total_num_schedule, schedule_id, node_id;

    if((vx_status)VX_SUCCESS == ownReferenceLock(&graph->base))
    {

        /* total number of times to schedule is user requested num_schedule
         * + any other pending graph schedule
         */
        total_num_schedule = num_schedule + graph->schedule_pending_count;

        for(schedule_id=0; schedule_id<total_num_schedule; schedule_id++)
        {
            graph_obj_desc = ownGraphDequeueFreeObjDesc(graph);
            if(graph_obj_desc!=NULL)
            {
                uint64_t beg_time;

                beg_time = tivxPlatformGetTimeInUsecs();

                tivx_uint64_to_uint32(
                    beg_time,
                    &graph_obj_desc->exe_time_beg_h,
                    &graph_obj_desc->exe_time_beg_l
                );

                ownLogRtTraceGraphExeStart(beg_time, graph_obj_desc);

                ownGraphClearState(graph, graph_obj_desc->pipeline_id);

                graph_obj_desc->state = (vx_enum)VX_GRAPH_STATE_RUNNING;
                graph->state = (vx_enum)VX_GRAPH_STATE_RUNNING;

                /* a graph is about to be submitted, clear all_graph_completed_event if not already cleared */
                (void)tivxEventClear(graph->all_graph_completed_event);

                /* a graph is submitted for execution so increment below field */
                graph->submitted_count++;

                VX_PRINT(VX_ZONE_INFO,"Scheduling Graph (graph=%d, pipe=%d)\n",
                    graph_obj_desc->base.obj_desc_id,
                    graph_obj_desc->pipeline_id
                    );

                /* trigger graph execution by scheduling the head nodes
                 * Head nodes will trigger further nodes execution after
                 * their completion
                 * This will continue until leaf nodes executes
                 * After a leaf node executes, it will send a completion
                 * event.
                 * After all completion events are received, a graph is
                 * considered to have
                 * executed
                 */
                for(node_id=0; node_id<graph->num_head_nodes; node_id++)
                {
                    tmp_status = ownNodeKernelSchedule(graph->head_nodes[node_id], graph_obj_desc->pipeline_id);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_PIPELINE_UTJT006
<justification end> */
/* TIOVX-1813- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_PIPELINE_UTJT006 */
                    if((vx_status)VX_SUCCESS != tmp_status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Failed to schedule kernel\n");
                        status = tmp_status;
                    }
/* LDRA_JUSTIFY_END */
                }
            }
            else
            {
                /* For VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO/MANUAL,
                 * lack of graph descriptor, implies multiple graphs executions
                 * on-going in pipeline hence lack of graph obj desc means
                 * this graph schedule MUST be kept pending and tried after a previous graph execution
                 * completes.
                 */
                break;
            }
        }

        if( (graph->schedule_mode!=(vx_enum)VX_GRAPH_SCHEDULE_MODE_NORMAL) ||
            ((vx_bool)vx_true_e == graph->is_streaming_enabled) )
        {
            /* Below logic updates the pending graph schedule
             */
            graph->schedule_pending_count = total_num_schedule - schedule_id;
        }
        else
        {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_PIPELINE_UTJT007
<justification end> */
/* TIOVX-1813- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_PIPELINE_UTJT007 */
            if(schedule_id!=total_num_schedule)
            {
                /* for normal modes if all reqired graph schedules did not succeed
                 * then this is an error condition as user has tried
                 * doing schedule more times than is supported
                 */
                VX_PRINT(VX_ZONE_ERROR,"Free graph descriptor not available, cannot schedule graph\n");
                status = (vx_status)VX_ERROR_NO_RESOURCES;
            }
/* LDRA_JUSTIFY_END */
        }
        (void)ownReferenceUnlock(&graph->base);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Failed to lock reference\n");
        status = (vx_status)VX_FAILURE;
    }
    return status;
}

vx_bool ownGraphDoScheduleGraphAfterEnqueue(vx_graph graph, uint32_t graph_parameter_index)
{
    vx_bool do_schedule_graph_after_enqueue = (vx_bool)vx_false_e;

    if(graph != NULL)
    {
        if(graph->schedule_mode==(vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO)
        {
            if(graph_parameter_index == 0U)
            {
                do_schedule_graph_after_enqueue = (vx_bool)vx_true_e;
            }
        }
    }
    return do_schedule_graph_after_enqueue;
}

tivx_data_ref_queue ownGraphGetParameterDataRefQueue(vx_graph graph, vx_uint32 graph_parameter_index)
{
    tivx_data_ref_queue ref = NULL;

    if((graph != NULL) && (graph_parameter_index < graph->num_params))
    {
        ref = graph->parameters[graph_parameter_index].data_ref_queue;
    }
    return ref;
}

vx_status tivxSetGraphPipelineDepth(vx_graph graph, vx_uint32 pipeline_depth)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        if (graph->verified == (vx_bool)vx_true_e)
        {
            VX_PRINT(VX_ZONE_ERROR, "Not supported on verified graph\n");
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            if (pipeline_depth < TIVX_GRAPH_MAX_PIPELINE_DEPTH)
            {
                graph->pipeline_depth = pipeline_depth;
                graph->is_pipeline_depth_set = (vx_bool)vx_true_e;
                ownLogSetResourceUsedValue("TIVX_GRAPH_MAX_PIPELINE_DEPTH", (uint16_t)graph->pipeline_depth+1U);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "pipeline depth greater than max allowed pipeline depth\n");
                VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_GRAPH_MAX_PIPELINE_DEPTH in tiovx/include/TI/tivx_config.h\n");
                status = (vx_status)VX_ERROR_INVALID_VALUE;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

uint32_t ownGraphGetNumSchedule(vx_graph graph)
{
    uint32_t num_schedule = 0;

    if(graph != NULL)
    {
        if(graph->schedule_mode==(vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL)
        {
            uint32_t min_count = (uint32_t)-1;
            uint32_t count, i;

            for(i=0; i<graph->num_params; i++)
            {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR022
<justification end> */
                if(graph->parameters[i].queue_enable != 0) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR022 */
                {
                    count = 0;

                    (void)ownDataRefQueueGetReadyQueueCount(
                            graph->parameters[i].data_ref_queue, &count);

                    if(count<min_count)
                    {
                        min_count = count;
                    }
                }
/* LDRA_JUSTIFY_END */
            }
            if(min_count == (uint32_t)-1)
            {
                min_count = 0;
            }
            num_schedule = min_count;
        }
    }
    return num_schedule;
}

vx_status ownGraphValidatePipelineParameters(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t param_idx = 0U;

    if ((vx_bool)vx_true_e == graph->is_pipelining_enabled)
    {
        for (param_idx = 0U; param_idx < graph->num_params; param_idx++)
        {
            vx_node node;
            uint32_t node_idx;

            node_idx = graph->parameters[param_idx].index;
            node     = graph->parameters[param_idx].node;

            /* Value of 0 is the default.  If the value is set
             * to 0, then the buffers have not been set.  Therefore,
             * if both are nonzero, then there is an error, so flagging */
            if ( (0U != graph->parameters[param_idx].num_buf) &&
                 (0U != node->parameter_index_num_buf[node_idx]) )
            {
                vx_reference node_ref;
                vx_reference param_ref;

                node_ref  = vxCastRefFromNode(node);
                param_ref = (vx_reference)node->parameters[node_idx];

                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Invalid pipelining parameters\n");
                VX_PRINT(VX_ZONE_ERROR, "Parameter %s of node %s has multiple buffers set both as a graph parameter and using tivxSetNodeParameterNumBufByIndex\n", param_ref->name, node_ref->name);
            }
        }
    }

    return status;
}

/* Returning optimal num bufs as 1 plus number of input nodes connected to ref */
static uint32_t ownGraphGetOptimalNumBuf(vx_graph graph, vx_reference ref)
{
    uint32_t num_bufs = 1U;
    uint32_t i;

    for(i=0;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR023
<justification end>*/
    i<graph->num_data_ref;
/* LDRA_JUSTIFY_END */
    i++) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR023 */
    {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR024
<justification end> */
        if (i < TIVX_GRAPH_MAX_DATA_REF) /* TIOVX-1945- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_PIPELINE_UBR024 */
        {
            if(ownGraphCheckIsRefMatch(graph, graph->data_ref[i], ref) != 0)
            {
                /* Adding the number of nodes that consume this reference */
                num_bufs += graph->data_ref_num_in_nodes[i];
                break;
            }
        }
/* LDRA_JUSTIFY_END */
    }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_PIPELINE_UTJT008
<justification end> */
/* TIOVX-1813- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_PIPELINE_UTJT008 */
    if (num_bufs >= TIVX_OBJ_DESC_QUEUE_MAX_DEPTH)
    {
        VX_PRINT(VX_ZONE_INFO, "Required number of buffers = %d but max buffer depth = %d\n", num_bufs, (int32_t)TIVX_OBJ_DESC_QUEUE_MAX_DEPTH-1);
        VX_PRINT(VX_ZONE_INFO, "Will need to increase the value of TIVX_OBJ_DESC_QUEUE_MAX_DEPTH in tiovx/include/TI/tivx_config.h to get full performance\n");
        num_bufs = (int32_t)TIVX_OBJ_DESC_QUEUE_MAX_DEPTH-1;
    }
/* LDRA_JUSTIFY_END */

    return num_bufs;
}

static vx_bool isLeafNode(vx_graph graph, vx_node node)
{
    vx_bool is_leaf_node = (vx_bool)vx_false_e;
    uint32_t i = 0;

    for (i = 0; i < TIVX_GRAPH_MAX_HEAD_NODES; i++)
    {
        if (node == graph->leaf_nodes[i])
        {
            is_leaf_node = (vx_bool)vx_true_e;
            break;
        }
    }

    return is_leaf_node;
}

void ownGraphDetectAndSetNumBuf(vx_graph graph)
{
    vx_node node_cur;
    uint32_t node_cur_idx;
    uint32_t prm_cur_idx;
    uint32_t prm_dir;
    uint32_t graph_param_idx = 0U;
    vx_reference ref;
    vx_bool is_ref_graph_param = (vx_bool)vx_false_e;

    if ((vx_bool)vx_true_e == graph->is_pipelining_enabled)
    {
        for(node_cur_idx=0; node_cur_idx<graph->num_nodes; node_cur_idx++)
        {
            node_cur = graph->nodes[node_cur_idx];

            /* Not checking outputs of leaf nodes given that leaf node
             * outputs are not being consumed and should be set as
             * graph parameter if needed to be consumed downstream */
            if ((vx_bool)vx_false_e == isLeafNode(graph, node_cur))
            {
                for(prm_cur_idx=0; prm_cur_idx<ownNodeGetNumParameters(node_cur); prm_cur_idx++)
                {
                    ref = ownNodeGetParameterRef(node_cur, prm_cur_idx);
                    prm_dir = (uint32_t)ownNodeGetParameterDir(node_cur, prm_cur_idx);

                    if( (ref!=NULL) && ((vx_enum)prm_dir !=  (vx_enum)VX_INPUT)) /* ref could be NULL due to optional parameters */
                    {
                        is_ref_graph_param = (vx_bool)vx_false_e;

                        /* Checking to see if node parameter is also graph parameter, breaking if so */
                        for (graph_param_idx = 0U; graph_param_idx < graph->num_params; graph_param_idx++)
                        {
                            if (ref == graph->parameters[graph_param_idx].refs_list[0])
                            {
                                is_ref_graph_param = (vx_bool)vx_true_e;
                                break;
                            }
                        }

                        if ( (vx_bool)vx_false_e == is_ref_graph_param )
                        {
                            uint32_t optimal_num_buf;
                            vx_reference node_ref;

                            node_ref = vxCastRefFromNode(node_cur);
                            optimal_num_buf = ownGraphGetOptimalNumBuf(graph, ref);

                            /* Given that 0 is the default value, this if statement checks
                             * for if the buffers have not been set at all */
                            if (0U == node_cur->parameter_index_num_buf[prm_cur_idx])
                            {
                                node_cur->parameter_index_num_buf[prm_cur_idx] = optimal_num_buf;
                                VX_PRINT(VX_ZONE_INFO, "Buffer depth not set by user at node %s parameter %s\n", node_ref->name, ref->name);
                                VX_PRINT(VX_ZONE_INFO, "Setting number of buffers to %d\n", node_cur->parameter_index_num_buf[prm_cur_idx]);
                            }
                            else if (optimal_num_buf > node_cur->parameter_index_num_buf[prm_cur_idx])
                            {
                                /* Flagging to user if the number of buffers set is less than optimal */
                                VX_PRINT(VX_ZONE_INFO, "Internally computed buffer value greater than buffers set at node %s parameter %s\n", node_ref->name, ref->name);
                                VX_PRINT(VX_ZONE_INFO, "Computed number of buffers = %d, set number of buffers = %d\n", optimal_num_buf, node_cur->parameter_index_num_buf[prm_cur_idx]);
                            }
                            else
                            {
                                /* the number of buffers set is greater than optimal. */
                            }
                        }
                    }
                }
            }
        }
    }

    return;
}

vx_uint32 ownGraphGetPipeDepth(vx_graph graph)
{
    uint32_t pipe_depth = 1;
    uint32_t node_cur_idx;
    vx_node node_cur;

    for(node_cur_idx=0; node_cur_idx<graph->num_nodes; node_cur_idx++)
    {
        node_cur = graph->nodes[node_cur_idx];

        if ((vx_bool)vx_true_e == isLeafNode(graph, node_cur))
        {
            if (pipe_depth <= node_cur->node_depth)
            {
                pipe_depth = node_cur->node_depth;
            }
        }
    }

    if ( (pipe_depth >= TIVX_GRAPH_MAX_PIPELINE_DEPTH) &&
         ((vx_bool)vx_false_e == graph->is_pipeline_depth_set) )
    {
        VX_PRINT(VX_ZONE_INFO, "Required pipe depth = %d but max pipe depth = %d\n", pipe_depth, (int32_t)TIVX_GRAPH_MAX_PIPELINE_DEPTH-1);
        VX_PRINT(VX_ZONE_INFO, "Will need to increase the value of TIVX_GRAPH_MAX_PIPELINE_DEPTH in tiovx/include/TI/tivx_config.h to get full performance\n");
        pipe_depth = (int32_t)TIVX_GRAPH_MAX_PIPELINE_DEPTH-1;
    }

    return pipe_depth;
}
