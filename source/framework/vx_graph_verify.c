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
 * Since graph verify is involved logic, putting it in a different file.
 */

#include <vx_internal.h>

static vx_status ownGraphValidRectCallback(
    vx_graph graph, vx_node node, vx_meta_format meta[]);
static vx_status ownGraphInitVirtualNode(
    vx_graph graph, vx_node node, vx_meta_format meta[]);
static vx_status ownGraphNodeKernelValidate(
    vx_graph graph, vx_meta_format meta[]);
static vx_status ownGraphNodeKernelInit(vx_graph graph);
static vx_status ownGraphNodeKernelDeinit(vx_graph graph);
static vx_bool ownGraphIsRefMatch(vx_graph graph, vx_reference ref1, vx_reference ref2);
static vx_status ownGraphCalcInAndOutNodes(vx_graph graph);
static vx_status ownGraphCalcHeadAndLeafNodes(vx_graph graph);
static vx_status ownGraphAllocateDataObjects(vx_graph graph);
static vx_status ownGraphCreateNodeCallbackCommands(vx_graph graph);
static vx_status ownGraphAddDataRefQ(vx_graph graph, vx_node node, uint32_t index);
static vx_status ownGraphDetectSourceSink(vx_graph graph);
static vx_status ownGraphAddDataReference(vx_graph graph, vx_reference ref, uint32_t prm_dir, uint32_t check);
static vx_status ownGraphAllocateDataObject(vx_graph graph, vx_node node_cur, uint32_t prm_cur_idx, vx_reference ref);
static vx_status ownGraphCheckAndCreateDelayDataReferenceQueues(vx_graph graph, vx_node node,
                                                                uint32_t index, tivx_data_ref_queue data_ref_q);
static vx_status ownGraphCreateAndLinkDataReferenceQueues(vx_graph graph);
static vx_status ownGraphCreateGraphParameterDataReferenceQueues(vx_graph graph);
static vx_status ownGraphCreateIntermediateDataReferenceQueues(vx_graph graph);
static vx_status ownGraphFindAndAddDataReferences(vx_graph graph);
static uint32_t ownGraphGetNumInNodes(vx_graph graph, vx_node node, uint32_t node_prm_idx);
static void ownGraphLinkArrayElements(vx_graph graph);
static void ownGraphLinkDataReferenceQueues(vx_graph graph);
static void ownGraphLinkDataReferenceQueuesToNodeIndex(vx_graph graph, tivx_data_ref_queue data_ref_q,
                                                       vx_node node, uint32_t index);
static vx_status ownGraphNodePipeline(vx_graph graph);
static vx_status ownGraphPrimeDataReferenceQueues(vx_graph graph);
static vx_status ownGraphUpdateDataRefAfterKernetInit(vx_graph graph, vx_reference exemplar, vx_reference ref);
static vx_status ownGraphUpdateDataReferenceQueueRefsAfterKernelInit(vx_graph graph);
static vx_status ownGraphUpdateImageRefAfterKernetInit(vx_graph graph, vx_image exemplar, vx_image ref);
static vx_status ownGraphUpdateObjArrRefAfterKernetInit(vx_graph graph, vx_object_array exemplar, vx_object_array ref);
static vx_status ownGraphUpdatePyramidRefAfterKernetInit(vx_graph graph, vx_pyramid exemplar, vx_pyramid ref);

/* Add's data reference to a list, increments number of times it is refered as input node */
static vx_status ownGraphAddDataReference(vx_graph graph, vx_reference ref, uint32_t prm_dir, uint32_t check)
{
    uint32_t i;
    vx_status status = (vx_status)VX_FAILURE;

    for(i=0; i<graph->num_data_ref; i++)
    {
        if((check != 0U) && (ownGraphCheckIsRefMatch(graph, graph->data_ref[i], ref) != (vx_bool)vx_false_e))
        {
            /* increment num_in_node count for ref */
            if(prm_dir==(uint32_t)VX_INPUT)
            {
                graph->data_ref_num_in_nodes[i]++;
            }
            status = (vx_status)VX_SUCCESS;
            break;
        }
    }
    if( (i == graph->num_data_ref)
        && (graph->num_data_ref < TIVX_GRAPH_MAX_DATA_REF))
    {
        /* 'ref' not present in 'data_ref' list so add it */
        graph->data_ref[i] = ref;
        graph->data_ref_num_in_nodes[i] = 0;
        graph->data_ref_num_out_nodes[i] = 0;
        if(prm_dir==(uint32_t)VX_INPUT)
        {
            graph->data_ref_num_in_nodes[i]++;
        }
        else
        {
            graph->data_ref_num_out_nodes[i]++;
        }
        graph->num_data_ref++;
        ownLogSetResourceUsedValue("TIVX_GRAPH_MAX_DATA_REF", (uint16_t)graph->num_data_ref);
        status = (vx_status)VX_SUCCESS;
    }
    else if (graph->num_data_ref >= TIVX_GRAPH_MAX_DATA_REF)
    {
        VX_PRINT(VX_ZONE_WARNING, "May need to increase the value of TIVX_GRAPH_MAX_DATA_REF in tiovx/include/TI/tivx_config.h\n");
    }
    else
    {
        /* do nothing */
    }

    if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
            ||
        (ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
       )
    {
        /* Check set to 0 because a data_ref_num_in_nodes is not supposed to be set for parent object */
        ownGraphAddDataReference(graph, ref->scope, prm_dir, 0);
    }

    return status;
}

static vx_status ownGraphDetectSourceSink(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t cur_node_idx, out_node_idx, num_out_nodes;
    vx_node cur_node, next_node;

    for(cur_node_idx=0; cur_node_idx<graph->num_nodes; cur_node_idx++)
    {
        cur_node = graph->nodes[cur_node_idx];

        if (NULL != cur_node)
        {
            if (cur_node->kernel->num_pipeup_bufs > 1U)
            {
                num_out_nodes = (uint16_t)ownNodeGetNumOutNodes(cur_node);

                for(out_node_idx=0; out_node_idx < num_out_nodes; out_node_idx++)
                {
                    next_node = ownNodeGetNextNode(cur_node, out_node_idx);

                    if (NULL != next_node)
                    {
                        if (next_node->kernel->num_sink_bufs > 1U)
                        {
                            if (next_node->kernel->num_sink_bufs > graph->nodes[cur_node_idx]->kernel->connected_sink_bufs)
                            {
                                graph->nodes[cur_node_idx]->kernel->connected_sink_bufs = next_node->kernel->num_sink_bufs;
                            }
                        }
                    }
                }
            }
        }
    }

    return status;
}

static uint32_t ownGraphGetNumInNodes(vx_graph graph, vx_node node, uint32_t node_prm_idx)
{
    uint32_t num_in_nodes = 0, i;
    vx_reference ref;

    ref = ownNodeGetParameterRef(node, node_prm_idx);
    if(ref != NULL)
    {
        for(i=0; i<graph->num_data_ref; i++)
        {
            if(ownGraphCheckIsRefMatch(graph, graph->data_ref[i], ref) != 0)
            {
                num_in_nodes = graph->data_ref_num_in_nodes[i];
                break;
            }
        }
    }
    return num_in_nodes;
}

#if 0
static uint32_t ownGraphGetNumOutNodes(vx_graph graph, vx_node node, uint32_t node_prm_idx)
{
    uint32_t num_out_nodes = 0, i;
    vx_reference ref;

    ref = ownNodeGetParameterRef(node, node_prm_idx);
    if(ref != NULL)
    {
        for(i=0; i<graph->num_data_ref; i++)
        {
            if(ref==graph->data_ref[i])
            {
                num_out_nodes = graph->data_ref_num_out_nodes[i];
                break;
            }
        }
    }
    return num_out_nodes;
}
#endif

/* find all data references within graph and collect them in a list */
static vx_status ownGraphFindAndAddDataReferences(vx_graph graph)
{
    vx_node node_cur;
    uint32_t node_cur_idx;
    uint32_t prm_cur_idx;
    uint32_t prm_dir;
    vx_reference ref;
    vx_status status = (vx_status)VX_SUCCESS;

    graph->num_data_ref = 0;

    for(node_cur_idx=0; node_cur_idx<graph->num_nodes; node_cur_idx++)
    {
        node_cur = graph->nodes[node_cur_idx];

        for(prm_cur_idx=0; prm_cur_idx<ownNodeGetNumParameters(node_cur); prm_cur_idx++)
        {
            ref = ownNodeGetParameterRef(node_cur, prm_cur_idx);
            prm_dir = (uint32_t)ownNodeGetParameterDir(node_cur, prm_cur_idx);

            if(ref!=NULL) /* ref could be NULL due to optional parameters */
            {
                status = ownGraphAddDataReference(graph, ref, prm_dir, 1);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Unable to add data reference to data reference list in graph\n");
                    break;
                }
            }
        }
    }

    return status;
}

static vx_status ownGraphValidRectCallback(
    vx_graph graph, vx_node node, vx_meta_format meta[])
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 num_in_image = 0, num_params, i;
    vx_reference ref;
    vx_meta_format mf;

    num_params = node->kernel->signature.num_parameters;
    for (i = 0; i < num_params; i ++)
    {
        ref = node->parameters[i];

        if( (node->kernel->signature.directions[i] == (vx_enum)VX_INPUT)
            &&
            (NULL != ref)
            &&
            (ref->type == (vx_enum)VX_TYPE_IMAGE)
            )
        {
            graph->in_valid_rect_ptr[num_in_image] = &graph->in_valid_rect[num_in_image];

            tivxCheckStatus(&status, vxGetValidRegionImage((vx_image)ref, &graph->in_valid_rect[num_in_image]));

            num_in_image++;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        for (i = 0; i < num_params; i ++)
        {
            ref = node->parameters[i];
            mf = meta[i];

            if( (node->kernel->signature.directions[i] == (vx_enum)VX_OUTPUT)
                &&
                (NULL != ref)
                &&
                (NULL != mf)
                &&
                (NULL != mf->valid_rect_callback)
                )
            {
                if(ref->type == (vx_enum)VX_TYPE_IMAGE)
                {
                    vx_status tmp_status;

                    graph->out_valid_rect_ptr[0] = &graph->out_valid_rect[0];

                    tmp_status = mf->valid_rect_callback(node, i,
                                    (const vx_rectangle_t* const*)graph->in_valid_rect_ptr,
                                    graph->out_valid_rect_ptr);
                    if(tmp_status==(vx_status)VX_SUCCESS)
                    {
                        tivxCheckStatus(&status, vxSetImageValidRectangle((vx_image)ref, &graph->out_valid_rect[0]));
                    }
                }
                else
                if(ref->type == (vx_enum)VX_TYPE_PYRAMID)
                {
                    vx_status tmp_status;
                    vx_size levels, k;

                    tivxCheckStatus(&status, vxQueryPyramid((vx_pyramid)ref, (vx_enum)VX_PYRAMID_LEVELS, &levels, sizeof(levels)));

                    if(status==(vx_status)VX_SUCCESS)
                    {
                        for(k=0; k<levels; k++)
                        {
                            graph->out_valid_rect_ptr[k] = &graph->out_valid_rect[k];
                        }

                        tmp_status = mf->valid_rect_callback(node, i,
                                        (const vx_rectangle_t* const*)graph->in_valid_rect_ptr,
                                        graph->out_valid_rect_ptr);
                        if(tmp_status==(vx_status)VX_SUCCESS)
                        {
                            for(k=0; k<levels; k++)
                            {
                                vx_image img = vxGetPyramidLevel((vx_pyramid)ref, (uint32_t)k);

                                tivxCheckStatus(&status, vxSetImageValidRectangle(img, &graph->out_valid_rect[k]));

                                vxReleaseImage(&img);
                            }
                        }
                    }
                }
                else
                {
                    /* not supported for other references */
                    status = (vx_status)VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR,"not supported for references other than image or pyramid\n");
                }
            }
        }
    }
    return status;
}

static vx_status ownGraphInitVirtualNode(
    vx_graph graph, vx_node node, vx_meta_format meta[])
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i, num_params;
    vx_reference ref;
    vx_meta_format mf;
    vx_pyramid pmd;
    vx_size levels;
    vx_enum type;

    num_params = node->kernel->signature.num_parameters;
    for (i = 0; i < num_params; i ++)
    {
        ref = node->parameters[i];
        mf = meta[i];

        if( (ref != NULL) && (mf != NULL) )
        {
            if ((node->kernel->signature.directions[i] == (vx_enum)VX_OUTPUT) &&
                ((vx_bool)vx_true_e == ref->is_virtual))
            {
                if ((ref->scope->type == (vx_enum)VX_TYPE_GRAPH) && (ref->scope != (vx_reference)graph))
                {
                    status = (vx_status)VX_ERROR_INVALID_SCOPE;
                    VX_PRINT(VX_ZONE_ERROR,"invalid scope\n");
                }

                if ((vx_status)VX_SUCCESS == status)
                {
                    switch (mf->type)
                    {
                        case (vx_enum)VX_TYPE_SCALAR:
                            status = vxQueryScalar((vx_scalar)ref,
                                (vx_enum)VX_SCALAR_TYPE, &type, sizeof(type));
                            /* For scalar, just check if type is correct or
                                not */
                            if ((vx_status)VX_SUCCESS == status)
                            {
                                if (type != mf->sc.type)
                                {
                                    status = (vx_status)VX_ERROR_INVALID_TYPE;
                                    VX_PRINT(VX_ZONE_ERROR,"invalid scalar type\n");
                                }
                            }
                            break;
                        case (vx_enum)VX_TYPE_IMAGE:
                            if (0U == mf->img.width)
                            {
                                status = (vx_status)VX_ERROR_INVALID_VALUE;
                                VX_PRINT(VX_ZONE_ERROR,"image width value equal to zero\n");
                            }
                            else if (0U == mf->img.height)
                            {
                                status = (vx_status)VX_ERROR_INVALID_VALUE;
                                VX_PRINT(VX_ZONE_ERROR,"image height value equal to zero\n");
                            }
                            else
                            {
                                status = ownInitVirtualImage((vx_image)ref,
                                    mf->img.width, mf->img.height,
                                    mf->img.format);
                            }
                            break;
                        case (vx_enum)VX_TYPE_ARRAY:
                            status = ownInitVirtualArray(
                                (vx_array)ref, mf->arr.item_type,
                                mf->arr.capacity);
                            break;
                        case (vx_enum)VX_TYPE_PYRAMID:
                            pmd = (vx_pyramid)ref;

                            status = vxQueryPyramid(
                                pmd, (vx_enum)VX_PYRAMID_LEVELS, &levels,
                                sizeof(levels));

                            if ((vx_status)VX_SUCCESS == status)
                            {
                                /* Levels must be same even in this case */
                                if (levels != mf->pmd.levels)
                                {
                                    status = (vx_status)VX_ERROR_INVALID_VALUE;
                                    VX_PRINT(VX_ZONE_ERROR,"pyramid levels incorrect\n");
                                }
                                else if (0U == mf->pmd.width)
                                {
                                    status = (vx_status)VX_ERROR_INVALID_VALUE;
                                    VX_PRINT(VX_ZONE_ERROR,"pyramid width equal to zero\n");
                                }
                                else if (0U == mf->pmd.height)
                                {
                                    status = (vx_status)VX_ERROR_INVALID_VALUE;
                                    VX_PRINT(VX_ZONE_ERROR,"pyramid height equal to zero\n");
                                }
                                else
                                {
                                    status = ownInitVirtualPyramid(pmd,
                                        mf->pmd.width, mf->pmd.height,
                                        mf->pmd.format);
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }

                if (status != (vx_status)VX_SUCCESS)
                {
                    break;
                }
            }
        }
    }

    return (status);
}

static vx_status ownGraphNodeKernelValidate(
    vx_graph graph, vx_meta_format meta[])
{
    vx_node node;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i, j;

    for(i=0; i<graph->num_nodes; i++)
    {
        node = graph->nodes[i];

        status = ownNodeKernelValidate(node, meta);
        if(status == (vx_status)VX_SUCCESS)
        {
            status = ownGraphInitVirtualNode(graph, node, meta);
            if(status == (vx_status)VX_SUCCESS)
            {
                status = ownGraphValidRectCallback(graph, node, meta);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"graph valid rectangle callback failed at index %d\n", i);
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,"graph init virtual node failed at index %d\n", i);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"node kernel validate failed for kernel %s at index %d\n", node->kernel->name, i);
        }

        if(status != (vx_status)VX_SUCCESS)
        {
            break;
        }

        for (j = 0; j < TIVX_KERNEL_MAX_PARAMS; j ++)
        {
            meta[j]->valid_rect_callback = NULL;
        }
    }

    return status;
}

static vx_status ownGraphNodeKernelInit(vx_graph graph)
{
    vx_node node;
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status status_name = (vx_status)VX_SUCCESS;
    uint32_t i;

    for(i=0; i<graph->num_nodes; i++)
    {
        node = graph->nodes[i];

        status = ownNodeKernelInitKernelName(node);

        if(status != (vx_status)VX_SUCCESS )
        {
            status_name = status;
            VX_PRINT(VX_ZONE_ERROR,"Node kernel name init for node at index %d failed\n", i);
            break;
        }
    }

    if((vx_status)VX_SUCCESS == status)
    {
        for(i=0; i<graph->num_nodes; i++)
        {
            node = graph->nodes[i];

            if(node && node->kernel)
            {
                status = ownNodeKernelInit(node);
                VX_PRINT(VX_ZONE_INFO, "kernel init for node %d, kernel %s ...\n", i, node->kernel->name);
            }

            if(status != (vx_status)VX_SUCCESS )
            {
                VX_PRINT(VX_ZONE_ERROR,"kernel init for node %d, kernel %s ... failed !!!\n", i, node->kernel->name);
                break;
            }

            if(node && node->kernel)
            {
                VX_PRINT(VX_ZONE_INFO, "kernel init for node %d, kernel %s ... done !!!\n", i, node->kernel->name);
            }
        }
    }

    if((vx_status)VX_SUCCESS == status_name)
    {
        for(i=0; i<graph->num_nodes; i++)
        {
            node = graph->nodes[i];

            status_name = ownNodeKernelDeinitKernelName(node);

            if(status_name != (vx_status)VX_SUCCESS )
            {
                VX_PRINT(VX_ZONE_ERROR,"Node kernel name deinit for node at index %d failed\n", i);
                break;
            }
        }
    }

    if(status_name != (vx_status)VX_SUCCESS )
    {
        status = status_name;
    }

    return status;
}

static vx_status ownGraphNodeKernelDeinit(vx_graph graph)
{
    vx_node node;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;

    for(i=0; i<graph->num_nodes; i++)
    {
        node = graph->nodes[i];

        status = ownNodeKernelDeinit(node);
        if(status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"Node kernel de-init for node at index %d failed\n", i);
            break;
        }
    }

    return status;
}

static vx_bool ownGraphIsRefMatch(vx_graph graph, vx_reference ref1, vx_reference ref2)
{
    vx_bool is_match = (vx_bool)vx_false_e;

    if((NULL != ref1) && (NULL != ref2))
    {
        if(ref1 == ref2)
        {
            is_match = (vx_bool)vx_true_e;
        }
        else if (((vx_reference)graph != ref2->scope) &&
            (ref1 == ref2->scope))
        {
            is_match = (vx_bool)vx_true_e;
        }
        else if ( ((vx_enum)VX_TYPE_IMAGE==ref1->type) &&
                  ((vx_enum)VX_TYPE_IMAGE==ref2->type) )
        {
            vx_image image_ref1 = (vx_image)ref1;
            vx_image image_ref2 = (vx_image)ref2;
            vx_image parent_ref1 = image_ref1->parent;
            vx_image parent_ref2 = image_ref2->parent;

            if (NULL != parent_ref1)
            {
                if (parent_ref1 == (vx_image)image_ref2)
                {
                    is_match = (vx_bool)vx_true_e;
                }
            }

            if (NULL != parent_ref2)
            {
                if (parent_ref2 == (vx_image)image_ref1)
                {
                    is_match = (vx_bool)vx_true_e;
                }
            }
        }
        else
        {
            is_match = (vx_bool)vx_false_e;
        }
    }
    return is_match;
}

/* Abstracted check for checking if references match
 * "vx_true_e" will be returned if references match or if references parent object matches */
vx_bool ownGraphCheckIsRefMatch(vx_graph graph, vx_reference ref1, vx_reference ref2)
{
    vx_bool ret = (vx_bool)vx_false_e;
    vx_reference parent_ref_node_cur, parent_ref_node_next;

    parent_ref_node_cur = NULL;
    parent_ref_node_next = NULL;

    if (NULL != ref1)
    {
        if ((vx_bool)vx_true_e == ref1->is_array_element)
        {
            parent_ref_node_cur = ref1->scope;
        }
    }

    if (NULL != ref2)
    {
        if ((vx_bool)vx_true_e == ref2->is_array_element)
        {
            parent_ref_node_next = ref2->scope;
        }
     }

     /* check if input data reference of next node is equal to
        output data reference of current */
     if( (ownGraphIsRefMatch(graph, ref1, ref2) != (vx_bool)vx_false_e) ||
         (ownGraphIsRefMatch(graph, ref1, parent_ref_node_next) != (vx_bool)vx_false_e) ||
         (ownGraphIsRefMatch(graph, parent_ref_node_cur, ref2) != (vx_bool)vx_false_e))
     {
         ret = (vx_bool)vx_true_e;
     }

     return ret;
}

static vx_status ownGraphCalcInAndOutNodes(vx_graph graph)
{
    vx_node node_cur, node_next;
    uint32_t node_cur_idx, node_next_idx;
    uint32_t prm_cur_idx, prm_next_idx;
    uint32_t prm_cur_dir, prm_next_dir;
    vx_reference ref1, ref2;
    vx_status status = (vx_status)VX_SUCCESS;

    for(node_cur_idx=0; node_cur_idx<graph->num_nodes; node_cur_idx++)
    {
        node_cur = graph->nodes[node_cur_idx];

        for(prm_cur_idx=0; prm_cur_idx<ownNodeGetNumParameters(node_cur); prm_cur_idx++)
        {
            prm_cur_dir = (uint32_t)ownNodeGetParameterDir(node_cur, prm_cur_idx);

            ref1 = ownNodeGetParameterRef(node_cur, prm_cur_idx);

            if( (ref1) && ((prm_cur_dir == (uint32_t)VX_OUTPUT) || (prm_cur_dir == (uint32_t)VX_BIDIRECTIONAL)) )
            {
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
                            if( (prm_next_dir == (uint32_t)VX_INPUT) || (prm_next_dir == (uint32_t)VX_BIDIRECTIONAL) )
                            {
                                /* check if input data reference of next node is equal to
                                   output data reference of current */
                                if( ownGraphCheckIsRefMatch(graph, ref1, ref2) != 0 )
                                {
                                    /* add node_next as output node for current node if not already added */
                                    status = ownNodeAddOutNode(node_cur, node_next);

                                    if(status == (vx_status)VX_SUCCESS)
                                    {
                                        /* add node_current as input node for next node if not already added */
                                        status = ownNodeAddInNode(node_next, node_cur);
                                        if (status != (vx_status)VX_SUCCESS)
                                        {
                                            VX_PRINT(VX_ZONE_ERROR,"Add in node at index %d failed\n", node_cur_idx);
                                        }
                                    }
                                    else
                                    {
                                        VX_PRINT(VX_ZONE_ERROR,"Add out node at index %d failed\n", node_cur_idx);
                                    }
                                }
                            }
                            else
                            if( prm_next_dir == (uint32_t)VX_OUTPUT )
                            {
                                vx_reference parent_ref_node_cur, parent_ref_node_next;

                                parent_ref_node_cur = NULL;
                                parent_ref_node_next = NULL;

                                if(0 != node_cur->replicated_flags[prm_cur_idx])
                                {
                                    parent_ref_node_cur = ref1->scope;
                                }

                                if(0 != node_next->replicated_flags[prm_next_idx])
                                {
                                    parent_ref_node_next = ref2->scope;
                                }

                                /* check if any output of next node matches current node
                                 * This would mean two nodes output to same data object which is not allowed
                                 */
                                if( (ownGraphIsRefMatch(graph, ref1, ref2) != (vx_bool)vx_false_e) ||
                                    (ownGraphIsRefMatch(graph, ref1, parent_ref_node_next) != (vx_bool)vx_false_e) ||
                                    (ownGraphIsRefMatch(graph, parent_ref_node_cur, ref2) != (vx_bool)vx_false_e))
                                {
                                    status = (vx_status)VX_FAILURE;
                                    VX_PRINT(VX_ZONE_ERROR,"Output of next node matches current node at index %d failed\n", node_cur_idx);
                                }
                            }
                            else
                            {
                                /* Do nothing as there is no other
                                   direction possible */
                            }
                        }
                    }
                }
            }
        }
    }

    return status;
}

static vx_status ownGraphCalcHeadAndLeafNodes(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_node node;
    uint32_t i;
    uint32_t num_in, num_out;

    graph->num_head_nodes = 0;
    graph->num_leaf_nodes = 0;

    for(i=0; (i<graph->num_nodes) && (status == (vx_status)VX_SUCCESS); i++)
    {
        node = graph->nodes[i];

        if((node->super_node == NULL) ||
           (node->is_super_node == (vx_bool)vx_true_e))
        {

            num_in = ownNodeGetNumInNodes(node);
            num_out = ownNodeGetNumOutNodes(node);

            if(num_in==0U)
            {
                if (graph->num_head_nodes >= TIVX_GRAPH_MAX_HEAD_NODES)
                {
                    graph->num_head_nodes = TIVX_GRAPH_MAX_HEAD_NODES;
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                    VX_PRINT(VX_ZONE_ERROR,"Maximum number of head nodes (%d) exceeded\n", TIVX_GRAPH_MAX_HEAD_NODES);
                    VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_GRAPH_MAX_HEAD_NODES in tiovx/include/TI/tivx_config.h\n");
                }
                else
                {
                    graph->head_nodes[graph->num_head_nodes] = node;
                    graph->num_head_nodes++;
                    ownLogSetResourceUsedValue("TIVX_GRAPH_MAX_HEAD_NODES", (uint16_t)graph->num_head_nodes);
                }
            }
            if((num_out==0U) && (status == (vx_status)VX_SUCCESS))
            {
                if (graph->num_leaf_nodes >= TIVX_GRAPH_MAX_LEAF_NODES)
                {
                    graph->num_leaf_nodes = TIVX_GRAPH_MAX_LEAF_NODES;
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                    VX_PRINT(VX_ZONE_ERROR,"Maximum number of leaf nodes (%d) exceeded\n", TIVX_GRAPH_MAX_LEAF_NODES);
                    VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_GRAPH_MAX_LEAF_NODES in tiovx/include/TI/tivx_config.h\n");
                }
                else
                {
                    graph->leaf_nodes[graph->num_leaf_nodes] = node;
                    graph->num_leaf_nodes++;
                    ownLogSetResourceUsedValue("TIVX_GRAPH_MAX_LEAF_NODES", (uint16_t)graph->num_leaf_nodes);
                }
            }
        }
    }

    return status;
}

static vx_status ownGraphAllocateDataObject(vx_graph graph, vx_node node_cur, uint32_t prm_cur_idx, vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownNodeIsPrmReplicated(node_cur, prm_cur_idx) != 0)
    {
        /* if this is a replicated node, replicated parameter
         * then allocate memory for parent object
         */
        status = ownReferenceAllocMem(ref->scope);
        if (status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"Memory allocation for replicated parameter parent object failed\n");
        }
    }
    else
    if(ref->delay != NULL )
    {
        /* if this is part of delay then allocate memory for all
         * delay objects
         */
        status = ownReferenceAllocMem((vx_reference)ref->delay);
        if (status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"Memory allocation for delay objects failed\n");
        }
    }
    else
    {
        /* alloc memory for data reference, if not already allocated
         * Its ok to call this multiple times for the same reference
         * memory gets allocated only once
         */
        status = ownReferenceAllocMem(ref);
        if (status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"Memory allocation for data reference failed\n");
        }
    }
    return status;
}

static vx_status ownGraphAllocateDataObjects(vx_graph graph)
{
    vx_node node_cur;
    uint32_t node_cur_idx;
    uint32_t prm_cur_idx;
    vx_status status = (vx_status)VX_SUCCESS;
    vx_reference ref;

    for(node_cur_idx=0; node_cur_idx<graph->num_nodes; node_cur_idx++)
    {
        node_cur = graph->nodes[node_cur_idx];

        for(prm_cur_idx=0; prm_cur_idx<ownNodeGetNumParameters(node_cur); prm_cur_idx++)
        {
            ref = ownNodeGetParameterRef(node_cur, prm_cur_idx);

            /* reference could be null for optional parameters */
            if (NULL != ref)
            {
                status = ownGraphAllocateDataObject(graph, node_cur, prm_cur_idx, ref);
                if(status != (vx_status)VX_SUCCESS)
                {
                    break;
                }
            }
        }
        if(status != (vx_status)VX_SUCCESS)
        {
            break;
        }
    }
    return status;
}

static vx_status ownGraphCreateNodeCallbackCommands(vx_graph graph)
{
    vx_node node;
    uint32_t i, pipe_id;
    vx_status status = (vx_status)VX_SUCCESS;

    for(i=0; i<graph->num_nodes; i++)
    {
        node = graph->nodes[i];

        for(pipe_id=0; pipe_id<graph->pipeline_depth; pipe_id++)
        {
            status = ownNodeCreateUserCallbackCommand(node, pipe_id);
            if(status != (vx_status)VX_SUCCESS)
            {
                break;
            }
        }
        if(status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"Node create user callback command at index %d failed\n", i);
            break;
        }
    }

    return status;
}

static void ownGraphLinkArrayElements(vx_graph graph)
{
    vx_node node;
    uint32_t node_id, prm_id;

    /* find the (nodes,index) in the graph where node_prm_ref is used as input/output
     * and insert data ref q handle at those (nodes,index)
     * Also enable data ref queue at those (nodes,index)
     */
    for(node_id=0; node_id<graph->num_nodes; node_id++)
    {
        node = graph->nodes[node_id];

        for(prm_id=0; prm_id<ownNodeGetNumParameters(node); prm_id++)
        {
            vx_reference ref;

            ref = ownNodeGetParameterRef(node, prm_id);
            if(ref!=NULL)
            {
                if((vx_bool)vx_true_e == ref->is_array_element)
                {
                    ownNodeLinkArrayElement(node, prm_id);
                }
            }
        }
    }
}

static void ownGraphLinkDataReferenceQueuesToNodeIndex(vx_graph graph,
                    tivx_data_ref_queue data_ref_q,
                    vx_node node, uint32_t index)
{
    uint32_t node_id, prm_id;
    vx_reference node_prm_ref;

    node_prm_ref = ownNodeGetParameterRef(node, index);

    /* find the (nodes,index) in the graph where node_prm_ref is used as input/output
     * and insert data ref q handle at those (nodes,index)
     * Also enable data ref queue at those (nodes,index)
     */
    for(node_id=0; node_id<graph->num_nodes; node_id++)
    {
        node = graph->nodes[node_id];

        for(prm_id=0; prm_id<ownNodeGetNumParameters(node); prm_id++)
        {
            vx_reference ref;

            ref = ownNodeGetParameterRef(node, prm_id);
            if((ref != NULL) && (node_prm_ref != NULL))
            {
                if(ref==node_prm_ref)
                {
                    ownNodeLinkDataRefQueue(node, prm_id, data_ref_q);
                }
                else
                if(node_prm_ref==ref->scope)
                {
                    ownNodeLinkDataRefQueue(node, prm_id, data_ref_q);
                }
                else
                {
                    /* do nothing */
                }
                if(node_prm_ref->scope==ref)
                {
                    ownNodeLinkDataRefQueue(node, prm_id, data_ref_q);
                }
            }
        }
    }
}

static void ownGraphLinkDataReferenceQueues(vx_graph graph)
{
    uint32_t i;

    for(i=0; i<graph->num_params; i++)
    {
        if(graph->parameters[i].queue_enable != 0)
        {
            ownGraphLinkDataReferenceQueuesToNodeIndex(graph,
                        graph->parameters[i].data_ref_queue,
                        graph->parameters[i].node,
                        graph->parameters[i].index);
        }
    }
    for(i=0; i<graph->num_data_ref_q; i++)
    {
        ownGraphLinkDataReferenceQueuesToNodeIndex(graph,
                    graph->data_ref_q_list[i].data_ref_queue,
                    graph->data_ref_q_list[i].node,
                    graph->data_ref_q_list[i].index);

    }
    for(i=0; i<graph->num_delay_data_ref_q; i++)
    {
        if(graph->delay_data_ref_q_list[i].node!=NULL)
        {
            ownGraphLinkDataReferenceQueuesToNodeIndex(graph,
                    graph->delay_data_ref_q_list[i].data_ref_queue,
                    graph->delay_data_ref_q_list[i].node,
                    graph->delay_data_ref_q_list[i].index);
        }
    }
}

/* enqueue initial references into intermediate data ref queues */
static vx_status ownGraphPrimeDataReferenceQueues(vx_graph graph)
{
    uint32_t i, buf_id;
    vx_reference ref;
    uint16_t ref_obj_desc_id;
    vx_status status = (vx_status)VX_SUCCESS;

    for(i=0; i<graph->num_data_ref_q; i++)
    {
        tivx_data_ref_queue data_ref_q;

        data_ref_q = graph->data_ref_q_list[i].data_ref_queue;

        for(buf_id=0; buf_id<graph->data_ref_q_list[i].num_buf; buf_id++)
        {
            ref = graph->data_ref_q_list[i].refs_list[buf_id];
            if(ref && ref->obj_desc)
            {
                ref_obj_desc_id = ref->obj_desc->obj_desc_id;

                status = ownObjDescQueueEnqueue(data_ref_q->acquire_q_obj_desc_id, ref_obj_desc_id);
            }
            else
            {
                status = (vx_status)VX_FAILURE;
            }
            if(status!=(vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "Unable to prime data ref queue\n");
                break;
            }
        }
    }
    for(i=0; i<graph->num_delay_data_ref_q; i++)
    {
        tivx_data_ref_queue data_ref_q;

        data_ref_q = graph->delay_data_ref_q_list[i].data_ref_queue;

        if(graph->delay_data_ref_q_list[i].node != NULL)
        {
            ref = ownNodeGetParameterRef(graph->delay_data_ref_q_list[i].node, graph->delay_data_ref_q_list[i].index);
            if(ref && ref->obj_desc)
            {
                ref_obj_desc_id = ref->obj_desc->obj_desc_id;

                status = ownObjDescQueueEnqueue(data_ref_q->acquire_q_obj_desc_id, ref_obj_desc_id);
            }
            else
            {
                status = (vx_status)VX_FAILURE;
            }
            if(status!=(vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "Unable to prime delay data ref queue\n");
            }
        }
        else
        {
            vx_delay delay = graph->delay_data_ref_q_list[i].delay_ref;
            uint32_t delay_slot_index = graph->delay_data_ref_q_list[i].delay_slot_index;

            /* data reference queue with no node as input */
            ref = delay->refs[delay_slot_index];
            if(ref && ref->obj_desc)
            {
                ref_obj_desc_id = ref->obj_desc->obj_desc_id;

                status = ownObjDescQueueEnqueue(data_ref_q->acquire_q_obj_desc_id, ref_obj_desc_id);
            }
            else
            {
                status = (vx_status)VX_FAILURE;
            }
            if(status!=(vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "Unable to prime delay data ref queue\n");
            }
        }
    }
    return status;
}

static vx_status ownGraphCheckAndCreateDelayDataReferenceQueues(vx_graph graph,
            vx_node node,
            uint32_t index,
            tivx_data_ref_queue data_ref_q)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_reference ref = ownNodeGetParameterRef(
                            node,
                            index);

    if (ref != NULL)
    {
        if((ownIsValidSpecificReference((vx_reference)ref->delay, (vx_enum)VX_TYPE_DELAY) != (vx_bool)vx_false_e))
        {
            uint32_t delay_slot_index;
            vx_delay delay = (vx_delay)ref->delay;
            tivx_data_ref_queue delay_data_ref_q_list[TIVX_DELAY_MAX_OBJECT];
            vx_bool auto_age_delay_slot[TIVX_DELAY_MAX_OBJECT];

            for(delay_slot_index=0; delay_slot_index<delay->count; delay_slot_index++)
            {
                auto_age_delay_slot[delay_slot_index] = (vx_bool)vx_false_e;
                if((delay->set[delay_slot_index].node ==
                    node)
                    &&
                   ( delay->set[delay_slot_index].index ==
                    index))
                {
                    delay_data_ref_q_list[delay_slot_index] = data_ref_q;
                }
                else
                {
                    if(graph->num_delay_data_ref_q<TIVX_GRAPH_MAX_DATA_REF_QUEUE)
                    {
                        tivx_data_ref_queue_create_params_t data_ref_create_prms;

                        graph->delay_data_ref_q_list[graph->num_delay_data_ref_q].node = delay->set[delay_slot_index].node;
                        graph->delay_data_ref_q_list[graph->num_delay_data_ref_q].index = delay->set[delay_slot_index].index;
                        graph->delay_data_ref_q_list[graph->num_delay_data_ref_q].delay_ref = delay;
                        graph->delay_data_ref_q_list[graph->num_delay_data_ref_q].delay_slot_index = delay_slot_index;

                        data_ref_create_prms.pipeline_depth = graph->pipeline_depth;
                        data_ref_create_prms.enable_user_queueing = (vx_bool)vx_false_e;
                        if(delay->set[delay_slot_index].node!=NULL)
                        {
                            data_ref_create_prms.num_in_nodes = ownGraphGetNumInNodes(
                                        graph, delay->set[delay_slot_index].node, delay->set[delay_slot_index].index);
                        }
                        else
                        {
                            /* this is a data ref q at a delay slot which is not used as input
                             * by any node.
                             * Such a delay slot needs to be auto aged.
                             */
                            data_ref_create_prms.num_in_nodes = 0;
                            auto_age_delay_slot[delay_slot_index] = (vx_bool)vx_true_e;
                        }
                        data_ref_create_prms.is_enable_send_ref_consumed_event =
                                        (vx_bool)vx_false_e;
                        data_ref_create_prms.graph_parameter_index = (uint32_t)-1;

                        graph->delay_data_ref_q_list[graph->num_delay_data_ref_q].data_ref_queue =
                            tivxDataRefQueueCreate(graph, &data_ref_create_prms);

                        if(graph->delay_data_ref_q_list[graph->num_delay_data_ref_q].data_ref_queue == NULL)
                        {
                            status = (vx_status)VX_ERROR_NO_RESOURCES;
                            VX_PRINT(VX_ZONE_ERROR,"Unable to allocate data ref queue for delay \n");
                        }
                        if(status == (vx_status)VX_SUCCESS)
                        {
                            delay_data_ref_q_list[delay_slot_index] = graph->delay_data_ref_q_list[graph->num_delay_data_ref_q].data_ref_queue;
                            graph->num_delay_data_ref_q++;
                        }
                    }
                    else
                    {
                        status = (vx_status)VX_ERROR_NO_RESOURCES;
                        VX_PRINT(VX_ZONE_ERROR,"Exceed number of data reference queue list for delays \n");
                    }
                }
                if(status!=(vx_status)VX_SUCCESS)
                {
                    break;
                }
            }
            if(status==(vx_status)VX_SUCCESS)
            {
                ownDataRefQueueLinkDelayDataRefQueues(delay_data_ref_q_list, auto_age_delay_slot, delay->count);
            }
        }
    }
    else
    {
        vx_reference ref;
        ref = (vx_reference)node;
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR,"Graph parameter of node %s at index %d is NULL \n", ref->name, index);
    }

    return status;
}

static vx_status ownGraphCreateIntermediateDataReferenceQueues(vx_graph graph)
{
    tivx_data_ref_queue_create_params_t data_ref_create_prms;
    uint32_t i;
    vx_status status = (vx_status)VX_SUCCESS;

    for(i=0; i<graph->num_data_ref_q; i++)
    {
        data_ref_create_prms.pipeline_depth = graph->pipeline_depth;
        data_ref_create_prms.enable_user_queueing = (vx_bool)vx_false_e;
        data_ref_create_prms.num_in_nodes = ownGraphGetNumInNodes(
                        graph, graph->data_ref_q_list[i].node, graph->data_ref_q_list[i].index);
        data_ref_create_prms.is_enable_send_ref_consumed_event = (vx_bool)vx_false_e;
        data_ref_create_prms.graph_parameter_index = (uint32_t)-1;

        graph->data_ref_q_list[i].data_ref_queue =
            tivxDataRefQueueCreate(graph, &data_ref_create_prms);

        if(graph->data_ref_q_list[i].data_ref_queue == NULL)
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
        /* check for and handle delay's */
        if(status==(vx_status)VX_SUCCESS)
        {
            status = ownGraphCheckAndCreateDelayDataReferenceQueues(graph,
                graph->data_ref_q_list[i].node,
                graph->data_ref_q_list[i].index,
                graph->data_ref_q_list[i].data_ref_queue
                );
        }
        if(status==(vx_status)VX_SUCCESS)
        {
            uint32_t buf_id;

            for(buf_id=0; buf_id<graph->data_ref_q_list[i].num_buf; buf_id++)
            {
                if(graph->data_ref_q_list[i].refs_list[buf_id]!=NULL)
                {
                    /* alloc memory for references that can be enqueued in data ref queues */
                    status = ownGraphAllocateDataObject(graph,
                            graph->data_ref_q_list[i].node, graph->data_ref_q_list[i].index,
                            graph->data_ref_q_list[i].refs_list[buf_id]);
                }
                else
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Invalid reference in refs_list\n");
                }
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate data object memory \n");
                    break;
                }
            }
        }
        if(status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to create data ref queue\n");
            break;
        }
    }

    return status;
}

static vx_status ownGraphCreateGraphParameterDataReferenceQueues(vx_graph graph)
{
    tivx_data_ref_queue_create_params_t data_ref_create_prms;
    uint32_t i;
    vx_status status = (vx_status)VX_SUCCESS;

    for(i=0; i<graph->num_params; i++)
    {
        if(graph->parameters[i].queue_enable != 0)
        {
            data_ref_create_prms.pipeline_depth = graph->pipeline_depth;
            data_ref_create_prms.enable_user_queueing = (vx_bool)vx_true_e;
            data_ref_create_prms.num_in_nodes = ownGraphGetNumInNodes(
                            graph, graph->parameters[i].node, graph->parameters[i].index);
            data_ref_create_prms.is_enable_send_ref_consumed_event =
                            graph->parameters[i].is_enable_send_ref_consumed_event;
            data_ref_create_prms.graph_parameter_index = i;

            graph->parameters[i].data_ref_queue =
                tivxDataRefQueueCreate(graph, &data_ref_create_prms);

            if(graph->parameters[i].data_ref_queue == NULL)
            {
                status = (vx_status)VX_ERROR_NO_RESOURCES;
            }
            /* check for and handle delay's */
            if(status==(vx_status)VX_SUCCESS)
            {
                status = ownGraphCheckAndCreateDelayDataReferenceQueues(graph,
                    graph->parameters[i].node,
                    graph->parameters[i].index,
                    graph->parameters[i].data_ref_queue
                    );
            }
            if(status==(vx_status)VX_SUCCESS)
            {
                uint32_t buf_id;

                for(buf_id=0; buf_id<graph->parameters[i].num_buf; buf_id++)
                {
                    if(graph->parameters[i].refs_list[buf_id]!=NULL)
                    {
                        /* alloc memory for references that can be enqueued in data ref queues */
                        status = ownGraphAllocateDataObject(graph,
                                graph->parameters[i].node, graph->parameters[i].index,
                                graph->parameters[i].refs_list[buf_id]);
                    }
                    else
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "Invalid reference in refs_list\n");
                    }
                    if(status != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate data object memory \n");
                        break;
                    }
                }
            }
            if(status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "Unable to create data ref queue\n");
                break;
            }
        }
    }

    return status;
}

static vx_status ownGraphUpdateImageRefAfterKernetInit(vx_graph graph, vx_image exemplar, vx_image ref)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if((exemplar != NULL) && (ref != NULL))
    {
        tivx_obj_desc_image_t *img_ref_obj_desc = (tivx_obj_desc_image_t *)ref->base.obj_desc;
        tivx_obj_desc_image_t *img_exemplar_obj_desc = (tivx_obj_desc_image_t *)exemplar->base.obj_desc;

        if((img_ref_obj_desc != NULL)
            && (img_exemplar_obj_desc != NULL)
            && (img_ref_obj_desc->base.type == (uint32_t)TIVX_OBJ_DESC_IMAGE)
            && (img_exemplar_obj_desc->base.type == (uint32_t)TIVX_OBJ_DESC_IMAGE)
            )
        {
            tivx_obj_desc_memcpy(&img_ref_obj_desc->valid_roi, &img_exemplar_obj_desc->valid_roi, (uint32_t)sizeof(img_exemplar_obj_desc->valid_roi));
        }
        else
        {
            status = (vx_status)VX_FAILURE;
        }
    }
    else
    {
        status = (vx_status)VX_FAILURE;
    }
    if(status!=(vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to update image meta data after kernel init\n");
    }
    return status;
}

static vx_status ownGraphUpdatePyramidRefAfterKernetInit(vx_graph graph, vx_pyramid exemplar, vx_pyramid ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_size ref_levels, exemplar_levels, i;

    tivxCheckStatus(&status, vxQueryPyramid(exemplar, (vx_enum)VX_PYRAMID_LEVELS, &exemplar_levels, sizeof(exemplar_levels)));
    tivxCheckStatus(&status, vxQueryPyramid(ref, (vx_enum)VX_PYRAMID_LEVELS, &ref_levels, sizeof(ref_levels)));

    if((ref_levels == exemplar_levels) && (status == (vx_status)VX_SUCCESS))
    {
        for(i=0; i<ref_levels; i++)
        {
            tivxCheckStatus(&status, ownGraphUpdateImageRefAfterKernetInit(graph, exemplar->img[i], ref->img[i]));
        }
    }
    if(status!=(vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to update pyramid meta data after kernel init\n");
    }
    return status;
}

static vx_status ownGraphUpdateObjArrRefAfterKernetInit(vx_graph graph, vx_object_array exemplar, vx_object_array ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_size ref_count, exemplar_count, i;

    tivxCheckStatus(&status, vxQueryObjectArray(exemplar, (vx_enum)VX_OBJECT_ARRAY_NUMITEMS, &exemplar_count, sizeof(exemplar_count)));
    tivxCheckStatus(&status, vxQueryObjectArray(ref, (vx_enum)VX_OBJECT_ARRAY_NUMITEMS, &ref_count, sizeof(ref_count)));

    if((ref_count == exemplar_count) && (status == (vx_status)VX_SUCCESS))
    {
        for(i=0; i<ref_count; i++)
        {
            if ((ownIsValidSpecificReference(ref->ref[i], (vx_enum)VX_TYPE_IMAGE) == (vx_bool)vx_true_e)
              && (ownIsValidSpecificReference(exemplar->ref[i], (vx_enum)VX_TYPE_IMAGE) == (vx_bool)vx_true_e))
            {
                tivxCheckStatus(&status, ownGraphUpdateImageRefAfterKernetInit(graph, (vx_image)exemplar->ref[i], (vx_image)ref->ref[i]));
            }
            else
            if ((ownIsValidSpecificReference(ref->ref[i], (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
              && (ownIsValidSpecificReference(exemplar->ref[i], (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e))
            {
                tivxCheckStatus(&status, ownGraphUpdatePyramidRefAfterKernetInit(graph, (vx_pyramid)exemplar->ref[i], (vx_pyramid)ref->ref[i]));
            }
            else
            {
                /* do nothing */
            }
        }
    }
    if(status!=(vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to update pyramid meta data after kernel init\n");
    }
    return status;
}

static vx_status ownGraphUpdateDataRefAfterKernetInit(vx_graph graph, vx_reference exemplar, vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((ownIsValidSpecificReference(ref, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
        && (ownIsValidSpecificReference(exemplar, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e))
    {
        status = ownGraphUpdatePyramidRefAfterKernetInit(graph, (vx_pyramid)exemplar, (vx_pyramid)ref);
    }
    else
    if ((ownIsValidSpecificReference(ref, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
        && (ownIsValidSpecificReference(exemplar, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e))
    {
        status = ownGraphUpdateObjArrRefAfterKernetInit(graph, (vx_object_array)exemplar, (vx_object_array)ref);
    }
    else
    if ((ownIsValidSpecificReference(ref, (vx_enum)VX_TYPE_IMAGE) == (vx_bool)vx_true_e)
      && (ownIsValidSpecificReference(exemplar, (vx_enum)VX_TYPE_IMAGE) == (vx_bool)vx_true_e))
    {
        status = ownGraphUpdateImageRefAfterKernetInit(graph, (vx_image)exemplar, (vx_image)ref);
    }
    else
    {
        /* do nothing */
    }

    /* below is to take care of replicate case */
    if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
      && (ownIsValidSpecificReference(exemplar->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e))
    {
        status = ownGraphUpdatePyramidRefAfterKernetInit(graph, (vx_pyramid)exemplar->scope, (vx_pyramid)ref->scope);
    }
    else
    if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
      && (ownIsValidSpecificReference(exemplar->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e))
    {
        status = ownGraphUpdateObjArrRefAfterKernetInit(graph, (vx_object_array)exemplar->scope, (vx_object_array)ref->scope);
    }
    else
    {
        /* do nothing */
    }
    if(status!=(vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to update data ref queue data refs meta data after kernel init\n");
    }

    return status;
}

/* kernel init could update meta data for some data ref's like valid_roi for image
 * This get updated for 0th data ref in a data ref queue
 * This function updates for other ref's in a data ref queue based on
 * updates in 0th data ref
 */
static vx_status ownGraphUpdateDataReferenceQueueRefsAfterKernelInit(vx_graph graph)
{
    uint32_t buf_id, i;
    vx_status status = (vx_status)VX_SUCCESS;

    for(i=0; i<graph->num_params; i++)
    {
        if(graph->parameters[i].queue_enable != 0)
        {
            for(buf_id=1; buf_id<graph->parameters[i].num_buf; buf_id++)
            {
                if((graph->parameters[i].refs_list[buf_id] != NULL)
                    && (graph->parameters[i].refs_list[0] != NULL))
                {
                    status = ownGraphUpdateDataRefAfterKernetInit(graph,
                            graph->parameters[i].refs_list[0], /* exemplar */
                            graph->parameters[i].refs_list[buf_id]);
                }
            }
        }
    }
    for(i=0; i<graph->num_data_ref_q; i++)
    {
        for(buf_id=1; buf_id<graph->data_ref_q_list[i].num_buf; buf_id++)
        {
            if((graph->data_ref_q_list[i].refs_list[buf_id] != NULL)
              && (graph->data_ref_q_list[i].refs_list[0] != NULL))
            {
                status = ownGraphUpdateDataRefAfterKernetInit(graph,
                        graph->data_ref_q_list[i].refs_list[0], /* exemplar */
                        graph->data_ref_q_list[i].refs_list[buf_id]);
            }
        }
    }
    return status;
}

static vx_status ownGraphCreateAndLinkDataReferenceQueues(vx_graph graph)
{
    vx_status status;

    status = ownGraphCreateGraphParameterDataReferenceQueues(graph);
    if(status==(vx_status)VX_SUCCESS)
    {
        status = ownGraphCreateIntermediateDataReferenceQueues(graph);
    }
    if(status==(vx_status)VX_SUCCESS)
    {
        status = ownGraphPrimeDataReferenceQueues(graph);
    }
    if(status==(vx_status)VX_SUCCESS)
    {
        ownGraphLinkDataReferenceQueues(graph);
    }
    return status;
}

/* called during graph verify,
 * this function is called for non-graph parameters that are intermediate data objects
 * within a graph
 */
static vx_status ownGraphAddDataRefQ(vx_graph graph, vx_node node, uint32_t index)
{
    vx_bool skip_add_data_ref_q = (vx_bool)vx_false_e;
    vx_status status = (vx_status)VX_SUCCESS;
    vx_reference param_ref;

    param_ref = ownNodeGetParameterRef(node, index);

    /* Dont make a data ref queue if below is true
     * - if node parameter is input
     * - or node parameter is output but this is a leaf node
     *   - Note: exception here is if it is a delay b/c the delay slot in question
     *           may not be connected to another node
     * - or no node reference specified at the node,index
     * Here no data ref queue is required since if user really wanted to access
     * the data ref, user would have a graph parameter out of this node,index
     */
    if((ownNodeGetParameterDir(node, index) != (vx_enum)VX_OUTPUT) /* input parameter */
        || (param_ref == NULL) /* no reference specified at node,index */
        || ((ownGraphGetNumInNodes(graph, node, index) == 0U) && /* leaf parameter and not a delay */
             !(ownIsValidSpecificReference((vx_reference)param_ref->delay, (vx_enum)VX_TYPE_DELAY) != (vx_bool)vx_false_e))
        )
    {
        skip_add_data_ref_q = (vx_bool)vx_true_e;
    }

    if(skip_add_data_ref_q==(vx_bool)vx_false_e)
    {
        uint32_t i;

        /* check if (node, index) is a graph parameter and if queueing is already enabled,
        * if yes then do nothing */
        for(i=0; i<graph->num_params; i++)
        {
            if((graph->parameters[i].node==node) &&
                (graph->parameters[i].index==index) &&
                (graph->parameters[i].queue_enable == (vx_bool)vx_true_e))
            {
                skip_add_data_ref_q = (vx_bool)vx_true_e;
                break;
            }
        }
    }

    if(skip_add_data_ref_q==(vx_bool)vx_false_e)
    {
        if(graph->num_data_ref_q<TIVX_GRAPH_MAX_DATA_REF_QUEUE)
        {
            uint32_t num_buf;
            vx_reference exemplar;

            graph->data_ref_q_list[graph->num_data_ref_q].node = node;
            graph->data_ref_q_list[graph->num_data_ref_q].index = index;
            graph->data_ref_q_list[graph->num_data_ref_q].num_buf = 1;
            graph->data_ref_q_list[graph->num_data_ref_q].data_ref_queue = NULL;
            graph->data_ref_q_list[graph->num_data_ref_q].refs_list[0] = ownNodeGetParameterRef(node, index);

            /* if user has requested more than 1 buf at this node, then allocate the additional references */
            num_buf = ownNodeGetParameterNumBuf(node, index);

            if(num_buf>0U)
            {
                uint32_t buf_id;
                vx_bool is_replicated;

                is_replicated = ownNodeIsPrmReplicated(node, index);

                exemplar = graph->data_ref_q_list[graph->num_data_ref_q].refs_list[0];
                if(is_replicated != 0)
                {
                    exemplar = exemplar->scope;
                }
                for(buf_id=1; buf_id<num_buf; buf_id++)
                {
                    vx_reference ref;
                    ref = ownCreateReferenceFromExemplar(graph->base.context, exemplar);
                    if(ref==NULL)
                    {
                        status = (vx_status)VX_ERROR_NO_RESOURCES;
                        VX_PRINT(VX_ZONE_ERROR,"Unable to create references\n");
                    }
                    if(status==(vx_status)VX_SUCCESS)
                    {
                        if(is_replicated != 0)
                        {
                            if (ownIsValidSpecificReference(ref, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
                            {
                                vx_pyramid pyramid = (vx_pyramid)ref;
                                ref = (vx_reference)pyramid->img[0];
                            }
                            else if (ownIsValidSpecificReference(ref, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
                            {
                                vx_object_array object_array = (vx_object_array)ref;
                                ref = object_array->ref[0];
                            }
                            else
                            {
                                VX_PRINT(VX_ZONE_ERROR,"Invalid reference type for replicated parameter\n");
                                status = (vx_status)VX_FAILURE;
                                ref = NULL;
                            }
                        }
                        graph->data_ref_q_list[graph->num_data_ref_q].refs_list[buf_id] = ref;
                    }
                    if(status!=(vx_status)VX_SUCCESS)
                    {
                        break;
                    }
                }
                if(status==(vx_status)VX_SUCCESS)
                {
                    graph->data_ref_q_list[graph->num_data_ref_q].num_buf = num_buf;
                }
            }
            if(status==(vx_status)VX_SUCCESS)
            {
                graph->num_data_ref_q++;
                ownLogSetResourceUsedValue("TIVX_GRAPH_MAX_DATA_REF_QUEUE", (uint16_t)graph->num_data_ref_q);
            }
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
            VX_PRINT(VX_ZONE_ERROR, "Unable to add data ref q to graph since list is full \n");
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_GRAPH_MAX_DATA_REF_QUEUE in tiovx/include/TI/tivx_config.h\n");
        }
    }
    return status;
}

static vx_status ownGraphNodePipeline(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t node_id, calculated_pipe_depth;

    if ((vx_bool)vx_true_e == graph->is_pipelining_enabled)
    {
        calculated_pipe_depth = ownGraphGetPipeDepth(graph);

        if ((vx_bool)vx_false_e == graph->is_pipeline_depth_set)
        {
            graph->pipeline_depth = calculated_pipe_depth;
        }
        else
        {
            if (calculated_pipe_depth > graph->pipeline_depth)
            {
                VX_PRINT(VX_ZONE_OPTIMIZATION, "Pipe depth value of %d set via tivxSetGraphPipelineDepth may not be optimal\n", graph->pipeline_depth);
                VX_PRINT(VX_ZONE_OPTIMIZATION, "The calculated pipe depth value is %d\n", calculated_pipe_depth);
            }
        }
    }

    for(node_id=0; node_id<graph->num_nodes; node_id++)
    {
        if(graph->nodes[node_id]->kernel->num_pipeup_bufs >= graph->pipeline_depth)
        {
            graph->pipeline_depth = graph->nodes[node_id]->kernel->num_pipeup_bufs;
        }
    }

    for(node_id=0; node_id<graph->num_nodes; node_id++)
    {
        ownNodeSetObjDescParamDirection(graph->nodes[node_id]);

        status = ownNodeAllocObjDescForPipeline(graph->nodes[node_id], graph->pipeline_depth);
        if(status!=(vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"Unable to alloc obj descriptors at node for pipelining\n");
            break;
        }
    }

    if(status==(vx_status)VX_SUCCESS)
    {
        for(node_id=0; node_id<graph->num_nodes; node_id++)
        {
            /* update out_node_id[], in_node_id[]
             * from 0th obj desc
             */
            ownNodeLinkObjDescForPipeline(graph->nodes[node_id]);
        }
    }

    if(status==(vx_status)VX_SUCCESS)
    {
        /* make data references at graph parameter only if graph is in queuing mode */
        if((graph->schedule_mode == (vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO)
            ||
            (graph->schedule_mode == (vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL)
            ||
            (graph->pipeline_depth > 1U)
            )
        {
            for(node_id=0; node_id<graph->num_nodes; node_id++)
            {
                uint32_t prm_id;
                vx_node node;

                node = graph->nodes[node_id];

                for(prm_id=0; prm_id<ownNodeGetNumParameters(node); prm_id++)
                {
                    status = ownGraphAddDataRefQ(graph, node, prm_id);
                    if(status!=(vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Unable to add data ref q to graph\n");
                        break;
                    }
                }
                if(status!=(vx_status)VX_SUCCESS)
                {
                    break;
                }
            }
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxVerifyGraph(vx_graph graph)
{
    uint32_t i;
    vx_status status = (vx_status)VX_SUCCESS;
    vx_meta_format meta[TIVX_KERNEL_MAX_PARAMS] = {NULL};
    vx_bool first_time_verify = (vx_bool)vx_true_e;

    if (NULL != graph)
    {
        first_time_verify = ((graph->verified == (vx_bool)vx_false_e) &&
            (graph->reverify == (vx_bool)vx_false_e)) ? (vx_bool)vx_true_e : (vx_bool)vx_false_e;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid graph reference\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    for (i = 0; (i < TIVX_KERNEL_MAX_PARAMS) && ((vx_status)VX_SUCCESS == status); i ++)
    {
        meta[i] = ownCreateMetaFormat(graph->base.context);

        /* This should not fail at all */
        if (vxGetStatus((vx_reference)meta[i]) != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"Unable to create meta format object\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }

    if((ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH)) &&
        ((vx_status)VX_SUCCESS == status))
    {
        ownReferenceLock(&graph->base);

        graph->verified = (vx_bool)vx_false_e;

        if(first_time_verify == (vx_bool)vx_false_e)
        {
            ownGraphNodeKernelDeinit(graph);
        }
        {
            /* Find out nodes and in nodes for each node in the graph
             * No resources are allocated in this step
             */
            status = ownGraphCalcInAndOutNodes(graph);
            if(status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,"Unable to calculate out nodes and in nodes for each node\n");
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                vx_bool has_cycle;

                ownContextLock(graph->base.context);

                /* Topological sort graph to find cycles
                 */
                ownGraphTopologicalSort(
                            &graph->base.context->graph_sort_context,
                            graph->nodes,
                            graph->num_nodes,
                            &has_cycle);

                ownContextUnlock(graph->base.context);

                if(has_cycle != 0)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Topological sort failed, due to cycles in graph\n");
                    status = (vx_status)VX_FAILURE;
                }
            }

            /* Detects connection between source and sink nodes */
            if(status == (vx_status)VX_SUCCESS)
            {
                ownGraphDetectSourceSink(graph);
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* Call validate function for each node
                 * If validation fails then return with error
                 * No resources are allocated in this step
                 */
                status = ownGraphNodeKernelValidate(graph, meta);

                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Node kernel Validate failed\n");
                }
            }

            /* Detects errors in pipelining parameters being set as graph parameter and multiple buffers at node */
            if(status == (vx_status)VX_SUCCESS)
            {
                status = ownGraphValidatePipelineParameters(graph);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Error in pipelining parameters\n");
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* Configure graph processing to account for any super nodes
                 */
                status = ownGraphSuperNodeConfigure(graph);
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* Find head nodes and leaf nodes
                 * in graph
                 * No resources are allocated in this step
                 */
                status = ownGraphCalcHeadAndLeafNodes(graph);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Find head nodes and leaf nodes failed\n");
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                status = ownGraphFindAndAddDataReferences(graph);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Find and add data references failed\n");
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* detect num bufs */
                ownGraphDetectAndSetNumBuf(graph);
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* Allocate memory associated with data objects of this graph
                 * Memory resources are allocated in this step
                 * No need to free them in case of error, since they get free'ed during
                 * data object release
                 */
                status = ownGraphAllocateDataObjects(graph);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Memory alloc for data objects failed\n");
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* Pipeline node objects */
                status = ownGraphNodePipeline(graph);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Node pipelining failed\n");
                }
            }


            if(status == (vx_status)VX_SUCCESS)
            {
                /* Set node callback commands
                 * If case of any error these command are free'ed during
                 * graph release
                 */
                status = ownGraphCreateNodeCallbackCommands(graph);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Create node callback commands failed\n");
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* link array elements to node parameters */
                ownGraphLinkArrayElements(graph);
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* create and link data references queues to node parameters */
                status = ownGraphCreateAndLinkDataReferenceQueues(graph);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Create data ref queues failed\n");
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* Call target kernel init for each node
                 * This results in message coomunication with target kernels
                 * Memory gets allocated which MUST be free'ed via target kernel
                 * deinit.
                 * kernel deinit called during node release
                 */
                status = ownGraphNodeKernelInit(graph);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Node kernel init failed\n");
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* update data refs within data ref queues for meta data updated during kernel init */
                status = ownGraphUpdateDataReferenceQueueRefsAfterKernelInit(graph);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Unable to update data ref queue refs for graph\n");
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* alloc object descriptor for graph and enqueue them */
                status = ownGraphAllocAndEnqueueObjDescForPipeline(graph);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Unable to alloc obj desc for graph\n");
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* alloc everything for streaming */
                status = ownGraphAllocForStreaming(graph);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Unable to alloc streaming objects for graph\n");
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* verify graph schedule mode with streaming */
                status = ownGraphVerifyStreamingMode(graph);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"If streaming is enabled, schedule mode must be normal\n");
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* everything passed, now graph is verified */
                graph->verified = (vx_bool)vx_true_e;
                graph->reverify = (vx_bool)vx_false_e;
                graph->state = (vx_enum)VX_GRAPH_STATE_VERIFIED;
            }

            if(status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,"Graph verify failed\n");
                /* deinit kernel to recover resources */
                ownGraphNodeKernelDeinit(graph);
            }

        }

        ownReferenceUnlock(&graph->base);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid graph reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    for (i = 0; i < TIVX_KERNEL_MAX_PARAMS; i ++)
    {
        if (NULL != meta[i])
        {
            ownReleaseMetaFormat(&meta[i]);
        }
    }

    return status;
}
