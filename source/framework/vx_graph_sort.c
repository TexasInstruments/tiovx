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
