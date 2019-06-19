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
//#include <tivx_kernel_supernode.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>

/* Added from source/include */
#include <tivx_obj_desc_priv.h>
#include <tivx_target_kernel_instance.h>

#define TIVX_BAM_MAX_NODES 10
#define TIVX_BAM_MAX_EDGES 10
#define TIVX_MAX_BUF_PARAMS 16

typedef struct
{
    tivx_bam_graph_handle graph_handle;
    tivx_target_kernel_instance target_kernel_instance[TIVX_SUPER_NODE_MAX_NODES];
    tivx_target_kernel knl[TIVX_SUPER_NODE_MAX_NODES];
    VXLIB_bufParams2D_t buf_params[TIVX_MAX_BUF_PARAMS];
    tivx_obj_desc_image_t *obj_desc_image[TIVX_MAX_BUF_PARAMS];
    int32_t buf_params_cnt;
    int32_t src_buf_params_cnt;

} tivxSupernodeParams;

static tivx_target_kernel vx_supernode_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelSupernodeProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelSupernodeCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelSupernodeDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static void tivxSupernodeFreeMem(tivxSupernodeParams *prms);

static uint8_t tivxGetNodeIndexFromNodeList(
    uint16_t node_obj_desc_id,
    uint16_t *node_obj_desc_id_list, uint8_t num_nodes);

static vx_status tivxGetNodePort(tivx_obj_desc_super_node_t *super_node, tivxSupernodeParams *prms,
    BAM_EdgeParams edge_list[], uint8_t node_index, int32_t bam_edge_cnt, int32_t i, int32_t src_dst);

static vx_status VX_CALLBACK tivxKernelSupernodeProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i, j;
    tivxSupernodeParams *prms = NULL;
    uint32_t size;
    tivx_obj_desc_super_node_t *super_node = (tivx_obj_desc_super_node_t *)obj_desc[0];

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&prms, &size);

    if ((VX_SUCCESS != status) || (NULL == prms) ||
        (sizeof(tivxSupernodeParams) != size))
    {
        status = VX_FAILURE;
    }

    if (VX_SUCCESS == status)
    {
        void *img_ptrs[TIVX_MAX_BUF_PARAMS];

        for(i=0; i < prms->buf_params_cnt; i++)
        {
            uint8_t *ptr;
            void *target_ptr;

            target_ptr = tivxMemShared2TargetPtr(
                prms->obj_desc_image[i]->mem_ptr[0].shared_ptr, prms->obj_desc_image[i]->mem_ptr[0].mem_heap_region);

            tivxSetPointerLocation(prms->obj_desc_image[i], &target_ptr, &ptr);
            img_ptrs[i] = ptr;
        }

        status = tivxBamUpdatePointers(prms->graph_handle, prms->src_buf_params_cnt, prms->buf_params_cnt-prms->src_buf_params_cnt, img_ptrs);
    }

    if (VX_SUCCESS == status)
    {
        /* Call PreProcess Callbacks */
        for(i = 0; i < super_node->num_nodes; i++)
        {
            tivx_obj_desc_node_t *node_obj_desc;
            tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS];

            node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->node_obj_desc_id[i]);

            for(j=0; j<node_obj_desc->num_params ; j++)
            {
                params[j] = tivxObjDescGet(node_obj_desc->data_id[j]);
            }

            if ((NULL != prms->knl[i]) && (NULL != prms->knl[i]->preprocess_func))
            {
                status = prms->knl[i]->preprocess_func(
                    prms->target_kernel_instance[i], params, node_obj_desc->num_params, prms->knl[i]->caller_priv_arg);
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        status  = tivxBamProcessGraph(prms->graph_handle);
    }

    if (VX_SUCCESS == status)
    {
        /* Call PostProcess Callbacks */
        for(i = 0; i < super_node->num_nodes; i++)
        {
            tivx_obj_desc_node_t *node_obj_desc;
            tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS];

            node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->node_obj_desc_id[i]);

            for(j=0; j<node_obj_desc->num_params ; j++)
            {
                params[j] = tivxObjDescGet(node_obj_desc->data_id[j]);
            }

            if ((NULL != prms->knl[i]) && (NULL != prms->knl[i]->postprocess_func))
            {
                status = prms->knl[i]->postprocess_func(
                    prms->target_kernel_instance[i], params, node_obj_desc->num_params, prms->knl[i]->caller_priv_arg);
            }
        }
    }

    if (status != VXLIB_SUCCESS)
    {
        status = VX_FAILURE;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelSupernodeCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    int32_t i, j;
    vx_status status = VX_SUCCESS;
    tivxSupernodeParams *prms = NULL;
    BAM_NodeParams node_list[TIVX_BAM_MAX_NODES];
    BAM_EdgeParams edge_list[TIVX_BAM_MAX_EDGES];
    tivx_bam_kernel_details_t kernel_details[TIVX_BAM_MAX_NODES];
    VXLIB_bufParams2D_t *pBuf_params[TIVX_MAX_BUF_PARAMS];
    int32_t bam_node_cnt = 0;
    int32_t bam_edge_cnt = 0;
    void *scratch[TIVX_SUPER_NODE_MAX_NODES];
    int32_t size[TIVX_SUPER_NODE_MAX_NODES];
    uint8_t found_indices[TIVX_SUPER_NODE_MAX_EDGES];
    uint8_t node_index;
    tivx_obj_desc_super_node_t *super_node = (tivx_obj_desc_super_node_t *)obj_desc[0];

    prms = tivxMemAlloc(sizeof(tivxSupernodeParams), TIVX_MEM_EXTERNAL);

    if (NULL != prms)
    {
        memset(prms, 0, sizeof(tivxSupernodeParams));
        memset(scratch, 0, sizeof(scratch));
        memset(size, 0, sizeof(size));

        /* ************** */
        /* FILL NODE LIST */
        /* ************** */

        node_list[bam_node_cnt].nodeIndex = bam_node_cnt;
        node_list[bam_node_cnt].kernelId = BAM_KERNELID_DMAREAD_AUTOINCREMENT;
        node_list[bam_node_cnt].kernelArgs = NULL;
        bam_node_cnt++;

        for(i = 0; i < super_node->num_nodes; i++)
        {
            tivx_obj_desc_node_t *node_obj_desc;
            tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS];
            tivx_obj_desc_kernel_name_t *kernel_name_obj_desc;
            volatile char *kernel_name = NULL;

            node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->node_obj_desc_id[i]);

            kernel_name_obj_desc = (tivx_obj_desc_kernel_name_t*)tivxObjDescGet(node_obj_desc->kernel_name_obj_desc_id);
            if(kernel_name_obj_desc!=NULL)
            {
                kernel_name = kernel_name_obj_desc->kernel_name;
            }

            for(j=0; j<node_obj_desc->num_params ; j++)
            {
                params[j] = tivxObjDescGet(node_obj_desc->data_id[j]);
            }

            prms->target_kernel_instance[i] = tivxTargetKernelInstanceAlloc(
                node_obj_desc->kernel_id, kernel_name, node_obj_desc->target_id);

            // TODO: The same table which lists the callbacks, should also list the scratch memory requirement
            size[i] = 0;
            if ( size[i] > 0 )
            {
                scratch[i] = tivxMemAlloc(size[i], TIVX_MEM_EXTERNAL);
            }

            /* Adds Bam nodes to top level node list (can be more than 1) */
            if (NULL != prms->target_kernel_instance[i])
            {
                /* copy border mode also in the target_kernel_instance */
                tivx_obj_desc_memcpy(&prms->target_kernel_instance[i]->border_mode, &node_obj_desc->border_mode, sizeof(vx_border_t));

                /* Check if the kernel is valid */
                prms->knl[i] = tivxTargetKernelInstanceGetKernel(prms->target_kernel_instance[i]);

                if ((NULL != prms->knl[i]) && (NULL != prms->knl[i]->create_in_bam_func))
                {
                    status = prms->knl[i]->create_in_bam_func(
                        prms->target_kernel_instance[i], params, node_obj_desc->num_params, prms->knl[i]->caller_priv_arg,
                        node_list, kernel_details, &bam_node_cnt, scratch[i]);
                }
                else
                {
                    tivxTargetKernelInstanceFree(&prms->target_kernel_instance[i]);
                    VX_PRINT(VX_ZONE_ERROR, "tivxKernelSupernodeCreate: Kernel does not support being part of a supernode\n");
                    status = VX_FAILURE;
                }
            }

            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxKernelSupernodeCreate: Kernel create callback failed\n");
                break;
            }

            bam_node_cnt++;
        }

        node_list[bam_node_cnt].nodeIndex = bam_node_cnt;
        node_list[bam_node_cnt].kernelId = BAM_KERNELID_DMAWRITE_AUTOINCREMENT;
        node_list[bam_node_cnt].kernelArgs = NULL;
        bam_node_cnt++;

        node_list[bam_node_cnt].nodeIndex = BAM_END_NODE_MARKER;
        node_list[bam_node_cnt].kernelId = 0;
        node_list[bam_node_cnt].kernelArgs = NULL;
        bam_node_cnt++;

        /* ************** */
        /* FILL EDGE LIST */
        /* ************** */

        if (VX_SUCCESS == status)
        {
            for(i = 0; i < super_node->num_edges; i++)
            {
                /* Find all the source edges */
                if(super_node->edge_list[i].src_node_obj_desc_id == TIVX_OBJ_DESC_INVALID)
                {
                    tivx_obj_desc_node_t *dst_node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->edge_list[i].dst_node_obj_desc_id);
                    tivx_obj_desc_t *dst_param = tivxObjDescGet(dst_node_obj_desc->data_id[super_node->edge_list[i].dst_node_prm_idx]);

                    /* BAM wrapper assumes source nodes operate on images for now */
                    if(dst_param->type == TIVX_OBJ_DESC_IMAGE)
                    {
                        tivx_obj_desc_image_t *src = (tivx_obj_desc_image_t *)dst_param;
                        int32_t found = 0;

                        for(j=0; j < prms->buf_params_cnt; j++)
                        {
                            if(super_node->edge_list[i].src_node_prm_idx == found_indices[j])
                            {
                                found = 1;
                                break;
                            }
                        }

                        if (0 == found)
                        {
                            /* Add unique source edge to parameter list */
                            prms->obj_desc_image[prms->buf_params_cnt] = src;
                            tivxInitBufParams(src, &prms->buf_params[prms->buf_params_cnt]);
                            pBuf_params[prms->buf_params_cnt] = &prms->buf_params[prms->buf_params_cnt];
                            found_indices[prms->buf_params_cnt] = super_node->edge_list[i].src_node_prm_idx;
                            prms->buf_params_cnt++;
                        }

                        edge_list[bam_edge_cnt].upStreamNode.id = 0;
                        edge_list[bam_edge_cnt].upStreamNode.port = super_node->edge_list[i].src_node_prm_idx;
                        node_index = tivxGetNodeIndexFromNodeList(super_node->edge_list[i].dst_node_obj_desc_id, super_node->node_obj_desc_id, super_node->num_nodes);

                        status = tivxGetNodePort(super_node, prms, edge_list, node_index, bam_edge_cnt, i, 1);

                        bam_edge_cnt++;
                    }
                }
            }

            prms->src_buf_params_cnt = prms->buf_params_cnt;
        }

        if (VX_SUCCESS == status)
        {
            /* Add any/all internal edges from nodes */
            for(i = 0; i < super_node->num_nodes; i++)
            {
                if (NULL != prms->knl[i]->append_internal_edges_func)
                {
                    status = prms->knl[i]->append_internal_edges_func(
                        prms->target_kernel_instance[i], edge_list, &bam_edge_cnt);
                }
            }

            for(i = 0; i < super_node->num_edges; i++)
            {
                /* Find all the internal edges */
                if((super_node->edge_list[i].src_node_obj_desc_id != TIVX_OBJ_DESC_INVALID) &&
                   (super_node->edge_list[i].dst_node_obj_desc_id != TIVX_OBJ_DESC_INVALID))
                {
                    tivx_obj_desc_node_t *src_node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->edge_list[i].src_node_obj_desc_id);
                    tivx_obj_desc_t *src_param = tivxObjDescGet(src_node_obj_desc->data_id[super_node->edge_list[i].src_node_prm_idx]);
                    tivx_obj_desc_node_t *dst_node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->edge_list[i].dst_node_obj_desc_id);
                    tivx_obj_desc_t *dst_param = tivxObjDescGet(dst_node_obj_desc->data_id[super_node->edge_list[i].dst_node_prm_idx]);

                    /* BAM wrapper assumes source nodes operate on images for now */
                    if((src_param->type == TIVX_OBJ_DESC_IMAGE) && (dst_param->type == TIVX_OBJ_DESC_IMAGE))
                    {

                        node_index = tivxGetNodeIndexFromNodeList(super_node->edge_list[i].src_node_obj_desc_id, super_node->node_obj_desc_id, super_node->num_nodes);
                        status = tivxGetNodePort(super_node, prms, edge_list, node_index, bam_edge_cnt, i, 0);

                        node_index = tivxGetNodeIndexFromNodeList(super_node->edge_list[i].dst_node_obj_desc_id, super_node->node_obj_desc_id, super_node->num_nodes);
                        status = tivxGetNodePort(super_node, prms, edge_list, node_index, bam_edge_cnt, i, 1);

                        bam_edge_cnt++;
                    }
                }
            }

            for(i = 0; i < super_node->num_edges; i++)
            {
                /* Find all the sink edges */
                if(super_node->edge_list[i].dst_node_obj_desc_id == TIVX_OBJ_DESC_INVALID)
                {
                    tivx_obj_desc_node_t *src_node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(super_node->edge_list[i].src_node_obj_desc_id);
                    tivx_obj_desc_t *src_param = tivxObjDescGet(src_node_obj_desc->data_id[super_node->edge_list[i].src_node_prm_idx]);

                    /* BAM wrapper assumes sink nodes operate on images for now */
                    if(src_param->type == TIVX_OBJ_DESC_IMAGE)
                    {
                        tivx_obj_desc_image_t *dst = (tivx_obj_desc_image_t *)src_param;
                        int32_t found = 0;

                        for(j=0; j < (prms->buf_params_cnt - prms->src_buf_params_cnt); j++)
                        {
                            if(super_node->edge_list[i].dst_node_prm_idx == found_indices[j])
                            {
                                found = 1;
                                break;
                            }
                        }
                        // TODO: take this out after testing
                        if (0 == found)
                        {
                            /* Add unique sink edge to parameter list */
                            prms->obj_desc_image[prms->buf_params_cnt] = dst;
                            tivxInitBufParams(dst, &prms->buf_params[prms->buf_params_cnt]);
                            pBuf_params[prms->buf_params_cnt] = &prms->buf_params[prms->buf_params_cnt];
                            found_indices[(prms->buf_params_cnt - prms->src_buf_params_cnt)] = super_node->edge_list[i].dst_node_prm_idx;
                            prms->buf_params_cnt++;
                        }

                        node_index = tivxGetNodeIndexFromNodeList(super_node->edge_list[i].src_node_obj_desc_id, super_node->node_obj_desc_id, super_node->num_nodes);
                        status = tivxGetNodePort(super_node, prms, edge_list, node_index, bam_edge_cnt, i, 0);

                        edge_list[bam_edge_cnt].downStreamNode.id = bam_node_cnt-2;
                        edge_list[bam_edge_cnt].downStreamNode.port = super_node->edge_list[i].dst_node_prm_idx;
                        bam_edge_cnt++;
                    }
                }
            }
            edge_list[bam_edge_cnt].upStreamNode.id     = BAM_END_NODE_MARKER;
            edge_list[bam_edge_cnt].upStreamNode.port   = 0;
            edge_list[bam_edge_cnt].downStreamNode.id   = BAM_END_NODE_MARKER;
            edge_list[bam_edge_cnt].downStreamNode.port = 0;
            bam_edge_cnt++;

            if (VX_SUCCESS == status)
            {
                status = tivxBamCreateHandleMultiNode(node_list,
                    bam_node_cnt,
                    edge_list,
                    bam_edge_cnt,
                    pBuf_params, kernel_details,
                    &prms->graph_handle);
            }
        }
    }
    else
    {
        status = VX_ERROR_NO_MEMORY;
    }

    for(i = 0; i < super_node->num_nodes; i++)
    {
        if(scratch[i] != NULL)
        {
            tivxMemFree(scratch[i], size[i], TIVX_MEM_EXTERNAL);
        }
    }

    if (VX_SUCCESS == status)
    {
        tivxSetTargetKernelInstanceContext(kernel, prms,
            sizeof(tivxSupernodeParams));
    }
    else
    {
        if (NULL != prms)
        {
            tivxSupernodeFreeMem(prms);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelSupernodeDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size, i, j;
    tivxSupernodeParams *prms = NULL;
    tivx_obj_desc_super_node_t *super_node = (tivx_obj_desc_super_node_t *)obj_desc[0];

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxSupernodeParams) == size))
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

                if ((NULL != prms->knl[i]) && (NULL != prms->knl[i]->delete_func))
                {
                    status = prms->knl[i]->delete_func(
                        prms->target_kernel_instance[i], params, node_obj_desc->num_params, prms->knl[i]->caller_priv_arg);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "tivxKernelSupernodeCreate: Kernel does not have delete function registered\n");
                    status = VX_FAILURE;
                }

                if (NULL != prms->target_kernel_instance[i])
                {
                    tivxTargetKernelInstanceFree(&prms->target_kernel_instance[i]);
                }
            }
            tivxBamDestroyHandle(prms->graph_handle);
            tivxSupernodeFreeMem(prms);
        }
    }

    return (status);
}

static void tivxSupernodeFreeMem(tivxSupernodeParams *prms)
{
    if (NULL != prms)
    {
        tivxMemFree(prms, sizeof(tivxSupernodeParams), TIVX_MEM_EXTERNAL);
    }
}

void tivxAddTargetKernelBamSupernode(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_DSP1) || (self_cpu == TIVX_CPU_ID_DSP2))
    {
        if (self_cpu == TIVX_CPU_ID_DSP1)
        {
            strncpy(target_name, TIVX_TARGET_DSP1,
                TIVX_TARGET_MAX_NAME);
        }
        else
        {
            strncpy(target_name, TIVX_TARGET_DSP2,
                TIVX_TARGET_MAX_NAME);
        }

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

static uint8_t tivxGetNodeIndexFromNodeList(uint16_t node_obj_desc_id, uint16_t *node_obj_desc_id_list, uint8_t num_nodes)
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
                                 BAM_EdgeParams edge_list[], uint8_t node_index, int32_t bam_edge_cnt, int32_t i,
                                 int32_t src_dst)
{
    vx_status status = VX_FAILURE;

    if (super_node->num_nodes > node_index)
    {
        if (NULL != prms->knl[node_index]->get_node_port_func)
        {
            if (0 == src_dst)
            {
                status = prms->knl[node_index]->get_node_port_func(
                    prms->target_kernel_instance[node_index], super_node->edge_list[i].src_node_prm_idx,
                    &edge_list[bam_edge_cnt].upStreamNode.id,
                    &edge_list[bam_edge_cnt].upStreamNode.port);
            }
            else
            {
                status = prms->knl[node_index]->get_node_port_func(
                    prms->target_kernel_instance[node_index], super_node->edge_list[i].dst_node_prm_idx,
                    &edge_list[bam_edge_cnt].downStreamNode.id,
                    &edge_list[bam_edge_cnt].downStreamNode.port);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxKernelSupernodeCreate: get_node_port_func is NULL\n");
            status = VX_FAILURE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxKernelSupernodeCreate: invalid node index\n");
        status = VX_FAILURE;
    }
    return status;
}
