/*
*
* Copyright (c) 2017-2019 Texas Instruments Incorporated
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



#include "tivx_bam_kernel_wrapper.h"
#include <TI/tivx_target_kernel.h>
#include "bam_dma_one_shot_node.h"
#include "edma_utils_autoincrement_v2.h"
#include "edma_utils.h"

#define USE_ALGFRAMEWORK_2_8_0_0

#ifdef HOST_EMULATION
extern CSL_EdmaccRegs dummyEDMAreg;
extern uint32_t edmaBase[1];
#endif

#define SOURCE_NODE  0U
#define COMPUTE_NODE 1U
#define SINK_NODE    2U

static BAM_InternalMemParams gIntMemParams;

/**
 *  PRIVATE STRUCTS
 */

/* Contains arguments to configure DMA and compute nodes */
typedef struct _tivx_bam_graph_args_single
{
    EDMA_UTILS_autoIncrement_initParam_v2  dma_read_autoinc_args;
    EDMA_UTILS_autoIncrement_initParam_v2  dma_write_autoinc_args;
    BAM_DMA_OneShot_Args                  dma_read_oneshot_args;
    BAM_DMA_OneShot_Args                  dma_write_oneshot_args;
    VXLIB_bufParams2D_t                 **buf_params;
    BAM_KernelInfo                       *kernel_info;
    void                                   *compute_kernel_args;

} tivx_bam_graph_args_single_t;

typedef struct _tivx_data_block_params
{
    VXLIB_bufParams2D_t block_params;
    int32_t upstream_node;
    uint32_t upstream_data_block_index;
    int32_t set_flag;
    uint32_t block_width_reduction;
    uint32_t block_height_reduction;
    uint32_t opt_x;
    float xScale;
    float yScale;
    uint32_t total_block_width_reduction;
    uint32_t total_block_height_reduction;
    uint32_t total_opt_x;
}tivx_data_block_params_t;

typedef struct _tivx_edge_params
{
    uint32_t             data_block_index;

}tivx_edge_params_t;

/* Contains arguments to configure DMA and compute nodes */
typedef struct _tivx_bam_graph_args_multi
{
    EDMA_UTILS_autoIncrement_initParam_v2  dma_read_autoinc_args;
    EDMA_UTILS_autoIncrement_initParam_v2  dma_write_autoinc_args;
    BAM_DMA_OneShot_Args                  dma_read_oneshot_args;
    BAM_DMA_OneShot_Args                  dma_write_oneshot_args;
    tivx_bam_kernel_details_t          *kernel_details;
    VXLIB_bufParams2D_t                 **buf_params;
    void                                  *compute_kernel_args[TIVX_BAM_MAX_NODES];
    BAM_NodeParams *node_list;
    BAM_EdgeParams *edge_list;
    tivx_edge_params_t *edge_params;
    tivx_data_block_params_t *data_block_list;
    uint32_t num_data_blocks;
    int32_t  num_nodes;
    int32_t  num_edges;

} tivx_bam_graph_args_multi_t;


/* Contains context information for an instance of a BAM graph */
typedef struct _tivx_bam_graph_handle
{
    BAM_GraphHandle     bam_graph_handle;
    BAM_GraphMem        bam_graph_ptrs;
    BAM_GraphMemReq     bam_graph_sizes;
    uint8_t             sink_node;
    uint8_t             one_shot_flag;
    uint8_t             multi_flag;

}tivx_bam_graph_handle_t;


/**
 *  PRIVATE FUNCTIONS
 */

static inline void assignDMAautoIncrementParams(
    EDMA_UTILS_autoIncrement_transferProperties * param,
    uint16_t    roiWidth,
    uint16_t    roiHeight,
    uint16_t    blkWidth,
    uint16_t    blkHeight,
    uint16_t    extBlkIncrementX,
    uint16_t    extBlkIncrementY,
    uint16_t    intBlkIncrementX,
    uint16_t    intBlkIncrementY,
    uint32_t    roiOffset,
    uint32_t    blockOffset,
    uint8_t     *extMemPtr,
    uint16_t    extMemPtrStride,
    uint8_t     *interMemPtr,
    uint16_t    interMemPtrStride,
    uint8_t     dmaQueNo
);

static int32_t tivxBam_initKernelsArgsSingle(void *args, BAM_BlockDimParams *blockDimParams);

static int32_t tivxBam_initKernelsArgsMulti(void *args, BAM_BlockDimParams *blockDimParams);


static int32_t getNodeIndexFromKernelId(BAM_NodeParams node_list[],
                                      int32_t num_nodes,
                                      BAM_KernelId kernelId,
                                      uint8_t *node_index);

static int32_t getNumDownstreamNodes(BAM_EdgeParams edge_list[],
                                 int32_t num_edges,
                                 uint8_t source_node_id,
                                 uint8_t *num_outputs,
                                 uint8_t *num_unique_ports);

static int32_t getNumUpstreamNodes(BAM_EdgeParams edge_list[],
                                int32_t num_edges,
                                uint8_t dest_node_id,
                                uint8_t *num_inputs);

static int32_t getDataBlockInput(BAM_EdgeParams edge_list[],
                                   tivx_edge_params_t edge_params[],
                                   int32_t num_edges,
                                   uint8_t node_idx,
                                   uint8_t port_num);

static int32_t getDataBlockOutput(BAM_EdgeParams edge_list[],
                                   tivx_edge_params_t edge_params[],
                                   int32_t num_edges,
                                   uint8_t node_idx,
                                   uint8_t port_num);

static void tivxBamFreeContextPtrs(tivx_bam_graph_handle *graph_handle);

static inline void assignDMAautoIncrementParams(
    EDMA_UTILS_autoIncrement_transferProperties * param,
    uint16_t    roiWidth,
    uint16_t    roiHeight,
    uint16_t    blkWidth,
    uint16_t    blkHeight,
    uint16_t    extBlkIncrementX,
    uint16_t    extBlkIncrementY,
    uint16_t    intBlkIncrementX,
    uint16_t    intBlkIncrementY,
    uint32_t    roiOffset,
    uint32_t    blockOffset,
    uint8_t     *extMemPtr,
    uint16_t    extMemPtrStride,
    uint8_t     *interMemPtr,
    uint16_t    interMemPtrStride,
    uint8_t     dmaQueNo
)
{
    param->roiWidth             = roiWidth;
    param->roiHeight            = roiHeight;
    param->blkWidth             = blkWidth;
    param->blkHeight            = blkHeight;
    param->extBlkIncrementX     = extBlkIncrementX;
    param->extBlkIncrementY     = extBlkIncrementY;
    param->intBlkIncrementX     = intBlkIncrementX;
    param->intBlkIncrementY     = intBlkIncrementY;
    param->roiOffset            = roiOffset;
    param->blkOffset            = blockOffset;
    param->extMemPtr            = extMemPtr;
    param->extMemPtrStride      = extMemPtrStride ;
    param->interMemPtr          = interMemPtr ;
    param->interMemPtrStride    = interMemPtrStride;
    param->dmaQueNo             = dmaQueNo;
}

/*-------------------------------------------------------------------------*/
/* Function to initialize kernel arguments                                 */
/*-------------------------------------------------------------------------*/
/* MISRA.PPARAM.NEEDS.CONST
 * MISRAC_WAIVER:
 * BAM_BlockDimParams not const due to BAM requirements
 */
static int32_t tivxBam_initKernelsArgsSingle(void *args, BAM_BlockDimParams *blockDimParams)
{
    int32_t status = BAM_S_SUCCESS;
    int32_t i, j;
    float xScale, yScale;

    tivx_bam_graph_args_single_t                 *graph_args          = (tivx_bam_graph_args_single_t*)args;
    EDMA_UTILS_autoIncrement_initParam_v2  *dma_read_autoinc_args  = &(graph_args->dma_read_autoinc_args);
    EDMA_UTILS_autoIncrement_initParam_v2  *dma_write_autoinc_args = &(graph_args->dma_write_autoinc_args);
    BAM_DMA_OneShot_Args                  *dma_write_oneshot_args = &(graph_args->dma_write_oneshot_args);

    BAM_KernelInfo                       *kernel_info = graph_args->kernel_info;
    VXLIB_bufParams2D_t                 **buf_params = graph_args->buf_params;

    /* Assumes that the first part of every computeKernel in VXLIB is an array of VXLIB_bufParams2D_t */
    VXLIB_bufParams2D_t                      *compute_kernel_args = graph_args->compute_kernel_args;

    uint32_t num_bytes;

    uint16_t num_blks_horz = (uint16_t)(((buf_params[0]->dim_x-1) / blockDimParams->blockWidth) + 1);
    uint16_t num_blks_vert = (uint16_t)(((buf_params[0]->dim_y-1) / blockDimParams->blockHeight) + 1);

    uint16_t optimize_x = 0;
    uint16_t width_reduction = 0;
    uint16_t height_reduction = 0;
    uint32_t dmaQue = 0U;

    if(kernel_info->nodeType == BAM_NODE_COMPUTE_NEIGHBORHOOD_OP) {
        width_reduction = (uint16_t)((kernel_info->kernelExtraInfo.metaInfo >> 16) - 1);
        height_reduction = (uint16_t)((kernel_info->kernelExtraInfo.metaInfo & 0xFFFFU) - 1U);

        /* Some kernels are optimized if input width == output width.  With this enabled, we want to
         * increase the stride of the output buffer to match the input buffer, and make this output
         * width equal to the stride for the kernel processing */
        if(kernel_info->kernelExtraInfo.optimizationInfo == 1) {
            optimize_x = (uint16_t)((kernel_info->kernelExtraInfo.metaInfo >> 16) - 1);
        }
    }

    /* Configure dma_read_autoinc_args for SOURCE_NODE */
    dma_read_autoinc_args->initParams.numInTransfers   = kernel_info->numInputDataBlocks;
    dma_read_autoinc_args->initParams.numOutTransfers  = 0;
    dma_read_autoinc_args->initParams.transferType     = EDMA_UTILS_TRANSFER_IN;
    dma_read_autoinc_args->pingPongOffset = gIntMemParams.dataIoMemSize / 2;

    for(i=0; i<kernel_info->numInputDataBlocks; i++)
    {
        xScale = kernel_info->kernelExtraInfo.horzSamplingFactor[i];
        yScale = kernel_info->kernelExtraInfo.vertSamplingFactor[i];

        num_bytes = (uint32_t)VXLIB_sizeof(buf_params[i]->data_type);

        compute_kernel_args[i].data_type = buf_params[i]->data_type;
        compute_kernel_args[i].dim_x     = blockDimParams->blockWidth*xScale;
        compute_kernel_args[i].dim_y     = blockDimParams->blockHeight*yScale;
        compute_kernel_args[i].stride_y  = (int32_t)(compute_kernel_args[i].dim_x*num_bytes);

        assignDMAautoIncrementParams(&dma_read_autoinc_args->initParams.transferProp[i],
            buf_params[i]->dim_x*num_bytes,/* roiWidth */
            buf_params[i]->dim_y,/* roiHeight */
            (blockDimParams->blockWidth*xScale)*num_bytes,/*blkWidth */
            blockDimParams->blockHeight*yScale,/*blkHeight*/
            ((blockDimParams->blockWidth-width_reduction)*xScale)*num_bytes,/* extBlkIncrementX */
            (blockDimParams->blockHeight-height_reduction)*yScale,/* extBlkIncrementY */
            0U,/* intBlkIncrementX */
            0U,/* intBlkIncrementY */
            0U,/* roiOffset */
            0U,/* blkOffset */
            NULL,/* extMemPtr : This will come during process call */
            buf_params[i]->stride_y,/* extMemPtrStride */
            NULL,/* DMA node will be populating this field */
            compute_kernel_args[i].stride_y,/* interMemPtrStride */
            dmaQue /* dmaQueNo */
            );
        dmaQue = dmaQue ^ 1;
    }
    j = i;

    if(kernel_info->numOutputDataBlocks == 0)
    {
        /* Configure dma_write_oneshot_args for SINK_NODE */
        dma_write_oneshot_args->numOutTransfers        = 0;
        dma_write_oneshot_args->transferType           = EDMA_UTILS_TRANSFER_OUT;
        dma_write_oneshot_args->numTotalBlocksInFrame  = num_blks_horz * num_blks_vert;
        dma_write_oneshot_args->triggerBlockId         = dma_write_oneshot_args->numTotalBlocksInFrame;
    }
    else
    {
        /* Configure dma_write_autoinc_args for SINK_NODE */
        dma_write_autoinc_args->initParams.numInTransfers   = 0;
        dma_write_autoinc_args->initParams.numOutTransfers  = kernel_info->numOutputDataBlocks;
        dma_write_autoinc_args->initParams.transferType     = EDMA_UTILS_TRANSFER_OUT;
        dma_write_autoinc_args->pingPongOffset = gIntMemParams.dataIoMemSize / 2;

        for(i=0; i<kernel_info->numOutputDataBlocks; i++)
        {
            uint16_t block_width_out, block_height_out;
            xScale = kernel_info->kernelExtraInfo.horzSamplingFactor[j];
            yScale = kernel_info->kernelExtraInfo.vertSamplingFactor[j];

            num_bytes = (uint32_t)VXLIB_sizeof(buf_params[j]->data_type);

            block_width_out = (blockDimParams->blockWidth-width_reduction)*xScale;
            block_height_out = (blockDimParams->blockHeight-height_reduction)*yScale;

            compute_kernel_args[j].data_type = buf_params[j]->data_type;
            compute_kernel_args[j].dim_x     = block_width_out + optimize_x;
            compute_kernel_args[j].dim_y     = block_height_out;
            compute_kernel_args[j].stride_y  = (int32_t)(compute_kernel_args[j].dim_x*num_bytes);

            assignDMAautoIncrementParams(&dma_write_autoinc_args->initParams.transferProp[i],
                buf_params[j]->dim_x*num_bytes,/* roiWidth */
                buf_params[j]->dim_y,/* roiHeight */
                block_width_out*num_bytes,/*blkWidth */
                block_height_out,/*blkHeight*/
                block_width_out*num_bytes,/* extBlkIncrementX */
                compute_kernel_args[j].dim_y,/* extBlkIncrementY */
                0,/* intBlkIncrementX */
                0,/* intBlkIncrementY */
                0,/* roiOffset */
                0,/* blkOffset */
                NULL,/* extMemPtr : This will come during process call */
                buf_params[j]->stride_y,/* extMemPtrStride */
                NULL,/* DMA node will be populating this field */
                compute_kernel_args[j].stride_y,/* interMemPtrStride */
                dmaQue /* dmaQueNo */
                );
            dmaQue = dmaQue ^ 1;

            j++;
        }
    }

    return (status);
}

static int32_t getNodeIndexFromKernelId(BAM_NodeParams node_list[],
                                      int32_t num_nodes,
                                      BAM_KernelId kernelId,
                                      uint8_t *node_index)
{
    int32_t i;
    int32_t found = 0;
    for(i = 0; i < num_nodes; i++)
    {
        if(node_list[i].kernelId == kernelId)
        {
            *node_index = node_list[i].nodeIndex;
            found = 1;
            break;
        }
    }

    return found;
}

static int32_t getNumDownstreamNodes(BAM_EdgeParams edge_list[],
                                 int32_t num_edges,
                                 uint8_t source_node_id,
                                 uint8_t *num_outputs,
                                 uint8_t *num_unique_ports)
{
    int32_t i, j;
    int32_t found = 0;
    int32_t port_found = 0;
    int32_t n_outputs = 0;
    int32_t n_unique_ports = 0;

    int32_t found_ports[TIVX_BAM_MAX_EDGES];

    for(i = 0; i < num_edges; i++)
    {
        if(edge_list[i].upStreamNode.id == source_node_id)
        {
            found = 1;
            n_outputs++;
            port_found = 0;

            for(j=0; j < n_unique_ports;j++)
            {

                if(edge_list[i].upStreamNode.port == found_ports[j])
                {
                    port_found = 1;
                    break;
                }
            }

            if(0 == port_found)
            {
                found_ports[n_unique_ports] = edge_list[i].upStreamNode.port;
                n_unique_ports++;
            }
        }
    }

    if(NULL != num_outputs)
    {
        *num_outputs = n_outputs;
    }

    if(NULL != num_unique_ports)
    {
        *num_unique_ports = n_unique_ports;
    }

    return found;
}

static int32_t getNumUpstreamNodes(BAM_EdgeParams edge_list[],
                                int32_t num_edges,
                                uint8_t dest_node_id,
                                uint8_t *num_inputs)
{
    int32_t i;
    int32_t found = 0;
    int32_t n_inputs = 0;
    for(i = 0; i < num_edges; i++)
    {
        if(edge_list[i].downStreamNode.id == dest_node_id)
        {
            found = 1;
            n_inputs++;
        }
    }

    *num_inputs = n_inputs;

    return found;
}

static int32_t getDataBlockInput(BAM_EdgeParams edge_list[],
                                   tivx_edge_params_t edge_params[],
                                   int32_t num_edges,
                                   uint8_t node_idx,
                                   uint8_t port_num)
{
    int32_t i;
    for(i = 0; i < num_edges; i++)
    {
        if((edge_list[i].downStreamNode.id == node_idx) &&
           (edge_list[i].downStreamNode.port == port_num) )
        {
            return edge_params[i].data_block_index;
        }
    }

    return 0;
}

static int32_t getDataBlockOutput(BAM_EdgeParams edge_list[],
                                   tivx_edge_params_t edge_params[],
                                   int32_t num_edges,
                                   uint8_t node_idx,
                                   uint8_t port_num)
{
    int32_t i;
    for(i = 0; i < num_edges; i++)
    {
        if((edge_list[i].upStreamNode.id == node_idx) &&
           (edge_list[i].upStreamNode.port == port_num) )
        {
            return edge_params[i].data_block_index;
        }
    }

    return 0;
}


/*-------------------------------------------------------------------------*/
/* Function to initialize kernel arguments                                 */
/*-------------------------------------------------------------------------*/
/* MISRA.PPARAM.NEEDS.CONST
 * MISRAC_WAIVER:
 * BAM_BlockDimParams not const due to BAM requirements
 */
static int32_t tivxBam_initKernelsArgsMulti(void *args, BAM_BlockDimParams *blockDimParams)
{
    int32_t status = BAM_S_SUCCESS;
    int32_t i, j = 0, k = 0;
    uint8_t node_index;
    float xScale, yScale;

    tivx_bam_graph_args_multi_t                 *graph_args          = (tivx_bam_graph_args_multi_t*)args;
    EDMA_UTILS_autoIncrement_initParam_v2  *dma_read_autoinc_args  = &(graph_args->dma_read_autoinc_args);
    EDMA_UTILS_autoIncrement_initParam_v2  *dma_write_autoinc_args = &(graph_args->dma_write_autoinc_args);
    BAM_DMA_OneShot_Args                  *dma_write_oneshot_args = &(graph_args->dma_write_oneshot_args);

    tivx_bam_kernel_details_t          *kernel_details = graph_args->kernel_details;
    VXLIB_bufParams2D_t                 **buf_params = graph_args->buf_params;
    tivx_data_block_params_t            *data_blocks = graph_args->data_block_list;

    uint32_t num_bytes;
    uint16_t optimize_x = 0;
    uint16_t in_block_width  = blockDimParams->blockWidth;
    uint16_t in_block_height = blockDimParams->blockHeight;
    uint16_t out_block_width  = in_block_width;
    uint16_t out_block_height = in_block_height;
    uint32_t dmaQue = 0U;
    uint8_t num_graph_outputs;

    /* TIOVX-186:
     * - This loop assumes that each node in the graph is sequential with respect propogating the block size reductions
     * - If a graph is defined which this assumption is not true, then this may cause unexpected behaviors.
     * - Additional logic would need to be put in place to traverse the graph topology more accuratly to account for
     *   topologies which are not strictly sequential.
     *
     *   Issue fixed, need more testing but on the DMA callbacks, they still require same block with and height.
     *
     * - Furthermore, this loop assumes that the output block width and height does not go below 1.  If it does, then
     *   this may cause unexpected behaviors.
     * - Additional logic would need to be put in place to handle this case (either return some error for the user
     *   to break the graph up, or break the graph up internally are a couple of options)
     */

    if(getNumUpstreamNodes(graph_args->edge_list, graph_args->num_edges, graph_args->num_nodes-1, &num_graph_outputs) != 0)
    {
        int32_t block_index;
        uint32_t block_width_reduction = 0;
        uint32_t block_height_reduction = 0;
        uint32_t opt_x = 0;

        for(i=0; i<num_graph_outputs; i++)
        {
            block_index = getDataBlockInput(graph_args->edge_list, graph_args->edge_params,
                                   graph_args->num_edges, graph_args->num_nodes-1, i);

            if (data_blocks[block_index].total_block_width_reduction > block_width_reduction)
            {
                block_width_reduction = data_blocks[block_index].total_block_width_reduction;
            }
            if (data_blocks[block_index].total_block_height_reduction > block_height_reduction)
            {
                block_height_reduction = data_blocks[block_index].total_block_height_reduction;
            }
            if (data_blocks[block_index].total_opt_x > opt_x)
            {
                opt_x = data_blocks[block_index].total_opt_x;
            }
        }
        out_block_width -= block_width_reduction;
        out_block_height -= block_height_reduction;
        optimize_x += opt_x;
    }

    /* Configure for SOURCE_NODE */

    if(getNodeIndexFromKernelId(graph_args->node_list, graph_args->num_nodes, BAM_KERNELID_DMAREAD_AUTOINCREMENT, &node_index) != 0)
    {
        uint8_t num_transfers;
        if(getNumDownstreamNodes(graph_args->edge_list, graph_args->num_edges, node_index, NULL, &num_transfers) != 0)
        {
            int32_t block_index;
            dma_read_autoinc_args->initParams.numInTransfers   = num_transfers;
            dma_read_autoinc_args->initParams.transferType     = EDMA_UTILS_TRANSFER_IN;
            dma_read_autoinc_args->pingPongOffset = gIntMemParams.dataIoMemSize / 2;

            for(i=0; i<num_transfers; i++)
            {
                num_bytes = (uint32_t)VXLIB_sizeof(buf_params[i]->data_type);

                block_index = getDataBlockOutput(graph_args->edge_list, graph_args->edge_params, graph_args->num_edges, node_index, i);

                xScale = data_blocks[block_index].xScale;
                yScale = data_blocks[block_index].yScale;

                data_blocks[block_index].block_params.data_type = buf_params[i]->data_type;
                data_blocks[block_index].block_params.dim_x     = blockDimParams->blockWidth*xScale;
                data_blocks[block_index].block_params.dim_y     = blockDimParams->blockHeight*yScale;
                data_blocks[block_index].block_params.stride_y  = data_blocks[block_index].block_params.dim_x*num_bytes;
                data_blocks[block_index].set_flag = 1;

                assignDMAautoIncrementParams(&dma_read_autoinc_args->initParams.transferProp[i],
                    buf_params[i]->dim_x*num_bytes,/* roiWidth */
                    buf_params[i]->dim_y,/* roiHeight */
                    in_block_width*xScale*num_bytes,/*blkWidth */
                    in_block_height*yScale,/*blkHeight*/
                    out_block_width*xScale*num_bytes,/* extBlkIncrementX */
                    out_block_height*yScale,/* extBlkIncrementY */
                    0U,/* intBlkIncrementX */
                    0U,/* intBlkIncrementY */
                    0U,/* roiOffset */
                    0U,/* blkOffset */
                    NULL,/* extMemPtr : This will come during process call */
                    buf_params[i]->stride_y,/* extMemPtrStride */
                    NULL,/* DMA node will be populating this field */
                    blockDimParams->blockWidth*xScale*num_bytes,/* interMemPtrStride */
                    dmaQue /* dmaQueNo */
                    );
                dmaQue = dmaQue ^ 1;

            }
            k = i;
        }
    }
    else if(getNodeIndexFromKernelId(graph_args->node_list, graph_args->num_nodes, BAM_KERNELID_DMAREAD_NULL, &node_index) != 0)
    {
        /* TIOVX-186:
         * - This case is where a full-frame DMA read is needed for each frame before any block processing is done; perhaps an input
         *   histogram, or lookup table.
         * - Currently not supported for current release
         */
    }
    else
    {
        /* Do nothing.  There is no source DMA.  Graph inputs are perhaps using cache for source. */
    }

    /* Set parameters for each node */
    for(i=1; i < graph_args->num_nodes-1; i++)
    {
        /* Assumes that the first part of every computeKernel in VXLIB is an array of VXLIB_bufParams2D_t */
        VXLIB_bufParams2D_t  *compute_kernel_args = graph_args->compute_kernel_args[i];
        uint8_t exactFlag = 0;
#if 0 /* Probably will use this ... not yet */
        uint8_t optFlag = 0;
#endif

        int32_t num_inputs = kernel_details[i].kernel_info.numInputDataBlocks;
        int32_t num_outputs = kernel_details[i].kernel_info.numOutputDataBlocks;

#if 0 /* Probably will use this ... not yet */
        if(kernel_details[i].kernel_info.kernelExtraInfo.optimizationInfo == 1) {
            optFlag = 1;
        }
#endif

        if((kernel_details[i].kernel_info.kernelExtraInfo.constraintInfo & (1U<<31)) != 0) {
            exactFlag = 1;
        }

        for(j=0; j<num_inputs; j++)
        {
            int32_t block_index = getDataBlockInput(graph_args->edge_list, graph_args->edge_params,
                                   graph_args->num_edges, i, j);

            num_bytes = (uint32_t)VXLIB_sizeof(data_blocks[block_index].block_params.data_type);

            if(data_blocks[block_index].upstream_node == 0)
            {
                memcpy(&compute_kernel_args[j], &data_blocks[block_index].block_params, sizeof(VXLIB_bufParams2D_t));
            }
            else
            {
                xScale = data_blocks[block_index].xScale;
                yScale = data_blocks[block_index].yScale;

                compute_kernel_args[j].data_type = data_blocks[block_index].block_params.data_type;
                if (exactFlag)
                {
                    compute_kernel_args[j].dim_x     = (in_block_width - (data_blocks[block_index].total_block_width_reduction))*xScale;
                }
                else
                {
                    compute_kernel_args[j].dim_x     = (in_block_width - (data_blocks[block_index].total_block_width_reduction -
                                                                          data_blocks[block_index].total_opt_x)) * xScale;
                }
                compute_kernel_args[j].dim_y     = (in_block_height - data_blocks[block_index].total_block_height_reduction)*yScale;
                /*compute_kernel_args[j].stride_y  = compute_kernel_args[j].dim_x * num_bytes;*/
                compute_kernel_args[j].stride_y  = data_blocks[block_index].block_params.stride_y;
            }
        }

        for(j=0; j<num_outputs; j++)
        {
            int32_t block_index = getDataBlockOutput(graph_args->edge_list, graph_args->edge_params,
                                   graph_args->num_edges, i, j);

            num_bytes = (uint32_t)VXLIB_sizeof(data_blocks[block_index].block_params.data_type);

            xScale = data_blocks[block_index].xScale;
            yScale = data_blocks[block_index].yScale;

            compute_kernel_args[j+num_inputs].data_type = data_blocks[block_index].block_params.data_type;
            if (exactFlag)
            {
                compute_kernel_args[j+num_inputs].dim_x     = (in_block_width - (data_blocks[block_index].total_block_width_reduction))*xScale;
            }
            else
            {
                compute_kernel_args[j+num_inputs].dim_x     = (in_block_width - (data_blocks[block_index].total_block_width_reduction -
                                                                                 data_blocks[block_index].total_opt_x)) * xScale;
            }
            compute_kernel_args[j+num_inputs].dim_y     = (in_block_height - data_blocks[block_index].total_block_height_reduction)*yScale;
            compute_kernel_args[j+num_inputs].stride_y  = compute_kernel_args[j+num_inputs].dim_x * num_bytes;
            data_blocks[block_index].block_params.stride_y = (int32_t)(compute_kernel_args[j+num_inputs].stride_y);
        }
    }

    /* Configure for SINK_NODE */

    if(getNodeIndexFromKernelId(graph_args->node_list, graph_args->num_nodes, BAM_KERNELID_DMAWRITE_AUTOINCREMENT, &node_index) != 0)
    {
        uint8_t num_transfers;
        if(getNumUpstreamNodes(graph_args->edge_list, graph_args->num_edges, node_index, &num_transfers) != 0)
        {
            int32_t block_index;
            dma_write_autoinc_args->initParams.numOutTransfers  = num_transfers;
            dma_write_autoinc_args->initParams.transferType     = EDMA_UTILS_TRANSFER_OUT;
            dma_write_autoinc_args->pingPongOffset = gIntMemParams.dataIoMemSize / 2;

            for(i=0; i<num_transfers; i++)
            {
                num_bytes = (uint32_t)VXLIB_sizeof(buf_params[k]->data_type);

                block_index = getDataBlockInput(graph_args->edge_list, graph_args->edge_params, graph_args->num_edges, node_index, i);

                xScale = data_blocks[block_index].xScale;
                yScale = data_blocks[block_index].yScale;
/*
                if (block_index == 10 || block_index == 11) {
                    out_block_width = 58;
                    out_block_height = 42;
                    optimize_x = 6;
                }
                else {
                    out_block_width = 60;
                    out_block_height = 44;
                    optimize_x = 4;
                }
*/
                assignDMAautoIncrementParams(&dma_write_autoinc_args->initParams.transferProp[i],
                    buf_params[k]->dim_x*num_bytes,/* roiWidth */
                    buf_params[k]->dim_y,/* roiHeight */
                    out_block_width*xScale*num_bytes,/*blkWidth */
                    out_block_height*yScale,/*blkHeight*/
                    out_block_width*xScale*num_bytes,/* extBlkIncrementX */
                    out_block_height*yScale,/* extBlkIncrementY */
                    0,/* intBlkIncrementX */
                    0,/* intBlkIncrementY */
                    0,/* roiOffset */
                    0,/* blkOffset */
                    NULL,/* extMemPtr : This will come during process call */
                    buf_params[k]->stride_y,/* extMemPtrStride */
                    NULL,/* DMA node will be populating this field */
                    (int32_t)(((uint32_t)(out_block_width*xScale + optimize_x))*num_bytes),/* interMemPtrStride */
                    dmaQue /* dmaQueNo */
                    );
                dmaQue = dmaQue ^ 1;
                k++;
            }
        }
    }
    else if(getNodeIndexFromKernelId(graph_args->node_list, graph_args->num_nodes, BAM_KERNELID_DMAWRITE_NULL, &node_index) != 0)
    {
        uint16_t numBlksHorz = (uint16_t)(((buf_params[0]->dim_x-1) / blockDimParams->blockWidth) + 1);
        uint16_t numBlksVert = (uint16_t)(((buf_params[0]->dim_y-1) / blockDimParams->blockHeight) + 1);

        uint8_t num_transfers;
        getNumUpstreamNodes(graph_args->edge_list, graph_args->num_edges, node_index, &num_transfers);
        /* Configure dma_write_oneshot_args for SINK_NODE */
        /* If there's an one shot image output, num_transfers > 0*/
        dma_write_oneshot_args->numOutTransfers        = num_transfers;
        dma_write_oneshot_args->transferType           = EDMA_UTILS_TRANSFER_OUT;
        dma_write_oneshot_args->numTotalBlocksInFrame  = numBlksHorz * numBlksVert;
        dma_write_oneshot_args->triggerBlockId         = dma_write_oneshot_args->numTotalBlocksInFrame - 1;

        for(i=0; i<num_transfers; i++)
        {
            num_bytes = (uint32_t)VXLIB_sizeof(buf_params[k]->data_type);

            dma_write_oneshot_args->transferProp[i].blkWidth = buf_params[k]->dim_x*num_bytes;
            dma_write_oneshot_args->transferProp[i].blkHeight = buf_params[k]->dim_y;
            dma_write_oneshot_args->transferProp[i].extMemPtr = 0;
            dma_write_oneshot_args->transferProp[i].interMemPtr = 0;
            dma_write_oneshot_args->transferProp[i].extMemPtrStride = buf_params[k]->dim_x*num_bytes;
            dma_write_oneshot_args->transferProp[i].interMemPtrStride = buf_params[k]->dim_x*num_bytes;

            k++;
        }
    }
    else
    {
        /* Do nothing.  There is no destination DMA.  Graph outputs are perhaps using cache for destination. */
    }

    return (status);
}


static void tivxBamFreeContextPtrs(tivx_bam_graph_handle *graph_handle)
{
    tivx_bam_graph_handle_t *p_handle = (tivx_bam_graph_handle_t *)graph_handle;

    if(p_handle->bam_graph_ptrs.graphcontext != 0)
    {
        tivxMemFree(p_handle->bam_graph_ptrs.graphcontext,
                    p_handle->bam_graph_sizes.graphcontextSize,
                    (vx_enum)TIVX_MEM_EXTERNAL);
        p_handle->bam_graph_ptrs.graphcontext = NULL;
    }

    if(p_handle->bam_graph_ptrs.graphScratch != 0)
    {
        tivxMemFree(p_handle->bam_graph_ptrs.graphScratch,
                    p_handle->bam_graph_sizes.graphScratchSize,
                    (vx_enum)TIVX_MEM_EXTERNAL);
        p_handle->bam_graph_ptrs.graphScratch = NULL;
    }

    if(p_handle->bam_graph_ptrs.graphObj != 0)
    {
        tivxMemFree(p_handle->bam_graph_ptrs.graphObj,
                    p_handle->bam_graph_sizes.graphObjSize,
                    (vx_enum)TIVX_MEM_EXTERNAL);
        p_handle->bam_graph_ptrs.graphObj = NULL;
    }
}


/**
 * PUBLIC FUNCTIONS
 */

vx_status tivxBamMemInit(void *ibuf_mem, uint32_t ibuf_size,
                          void *wbuf_mem, uint32_t wbuf_size)
{
    vx_status status_v = (vx_status)VX_SUCCESS;

    gIntMemParams.dataIoMem              = ibuf_mem;
    gIntMemParams.scratchOrConstMem      = wbuf_mem;
    gIntMemParams.dataIoMemSize          = ibuf_size;
    gIntMemParams.scratchOrConstMemSize  = wbuf_size;

    return status_v;
}

vx_status tivxBamUpdatePointers(tivx_bam_graph_handle graph_handle,
                               uint32_t num_inputs,
                               uint32_t num_outputs,
                               void  *ptrs[])

{
    EDMA_UTILS_autoIncrement_updateParams  dma_update_params;
    BAM_DMA_OneShot_CtlArgs oneshotParams;

    vx_status status_v = (vx_status)VX_SUCCESS;
    int32_t status = BAM_S_SUCCESS;
    uint32_t i, j;

    tivx_bam_graph_handle_t *p_handle = (tivx_bam_graph_handle_t *)graph_handle;

    dma_update_params.transferType = EDMA_UTILS_TRANSFER_IN;
    dma_update_params.updateMask= EDMA_UTILS_AUTOINCREMENT_UPDATE_MASK_EXTMEMPTR;

    for( i = 0; i < num_inputs; i++)
    {
        dma_update_params.updateParams[i].extMemPtr = ptrs[i];
    }
    j=i;

    status= BAM_controlNode(p_handle->bam_graph_handle, SOURCE_NODE, &dma_update_params);

    if (status!= BAM_S_SUCCESS) {
        /* CHECK_MISRA("-14.4")  -> Disable rule 14.4  */
        /* GOTO is used at error check to jump to end of function, to exit.   */
        goto Exit;
        /* RESET_MISRA("14.4")  -> Reset rule 14.4  */
    }

    if(1 == p_handle->one_shot_flag)
    {
        oneshotParams.numTransfers = num_outputs;

        for( i = 0; i < num_outputs; i++)
        {
            oneshotParams.extMemPtr[i] = ptrs[j];
            oneshotParams.extMemPtrStride[i] = 0; /* Function expects for this to update */
            j++;
        }

        status= BAM_controlNode(p_handle->bam_graph_handle, p_handle->sink_node, &oneshotParams);
    }
    else
    {
        dma_update_params.transferType = EDMA_UTILS_TRANSFER_OUT;

        for( i = 0; i < num_outputs; i++)
        {
            dma_update_params.updateParams[i].extMemPtr = ptrs[j];
            j++;
        }

        status= BAM_controlNode(p_handle->bam_graph_handle, p_handle->sink_node, &dma_update_params);
    }

 Exit:
    if(BAM_S_SUCCESS != status)
    {
        status_v = (vx_status)VX_FAILURE;
    }
    return status_v;
}

vx_status tivxBamControlNode(tivx_bam_graph_handle graph_handle,
                             uint32_t node_id,
                             uint32_t cmd,
                             void  *payload)
{
    vx_status status_v = (vx_status)VX_SUCCESS;
    BAM_Status status_b = BAM_S_SUCCESS;
    BAM_KernelCommonControlArgs packet;
    tivx_bam_graph_handle_t *p_handle = (tivx_bam_graph_handle_t *)graph_handle;
    uint32_t node = node_id;

    packet.cmdId = cmd;
    packet.payload = payload;

    if(p_handle->multi_flag == 0)
    {
        node = COMPUTE_NODE;
    }

    status_b = BAM_controlNode(p_handle->bam_graph_handle, node, &packet);

    if(BAM_S_SUCCESS != status_b)
    {
        status_v = (vx_status)VX_FAILURE;
    }

    return status_v;
}

vx_status tivxBamProcessGraph(tivx_bam_graph_handle graph_handle)
{
    vx_status status_v = (vx_status)VX_SUCCESS;
    BAM_Status status_b = BAM_S_SUCCESS;
    BAM_InArgs in_args;
    BAM_OutArgs out_args;
    BAM_ProcessHints hints;

    tivx_bam_graph_handle_t *p_handle = (tivx_bam_graph_handle_t *)graph_handle;

    in_args.size = sizeof(BAM_InArgs);
    in_args.sliceIndex = 0;
    out_args.size = sizeof(BAM_OutArgs);

    /* Specify which processing schedule is the best, unsupported for now */
    hints.priority= BAM_COMPUTE_FIRST;

    status_b  = BAM_process(p_handle->bam_graph_handle, &in_args, &out_args, &hints);

    if(BAM_S_SUCCESS != status_b)
    {
        status_v = (vx_status)VX_FAILURE;
    }

    return status_v;
}

/* MISRA.PPARAM.NEEDS.CONST
 * MISRAC_WAIVER:
 * compute_kernel_params not const due to BAM requirements
 */
vx_status tivxBamCreateHandleSingleNode(BAM_TI_KernelID kernel_id,
                                        VXLIB_bufParams2D_t *buf_params[],
                                        tivx_bam_kernel_details_t *kernel_details,
                                        tivx_bam_graph_handle *graph_handle)
{
    vx_status status_v = (vx_status)VX_SUCCESS;
    BAM_Status status_b = BAM_S_SUCCESS;
    BAM_CreateGraphParams graph_create_params;
    tivx_bam_graph_args_single_t graph_args;
    tivx_bam_graph_handle_t *p_graph_handle = NULL;
    BAM_GraphMem *p_graph_ptrs = NULL;
    BAM_GraphMemReq *p_graph_sizes = NULL;
    int32_t i, j, skip_port;
    int32_t one_shot_flag;
    int16_t block_width, block_height;

#ifdef HOST_EMULATION
    edmaBase[0] = (uint32_t)(&dummyEDMAreg);
#endif

    /* Initialized everything but the compute kernel ID */
    BAM_NodeParams node_list[] = { \
        {SOURCE_NODE, BAM_KERNELID_DMAREAD_AUTOINCREMENT, NULL}, \
        {COMPUTE_NODE, 0, NULL}, \
        {SINK_NODE, BAM_KERNELID_DMAWRITE_AUTOINCREMENT, NULL}, \
        {BAM_END_NODE_MARKER,   0,                          NULL},\
    };

    BAM_EdgeParams edge_list[TIVX_BAM_MAX_EDGES];

    if((NULL == buf_params) || (NULL == graph_handle) || (NULL == kernel_details))
    {
        status_v = (vx_status)VX_FAILURE;
    }

    if( ((kernel_details->kernel_info.numInputDataBlocks +
          (kernel_details->kernel_info.numOutputDataBlocks == 0) ? 1 : kernel_details->kernel_info.numOutputDataBlocks) + 1U) > TIVX_BAM_MAX_EDGES)
    {
        VX_PRINT(VX_ZONE_ERROR, "BAM graph overflows TIVX_BAM_MAX_EDGES.  May need to increase value of TIVX_BAM_MAX_EDGES in kernels/include/tivx_bam_kernel_wrapper.h\n");
        status_v = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        /* Initialize to NULL in case there are any failures */
        *graph_handle = NULL;

        /* For now keep separate ... these could potentially be an array of 2 of same structure */
        graph_args.kernel_info      = &kernel_details->kernel_info;
        graph_args.buf_params       = buf_params;
        graph_args.compute_kernel_args = tivxMemAlloc(kernel_details->kernel_info.kernelArgSize, (vx_enum)TIVX_MEM_EXTERNAL);

        if(NULL == graph_args.compute_kernel_args)
        {
            status_v = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        if(NULL != kernel_details->compute_kernel_params)
        {
            VXLIB_bufParams2D_t *startPtr = (VXLIB_bufParams2D_t*)graph_args.compute_kernel_args;
            int32_t offset = kernel_details->kernel_info.numInputDataBlocks + kernel_details->kernel_info.numOutputDataBlocks;
            int32_t paramsSize = kernel_details->kernel_info.kernelArgSize - (offset*sizeof(VXLIB_bufParams2D_t));

            memcpy(&startPtr[offset], kernel_details->compute_kernel_params, (size_t)paramsSize);
        }

        /* Finish initializing node_list */

        node_list[COMPUTE_NODE].kernelId = kernel_id;

        node_list[SOURCE_NODE].kernelArgs = (void *)&graph_args.dma_read_autoinc_args;
        node_list[COMPUTE_NODE].kernelArgs = graph_args.compute_kernel_args;

        if(kernel_details->kernel_info.numOutputDataBlocks == 0)
        {
            one_shot_flag = 1;
            node_list[SINK_NODE].kernelId = BAM_KERNELID_DMAWRITE_NULL;
            node_list[SINK_NODE].kernelArgs = (void *)&graph_args.dma_write_oneshot_args;
        }
        else
        {
            one_shot_flag = 0;
            node_list[SINK_NODE].kernelArgs = (void *)&graph_args.dma_write_autoinc_args;
        }

        /* Initialize edge_list */

        for(i = 0; i < kernel_details->kernel_info.numInputDataBlocks; i++)
        {
            edge_list[i].upStreamNode.id     = SOURCE_NODE;
            edge_list[i].upStreamNode.port   = i;
            edge_list[i].downStreamNode.id   = COMPUTE_NODE;
            edge_list[i].downStreamNode.port = i;
        }
        j = i;
        skip_port = 0;
        if(kernel_details->kernel_info.numOutputDataBlocks == 0)
        {
            edge_list[j].upStreamNode.id     = COMPUTE_NODE;
            edge_list[j].upStreamNode.port   = 0;
            edge_list[j].downStreamNode.id   = BAM_NULL_NODE;
            edge_list[j].downStreamNode.port = 0;
            j++;
        }
        else
        {
            for(i = 0; i < kernel_details->kernel_info.numOutputDataBlocks; i++)
            {
                edge_list[j].upStreamNode.id     = COMPUTE_NODE;
                edge_list[j].upStreamNode.port   = i;

                if( buf_params[j] != NULL )
                {
                    edge_list[j].downStreamNode.id   = SINK_NODE;
                    edge_list[j].downStreamNode.port = i-skip_port;
                }
                else
                {
                    edge_list[j].downStreamNode.id   = BAM_NULL_NODE;
                    edge_list[j].downStreamNode.port = 0;
                    skip_port++;
                }
                j++;
            }
        }
        edge_list[j].upStreamNode.id     = BAM_END_NODE_MARKER;
        edge_list[j].upStreamNode.port   = 0;
        edge_list[j].downStreamNode.id   = BAM_END_NODE_MARKER;
        edge_list[j].downStreamNode.port = 0;

        /* Allocate memory for context handle */
        p_graph_handle = tivxMemAlloc(sizeof(tivx_bam_graph_handle_t), (vx_enum)TIVX_MEM_EXTERNAL);

        if(NULL == p_graph_handle)
        {
            status_v = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        p_graph_handle->one_shot_flag = one_shot_flag;
        p_graph_handle->multi_flag = 0;
        p_graph_handle->sink_node = SINK_NODE;

        p_graph_ptrs = &p_graph_handle->bam_graph_ptrs;
        p_graph_sizes = &p_graph_handle->bam_graph_sizes;

        status_b = BAM_initKernelDB(&gBAM_TI_kernelDBdef);

        if(BAM_S_SUCCESS != status_b)
        {
            status_v = (vx_status)VX_FAILURE;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        p_graph_sizes->graphObjSize     = 10000;
        p_graph_sizes->graphScratchSize = 10000;
        p_graph_sizes->graphcontextSize = 10000;

        p_graph_ptrs->graphObj     = tivxMemAlloc(p_graph_sizes->graphObjSize, (vx_enum)TIVX_MEM_EXTERNAL);
        p_graph_ptrs->graphScratch = tivxMemAlloc(p_graph_sizes->graphScratchSize, (vx_enum)TIVX_MEM_EXTERNAL);
        p_graph_ptrs->graphcontext = tivxMemAlloc(p_graph_sizes->graphcontextSize, (vx_enum)TIVX_MEM_EXTERNAL);

        if((NULL == p_graph_ptrs->graphObj) ||
            (NULL == p_graph_ptrs->graphScratch) ||
            (NULL == p_graph_ptrs->graphcontext))
        {
            status_v = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        block_width = VXLIB_min(kernel_details->block_width, buf_params[0]->dim_x);
        block_height = VXLIB_min(kernel_details->block_height, buf_params[0]->dim_y);

        /*---------------------------------------------------------------*/
        /* Initialize Graph creation time parameters                     */
        /*---------------------------------------------------------------*/
        graph_create_params.coreType             = BAM_DSP_C66x;
        graph_create_params.kernelDB             = &gBAM_TI_kernelDBdef;
        graph_create_params.nodeList             = (BAM_NodeParams*)node_list;
        graph_create_params.edgeList             = (BAM_EdgeParams*)edge_list;
        graph_create_params.graphMem             = p_graph_ptrs->graphObj;
        graph_create_params.graphMemSize         = p_graph_sizes->graphObjSize;
        graph_create_params.onChipScratchMem     = p_graph_ptrs->graphScratch;
        graph_create_params.onChipScratchMemSize = p_graph_sizes->graphScratchSize;
        graph_create_params.extMem               = p_graph_ptrs->graphcontext;
        graph_create_params.extMemSize           = p_graph_sizes->graphcontextSize;
        graph_create_params.useSmartMemAlloc     = (uint32_t)true;
        graph_create_params.optimizeBlockDim     = (uint32_t)false;

        memcpy(&graph_create_params.intMemParams, &gIntMemParams, sizeof(BAM_InternalMemParams));

        /*---------------------------------------------------------------*/
        /* Initialize the members related to the  kernels init function  */
        /*---------------------------------------------------------------*/
        graph_create_params.initKernelsArgsFunc   = &tivxBam_initKernelsArgsSingle;
        graph_create_params.initKernelsArgsParams = &graph_args;

        graph_create_params.blockDimParams.blockWidth  = block_width;
        graph_create_params.blockDimParams.blockHeight = block_height;
        graph_create_params.blockDimParams.blockWidthStep = 8;
        graph_create_params.blockDimParams.blockHeightStep = 2;
        graph_create_params.blockDimParams.blockWidthDivisorOf = 0;
        graph_create_params.blockDimParams.blockHeightDivisorOf = 0;
        #ifdef USE_ALGFRAMEWORK_2_8_0_0
        graph_create_params.blockDimParams.blockWidthMax = buf_params[0]->dim_x;
        graph_create_params.blockDimParams.blockHeightMax = VXLIB_max(8, buf_params[0]->dim_y/4);
        #endif

        status_b = BAM_createGraph(&graph_create_params, &p_graph_handle->bam_graph_handle);

        if(BAM_S_SUCCESS != status_b)
        {
            status_v = (vx_status)VX_FAILURE;
        }

        block_width = graph_create_params.blockDimParams.blockWidth;
        block_height = graph_create_params.blockDimParams.blockHeight;

        tivxBamFreeContextPtrs((tivx_bam_graph_handle)p_graph_handle);
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        if(graph_create_params.graphMemConsumed > 0) {
            p_graph_sizes->graphObjSize = graph_create_params.graphMemConsumed;
            p_graph_ptrs->graphObj = tivxMemAlloc(p_graph_sizes->graphObjSize, (vx_enum)TIVX_MEM_EXTERNAL);
            graph_create_params.graphMemSize = p_graph_sizes->graphObjSize;
            graph_create_params.graphMem = p_graph_ptrs->graphObj;

            if(NULL == p_graph_ptrs->graphObj)
            {
                status_v = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }
        else
        {
            graph_create_params.graphMemSize = 0;
            p_graph_sizes->graphObjSize = 0;
        }

        if(graph_create_params.onChipScratchMemConsumed > 0) {
            p_graph_sizes->graphScratchSize = graph_create_params.onChipScratchMemConsumed;
            p_graph_ptrs->graphScratch = tivxMemAlloc(p_graph_sizes->graphScratchSize, (vx_enum)TIVX_MEM_EXTERNAL);
            graph_create_params.onChipScratchMemSize = p_graph_sizes->graphScratchSize;
            graph_create_params.onChipScratchMem = p_graph_ptrs->graphScratch;

            if(NULL == p_graph_ptrs->graphScratch)
            {
                status_v = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }
        else
        {
            graph_create_params.onChipScratchMemSize = 0;
            p_graph_sizes->graphScratchSize = 0;
        }

        if(graph_create_params.extMemConsumed > 0) {
            p_graph_sizes->graphcontextSize = graph_create_params.extMemConsumed;
            p_graph_ptrs->graphcontext = tivxMemAlloc(p_graph_sizes->graphcontextSize, (vx_enum)TIVX_MEM_EXTERNAL);
            graph_create_params.extMemSize = p_graph_sizes->graphcontextSize;
            graph_create_params.extMem = p_graph_ptrs->graphcontext;

            if(NULL == p_graph_ptrs->graphcontext)
            {
                status_v = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }
        else
        {
            graph_create_params.extMemSize = 0;
            p_graph_sizes->graphcontextSize = 0;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        graph_create_params.optimizeBlockDim     = (uint32_t)false;

        status_b = BAM_createGraph(&graph_create_params, &p_graph_handle->bam_graph_handle);

        if(BAM_S_SUCCESS != status_b)
        {
            status_v = (vx_status)VX_FAILURE;
        }

        tivxMemFree(graph_args.compute_kernel_args, kernel_details->kernel_info.kernelArgSize, (vx_enum)TIVX_MEM_EXTERNAL);
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        BAM_KernelCommonControlArgs cmd;
        BAM_KernelCommonControlFrameArgs ctrlArgs;

        ctrlArgs.frameWidth = buf_params[0]->dim_x;
        ctrlArgs.frameHeight = buf_params[0]->dim_y;
        ctrlArgs.blockWidth = block_width;
        ctrlArgs.blockHeight = block_height;

        cmd.cmdId = BAM_CTRL_CMD_ID_SET_FRAME_ARGS;
        cmd.payload = &ctrlArgs;

        status_b = BAM_controlNode(p_graph_handle->bam_graph_handle, COMPUTE_NODE, &cmd);

        /* Some kernels may not have control function, so ignore error here */
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        EDMA_UTILS_setEdma3RmHandle(NULL);

        *graph_handle = p_graph_handle;
    }

    return status_v;
}

/* MISRA.PPARAM.NEEDS.CONST
 * MISRAC_WAIVER:
 * compute_kernel_params not const due to BAM requirements
 */
vx_status tivxBamCreateHandleMultiNode(BAM_NodeParams node_list[],
                                       uint32_t max_nodes,
                                       BAM_EdgeParams edge_list[],
                                       uint32_t max_edges,
                                       VXLIB_bufParams2D_t *buf_params[],
                                       tivx_bam_kernel_details_t kernel_details[],
                                       tivx_bam_graph_handle *graph_handle)
{
    vx_status status_v = (vx_status)VX_SUCCESS;
    BAM_Status status_b = BAM_S_SUCCESS;
    BAM_CreateGraphParams graph_create_params;
    tivx_bam_graph_args_multi_t graph_args;
    tivx_bam_graph_handle_t *p_graph_handle = NULL;
    BAM_GraphMem *p_graph_ptrs = NULL;
    BAM_GraphMemReq *p_graph_sizes = NULL;
    int32_t i, k, m;
    int16_t block_width, block_height;
    int32_t num_nodes, num_edges = 0;
    tivx_data_block_params_t *data_blocks = NULL;
    tivx_edge_params_t *edge_params = NULL;
    uint32_t num_data_blocks;
    int32_t one_shot_flag;

#ifdef HOST_EMULATION
    edmaBase[0] = (uint32_t)(&dummyEDMAreg);
#endif

    if(NULL == graph_handle)
    {
        status_v = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        /* Initialize to NULL in case there are any failures */
        *graph_handle = NULL;

        if((NULL == node_list) || (NULL == edge_list) || (NULL == buf_params) || (NULL == kernel_details))
        {
            VX_PRINT(VX_ZONE_ERROR, "NULL input parameter pointer\n");
            status_v = (vx_status)VX_FAILURE;
        }

        if ((0 == max_nodes) || (0 == max_edges) ||
            (max_nodes > TIVX_BAM_MAX_NODES) || (max_edges > TIVX_BAM_MAX_EDGES))
        {
            VX_PRINT(VX_ZONE_ERROR, "Unsupported value for either max_nodes or max_edges\n");
            status_v = (vx_status)VX_FAILURE;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        for(i = 0; i < max_nodes; i++)
        {
            if(node_list[i].nodeIndex == BAM_END_NODE_MARKER)
            {
                break;
            }
        }
        if ((i == TIVX_BAM_MAX_NODES) || (i == max_nodes))
        {
            status_v = (vx_status)VX_FAILURE;
        }
        else
        {
            num_nodes = i;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        for(i = 0; i < max_edges; i++)
        {
            if(edge_list[i].upStreamNode.id == BAM_END_NODE_MARKER)
            {
                break;
            }
        }
        if ((i == TIVX_BAM_MAX_EDGES) || (i == max_edges))
        {
            status_v = (vx_status)VX_FAILURE;
        }
        else
        {
            num_edges = i;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        /* For now keep separate ... these could potentially be an array of 2 of same structure */
        graph_args.kernel_details     = kernel_details;
        graph_args.buf_params         = buf_params;
        graph_args.num_nodes          = num_nodes;
        graph_args.num_edges          = num_edges;
        graph_args.node_list          = node_list;
        graph_args.edge_list          = edge_list;

        /*
         * At this point, the specific kernelArgs for source node does not need to be known since
         * tivxBam_initKernelsArgsMulti() will figure this out.  In order to create the graph, we need
         * to assign something, so we assign autoinc_args (even if the source is not using autoinc)
         */
        node_list[0].kernelArgs = (void *)&graph_args.dma_read_autoinc_args;
        for(i = 1; i < num_nodes-1; i++)
        {
            graph_args.compute_kernel_args[i] = tivxMemAlloc(kernel_details[i].kernel_info.kernelArgSize, (vx_enum)TIVX_MEM_EXTERNAL);
            node_list[i].kernelArgs = graph_args.compute_kernel_args[i];
            /* TIOVX-186:
             * - Although this works, may investigate more: since node_list is in graph_args, perhaps the compute_kernel_args
             *   doesn't need to be there.
             */
            if(NULL == graph_args.compute_kernel_args[i])
            {
                status_v = (vx_status)VX_ERROR_NO_MEMORY;
                break;
            }
        }
        /*
         * At this point, the specific kernelArgs for destination node does not need to be known since
         * tivxBam_initKernelsArgsMulti() will figure this out.  In order to create the graph, we need
         * to assign something, so we assign autoinc_args (even if the destination is not using autoinc)
         */
        if(node_list[i].kernelId == BAM_KERNELID_DMAWRITE_NULL)
        {
            one_shot_flag = 1;
            node_list[i].kernelArgs = (void *)&graph_args.dma_write_oneshot_args;
        }
        else
        {
            one_shot_flag = 0;
            node_list[i].kernelArgs = (void *)&graph_args.dma_write_autoinc_args;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        for(i = 1; i < num_nodes-1; i++)
        {
            if(NULL != kernel_details[i].compute_kernel_params)
            {
                VXLIB_bufParams2D_t *startPtr = (VXLIB_bufParams2D_t*)graph_args.compute_kernel_args[i];
                int32_t offset = kernel_details[i].kernel_info.numInputDataBlocks + kernel_details[i].kernel_info.numOutputDataBlocks;
                int32_t paramsSize = kernel_details[i].kernel_info.kernelArgSize - (offset*sizeof(VXLIB_bufParams2D_t));

                memcpy(&startPtr[offset], kernel_details[i].compute_kernel_params, (size_t)paramsSize);
            }
        }

        /* Allocate memory for context handle */
        p_graph_handle = tivxMemAlloc(sizeof(tivx_bam_graph_handle_t), (vx_enum)TIVX_MEM_EXTERNAL);

        if(NULL == p_graph_handle)
        {
            status_v = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        /* TIOVX-186:
         * - May need to update this based on node_list
         */
        p_graph_handle->one_shot_flag = one_shot_flag;
        p_graph_handle->multi_flag = 1;
        p_graph_handle->sink_node = num_nodes-1;

        p_graph_ptrs = &p_graph_handle->bam_graph_ptrs;
        p_graph_sizes = &p_graph_handle->bam_graph_sizes;

        status_b = BAM_initKernelDB(&gBAM_TI_kernelDBdef);

        if(BAM_S_SUCCESS != status_b)
        {
            status_v = (vx_status)VX_FAILURE;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        edge_params = tivxMemAlloc(sizeof(tivx_edge_params_t)*num_edges, (vx_enum)TIVX_MEM_EXTERNAL);

        if(NULL == edge_params)
        {
            status_v = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        num_data_blocks= 1U;

        /* From the list of edges, compute number of datablocks that will need to be allocated
                Basically every time, the pair (upStreamNodeId, upStreamNodePort) changes, a data block must be created
         */
        edge_params[0].data_block_index = 0;

        i=1;

        while (edge_list[i].upStreamNode.id != BAM_END_NODE_MARKER) {
            if ((edge_list[i].upStreamNode.id != edge_list[i-1].upStreamNode.id) ||
                (edge_list[i].upStreamNode.port != edge_list[i-1].upStreamNode.port)) {
                num_data_blocks++;
            }
            edge_params[i].data_block_index = num_data_blocks-1;
            i++;
        }

        data_blocks = tivxMemAlloc(sizeof(tivx_data_block_params_t)*num_data_blocks, (vx_enum)TIVX_MEM_EXTERNAL);

        if(NULL == data_blocks)
        {
            status_v = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        memset(data_blocks, 0, sizeof(tivx_data_block_params_t)*num_data_blocks);

        graph_args.data_block_list = data_blocks;
        graph_args.num_data_blocks = num_data_blocks;
        graph_args.edge_params = edge_params;

        /* For each data_block (fill in the parameters) */
        /* Block parameters are a factor of
         * 1. Upstream block sizes and
         * 2. Upstream node characteristics
         */
        for(i=0; i < num_data_blocks; i++)
        {
            for(k=0; (k < num_edges) && (k < max_edges); k++)
            {
                /* Find edge associated with this block index */
                if(edge_params[k].data_block_index == i)
                {
                    /* Now we can know the (2) upstream node characteristics from edge_list */
                    uint32_t upstream_node = edge_list[k].upStreamNode.id;
                    uint32_t upstream_port = edge_list[k].upStreamNode.port;
                    uint32_t downstream_node = edge_list[k].downStreamNode.id;
                    uint32_t downstream_port = edge_list[k].downStreamNode.port;
                    int32_t block_width_reduction = 0;
                    int32_t block_height_reduction = 0;
                    float maxxScale = 0.0;
                    float maxyScale = 0.0;
                    float maxInputxScale = 0.0;
                    float maxInputyScale = 0.0;
                    float xScale = 0.0;
                    float yScale = 0.0;
                    int32_t opt_x = 0;
                    int32_t num_inputs;
                    int32_t j;

                    if(upstream_node > 0)
                    {
                        if(kernel_details[upstream_node].kernel_info.nodeType == BAM_NODE_COMPUTE_NEIGHBORHOOD_OP)
                        {
                            block_width_reduction = (uint16_t)((kernel_details[upstream_node].kernel_info.kernelExtraInfo.metaInfo >> 16) - 1);
                            block_height_reduction = (uint16_t)((kernel_details[upstream_node].kernel_info.kernelExtraInfo.metaInfo & 0xFFFFU) - 1U);

                            /* Some kernels are optimized if input width == output width.  With this enabled, we want to
                             * increase the stride of the output buffer to match the input buffer, and make this output
                             * width equal to the stride for the kernel processing */
                            if(kernel_details[upstream_node].kernel_info.kernelExtraInfo.optimizationInfo == 1) {
                                opt_x = (uint16_t)((kernel_details[upstream_node].kernel_info.kernelExtraInfo.metaInfo >> 16) - 1);
                            }
                        }

                        data_blocks[i].block_params.data_type = kernel_details[upstream_node].kernel_info.kernelExtraInfo.typeOutputElmt[upstream_port];

                        num_inputs = kernel_details[upstream_node].kernel_info.numInputDataBlocks;
                        xScale = (float)(kernel_details[upstream_node].kernel_info.kernelExtraInfo.horzSamplingFactor[num_inputs+upstream_port]);
                        yScale = (float)(kernel_details[upstream_node].kernel_info.kernelExtraInfo.vertSamplingFactor[num_inputs+upstream_port]);

                        maxInputxScale = (float)(kernel_details[upstream_node].kernel_info.kernelExtraInfo.horzSamplingFactor[0]);
                        maxInputyScale = (float)(kernel_details[upstream_node].kernel_info.kernelExtraInfo.vertSamplingFactor[0]);

                        for(j = 0; j < num_inputs; j++)
                        {
                            if ((float)(kernel_details[upstream_node].kernel_info.kernelExtraInfo.horzSamplingFactor[j]) > maxInputxScale)
                            {
                                maxInputxScale = (float)(kernel_details[upstream_node].kernel_info.kernelExtraInfo.horzSamplingFactor[j]);
                            }
                            if ((float)(kernel_details[upstream_node].kernel_info.kernelExtraInfo.vertSamplingFactor[j]) > maxInputyScale)
                            {
                                maxInputyScale = (float)(kernel_details[upstream_node].kernel_info.kernelExtraInfo.vertSamplingFactor[j]);
                            }
                        }
                        xScale = xScale / maxInputxScale;
                        yScale = yScale / maxInputyScale;
                    }
                    else if (upstream_node == 0)
                    {
                        maxxScale = 1.0;
                        maxyScale = 1.0;
                        xScale = (float)(kernel_details[downstream_node].kernel_info.kernelExtraInfo.horzSamplingFactor[downstream_port]);
                        yScale = (float)(kernel_details[downstream_node].kernel_info.kernelExtraInfo.vertSamplingFactor[downstream_port]);
                    }

                    /* Program size related block params during InitArgs function */
                    data_blocks[i].block_width_reduction = block_width_reduction;
                    data_blocks[i].block_height_reduction = block_height_reduction;
                    data_blocks[i].opt_x = opt_x;


                    for(m = 0; m < num_edges; m++)
                    {
                        /* Find input edges associated with upstream node */
                        if(edge_list[m].downStreamNode.id == upstream_node)
                        {
                            uint32_t idx = edge_params[m].data_block_index;
                            uint32_t new_block_width_reduction = data_blocks[idx].total_block_width_reduction+block_width_reduction;
                            uint32_t new_block_height_reduction = data_blocks[idx].total_block_height_reduction+block_height_reduction;
                            uint32_t new_opt_x = data_blocks[idx].total_opt_x+opt_x;

                            /* Now we can know the (1) upstream block size information */
                            data_blocks[i].upstream_node = upstream_node;
                            data_blocks[i].upstream_data_block_index = idx;

                            if(new_block_width_reduction > data_blocks[i].total_block_width_reduction)
                            {
                                data_blocks[i].total_block_width_reduction = new_block_width_reduction;
                            }
                            if(new_block_height_reduction > data_blocks[i].total_block_height_reduction)
                            {
                                data_blocks[i].total_block_height_reduction = new_block_height_reduction;
                            }
                            if(new_opt_x > data_blocks[i].total_opt_x)
                            {
                                data_blocks[i].total_opt_x = new_opt_x;
                            }


                            if (data_blocks[idx].xScale > maxxScale)
                            {
                                maxxScale = data_blocks[idx].xScale;
                            }
                            if (data_blocks[idx].yScale > maxyScale)
                            {
                                maxyScale = data_blocks[idx].yScale;
                            }
                        }
                    }

                    data_blocks[i].xScale = xScale * maxxScale;
                    data_blocks[i].yScale = yScale * maxyScale;
                    break;
                }
            }
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        p_graph_sizes->graphObjSize     = 3000*num_nodes;
        p_graph_sizes->graphScratchSize = 3000*num_nodes;
        p_graph_sizes->graphcontextSize = 3000*num_nodes;

        p_graph_ptrs->graphObj     = tivxMemAlloc(p_graph_sizes->graphObjSize, (vx_enum)TIVX_MEM_EXTERNAL);
        p_graph_ptrs->graphScratch = tivxMemAlloc(p_graph_sizes->graphScratchSize, (vx_enum)TIVX_MEM_EXTERNAL);
        p_graph_ptrs->graphcontext = tivxMemAlloc(p_graph_sizes->graphcontextSize, (vx_enum)TIVX_MEM_EXTERNAL);

        if((NULL == p_graph_ptrs->graphObj) ||
            (NULL == p_graph_ptrs->graphScratch) ||
            (NULL == p_graph_ptrs->graphcontext))
        {
            status_v = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        block_width = VXLIB_min(kernel_details[0].block_width, buf_params[0]->dim_x);
        block_height = VXLIB_min(kernel_details[0].block_height, buf_params[0]->dim_y);

        /*---------------------------------------------------------------*/
        /* Initialize Graph creation time parameters                     */
        /*---------------------------------------------------------------*/
        graph_create_params.coreType             = BAM_DSP_C66x;
        graph_create_params.kernelDB             = &gBAM_TI_kernelDBdef;
        graph_create_params.nodeList             = (BAM_NodeParams*)node_list;
        graph_create_params.edgeList             = (BAM_EdgeParams*)edge_list;
        graph_create_params.graphMem             = p_graph_ptrs->graphObj;
        graph_create_params.graphMemSize         = p_graph_sizes->graphObjSize;
        graph_create_params.onChipScratchMem     = p_graph_ptrs->graphScratch;
        graph_create_params.onChipScratchMemSize = p_graph_sizes->graphScratchSize;
        graph_create_params.extMem               = p_graph_ptrs->graphcontext;
        graph_create_params.extMemSize           = p_graph_sizes->graphcontextSize;
        graph_create_params.useSmartMemAlloc     = (uint32_t)true;
        graph_create_params.optimizeBlockDim     = (uint32_t)false;

        memcpy(&graph_create_params.intMemParams, &gIntMemParams, sizeof(BAM_InternalMemParams));

        /*---------------------------------------------------------------*/
        /* Initialize the members related to the  kernels init function  */
        /*---------------------------------------------------------------*/
        graph_create_params.initKernelsArgsFunc   = &tivxBam_initKernelsArgsMulti;
        graph_create_params.initKernelsArgsParams = &graph_args;

        graph_create_params.blockDimParams.blockWidth  = block_width;
        graph_create_params.blockDimParams.blockHeight = block_height;
        graph_create_params.blockDimParams.blockWidthStep = 8;
        graph_create_params.blockDimParams.blockHeightStep = 2;
        graph_create_params.blockDimParams.blockWidthDivisorOf = 0;
        graph_create_params.blockDimParams.blockHeightDivisorOf = 0;
        #ifdef USE_ALGFRAMEWORK_2_8_0_0
        graph_create_params.blockDimParams.blockWidthMax = buf_params[0]->dim_x;
        graph_create_params.blockDimParams.blockHeightMax = VXLIB_max(8, buf_params[0]->dim_y/4);
        #endif

        status_b = BAM_createGraph(&graph_create_params, &p_graph_handle->bam_graph_handle);

        if(BAM_S_SUCCESS != status_b)
        {
            status_v = (vx_status)VX_FAILURE;
        }

        block_width = graph_create_params.blockDimParams.blockWidth;
        block_height = graph_create_params.blockDimParams.blockHeight;

        tivxBamFreeContextPtrs((tivx_bam_graph_handle)p_graph_handle);
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        if(graph_create_params.graphMemConsumed > 0) {
            p_graph_sizes->graphObjSize = graph_create_params.graphMemConsumed;
            p_graph_ptrs->graphObj = tivxMemAlloc(p_graph_sizes->graphObjSize, (vx_enum)TIVX_MEM_EXTERNAL);
            graph_create_params.graphMemSize = p_graph_sizes->graphObjSize;
            graph_create_params.graphMem = p_graph_ptrs->graphObj;

            if(NULL == p_graph_ptrs->graphObj)
            {
                status_v = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }
        else
        {
            graph_create_params.graphMemSize = 0;
            p_graph_sizes->graphObjSize = 0;
        }

        if(graph_create_params.onChipScratchMemConsumed > 0) {
            p_graph_sizes->graphScratchSize = graph_create_params.onChipScratchMemConsumed;
            p_graph_ptrs->graphScratch = tivxMemAlloc(p_graph_sizes->graphScratchSize, (vx_enum)TIVX_MEM_EXTERNAL);
            graph_create_params.onChipScratchMemSize = p_graph_sizes->graphScratchSize;
            graph_create_params.onChipScratchMem = p_graph_ptrs->graphScratch;

            if(NULL == p_graph_ptrs->graphScratch)
            {
                status_v = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }
        else
        {
            graph_create_params.onChipScratchMemSize = 0;
            p_graph_sizes->graphScratchSize = 0;
        }

        if(graph_create_params.extMemConsumed > 0) {
            p_graph_sizes->graphcontextSize = graph_create_params.extMemConsumed;
            p_graph_ptrs->graphcontext = tivxMemAlloc(p_graph_sizes->graphcontextSize, (vx_enum)TIVX_MEM_EXTERNAL);
            graph_create_params.extMemSize = p_graph_sizes->graphcontextSize;
            graph_create_params.extMem = p_graph_ptrs->graphcontext;

            if(NULL == p_graph_ptrs->graphcontext)
            {
                status_v = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }
        else
        {
            graph_create_params.extMemSize = 0;
            p_graph_sizes->graphcontextSize = 0;
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        graph_create_params.optimizeBlockDim     = (uint32_t)false;

        status_b = BAM_createGraph(&graph_create_params, &p_graph_handle->bam_graph_handle);

        if(BAM_S_SUCCESS != status_b)
        {
            status_v = (vx_status)VX_FAILURE;
        }

        for(i = 1; i < num_nodes-1; i++)
        {
            tivxMemFree(graph_args.compute_kernel_args[i], kernel_details[i].kernel_info.kernelArgSize, (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        BAM_KernelCommonControlArgs cmd;
        BAM_KernelCommonControlFrameArgs ctrlArgs;

        cmd.cmdId = BAM_CTRL_CMD_ID_SET_FRAME_ARGS;
        cmd.payload = &ctrlArgs;

        for(i = 1; i < num_nodes-1; i++)
        {
            int32_t num_inputs = kernel_details[i].kernel_info.numInputDataBlocks;
            uint32_t old_block_width = block_width;
            uint32_t old_block_height = block_height;
            int32_t j;

            for(j = 0; j < num_inputs; j++)
            {
                int32_t block_index = getDataBlockInput(edge_list, edge_params,
                                       num_edges, i, j);

                uint32_t new_block_width = block_width-data_blocks[block_index].total_block_width_reduction;
                uint32_t new_block_height = block_height-data_blocks[block_index].total_block_height_reduction;

                if((new_block_width <= old_block_width) || (new_block_height <= old_block_height))
                {
                    old_block_width = new_block_width;
                    old_block_height = new_block_height;
                    ctrlArgs.frameWidth = buf_params[0]->dim_x-data_blocks[block_index].total_block_width_reduction;
                    ctrlArgs.frameHeight = buf_params[0]->dim_y-data_blocks[block_index].total_block_height_reduction;
                    ctrlArgs.blockWidth = block_width-data_blocks[block_index].total_block_width_reduction;
                    ctrlArgs.blockHeight = block_height-data_blocks[block_index].total_block_height_reduction;
                }
            }

            status_b = BAM_controlNode(p_graph_handle->bam_graph_handle, i, &cmd);
        }
        /* Some kernels may not have control function, so ignore error here */
    }

    if((vx_status)VX_SUCCESS == status_v)
    {
        EDMA_UTILS_setEdma3RmHandle(NULL);

        *graph_handle = p_graph_handle;
    }

    /* Clean up temporary memory */
    if( edge_params != NULL)
    {
        tivxMemFree(edge_params, sizeof(tivx_edge_params_t)*num_edges, (vx_enum)TIVX_MEM_EXTERNAL);
    }
    if( data_blocks != NULL)
    {
        tivxMemFree(data_blocks, sizeof(tivx_data_block_params_t)*num_data_blocks, (vx_enum)TIVX_MEM_EXTERNAL);
    }

    return status_v;
}

void tivxBamDestroyHandle(tivx_bam_graph_handle graph_handle)
{
    tivxBamFreeContextPtrs(graph_handle);
    tivxMemFree(graph_handle, sizeof(tivx_bam_graph_handle_t), (vx_enum)TIVX_MEM_EXTERNAL);
}

vx_status tivxBamInitKernelDetails(tivx_bam_kernel_details_t *kernel_details,
                                   uint32_t num_bam_nodes,
                                   tivx_target_kernel_instance kernel)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if( kernel_details != NULL )
    {
        if(num_bam_nodes > 0)
        {
            memset(&kernel_details[0], 0, sizeof(tivx_bam_kernel_details_t)*num_bam_nodes);
            kernel_details[0].block_width = kernel->block_width;
            kernel_details[0].block_height = kernel->block_height;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"num_bam_nodes is 0\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"kernel_details is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}
