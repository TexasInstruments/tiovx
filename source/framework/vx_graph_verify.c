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
static vx_status ownGraphAddDataRefQ(vx_graph graph, vx_node node, uint32_t idx);
static vx_status ownGraphDetectSourceSink(vx_graph graph);
static vx_status ownGraphAddDataReference(vx_graph graph, vx_reference ref, uint32_t prm_dir, uint32_t check);
static vx_status ownGraphCheckAndCreateDelayDataReferenceQueues(vx_graph graph, vx_node node,
                                                                uint32_t idx, tivx_data_ref_queue data_ref_q);
static vx_status ownGraphCreateAndLinkDataReferenceQueues(vx_graph graph);
static vx_status ownGraphCreateGraphParameterDataReferenceQueues(vx_graph graph);
static vx_status ownGraphCreateIntermediateDataReferenceQueues(vx_graph graph);
static vx_status ownGraphFindAndAddDataReferences(vx_graph graph);
static uint32_t ownGraphGetNumInNodes(vx_graph graph, vx_node node, uint32_t node_prm_idx);
static void ownGraphLinkArrayElements(vx_graph graph);
static void ownGraphLinkDataReferenceQueues(vx_graph graph);
static void ownGraphLinkDataReferenceQueuesToNodeIndex(vx_graph graph, tivx_data_ref_queue data_ref_q,
                                                       vx_node node, uint32_t idx);
static vx_status ownGraphNodePipeline(vx_graph graph);
static vx_status ownGraphPrimeDataReferenceQueues(vx_graph graph);
static vx_status ownGraphUpdateDataRefAfterKernetInit(vx_reference exemplar, vx_reference ref);
static vx_status ownGraphUpdateDataReferenceQueueRefsAfterKernelInit(vx_graph graph);
static vx_status ownGraphUpdateImageRefAfterKernetInit(vx_image exemplar, vx_image ref);
static vx_status ownGraphUpdateObjArrRefAfterKernetInit(vx_object_array exemplar, vx_object_array ref);
static vx_status ownGraphUpdatePyramidRefAfterKernetInit(vx_pyramid exemplar, vx_pyramid ref);
static vx_status ownGraphAddSingleDataReference(vx_graph graph, vx_reference ref, uint32_t prm_dir, uint32_t check);

/* Validate graph parameters; two parameters should not point to the same node & index, and two parameters should not have the same initial reference set. */
static vx_status ownGraphValidateParameters(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32  param_idx;
    for (param_idx = 1; param_idx < graph->num_params; ++param_idx)
    {
        vx_uint32 nxt_idx = 0U;
        vx_reference param_ref = graph->parameters[param_idx-1U].node->parameters[graph->parameters[param_idx-1U].index];

        for (nxt_idx = param_idx; nxt_idx < graph->num_params; ++nxt_idx)
        {
            if (param_ref == graph->parameters[nxt_idx].node->parameters[graph->parameters[nxt_idx].index])
            {
                status =(vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Invalid graph parameters: #%u and #%u both attached to reference \"%s\"\n", param_idx - 1U, nxt_idx, param_ref->name);
            }
        }
    }
    return status;
}

/* Link graph parameters if not pipelining or streaming */
static vx_status ownGraphLinkParameters(vx_graph graph)
{
    vx_uint32 p_index;
    vx_status status = (vx_status)VX_SUCCESS;
    for (p_index = 0; p_index < graph->num_params; ++p_index)
    {
        vx_node node = graph->parameters[p_index].node;
        vx_uint32 index = graph->parameters[p_index].index;
        vx_reference ref = node->parameters[index];
        vx_uint32 node_idx;
        graph->parameters[p_index].num_other = 0;
        for (node_idx = 0; node_idx < graph->num_nodes;++node_idx)
        {
            vx_node this_node = graph->nodes[node_idx];
            vx_uint32 this_index;
            for (this_index = 0; this_index < this_node->kernel->signature.num_parameters; this_index++)
            {
                if ((this_node != node) || (this_index != index))
                {
                    if (this_node->parameters[this_index] == ref)
                    {
                        if (graph->parameters[p_index].num_other >= TIVX_GRAPH_MAX_PARAM_REFS)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Too many linked references for graph parameter %d, increase TIVX_GRAPH_MAX_PARAM_REFS\n", p_index);
                            status = (vx_status)VX_ERROR_NO_RESOURCES;
                            break;
                        }
                        else
                        {               
                            /* we have another occurrence of the reference in the graph, record it */
                            graph->parameters[p_index].params_list[graph->parameters[p_index].num_other].node = this_node;
                            graph->parameters[p_index].params_list[graph->parameters[p_index].num_other].index = this_index;
                            graph->parameters[p_index].num_other++;
                        }
                    }
                }
            }
        }
    }
    return status;
}

/* Add's data reference to a list, increments number of times it is referred as input node */
static vx_status ownGraphAddDataReference(vx_graph graph, vx_reference ref, uint32_t prm_dir, uint32_t check)
{
    vx_status status = (vx_status)VX_FAILURE;

    status = ownGraphAddSingleDataReference(graph, ref, prm_dir, check);

    if ((vx_status)VX_SUCCESS==status) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR001 */
    {
        if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
                ||
            (ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
           )
        {
            /* Check set to 0 because a data_ref_num_in_nodes is not supposed to be set for parent object */
            /* Note: given that object arrays/pyramid cannot be made to have parent objects themselves, this
             * is a valid way of adding these data references, rather than needing a recursive check */
            status = ownGraphAddSingleDataReference(graph, ref->scope, prm_dir, 0);
        }
    }

    return status;
}

static vx_status ownGraphAddSingleDataReference(vx_graph graph, vx_reference ref, uint32_t prm_dir, uint32_t check)
{
    uint32_t i;
    vx_status status = (vx_status)VX_FAILURE;

    for(i=0; i<graph->num_data_ref; i++)
    {
        if((check != 0U) && (ownGraphCheckIsRefMatch(graph, graph->data_ref[i], ref) != (vx_bool)vx_false_e))
        {
            /* increment num_in_node count for ref */
            if((uint32_t)VX_OUTPUT != prm_dir) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR002 */
            {
                /* Input or bidirectional */
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
        if((uint32_t)VX_OUTPUT != prm_dir)
        {
            /* input */
            graph->data_ref_num_in_nodes[i]++;
        }
        graph->num_data_ref++;
        ownLogSetResourceUsedValue("TIVX_GRAPH_MAX_DATA_REF", (uint16_t)graph->num_data_ref);
        status = (vx_status)VX_SUCCESS;
    }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM001 */
    else if (graph->num_data_ref >= TIVX_GRAPH_MAX_DATA_REF)
    {
        VX_PRINT(VX_ZONE_WARNING, "May need to increase the value of TIVX_GRAPH_MAX_DATA_REF in tiovx/include/TI/tivx_config.h\n");
    }
#endif
    else
    {
        /* do nothing */
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

        if (NULL != cur_node) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR003 */
        {
            if (cur_node->kernel->num_pipeup_bufs > 1U)
            {
                num_out_nodes = (uint16_t)ownNodeGetNumOutNodes(cur_node);

                for(out_node_idx=0; out_node_idx < num_out_nodes; out_node_idx++)
                {
                    next_node = ownNodeGetNextNode(cur_node, out_node_idx);

                    if (NULL != next_node) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR004 */
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
    if(ref != NULL) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR005 */
    {
        for(i=0; i<graph->num_data_ref; i++) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR006 */
        {
            if (i < TIVX_GRAPH_MAX_DATA_REF) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR007 */
            {
                if(ownGraphCheckIsRefMatch(graph, graph->data_ref[i], ref) != 0)
                {
                    num_in_nodes = graph->data_ref_num_in_nodes[i];
                    break;
                }
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
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM002 */
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Unable to add data reference to data reference list in graph\n");
                    break;
                }
#endif
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

        if( ((vx_enum)VX_OUTPUT != node->kernel->signature.directions[i])
            &&
            (NULL != ref)
            &&
            (ref->type == (vx_enum)VX_TYPE_IMAGE)
            )
        {
            graph->in_valid_rect_ptr[num_in_image] = &graph->in_valid_rect[num_in_image];

            /* status set to NULL due to preceding type check */
            tivxCheckStatus(&status, vxGetValidRegionImage(vxCastRefAsImage(ref, NULL), &graph->in_valid_rect[num_in_image]));

            num_in_image++;
        }
    }

    if(status == (vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR008 */
    {
        for (i = 0; i < num_params; i ++)
        {
            ref = node->parameters[i];
            mf = meta[i];

            if( ((vx_enum)VX_INPUT != node->kernel->signature.directions[i])
                &&
                (NULL != ref)
                &&
                (NULL != mf) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR009 */
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
                    if(tmp_status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR010 */
                    {
                        /* status set to NULL due to preceding type check */
                        tivxCheckStatus(&status, vxSetImageValidRectangle(vxCastRefAsImage(ref, NULL), &graph->out_valid_rect[0]));
                    }
                }
                else
                if(ref->type == (vx_enum)VX_TYPE_PYRAMID) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR011 */
                {
                    vx_status tmp_status;
                    vx_size levels, k;

                    /* status set to NULL due to preceding type check */
                    tivxCheckStatus(&status, vxQueryPyramid(vxCastRefAsPyramid(ref, NULL), (vx_enum)VX_PYRAMID_LEVELS, &levels, sizeof(levels)));

                    if(status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR012 */
                    {
                        for(k=0; k<levels; k++)
                        {
                            graph->out_valid_rect_ptr[k] = &graph->out_valid_rect[k];
                        }

                        tmp_status = mf->valid_rect_callback(node, i,
                                        (const vx_rectangle_t* const*)graph->in_valid_rect_ptr,
                                        graph->out_valid_rect_ptr);
                        if(tmp_status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR013 */
                        {
                            for(k=0; k<levels; k++)
                            {
                                /* status set to NULL due to preceding type check */
                                vx_image img = vxGetPyramidLevel(vxCastRefAsPyramid(ref, NULL), (uint32_t)k);

                                tivxCheckStatus(&status, vxSetImageValidRectangle(img, &graph->out_valid_rect[k]));

                                (void)vxReleaseImage(&img);
                            }
                        }
                    }
                }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM003 */
                else
                {
                    /* not supported for other references */
                    status = (vx_status)VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR,"not supported for references other than image or pyramid\n");
                }
#endif
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
            if (((vx_enum)VX_OUTPUT == node->kernel->signature.directions[i]) &&
                ((vx_bool)vx_true_e == ref->is_virtual))
            {
                if ((ref->scope->type == (vx_enum)VX_TYPE_GRAPH) && (ref->scope != vxCastRefFromGraph(graph)))
                {
                    status = (vx_status)VX_ERROR_INVALID_SCOPE;
                    VX_PRINT(VX_ZONE_ERROR,"invalid scope\n");
                }

                if ((vx_status)VX_SUCCESS == status)
                {
                    switch (mf->type) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR071 */
                    {
                        case (vx_enum)VX_TYPE_SCALAR:
                            /* status set to NULL due to preceding type check */
                            status = vxQueryScalar(vxCastRefAsScalar(ref, NULL),
                                (vx_enum)VX_SCALAR_TYPE, &type, sizeof(type));
                            /* For scalar, just check if type is correct or
                                not */
                            if ((vx_status)VX_SUCCESS == status) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR014 */
                            {
                                if (type != mf->sc.type) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR015 */
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
                                /* status set to NULL due to preceding type check */
                                status = ownInitVirtualImage(vxCastRefAsImage(ref, NULL),
                                    mf->img.width, mf->img.height,
                                    mf->img.format);
                            }
                            break;
                        case (vx_enum)VX_TYPE_ARRAY:
                            /* status set to NULL due to preceding type check */
                            status = ownInitVirtualArray(
                                vxCastRefAsArray(ref, NULL), mf->arr.item_type,
                                mf->arr.capacity);
                            break;
                        case (vx_enum)VX_TYPE_PYRAMID:
                            /* status set to NULL due to preceding type check */
                            pmd = vxCastRefAsPyramid(ref, NULL);

                            status = vxQueryPyramid(
                                pmd, (vx_enum)VX_PYRAMID_LEVELS, &levels,
                                sizeof(levels));

                            if ((vx_status)VX_SUCCESS == status) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR016 */
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
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM004 */
                                else if (0U == mf->pmd.height)
                                {
                                    status = (vx_status)VX_ERROR_INVALID_VALUE;
                                    VX_PRINT(VX_ZONE_ERROR,"pyramid height equal to zero\n");
                                }
#endif
                                else
                                {
                                    status = ownInitVirtualPyramid(pmd,
                                        mf->pmd.width, mf->pmd.height,
                                        mf->pmd.format);
                                }
                            }
                            break;
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM007 */
                        default:
                            break;
#endif
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
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT001 */
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"graph valid rectangle callback failed at index %d\n", i);
                }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT001 */
/*LDRA_ANALYSIS*/
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

            if(node && /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR017 */
            node->kernel) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR018 */
            {
                status = ownNodeKernelInit(node);
                VX_PRINT(VX_ZONE_INFO, "kernel init for node %d, kernel %s ...\n", i, node->kernel->name);
            }

            if(status != (vx_status)VX_SUCCESS )
            {
                VX_PRINT(VX_ZONE_ERROR,"kernel init for node %d, kernel %s ... failed !!!\n", i, node->kernel->name);
                break;
            }

            if(node && /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR019 */
            node->kernel) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR020 */
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
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM031 */
            if(status_name != (vx_status)VX_SUCCESS )
            {
                VX_PRINT(VX_ZONE_ERROR,"Node kernel name deinit for node at index %d failed\n", i);
                break;
            }
#endif
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
        else if ((vxCastRefFromGraph(graph) != ref2->scope) &&
            (ref1 == ref2->scope))
        {
            is_match = (vx_bool)vx_true_e;
        }
        else if ( ((vx_enum)VX_TYPE_IMAGE==ref1->type) &&
                  ((vx_enum)VX_TYPE_IMAGE==ref2->type) )
        {
            /* status set to NULL due to preceding type check */
            vx_image image_ref1 = vxCastRefAsImage(ref1, NULL);
            vx_image image_ref2 = vxCastRefAsImage(ref2, NULL);
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
         (ownGraphIsRefMatch(graph, ref1, parent_ref_node_next) != (vx_bool)vx_false_e) || /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR021 */
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

    for (node_cur_idx = 0; (node_cur_idx < graph->num_nodes) && ((vx_status)VX_SUCCESS == status); node_cur_idx++)
    {
        node_cur = graph->nodes[node_cur_idx];
        uint32_t num_node_params = ownNodeGetNumParameters(node_cur);

#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM005 */
        if (TIVX_KERNEL_MAX_PARAMS < num_node_params)
        {
            /* HARD limit on the number of kernel parameters that can be processed */
            VX_PRINT(VX_ZONE_ERROR, "No more than TIVX_KERNEL_MAX_PARAMS parameters are allowed per kernel!");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
#endif
        for (prm_cur_idx = 0;
             (prm_cur_idx < num_node_params) &&
               ((vx_status)VX_SUCCESS == status);
             prm_cur_idx++)
        {
            prm_cur_dir = (uint32_t)ownNodeGetParameterDir(node_cur, prm_cur_idx);
            ref1 = ownNodeGetParameterRef(node_cur, prm_cur_idx);
            if( (ref1) && ((prm_cur_dir == (uint32_t)VX_OUTPUT) || (prm_cur_dir == (uint32_t)VX_BIDIRECTIONAL)) )
            {
                vx_uint32 outputs_attached = 0U;
                vx_uint32 inputs_attached = 0U;
                vx_uint32 biputs_attached = 0U;
                /* No read-only object can be modified: Special check for uniform images. */
                if ((ref1->type == (vx_enum)VX_TYPE_IMAGE) &&
                    ((vx_enum)TIVX_IMAGE_UNIFORM ==(vx_enum)((tivx_obj_desc_image_t *)ref1->obj_desc)->create_type))
                {
                    status = (vx_status)VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR,"Cannot write to uniform image at node index %d failed\n", node_cur_idx);
                }
                /* for each output, see if it matches any node input data */
                for (node_next_idx = 0; (node_next_idx < graph->num_nodes) && ((vx_status)VX_SUCCESS == status); node_next_idx++)
                {
                    node_next = graph->nodes[node_next_idx];
                    for (prm_next_idx = 0;
                         (prm_next_idx < ownNodeGetNumParameters(node_next)) &&
                           ((vx_status)VX_SUCCESS == status);
                         prm_next_idx++)
                    {
                        prm_next_dir = (uint32_t)ownNodeGetParameterDir(node_next, prm_next_idx);
                        ref2 = ownNodeGetParameterRef(node_next, prm_next_idx);
                        if (ref2 != NULL)
                        {
                            if (ownGraphCheckIsRefMatch(graph, ref1, ref2) != 0)
                            {
                                if (node_cur_idx == node_next_idx)
                                {
                                    if (prm_cur_idx != prm_next_idx)
                                    {
                                    /* We have an error: output connected to same node */
                                        status = (vx_status)VX_FAILURE;
                                        VX_PRINT(VX_ZONE_ERROR,"Output of node connected to same node at index %d failed\n", node_cur_idx);
                                    }
                                }
                                else if (prm_cur_dir == prm_next_dir)
                                {
                                    /* We have an error: two modifiers or two writers of the same edge */
                                    status = (vx_status)VX_FAILURE;
                                    VX_PRINT(VX_ZONE_ERROR,"Two modifiers or two writers of the same edge, nodes %d and %d\n", node_cur_idx, node_next_idx);
                                }
                                else if ((uint32_t)VX_INPUT == prm_next_dir)
                                {
                                    ++inputs_attached;
                                    /* add node_next as output node for current node if not already added */
                                    /* but we don't do it if this parameter is an output attached to a bidirectional */
                                    if (0U == biputs_attached)
                                    {
                                        status = ownNodeAddOutNode(node_cur, node_next);
                                        if(status == (vx_status)VX_SUCCESS)/* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR022 */
                                        {
                                            /* add node_current as input node for next node if not already added */
                                            status = ownNodeAddInNode(node_next, node_cur);
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT002 */
                                            if (status != (vx_status)VX_SUCCESS)
                                            {
                                                VX_PRINT(VX_ZONE_ERROR,"Add in node at index %d failed\n", node_cur_idx);
                                            }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT002 */
/*LDRA_ANALYSIS*/
                                        }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT003 */
                                        else
                                        {
                                            VX_PRINT(VX_ZONE_ERROR,"Add out node at index %d failed\n", node_cur_idx);
                                        }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT003 */
#endif
                                    }
                                }
                                else if ((vx_enum)VX_BIDIRECTIONAL == (vx_enum)prm_next_dir)
                                {
                                    /* Current node must be an output, proceed as if it was the only input attached */
                                    /* We must make the bidirectional parameter node the only one following the
                                       output parameter node; to do this we go through all the out nodes and replace
                                       the output parameter node by the bidirectional parameter node in the in node list */
                                    ++biputs_attached;
                                    uint32_t i;
                                    for (i = 0; (i < ownNodeGetNumOutNodes(node_cur)) && ((vx_status)VX_SUCCESS == status); ++i)
                                    {
                                        vx_node out_node = ownNodeGetNextNode(node_cur, i);
                                        if ((vx_node)NULL != out_node) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR023 */
                                        {
                                          status = ownNodeReplaceInNode(out_node, node_cur, node_next);
                                        }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM028 */
                                        else
                                        {
                                          status = (vx_status)VX_FAILURE;
                                        }
#endif
                                    }

                                    if ((vx_status)VX_SUCCESS == (vx_status)status) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR024 */
                                    {
                                        node_cur->obj_desc[0]->num_out_nodes = 0;
                                        node_next->obj_desc[0]->num_in_nodes = 0;
                                        status = ownNodeAddOutNode(node_cur, node_next);
                                        if ((vx_status)VX_SUCCESS == (vx_status)status) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR025 */
                                        {
                                            status = ownNodeAddInNode(node_next, node_cur);
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT004 */
                                            if ((vx_status)VX_SUCCESS != (vx_status)status)
                                            {
                                                VX_PRINT(VX_ZONE_ERROR, "Add in node for bidirectional at index %d failed\n", node_cur_idx);
                                            }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT004 */
/*LDRA_ANALYSIS*/
                                        }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT005 */
                                        else
                                        {
                                            VX_PRINT(VX_ZONE_ERROR, "Add out node for bidirectional at index %d failed\n", node_cur_idx);
                                        }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT005 */
#endif
                                    }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM029 */
                                    else
                                    {
                                        VX_PRINT(VX_ZONE_ERROR, "Failed to replace input node %d with input node %d\n", node_cur_idx, node_next_idx);
                                    }
#endif
                                }
                                else
                                {
                                    /* prm_cur_dir must be bidirectional and prm_next_dir must be an output; here
                                       we count the number of times that the bidirectional has an output attached. */
                                    ++outputs_attached;
                                }
                            }
                        }
                    }
                }
                if ((prm_cur_dir == (uint32_t)VX_BIDIRECTIONAL) &&
                    (ref1->is_virtual == (vx_bool)vx_true_e))
                {
                    /* A virtual bidirectional parameter must be connected to exactly one output and at least one input,
                    except if the kernel is VX_KERNEL_MOVE */
                    if (outputs_attached != 1U)
                    {
                        status = (vx_status)VX_FAILURE;
                        VX_PRINT(VX_ZONE_ERROR,"Virtual bidirectional parameter must be connected to one output at index %d failed\n", node_cur_idx);
                    }
                    else if (((vx_enum)VX_KERNEL_MOVE != node_cur->kernel->enumeration) &&
                             (inputs_attached == 0U))
                    {
                        status = (vx_status)VX_FAILURE;
                        VX_PRINT(VX_ZONE_ERROR,"Virtual bidirectional parameter must be connected to at least one input at index %d failed\n", node_cur_idx);
                    }
                    else
                    {
                        /* Do nothing, required by MISRA-C */
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

        if( node->super_node == NULL /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR026 */
#if defined(BUILD_BAM)
        || (node->is_super_node == (vx_bool)vx_true_e)
#endif
        )
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

vx_status ownGraphAllocateDataObject(vx_node node_cur, uint32_t prm_cur_idx, vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownNodeIsPrmReplicated(node_cur, prm_cur_idx) != 0)
    {
        /* if this is a replicated node, replicated parameter
         * then allocate memory for parent object
         */
        status = ownReferenceAllocMem(ref->scope);
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT006 */
        if (status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"Memory allocation for replicated parameter parent object failed\n");
        }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT006 */
/*LDRA_ANALYSIS*/
    }
    else
    if(ref->delay != NULL )
    {
        /* if this is part of delay then allocate memory for all
         * delay objects
         */
        status = ownReferenceAllocMem(vxCastRefFromDelay(ref->delay));
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
                status = ownGraphAllocateDataObject(node_cur, prm_cur_idx, ref);
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
                    vx_node node, uint32_t idx)
{
    uint32_t node_id, prm_id;
    vx_reference node_prm_ref;

    node_prm_ref = ownNodeGetParameterRef(node, idx);

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
            if(ref && /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR027 */
            ref->obj_desc) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR028 */
            {
                ref_obj_desc_id = ref->obj_desc->obj_desc_id;

                status = ownObjDescQueueEnqueue(data_ref_q->acquire_q_obj_desc_id, ref_obj_desc_id);
            }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM008 */
            else
            {
                status = (vx_status)VX_FAILURE;
            }
#endif
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM009 */
            if(status!=(vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "Unable to prime data ref queue\n");
                break;
            }
#endif
        }
    }
    for(i=0; i<graph->num_delay_data_ref_q; i++)
    {
        tivx_data_ref_queue data_ref_q;

        data_ref_q = graph->delay_data_ref_q_list[i].data_ref_queue;

        if(graph->delay_data_ref_q_list[i].node != NULL) 
        {
            ref = ownNodeGetParameterRef(graph->delay_data_ref_q_list[i].node, graph->delay_data_ref_q_list[i].index);
            if(ref && /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR029 */
            ref->obj_desc) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR030 */
            {
                ref_obj_desc_id = ref->obj_desc->obj_desc_id;

                status = ownObjDescQueueEnqueue(data_ref_q->acquire_q_obj_desc_id, ref_obj_desc_id);
            }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM010 */
            else
            {
                status = (vx_status)VX_FAILURE;
            }
#endif
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM011 */
            if(status!=(vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "Unable to prime delay data ref queue\n");
            }
#endif
        }
        else
        {
            vx_delay delay = graph->delay_data_ref_q_list[i].delay_ref;
            uint32_t delay_slot_index = graph->delay_data_ref_q_list[i].delay_slot_index;

            /* data reference queue with no node as input */
            ref = delay->refs[delay_slot_index];
            if(ref && /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR031 */
            ref->obj_desc) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR032 */
            {
                ref_obj_desc_id = ref->obj_desc->obj_desc_id;

                status = ownObjDescQueueEnqueue(data_ref_q->acquire_q_obj_desc_id, ref_obj_desc_id);
            }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM012 */
            else
            {
                status = (vx_status)VX_FAILURE;
            }
#endif
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM013 */
            if(status!=(vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "Unable to prime delay data ref queue\n");
            }
#endif
        }
    }
    return status;
}

static vx_status ownGraphCheckAndCreateDelayDataReferenceQueues(vx_graph graph,
            vx_node node,
            uint32_t idx,
            tivx_data_ref_queue data_ref_q)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_reference ref = ownNodeGetParameterRef(
                            node,
                            idx);

    if (ref != NULL) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR033 */
    {
        if((ownIsValidSpecificReference(vxCastRefFromDelay(ref->delay), (vx_enum)VX_TYPE_DELAY) != (vx_bool)vx_false_e))
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
                    idx))
                {
                    delay_data_ref_q_list[delay_slot_index] = data_ref_q;
                }
                else
                {
                    if(graph->num_delay_data_ref_q<TIVX_GRAPH_MAX_DATA_REF_QUEUE) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR034 */
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

#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM014 */
                        if(graph->delay_data_ref_q_list[graph->num_delay_data_ref_q].data_ref_queue == NULL)
                        {
                            status = (vx_status)VX_ERROR_NO_RESOURCES;
                            VX_PRINT(VX_ZONE_ERROR,"Unable to allocate data ref queue for delay \n");
                        }
#endif
                        if(status == (vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR035 */
                        {
                            delay_data_ref_q_list[delay_slot_index] = graph->delay_data_ref_q_list[graph->num_delay_data_ref_q].data_ref_queue;
                            graph->num_delay_data_ref_q++;
                        }
                    }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT007 */
                    else
                    {
                        status = (vx_status)VX_ERROR_NO_RESOURCES;
                        VX_PRINT(VX_ZONE_ERROR,"Exceed number of data reference queue list for delays \n");
                    }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT007 */
#endif

                }
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT008 */
                if(status!=(vx_status)VX_SUCCESS)
                {
                    break;
                }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT008 */
/*LDRA_ANALYSIS*/
            }
            if(status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR036 */
            {
                /* always returning success */
                (void)ownDataRefQueueLinkDelayDataRefQueues(delay_data_ref_q_list, auto_age_delay_slot, delay->count);
            }
        }
    }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM015 */
    else
    {
        vx_reference node_ref;
        node_ref = vxCastRefFromNode(node);
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR,"Graph parameter of node %s at index %d is NULL \n", node_ref->name, idx);
    }
#endif

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
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT009 */
        if(graph->data_ref_q_list[i].data_ref_queue == NULL)
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT009 */
/*LDRA_ANALYSIS*/
        /* check for and handle delay's */
        if(status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR037 */
        {
            status = ownGraphCheckAndCreateDelayDataReferenceQueues(graph,
                graph->data_ref_q_list[i].node,
                graph->data_ref_q_list[i].index,
                graph->data_ref_q_list[i].data_ref_queue
                );
        }
        if(status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR038 */
        {
            uint32_t buf_id;

            for(buf_id=0; buf_id<graph->data_ref_q_list[i].num_buf; buf_id++)
            {
                if(graph->data_ref_q_list[i].refs_list[buf_id]!=NULL) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR039 */
                {
                    /* alloc memory for references that can be enqueued in data ref queues */
                    status = ownGraphAllocateDataObject(graph->data_ref_q_list[i].node, graph->data_ref_q_list[i].index,
                            graph->data_ref_q_list[i].refs_list[buf_id]);
                }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM016 */
                else
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Invalid reference in refs_list\n");
                }
#endif
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT010 */
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Unable to allocate data object memory \n");
                    break;
                }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT010 */
/*LDRA_ANALYSIS*/
            }
        }
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT011 */
        if(status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to create data ref queue\n");
            break;
        }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT011 */
/*LDRA_ANALYSIS*/
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
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT012 */
            if(graph->parameters[i].data_ref_queue == NULL)
            {
                status = (vx_status)VX_ERROR_NO_RESOURCES;
            }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT012 */
/*LDRA_ANALYSIS*/
            /* check for and handle delay's */
            if(status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR040 */
            {
                status = ownGraphCheckAndCreateDelayDataReferenceQueues(graph,
                    graph->parameters[i].node,
                    graph->parameters[i].index,
                    graph->parameters[i].data_ref_queue
                    );
            }
            if(status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR041 */
            {
                uint32_t buf_id;

                for(buf_id=0; buf_id<graph->parameters[i].num_buf; buf_id++)
                {
                    if(graph->parameters[i].refs_list[buf_id]!=NULL) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR042 */
                    {
                        /* alloc memory for references that can be enqueued in data ref queues */
                        status = ownGraphAllocateDataObject(graph->parameters[i].node, graph->parameters[i].index,
                                graph->parameters[i].refs_list[buf_id]);
                    }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM017 */
                    else
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "Invalid reference in refs_list\n");
                    }
#endif
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT013 */
                    if(status != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate data object memory \n");
                        break;
                    }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT013 */
/*LDRA_ANALYSIS*/
                }
            }
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT014 */
            if(status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "Unable to create data ref queue\n");
                break;
            }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT014 */
/*LDRA_ANALYSIS*/
        }
    }

    return status;
}

static vx_status ownGraphUpdateImageRefAfterKernetInit(vx_image exemplar, vx_image ref)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if((exemplar != NULL) && /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR043 */
    (ref != NULL)) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR044 */
    {
        tivx_obj_desc_image_t *img_ref_obj_desc = (tivx_obj_desc_image_t *)ref->base.obj_desc;
        tivx_obj_desc_image_t *img_exemplar_obj_desc = (tivx_obj_desc_image_t *)exemplar->base.obj_desc;

        if((img_ref_obj_desc != NULL) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR045 */
            && (img_exemplar_obj_desc != NULL) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR046 */
            && (img_ref_obj_desc->base.type == (uint32_t)TIVX_OBJ_DESC_IMAGE) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR047 */
            && (img_exemplar_obj_desc->base.type == (uint32_t)TIVX_OBJ_DESC_IMAGE) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR048 */
            )
        {
            tivx_obj_desc_memcpy(&img_ref_obj_desc->valid_roi, &img_exemplar_obj_desc->valid_roi, (uint32_t)sizeof(img_exemplar_obj_desc->valid_roi));
        }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM018 */
        else
        {
            status = (vx_status)VX_FAILURE;
        }
#endif
    }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM019 */
    else
    {
        status = (vx_status)VX_FAILURE;
    }
#endif
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM020 */
    if(status!=(vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to update image meta data after kernel init\n");
    }
#endif
    return status;
}

static vx_status ownGraphUpdatePyramidRefAfterKernetInit(vx_pyramid exemplar, vx_pyramid ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_size ref_levels, exemplar_levels, i;

    tivxCheckStatus(&status, vxQueryPyramid(exemplar, (vx_enum)VX_PYRAMID_LEVELS, &exemplar_levels, sizeof(exemplar_levels)));
    tivxCheckStatus(&status, vxQueryPyramid(ref, (vx_enum)VX_PYRAMID_LEVELS, &ref_levels, sizeof(ref_levels)));

    if((ref_levels == exemplar_levels) && /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR049 */
    (status == (vx_status)VX_SUCCESS)) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR050 */
    {
        for(i=0; i<ref_levels; i++)
        {
            tivxCheckStatus(&status, ownGraphUpdateImageRefAfterKernetInit(exemplar->img[i], ref->img[i]));
        }
    }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM021 */
    if(status!=(vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to update pyramid meta data after kernel init\n");
    }
#endif
    return status;
}

static vx_status ownGraphUpdateObjArrRefAfterKernetInit(vx_object_array exemplar, vx_object_array ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_size ref_count, exemplar_count, i;

    tivxCheckStatus(&status, vxQueryObjectArray(exemplar, (vx_enum)VX_OBJECT_ARRAY_NUMITEMS, &exemplar_count, sizeof(exemplar_count)));
    tivxCheckStatus(&status, vxQueryObjectArray(ref, (vx_enum)VX_OBJECT_ARRAY_NUMITEMS, &ref_count, sizeof(ref_count)));

    if((ref_count == exemplar_count) && /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR051 */
    (status == (vx_status)VX_SUCCESS)) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR052 */
    {
        for(i=0; i<ref_count; i++)
        {
            if ((ownIsValidSpecificReference(ref->ref[i], (vx_enum)VX_TYPE_IMAGE) == (vx_bool)vx_true_e)
              && (ownIsValidSpecificReference(exemplar->ref[i], (vx_enum)VX_TYPE_IMAGE) == (vx_bool)vx_true_e))
            {
                /* status set to NULL due to preceding type check */
                tivxCheckStatus(&status, ownGraphUpdateImageRefAfterKernetInit(vxCastRefAsImage(exemplar->ref[i], NULL), vxCastRefAsImage(ref->ref[i], NULL)));
            }
            else
            if ((ownIsValidSpecificReference(ref->ref[i], (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
              && (ownIsValidSpecificReference(exemplar->ref[i], (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e))
            {
                /* status set to NULL due to preceding type check */
                tivxCheckStatus(&status, ownGraphUpdatePyramidRefAfterKernetInit(vxCastRefAsPyramid(exemplar->ref[i], NULL), vxCastRefAsPyramid(ref->ref[i], NULL)));
            }
            else
            {
                /* do nothing */
            }
        }
    }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM022 */
    if(status!=(vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to update pyramid meta data after kernel init\n");
    }
#endif
    return status;
}

static vx_status ownGraphUpdateDataRefAfterKernetInit(vx_reference exemplar, vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((ownIsValidSpecificReference(ref, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
        && (ownIsValidSpecificReference(exemplar, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e))
    {
        /* status set to NULL due to preceding type check */
        status = ownGraphUpdatePyramidRefAfterKernetInit(vxCastRefAsPyramid(exemplar, NULL), vxCastRefAsPyramid(ref, NULL));
    }
    else
    if ((ownIsValidSpecificReference(ref, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
        && (ownIsValidSpecificReference(exemplar, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e))
    {
        /* status set to NULL due to preceding type check */
        status = ownGraphUpdateObjArrRefAfterKernetInit(vxCastRefAsObjectArray(exemplar, NULL), vxCastRefAsObjectArray(ref, NULL));
    }
    else
    if ((ownIsValidSpecificReference(ref, (vx_enum)VX_TYPE_IMAGE) == (vx_bool)vx_true_e)
      && (ownIsValidSpecificReference(exemplar, (vx_enum)VX_TYPE_IMAGE) == (vx_bool)vx_true_e))
    {
        /* status set to NULL due to preceding type check */
        status = ownGraphUpdateImageRefAfterKernetInit(vxCastRefAsImage(exemplar, NULL), vxCastRefAsImage(ref, NULL));
    }
    else
    {
        /* do nothing */
    }

    /* below is to take care of replicate case */
    if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
      && (ownIsValidSpecificReference(exemplar->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e))
    {
        /* status set to NULL due to preceding type check */
        status = ownGraphUpdatePyramidRefAfterKernetInit(vxCastRefAsPyramid(exemplar->scope, NULL), vxCastRefAsPyramid(ref->scope, NULL));
    }
    else
    if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
      && (ownIsValidSpecificReference(exemplar->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e))
    {
        /* status set to NULL due to preceding type check */
        status = ownGraphUpdateObjArrRefAfterKernetInit(vxCastRefAsObjectArray(exemplar->scope, NULL), vxCastRefAsObjectArray(ref->scope, NULL));
    }
    else
    {
        /* do nothing */
    }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM023 */
    if(status!=(vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to update data ref queue data refs meta data after kernel init\n");
    }
#endif

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
                if((graph->parameters[i].refs_list[buf_id] != NULL) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR053 */
                    && (graph->parameters[i].refs_list[0] != NULL)) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR054 */
                {
                    status = ownGraphUpdateDataRefAfterKernetInit(graph->parameters[i].refs_list[0], /* exemplar */
                            graph->parameters[i].refs_list[buf_id]);
                }
            }
        }
    }
    for(i=0; i<graph->num_data_ref_q; i++)
    {
        for(buf_id=1; buf_id<graph->data_ref_q_list[i].num_buf; buf_id++)
        {
            if((graph->data_ref_q_list[i].refs_list[buf_id] != NULL) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR055 */
              && (graph->data_ref_q_list[i].refs_list[0] != NULL)) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR056 */
            {
                status = ownGraphUpdateDataRefAfterKernetInit(graph->data_ref_q_list[i].refs_list[0], /* exemplar */
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
    if(status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR057 */
    {
        status = ownGraphCreateIntermediateDataReferenceQueues(graph);
    }
    if(status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR058 */
    {
        status = ownGraphPrimeDataReferenceQueues(graph);
    }
    if(status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR059 */
    {
        ownGraphLinkDataReferenceQueues(graph);
    }
    return status;
}

/* called during graph verify,
 * this function is called for non-graph parameters that are intermediate data objects
 * within a graph
 */
static vx_status ownGraphAddDataRefQ(vx_graph graph, vx_node node, uint32_t idx)
{
    vx_bool skip_add_data_ref_q = (vx_bool)vx_false_e;
    vx_status status = (vx_status)VX_SUCCESS;
    vx_reference param_ref;

    param_ref = ownNodeGetParameterRef(node, idx);

    /* Dont make a data ref queue if below is true
     * - if node parameter is input
     * - or if this is a leaf node
     *   - Note: exception here is if it is a delay b/c the delay slot in question
     *           may not be connected to another node
     * - or no node reference specified at the node,idx
     * Here no data ref queue is required since if user really wanted to access
     * the data ref, user would have a graph parameter out of this node, idx
     */
    if((ownNodeGetParameterDir(node, idx) == (vx_enum)VX_INPUT) /* input parameter */
        || (param_ref == NULL) /* no reference specified at node,index */
        || (   (ownGraphGetNumInNodes(graph, node, idx) == 0U)
            && !((param_ref->delay != NULL) /* leaf parameter and not a delay */
            && (ownIsValidSpecificReference(vxCastRefFromDelay(param_ref->delay), (vx_enum)VX_TYPE_DELAY) != (vx_bool)vx_false_e))
           )
        )
    {
        skip_add_data_ref_q = (vx_bool)vx_true_e;
    }
    else
    {
        uint32_t i;
        /* check if there is any graph parameter with the same reference and with queueing already enabled,
        * if yes then do nothing */
        for(i = 0; i < graph->num_params; i++)
        {
            if(i < TIVX_GRAPH_MAX_PARAMS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR060 */
            {
                vx_reference gparam_ref;
                gparam_ref = ownNodeGetParameterRef(graph->parameters[i].node, graph->parameters[i].index);
                if ((param_ref == gparam_ref) && (graph->parameters[i].queue_enable == (vx_bool)vx_true_e))
                {
                    skip_add_data_ref_q = (vx_bool)vx_true_e;
                    break;
                }
            }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM024 */
            else
            {
                status = (vx_status)VX_ERROR_INVALID_VALUE;
                VX_PRINT(VX_ZONE_ERROR,"Supplied parameter has an incorrect value\n");
            }
#endif
        }
    }

    if(skip_add_data_ref_q==(vx_bool)vx_false_e)
    {
        if(graph->num_data_ref_q<TIVX_GRAPH_MAX_DATA_REF_QUEUE) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR061 */
        {
            uint32_t num_buf;
            vx_reference exemplar;

            graph->data_ref_q_list[graph->num_data_ref_q].node = node;
            graph->data_ref_q_list[graph->num_data_ref_q].index = idx;
            graph->data_ref_q_list[graph->num_data_ref_q].num_buf = 1;
            graph->data_ref_q_list[graph->num_data_ref_q].data_ref_queue = NULL;
            graph->data_ref_q_list[graph->num_data_ref_q].refs_list[0] = ownNodeGetParameterRef(node, idx);

            /* if user has requested more than 1 buf at this node, then allocate the additional references */
            num_buf = ownNodeGetParameterNumBuf(node, idx);

            if(num_buf>0U) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR062 */
            {
                uint32_t buf_id;
                vx_bool is_replicated;

                is_replicated = ownNodeIsPrmReplicated(node, idx);

                exemplar = graph->data_ref_q_list[graph->num_data_ref_q].refs_list[0];
                if(is_replicated != 0)
                {
                    exemplar = exemplar->scope;
                }
                for(buf_id=1; buf_id<num_buf; buf_id++)
                {
                    vx_reference ref;
                    ref = tivxCreateReferenceFromExemplar(graph->base.context, exemplar);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM025 */
                    if(ref==NULL)
                    {
                        status = (vx_status)VX_ERROR_NO_RESOURCES;
                        VX_PRINT(VX_ZONE_ERROR,"Unable to create references\n");
                    }
#endif
                    if(status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR063 */
                    {
                        if(is_replicated != 0)
                        {
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT015 */
                            if (ownIsValidSpecificReference(ref, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
                            {
                                /* status set to NULL due to preceding type check */
                                vx_pyramid pyramid = vxCastRefAsPyramid(ref, NULL);
                                ref = vxCastRefFromImage(pyramid->img[0]);
                            }
                            else
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT015 */
#endif
                            if (ownIsValidSpecificReference(ref, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR064 */
                            {
                                /* status set to NULL due to preceding type check */
                                vx_object_array object_array = vxCastRefAsObjectArray(ref, NULL);
                                ref = object_array->ref[0];
                            }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM026 */
                            else
                            {
                                VX_PRINT(VX_ZONE_ERROR,"Invalid reference type for replicated parameter\n");
                                status = (vx_status)VX_FAILURE;
                                ref = NULL;
                            }
#endif
                        }
                        graph->data_ref_q_list[graph->num_data_ref_q].refs_list[buf_id] = ref;
                    }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM027 */
                    if(status!=(vx_status)VX_SUCCESS)
                    {
                        break;
                    }
#endif
                }
                if(status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR065 */
                {
                    graph->data_ref_q_list[graph->num_data_ref_q].num_buf = num_buf;
                }
            }
            if(status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR066 */
            {
                graph->num_data_ref_q++;
                ownLogSetResourceUsedValue("TIVX_GRAPH_MAX_DATA_REF_QUEUE", (uint16_t)graph->num_data_ref_q);
            }
        }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT016 */
        else
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
            VX_PRINT(VX_ZONE_ERROR, "Unable to add data ref q to graph since list is full \n");
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_GRAPH_MAX_DATA_REF_QUEUE in tiovx/include/TI/tivx_config.h\n");
        }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT016 */
#endif
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
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT017 */
        if(status!=(vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"Unable to alloc obj descriptors at node for pipelining\n");
            break;
        }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT017 */
/*LDRA_ANALYSIS*/
    }

    if(status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR067 */
    {
        for(node_id=0; node_id<graph->num_nodes; node_id++)
        {
            /* update out_node_id[], in_node_id[]
             * from 0th obj desc
             */
            ownNodeLinkObjDescForPipeline(graph->nodes[node_id]);
        }
    }

    if(status==(vx_status)VX_SUCCESS) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR068 */
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
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT018 */
                    if(status!=(vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Unable to add data ref q to graph\n");
                        break;
                    }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT018 */
/*LDRA_ANALYSIS*/
                }
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT019 */
                if(status!=(vx_status)VX_SUCCESS)
                {
                    break;
                }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT019 */
/*LDRA_ANALYSIS*/
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
        if (vxGetStatus(vxCastRefFromMetaFormat(meta[i])) != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"Unable to create meta format object\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }

    if((ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e) &&
        ((vx_status)VX_SUCCESS == status))
    {
        if((vx_status)VX_SUCCESS == ownReferenceLock(&graph->base)) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR069 */
        {
            graph->verified = (vx_bool)vx_false_e;

            if(first_time_verify == (vx_bool)vx_false_e)
            {
                status = ownGraphNodeKernelDeinit(graph);
                if((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Graph Node kernel de-init failed\n");
                }
            }
            if(status == (vx_status)VX_SUCCESS)
            {
                /* Find out nodes and in nodes for each node in the graph
                * No resources are allocated in this step
                */
                status = ownGraphCalcInAndOutNodes(graph);
                if(status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Unable to calculate out nodes and in nodes for each node\n");
                }
                else
                {
                    /* Optimise out Copy and Move nodes where possible */
                    status = ownGraphProcessCopyMoveNodes(graph);
                }

                if(status == (vx_status)VX_SUCCESS)
                {
                    vx_bool has_cycle = (vx_bool)vx_false_e;

                    if((vx_status)VX_SUCCESS == ownContextLock(graph->base.context)) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_VERIFY_UBR070 */
                    {

                        /* Topological sort graph to find cycles
                        */
                        ownGraphTopologicalSort(
                                    &graph->base.context->graph_sort_context,
                                    graph->nodes,
                                    graph->num_nodes,
                                    &has_cycle);

                        (void)ownContextUnlock(graph->base.context);
                    }

                    if(has_cycle != 0)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Topological sort failed, due to cycles in graph\n");
                        status = (vx_status)VX_FAILURE;
                    }
                }

                /* Detects connection between source and sink nodes */
                if(status == (vx_status)VX_SUCCESS)
                {
                    /* always returning success */
                    (void)ownGraphDetectSourceSink(graph);
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

                /* Detect errors in graph parameter assignment */
                if ((vx_status)VX_SUCCESS == status)
                {
                    status = ownGraphValidateParameters(graph);
                }

                /* Collect multiple references into each graph parameter */
                if ((vx_status)VX_SUCCESS == status)
                {
                    status = ownGraphLinkParameters(graph);
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

                #if defined(BUILD_BAM)

                if(status == (vx_status)VX_SUCCESS)
                {
                    /* Configure graph processing to account for any super nodes
                    */
                    status = ownGraphSuperNodeConfigure(graph);
                }

                #endif

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
#ifdef LDRA_UNTESTABLE_CODE
/* LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM006 */

                    if(status != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Find and add data references failed\n");
                    }
#endif
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
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT020 */
                    if(status != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Node pipelining failed\n");
                    }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT020 */
/*LDRA_ANALYSIS*/
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
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT021 */
                    if(status != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Create data ref queues failed\n");
                    }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT021 */
/*LDRA_ANALYSIS*/
                }

                if(status == (vx_status)VX_SUCCESS)
                {
                    /* Call target kernel init for each node
                    * This results in message communication with target kernels
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
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1676- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UM030 */
                    if(status != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Unable to update data ref queue refs for graph\n");
                    }
#endif
                }

                if(status == (vx_status)VX_SUCCESS)
                {
                    /* alloc object descriptor for graph and enqueue them */
                    status = ownGraphAllocAndEnqueueObjDescForPipeline(graph);
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT022 */
                    if(status != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Unable to alloc obj desc for graph\n");
                    }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT022 */
/*LDRA_ANALYSIS*/
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
/*LDRA_NOANALYSIS*/
/* TIOVX-1808- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT023 */
                    if(status != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"If streaming is enabled, schedule mode must be normal\n");
                    }
/* END: TIOVX_CODE_COVERAGE_GRAPH_VERIFY_UTJT023 */
/*LDRA_ANALYSIS*/
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
                    if((vx_status)VX_SUCCESS != ownGraphNodeKernelDeinit(graph))
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Grade node kernel de-init failed\n");
                    }
                }

            }

            (void)ownReferenceUnlock(&graph->base);
        }
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
            if((vx_status)VX_SUCCESS != ownReleaseMetaFormat(&meta[i]))
            {
                VX_PRINT(VX_ZONE_ERROR,"Failed to release met-format object\n");
            }
        }
    }

    return status;
}
