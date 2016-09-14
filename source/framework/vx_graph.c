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
/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <vx_internal.h>

static vx_status ownDestructGraph(vx_reference ref)
{
    vx_graph graph = (vx_graph)ref;

    while (graph->num_nodes)
    {
        vx_node node = graph->nodes[0];

        vxRemoveNode(&node);
    }

    return VX_SUCCESS;
}

vx_status ownResetGraphPerf(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    if (graph && ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
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
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    return status;
}

int32_t ownGraphGetFreeNodeIndex(vx_graph graph)
{
    int32_t free_index = -1;

    if (graph && ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
    {
        if(graph->num_nodes < TIVX_GRAPH_MAX_NODES)
        {
            free_index = graph->num_nodes;
        }
    }

    return free_index;
}

vx_status ownGraphAddNode(vx_graph graph, vx_node node, int32_t index)
{
    vx_status status = VX_SUCCESS;

    if (graph && ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
    {
        if(index < TIVX_GRAPH_MAX_NODES && index == graph->num_nodes)
        {
            /* index MUST be graph->num_nodes, since that is what is returned via
                ownGraphGetFreeNodeIndex() */
            ownIncrementReference(&node->base, VX_INTERNAL);
            graph->nodes[graph->num_nodes] = node;
            graph->num_nodes++;;
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status ownGraphRemoveNode(vx_graph graph, vx_node node)
{
    vx_status status = VX_FAILURE;
    uint32_t i;

    if (graph && ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
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
                break;
            }
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;

}

VX_API_ENTRY vx_graph VX_API_CALL vxCreateGraph(vx_context context)
{
    vx_graph  graph = NULL;
    uint32_t idx;

    if (ownIsValidContext(context) == vx_true_e)
    {
        graph = (vx_graph)ownCreateReference(context, VX_TYPE_GRAPH, VX_EXTERNAL, &context->base);
        if (vxGetStatus((vx_reference)graph) == VX_SUCCESS && graph->base.type == VX_TYPE_GRAPH)
        {
            graph->base.destructor_callback = ownDestructGraph;
            graph->base.release_callback = (tivx_reference_release_callback_f)vxReleaseGraph;

            graph->num_nodes = 0;
            graph->num_head_nodes = 0;
            graph->num_leaf_nodes = 0;
            graph->num_params = 0;

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
            graph->state = VX_GRAPH_STATE_UNVERIFIED;
        }
    }

    return (vx_graph)graph;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetGraphAttribute(vx_graph graph, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
    {
        status = VX_ERROR_NOT_SUPPORTED;
    }
    else
    {
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
                if (VX_CHECK_PARAM(ptr, size, vx_perf_t, 0x3))
                {
                    memcpy(ptr, &graph->perf, size);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_GRAPH_STATE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_status *)ptr = graph->state;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_GRAPH_NUMNODES:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = graph->num_nodes;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_GRAPH_NUMPARAMETERS:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = graph->num_params;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
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
            graph->num_params++;
            status = VX_SUCCESS;
        }
        else
        {
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
            graph->num_params++;
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    else
    {

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
    unsigned int i;
    vx_status status = VX_SUCCESS;
    vx_bool is_registered = vx_false_e;
    vx_bool is_full = vx_true_e;

    if(delay && ownIsValidSpecificReference((vx_reference)delay, VX_TYPE_DELAY))
    {
        if(graph && ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH))
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
                        break;
                    }
                }

                /* report error if there is no empty slots to register delay */
                if (is_full == vx_true_e)
                    status = VX_ERROR_NO_RESOURCES;
            }
        }
        else
        {
            status = VX_ERROR_INVALID_REFERENCE;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxProcessGraph(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    if(graph && ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH))
    {
        /* verify graph if not already verified */
        status = vxVerifyGraph(graph);
        if(status == VX_SUCCESS)
        {
            status = vxScheduleGraph(graph);
            if(status == VX_SUCCESS)
            {
                status = vxWaitGraph(graph);
            }
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxScheduleGraph(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    if(graph && ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH)
        && (
            graph->state == VX_GRAPH_STATE_VERIFIED
            ||
            graph->state == VX_GRAPH_STATE_COMPLETED
           )
        )
    {
        /* A new graph cannot be scheduled until current graph execution finishes
         */
        ownReferenceLock(&graph->base);

        /* trigger graph execution by scheduling the head nodes
         * Head nodes will trigger further nodes execution after their completion
         * This will continue until leaf nodes executes
         * After a leaf node executes, it will send a completion
         * event.
         * After all completion events are received, a graph is considered to have
         * executed
         */
        for(i=0; i<graph->num_head_nodes; i++)
        {
            ownNodeKernelSchedule(graph->head_nodes[i]);
        }

        graph->state = VX_GRAPH_STATE_RUNNING;
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxWaitGraph(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    if(graph && ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH)
        && graph->state == VX_GRAPH_STATE_RUNNING
        )
    {
        /* wait for completion events from all leaf nodes */
        for(i=0; i<graph->num_leaf_nodes; i++)
        {
            ownNodeWaitCompletionEvent(graph->leaf_nodes[i]);
        }

        graph->state = VX_GRAPH_STATE_COMPLETED;

        ownReferenceUnlock(&graph->base);
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}
