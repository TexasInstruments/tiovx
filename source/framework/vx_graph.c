/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#include <vx_internal.h>

static vx_status ownDestructGraph(vx_reference ref);
static vx_status ownResetGraphPerf(vx_graph graph);

static vx_status ownDestructGraph(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status status1 = (vx_status)VX_SUCCESS;
    vx_graph graph = (vx_graph)ref;

    ownGraphFreeStreaming(graph);

    {
        uint32_t i;

        for(i=0; i<graph->num_params; i++)
        {
            if((graph->parameters[i].queue_enable != (vx_bool)vx_false_e) && (graph->parameters[i].data_ref_queue != NULL))
            {
                ownDataRefQueueRelease(&graph->parameters[i].data_ref_queue);
            }
        }
        for(i=0; i<graph->num_data_ref_q; i++)
        {
            if(graph->data_ref_q_list[i].data_ref_queue != NULL)
            {
                ownDataRefQueueRelease(&graph->data_ref_q_list[i].data_ref_queue);
            }
            if(graph->data_ref_q_list[i].num_buf > 1U)
            {
                vx_bool is_replicated
                            = ownNodeIsPrmReplicated(graph->data_ref_q_list[i].node, graph->data_ref_q_list[i].index);
                uint32_t buf_id;

                /* additional references allocated during verify, release them */
                for(buf_id=1; buf_id<graph->data_ref_q_list[i].num_buf; buf_id++)
                {
                    vx_reference data_ref = graph->data_ref_q_list[i].refs_list[buf_id];

                    if(is_replicated != 0)
                    {
                        data_ref = data_ref->scope;
                    }

                    vxReleaseReference(&data_ref);

                    graph->data_ref_q_list[i].refs_list[buf_id] = NULL;
                }

                graph->data_ref_q_list[i].node  = NULL;
                graph->data_ref_q_list[i].index = 0;
            }
        }
        for(i=0; i<graph->num_delay_data_ref_q; i++)
        {
            if(graph->delay_data_ref_q_list[i].data_ref_queue != NULL)
            {
                ownDataRefQueueRelease(&graph->delay_data_ref_q_list[i].data_ref_queue);
            }
        }

        for(i=0; i<graph->num_supernodes; i++)
        {
            status1 = ownReleaseReferenceInt((vx_reference *)&graph->supernodes[i], TIVX_TYPE_SUPER_NODE, (vx_enum)VX_INTERNAL, NULL);

            if (status1 != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "ownReleaseReferenceInt() failed.\n");
                status = status1;
            }

            graph->supernodes[i] = NULL;
        }
        graph->num_supernodes = 0;
    }

    while (graph->num_nodes != 0U)
    {
        vx_node node = graph->nodes[0];

        status1 = vxRemoveNode(&node);

        if (status1 != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "ownReleaseReferenceInt() failed.\n");
            status = status1;
        }
    }

    ownGraphDeleteQueues(graph);
    ownGraphFreeObjDesc(graph);
    tivxEventDelete(&graph->all_graph_completed_event);

    return (vx_status)status;
}

static vx_status ownResetGraphPerf(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        graph->perf.tmp = 0;
        graph->perf.beg = 0;
        graph->perf.end = 0;
        graph->perf.sum = 0;
        graph->perf.avg = 0;
        graph->perf.min = 0xFFFFFFFFFFFFFFFFUL;
        graph->perf.num = 0;
        graph->perf.max = 0;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph object\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    return status;
}

vx_status ownUpdateGraphPerf(vx_graph graph, uint32_t pipeline_id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) ==
            (vx_bool)vx_true_e) &&
            (pipeline_id < graph->pipeline_depth))
    {
        uint64_t beg_time, end_time;
        tivx_obj_desc_graph_t *obj_desc;

        obj_desc = graph->obj_desc[pipeline_id];

        tivx_uint32_to_uint64(&beg_time, obj_desc->exe_time_beg_h, obj_desc->exe_time_beg_l);
        tivx_uint32_to_uint64(&end_time, obj_desc->exe_time_end_h, obj_desc->exe_time_end_l);

        graph->perf.beg = beg_time*1000U; /* convert to nano secs */
        graph->perf.end = end_time*1000U; /* convert to nano secs */
        graph->perf.tmp = (end_time - beg_time)*1000U; /* convert to nano secs */
        graph->perf.sum += graph->perf.tmp;
        graph->perf.num++;
        if(graph->perf.tmp < graph->perf.min)
        {
            graph->perf.min = graph->perf.tmp;
        }
        if(graph->perf.tmp > graph->perf.max)
        {
            graph->perf.max = graph->perf.tmp;
        }
        graph->perf.avg = graph->perf.sum/graph->perf.num;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph object\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    return status;
}

int32_t ownGraphGetFreeNodeIndex(vx_graph graph)
{
    int32_t free_index = -(int32_t)1;

    if (ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        if(graph->num_nodes < TIVX_GRAPH_MAX_NODES)
        {
            free_index = (int32_t)graph->num_nodes;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Max nodes per graph (%d) has been exceeded\n", TIVX_GRAPH_MAX_NODES);
        }
    }

    return free_index;
}

vx_status ownGraphAddNode(vx_graph graph, vx_node node, int32_t index)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        if( (index < (int32_t)TIVX_GRAPH_MAX_NODES) && (index == (int32_t)graph->num_nodes) )
        {
            /* index MUST be graph->num_nodes, since that is what is returned via
                ownGraphGetFreeNodeIndex() */
            ownIncrementReference(&node->base, (vx_enum)VX_INTERNAL);
            graph->nodes[graph->num_nodes] = node;
            graph->num_nodes++;
            ownGraphSetReverify(graph);
            ownLogSetResourceUsedValue("TIVX_GRAPH_MAX_NODES", (uint16_t)graph->num_nodes);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "invalid graph index\n");
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_GRAPH_MAX_NODES in tiovx/include/TI/tivx_config.h\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph object\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status ownGraphAddSuperNode(vx_graph graph, tivx_super_node super_node)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        if( (graph->num_supernodes < TIVX_GRAPH_MAX_SUPER_NODES) )
        {
            ownIncrementReference(&super_node->base, (vx_enum)VX_INTERNAL);
            graph->supernodes[graph->num_supernodes] = super_node;
            graph->num_supernodes++;
            ownGraphSetReverify(graph);
            ownLogSetResourceUsedValue("TIVX_GRAPH_MAX_SUPER_NODES", (uint16_t)graph->num_supernodes);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_GRAPH_MAX_SUPER_NODES in tiovx/include/TI/tivx_config.h\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph object\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status ownGraphRemoveNode(vx_graph graph, vx_node node)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status status1 = (vx_status)VX_SUCCESS;
    uint32_t i;

    if (ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        /* remove node from head nodes and leaf nodes if found */
        for(i=0; i < graph->num_head_nodes; i++)
        {
            if(node==graph->head_nodes[i])
            {
                /* swap with last entry to make the list compact */
                graph->head_nodes[i] = graph->head_nodes[graph->num_head_nodes-1U];
                graph->head_nodes[graph->num_head_nodes-1U] = NULL;
                graph->num_head_nodes--;
                break;
            }
        }

        for(i=0; i < graph->num_leaf_nodes; i++)
        {
            if(node==graph->leaf_nodes[i])
            {
                /* swap with last entry to make the list compact */
                graph->leaf_nodes[i] = graph->leaf_nodes[graph->num_leaf_nodes-1U];
                graph->head_nodes[graph->num_leaf_nodes-1U] = NULL;
                graph->num_leaf_nodes--;
                break;
            }
        }

        for(i=0; i < graph->num_nodes; i++)
        {
            if(node==graph->nodes[i])
            {
                /* swap with last entry to make the list compact */
                graph->nodes[i] = graph->nodes[graph->num_nodes-1U];
                graph->nodes[graph->num_nodes-1U] = NULL;
                graph->num_nodes--;
                status1 = ownReleaseReferenceInt((vx_reference *)&node, (vx_enum)VX_TYPE_NODE, (vx_enum)VX_INTERNAL, NULL);

                if (status1 != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "ownReleaseReferenceInt() failed.\n");
                    status = status1;
                }

                ownGraphSetReverify(graph);
                break;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph object\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;

}

void ownGraphClearState(vx_graph graph, uint32_t pipeline_id)
{
    uint32_t i;

    for(i=0; i < graph->num_nodes; i++)
    {
        ownNodeClearExecuteState(graph->nodes[i], pipeline_id);
    }
}

VX_API_ENTRY vx_graph VX_API_CALL vxCreateGraph(vx_context context)
{
    vx_graph  graph = NULL;
    uint32_t idx;
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        graph = (vx_graph)ownCreateReference(context, (vx_enum)VX_TYPE_GRAPH, (vx_enum)VX_EXTERNAL, &context->base);
        if ( (vxGetStatus((vx_reference)graph) == (vx_status)VX_SUCCESS) && (graph->base.type == (vx_enum)VX_TYPE_GRAPH) )
        {
            graph->base.destructor_callback = &ownDestructGraph;
            graph->base.release_callback = (tivx_reference_release_callback_f)&vxReleaseGraph;

            graph->num_nodes = 0;
            graph->num_head_nodes = 0;
            graph->num_leaf_nodes = 0;
            graph->num_params = 0;
            graph->pipeline_depth = 1;
            graph->streaming_executions = 0;
            graph->graph_completed_app_value = 0;
            graph->is_streaming   = (vx_bool)vx_false_e;
            graph->is_streaming_enabled   = (vx_bool)vx_false_e;
            graph->trigger_node_set   = (vx_bool)vx_false_e;
            graph->is_enable_send_complete_event = (vx_bool)vx_false_e;
            graph->stop_done = NULL;
            graph->delete_done = NULL;
            graph->schedule_mode = (vx_enum)VX_GRAPH_SCHEDULE_MODE_NORMAL;
            graph->is_pipelining_enabled = (vx_bool)vx_false_e;
            graph->is_pipeline_depth_set = (vx_bool)vx_false_e;
            graph->schedule_pending_count = 0;
            graph->submitted_count = 0;
            graph->num_data_ref = 0;
            graph->num_data_ref_q = 0;
            graph->num_delay_data_ref_q = 0;
            graph->num_supernodes = 0;
            graph->timeout_val = TIVX_DEFAULT_GRAPH_TIMEOUT;

            ownResetGraphPerf(graph);

            for (idx = 0; idx < TIVX_GRAPH_MAX_DELAYS; idx++)
            {
                graph->delays[idx] = NULL;
            }
            for (idx = 0; idx < TIVX_GRAPH_MAX_NODES; idx++)
            {
                graph->nodes[idx] = NULL;
            }
            for (idx = 0; idx < TIVX_GRAPH_MAX_HEAD_NODES; idx++)
            {
                graph->head_nodes[idx] = NULL;
            }
            for (idx = 0; idx < TIVX_GRAPH_MAX_LEAF_NODES; idx++)
            {
                graph->leaf_nodes[idx] = NULL;
            }

            graph->verified = (vx_bool)vx_false_e;
            graph->reverify = (vx_bool)vx_false_e;
            graph->state = (vx_enum)VX_GRAPH_STATE_UNVERIFIED;

            status = tivxEventCreate(&graph->all_graph_completed_event);
            if(status==(vx_status)VX_SUCCESS)
            {
                status = ownGraphCreateQueues(graph);
            }
            if(status!=(vx_status)VX_SUCCESS)
            {
                vxReleaseGraph(&graph);

                VX_PRINT(VX_ZONE_ERROR, "Could not create graph\n");
                graph = (vx_graph)ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
            }

        }
    }

    return (vx_graph)graph;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetGraphAttribute(vx_graph graph, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if (ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        switch (attribute)
        {
            case (vx_enum)TIVX_GRAPH_TIMEOUT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    vx_uint32   timeout_val = *(vx_uint32*)ptr;

                    /* Validate the timeout. It cannot be zero. */
                    if (timeout_val == 0)
                    {
                        VX_PRINT(VX_ZONE_ERROR,
                                 "Invalid timeout value specified: %d\n",
                                 timeout_val);
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                    else
                    {
                        graph->timeout_val = timeout_val;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Set TIVX_GRAPH_TIMEOUT failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR,"Invalid attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph object\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryGraph(vx_graph graph, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if (ownIsValidReference(&graph->base) == (vx_bool)vx_true_e)
    {
        switch (attribute)
        {
            case (vx_enum)VX_GRAPH_PERFORMANCE:
                if (VX_CHECK_PARAM(ptr, size, vx_perf_t, 0x3U))
                {
                    memcpy(ptr, &graph->perf, size);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query graph performance failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_GRAPH_STATE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_status *)ptr = graph->state;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query graph state failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_GRAPH_NUMNODES:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = graph->num_nodes - graph->num_supernodes;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query graph number of nodes failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_GRAPH_NUMPARAMETERS:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = graph->num_params;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query graph number of parameters failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_GRAPH_STREAM_EXECUTIONS:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = graph->streaming_executions;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query graph number of streaming executions\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_GRAPH_TIMEOUT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = graph->timeout_val;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query TIVX_GRAPH_TIMEOUT failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_GRAPH_PIPELINE_DEPTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = graph->pipeline_depth;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query TIVX_GRAPH_PIPELINE_DEPTH failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "invalid attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseGraph(vx_graph *g)
{
    return ownReleaseReferenceInt((vx_reference *)g, (vx_enum)VX_TYPE_GRAPH, (vx_enum)VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_status VX_API_CALL vxAddParameterToGraph(vx_graph graph, vx_parameter param)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    if ((ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e) &&
        (ownIsValidSpecificReference((vx_reference)param, (vx_enum)VX_TYPE_PARAMETER) == (vx_bool)vx_true_e))
    {
        if(graph->num_params < TIVX_GRAPH_MAX_PARAMS)
        {
            graph->parameters[graph->num_params].node = param->node;
            graph->parameters[graph->num_params].index = param->index;
            graph->parameters[graph->num_params].queue_enable = (vx_bool)vx_false_e;
            graph->parameters[graph->num_params].is_enable_send_ref_consumed_event = (vx_bool)vx_false_e;
            graph->parameters[graph->num_params].graph_consumed_app_value = 0U;
            graph->parameters[graph->num_params].data_ref_queue = NULL;
            graph->parameters[graph->num_params].num_buf = 0;
            graph->parameters[graph->num_params].type = (vx_enum)VX_TYPE_PARAMETER;
            graph->num_params++;
            ownLogSetResourceUsedValue("TIVX_GRAPH_MAX_PARAMS", (uint16_t)graph->num_params);
            status = (vx_status)VX_SUCCESS;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "number of graph parameters greater than maximum allowed\n");
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_GRAPH_MAX_PARAMS in tiovx/include/TI/tivx_config.h\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }
    else if ((ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e) &&
              (ownIsValidSpecificReference((vx_reference)param, (vx_enum)VX_TYPE_PARAMETER) == (vx_bool)vx_false_e))
    {
        if(graph->num_params < TIVX_GRAPH_MAX_PARAMS)
        {
            /* insert an empty parameter */
            graph->parameters[graph->num_params].node = NULL;
            graph->parameters[graph->num_params].index = 0;
            graph->parameters[graph->num_params].queue_enable = (vx_bool)vx_false_e;
            graph->num_params++;
            ownLogSetResourceUsedValue("TIVX_GRAPH_MAX_PARAMS", (uint16_t)graph->num_params);
            status = (vx_status)VX_SUCCESS;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "number of graph parameters greater than maximum allowed\n");
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_GRAPH_MAX_PARAMS in tiovx/include/TI/tivx_config.h\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }
    else
    {
        /* Do Nothing */
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetGraphParameterByIndex(vx_graph graph, vx_uint32 index, vx_reference value)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    if (ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        if ((index < TIVX_GRAPH_MAX_PARAMS) && (index < graph->num_params))
        {
            status = vxSetParameterByIndex(graph->parameters[index].node,
                                           graph->parameters[index].index,
                                           value);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "index greater than number of parameters allowed\n");
            status = (vx_status)VX_ERROR_INVALID_VALUE;
        }
    }
    return status;
}

VX_API_ENTRY vx_parameter VX_API_CALL vxGetGraphParameterByIndex(vx_graph graph, vx_uint32 index)
{
    vx_parameter parameter = NULL;
    if (ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        if ((index < TIVX_GRAPH_MAX_PARAMS) && (index < graph->num_params))
        {
            vx_uint32 node_index = graph->parameters[index].index;
            parameter = vxGetParameterByIndex((vx_node)graph->parameters[index].node, node_index);
        }
    }
    else
    {

    }
    return parameter;
}

VX_API_ENTRY vx_bool VX_API_CALL vxIsGraphVerified(vx_graph graph)
{
    vx_bool verified = (vx_bool)vx_false_e;
    if (ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        verified = graph->verified;
    }
    return verified;
}

VX_API_ENTRY vx_status VX_API_CALL vxRegisterAutoAging(vx_graph graph, vx_delay delay)
{
    uint8_t i;
    vx_status status = (vx_status)VX_SUCCESS;
    vx_bool is_registered = (vx_bool)vx_false_e;
    vx_bool is_full = (vx_bool)vx_true_e;

    if(ownIsValidSpecificReference((vx_reference)delay, (vx_enum)VX_TYPE_DELAY) != (vx_bool)vx_false_e)
    {
        if(ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
        {
            /* check if this particular delay is already registered in the graph */
            for (i = 0; i < TIVX_GRAPH_MAX_DELAYS; i++)
            {
                if (graph->delays[i] == delay)
                {
                    is_registered = (vx_bool)vx_true_e;
                    break;
                }
            }

            /* if not regisered yet, find the first empty slot and register delay */
            if (is_registered == (vx_bool)vx_false_e)
            {
                for (i = 0; i < TIVX_GRAPH_MAX_DELAYS; i++)
                {
                    if (graph->delays[i] == NULL)
                    {
                        is_full = (vx_bool)vx_false_e;
                        graph->delays[i] = delay;
                        ownLogSetResourceUsedValue("TIVX_GRAPH_MAX_DELAYS", (uint16_t)i+1U);
                        break;
                    }
                }

                /* report error if there is no empty slots to register delay */
                if (is_full == (vx_bool)vx_true_e)
                {
                    VX_PRINT(VX_ZONE_ERROR, "no empty slots to register delay\n");
                    VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_GRAPH_MAX_DELAYS in tiovx/include/TI/tivx_config.h\n");
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "invalid graph reference\n");
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid delay reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxProcessGraph(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
    {
        status = vxScheduleGraph(graph);
        if(status == (vx_status)VX_SUCCESS)
        {
            status = vxWaitGraph(graph);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "schedule graph failed\n");
        }

        if(status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "wait graph failed\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status ownGraphScheduleGraphWrapper(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if((vx_bool)vx_false_e == vxIsGraphVerified(graph))
    {
        /* verify graph if not already verified */
        status = vxVerifyGraph(graph);
    }

    if ((status == (vx_status)VX_SUCCESS)
        && ( (graph->state == (vx_enum)VX_GRAPH_STATE_VERIFIED) ||
             (graph->state == (vx_enum)VX_GRAPH_STATE_COMPLETED) ||
             (graph->state == (vx_enum)VX_GRAPH_STATE_ABANDONED)
            ))
    {
        /* If streaming and pipelining are enabled, AUTO scheduling will schedule one at a time */
        if( graph->schedule_mode==(vx_enum)VX_GRAPH_SCHEDULE_MODE_NORMAL )
        {
            /* schedule graph one time */
            ownGraphScheduleGraph(graph, 1);
        }
        else
        if( (graph->schedule_mode==(vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL) &&
            ((vx_bool)vx_false_e == graph->is_streaming_enabled) )
        {
            uint32_t num_schedule = ownGraphGetNumSchedule(graph);

            if(num_schedule>0U)
            {
                /* schedule graph 'num_schedule' times */
                ownGraphScheduleGraph(graph, num_schedule);
            }
        }
        else
        if( (graph->schedule_mode==(vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL) &&
            ((vx_bool)vx_true_e == graph->is_streaming_enabled) )
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "manual mode is not allowed with streaming enabled\n");
        }
        else
        {
            /* Do nothing, required by MISRA-C */
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "graph is not in a state required to be scheduled\n");
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxScheduleGraph(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
    {
        if (graph->is_streaming_enabled != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "graph is already streaming\n");
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }
        else
        {
            if(graph->schedule_mode==(vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO)
            {
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                VX_PRINT(VX_ZONE_ERROR, "not supported for VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO\n");
            }
            else
            {
                status = ownGraphScheduleGraphWrapper(graph);
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxWaitGraph(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) ==
            (vx_bool)vx_true_e)
    {
        if ((graph->state == (vx_enum)VX_GRAPH_STATE_RUNNING)
           || (graph->state == (vx_enum)VX_GRAPH_STATE_COMPLETED)
           || (graph->state == (vx_enum)VX_GRAPH_STATE_ABANDONED))
        {
            /* wait for all previous graph executions to finish
             *
             */
            status = tivxEventWait(graph->all_graph_completed_event, graph->timeout_val);
            if(status == (vx_status)VX_SUCCESS)
            {
                if(graph->state == (vx_enum)VX_GRAPH_STATE_ABANDONED)
                {
                    status = (vx_status)VX_ERROR_GRAPH_ABANDONED;
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxEventWait() failed.\n");
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "graph not in expected state\n");
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

void ownGraphSetReverify(vx_graph graph)
{
    if(graph != NULL)
    {
        graph->reverify = graph->verified;
        graph->verified = (vx_bool)vx_false_e;
        graph->state = (vx_enum)VX_GRAPH_STATE_UNVERIFIED;
    }
}

void ownSendGraphCompletedEvent(vx_graph graph)
{
    if((graph != NULL) && (graph->base.context != NULL))
    {
        if(graph->is_enable_send_complete_event != 0)
        {
            uint64_t timestamp;

            timestamp = tivxPlatformGetTimeInUsecs()*1000U; /* in nano-secs */

            ownEventQueueAddEvent(&graph->base.context->event_queue,
                        (vx_enum)VX_EVENT_GRAPH_COMPLETED, timestamp, graph->graph_completed_app_value,
                        (uintptr_t)graph, (uintptr_t)0, (uintptr_t)0);
        }
    }
}

vx_status ownGraphRegisterCompletionEvent(vx_graph graph, vx_uint32 app_value)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        if (graph->verified == (vx_bool)vx_true_e)
        {
            VX_PRINT(VX_ZONE_ERROR, "Cannot register event on verified graph\n");
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            graph->is_enable_send_complete_event = (vx_bool)vx_true_e;
            graph->graph_completed_app_value = app_value;
            VX_PRINT(VX_ZONE_INFO, "Enabling completion event at graph [%s]\n", graph->base.name);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

vx_status ownGraphRegisterParameterConsumedEvent(vx_graph graph, uint32_t graph_parameter_index, vx_uint32 app_value)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        if (graph->verified == (vx_bool)vx_true_e)
        {
            VX_PRINT(VX_ZONE_ERROR, "Cannot register event on verified graph\n");
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            if(graph_parameter_index < graph->num_params)
            {
                graph->parameters[graph_parameter_index].is_enable_send_ref_consumed_event
                    = (vx_bool)vx_true_e;
                graph->parameters[graph_parameter_index].graph_consumed_app_value = app_value;
                VX_PRINT(VX_ZONE_INFO, "Enabling parameter ref consumed event at graph [%s], param %d\n",
                    graph->base.name, graph_parameter_index);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

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

void ownSetGraphState(vx_graph graph, uint32_t pipeline_id, vx_enum state)
{
    if((graph != NULL) && (pipeline_id < graph->pipeline_depth))
    {
        tivx_obj_desc_graph_t *graph_obj_desc = graph->obj_desc[pipeline_id];

        if(graph_obj_desc!=NULL)
        {
            graph_obj_desc->state = (uint32_t)state;
        }
    }
}

vx_node tivxGraphGetNode(vx_graph graph, uint32_t index)
{
    vx_node node = NULL;

    if (ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        if(vxIsGraphVerified(graph) != 0)
        {
            if( (index < TIVX_GRAPH_MAX_NODES) && (index < graph->num_nodes) )
            {
                node = graph->nodes[index];

                if(ownIsValidSpecificReference((vx_reference)node, (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
                {
                    /* valid node return it */
                }
                else
                {
                    node = NULL;
                    VX_PRINT(VX_ZONE_ERROR, "invalid node object @ index %d\n", index);
                }
            }
            else
            {
                node = NULL;
                VX_PRINT(VX_ZONE_ERROR, "node index %d not valid\n", index);
            }
        }
        else
        {
            node = NULL;
            VX_PRINT(VX_ZONE_ERROR, "graph not verified\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph object\n");
        node = NULL;
    }

    return node;
}
