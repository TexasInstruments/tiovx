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

#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_target_kernels_priv.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>
#include <vx_internal.h>

/* Added from source/include */
#include <tivx_obj_desc_priv.h>
#include <tivx_target_kernel_instance.h>

typedef struct _tivx_edge_sort_indices {
    uint16_t left_index;
    uint16_t right_index;
} tivx_edge_sort_indices;

typedef struct _tivx_edge_sort_context {
 /*! stack used while sorting */
    tivx_edge_sort_indices  stack[TIVX_BAM_MAX_EDGES];
    /*! stack top */
    int16_t  stack_top;
    /*! stack max size */
    uint16_t  stack_max_elems;

} tivx_edge_sort_context;

typedef struct
{
    tivx_bam_graph_handle graph_handle;
    tivx_target_kernel_instance target_kernel_instance[TIVX_SUPER_NODE_MAX_NODES];
    tivx_target_kernel knl[TIVX_SUPER_NODE_MAX_NODES];
    VXLIB_bufParams2D_t buf_params[TIVX_MAX_BUF_PARAMS];
    tivx_obj_desc_image_t *obj_desc_image[TIVX_MAX_BUF_PARAMS];
    int32_t obj_desc_image_planes[TIVX_MAX_BUF_PARAMS][3];
    int32_t obj_desc_image_count;
    int32_t buf_params_cnt;
    int32_t src_buf_params_cnt;

} tivxSupernodeParams;

static tivx_target_kernel vx_supernode_target_kernel = NULL;

static void tivxEdgeSortSwap(BAM_EdgeParams *edge_in_left, BAM_EdgeParams *edge_in_right);
static uint16_t tivxEdgeSortPartition(BAM_EdgeParams edge_list[], tivx_edge_sort_indices indices);
static void tivxEdgeSortQuick(BAM_EdgeParams edge_list[], tivx_edge_sort_indices indices);

static inline void tivxEdgeSortStackReset(tivx_edge_sort_context *context, uint16_t max_elems);
static inline vx_bool tivxEdgeSortStackPush(tivx_edge_sort_context *context, tivx_edge_sort_indices indices);
static inline vx_bool tivxEdgeSortStackPop(tivx_edge_sort_context *context, tivx_edge_sort_indices *indices);
static inline vx_bool tivxEdgeSortStackIsEmpty(const tivx_edge_sort_context *context);

static vx_status VX_CALLBACK tivxKernelSupernodeProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelSupernodeCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelSupernodeDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static void tivxSupernodeDestruct(tivxSupernodeParams *prms, const tivx_obj_desc_super_node_t *super_node);

static uint8_t tivxGetNodeIndexFromNodeList(
    uint16_t node_obj_desc_id,
    const uint16_t *node_obj_desc_id_list, uint8_t num_nodes);

static vx_status tivxGetNodePort(tivx_obj_desc_super_node_t *super_node, tivxSupernodeParams *prms,
    BAM_EdgeParams edge_list[], uint8_t node_index, int32_t bam_edge_cnt, int32_t i, int32_t plane, int32_t src_dst);

static vx_status VX_CALLBACK tivxKernelSupernodeProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t i, j;
    tivxSupernodeParams *prms = NULL;
    uint32_t size;
    uint32_t ptr_count = 0;
    tivx_obj_desc_super_node_t *super_node = (tivx_obj_desc_super_node_t *)obj_desc[0];

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
        (sizeof(tivxSupernodeParams) != size))
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *img_ptrs[TIVX_MAX_BUF_PARAMS];

        for(i=0; i < prms->obj_desc_image_count; i++)
        {
            uint8_t *ptr[TIVX_IMAGE_MAX_PLANES] = {NULL};
            void    *target_ptr[TIVX_IMAGE_MAX_PLANES] = {NULL};

            for (j = 0; j < (int32_t)prms->obj_desc_image[i]->planes; j++)
            {
                target_ptr[j] = tivxMemShared2TargetPtr(&prms->obj_desc_image[i]->mem_ptr[j]);
            }

            tivxSetPointerLocation(prms->obj_desc_image[i], target_ptr, (uint8_t**)&ptr);

            for (j = 0; j < (int32_t)prms->obj_desc_image[i]->planes; j++)
            {
                if (prms->obj_desc_image_planes[i][j] != 0)
                {
                    img_ptrs[ptr_count] = ptr[j];
                    ptr_count++;
                }
            }
        }

        status = (vx_status)tivxBamUpdatePointers(prms->graph_handle, (uint32_t)prms->src_buf_params_cnt, (uint32_t)prms->buf_params_cnt - (uint32_t)prms->src_buf_params_cnt, img_ptrs);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Call PreProcess Callbacks */
        for(i = 0; i < (int32_t)super_node->num_nodes; i++)
        {
            tivx_obj_desc_node_t *node_obj_desc;
            tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS];

            node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->node_obj_desc_id[i]);

            for(j=0; j < (int32_t)node_obj_desc->num_params ; j++)
            {
                params[j] = tivxObjDescGet(node_obj_desc->data_id[j]);
            }

            tivxTargetSetTimestamp(node_obj_desc, params);

            if ((NULL != prms->knl[i]) && (NULL != prms->knl[i]->preprocess_func))
            {
                status = prms->knl[i]->preprocess_func(
                    prms->target_kernel_instance[i], params, node_obj_desc->num_params, &prms->graph_handle, prms->knl[i]->caller_priv_arg);
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status  = tivxBamProcessGraph(prms->graph_handle);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Call PostProcess Callbacks */
        for(i = 0; i < (int32_t)super_node->num_nodes; i++)
        {
            tivx_obj_desc_node_t *node_obj_desc;
            tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS];

            node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->node_obj_desc_id[i]);

            for(j=0; j < (int32_t)node_obj_desc->num_params ; j++)
            {
                params[j] = tivxObjDescGet(node_obj_desc->data_id[j]);
            }

            if ((NULL != prms->knl[i]) && (NULL != prms->knl[i]->postprocess_func))
            {
                status = prms->knl[i]->postprocess_func(
                    prms->target_kernel_instance[i], params, node_obj_desc->num_params, &prms->graph_handle, prms->knl[i]->caller_priv_arg);
            }
        }
    }

    if (status != (vx_status)VXLIB_SUCCESS)
    {
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelSupernodeCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    int32_t i, j, k;
    vx_status status = (vx_status)VX_SUCCESS;
    tivxSupernodeParams *prms = NULL;
    BAM_NodeParams node_list[TIVX_BAM_MAX_NODES];
    BAM_EdgeParams edge_list[TIVX_BAM_MAX_EDGES];
    tivx_bam_kernel_details_t kernel_details[TIVX_BAM_MAX_NODES];
    VXLIB_bufParams2D_t *pBuf_params[TIVX_MAX_BUF_PARAMS];
    int32_t bam_node_cnt = 0;
    int32_t bam_edge_cnt = 0;
    int32_t ref_count = 0;
    int32_t out_ref_count = 0;
    int32_t port_count = 0;
    int32_t valid_node_port;
    void *scratch[TIVX_SUPER_NODE_MAX_NODES];
    int32_t size[TIVX_SUPER_NODE_MAX_NODES];
    uint8_t found_indices[TIVX_SUPER_NODE_MAX_EDGES];
    uint8_t node_index, src_node_index, dst_node_index;
    uint8_t one_shot_flag = 0;
    uint8_t found_edges_to_sink = 0;
    int32_t skip_port;
    tivx_obj_desc_super_node_t *super_node = (tivx_obj_desc_super_node_t *)obj_desc[0];
    tivx_edge_sort_indices indices;

    status = tivxBamInitKernelDetails(&kernel_details[0], TIVX_BAM_MAX_NODES, kernel);

    memset(size, 0, sizeof(size));
    memset(scratch, 0, sizeof(scratch));

    if (TIVX_SUPER_NODE_MAX_NODES < super_node->num_nodes)
    {
        VX_PRINT(VX_ZONE_ERROR, "num_nodes in supernode exceeds TIVX_SUPER_NODE_MAX_NODES\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        prms = tivxMemAlloc(sizeof(tivxSupernodeParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }

    if (NULL != prms)
    {
        memset(prms, 0, sizeof(tivxSupernodeParams));

        /* ************** */
        /* FILL NODE LIST */
        /* ************** */

        node_list[bam_node_cnt].nodeIndex = (vx_uint8)bam_node_cnt;
        node_list[bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_DMAREAD_AUTOINCREMENT;
        node_list[bam_node_cnt].kernelArgs = NULL;
        bam_node_cnt++;

        for(i = 0; i < (int32_t)super_node->num_nodes; i++)
        {
            tivx_obj_desc_node_t *node_obj_desc;
            tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS];
            tivx_obj_desc_kernel_name_t *kernel_name_obj_desc;
            volatile char *kernel_name = NULL;

            node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->node_obj_desc_id[i]);

            kernel_name_obj_desc = (tivx_obj_desc_kernel_name_t*)tivxObjDescGet((uint16_t)node_obj_desc->kernel_name_obj_desc_id);
            if(kernel_name_obj_desc != NULL)
            {
                kernel_name = kernel_name_obj_desc->kernel_name;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "kernel_name_obj_desc[%d] is NULL\n", i);
                status = (vx_status)VX_FAILURE;
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                for(j=0; j < (int32_t)node_obj_desc->num_params ; j++)
                {
                    params[j] = tivxObjDescGet(node_obj_desc->data_id[j]);
                }

                prms->target_kernel_instance[i] = tivxTargetKernelInstanceAlloc(
                    (int32_t)node_obj_desc->kernel_id, kernel_name, (int32_t)node_obj_desc->target_id);

                /* Adds Bam nodes to top level node list (can be more than 1) */
                if (NULL == prms->target_kernel_instance[i])
                {
                    VX_PRINT(VX_ZONE_ERROR, "target_kernel_instance[%d] is NULL, out of memory\n", i);
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                /* copy border mode also in the target_kernel_instance */
                tivx_obj_desc_memcpy(&prms->target_kernel_instance[i]->border_mode, &node_obj_desc->border_mode, sizeof(vx_border_t));

                /* Check if the kernel is valid */
                prms->knl[i] = tivxTargetKernelInstanceGetKernel(prms->target_kernel_instance[i]);

                if ((NULL != prms->knl[i]) && (NULL != prms->knl[i]->create_in_bam_func))
                {
                    size[i] = prms->knl[i]->kernel_params_size;
                    if ( size[i] > 0 )
                    {
                        scratch[i] = tivxMemAlloc((uint32_t)size[i], (vx_enum)TIVX_MEM_EXTERNAL);
                        if(NULL == scratch[i])
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate scratch for kernel[%s], size = %d\n", kernel_name, size[i]);
                            status = (vx_status)VX_ERROR_NO_RESOURCES;
                        }
                        else
                        {
                            memset(scratch[i], 0, (uint32_t)size[i]);
                        }
                    }

                    if ((vx_status)VX_SUCCESS == status)
                    {
                        status = prms->knl[i]->create_in_bam_func(
                            prms->target_kernel_instance[i], params, node_obj_desc->num_params, prms->knl[i]->caller_priv_arg,
                            node_list, kernel_details, &bam_node_cnt, scratch[i], &size[i]);

                        if((vx_status)VX_SUCCESS != status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Kernel[%s] 'create_in_bam_func' failed\n", kernel_name);
                        }

                        /* In addition to nodes added here, there are 2 nodes which will be added at end, consider it here so we only need to check in one place */
                        if(bam_node_cnt >= ((int32_t)TIVX_BAM_MAX_NODES-2))
                        {
                            VX_PRINT(VX_ZONE_ERROR, "BAM graph overflows TIVX_BAM_MAX_NODES.  May need to increase value of TIVX_BAM_MAX_NODES in kernels/include/tivx_bam_kernel_wrapper.h\n");
                            status = (vx_status)VX_FAILURE;
                        }
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Kernel[%s] does not support being part of a supernode\n", kernel_name);
                    status = (vx_status)VX_FAILURE;
                }

                if ((vx_status)VX_SUCCESS == status)
                {
                    if(kernel_details[bam_node_cnt].kernel_info.numOutputDataBlocks == 0U)
                    {
                        char str1[50];
                        /* This is a corner case, if new cases like this arise, need to update here */
                        strcpy(str1, "org.khronos.openvx.canny_edge_detector");
                        if (tivx_obj_desc_strncmp(kernel_name, str1, strlen(str1)) == 0)
                        {
                            tivx_obj_desc_node_t *src_node_obj_desc;
                            tivx_obj_desc_kernel_name_t *src_kernel_name_obj_desc;
                            volatile char *src_kernel_name = NULL;

                            one_shot_flag = 0;

                            for(j = 0; j < (int32_t)super_node->num_edges; j++)
                            {
                                if((int32_t)super_node->edge_list[j].src_node_obj_desc_id != (vx_enum)TIVX_OBJ_DESC_INVALID)
                                {
                                    src_node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->edge_list[j].src_node_obj_desc_id);
                                    src_kernel_name_obj_desc = (tivx_obj_desc_kernel_name_t*)tivxObjDescGet((uint16_t)src_node_obj_desc->kernel_name_obj_desc_id);

                                    if(src_kernel_name_obj_desc!=NULL)
                                    {
                                        src_kernel_name = src_kernel_name_obj_desc->kernel_name;
                                    }

                                    if (tivx_obj_desc_strncmp(src_kernel_name, str1, strlen(str1)) == 0)
                                    {
                                        if((int32_t)super_node->edge_list[j].dst_node_obj_desc_id != (vx_enum)TIVX_OBJ_DESC_INVALID)
                                        {
                                            VX_PRINT(VX_ZONE_ERROR, "Canny Edge Detector's output could NOT be an internal edge in the supernode\n");
                                            status = (vx_status)VX_FAILURE;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            one_shot_flag = 1;
                        }
                    }
                }
            }

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Supernode failed to create BAM node for kernel [%s]\n", kernel_name);
                break;
            }

            bam_node_cnt++;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            node_list[bam_node_cnt].nodeIndex = (uint8_t)bam_node_cnt;
            if (one_shot_flag == 0U) {
                node_list[bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_DMAWRITE_AUTOINCREMENT;
            }
            else
            {
                node_list[bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_DMAWRITE_NULL;
            }
            node_list[bam_node_cnt].kernelArgs = NULL;
            bam_node_cnt++;

            node_list[bam_node_cnt].nodeIndex = BAM_END_NODE_MARKER;
            node_list[bam_node_cnt].kernelId = 0;
            node_list[bam_node_cnt].kernelArgs = NULL;
            bam_node_cnt++;

            /* ************** */
            /* FILL EDGE LIST */
            /* ************** */

            for(i = 0; i < (int32_t)super_node->num_edges; i++)
            {
                uint8_t port_used = 0;

                for (j = 0; j < (int32_t)prms->obj_desc_image_count; j++)
                {
                    if (found_indices[j] == super_node->edge_list[i].src_node_prm_idx)
                    {
                        port_used = 1U;
                        break;
                    }
                }

                if (port_used == 0U)
                {
                    /* Find all the source edges */
                    if((int32_t)super_node->edge_list[i].src_node_obj_desc_id == (vx_enum)TIVX_OBJ_DESC_INVALID)
                    {
                        tivx_obj_desc_node_t *dst_node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->edge_list[i].dst_node_obj_desc_id);
                        tivx_obj_desc_t *dst_param = tivxObjDescGet(dst_node_obj_desc->data_id[super_node->edge_list[i].dst_node_prm_idx]);

                        /* BAM wrapper assumes source nodes operate on images for now */
                        if((int32_t)dst_param->type == (vx_enum)TIVX_OBJ_DESC_IMAGE)
                        {
                            found_indices[prms->obj_desc_image_count] = (uint8_t)super_node->edge_list[i].src_node_prm_idx;

                            tivx_obj_desc_image_t *src = (tivx_obj_desc_image_t *)dst_param;

                            for(j = i; j < (int32_t)super_node->num_edges; j++)
                            {
                                if (((int32_t)super_node->edge_list[j].src_node_obj_desc_id == (vx_enum)TIVX_OBJ_DESC_INVALID) &&
                                    (super_node->edge_list[j].src_node_prm_idx == super_node->edge_list[i].src_node_prm_idx))
                                {
                                    node_index = tivxGetNodeIndexFromNodeList(super_node->edge_list[j].dst_node_obj_desc_id, super_node->node_obj_desc_id, (uint8_t)super_node->num_nodes);

                                    skip_port = 0;

                                    for (k = 0; k < (int32_t)src->planes; k++)
                                    {
                                        status = tivxGetNodePort(super_node, prms, edge_list, node_index, bam_edge_cnt, j, k, 1);
                                        if ((vx_status)VX_SUCCESS != status)
                                        {
                                            break;
                                        }
                                        valid_node_port = (int32_t)edge_list[bam_edge_cnt].downStreamNode.port;

                                        if (valid_node_port != TIVX_IMAGE_NULL_PLANE) {
                                            prms->obj_desc_image_planes[prms->obj_desc_image_count][k] = 1;
                                        }
                                    }
                                    if ((vx_status)VX_SUCCESS != status)
                                    {
                                        break;
                                    }
                                }
                            }

                            if ((vx_status)VX_SUCCESS == status)
                            {
                                for(j = i; j < (int32_t)super_node->num_edges; j++)
                                {
                                    if (((int32_t)super_node->edge_list[j].src_node_obj_desc_id == (vx_enum)TIVX_OBJ_DESC_INVALID) &&
                                        (super_node->edge_list[j].src_node_prm_idx == super_node->edge_list[i].src_node_prm_idx))
                                    {
                                        node_index = tivxGetNodeIndexFromNodeList(super_node->edge_list[j].dst_node_obj_desc_id, super_node->node_obj_desc_id, (uint8_t)super_node->num_nodes);

                                        skip_port = 0;

                                        for (k = 0; k < (int32_t)src->planes; k++)
                                        {
                                            status = tivxGetNodePort(super_node, prms, edge_list, node_index, bam_edge_cnt, j, k, 1);
                                            if ((vx_status)VX_SUCCESS != status)
                                            {
                                                break;
                                            }

                                            valid_node_port = (int32_t)edge_list[bam_edge_cnt].downStreamNode.port;

                                            if (valid_node_port != TIVX_IMAGE_NULL_PLANE) {
                                                edge_list[bam_edge_cnt].upStreamNode.id = 0;
                                                edge_list[bam_edge_cnt].upStreamNode.port = (uint8_t)port_count + (uint8_t)k;
                                                /*filling the gaps*/
                                                if ((k == 1) &&
                                                   (prms->obj_desc_image_planes[prms->obj_desc_image_count][0] == 0))
                                                {
                                                    edge_list[bam_edge_cnt].upStreamNode.port = (uint8_t)port_count + (uint8_t)k - 1U;
                                                }

                                                if ((k == 2) &&
                                                    ((prms->obj_desc_image_planes[prms->obj_desc_image_count][0] == 0) &&
                                                    (prms->obj_desc_image_planes[prms->obj_desc_image_count][1] == 0)))
                                                {
                                                    edge_list[bam_edge_cnt].upStreamNode.port = (uint8_t)port_count + (uint8_t)k - 2U;
                                                }
                                                else if ((k == 2) &&
                                                         ((prms->obj_desc_image_planes[prms->obj_desc_image_count][0] == 1) &&
                                                         (prms->obj_desc_image_planes[prms->obj_desc_image_count][1] == 0)))
                                                {
                                                    edge_list[bam_edge_cnt].upStreamNode.port = (uint8_t)port_count + (uint8_t)k - 1U;
                                                }
                                                else if ((k == 2) &&
                                                         ((prms->obj_desc_image_planes[prms->obj_desc_image_count][0] == 0) &&
                                                         (prms->obj_desc_image_planes[prms->obj_desc_image_count][1] == 1)))
                                                {
                                                    edge_list[bam_edge_cnt].upStreamNode.port = (uint8_t)port_count + (uint8_t)k - 1U;
                                                }
                                                else if (k == 2)
                                                {
                                                    edge_list[bam_edge_cnt].upStreamNode.port = (uint8_t)port_count + (uint8_t)k;
                                                }
                                                else
                                                {
                                                    /* do nothing */
                                                }

                                                edge_list[bam_edge_cnt].downStreamNode.port = (uint8_t)valid_node_port - (uint8_t)skip_port;
                                                bam_edge_cnt++;
                                            }
                                            else
                                            {
                                                skip_port++;
                                            }
                                        }
                                        if ((vx_status)VX_SUCCESS != status)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }

                            if ((vx_status)VX_SUCCESS == status)
                            {
                                prms->obj_desc_image[prms->obj_desc_image_count] = src;
                                tivxInitBufParams(src, &prms->buf_params[ref_count]);

                                for (j = 0; j < (int32_t)src->planes; j++)
                                {
                                    if (prms->obj_desc_image_planes[prms->obj_desc_image_count][j] != 0)
                                    {
                                        if(prms->buf_params_cnt < (int32_t)TIVX_MAX_BUF_PARAMS)
                                        {
                                            pBuf_params[prms->buf_params_cnt] = &prms->buf_params[ref_count];
                                            prms->buf_params_cnt++;
                                        }
                                        else
                                        {
                                            VX_PRINT(VX_ZONE_ERROR, "BAM graph overflows TIVX_MAX_BUF_PARAMS.  May need to increase value of TIVX_MAX_BUF_PARAMS in kernels/include/tivx_bam_kernel_wrapper.h\n");
                                            status = (vx_status)VX_FAILURE;
                                            break;
                                        }
                                    }
                                    ref_count++;
                                }

                                port_count += prms->obj_desc_image_planes[prms->obj_desc_image_count][0] +
                                              prms->obj_desc_image_planes[prms->obj_desc_image_count][1] +
                                              prms->obj_desc_image_planes[prms->obj_desc_image_count][2];
                                prms->obj_desc_image_count++;
                            }

                            if ((vx_status)VX_SUCCESS != status)
                            {
                                break;
                            }
                        }
                    }
                }
            }

            prms->src_buf_params_cnt = prms->buf_params_cnt;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            /* Add any/all internal edges from nodes */
            for(i = 0; i < (int32_t)super_node->num_nodes; i++)
            {
                if (NULL != prms->knl[i]->append_internal_edges_func)
                {
                    status = prms->knl[i]->append_internal_edges_func(
                        prms->target_kernel_instance[i], edge_list, &bam_edge_cnt);
                    if ((vx_status)VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Kernel[%d] 'append_internal_edges_func' callback failed\n", i);
                    }
                    if (bam_edge_cnt >= (int32_t)TIVX_BAM_MAX_EDGES)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "When calling kernel[%d] 'append_internal_edges_func',\n", i);
                        VX_PRINT(VX_ZONE_ERROR, "BAM graph overflows TIVX_BAM_MAX_EDGES.  May need to increase value of TIVX_BAM_MAX_EDGES in kernels/include/tivx_bam_kernel_wrapper.h\n");
                        status = (vx_status)VX_FAILURE;
                    }
                    if ((vx_status)VX_SUCCESS != status)
                    {
                        break;
                    }
                }
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            for(i = 0; i < (int32_t)super_node->num_edges; i++)
            {
                /* Find all the internal edges */
                if(((int32_t)super_node->edge_list[i].src_node_obj_desc_id != (vx_enum)TIVX_OBJ_DESC_INVALID) &&
                   ((int32_t)super_node->edge_list[i].dst_node_obj_desc_id != (vx_enum)TIVX_OBJ_DESC_INVALID))
                {
                    tivx_obj_desc_node_t *src_node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->edge_list[i].src_node_obj_desc_id);
                    tivx_obj_desc_t *src_param = tivxObjDescGet(src_node_obj_desc->data_id[super_node->edge_list[i].src_node_prm_idx]);
                    tivx_obj_desc_node_t *dst_node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->edge_list[i].dst_node_obj_desc_id);
                    tivx_obj_desc_t *dst_param = tivxObjDescGet(dst_node_obj_desc->data_id[super_node->edge_list[i].dst_node_prm_idx]);

                    /* BAM wrapper assumes source nodes operate on images for now */
                    if(((int32_t)src_param->type == (vx_enum)TIVX_OBJ_DESC_IMAGE) && ((int32_t)dst_param->type == (vx_enum)TIVX_OBJ_DESC_IMAGE))
                    {
                        /* Assume that the image formats of src_param and dst_param must be the same, this would have been verified by graph_verify*/
                        tivx_obj_desc_image_t *src = (tivx_obj_desc_image_t *)src_param;
                        src_node_index = tivxGetNodeIndexFromNodeList(super_node->edge_list[i].src_node_obj_desc_id, super_node->node_obj_desc_id, (uint8_t)super_node->num_nodes);
                        dst_node_index = tivxGetNodeIndexFromNodeList(super_node->edge_list[i].dst_node_obj_desc_id, super_node->node_obj_desc_id, (uint8_t)super_node->num_nodes);

                        skip_port = 0;
                        for (j = 0; j < (int32_t)src->planes; j++)
                        {
                            vx_uint32 temp_status = (vx_uint32)status;
                            temp_status |= (vx_uint32)tivxGetNodePort(super_node, prms, edge_list, src_node_index, bam_edge_cnt, i, j, 0);
                            temp_status |= (vx_uint32)tivxGetNodePort(super_node, prms, edge_list, dst_node_index, bam_edge_cnt, i, j, 1);
                            status = (vx_status)temp_status;

                            if ((vx_status)VX_SUCCESS != status)
                            {
                                break;
                            }

                            valid_node_port = (int32_t)edge_list[bam_edge_cnt].downStreamNode.port;

                            if (valid_node_port == TIVX_IMAGE_NULL_PLANE)
                            {
                                edge_list[bam_edge_cnt].downStreamNode.id = BAM_NULL_NODE;
                                edge_list[bam_edge_cnt].downStreamNode.port = 0;
                                skip_port++;
                            }
                            else
                            {
                                edge_list[bam_edge_cnt].downStreamNode.port = (uint8_t)valid_node_port - (uint8_t)skip_port;
                            }
                            bam_edge_cnt++;
                        }
                        if ((vx_status)VX_SUCCESS != status)
                        {
                            break;
                        }
                    }
                }
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            for(i = 0; i < (int32_t)super_node->num_edges; i++)
            {
                /* Find all the sink edges */
                if((int32_t)super_node->edge_list[i].dst_node_obj_desc_id == (vx_enum)TIVX_OBJ_DESC_INVALID)
                {
                    tivx_obj_desc_node_t *src_node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->edge_list[i].src_node_obj_desc_id);
                    tivx_obj_desc_t *src_param = tivxObjDescGet(src_node_obj_desc->data_id[super_node->edge_list[i].src_node_prm_idx]);

                    /* BAM wrapper assumes sink nodes operate on images for now */
                    if((int32_t)src_param->type == (vx_enum)TIVX_OBJ_DESC_IMAGE)
                    {
                        tivx_obj_desc_image_t *dst = (tivx_obj_desc_image_t *)src_param;

                        /* Add unique sink edge to parameter list */
                        prms->obj_desc_image[prms->obj_desc_image_count] = dst;
                        tivxInitBufParams(dst, &prms->buf_params[ref_count]);

                        for (j = 0; j < (int32_t)dst->planes; j++)
                        {
                            prms->obj_desc_image_planes[prms->obj_desc_image_count][j] = 1;
                            if(prms->buf_params_cnt < (int32_t)TIVX_MAX_BUF_PARAMS)
                            {
                                pBuf_params[prms->buf_params_cnt] = &prms->buf_params[ref_count];
                                prms->buf_params_cnt++;
                                ref_count++;
                            }
                            else
                            {
                                VX_PRINT(VX_ZONE_ERROR, "BAM graph overflows TIVX_MAX_BUF_PARAMS.  May need to increase value of TIVX_MAX_BUF_PARAMS in kernels/include/tivx_bam_kernel_wrapper.h\n");
                                status = (vx_status)VX_FAILURE;
                                break;
                            }
                        }

                        if ((vx_status)VX_SUCCESS == status)
                        {
                            prms->obj_desc_image_count++;

                            node_index = tivxGetNodeIndexFromNodeList(super_node->edge_list[i].src_node_obj_desc_id, super_node->node_obj_desc_id, (uint8_t)super_node->num_nodes);

                            for (j = 0; j < (int32_t)dst->planes; j++)
                            {
                                status = tivxGetNodePort(super_node, prms, edge_list, node_index, bam_edge_cnt, i, j, 0);
                                if ((vx_status)VX_SUCCESS != status)
                                {
                                    break;
                                }
                                edge_list[bam_edge_cnt].downStreamNode.id = (uint8_t)bam_node_cnt-2U;
                                edge_list[bam_edge_cnt].downStreamNode.port = (uint8_t)out_ref_count;
                                bam_edge_cnt++;
                                out_ref_count++;
                            }

                            found_edges_to_sink = 1;
                        }

                        if ((vx_status)VX_SUCCESS != status)
                        {
                            break;
                        }
                    }
                }
            }
            if ((found_edges_to_sink & one_shot_flag) != 0U)
            {
                VX_PRINT(VX_ZONE_ERROR, "In a supernode, reduction kernels cannot be used with any other kernels which have image outputs\n");
                status = (vx_status)VX_FAILURE;
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            if (bam_edge_cnt >= (int32_t)TIVX_BAM_MAX_EDGES)
            {
                VX_PRINT(VX_ZONE_ERROR, "BAM graph overflows TIVX_BAM_MAX_EDGES.  May need to increase value of TIVX_BAM_MAX_EDGES in kernels/include/tivx_bam_kernel_wrapper.h\n");
                status = (vx_status)VX_FAILURE;
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            edge_list[bam_edge_cnt].upStreamNode.id     = BAM_END_NODE_MARKER;
            edge_list[bam_edge_cnt].upStreamNode.port   = 0;
            edge_list[bam_edge_cnt].downStreamNode.id   = BAM_END_NODE_MARKER;
            edge_list[bam_edge_cnt].downStreamNode.port = 0;
            bam_edge_cnt++;

            /*logic to eliminate unnecessary null nodes*/
            for (i = 0; i < bam_edge_cnt; i++)
            {
                if (edge_list[i].downStreamNode.id == BAM_NULL_NODE)
                {
                    for (j = 0; j < bam_edge_cnt; j++)
                    {
                        if (i != j)
                        {
                            if ((edge_list[j].upStreamNode.id == edge_list[i].upStreamNode.id) &&
                                (edge_list[j].upStreamNode.port == edge_list[i].upStreamNode.port) &&
                                (edge_list[j].downStreamNode.id == BAM_NULL_NODE))
                            {
                                edge_list[j].upStreamNode.id     = 255U;
                                edge_list[j].upStreamNode.port   = 255U;
                                edge_list[j].downStreamNode.id   = 255U;
                            }
                        }
                    }
                }
            }

            for (i = 0; i < bam_edge_cnt; i++)
            {
                if (edge_list[i].downStreamNode.id == BAM_NULL_NODE)
                {
                    for (j = 0; j < bam_edge_cnt; j++)
                    {
                        if (i != j)
                        {
                            if ((edge_list[j].upStreamNode.id == edge_list[i].upStreamNode.id) &&
                                (edge_list[j].upStreamNode.port == edge_list[i].upStreamNode.port) &&
                                !((edge_list[j].upStreamNode.id == 255U) && (edge_list[j].upStreamNode.port == 255U)))
                            {
                                edge_list[i].downStreamNode.id   = edge_list[j].downStreamNode.id;
                                edge_list[i].downStreamNode.port = edge_list[j].downStreamNode.port;
                                edge_list[j].upStreamNode.id     = 255U;
                                edge_list[j].upStreamNode.port   = 255U;
                                edge_list[j].downStreamNode.id   = 255U;
                            }
                        }
                    }
                }
            }

            for (i = 0; i < bam_edge_cnt; i++)
            {
                if ((edge_list[i].upStreamNode.id == 255U) && (edge_list[i].upStreamNode.port == 255U))
                {
                    for (j = i; j < bam_edge_cnt; j++)
                    {
                        if ((j+1) < bam_edge_cnt)
                        {
                            edge_list[j].upStreamNode.id     = edge_list[j+1].upStreamNode.id ;
                            edge_list[j].upStreamNode.port   = edge_list[j+1].upStreamNode.port;
                            edge_list[j].downStreamNode.id   = edge_list[j+1].downStreamNode.id;
                            edge_list[j].downStreamNode.port = edge_list[j+1].downStreamNode.port;
                        }
                    }
                    i--;
                    bam_edge_cnt--;
                }
            }

            indices.left_index = 0;
            indices.right_index = (uint16_t)bam_edge_cnt-2U;
            tivxEdgeSortQuick(edge_list, indices);

            status = tivxBamCreateHandleMultiNode(node_list,
                (uint32_t)bam_node_cnt,
                edge_list,
                (uint32_t)bam_edge_cnt,
                pBuf_params, kernel_details,
                &prms->graph_handle);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "prms is NULL\n");
        status = (vx_status)VX_ERROR_NO_MEMORY;
    }

    for(i = 0; i < (int32_t)super_node->num_nodes; i++)
    {
        if(scratch[i] != NULL)
        {
            tivxMemFree(scratch[i], (uint32_t)size[i], (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxSetTargetKernelInstanceContext(kernel, prms,
            sizeof(tivxSupernodeParams));
    }
    else
    {
        tivxSupernodeDestruct(prms, super_node);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelSupernodeDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t size;
    tivxSupernodeParams *prms = NULL;
    tivx_obj_desc_super_node_t *super_node = (tivx_obj_desc_super_node_t *)obj_desc[0];

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxSupernodeParams) == size))
        {
            tivxSupernodeDestruct(prms, super_node);
        }
    }

    return (status);
}

static void tivxSupernodeDestruct(tivxSupernodeParams *prms, const tivx_obj_desc_super_node_t *super_node)
{
    uint32_t i, j;
    vx_status status = (vx_status)VX_SUCCESS;

    if ((NULL != super_node) && (NULL != prms))
    {
        for(i = 0; i < super_node->num_nodes; i++)
        {
            tivx_obj_desc_node_t *node_obj_desc;
            tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS];

            node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->node_obj_desc_id[i]);

            for(j=0; j<node_obj_desc->num_params ; j++)
            {
                params[j] = tivxObjDescGet(node_obj_desc->data_id[j]);
            }

            if (NULL != prms->knl[i])
            {
                if (NULL != prms->knl[i]->delete_func)
                {
                    status = prms->knl[i]->delete_func(
                        prms->target_kernel_instance[i], params, node_obj_desc->num_params, prms->knl[i]->caller_priv_arg);
                    if( (vx_status)VX_SUCCESS != status )
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Kernel[%d] delete function failed\n", i);
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Kernel[%d] does not have delete function registered\n", i);
                    status = (vx_status)VX_FAILURE;
                }
            }

            if (NULL != prms->target_kernel_instance[i])
            {
                tivxTargetKernelInstanceFree(&prms->target_kernel_instance[i]);
            }
        }
    }

    if (NULL != prms)
    {
        if (NULL != prms->graph_handle)
        {
            tivxBamDestroyHandle(prms->graph_handle);
        }

        tivxMemFree(prms, sizeof(tivxSupernodeParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

void tivxAddTargetKernelBamSupernode(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_supernode_target_kernel = tivxAddTargetKernel(
            TIVX_KERNEL_SUPERNODE,
            target_name,
            tivxKernelSupernodeProcess,
            tivxKernelSupernodeCreate,
            tivxKernelSupernodeDelete,
            NULL,
            NULL);
    }
}

void tivxRemoveTargetKernelBamSupernode(void)
{
    tivxRemoveTargetKernel(vx_supernode_target_kernel);
}

static uint8_t tivxGetNodeIndexFromNodeList(uint16_t node_obj_desc_id, const uint16_t *node_obj_desc_id_list, uint8_t num_nodes)
{
    uint8_t i;
    uint8_t node_index = 0;

    for(i = 0; i < num_nodes; i++)
    {
        if(node_obj_desc_id == node_obj_desc_id_list[i])
        {
            node_index = i;
            break;
        }
    }

    return node_index;
}

static vx_status tivxGetNodePort(tivx_obj_desc_super_node_t *super_node, tivxSupernodeParams *prms,
                                 BAM_EdgeParams edge_list[], uint8_t node_index, int32_t bam_edge_cnt,
                                 int32_t i, int32_t plane, int32_t src_dst)
{
    vx_status status = (vx_status)VX_FAILURE;

    if (super_node->num_nodes > node_index)
    {
        if (bam_edge_cnt < (int32_t)TIVX_BAM_MAX_EDGES)
        {
            if (NULL != prms->knl[node_index]->get_node_port_func)
            {
                if (0 == src_dst)
                {
                    status = prms->knl[node_index]->get_node_port_func(
                        prms->target_kernel_instance[node_index], super_node->edge_list[i].src_node_prm_idx, plane,
                        &edge_list[bam_edge_cnt].upStreamNode.id,
                        &edge_list[bam_edge_cnt].upStreamNode.port);
                }
                else
                {
                    status = prms->knl[node_index]->get_node_port_func(
                        prms->target_kernel_instance[node_index], super_node->edge_list[i].dst_node_prm_idx, plane,
                        &edge_list[bam_edge_cnt].downStreamNode.id,
                        &edge_list[bam_edge_cnt].downStreamNode.port);
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "get_node_port_func is NULL\n");
                status = (vx_status)VX_FAILURE;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "BAM graph overflows TIVX_BAM_MAX_EDGES.  May need to increase value of TIVX_BAM_MAX_EDGES in kernels/include/tivx_bam_kernel_wrapper.h\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid node index\n");
        status = (vx_status)VX_FAILURE;
    }
    return status;
}

static void tivxEdgeSortSwap(BAM_EdgeParams *edge_in_left, BAM_EdgeParams *edge_in_right)
{
    BAM_EdgeParams temp_edge = *edge_in_left;
    *edge_in_left = *edge_in_right;
    *edge_in_right = temp_edge;
}

static uint16_t tivxEdgeSortPartition(BAM_EdgeParams edge_list[], tivx_edge_sort_indices indices)
{
    uint16_t j;
    uint32_t index_val;
    uint32_t pivot_val = ((uint32_t)edge_list[indices.right_index].upStreamNode.id*1000U)  +
                         ((uint32_t)edge_list[indices.right_index].upStreamNode.port*100U) +
                         ((uint32_t)edge_list[indices.right_index].downStreamNode.id*10U)  +
                         (uint32_t)edge_list[indices.right_index].downStreamNode.port;

    uint16_t i = indices.left_index;

    for (j = indices.left_index; j < indices.right_index; j++) {
        index_val = ((uint32_t)edge_list[j].upStreamNode.id*1000U)  +
                    ((uint32_t)edge_list[j].upStreamNode.port*100U) +
                    ((uint32_t)edge_list[j].downStreamNode.id*10U)  +
                    (uint32_t)edge_list[j].downStreamNode.port;
        if (index_val <= pivot_val) {
            tivxEdgeSortSwap(&edge_list[i], &edge_list[j]);
            i++;
        }
    }
    tivxEdgeSortSwap(&edge_list[i], &edge_list[indices.right_index]);
    return i;
}

static void tivxEdgeSortQuick(BAM_EdgeParams edge_list[], tivx_edge_sort_indices indices)
{
    tivx_edge_sort_context *context;
    tivx_edge_sort_indices left_array_indices, right_array_indices;

    context = tivxMemAlloc(sizeof(tivx_edge_sort_context), (vx_enum)TIVX_MEM_EXTERNAL);
    memset(context, 0, sizeof(tivx_edge_sort_context));

    tivxEdgeSortStackReset(context, (uint16_t)indices.right_index + 1U);

    tivxEdgeSortStackPush(context, indices);

    while (tivxEdgeSortStackIsEmpty(context) == 0) {

        tivxEdgeSortStackPop(context, &indices);

        uint16_t pivot = tivxEdgeSortPartition(edge_list, indices);

        if ((pivot - 1U) > indices.left_index) {
            left_array_indices.left_index = indices.left_index;
            left_array_indices.right_index = (uint16_t)pivot - 1U;
            tivxEdgeSortStackPush(context, left_array_indices);
        }

        if ((pivot + 1U) < indices.right_index) {
            right_array_indices.left_index = (uint16_t)pivot + 1U;
            right_array_indices.right_index = indices.right_index;
            tivxEdgeSortStackPush(context, right_array_indices);
        }
    }

    if (NULL != context)
    {
        tivxMemFree(context, sizeof(tivx_edge_sort_context), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

static inline void tivxEdgeSortStackReset(tivx_edge_sort_context *context, uint16_t max_elems)
{
    context->stack_top = 0;
    context->stack_max_elems = max_elems;
}

static inline vx_bool tivxEdgeSortStackPush(tivx_edge_sort_context *context, tivx_edge_sort_indices indices)
{
    vx_bool status = (vx_bool)vx_false_e;

    if(context->stack_top < (int32_t)context->stack_max_elems)
    {
        context->stack[context->stack_top] = indices;
        context->stack_top++;
        status = (vx_bool)vx_true_e;
    }
    else
    {
        /* for this to happen, edge_list size should have already passed TIVX_BAM_MAX_EDGES*/
        VX_PRINT(VX_ZONE_ERROR, "Stack Overflow at Supernode's Edge Sort Stack, increase TIVX_BAM_MAX_EDGES\n");
    }
    return status;
}

static inline vx_bool tivxEdgeSortStackPop(tivx_edge_sort_context *context, tivx_edge_sort_indices *indices)
{
    vx_bool status = (vx_bool)vx_false_e;

    if ((context->stack_top > 0) && (context->stack_top < (int32_t)TIVX_BAM_MAX_EDGES))
    {
        *indices = context->stack[context->stack_top-1];
        context->stack_top--;
        status = (vx_bool)vx_true_e;
    }
    else
    {
        if (context->stack_top <= 0) {
            VX_PRINT(VX_ZONE_ERROR, "Trying to pop while empty from Supernode's Edge Sort Stack\n");
        }
        else if (context->stack_top >= (int32_t)TIVX_BAM_MAX_EDGES)
        {
            VX_PRINT(VX_ZONE_ERROR, "Stack Overflow at Supernode's Edge Sort Stack, increase TIVX_BAM_MAX_EDGES\n");
        }
        else
        {
            /* do nothing */
        }
    }
    return status;
}

static inline vx_bool tivxEdgeSortStackIsEmpty(const tivx_edge_sort_context *context)
{
    vx_bool returnVal = (vx_bool)vx_true_e;

    if (context->stack_top > 0)
    {
        returnVal = (vx_bool)vx_false_e;
    }

    return returnVal;
}
