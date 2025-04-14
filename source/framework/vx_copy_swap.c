/*

 * Copyright (c) 2024 The Khronos Group Inc.
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


/*
Implementation file for the copy and swap utilities
*/

#include <vx_internal.h>

static vx_status ownCopyMoveRemoveNode(vx_graph graph, const vx_uint32 node_index, const vx_reference old_reference, const vx_reference new_reference);
static vx_bool ownCopyMoveReplaceInputCompatible(vx_reference old_reference, vx_reference new_reference);
static vx_bool ownCopyMoveReplaceOutputCompatible(vx_reference old_reference, vx_reference new_reference);
static void ownReassignGraphParameter(vx_graph graph, const vx_node node, const vx_uint32 index, const vx_reference ref, const vx_enum direction);
static vx_uint16 makeCopyMoveNodeList(vx_graph graph, vx_uint16 copy_move_indices[TIVX_GRAPH_MAX_NODES]);

static vx_status notifyTiovxMaxNodes(const char *in_or_out)
{
    VX_PRINT(VX_ZONE_ERROR, "number of %s nodes greater than maximum allowed\n", in_or_out);
    VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_NODE_MAX_%s_NODES in tiovx/include/TI/tivx_config.h\n", in_or_out);
    return (vx_status)VX_ERROR_NO_RESOURCES;
}

static vx_status ownCopyMoveRemoveNode(vx_graph graph, const vx_uint32 node_index, const vx_reference old_reference, const vx_reference new_reference)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 i;
    vx_node old_node = graph->nodes[node_index];
    tivx_obj_desc_node_t *old_obj = old_node->obj_desc[0];
    uint16_t old_node_id = old_obj->base.obj_desc_id;
    for (i = 0; (i < graph->num_nodes) && ((vx_status)VX_SUCCESS == status); ++i)
    {
        if (i != node_index)
        {
            vx_uint32 j;
            vx_node node = graph->nodes[i];
            for (j = 0; j < node->kernel->signature.num_parameters; ++j)
            {
                if (node->parameters[j] == old_reference)
                {
                    status = ownReleaseReferenceInt(&node->parameters[j], (vx_enum)node->parameters[j]->type, (vx_enum)VX_INTERNAL, NULL);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_COPY_SWAP_UTJT001
<justification end> */
                    if ((vx_status)VX_SUCCESS != status)
                    {
                        break;
                    }
/* LDRA_JUSTIFY_END */
                    /* Setting it as void since return value 'count' is not used further */
                    (void)ownIncrementReference(new_reference, (vx_enum)VX_INTERNAL);
                    /* Assign parameter descriptor id in the node */
                    node->obj_desc[0]->data_id[j] = ownReferenceGetObjDescId(new_reference);
                    node->parameters[j] = new_reference;
                }
            }
        }
    }
    /* Adjust in and out nodes, preserving node execution order:
        for all the out nodes of the old node we add the in nodes of the old node,
        and remove the old node from the in nodes.
    */
    for (i = 0; (i < old_obj->num_out_nodes) && ((vx_status)VX_SUCCESS == status); ++i)
    {
        vx_uint16 out_node_id = old_obj->out_node_id[i];
        vx_node out_node = (vx_node)ownReferenceGetHandleFromObjDescId(out_node_id);
        tivx_obj_desc_node_t *out_objd = out_node->obj_desc[0];
        vx_uint32 j = 0;
        /* remove old node from in nodes of the out node */
        while (old_node_id != out_objd->in_node_id[j])
        {
            j++;
        }
        out_objd->in_node_id[j] = out_objd->in_node_id[out_objd->num_in_nodes - 1U];
        out_objd->num_in_nodes = out_objd->num_in_nodes - 1U;
        /* add all in nodes of old_node to out_node */
        for (j = 0; j < old_obj->num_in_nodes; ++j)
        {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_COPY_SWAP_UBR001
<justification end>*/
            if (TIVX_NODE_MAX_IN_NODES > out_objd->num_in_nodes)
/* LDRA_JUSTIFY_END */
            {
                out_objd->in_node_id[out_objd->num_in_nodes] = old_obj->in_node_id[j];
                out_objd->num_in_nodes = out_objd->num_in_nodes + 1U;
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_COPY_SWAP_UTJT002
<justification end>*/
            else
            {
                status = notifyTiovxMaxNodes("IN");
                break;
            }
/* LDRA_JUSTIFY_END */
        }

    }
    for (i = 0; (i < old_obj->num_in_nodes) && ((vx_status)VX_SUCCESS == status); ++i)
    {
        vx_uint16 in_node_id = old_obj->in_node_id[i];
        vx_node in_node = (vx_node)ownReferenceGetHandleFromObjDescId(in_node_id);
        tivx_obj_desc_node_t *in_objd = in_node->obj_desc[0];
        vx_uint32 j = 0;
        /* remove old node from out nodes of the in node */
        while (old_node_id != in_objd->out_node_id[j])
        {
            j++;
        }
        in_objd->out_node_id[j] = in_objd->out_node_id[in_objd->num_out_nodes - 1U];
        in_objd->num_out_nodes = in_objd->num_out_nodes - 1U;
        /* add all out nodes of old_node to in_node */
        for (j = 0; j < old_obj->num_out_nodes; ++j)
        {
            if (TIVX_NODE_MAX_OUT_NODES > in_objd->num_out_nodes)
            {
                in_objd->out_node_id[in_objd->num_out_nodes] = old_obj->out_node_id[j];
                in_objd->num_out_nodes = in_objd->num_out_nodes + 1U;
            }
            else
            {
                status = notifyTiovxMaxNodes("OUT");
                break;
            }
        }
    }
    /* Remove the node from the graph */
    if ((vx_status)VX_SUCCESS == status)
    {
        vx_node node = graph->nodes[node_index];
        graph->nodes[node_index] = graph->nodes[graph->num_nodes-1U];
        graph->nodes[graph->num_nodes-1U] = NULL;
        graph->num_nodes--;
        node->graph = NULL; /* signal that it has been optimised away */
        status = ownReleaseReferenceInt((vx_reference *)&node, (vx_enum)VX_TYPE_NODE, (vx_enum)VX_INTERNAL, NULL);
    }
    return status;
}

/* Reassign any graph parameters that are using the given node and parameter index to the given reference and direction.
 * This is called after a node is removed. The index gives the parameter on the current node that is being removed.
   The reference gives the new reference that will replace it, the direction is the direction of the parameter.
 */
static void ownReassignGraphParameter(vx_graph graph, const vx_node node, const vx_uint32 index, const vx_reference ref, const vx_enum direction)
{
    vx_uint32 pi;
    for (pi = 0; pi < graph->num_params; ++pi)
    {
        if ((graph->parameters[pi].node == node) && /* TIOVX-2067- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_COPY_SWAP_UBR002 */
            (graph->parameters[pi].index == index)) /* TIOVX-2067- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_COPY_SWAP_UBR003 */
        {
            vx_uint32 ni;
            vx_uint32 pi2;
            for (ni = 0; ni < graph->num_nodes; ni++)
            {
                vx_node this_node = graph->nodes[ni];
                for (pi2 = 0;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_COPY_SWAP_UBR004
<justification end> */
                pi2 < this_node->kernel->signature.num_parameters;
/* LDRA_JUSTIFY_END */
                ++pi2)
                {
                    if ((ref == this_node->parameters[pi2]) &&
                        (direction == this_node->kernel->signature.directions[pi2]))
                    {
                        graph->parameters[pi].node = this_node;
                        graph->parameters[pi].index = pi2;
                        ni = graph->num_nodes;
                        break;
                    }
                }
            }
        }
    }
}

/* Return vx_true_e if the new reference can be used to replace the old reference at the output*/
static vx_bool ownCopyMoveReplaceOutputCompatible(vx_reference new_reference, vx_reference old_reference)
{
    /*
    Rules:
        Old (output) reference must be virtual (guaranteed by the caller), and
            if an image then a plain image type (not a sub-image; a virtual uniform image is not possible)
            if a tensor then not a view of a tensor or image
        If the old output reference cannot be in a delay as you can't get virtual objects from a delay
        If the old reference is not is a delay, but the new one is, it should be OK
        If the old output reference is in an object array or pyramid, and the new one isn't, we don't replace it
        If the new reference is in an object array or pyramid and the old one isn't that would be OK
        If they are both in the same sort of container, and are both in the first position, that would be OK
        If they are in different sorts of container we don't replace it
        If they are in not both at position zero in the same sort of container we don't replace it
        Finally: the objects must be copyable, i.e. validated by the copy validation, in this case not
        all attributes need match, because the output is virtual.

        Remember - this is more strict than the validator for the copy node, because we are not testing to see if the
        objects can be copied, we are testing to see if the copy node can be removed.
    */
    vx_bool ret = ((vx_status)VX_SUCCESS == new_reference->kernel_callback((vx_enum)VX_KERNEL_COPY, (vx_bool)vx_true_e, new_reference, old_reference)) ? (vx_bool)vx_true_e : (vx_bool)vx_false_e;
    if ((vx_bool)vx_true_e == ret)
    {
        if ((vx_enum)VX_TYPE_OBJECT_ARRAY == old_reference->scope->type)
        {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_COPY_SWAP_UBR005
<justification end> */
            if ((new_reference->scope->type != old_reference->scope->type) ||
                (old_reference != ((vx_object_array)old_reference->scope)->ref[0]) ||
                (new_reference != ((vx_object_array)new_reference->scope)->ref[0]))
            {
                ret = (vx_bool)vx_false_e;
            }
/* LDRA_JUSTIFY_END */
        }
        else if ((vx_enum)VX_TYPE_PYRAMID == old_reference->scope->type)
        {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_COPY_SWAP_UBR006
<justification end> */
            if ((new_reference->scope->type != old_reference->scope->type) ||
                ((vx_image)old_reference != ((vx_pyramid)old_reference->scope)->img[0]) ||
                ((vx_image)new_reference != ((vx_pyramid)new_reference->scope)->img[0]))
            {
                ret = (vx_bool)vx_false_e;
            }
/* LDRA_JUSTIFY_END */
        }
        else
        {
            switch (old_reference->type)
            {
                case (vx_enum)VX_TYPE_IMAGE:
                {
                    /* can't replace sub images or uniform images*/
                    tivx_obj_desc_image_t *obj_desc = (tivx_obj_desc_image_t *)old_reference->obj_desc;
                    if (((vx_uint32)TIVX_IMAGE_FROM_ROI == obj_desc->create_type) ||
                        ((vx_uint32)TIVX_IMAGE_FROM_CHANNEL == obj_desc->create_type))
                    {
                        ret = (vx_bool)vx_false_e;
                    }
                    break;
                }
                case (vx_enum)VX_TYPE_TENSOR:
                {
                    /* can't replace sub-tensors */
                    tivx_obj_desc_tensor_t *obj_desc = ( tivx_obj_desc_tensor_t *)old_reference->obj_desc;
                    if ((vx_uint32)TIVX_TENSOR_FROM_ROI == obj_desc->create_type)
                    {
                        ret = vx_false_e;
                    }
                    break;
                }       
                default:
                    break;
            }
        }
    }

    return ret;
}

/* Return vx_true_e if the new reference can be used to replace the old reference at the input */
static vx_bool ownCopyMoveReplaceInputCompatible(vx_reference old_reference, vx_reference new_reference)
{
    /*
    This is to check to see if backwards propagation is possible, where the output is the new reference,
    and the input is virtual. Basically we reverse all the tests, as though we are copying the output
    to the input. Because the validator is called, the metadata will be propagated backwards (not the
    usual way...)
    */
    return ownCopyMoveReplaceOutputCompatible(new_reference, old_reference);
}

static vx_uint16 makeCopyMoveNodeList(vx_graph graph, vx_uint16 copy_move_indices[TIVX_GRAPH_MAX_NODES])
{
    vx_uint16 num_copy_move_nodes = 0;
    vx_uint16 i;
    for (i = 0; i < graph->num_nodes; i++)
    {
        vx_enum kernel_enum = graph->nodes[i]->kernel->enumeration;
        if (((vx_enum)VX_KERNEL_COPY == kernel_enum) ||
            ((vx_enum)VX_KERNEL_MOVE == kernel_enum))
        {
            copy_move_indices[num_copy_move_nodes] = i;
            ++num_copy_move_nodes;
        }
    }
    return num_copy_move_nodes;
}

vx_status ownGraphProcessCopyMoveNodes(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint16 i;
    vx_uint16 copy_move_indices[TIVX_GRAPH_MAX_NODES];
    vx_uint16 num_copy_move_nodes = 0;
    vx_bool node_removed = (vx_bool)vx_false_e;

    /* Identify all copy or move nodes. This makes the iterative approach faster */
    num_copy_move_nodes = makeCopyMoveNodeList(graph, copy_move_indices);
    /* First, look for Move nodes with the first parameter connected to another node's input,
        this is illegal and will cause the graph to fail verification */
    for (i = 0; (i < num_copy_move_nodes) && ((vx_status)VX_SUCCESS == status); i++)
    {
        vx_node node = graph->nodes[copy_move_indices[i]];
        if ((vx_enum)VX_KERNEL_MOVE == node->kernel->enumeration)
        {
            vx_reference first = node->parameters[0];
            vx_uint32 j;
            /* Check for Move nodes with bidirectional attached to another input */
            for (j = 0; (j < graph->num_nodes) && ((vx_status)VX_SUCCESS == status); ++j)
            {
                if (copy_move_indices[i] != j)
                {
                    vx_node othernode = graph->nodes[j];
                    vx_uint32 k;
                    for (k = 0; k < othernode->kernel->signature.num_parameters; ++k)
                    {
                        if ((othernode->parameters[k] == first) &&
                            ((vx_enum)VX_INPUT == othernode->kernel->signature.directions[k]))
                        {
                            /* This is not allowed */
                            VX_PRINT(VX_ZONE_ERROR, "Move nodes must not have their first parameter connected to another node's input\n");
                            status = (vx_status)VX_FAILURE;
                            break;
                        }
                    }
                }
            }
        }
    }
    /* Copy and move node removal - Output nodes.
     * When ever we remove an output node, another
     * node may become an output node, so we iterate
     * until no more are found.
     */
    do
    {
        node_removed = (vx_bool)vx_false_e;
        for (i = 0; (i < num_copy_move_nodes) && ((vx_status)VX_SUCCESS == status); ++i)
        {
            vx_node node = graph->nodes[copy_move_indices[i]];
            vx_reference first = node->parameters[0];
            vx_reference second = node->parameters[1];
            vx_bool removable = first->is_virtual;
            if ((vx_bool)vx_false_e == second->is_virtual)
            {
                /* Copy nodes may only be removed if the input is not connected to another copy node input */
                if ((vx_enum)VX_KERNEL_COPY == node->kernel->enumeration)
                {
                    vx_uint32 j;
                    for (j = 0; j < num_copy_move_nodes; ++j)
                    {
                        if (i != j)
                        {
                            if (first == graph->nodes[copy_move_indices[j]]->parameters[0])
                            {
                                removable = (vx_bool)vx_false_e;
                                break;
                            }
                        }
                    }
                }
                if (((vx_bool)vx_true_e == removable) &&
                    ((vx_bool)vx_true_e == ownCopyMoveReplaceInputCompatible(first, second)))
                {
                    /* we can remove this node, propagating the output reference */
                    status = ownCopyMoveRemoveNode(graph, copy_move_indices[i], first, second);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_COPY_SWAP_UBR007
<justification end> */
                    if ((vx_status)VX_SUCCESS == status)
                    {
                        node_removed = (vx_bool)vx_true_e;
                        /* and re-make node list list */
                        num_copy_move_nodes = makeCopyMoveNodeList(graph, copy_move_indices);
                        /* Now process any graph parameters connected to the output */
                        ownReassignGraphParameter(graph, node, 1, second, (vx_enum)VX_OUTPUT);
                        /* Re-start the loop */
                        break;
                    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement <metric end>
<justification start> TIOVX_CODE_COVERAGE_COPY_SWAP_UTJT003
<justification end> */
                }
/* LDRA_JUSTIFY_END */
            }
        }
    } while ((vx_bool)vx_true_e == node_removed);
    /* Copy and move node removal - Input nodes.
     * When ever we remove an Input node, another
     * node may become an Input node, so we iterate
     * until no more are found.
     */
    do
    {
        node_removed = (vx_bool)vx_false_e;
        /* Look for nodes with input non-virtual and output virtual,
        and output not connected to a bidirectional of another node */
        for (i = 0; (i < num_copy_move_nodes) && ((vx_status)VX_SUCCESS == status); i++)
        {
            vx_node node = graph->nodes[copy_move_indices[i]];
            vx_reference first = node->parameters[0];
            vx_reference second = node->parameters[1];
            vx_bool removable = second->is_virtual;
            if ((vx_bool)vx_false_e == first->is_virtual)
            {
                /* check for a bidirectional connection in case of a copy node */
                if ((vx_enum)VX_KERNEL_COPY == node->kernel->enumeration)
                {
                    vx_uint32 j;
                    for (j = 0; (j < graph->num_nodes) && (removable == (vx_bool)(vx_true_e)); ++j)
                    {
                        if (copy_move_indices[i] != j)
                        {
                            vx_node othernode = graph->nodes[j];
                            vx_uint32 k;
                            for (k = 0; k < othernode->kernel->signature.num_parameters; ++k)
                            {
                                if ((othernode->parameters[k] == second) &&
                                    ((vx_enum)VX_BIDIRECTIONAL == othernode->kernel->signature.directions[k]))
                                {
                                    /* can't remove it */
                                    removable = (vx_bool)vx_false_e;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (((vx_bool)vx_true_e == removable) &&
                    ((vx_bool)vx_true_e == ownCopyMoveReplaceOutputCompatible(first, second)))
                {
                    /* we can remove this node, propagating the input reference */
                    status = ownCopyMoveRemoveNode(graph, copy_move_indices[i], second, first);
                    if ((vx_status)VX_SUCCESS == status)
                    {
                        node_removed = (vx_bool)vx_true_e;
                        /* and re-make node list list */
                        num_copy_move_nodes = makeCopyMoveNodeList(graph, copy_move_indices);
                        /* Process any graph parameter connected to the input */
                        ownReassignGraphParameter(graph, node, 0, first, (vx_enum)VX_INPUT);
                        /* re-start the loop */
                        break;
                    }
                }
            }
        }
    } while ((vx_bool)vx_true_e == node_removed);

    /* Now, look for Copy & Move nodes with both input and output virtual.
       We have to check for a bidirectional connection on the output of
       a copy node and make sure that the node has no siblings.
    */
    do
    {
        node_removed = (vx_bool)vx_false_e;
        for (i = 0; (i < num_copy_move_nodes) && ((vx_status)VX_SUCCESS == status); ++i)
        {
            vx_node node = graph->nodes[copy_move_indices[i]];
            vx_reference first = node->parameters[0];
            vx_reference second = node->parameters[1];
            vx_bool removable = first->is_virtual;
            if ((vx_bool)vx_true_e == second->is_virtual)
            {
                /* Copy nodes may only be removed if the input is not connected to another copy node input.
                We also check for bidirectional connections of the output here */
                if ((vx_enum)VX_KERNEL_COPY == node->kernel->enumeration)
                {
                    vx_uint32 j;
                    for (j = 0; (j < num_copy_move_nodes) && (removable == (vx_bool)(vx_true_e)); ++j)
                    {
                        if (i != j)
                        {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_COPY_SWAP_UBR008
<justification end> */
                            if (first == graph->nodes[copy_move_indices[j]]->parameters[0])
                            {
                                removable = (vx_bool)vx_false_e;
                            }
/* LDRA_JUSTIFY_END */
                        }
                    }
                    tivx_obj_desc_node_t * obj_desc = (tivx_obj_desc_node_t *)(node->obj_desc[0]);
                    for (j = 0; (j < obj_desc->num_in_nodes) && (removable == (vx_bool)(vx_true_e)); j++)
                    {
                        vx_node in_node = vxCastRefAsNode(ownReferenceGetHandleFromObjDescId(obj_desc->in_node_id[j]), NULL);
                        if (((tivx_obj_desc_node_t *)(in_node->obj_desc[0]))->num_out_nodes > 1U)
                        {
                            vx_uint32 k;
                            for (k = 0;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_COPY_SWAP_UBR009
<justification end> */
                            (k < graph->num_nodes) && (removable == (vx_bool)(vx_true_e));
/* LDRA_JUSTIFY_END */
                            ++k)
                            {
                                if (copy_move_indices[i] != k)
                                {
                                    vx_node othernode = graph->nodes[k];
                                    vx_uint32 l;
                                    for (l = 0; (l < othernode->kernel->signature.num_parameters) && (removable == (vx_bool)(vx_true_e)); ++l)
                                    {
                                        if ((othernode->parameters[l] == second) &&
                                            ((vx_enum)VX_BIDIRECTIONAL == othernode->kernel->signature.directions[l]))
                                        {
                                            removable = (vx_bool)vx_false_e;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                if ((vx_bool)vx_true_e == removable)
                {
                    /* We test for replacing the output node first. */
                    if ((vx_bool)vx_true_e == ownCopyMoveReplaceOutputCompatible(first, second))
                    {
                        /* we can remove this node, propagating the input reference */
                        status = ownCopyMoveRemoveNode(graph, copy_move_indices[i], second, first);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_COPY_SWAP_UBR010
<justification end> */
                        if ((vx_status)VX_SUCCESS == status)
                        {
                            node_removed = (vx_bool)vx_true_e;
                            /* and re-make node list list */
                            num_copy_move_nodes = makeCopyMoveNodeList(graph, copy_move_indices);
                            /* restart the loop */
                            break;
                        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement <metric end>
<justification start> TIOVX_CODE_COVERAGE_COPY_SWAP_UTJT004
<justification end> */
                    }
/* LDRA_JUSTIFY_END */
                }
            }
        }
    } while ((vx_bool)vx_true_e == node_removed);
    return status;
}
