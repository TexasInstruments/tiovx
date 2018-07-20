/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */



#include <vx_internal.h>

static vx_status ownDestructGraph(vx_reference ref);
static vx_status ownResetGraphPerf(vx_graph graph);

static vx_status ownDestructGraph(vx_reference ref)
{
    vx_graph graph = (vx_graph)ref;

    {
        uint32_t i;

        for(i=0; i<graph->num_params; i++)
        {
            if(graph->parameters[i].queue_enable && graph->parameters[i].data_ref_queue != NULL)
            {
                tivxDataRefQueueRelease(&graph->parameters[i].data_ref_queue);
            }
        }
        for(i=0; i<graph->num_data_ref_q; i++)
        {
            if(graph->data_ref_q_list[i].data_ref_queue != NULL)
            {
                tivxDataRefQueueRelease(&graph->data_ref_q_list[i].data_ref_queue);
            }
            if(graph->data_ref_q_list[i].num_buf > 1)
            {
                vx_bool is_replicated
                            = ownNodeIsPrmReplicated(graph->data_ref_q_list[i].node, graph->data_ref_q_list[i].index);
                uint32_t buf_id;

                /* additional references allocated during verify, release them */
                for(buf_id=1; buf_id<graph->data_ref_q_list[i].num_buf; buf_id++)
                {
                    vx_reference data_ref = graph->data_ref_q_list[i].refs_list[buf_id];

                    if(is_replicated)
                    {
                        data_ref = data_ref->scope;
                    }

                    vxReleaseReference(&data_ref);

                    graph->data_ref_q_list[i].refs_list[buf_id] = NULL;
                }
            }
        }
        for(i=0; i<graph->num_delay_data_ref_q; i++)
        {
            if(graph->delay_data_ref_q_list[i].data_ref_queue != NULL)
            {
                tivxDataRefQueueRelease(&graph->delay_data_ref_q_list[i].data_ref_queue);
            }
        }
    }

    while (graph->num_nodes)
    {
        vx_node node = graph->nodes[0];

        vxRemoveNode(&node);
    }

    ownGraphDeleteQueues(graph);
    ownGraphFreeObjDesc(graph);
    tivxEventDelete(&graph->all_graph_completed_event);

    return VX_SUCCESS;
}

static vx_status ownResetGraphPerf(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    if ((NULL != graph) &&
        (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e) )
    {
        graph->perf.tmp = 0;
        graph->perf.beg = 0;
        graph->perf.end = 0;
        graph->perf.sum = 0;
        graph->perf.avg = 0;
        graph->perf.min = 0xFFFFFFFFFFFFFFFFULL;
        graph->perf.num = 0;
        graph->perf.max = 0;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "ownResetGraphPerf: invalid graph object\n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    return status;
}

vx_status ownUpdateGraphPerf(vx_graph graph, uint32_t pipeline_id)
{
    vx_status status = VX_SUCCESS;

    if ((NULL != graph) &&
        (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) ==
            vx_true_e) &&
            pipeline_id < graph->pipeline_depth)
    {
        uint64_t beg_time, end_time;
        tivx_obj_desc_graph_t *obj_desc;

        obj_desc = graph->obj_desc[pipeline_id];

        tivx_uint32_to_uint64(&beg_time, obj_desc->exe_time_beg_h, obj_desc->exe_time_beg_l);
        tivx_uint32_to_uint64(&end_time, obj_desc->exe_time_end_h, obj_desc->exe_time_end_l);

        graph->perf.beg = beg_time*1000; /* convert to nano secs */
        graph->perf.end = end_time*1000; /* convert to nano secs */
        graph->perf.tmp = (end_time - beg_time)*1000; /* convert to nano secs */
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
        VX_PRINT(VX_ZONE_ERROR, "ownUpdateGraphPerf: invalid graph object\n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    return status;
}

int32_t ownGraphGetFreeNodeIndex(vx_graph graph)
{
    int32_t free_index = -(int32_t)1;

    if ((NULL != graph) &&
        (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e) )
    {
        if(graph->num_nodes < TIVX_GRAPH_MAX_NODES)
        {
            free_index = graph->num_nodes;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "ownGraphGetFreeNodeIndex: Max nodes per graph (%d) has been exceeded\n", TIVX_GRAPH_MAX_NODES);
        }
    }

    return free_index;
}

vx_status ownGraphAddNode(vx_graph graph, vx_node node, int32_t index)
{
    vx_status status = VX_SUCCESS;

    if ((NULL != graph) &&
        (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e) )
    {
        if( (index < TIVX_GRAPH_MAX_NODES) && (index == graph->num_nodes) )
        {
            /* index MUST be graph->num_nodes, since that is what is returned via
                ownGraphGetFreeNodeIndex() */
            ownIncrementReference(&node->base, VX_INTERNAL);
            graph->nodes[graph->num_nodes] = node;
            graph->num_nodes++;
            ownGraphSetReverify(graph);
            tivxLogSetResourceUsedValue("TIVX_GRAPH_MAX_NODES", graph->num_nodes);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "ownGraphAddNode: invalid graph index\n");
            VX_PRINT(VX_ZONE_ERROR, "ownGraphAddNode: May need to increase the value of TIVX_GRAPH_MAX_NODES in tiovx/include/tivx_config.h\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "ownGraphAddNode: invalid graph object\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status ownGraphRemoveNode(vx_graph graph, vx_node node)
{
    vx_status status = VX_FAILURE;
    uint32_t i;

    if ((NULL != graph) &&
        (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e) )
    {
        /* remove node from head nodes and leaf nodes if found */
        for(i=0; i < graph->num_head_nodes; i++)
        {
            if(node==graph->head_nodes[i])
            {
                /* swap with last entry to make the list compact */
                graph->head_nodes[i] = graph->head_nodes[graph->num_head_nodes-1];
                graph->head_nodes[graph->num_head_nodes-1] = NULL;
                graph->num_head_nodes--;
                break;
            }
        }

        for(i=0; i < graph->num_leaf_nodes; i++)
        {
            if(node==graph->leaf_nodes[i])
            {
                /* swap with last entry to make the list compact */
                graph->leaf_nodes[i] = graph->leaf_nodes[graph->num_leaf_nodes-1];
                graph->head_nodes[graph->num_leaf_nodes-1] = NULL;
                graph->num_leaf_nodes--;
                break;
            }
        }

        for(i=0; i < graph->num_nodes; i++)
        {
            if(node==graph->nodes[i])
            {
                /* swap with last entry to make the list compact */
                graph->nodes[i] = graph->nodes[graph->num_nodes-1];
                graph->nodes[graph->num_nodes-1] = NULL;
                graph->num_nodes--;
                ownReleaseReferenceInt((vx_reference *)&node, VX_TYPE_NODE, VX_INTERNAL, NULL);
                status = VX_SUCCESS;
                ownGraphSetReverify(graph);
                break;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "ownGraphRemoveNode: invalid graph object\n");
        status = VX_ERROR_INVALID_REFERENCE;
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
    vx_status status = VX_SUCCESS;

    if (ownIsValidContext(context) == vx_true_e)
    {
        graph = (vx_graph)ownCreateReference(context, VX_TYPE_GRAPH, VX_EXTERNAL, &context->base);
        if ( (vxGetStatus((vx_reference)graph) == VX_SUCCESS) && (graph->base.type == VX_TYPE_GRAPH) )
        {
            graph->base.destructor_callback = &ownDestructGraph;
            graph->base.release_callback = (tivx_reference_release_callback_f)&vxReleaseGraph;

            graph->num_nodes = 0;
            graph->num_head_nodes = 0;
            graph->num_leaf_nodes = 0;
            graph->num_params = 0;
            graph->pipeline_depth = 1;
            graph->is_enable_send_complete_event = vx_false_e;
            graph->schedule_mode = VX_GRAPH_SCHEDULE_MODE_NORMAL;
            graph->schedule_pending_count = 0;
            graph->submitted_count = 0;
            graph->num_data_ref = 0;
            graph->num_data_ref_q = 0;
            graph->num_delay_data_ref_q = 0;


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

            graph->verified = vx_false_e;
            graph->reverify = vx_false_e;
            graph->state = VX_GRAPH_STATE_UNVERIFIED;

            status = tivxEventCreate(&graph->all_graph_completed_event);
            if(status==VX_SUCCESS)
            {
                status = ownGraphCreateQueues(graph);
            }
            if(status!=VX_SUCCESS)
            {
                vxReleaseGraph(&graph);

                VX_PRINT(VX_ZONE_ERROR, "vxCreateGraph: Could not create graph\n");
                graph = (vx_graph)ownGetErrorObject(context, VX_ERROR_NO_RESOURCES);
            }

        }
    }

    return (vx_graph)graph;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetGraphAttribute(vx_graph graph, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetGraphAttribute: not supported\n");
        status = VX_ERROR_NOT_SUPPORTED;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetGraphAttribute: invalid graph object\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryGraph(vx_graph graph, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (ownIsValidReference(&graph->base) == vx_true_e)
    {
        switch (attribute)
        {
            case VX_GRAPH_PERFORMANCE:
                if (VX_CHECK_PARAM(ptr, size, vx_perf_t, 0x3U))
                {
                    memcpy(ptr, &graph->perf, size);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryGraph: query graph performance failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_GRAPH_STATE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_status *)ptr = graph->state;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryGraph: query graph state failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_GRAPH_NUMNODES:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = graph->num_nodes;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryGraph: query graph number of nodes failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_GRAPH_NUMPARAMETERS:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = graph->num_params;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryGraph: query graph number of parameters failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "vxQueryGraph: invalid attribute\n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryGraph: invalid graph reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseGraph(vx_graph *g)
{
    return ownReleaseReferenceInt((vx_reference *)g, VX_TYPE_GRAPH, VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_status VX_API_CALL vxAddParameterToGraph(vx_graph graph, vx_parameter param)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if ((ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e) &&
        (ownIsValidSpecificReference(&param->base, VX_TYPE_PARAMETER) == vx_true_e))
    {
        if(graph->num_params < TIVX_GRAPH_MAX_PARAMS)
        {
            graph->parameters[graph->num_params].node = param->node;
            graph->parameters[graph->num_params].index = param->index;
            graph->parameters[graph->num_params].queue_enable = vx_false_e;
            graph->parameters[graph->num_params].is_enable_send_ref_consumed_event = vx_false_e;
            graph->parameters[graph->num_params].data_ref_queue = NULL;
            graph->parameters[graph->num_params].num_buf = 0;
            graph->num_params++;
            tivxLogSetResourceUsedValue("TIVX_GRAPH_MAX_PARAMS", graph->num_params);
            status = VX_SUCCESS;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "vxAddParameterToGraph: number of graph parameters greater than maximum allowed\n");
            VX_PRINT(VX_ZONE_ERROR, "vxAddParameterToGraph: May need to increase the value of TIVX_GRAPH_MAX_PARAMS in tiovx/include/tivx_config.h\n");
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    else if ((ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e) &&
              (ownIsValidSpecificReference(&param->base, VX_TYPE_PARAMETER) == vx_false_e))
    {
        if(graph->num_params < TIVX_GRAPH_MAX_PARAMS)
        {
            /* insert an empty parameter */
            graph->parameters[graph->num_params].node = NULL;
            graph->parameters[graph->num_params].index = 0;
            graph->parameters[graph->num_params].queue_enable = vx_false_e;
            graph->num_params++;
            tivxLogSetResourceUsedValue("TIVX_GRAPH_MAX_PARAMS", graph->num_params);
            status = VX_SUCCESS;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "vxAddParameterToGraph: number of graph parameters greater than maximum allowed\n");
            VX_PRINT(VX_ZONE_ERROR, "vxAddParameterToGraph: May need to increase the value of TIVX_GRAPH_MAX_PARAMS in tiovx/include/tivx_config.h\n");
            status = VX_ERROR_NO_RESOURCES;
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
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
    {
        if ((index < TIVX_GRAPH_MAX_PARAMS) && (index < graph->num_params))
        {
            status = vxSetParameterByIndex(graph->parameters[index].node,
                                           graph->parameters[index].index,
                                           value);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "vxSetGraphParameterByIndex: index greater than number of parameters allowed\n");
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    return status;
}

VX_API_ENTRY vx_parameter VX_API_CALL vxGetGraphParameterByIndex(vx_graph graph, vx_uint32 index)
{
    vx_parameter parameter = NULL;
    if (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
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
    vx_bool verified = vx_false_e;
    if (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
    {
        verified = graph->verified;
    }
    return verified;
}

VX_API_ENTRY vx_status VX_API_CALL vxRegisterAutoAging(vx_graph graph, vx_delay delay)
{
    uint8_t i;
    vx_status status = VX_SUCCESS;
    vx_bool is_registered = vx_false_e;
    vx_bool is_full = vx_true_e;

    if((NULL != delay) &&
       (ownIsValidSpecificReference((vx_reference)delay, VX_TYPE_DELAY)))
    {
        if((NULL != graph) &&
           (ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH)))
        {
            /* check if this particular delay is already registered in the graph */
            for (i = 0; i < TIVX_GRAPH_MAX_DELAYS; i++)
            {
                if (graph->delays[i] == delay)
                {
                    is_registered = vx_true_e;
                    break;
                }
            }

            /* if not regisered yet, find the first empty slot and register delay */
            if (is_registered == vx_false_e)
            {
                for (i = 0; i < TIVX_GRAPH_MAX_DELAYS; i++)
                {
                    if (graph->delays[i] == NULL)
                    {
                        is_full = vx_false_e;
                        graph->delays[i] = delay;
                        tivxLogSetResourceUsedValue("TIVX_GRAPH_MAX_DELAYS", i+1);
                        break;
                    }
                }

                /* report error if there is no empty slots to register delay */
                if (is_full == vx_true_e)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxRegisterAutoAging: no empty slots to register delay\n");
                    VX_PRINT(VX_ZONE_ERROR, "vxRegisterAutoAging: May need to increase the value of TIVX_GRAPH_MAX_DELAYS in tiovx/include/tivx_config.h\n");
                    status = VX_ERROR_NO_RESOURCES;
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "vxRegisterAutoAging: invalid graph reference\n");
            status = VX_ERROR_INVALID_REFERENCE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxRegisterAutoAging: invalid delay reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxProcessGraph(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    if((NULL != graph) &&
       (ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH)))
    {
        status = vxScheduleGraph(graph);
        if(status == VX_SUCCESS)
        {
            status = vxWaitGraph(graph);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "vxProcessGraph: schedule graph failed\n");
        }

        if(status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxProcessGraph: wait graph failed\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxProcessGraph: invalid graph reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxScheduleGraph(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    if((NULL != graph) &&
       (ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH)))
    {
        if(graph->schedule_mode==VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO)
        {
            status = VX_ERROR_NOT_SUPPORTED;
            VX_PRINT(VX_ZONE_ERROR, "vxScheduleGraph: not supported for VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO\n");
        }
        else
        {
            if(vx_false_e == vxIsGraphVerified(graph))
            {
                /* verify graph if not already verified */
                status = vxVerifyGraph(graph);
            }

            if ((status == VX_SUCCESS)
                && ( (graph->state == VX_GRAPH_STATE_VERIFIED) ||
                     (graph->state == VX_GRAPH_STATE_COMPLETED) ||
                     (graph->state == VX_GRAPH_STATE_ABANDONED)
                    ))
            {
                if(graph->schedule_mode==VX_GRAPH_SCHEDULE_MODE_NORMAL)
                {
                    /* schedule graph one time */
                    ownGraphScheduleGraph(graph, 1);
                }
                else
                if(graph->schedule_mode==VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL)
                {
                    uint32_t num_schedule = ownGraphGetNumSchedule(graph);

                    if(num_schedule>0)
                    {
                        /* schedule graph 'num_schedule' times */
                        ownGraphScheduleGraph(graph, num_schedule);
                    }
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "vxScheduleGraph: graph is not in a state required to be scheduled\n");
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxScheduleGraph: invalid graph reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxWaitGraph(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    if((NULL!= graph) &&
        (ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH) ==
            vx_true_e))
    {
        if (graph->state == VX_GRAPH_STATE_RUNNING
           || graph->state == VX_GRAPH_STATE_COMPLETED
           || graph->state == VX_GRAPH_STATE_ABANDONED)
        {
            /* wait for all previous graph executions to finish
             *
             */
            status = tivxEventWait(graph->all_graph_completed_event, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

            if(status == VX_SUCCESS)
            {
                if(graph->state == VX_GRAPH_STATE_ABANDONED)
                {
                    status = VX_ERROR_GRAPH_ABANDONED;
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "vxWaitGraph: graph not in expected state\n");
            status = VX_ERROR_NOT_SUPPORTED;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxWaitGraph: invalid graph reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

void ownGraphSetReverify(vx_graph graph)
{
    if(graph)
    {
        graph->reverify = graph->verified;
        graph->verified = vx_false_e;
        graph->state = VX_GRAPH_STATE_UNVERIFIED;
    }
}

void ownSendGraphCompletedEvent(vx_graph graph)
{
    if(graph!=NULL && graph->base.context!=NULL)
    {
        if(graph->is_enable_send_complete_event)
        {
            uint64_t timestamp;

            timestamp = tivxPlatformGetTimeInUsecs()*1000; /* in nano-secs */

            tivxEventQueueAddEvent(&graph->base.context->event_queue,
                        VX_EVENT_GRAPH_COMPLETED, timestamp,
                        (uintptr_t)graph, (uintptr_t)0);
        }
    }
}

vx_status ownGraphRegisterCompletionEvent(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
    {
        if (graph->verified == vx_true_e)
        {
            VX_PRINT(VX_ZONE_ERROR, "Cannot register event on verified graph\n");
            status = VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            graph->is_enable_send_complete_event = vx_true_e;

            VX_PRINT(VX_ZONE_INFO, "Enabling completion event at graph [%s]\n", graph->base.name);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

vx_status ownGraphRegisterParameterConsumedEvent(vx_graph graph, uint32_t graph_parameter_index)
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
    {
        if (graph->verified == vx_true_e)
        {
            VX_PRINT(VX_ZONE_ERROR, "Cannot register event on verified graph\n");
            status = VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            if(graph_parameter_index < graph->num_params)
            {
                graph->parameters[graph_parameter_index].is_enable_send_ref_consumed_event
                    = vx_true_e;
                VX_PRINT(VX_ZONE_INFO, "Enabling parameter ref consumed event at graph [%s], param %d\n",
                    graph->base.name, graph_parameter_index);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
                status = VX_ERROR_INVALID_PARAMETERS;

            }

        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

void ownSetGraphState(vx_graph graph, uint32_t pipeline_id, vx_enum state)
{
    if(graph != NULL && pipeline_id < graph->pipeline_depth )
    {
        tivx_obj_desc_graph_t *graph_obj_desc = graph->obj_desc[pipeline_id];

        if(graph_obj_desc!=NULL)
        {
            graph_obj_desc->state = state;
        }
    }
}

