/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*
 * Topological Sort Algorithm
 */

#include <vx_internal.h>

static inline void tivxGraphSortStackReset(tivx_graph_sort_context *context, uint16_t max_elems);
static inline vx_bool tivxGraphSortStackPush(tivx_graph_sort_context *context, vx_node elem);
static inline vx_bool tivxGraphSortStackPop(tivx_graph_sort_context *context, vx_node *elem);

static inline void tivxGraphSortStackReset(tivx_graph_sort_context *context, uint16_t max_elems)
{
    context->stack_top = 0;
    context->stack_max_elems = max_elems;
}

static inline vx_bool tivxGraphSortStackPush(tivx_graph_sort_context *context, vx_node elem)
{
    vx_bool status = vx_false_e;

    if(context->stack_top < context->stack_max_elems)
    {
        context->stack[context->stack_top] = elem;
        context->stack_top++;
        status = vx_true_e;
    }
    return status;
}

static inline vx_bool tivxGraphSortStackPop(tivx_graph_sort_context *context, vx_node *elem)
{
    vx_bool status = vx_false_e;

    if ((context->stack_top > 0) && (context->stack_top < TIVX_GRAPH_MAX_NODES))
    {
        *elem = context->stack[context->stack_top-1];
        context->stack_top--;
        status = vx_true_e;
    }
    return status;
}

void ownGraphTopologicalSort(tivx_graph_sort_context *context,
    vx_node *nodes, uint32_t num_nodes, vx_bool *has_cycle)
{
    uint16_t cur_node_idx, out_node_idx, num_out_nodes;
    vx_node cur_node, next_node;

    if (num_nodes < TIVX_GRAPH_MAX_NODES)
    {
        tivxGraphSortStackReset(context, num_nodes);

        for(cur_node_idx=0; cur_node_idx<num_nodes; cur_node_idx++)
        {
            cur_node = nodes[cur_node_idx];

            cur_node->incounter = ownNodeGetNumInNodes(cur_node);
            if(cur_node->incounter==0)
            {
                tivxGraphSortStackPush(context, cur_node);
            }
        }
        cur_node_idx = 0;
        while( tivxGraphSortStackPop(context, &cur_node) )
        {
            context->sorted_nodes[cur_node_idx] = cur_node;
            cur_node_idx++;
            num_out_nodes = ownNodeGetNumOutNodes(cur_node);
            for(out_node_idx=0; out_node_idx < num_out_nodes; out_node_idx++)
            {
                next_node = ownNodeGetNextNode(cur_node, out_node_idx);
                if(next_node)
                {
                    next_node->incounter--;
                    if(next_node->incounter==0)
                    {
                        tivxGraphSortStackPush(context, next_node);
                    }
                }
            }
        }
        if( cur_node_idx == num_nodes)
        {
            uint32_t tmp_cur_node_idx;

            for(tmp_cur_node_idx=0; tmp_cur_node_idx<num_nodes;
                tmp_cur_node_idx++)
            {
                nodes[tmp_cur_node_idx] =
                    context->sorted_nodes[tmp_cur_node_idx];
            }
            *has_cycle = vx_false_e;
        }
        else
        {
            *has_cycle = vx_true_e;
        }
    }
}
