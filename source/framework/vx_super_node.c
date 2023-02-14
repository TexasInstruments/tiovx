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

static vx_bool ownIsValidCreateParams(vx_graph graph, vx_node nodes[], uint32_t num_nodes);
static vx_bool ownIsValidSuperNode(tivx_super_node super_node);
static vx_status ownDestructSuperNode(vx_reference ref);
static void ownInitSuperNode(tivx_super_node super_node, vx_node nodes[], uint32_t num_nodes);

static vx_bool ownIsValidCreateParams(vx_graph graph, vx_node nodes[], uint32_t num_nodes)
{
    vx_bool is_valid = (vx_bool)vx_true_e;

    if (TIVX_SUPER_NODE_MAX_NODES < num_nodes)
    {
        is_valid = (vx_bool)vx_false_e;
        VX_PRINT(VX_ZONE_ERROR, "num_nodes is higher than TIVX_SUPER_NODE_MAX_NODES\n");
        VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_SUPER_NODE_MAX_NODES in tiovx/include/TI/tivx_config.h\n");
    }

    if( (vx_bool)vx_true_e == is_valid )
    {
        uint32_t i;

        for(i=0; (i < num_nodes) && (is_valid == (vx_bool)vx_true_e); i++)
        {
            if ((ownIsValidSpecificReference((vx_reference)nodes[i], (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_false_e))
            {
                is_valid = (vx_bool)vx_false_e;
                VX_PRINT(VX_ZONE_ERROR, "node[%d] is not a valid node\n", i);
            }
            else if (nodes[i]->graph != graph)
            {
                is_valid = (vx_bool)vx_false_e;
                VX_PRINT(VX_ZONE_ERROR, "node[%d] is not in graph\n", i);
            }
            else if (nodes[i]->super_node != NULL)
            {
                is_valid = (vx_bool)vx_false_e;
                VX_PRINT(VX_ZONE_ERROR, "node[%d] is already in a super node\n", i);
            }
            else
            {
                /* do nothing */
            }
        }
    }
    return is_valid;
}

static vx_bool ownIsValidSuperNode(tivx_super_node super_node)
{
    vx_bool is_valid;

    if ((ownIsValidSpecificReference((vx_reference)super_node, TIVX_TYPE_SUPER_NODE) == (vx_bool)vx_true_e) &&
        (super_node->base.obj_desc != NULL)
       )
    {
        is_valid = (vx_bool)vx_true_e;
    }
    else
    {
        is_valid = (vx_bool)vx_false_e;
    }

    return is_valid;
}

static vx_status ownDestructSuperNode(vx_reference ref)
{
    tivx_obj_desc_super_node_t *obj_desc = NULL;
    tivx_super_node super_node = (tivx_super_node)ref;

    if(ref->type == TIVX_TYPE_SUPER_NODE)
    {
        obj_desc = (tivx_obj_desc_super_node_t *)ref->obj_desc;

        vxRemoveNode(&super_node->node);

        if(obj_desc!=NULL)
        {
            ownObjDescFree((tivx_obj_desc_t**)&obj_desc);
        }
    }
    return (vx_status)VX_SUCCESS;
}

static void ownInitSuperNode(tivx_super_node super_node, vx_node nodes[], uint32_t num_nodes)
{
    uint32_t i;
    tivx_obj_desc_super_node_t *obj_desc = NULL;

    obj_desc = (tivx_obj_desc_super_node_t *)super_node->base.obj_desc;

    super_node->node = NULL;

    for(i=0; i<num_nodes; i++)
    {
        super_node->nodes[i] = nodes[i];
        nodes[i]->super_node = super_node;
    }

    obj_desc->num_nodes = (uint16_t)num_nodes;

    ownLogSetResourceUsedValue("TIVX_SUPER_NODE_MAX_NODES", (uint16_t)num_nodes);
}

VX_API_ENTRY tivx_super_node VX_API_CALL tivxCreateSuperNode(vx_graph graph,
    vx_node nodes[], uint32_t num_nodes)
{
    tivx_super_node super_node = NULL;
    tivx_obj_desc_super_node_t *obj_desc = NULL;
    vx_kernel kernel;

    vx_context context = vxGetContext((vx_reference)graph);

    if (ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        if (ownIsValidCreateParams(graph, nodes, num_nodes) == (vx_bool)vx_true_e)
        {
            super_node = (tivx_super_node)ownCreateReference(context, TIVX_TYPE_SUPER_NODE, (vx_enum)VX_EXTERNAL, &graph->base);

            if ( (vxGetStatus((vx_reference)super_node) == (vx_status)VX_SUCCESS) && (super_node->base.type == TIVX_TYPE_SUPER_NODE) )
            {
                /* assign refernce type specific callback's */
                super_node->base.destructor_callback = &ownDestructSuperNode;
                super_node->base.mem_alloc_callback = NULL;
                super_node->base.release_callback = (tivx_reference_release_callback_f)&tivxReleaseSuperNode;

                obj_desc = (tivx_obj_desc_super_node_t*)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_SUPER_NODE, (vx_reference)super_node);

                if(obj_desc == NULL)
                {
                    tivxReleaseSuperNode(&super_node);

                    vxAddLogEntry(&graph->base, (vx_status)VX_ERROR_NO_RESOURCES, "Could not allocate super node object descriptor\n");
                    super_node = (tivx_super_node)ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate super node object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
                }
                else
                {
                    super_node->base.obj_desc = (tivx_obj_desc_t *)obj_desc;
                    ownInitSuperNode(super_node, nodes, num_nodes);

                    kernel = vxGetKernelByEnum(context, TIVX_KERNEL_SUPERNODE);
                    if (NULL != kernel)
                    {
                        super_node->node = vxCreateGenericNode(graph, kernel);
                        vxReleaseKernel(&kernel);

                        if (vxGetStatus((vx_reference)super_node->node) == (vx_status)VX_SUCCESS)
                        {
                            vx_status status;
                            super_node->node->super_node = super_node;
                            super_node->node->is_super_node = (vx_bool)vx_true_e;
                            tivxFlagBitSet(&super_node->node->obj_desc[0]->flags, TIVX_NODE_FLAG_IS_SUPERNODE);
                            super_node->node->obj_desc[0]->base.scope_obj_desc_id = obj_desc->base.obj_desc_id;
                            status = ownGraphAddSuperNode(graph, super_node);

                            if ((vx_status)VX_SUCCESS != status)
                            {
                                vxAddLogEntry((vx_reference)graph, (vx_status)VX_ERROR_INVALID_PARAMETERS, "Failed to add supernode to graph\n");
                                vxReleaseNode(&super_node->node);
                                tivxReleaseSuperNode(&super_node);
                            }
                        }
                        else
                        {
                            vxAddLogEntry((vx_reference)graph, (vx_status)VX_ERROR_INVALID_PARAMETERS, "Failed to create node with kernel enum TIVX_KERNEL_SUPERNODE\n");
                            VX_PRINT(VX_ZONE_ERROR, "Failed to create node with kernel enum TIVX_KERNEL_SUPERNODE\n");
                        }
                    }
                    else
                    {
                        vxAddLogEntry((vx_reference)graph, (vx_status)VX_ERROR_INVALID_PARAMETERS, "Failed to retrieve kernel enum TIVX_KERNEL_SUPERNODE\n");
                        super_node = (tivx_super_node)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
                    }
                }
            }
        }
        else
        {
            vxAddLogEntry((vx_reference)graph, (vx_status)VX_ERROR_INVALID_PARAMETERS, "Requested create parameters was invalid!\n");
            super_node = (tivx_super_node)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
        }
    }
    else
    {
        vxAddLogEntry((vx_reference)graph, (vx_status)VX_ERROR_INVALID_REFERENCE, "Graph %p is invalid!\n", graph);
        VX_PRINT(VX_ZONE_ERROR, "graph is invalid\n");
    }

    return super_node;
}

VX_API_ENTRY vx_status VX_API_CALL tivxReleaseSuperNode(tivx_super_node *super_node)
{
    return ownReleaseReferenceInt((vx_reference *)super_node, TIVX_TYPE_SUPER_NODE, (vx_enum)VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_status VX_API_CALL tivxQuerySuperNode(tivx_super_node super_node, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_obj_desc_super_node_t *obj_desc = NULL;

    obj_desc = (tivx_obj_desc_super_node_t *)super_node->base.obj_desc;

    if (ownIsValidSuperNode(super_node) == (vx_bool)vx_true_e)
    {
        switch (attribute)
        {
            case (vx_enum)TIVX_SUPER_NODE_TARGET_STRING:
                if ((ptr != NULL) && (size >= TIVX_TARGET_MAX_NAME))
                {
                    ownPlatformGetTargetName((int32_t)super_node->node->obj_desc[0]->target_id, ptr);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query super node target failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)TIVX_SUPER_NODE_PERFORMANCE:
                if (VX_CHECK_PARAM(ptr, size, vx_perf_t, 0x3U))
                {
                    memcpy(ptr, &super_node->node->perf, size);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query super node performance failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)TIVX_SUPER_NODE_STATUS:
                if (VX_CHECK_PARAM(ptr, size, vx_status, 0x3U))
                {
                    /* returns  status for pipeline index 0,
                     * may not reflect correct status
                     * in pipelined mode
                     */
                    *(vx_status *)ptr = (vx_status)super_node->node->obj_desc[0]->exe_status;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query super node status failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)TIVX_SUPER_NODE_NUM_NODES:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->num_nodes;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query super node num nodes failed\n");
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
        VX_PRINT(VX_ZONE_ERROR, "invalid super node reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxSetSuperNodeTarget(tivx_super_node super_node,
                                                          vx_enum target_enum,
                                                          const char* target_string)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    uint32_t i, num_nodes;

    if (ownIsValidSpecificReference((vx_reference)super_node, TIVX_TYPE_SUPER_NODE) == (vx_bool)vx_true_e)
    {
        tivx_obj_desc_super_node_t *obj_desc = NULL;

        obj_desc = (tivx_obj_desc_super_node_t *)super_node->base.obj_desc;

        num_nodes = obj_desc->num_nodes;

        status = vxSetNodeTarget(super_node->node, target_enum, target_string);

        if((vx_status)VX_SUCCESS == status)
        {
            for(i=0; i < num_nodes; i++)
            {
                status = vxSetNodeTarget(super_node->nodes[i], target_enum, target_string);

                if((vx_status)VX_SUCCESS != status)
                {
                    break;
                }
            }
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxSetSuperNodeTileSize(tivx_super_node super_node, vx_uint32 block_width, vx_uint32 block_height)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    tivx_obj_desc_node_t *node_obj_desc;

    if (ownIsValidSpecificReference((vx_reference)super_node, TIVX_TYPE_SUPER_NODE) == (vx_bool)vx_true_e)
    {
        /* In TI implementation, tivxSetSuperNodeTileSize() cannot be called after a graph is verified
         *
         * This is because it will set the tile size at the graph verify stage
         */
        if (super_node->node->graph->verified == (vx_bool)vx_true_e)
        {
            VX_PRINT(VX_ZONE_ERROR, "Graph has been verified\n");
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            node_obj_desc = (tivx_obj_desc_node_t *)super_node->node->obj_desc[0];
            node_obj_desc->block_width = block_width;
            node_obj_desc->block_height = block_height;
            status = (vx_status)VX_SUCCESS;
        }
    }
    return status;
}
