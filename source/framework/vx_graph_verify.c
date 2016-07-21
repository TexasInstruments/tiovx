/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*
 * Since graph verify is involved logic, putting it in a different file.
 */

#include <vx_internal.h>

static vx_status ownGraphNodeKernelValidate(vx_graph graph)
{
    vx_node node;
    vx_status status = VX_SUCCESS;
    uint32_t i;

    for(i=0; i<graph->num_nodes; i++)
    {
        node = graph->nodes[i];

        status = ownNodeKernelValidate(node);
        if(status != VX_SUCCESS )
        {
            break;
        }
    }

    return status;
}

static vx_status ownGraphNodeKernelInit(vx_graph graph)
{
    vx_node node;
    vx_status status = VX_SUCCESS;
    uint32_t i;

    for(i=0; i<graph->num_nodes; i++)
    {
        node = graph->nodes[i];

        status = ownNodeKernelInit(node);
        if(status != VX_SUCCESS )
        {
            break;
        }
    }

    return status;
}

static vx_status ownGraphNodeKernelDeinit(vx_graph graph)
{
    vx_node node;
    vx_status status = VX_SUCCESS;
    uint32_t i;

    for(i=0; i<graph->num_nodes; i++)
    {
        node = graph->nodes[i];

        status = ownNodeKernelDeinit(node);
        if(status != VX_SUCCESS )
        {
            break;
        }
    }

    return status;
}

static vx_status ownGraphCalcInAndOutNodes(vx_graph graph)
{
    vx_node node_cur, node_next;
    uint32_t node_cur_idx, node_next_idx;
    uint32_t prm_cur_idx, prm_next_idx;
    uint32_t prm_cur_dir, prm_next_dir;

    for(node_cur_idx=0; node_cur_idx<graph->num_nodes; node_cur_idx++)
    {
        node_cur = graph->nodes[node_cur_idx];

        for(prm_cur_idx=0; prm_cur_idx<ownNodeGetNumParameters(node_cur); prm_cur_idx++)
        {
            prm_cur_dir = ownNodeGetParameterDir(node_cur, prm_cur_idx);

            if( prm_cur_dir == VX_OUTPUT || prm_cur_dir == VX_BIDIRECTIONAL)
            {
                /* for each output, see if it matches any node input data */
                for(node_next_idx=(node_cur_idx+1)%graph->num_nodes;
                    node_next_idx!=node_cur_idx;
                    node_next_idx=(node_next_idx+1)%graph->num_nodes)
                {
                    node_next = graph->nodes[node_next_idx];

                    for(prm_next_idx=0; prm_next_idx < ownNodeGetNumParameters(node_next); prm_next_idx++)
                    {
                        prm_next_dir = ownNodeGetParameterDir(node_next, prm_next_idx);

                        if( prm_next_dir == VX_INPUT || prm_next_dir == VX_BIDIRECTIONAL)
                        {
                            /* check if input data reference of next node is equal to
                               output data reference of current */
                            if(ownNodeGetParameterRef(node_cur, prm_cur_idx) == ownNodeGetParameterRef(node_next, prm_next_idx))
                            {
                                /* add node_next as output node for current node if not already added */
                                ownNodeAddOutNode(node_cur, node_next);

                                /* add node_current as input node for next node if not already added */
                                ownNodeAddInNode(node_next, node_cur);
                            }
                        }
                    }
                }
            }
        }
    }

    return VX_SUCCESS;
}

static vx_status ownGraphCalcHeadAndLeafNodes(vx_graph graph)
{
    vx_node node;
    uint32_t i;
    uint32_t num_in, num_out;

    graph->num_head_nodes = 0;
    graph->num_leaf_nodes = 0;

    for(i=0; i<graph->num_nodes; i++)
    {
        node = graph->nodes[i];

        num_in = ownNodeGetNumInNodes(node);
        num_out = ownNodeGetNumOutNodes(node);

        if(num_in==0)
        {
            graph->head_nodes[graph->num_head_nodes] = node;
            graph->num_head_nodes++;
        }
        if(num_out==0)
        {
            graph->leaf_nodes[graph->num_leaf_nodes] = node;
            graph->num_leaf_nodes++;
        }
    }

    return VX_SUCCESS;
}

static vx_status ownGraphAllocateDataObjects(vx_graph graph)
{
    vx_node node_cur;
    uint32_t node_cur_idx;
    uint32_t prm_cur_idx;
    vx_status status = VX_SUCCESS;
    vx_reference ref;

    for(node_cur_idx=0; node_cur_idx<graph->num_nodes; node_cur_idx++)
    {
        node_cur = graph->nodes[node_cur_idx];

        for(prm_cur_idx=0; prm_cur_idx<ownNodeGetNumParameters(node_cur); prm_cur_idx++)
        {
            ref = ownNodeGetParameterRef(node_cur, prm_cur_idx);

            /* alloc memory for data reference, if not already allocated
             * Its ok to call this multiple times for the same reference
             * memory gets allocated only once
             */
            status = ownReferenceAllocMem(ref);

            if(status != VX_SUCCESS)
            {
                break;
            }
        }
        if(status != VX_SUCCESS)
        {
            break;
        }
    }
    return status;
}

static vx_status ownGraphNodeCreateCompletionEvents(vx_graph graph)
{
    vx_node node;
    uint32_t i;
    vx_status status = VX_SUCCESS;

    for(i=0; i<graph->num_leaf_nodes; i++)
    {
        node = graph->leaf_nodes[i];

        status = ownNodeCreateCompletionEvent(node);
        if(status != VX_SUCCESS)
        {
            break;
        }
    }

    return status;
}

static vx_status ownGraphCreateNodeCallbackCommands(vx_graph graph)
{
    vx_node node;
    uint32_t i;
    vx_status status = VX_SUCCESS;

    for(i=0; i<graph->num_nodes; i++)
    {
        node = graph->nodes[i];

        status = ownNodeCreateUserCallbackCommand(node);
        if(status != VX_SUCCESS)
        {
            break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxVerifyGraph(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    if(graph && ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH))
    {
        ownReferenceLock(&graph->base);

        if(graph->state == VX_GRAPH_STATE_UNVERIFIED)
        {
            /* Call validate function for each node
             * If validation fails then return with error
             * No resources are allcoated in this step
             */
            status = ownGraphNodeKernelValidate(graph);

            if(status == VX_SUCCESS)
            {
                /* Find out nodes and in nodes for each node in the graph
                 * No resources are allcoated in this step
                 */
                status = ownGraphCalcInAndOutNodes(graph);
            }

            if(status == VX_SUCCESS)
            {
                /* Find head nodes and leaf nodes
                 * in graph
                 * No resources are allcoated in this step
                 */
                status = ownGraphCalcHeadAndLeafNodes(graph);
            }

            if(status == VX_SUCCESS)
            {
                /* Allocate memory associated with data objects of this graph
                 * Memory resources are allcoated in this step
                 * No need to free them in case of error, since they get free'ed during
                 * data object release
                 */
                status = ownGraphAllocateDataObjects(graph);
            }

            if(status == VX_SUCCESS)
            {
                /* Set completion event's with leaf nodes
                 * If case of any error these events gets free'ed during
                 * graph release
                 */
                status = ownGraphNodeCreateCompletionEvents(graph);
            }

            if(status == VX_SUCCESS)
            {
                /* Set node callback commands
                 * If case of any error these command are free'ed during
                 * graph release
                 */
                status = ownGraphCreateNodeCallbackCommands(graph);
            }

            if(status == VX_SUCCESS)
            {
                /* Call target kernel init for each node
                 * This results in message coomunication with target kernels
                 * Memory gets allocated which MUST be free'ed via target kernel
                 * deinit.
                 * kernel deinit called during node release
                 */
                status = ownGraphNodeKernelInit(graph);
            }

            if(status == VX_SUCCESS)
            {
                /* everything passed, now graph is verified */
                graph->verified = vx_true_e;
                graph->state = VX_GRAPH_STATE_VERIFIED;
            }

            if(status != VX_SUCCESS)
            {
                /* deinit kernel to recover resources */
                ownGraphNodeKernelDeinit(graph);
            }
        }

        ownReferenceUnlock(&graph->base);
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}
