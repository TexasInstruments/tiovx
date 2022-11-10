/*
*
* Copyright (c) 2019 Texas Instruments Incorporated
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

static vx_status ownGraphCalcEdgeList(vx_graph graph, tivx_super_node super_node);
static vx_status ownGraphSuperNodeCheckTarget(tivx_super_node super_node);

/* While we are here, update the graph connection for the run-time processing dependencies */
/* called during graph verify if there are any super nodes
 * This function
 * 1. creates the super node edge_list, and
 * 2. restructures dependences (inserts super node in place of its nodes in graph dependency list)
 */
static vx_status ownGraphCalcEdgeList(vx_graph graph, tivx_super_node super_node)
{
    vx_node node_cur, node_next, node_next_tmp;
    uint32_t node_cur_idx, node_next_idx, i;
    uint32_t prm_cur_idx, prm_next_idx;
    uint32_t prm_cur_dir, prm_next_dir;
    uint32_t cnt = 0;
    vx_reference ref1, ref2;
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_super_node_t *obj_desc = (tivx_obj_desc_super_node_t *)super_node->base.obj_desc;;
    vx_bool found;
    vx_reference found_external_refs[TIVX_SUPER_NODE_MAX_EDGES];
    uint32_t num_found_external_refs = 0;

    /* For each node in the graph */
    for(node_cur_idx=0; node_cur_idx<graph->num_nodes; node_cur_idx++)
    {
        node_cur = graph->nodes[node_cur_idx];

        /* The ones in the super node */
        if ((node_cur->super_node == super_node) && ((vx_bool)vx_false_e == node_cur->is_super_node))
        {
            /* For each parameter */
            for(prm_cur_idx=0; prm_cur_idx<ownNodeGetNumParameters(node_cur); prm_cur_idx++)
            {
                /* if an input/output is null it will be deleted from the supernode edge list
                 * if the bam edge list ever needs NULL assigned to unused ports,
                 * this logic might need modifications to encode the NULL using a
                 * constant like TIVX_OBJ_DESC_NULL, consequently the tivxKernelSupernodeCreate()
                 * method will need modifications
                 */
                if (node_cur->parameters[prm_cur_idx] != NULL)
                {
                    if(node_cur->parameters[prm_cur_idx]->type == (vx_enum)VX_TYPE_IMAGE )
                    {
                        prm_cur_dir = (uint32_t)ownNodeGetParameterDir(node_cur, prm_cur_idx);

                        ref1 = ownNodeGetParameterRef(node_cur, prm_cur_idx);

                        /* Look for dangling or external inputs to the supernode */
                        if((vx_enum)prm_cur_dir == (vx_enum)VX_INPUT)
                        {
                            found = (vx_bool)vx_false_e;

                            /* for each input, see if it matches any node output data */
                            for(node_next_idx=(node_cur_idx+1U)%graph->num_nodes;
                                node_next_idx!=node_cur_idx;
                                node_next_idx=(node_next_idx+1U)%graph->num_nodes)
                            {
                                node_next = graph->nodes[node_next_idx];

                                for(prm_next_idx=0; prm_next_idx < ownNodeGetNumParameters(node_next); prm_next_idx++)
                                {
                                    prm_next_dir = (uint32_t)ownNodeGetParameterDir(node_next, prm_next_idx);

                                    ref2 = ownNodeGetParameterRef(node_next, prm_next_idx);

                                    if(ref2 != NULL)
                                    {
                                        if( (vx_enum)prm_next_dir == (vx_enum)VX_OUTPUT )
                                        {
                                            /* check if output data reference of next node is equal to
                                               input data reference of current */
                                            if( ownGraphCheckIsRefMatch(graph, ref1, ref2) != 0)
                                            {
                                                /* Node to Node edge */
                                                if(node_next->super_node == super_node)
                                                {
                                                    /* Edge is internal to super node */
                                                    /* Since we are looking for external edge inputs now, skip this edge for now */
                                                    /* It will be added when we scan supernode node outputs later */
                                                    found = (vx_bool)vx_true_e;
                                                }
                                                else
                                                {
                                                    /* Edge is external to super node, we can add edge to edge list */

                                                    /* Check to see if node_next is part of a supernode, if so use the supernode node */
                                                    if(node_next->super_node == NULL)
                                                    {
                                                        node_next_tmp = node_next;
                                                    }
                                                    else
                                                    {
                                                        node_next_tmp = node_next->super_node->node;
                                                    }

                                                    /* add node_next_tmp as input node for super node if not already added */
                                                    status = ownNodeAddInNode(super_node->node, node_next_tmp);

                                                    if(status == (vx_status)VX_SUCCESS)
                                                    {
                                                        /* replace super node as output node for next node and remove duplicates */
                                                        status = ownNodeReplaceOutNode(node_next, node_cur, super_node->node);
                                                        if (status != (vx_status)VX_SUCCESS)
                                                        {
                                                            VX_PRINT(VX_ZONE_ERROR,"Replace super node as another node's output failed\n");
                                                        }
                                                    }
                                                    else
                                                    {
                                                        VX_PRINT(VX_ZONE_ERROR,"Add in node for super node failed\n");
                                                    }
                                                }

                                                /* Since we found the producer of this reference, no need to look more */
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                            if (found == (vx_bool)vx_false_e)
                            {
                                if( TIVX_SUPER_NODE_MAX_EDGES > cnt)
                                {
                                    /* We found a external input edge */
                                    for(i=0; i<num_found_external_refs; i++)
                                    {
                                        if( ref1 == found_external_refs[i])
                                        {
                                            obj_desc->edge_list[cnt].src_node_prm_idx = (uint16_t)i;
                                            break;
                                        }
                                    }
                                    if((i + 1U) > num_found_external_refs)
                                    {
                                        found_external_refs[num_found_external_refs] = ref1;
                                        obj_desc->edge_list[cnt].src_node_prm_idx = (uint16_t)num_found_external_refs;
                                        num_found_external_refs++;
                                    }

                                    obj_desc->edge_list[cnt].src_node_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
                                    obj_desc->edge_list[cnt].dst_node_obj_desc_id = node_cur->obj_desc[0]->base.obj_desc_id;
                                    obj_desc->edge_list[cnt].dst_node_prm_idx = (uint16_t)prm_cur_idx;
                                    cnt++;
                                }
                                else
                                {
                                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                                    VX_PRINT(VX_ZONE_ERROR, "number of edges in super node exceeds TIVX_SUPER_NODE_MAX_EDGES\n");
                                    VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_SUPER_NODE_MAX_EDGES in tiovx/include/TI/tivx_config.h\n");
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if ( (vx_status)VX_SUCCESS == status )
    {
        num_found_external_refs = 0;

        /* For each node in the graph */
        for(node_cur_idx=0; node_cur_idx<graph->num_nodes; node_cur_idx++)
        {
            node_cur = graph->nodes[node_cur_idx];

            /* The ones in the super node */
            if ((node_cur->super_node == super_node) && ((vx_bool)vx_false_e == node_cur->is_super_node))
            {
                /* For each parameter */
                for(prm_cur_idx=0; prm_cur_idx<ownNodeGetNumParameters(node_cur); prm_cur_idx++)
                {
                    if (node_cur->parameters[prm_cur_idx] != NULL)
                    {
                        if( node_cur->parameters[prm_cur_idx]->type == (vx_enum)VX_TYPE_IMAGE )
                        {
                            prm_cur_dir = (uint32_t)ownNodeGetParameterDir(node_cur, prm_cur_idx);

                            ref1 = ownNodeGetParameterRef(node_cur, prm_cur_idx);

                            if( ((vx_enum)prm_cur_dir == (vx_enum)VX_OUTPUT) || ((vx_enum)prm_cur_dir == (vx_enum)VX_BIDIRECTIONAL))
                            {
                                found = (vx_bool)vx_false_e;

                                /* for each output, see if it matches any node input data */
                                for(node_next_idx=(node_cur_idx+1U)%graph->num_nodes;
                                    node_next_idx!=node_cur_idx;
                                    node_next_idx=(node_next_idx+1U)%graph->num_nodes)
                                {
                                    node_next = graph->nodes[node_next_idx];

                                    for(prm_next_idx=0; prm_next_idx < ownNodeGetNumParameters(node_next); prm_next_idx++)
                                    {
                                        prm_next_dir = (uint32_t)ownNodeGetParameterDir(node_next, prm_next_idx);

                                        ref2 = ownNodeGetParameterRef(node_next, prm_next_idx);

                                        if(ref2 != NULL)
                                        {
                                            if( ((vx_enum)prm_next_dir == (vx_enum)VX_INPUT) || ((vx_enum)prm_next_dir == (vx_enum)VX_BIDIRECTIONAL) )
                                            {
                                                /* check if input data reference of next node is equal to
                                                   output data reference of current */
                                                if( ownGraphCheckIsRefMatch(graph, ref1, ref2) != 0)
                                                {
                                                    if( TIVX_SUPER_NODE_MAX_EDGES > cnt)
                                                    {
                                                        /* Node to Node edge */
                                                        if(node_next->super_node == super_node)
                                                        {
                                                            /* Edge is internal to super node */
                                                            found = (vx_bool)vx_true_e;
                                                            obj_desc->edge_list[cnt].src_node_obj_desc_id = node_cur->obj_desc[0]->base.obj_desc_id;;
                                                            obj_desc->edge_list[cnt].src_node_prm_idx = (uint16_t)prm_cur_idx;
                                                            obj_desc->edge_list[cnt].dst_node_obj_desc_id = node_next->obj_desc[0]->base.obj_desc_id;
                                                            obj_desc->edge_list[cnt].dst_node_prm_idx = (uint16_t)prm_next_idx;
                                                            cnt++;
                                                        }
                                                        else
                                                        {
                                                            /* Edge is external to super node */
                                                            /* We found a external output edge */
                                                            for(i=0; i<num_found_external_refs; i++)
                                                            {
                                                                if( ref1 == found_external_refs[i])
                                                                {
                                                                    break;
                                                                }
                                                            }
                                                            if((i + 1U) > num_found_external_refs)
                                                            {
                                                                found_external_refs[num_found_external_refs] = ref1;
                                                                obj_desc->edge_list[cnt].src_node_obj_desc_id = node_cur->obj_desc[0]->base.obj_desc_id;;
                                                                obj_desc->edge_list[cnt].src_node_prm_idx = (uint16_t)prm_cur_idx;
                                                                obj_desc->edge_list[cnt].dst_node_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
                                                                obj_desc->edge_list[cnt].dst_node_prm_idx = (uint16_t)num_found_external_refs;
                                                                cnt++;
                                                                num_found_external_refs++;
                                                            }

                                                            /* add node_next as output node for super node if not already added */

                                                            /* Check to see if node_next is part of a supernode, if so use the supernode node */
                                                            if(node_next->super_node == NULL)
                                                            {
                                                                node_next_tmp = node_next;
                                                            }
                                                            else
                                                            {
                                                                node_next_tmp = node_next->super_node->node;
                                                            }

                                                            status = ownNodeAddOutNode(super_node->node, node_next_tmp);

                                                            if(status == (vx_status)VX_SUCCESS)
                                                            {
                                                                /* replace super node as input node for next node and remove duplicates */
                                                                status = ownNodeReplaceInNode(node_next, node_cur, super_node->node);
                                                                if (status != (vx_status)VX_SUCCESS)
                                                                {
                                                                    VX_PRINT(VX_ZONE_ERROR,"Replace super node as another node's input failed\n");
                                                                }
                                                            }
                                                            else
                                                            {
                                                                VX_PRINT(VX_ZONE_ERROR,"Add out node for super node failed\n");
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        status = (vx_status)VX_ERROR_NO_RESOURCES;
                                                        VX_PRINT(VX_ZONE_ERROR, "number of edges in super node exceeds TIVX_SUPER_NODE_MAX_EDGES\n");
                                                        VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_SUPER_NODE_MAX_EDGES in tiovx/include/TI/tivx_config.h\n");
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                if (found == (vx_bool)vx_false_e)
                                {
                                    if( TIVX_SUPER_NODE_MAX_EDGES > cnt)
                                    {
                                        /* We found a external output edge */
                                        for(i=0; i<num_found_external_refs; i++)
                                        {
                                            if( ref1 == found_external_refs[i])
                                            {
                                                break;
                                            }
                                        }
                                        if((i + 1U) > num_found_external_refs)
                                        {
                                            found_external_refs[num_found_external_refs] = ref1;
                                            obj_desc->edge_list[cnt].src_node_obj_desc_id = node_cur->obj_desc[0]->base.obj_desc_id;
                                            obj_desc->edge_list[cnt].src_node_prm_idx = (uint16_t)prm_cur_idx;
                                            obj_desc->edge_list[cnt].dst_node_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
                                            obj_desc->edge_list[cnt].dst_node_prm_idx = (uint16_t)num_found_external_refs;
                                            num_found_external_refs++;
                                            cnt++;
                                        }
                                    }
                                    else
                                    {
                                        status = (vx_status)VX_ERROR_NO_RESOURCES;
                                        VX_PRINT(VX_ZONE_ERROR, "number of edges in super node exceeds TIVX_SUPER_NODE_MAX_EDGES\n");
                                        VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_SUPER_NODE_MAX_EDGES in tiovx/include/TI/tivx_config.h\n");
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        obj_desc->num_edges = (uint16_t)cnt;
        ownLogSetResourceUsedValue("TIVX_SUPER_NODE_MAX_EDGES", (uint16_t)cnt);
    }

    return status;
}

static vx_status ownGraphSuperNodeCheckTarget(tivx_super_node super_node)
{
    uint32_t i, num_nodes_in_supernode, target_id;
    vx_status status = (vx_status)VX_SUCCESS;

    num_nodes_in_supernode =
        ((tivx_obj_desc_super_node_t *)super_node->base.obj_desc)->num_nodes;

    if(num_nodes_in_supernode > 0U)
    {
        target_id =
            ((tivx_obj_desc_node_t *)super_node->nodes[0]->obj_desc[0])->target_id;
    }

    for(i=1; i < num_nodes_in_supernode; i++)
    {
        if(target_id != ((tivx_obj_desc_node_t *)super_node->nodes[i]->obj_desc[0])->target_id)
        {
            status = (vx_status)VX_FAILURE;
            break;
        }
    }
    return status;
}

vx_status ownGraphSuperNodeConfigure(vx_graph graph)
{
    uint32_t i, j, cnt, num_nodes_in_supernode;
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_super_node super_node;
    tivx_obj_desc_super_node_t *obj_desc = NULL;
    tivx_obj_desc_node_t *node_obj_desc = NULL;

    for(i=0; (i < graph->num_supernodes) && (status == (vx_status)VX_SUCCESS); i++)
    {
        super_node = graph->supernodes[i];
        obj_desc = (tivx_obj_desc_super_node_t *)super_node->base.obj_desc;
        num_nodes_in_supernode = obj_desc->num_nodes;

        cnt = 0;

        /* Copy topological sorted list related to supernode into supernode list */
        for(j=0; j<graph->num_nodes; j++)
        {
            if(((vx_bool)vx_false_e == graph->nodes[j]->is_super_node) &&
               (super_node == graph->nodes[j]->super_node))
            {
                node_obj_desc = (tivx_obj_desc_node_t *)graph->nodes[j]->obj_desc[0];

                /* Update both object descriptor and host structure with sorted nodes */
                obj_desc->node_obj_desc_id[cnt] = node_obj_desc->base.obj_desc_id;
                super_node->nodes[cnt] = graph->nodes[j];
                cnt++;
            }
        }

        /* Check for number mismatch error */
        if(num_nodes_in_supernode != cnt)
        {
            VX_PRINT(VX_ZONE_ERROR,"Supernode node count not equal to number of nodes in graph associated with supernode\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {

            /* Check for target mismatch error */
            status = ownGraphSuperNodeCheckTarget(super_node);

            if(status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,"Supernode [%d] does not have the same target of all nodes within it\n", i);
            }
            else
            {
                /* Check for continuity for each node in super node */
                vx_bool is_continuous = (vx_bool)vx_false_e;

                ownContextLock(graph->base.context);

                if (num_nodes_in_supernode < TIVX_SUPER_NODE_MAX_NODES)
                {
                    ownGraphCheckContinuityOfSupernode(
                                &graph->base.context->graph_sort_context,
                                super_node,
                                num_nodes_in_supernode,
                                &is_continuous);
                }
                else
                {
                    status = (vx_status)VX_FAILURE;
                }

                ownContextUnlock(graph->base.context);

                if(is_continuous == (vx_bool)vx_false_e)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Supernode [%d] does not have continuity of all nodes within it\n", i);
                    status = (vx_status)VX_FAILURE;
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                cnt = 0;

                /* Create super node edge list and
                 * Update graph node execution dependencies to point to/from supernodes */
                status = ownGraphCalcEdgeList(graph, super_node);

                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Supernode [%d] failed ownGraphCalcEdgeList\n", i);
                }
            }
        }
    }

    if((status == (vx_status)VX_SUCCESS) && (graph->num_supernodes > 0U))
    {
        vx_bool has_cycle;

        ownContextLock(graph->base.context);

        /* Check for cycles again now that supernode execution dependences
         * are inserted
         */
        ownGraphCheckSupernodeCycles(
                &graph->base.context->graph_sort_context,
                graph->nodes,
                graph->num_nodes,
                &has_cycle);

        ownContextUnlock(graph->base.context);

        if(has_cycle != 0)
        {
              VX_PRINT(VX_ZONE_ERROR,"Supernode has cycles (output of supernode has path to input of same supernode))\n");
              status = (vx_status)VX_FAILURE;
        }
    }

    return status;
}
