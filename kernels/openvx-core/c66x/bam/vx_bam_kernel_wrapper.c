/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "vx_bam_kernel_wrapper.h"
#include <TI/tivx_target_kernel.h>
#include "bam_dma_one_shot_node.h"
#include "edma_utils_autoincrement.h"
#include "edma_utils.h"
#include <ti/vxlib/vxlib.h>

#ifdef HOST_EMULATION
extern CSL_EdmaccRegs dummyEDMAreg;
extern uint32_t edmaBase[1];
#endif

#define SOURCE_NODE  0U
#define COMPUTE_NODE 1U
#define SINK_NODE    2U

/**
 *  PRIVATE STRUCTS
 */

/* Contains arguments to configure DMA and compute nodes */
typedef struct _tivx_bam_graph_args
{
    EDMA_UTILS_autoIncrement_initParam  dma_read_autoinc_args;
    EDMA_UTILS_autoIncrement_initParam  dma_write_autoinc_args;
    BAM_DMA_OneShot_Args                  dma_read_oneshot_args;
    BAM_DMA_OneShot_Args                  dma_write_oneshot_args;
    tivx_bam_frame_params_t              *frame_params;
    void                                    *compute_kernel_args;

} tivx_bam_graph_args_t;

typedef struct _tivx_data_block_params
{
    VXLIB_bufParams2D_t block_params;
    int32_t upstream_node;
    uint32_t upstream_data_block_index;
    int32_t set_flag;
    uint32_t block_width_reduction;
    uint32_t block_height_reduction;
    uint32_t opt_x;
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
    EDMA_UTILS_autoIncrement_initParam  dma_read_autoinc_args;
    EDMA_UTILS_autoIncrement_initParam  dma_write_autoinc_args;
    BAM_DMA_OneShot_Args                  dma_read_oneshot_args;
    BAM_DMA_OneShot_Args                  dma_write_oneshot_args;
    tivx_bam_frame_params2_t              *frame_params;
    void                                    *compute_kernel_args[10];
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
                                 uint8_t *num_outputs);

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

    tivx_bam_graph_args_t                 *graph_args          = (tivx_bam_graph_args_t*)args;
    EDMA_UTILS_autoIncrement_initParam  *dma_read_autoinc_args  = &(graph_args->dma_read_autoinc_args);
    EDMA_UTILS_autoIncrement_initParam  *dma_write_autoinc_args = &(graph_args->dma_write_autoinc_args);
    BAM_DMA_OneShot_Args                  *dma_write_oneshot_args = &(graph_args->dma_write_oneshot_args);

    tivx_bam_frame_params_t               *frame_params = graph_args->frame_params;

    /* Assumes that the first part of every computeKernel in VXLIB is an array of VXLIB_bufParams2D_t */
    VXLIB_bufParams2D_t                      *compute_kernel_args = graph_args->compute_kernel_args;

    uint32_t num_bytes;
    uint16_t optimize_x = 0;
    uint16_t out_block_width  = blockDimParams->blockWidth;
    uint16_t out_block_height = blockDimParams->blockHeight;

    if(frame_params->kernel_info.nodeType == BAM_NODE_COMPUTE_NEIGHBORHOOD_OP) {
        out_block_width -= (uint16_t)((frame_params->kernel_info.kernelExtraInfo.metaInfo >> 16) - 1);
        out_block_height -= (uint16_t)((frame_params->kernel_info.kernelExtraInfo.metaInfo & 0xFFFFU) - 1U);

        /* Some kernels are optimized if input width == output width.  With this enabled, we want to
         * increase the stride of the output buffer to match the input buffer, and make this output
         * width equal to the stride for the kernel processing */
        if(frame_params->kernel_info.kernelExtraInfo.optimizationInfo == 1) {
            optimize_x = (uint16_t)((frame_params->kernel_info.kernelExtraInfo.metaInfo >> 16) - 1);
        }
    }

    /* Configure dma_read_autoinc_args for SOURCE_NODE */
    dma_read_autoinc_args->numInTransfers   = frame_params->kernel_info.numInputDataBlocks;
    dma_read_autoinc_args->transferType     = EDMA_UTILS_TRANSFER_IN;

    for(i=0; i<frame_params->kernel_info.numInputDataBlocks; i++)
    {
        num_bytes = (uint32_t)VXLIB_sizeof(frame_params->buf_params[i]->data_type);

        compute_kernel_args[i].data_type = frame_params->buf_params[i]->data_type;
        compute_kernel_args[i].dim_x     = blockDimParams->blockWidth;
        compute_kernel_args[i].dim_y     = blockDimParams->blockHeight;
        compute_kernel_args[i].stride_y  = blockDimParams->blockWidth*num_bytes;

        assignDMAautoIncrementParams(&dma_read_autoinc_args->transferProp[i],
            frame_params->buf_params[i]->dim_x*num_bytes,/* roiWidth */
            frame_params->buf_params[i]->dim_y,/* roiHeight */
            blockDimParams->blockWidth*num_bytes,/*blkWidth */
            blockDimParams->blockHeight,/*blkHeight*/
            out_block_width*num_bytes,/* extBlkIncrementX */
            out_block_height,/* extBlkIncrementY */
            0U,/* intBlkIncrementX */
            0U,/* intBlkIncrementY */
            0U,/* roiOffset */
            0U,/* blkOffset */
            NULL,/* extMemPtr : This will come during process call */
            frame_params->buf_params[i]->stride_y,/* extMemPtrStride */
            NULL,/* DMA node will be populating this field */
            compute_kernel_args[i].stride_y,/* interMemPtrStride */
            0U /* dmaQueNo */
            );
    }
    j = i;

    if((frame_params->kernel_info.nodeType == BAM_NODE_COMPUTE_FRAME_STATS_OP) ||
       (frame_params->kernel_info.nodeType == BAM_NODE_COMPUTE_MAP_TO_LIST_OP))
    {
        uint16_t numBlksHorz = (uint16_t)(((frame_params->buf_params[0]->dim_x-1) / blockDimParams->blockWidth) + 1);
        uint16_t numBlksVert = (uint16_t)(((frame_params->buf_params[0]->dim_y-1) / blockDimParams->blockHeight) + 1);

        /* Configure dma_write_autoinc_args for SINK_NODE */
        dma_write_oneshot_args->numOutTransfers        = frame_params->kernel_info.numOutputDataBlocks;
        dma_write_oneshot_args->transferType           = EDMA_UTILS_TRANSFER_OUT;
        dma_write_oneshot_args->numTotalBlocksInFrame  = numBlksHorz * numBlksVert;
        dma_write_oneshot_args->triggerBlockId         = dma_write_oneshot_args->numTotalBlocksInFrame - 1;

        for(i=0; i<frame_params->kernel_info.numOutputDataBlocks; i++)
        {
            num_bytes = (uint32_t)VXLIB_sizeof(frame_params->buf_params[j]->data_type);

            compute_kernel_args[j].data_type = frame_params->buf_params[j]->data_type;
            compute_kernel_args[j].dim_x     = frame_params->buf_params[j]->dim_x;
            compute_kernel_args[j].dim_y     = frame_params->buf_params[j]->dim_y;
            compute_kernel_args[j].stride_y  = compute_kernel_args[j].dim_x*num_bytes;

            dma_write_oneshot_args->transferProp[i].blkWidth = frame_params->buf_params[j]->dim_x*num_bytes;
            dma_write_oneshot_args->transferProp[i].blkHeight = frame_params->buf_params[j]->dim_y;
            dma_write_oneshot_args->transferProp[i].extMemPtr = 0;
            dma_write_oneshot_args->transferProp[i].interMemPtr = 0;
            dma_write_oneshot_args->transferProp[i].extMemPtrStride = frame_params->buf_params[j]->dim_x*num_bytes;
            dma_write_oneshot_args->transferProp[i].interMemPtrStride = frame_params->buf_params[j]->dim_x*num_bytes;

            j++;
        }
    }
    else
    {
        /* Configure dma_write_autoinc_args for SINK_NODE */
        dma_write_autoinc_args->numOutTransfers  = frame_params->kernel_info.numOutputDataBlocks;
        dma_write_autoinc_args->transferType     = EDMA_UTILS_TRANSFER_OUT;

        for(i=0; i<frame_params->kernel_info.numOutputDataBlocks; i++)
        {
            num_bytes = (uint32_t)VXLIB_sizeof(frame_params->buf_params[j]->data_type);

            compute_kernel_args[j].data_type = frame_params->buf_params[j]->data_type;
            compute_kernel_args[j].dim_x     = out_block_width + optimize_x;
            compute_kernel_args[j].dim_y     = out_block_height;
            compute_kernel_args[j].stride_y  = (int32_t)(((uint32_t)(out_block_width + optimize_x))*num_bytes);

            assignDMAautoIncrementParams(&dma_write_autoinc_args->transferProp[i],
                frame_params->buf_params[j]->dim_x*num_bytes,/* roiWidth */
                frame_params->buf_params[j]->dim_y,/* roiHeight */
                out_block_width*num_bytes,/*blkWidth */
                out_block_height,/*blkHeight*/
                out_block_width*num_bytes,/* extBlkIncrementX */
                out_block_height,/* extBlkIncrementY */
                0,/* intBlkIncrementX */
                0,/* intBlkIncrementY */
                0,/* roiOffset */
                0,/* blkOffset */
                NULL,/* extMemPtr : This will come during process call */
                frame_params->buf_params[j]->stride_y,/* extMemPtrStride */
                NULL,/* DMA node will be populating this field */
                compute_kernel_args[j].stride_y,/* interMemPtrStride */
                1U /* dmaQueNo */
                );

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
                                 uint8_t *num_outputs)
{
    int32_t i;
    int32_t found = 0;
    int32_t n_outputs = 0;
    for(i = 0; i < num_edges; i++)
    {
        if(edge_list[i].upStreamNode.id == source_node_id)
        {
            found = 1;
            n_outputs++;
        }
    }

    *num_outputs = n_outputs;

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
    int32_t i, j = 0;
    uint8_t node_index;

    tivx_bam_graph_args_multi_t                 *graph_args          = (tivx_bam_graph_args_multi_t*)args;
    EDMA_UTILS_autoIncrement_initParam  *dma_read_autoinc_args  = &(graph_args->dma_read_autoinc_args);
    EDMA_UTILS_autoIncrement_initParam  *dma_write_autoinc_args = &(graph_args->dma_write_autoinc_args);
    BAM_DMA_OneShot_Args                  *dma_write_oneshot_args = &(graph_args->dma_write_oneshot_args);

    tivx_bam_frame_params2_t               *frame_params = graph_args->frame_params;
    tivx_data_block_params_t               *data_blocks = graph_args->data_block_list;

    uint32_t num_bytes;
    uint16_t optimize_x = 0;
    uint16_t in_block_width  = blockDimParams->blockWidth;
    uint16_t in_block_height = blockDimParams->blockHeight;
    uint16_t out_block_width  = in_block_width;
    uint16_t out_block_height = in_block_height;

    /* TODO: This may not work generically for different graphs ... perhaps need to reset output block of input DMA
     *       after below computation is done. */
    for(i = 1; i < graph_args->num_nodes - 1; i++)
    {
        if(frame_params->kernel_info[i].nodeType == BAM_NODE_COMPUTE_NEIGHBORHOOD_OP) {
            out_block_width -= (uint16_t)((frame_params->kernel_info[i].kernelExtraInfo.metaInfo >> 16) - 1);
            out_block_height -= (uint16_t)((frame_params->kernel_info[i].kernelExtraInfo.metaInfo & 0xFFFFU) - 1U);

            /* Some kernels are optimized if input width == output width.  With this enabled, we want to
             * increase the stride of the output buffer to match the input buffer, and make this output
             * width equal to the stride for the kernel processing */
            if(frame_params->kernel_info[i].kernelExtraInfo.optimizationInfo == 1) {
                optimize_x += (uint16_t)((frame_params->kernel_info[i].kernelExtraInfo.metaInfo >> 16) - 1);
            }
        }
    }

    /* Configure for SOURCE_NODE */

    if(getNodeIndexFromKernelId(graph_args->node_list, graph_args->num_nodes, BAM_KERNELID_DMAREAD_AUTOINCREMENT, &node_index) != 0)
    {
        uint8_t num_transfers;
        if(getNumDownstreamNodes(graph_args->edge_list, graph_args->num_edges, node_index, &num_transfers) != 0)
        {
            dma_read_autoinc_args->numInTransfers   = num_transfers;
            dma_read_autoinc_args->transferType     = EDMA_UTILS_TRANSFER_IN;

            for(i=0; i<num_transfers; i++)
            {
                num_bytes = (uint32_t)VXLIB_sizeof(frame_params->buf_params[i]->data_type);

                data_blocks[i].block_params.data_type = frame_params->buf_params[i]->data_type;
                data_blocks[i].block_params.dim_x     = blockDimParams->blockWidth;
                data_blocks[i].block_params.dim_y     = blockDimParams->blockHeight;
                data_blocks[i].block_params.stride_y  = blockDimParams->blockWidth*num_bytes;
                data_blocks[i].set_flag = 1;

                assignDMAautoIncrementParams(&dma_read_autoinc_args->transferProp[i],
                    frame_params->buf_params[i]->dim_x*num_bytes,/* roiWidth */
                    frame_params->buf_params[i]->dim_y,/* roiHeight */
                    blockDimParams->blockWidth*num_bytes,/*blkWidth */
                    blockDimParams->blockHeight,/*blkHeight*/
                    out_block_width*num_bytes,/* extBlkIncrementX */
                    out_block_height,/* extBlkIncrementY */
                    0U,/* intBlkIncrementX */
                    0U,/* intBlkIncrementY */
                    0U,/* roiOffset */
                    0U,/* blkOffset */
                    NULL,/* extMemPtr : This will come during process call */
                    frame_params->buf_params[i]->stride_y,/* extMemPtrStride */
                    NULL,/* DMA node will be populating this field */
                    blockDimParams->blockWidth*num_bytes,/* interMemPtrStride */
                    0U /* dmaQueNo */
                    );
            }
            j = i;
        }
    }
    else if(getNodeIndexFromKernelId(graph_args->node_list, graph_args->num_nodes, BAM_KERNELID_DMAREAD_ONESHOT, &node_index) != 0)
    {
        // TODO
    }
    else
    {
        // There is no source DMA?  Maybe using cache for source
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

        int32_t num_inputs = frame_params->kernel_info[i].numInputDataBlocks;
        int32_t num_outputs = frame_params->kernel_info[i].numOutputDataBlocks;

#if 0 /* Probably will use this ... not yet */
        if(frame_params->kernel_info[i].kernelExtraInfo.optimizationInfo == 1) {
            optFlag = 1;
        }
#endif

        if((frame_params->kernel_info[i].kernelExtraInfo.constraintInfo & (1U<<31)) != 0) {
            exactFlag = 1;
        }

        for(j=0; j<num_inputs; j++)
        {
            int32_t block_index = getDataBlockInput(graph_args->edge_list, graph_args->edge_params,
                                   graph_args->num_edges, i, j);

            if(data_blocks[block_index].upstream_node == 0)
            {
                memcpy(&compute_kernel_args[j], &data_blocks[block_index].block_params, sizeof(VXLIB_bufParams2D_t));
            }
            else
            {
                compute_kernel_args[j].data_type = data_blocks[block_index].block_params.data_type;
                compute_kernel_args[j].dim_x     = in_block_width - (data_blocks[block_index].total_block_width_reduction * exactFlag);
                compute_kernel_args[j].dim_y     = in_block_height - data_blocks[block_index].total_block_height_reduction;
                compute_kernel_args[j].stride_y  = data_blocks[block_index].block_params.stride_y;
            }
        }

        for(j=0; j<num_outputs; j++)
        {
            int32_t block_index = getDataBlockOutput(graph_args->edge_list, graph_args->edge_params,
                                   graph_args->num_edges, i, j);

            num_bytes = (uint32_t)VXLIB_sizeof(data_blocks[block_index].block_params.data_type);

            compute_kernel_args[j+num_inputs].data_type = data_blocks[block_index].block_params.data_type;
            compute_kernel_args[j+num_inputs].dim_x     = in_block_width - (data_blocks[block_index].total_block_width_reduction * exactFlag);
            compute_kernel_args[j+num_inputs].dim_y     = in_block_height - data_blocks[block_index].total_block_height_reduction;
            compute_kernel_args[j+num_inputs].stride_y  = (in_block_width - (data_blocks[block_index].total_block_width_reduction * exactFlag)) * num_bytes;
            data_blocks[block_index].block_params.stride_y = compute_kernel_args[j+num_inputs].stride_y;
        }
    }

    /* Configure for SINK_NODE */

    if(getNodeIndexFromKernelId(graph_args->node_list, graph_args->num_nodes, BAM_KERNELID_DMAWRITE_AUTOINCREMENT, &node_index) != 0)
    {
        uint8_t num_transfers;
        if(getNumUpstreamNodes(graph_args->edge_list, graph_args->num_edges, node_index, &num_transfers) != 0)
        {
            dma_write_autoinc_args->numOutTransfers  = num_transfers;
            dma_write_autoinc_args->transferType     = EDMA_UTILS_TRANSFER_OUT;

            for(i=0; i<num_transfers; i++)
            {
                num_bytes = (uint32_t)VXLIB_sizeof(frame_params->buf_params[j]->data_type);

                assignDMAautoIncrementParams(&dma_write_autoinc_args->transferProp[i],
                    frame_params->buf_params[j]->dim_x*num_bytes,/* roiWidth */
                    frame_params->buf_params[j]->dim_y,/* roiHeight */
                    out_block_width*num_bytes,/*blkWidth */
                    out_block_height,/*blkHeight*/
                    out_block_width*num_bytes,/* extBlkIncrementX */
                    out_block_height,/* extBlkIncrementY */
                    0,/* intBlkIncrementX */
                    0,/* intBlkIncrementY */
                    0,/* roiOffset */
                    0,/* blkOffset */
                    NULL,/* extMemPtr : This will come during process call */
                    frame_params->buf_params[j]->stride_y,/* extMemPtrStride */
                    NULL,/* DMA node will be populating this field */
                    (int32_t)(((uint32_t)(out_block_width + optimize_x))*num_bytes),/* interMemPtrStride */
                    1U /* dmaQueNo */
                    );

                j++;
            }
        }
    }
    else if(getNodeIndexFromKernelId(graph_args->node_list, graph_args->num_nodes, BAM_KERNELID_DMAWRITE_ONESHOT, &node_index) != 0)
    {
        uint16_t numBlksHorz = (uint16_t)(((frame_params->buf_params[0]->dim_x-1) / blockDimParams->blockWidth) + 1);
        uint16_t numBlksVert = (uint16_t)(((frame_params->buf_params[0]->dim_y-1) / blockDimParams->blockHeight) + 1);

        uint8_t num_transfers;
        if(getNumUpstreamNodes(graph_args->edge_list, graph_args->num_edges, node_index, &num_transfers) != 0)
        {
            /* Configure dma_write_autoinc_args for SINK_NODE */
            dma_write_oneshot_args->numOutTransfers        = num_transfers;
            dma_write_oneshot_args->transferType           = EDMA_UTILS_TRANSFER_OUT;
            dma_write_oneshot_args->numTotalBlocksInFrame  = numBlksHorz * numBlksVert;
            dma_write_oneshot_args->triggerBlockId         = dma_write_oneshot_args->numTotalBlocksInFrame - 1;

            for(i=0; i<num_transfers; i++)
            {
                num_bytes = (uint32_t)VXLIB_sizeof(frame_params->buf_params[j]->data_type);

                dma_write_oneshot_args->transferProp[i].blkWidth = frame_params->buf_params[j]->dim_x*num_bytes;
                dma_write_oneshot_args->transferProp[i].blkHeight = frame_params->buf_params[j]->dim_y;
                dma_write_oneshot_args->transferProp[i].extMemPtr = 0;
                dma_write_oneshot_args->transferProp[i].interMemPtr = 0;
                dma_write_oneshot_args->transferProp[i].extMemPtrStride = frame_params->buf_params[j]->dim_x*num_bytes;
                dma_write_oneshot_args->transferProp[i].interMemPtrStride = frame_params->buf_params[j]->dim_x*num_bytes;

                j++;
            }
        }
    }
    else
    {
        // There is no dest DMA?  Maybe using cache for destination
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
                    TIVX_MEM_EXTERNAL);
        p_handle->bam_graph_ptrs.graphcontext = NULL;
    }

    if(p_handle->bam_graph_ptrs.graphScratch != 0)
    {
        tivxMemFree(p_handle->bam_graph_ptrs.graphScratch,
                    p_handle->bam_graph_sizes.graphScratchSize,
                    TIVX_MEM_EXTERNAL);
        p_handle->bam_graph_ptrs.graphScratch = NULL;
    }

    if(p_handle->bam_graph_ptrs.graphObj != 0)
    {
        tivxMemFree(p_handle->bam_graph_ptrs.graphObj,
                    p_handle->bam_graph_sizes.graphObjSize,
                    TIVX_MEM_EXTERNAL);
        p_handle->bam_graph_ptrs.graphObj = NULL;
    }
}


/**
 * PUBLIC FUNCTIONS
 */

vx_status tivxBamUpdatePointers(tivx_bam_graph_handle graph_handle,
                               uint32_t num_inputs,
                               uint32_t num_outputs,
                               void  *ptrs[])

{
    EDMA_UTILS_autoIncrement_updateParams  dma_update_params;
    BAM_DMA_OneShot_CtlArgs oneshotParams;

    vx_status status_v = VX_SUCCESS;
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
        status_v = VX_FAILURE;
    }
    return status_v;
}

vx_status tivxBamControlNode(tivx_bam_graph_handle graph_handle,
                             uint32_t node_id,
                             uint32_t cmd,
                             void  *payload)
{
    vx_status status_v = VX_SUCCESS;
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
        status_v = VX_FAILURE;
    }

    return status_v;
}

vx_status tivxBamProcessGraph(tivx_bam_graph_handle graph_handle)
{
    vx_status status_v = VX_SUCCESS;
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
        status_v = VX_FAILURE;
    }

    return status_v;
}

/* MISRA.PPARAM.NEEDS.CONST
 * MISRAC_WAIVER:
 * compute_kernel_params not const due to BAM requirements
 */
vx_status tivxBamCreateHandleSingleNode(BAM_TI_KernelID kernel_id,
                                        tivx_bam_frame_params_t *frame_params,
                                        void *compute_kernel_params,
                                        tivx_bam_graph_handle *graph_handle)
{
    vx_status status_v = VX_SUCCESS;
    BAM_Status status_b = BAM_S_SUCCESS;
    BAM_CreateGraphParams graph_create_params;
    tivx_bam_graph_args_t graph_args;
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

    BAM_EdgeParams edge_list[VXLIB_MAX_EDGES];

    /* Initialize to NULL in case there are any failures */
    *graph_handle = NULL;

    if((NULL == frame_params) || (NULL == graph_handle))
    {
        status_v = VX_FAILURE;
    }

    if(VX_SUCCESS == status_v)
    {
        /* For now keep separate ... these could potentially be an array of 2 of same structure */
        graph_args.frame_params       = frame_params;
        graph_args.compute_kernel_args = tivxMemAlloc(frame_params->kernel_info.kernelArgSize, TIVX_MEM_EXTERNAL);

        if(NULL == graph_args.compute_kernel_args)
        {
            status_v = VX_ERROR_NO_MEMORY;
        }
    }

    if(VX_SUCCESS == status_v)
    {
        if(NULL != compute_kernel_params)
        {
            VXLIB_bufParams2D_t *startPtr = (VXLIB_bufParams2D_t*)graph_args.compute_kernel_args;
            int32_t offset = frame_params->kernel_info.numInputDataBlocks + frame_params->kernel_info.numOutputDataBlocks;
            int32_t paramsSize = frame_params->kernel_info.kernelArgSize - (offset*sizeof(VXLIB_bufParams2D_t));

            memcpy(&startPtr[offset], compute_kernel_params, (size_t)paramsSize);
        }

        /* Finish initializing node_list */

        node_list[COMPUTE_NODE].kernelId = kernel_id;

        node_list[SOURCE_NODE].kernelArgs = (void *)&graph_args.dma_read_autoinc_args;
        node_list[COMPUTE_NODE].kernelArgs = graph_args.compute_kernel_args;

        if((frame_params->kernel_info.nodeType == BAM_NODE_COMPUTE_FRAME_STATS_OP) ||
           (frame_params->kernel_info.nodeType == BAM_NODE_COMPUTE_MAP_TO_LIST_OP))
        {
            one_shot_flag = 1;
            node_list[SINK_NODE].kernelId = BAM_KERNELID_DMAWRITE_ONESHOT;
            node_list[SINK_NODE].kernelArgs = (void *)&graph_args.dma_write_oneshot_args;
        }
        else
        {
            one_shot_flag = 0;
            node_list[SINK_NODE].kernelArgs = (void *)&graph_args.dma_write_autoinc_args;
        }

        /* Initialize edge_list */

        for(i = 0; i < frame_params->kernel_info.numInputDataBlocks; i++)
        {
            edge_list[i].upStreamNode.id     = SOURCE_NODE;
            edge_list[i].upStreamNode.port   = i;
            edge_list[i].downStreamNode.id   = COMPUTE_NODE;
            edge_list[i].downStreamNode.port = i;
        }
        j = i;
        skip_port = 0;
        for(i = 0; i < frame_params->kernel_info.numOutputDataBlocks; i++)
        {
            edge_list[j].upStreamNode.id     = COMPUTE_NODE;
            edge_list[j].upStreamNode.port   = i;

            if( frame_params->buf_params[j] != NULL )
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
        edge_list[j].upStreamNode.id     = BAM_END_NODE_MARKER;
        edge_list[j].upStreamNode.port   = 0;
        edge_list[j].downStreamNode.id   = BAM_END_NODE_MARKER;
        edge_list[j].downStreamNode.port = 0;

        /* Allocate memory for context handle */
        p_graph_handle = tivxMemAlloc(sizeof(tivx_bam_graph_handle_t), TIVX_MEM_EXTERNAL);

        if(NULL == p_graph_handle)
        {
            status_v = VX_ERROR_NO_MEMORY;
        }
    }

    if(VX_SUCCESS == status_v)
    {
        p_graph_handle->one_shot_flag = one_shot_flag;
        p_graph_handle->multi_flag = 0;
        p_graph_handle->sink_node = SINK_NODE;

        p_graph_ptrs = &p_graph_handle->bam_graph_ptrs;
        p_graph_sizes = &p_graph_handle->bam_graph_sizes;

        status_b = BAM_initKernelDB(&gBAM_TI_kernelDBdef);

        if(BAM_S_SUCCESS != status_b)
        {
            status_v = VX_FAILURE;
        }
    }

    if(VX_SUCCESS == status_v)
    {
        p_graph_sizes->graphObjSize     = 10000;
        p_graph_sizes->graphScratchSize = 10000;
        p_graph_sizes->graphcontextSize = 10000;

        p_graph_ptrs->graphObj     = tivxMemAlloc(p_graph_sizes->graphObjSize, TIVX_MEM_EXTERNAL);
        p_graph_ptrs->graphScratch = tivxMemAlloc(p_graph_sizes->graphScratchSize, TIVX_MEM_EXTERNAL);
        p_graph_ptrs->graphcontext = tivxMemAlloc(p_graph_sizes->graphcontextSize, TIVX_MEM_EXTERNAL);

        if((NULL == p_graph_ptrs->graphObj) ||
            (NULL == p_graph_ptrs->graphScratch) ||
            (NULL == p_graph_ptrs->graphcontext))
        {
            status_v = VX_ERROR_NO_MEMORY;
        }
    }

    if(VX_SUCCESS == status_v)
    {
        block_width = VXLIB_min(64, frame_params->buf_params[0]->dim_x);
        block_height = VXLIB_min(48, frame_params->buf_params[0]->dim_y);

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
        graph_create_params.useSmartMemAlloc     = (BOOL)true;
        graph_create_params.optimizeBlockDim     = (BOOL)false;

        /*---------------------------------------------------------------*/
        /* Initialize the members related to the  kernels init function  */
        /*---------------------------------------------------------------*/
        graph_create_params.initKernelsArgsFunc   = &tivxBam_initKernelsArgsSingle;
        graph_create_params.initKernelsArgsParams = &graph_args;

        graph_create_params.blockDimParams.blockWidth  = block_width;
        graph_create_params.blockDimParams.blockHeight = block_height;

        status_b = BAM_createGraph(&graph_create_params, &p_graph_handle->bam_graph_handle);

        if(BAM_S_SUCCESS != status_b)
        {
            status_v = VX_FAILURE;
        }

        tivxBamFreeContextPtrs((tivx_bam_graph_handle)p_graph_handle);
    }

    if(VX_SUCCESS == status_v)
    {
        if(graph_create_params.graphMemConsumed > 0) {
            p_graph_sizes->graphObjSize = graph_create_params.graphMemConsumed;
            p_graph_ptrs->graphObj = tivxMemAlloc(p_graph_sizes->graphObjSize, TIVX_MEM_EXTERNAL);
            graph_create_params.graphMemSize = p_graph_sizes->graphObjSize;
            graph_create_params.graphMem = p_graph_ptrs->graphObj;

            if(NULL == p_graph_ptrs->graphObj)
            {
                status_v = VX_ERROR_NO_MEMORY;
            }
        }
        else
        {
            graph_create_params.graphMemSize = 0;
            p_graph_sizes->graphObjSize = 0;
        }
        
        if(graph_create_params.onChipScratchMemConsumed > 0) {
            p_graph_sizes->graphScratchSize = graph_create_params.onChipScratchMemConsumed;
            p_graph_ptrs->graphScratch = tivxMemAlloc(p_graph_sizes->graphScratchSize, TIVX_MEM_EXTERNAL);
            graph_create_params.onChipScratchMemSize = p_graph_sizes->graphScratchSize;
            graph_create_params.onChipScratchMem = p_graph_ptrs->graphScratch;

            if(NULL == p_graph_ptrs->graphScratch)
            {
                status_v = VX_ERROR_NO_MEMORY;
            }
        }
        else
        {
            graph_create_params.onChipScratchMemSize = 0;
            p_graph_sizes->graphScratchSize = 0;
        }

        if(graph_create_params.extMemConsumed > 0) {
            p_graph_sizes->graphcontextSize = graph_create_params.extMemConsumed;
            p_graph_ptrs->graphcontext = tivxMemAlloc(p_graph_sizes->graphcontextSize, TIVX_MEM_EXTERNAL);
            graph_create_params.extMemSize = p_graph_sizes->graphcontextSize;
            graph_create_params.extMem = p_graph_ptrs->graphcontext;

            if(NULL == p_graph_ptrs->graphcontext)
            {
                status_v = VX_ERROR_NO_MEMORY;
            }
        }
        else
        {
            graph_create_params.extMemSize = 0;
            p_graph_sizes->graphcontextSize = 0;
        }
    }

    if(VX_SUCCESS == status_v)
    {
        status_b = BAM_createGraph(&graph_create_params, &p_graph_handle->bam_graph_handle);

        if(BAM_S_SUCCESS != status_b)
        {
            status_v = VX_FAILURE;
        }

        tivxMemFree(graph_args.compute_kernel_args, frame_params->kernel_info.kernelArgSize, TIVX_MEM_EXTERNAL);
    }

    if(VX_SUCCESS == status_v)
    {
        BAM_KernelCommonControlArgs cmd;
        BAM_KernelCommonControlFrameArgs ctrlArgs;

        ctrlArgs.frameWidth = frame_params->buf_params[0]->dim_x;
        ctrlArgs.frameHeight = frame_params->buf_params[0]->dim_y;
        ctrlArgs.blockWidth = block_width;
        ctrlArgs.blockHeight = block_height;

        cmd.cmdId = BAM_CTRL_CMD_ID_SET_FRAME_ARGS;
        cmd.payload = &ctrlArgs;

        status_b = BAM_controlNode(p_graph_handle->bam_graph_handle, COMPUTE_NODE, &cmd);

        /* Some kernels may not have control function, so ignore error here */
    }

    if(VX_SUCCESS == status_v)
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
                                       BAM_EdgeParams edge_list[],
                                       tivx_bam_frame_params2_t *frame_params,
                                       void *compute_kernel_params[10],
                                       tivx_bam_graph_handle *graph_handle)
{
    vx_status status_v = VX_SUCCESS;
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

#ifdef HOST_EMULATION
    edmaBase[0] = (uint32_t)(&dummyEDMAreg);
#endif

    if(NULL == graph_handle)
    {
        status_v = VX_FAILURE;
    }

    if(VX_SUCCESS == status_v)
    {
        /* Initialize to NULL in case there are any failures */
        *graph_handle = NULL;

        if((NULL == node_list) || (NULL == edge_list) || (NULL == frame_params))
        {
            status_v = VX_FAILURE;
        }
    }

    if(VX_SUCCESS == status_v)
    {
        for(i = 0; i < MAX_NODES; i++)
        {
            if(node_list[i].nodeIndex == BAM_END_NODE_MARKER)
            {
                break;
            }
        }
        if( i == MAX_NODES )
        {
            status_v = VX_FAILURE;
        }
        else
        {
            num_nodes = i;
        }
    }

    if(VX_SUCCESS == status_v)
    {
        for(i = 0; i < 100; i++)
        {
            if(edge_list[i].upStreamNode.id == BAM_END_NODE_MARKER)
            {
                break;
            }
        }
        if( i == 100 )
        {
            status_v = VX_FAILURE;
        }
        else
        {
            num_edges = i;
        }
    }

    if(VX_SUCCESS == status_v)
    {
        /* For now keep separate ... these could potentially be an array of 2 of same structure */
        graph_args.frame_params       = frame_params;
        graph_args.num_nodes          = num_nodes;
        graph_args.num_edges          = num_edges;
        graph_args.node_list          = node_list;
        graph_args.edge_list          = edge_list;

        // TODO: Hack to assume autoinc_args
        node_list[0].kernelArgs = (void *)&graph_args.dma_read_autoinc_args;
        for(i = 1; i < num_nodes-1; i++)
        {
            graph_args.compute_kernel_args[i] = tivxMemAlloc(frame_params->kernel_info[i].kernelArgSize, TIVX_MEM_EXTERNAL);
            node_list[i].kernelArgs = graph_args.compute_kernel_args[i];
            // TODO: since node_list is in graph_args, perhaps the compute_kernel_args doesn't need to be there
            if(NULL == graph_args.compute_kernel_args[i])
            {
                status_v = VX_ERROR_NO_MEMORY;
                break;
            }
        }
        // TODO: Hack to assume autoinc_args
        node_list[i].kernelArgs = (void *)&graph_args.dma_write_autoinc_args;
    }

    if(VX_SUCCESS == status_v)
    {
        if(NULL != compute_kernel_params)
        {
            for(i = 1; i < num_nodes-1; i++)
            {
                if(NULL != compute_kernel_params[i])
                {
                    VXLIB_bufParams2D_t *startPtr = (VXLIB_bufParams2D_t*)graph_args.compute_kernel_args[i];
                    int32_t offset = frame_params->kernel_info[i].numInputDataBlocks + frame_params->kernel_info[i].numOutputDataBlocks;
                    int32_t paramsSize = frame_params->kernel_info[i].kernelArgSize - (offset*sizeof(VXLIB_bufParams2D_t));

                    memcpy(&startPtr[offset], compute_kernel_params[i], (size_t)paramsSize);
                }
            }
        }

        /* Allocate memory for context handle */
        p_graph_handle = tivxMemAlloc(sizeof(tivx_bam_graph_handle_t), TIVX_MEM_EXTERNAL);

        if(NULL == p_graph_handle)
        {
            status_v = VX_ERROR_NO_MEMORY;
        }
    }

    if(VX_SUCCESS == status_v)
    {
        //TODO: update this based on node_list
        p_graph_handle->one_shot_flag = 0;
        p_graph_handle->multi_flag = 1;
        p_graph_handle->sink_node = num_nodes-1;

        p_graph_ptrs = &p_graph_handle->bam_graph_ptrs;
        p_graph_sizes = &p_graph_handle->bam_graph_sizes;

        status_b = BAM_initKernelDB(&gBAM_TI_kernelDBdef);

        if(BAM_S_SUCCESS != status_b)
        {
            status_v = VX_FAILURE;
        }
    }

    if(VX_SUCCESS == status_v)
    {
        edge_params = tivxMemAlloc(sizeof(tivx_edge_params_t)*num_edges, TIVX_MEM_EXTERNAL);

        if(NULL == edge_params)
        {
            status_v = VX_ERROR_NO_MEMORY;
        }
    }

    if(VX_SUCCESS == status_v)
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

        data_blocks = tivxMemAlloc(sizeof(tivx_data_block_params_t)*num_data_blocks, TIVX_MEM_EXTERNAL);

        if(NULL == data_blocks)
        {
            status_v = VX_ERROR_NO_MEMORY;
        }
    }

    if(VX_SUCCESS == status_v)
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
            for(k=0; k < num_edges; k++)
            {
                /* Find edge associated with this block index */
                if(edge_params[k].data_block_index == i)
                {
                    /* Now we can know the (2) upstream node characteristics from edge_list */
                    uint32_t upstream_node = edge_list[k].upStreamNode.id;
                    uint32_t upstream_port = edge_list[k].upStreamNode.port;
                    int32_t block_width_reduction = 0;
                    int32_t block_height_reduction = 0;
                    int32_t opt_x = 0;

                    if(upstream_node > 0)
                    {
                        if(frame_params->kernel_info[upstream_node].nodeType == BAM_NODE_COMPUTE_NEIGHBORHOOD_OP)
                        {
                            block_width_reduction = (uint16_t)((frame_params->kernel_info[upstream_node].kernelExtraInfo.metaInfo >> 16) - 1);
                            block_height_reduction = (uint16_t)((frame_params->kernel_info[upstream_node].kernelExtraInfo.metaInfo & 0xFFFFU) - 1U);

                            /* Some kernels are optimized if input width == output width.  With this enabled, we want to
                             * increase the stride of the output buffer to match the input buffer, and make this output
                             * width equal to the stride for the kernel processing */
                            if(frame_params->kernel_info[upstream_node].kernelExtraInfo.optimizationInfo == 1) {
                                opt_x = (uint16_t)((frame_params->kernel_info[upstream_node].kernelExtraInfo.metaInfo >> 16) - 1);
                            }
                        }

                        data_blocks[i].block_params.data_type = frame_params->kernel_info[upstream_node].kernelExtraInfo.typeOutputElmt[upstream_port];
                    }

                    /* Program size related block params during InitArgs function */
                    data_blocks[i].block_width_reduction = block_width_reduction;
                    data_blocks[i].block_height_reduction = block_height_reduction;
                    data_blocks[i].opt_x = opt_x;

                    for(m = 0; m < num_edges; m++)
                    {
                        /* Find input edge associated with upstream node */
                        if(edge_list[m].downStreamNode.id == upstream_node)
                        {
                            uint32_t idx = edge_params[m].data_block_index;
                            uint32_t new_block_width_reduction = data_blocks[idx].total_block_width_reduction+block_width_reduction;
                            uint32_t new_block_height_reduction = data_blocks[idx].total_block_height_reduction+block_height_reduction;

                            if(new_block_width_reduction > data_blocks[i].total_block_width_reduction)
                            {
                                /* Now we can know the (1) upstream block size information */
                                data_blocks[i].upstream_node = upstream_node;
                                data_blocks[i].upstream_data_block_index = idx;
                                data_blocks[i].total_block_width_reduction = new_block_width_reduction;
                                data_blocks[i].total_block_height_reduction = new_block_height_reduction;
                                data_blocks[i].total_opt_x = data_blocks[idx].total_opt_x+opt_x;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    if(VX_SUCCESS == status_v)
    {
        p_graph_sizes->graphObjSize     = 10000;
        p_graph_sizes->graphScratchSize = 10000;
        p_graph_sizes->graphcontextSize = 10000;

        p_graph_ptrs->graphObj     = tivxMemAlloc(p_graph_sizes->graphObjSize, TIVX_MEM_EXTERNAL);
        p_graph_ptrs->graphScratch = tivxMemAlloc(p_graph_sizes->graphScratchSize, TIVX_MEM_EXTERNAL);
        p_graph_ptrs->graphcontext = tivxMemAlloc(p_graph_sizes->graphcontextSize, TIVX_MEM_EXTERNAL);

        if((NULL == p_graph_ptrs->graphObj) ||
            (NULL == p_graph_ptrs->graphScratch) ||
            (NULL == p_graph_ptrs->graphcontext))
        {
            status_v = VX_ERROR_NO_MEMORY;
        }
    }

    if(VX_SUCCESS == status_v)
    {
        block_width = VXLIB_min(64, frame_params->buf_params[0]->dim_x);
        block_height = VXLIB_min(48, frame_params->buf_params[0]->dim_y);

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
        graph_create_params.useSmartMemAlloc     = (BOOL)true;
        graph_create_params.optimizeBlockDim     = (BOOL)false;

        /*---------------------------------------------------------------*/
        /* Initialize the members related to the  kernels init function  */
        /*---------------------------------------------------------------*/
        graph_create_params.initKernelsArgsFunc   = &tivxBam_initKernelsArgsMulti;
        graph_create_params.initKernelsArgsParams = &graph_args;

        graph_create_params.blockDimParams.blockWidth  = block_width;
        graph_create_params.blockDimParams.blockHeight = block_height;

        status_b = BAM_createGraph(&graph_create_params, &p_graph_handle->bam_graph_handle);

        if(BAM_S_SUCCESS != status_b)
        {
            status_v = VX_FAILURE;
        }

        tivxBamFreeContextPtrs((tivx_bam_graph_handle)p_graph_handle);
    }

    if(VX_SUCCESS == status_v)
    {
        if(graph_create_params.graphMemConsumed > 0) {
            p_graph_sizes->graphObjSize = graph_create_params.graphMemConsumed;
            p_graph_ptrs->graphObj = tivxMemAlloc(p_graph_sizes->graphObjSize, TIVX_MEM_EXTERNAL);
            graph_create_params.graphMemSize = p_graph_sizes->graphObjSize;
            graph_create_params.graphMem = p_graph_ptrs->graphObj;

            if(NULL == p_graph_ptrs->graphObj)
            {
                status_v = VX_ERROR_NO_MEMORY;
            }
        }
        else
        {
            graph_create_params.graphMemSize = 0;
            p_graph_sizes->graphObjSize = 0;
        }
        
        if(graph_create_params.onChipScratchMemConsumed > 0) {
            p_graph_sizes->graphScratchSize = graph_create_params.onChipScratchMemConsumed;
            p_graph_ptrs->graphScratch = tivxMemAlloc(p_graph_sizes->graphScratchSize, TIVX_MEM_EXTERNAL);
            graph_create_params.onChipScratchMemSize = p_graph_sizes->graphScratchSize;
            graph_create_params.onChipScratchMem = p_graph_ptrs->graphScratch;

            if(NULL == p_graph_ptrs->graphScratch)
            {
                status_v = VX_ERROR_NO_MEMORY;
            }
        }
        else
        {
            graph_create_params.onChipScratchMemSize = 0;
            p_graph_sizes->graphScratchSize = 0;
        }

        if(graph_create_params.extMemConsumed > 0) {
            p_graph_sizes->graphcontextSize = graph_create_params.extMemConsumed;
            p_graph_ptrs->graphcontext = tivxMemAlloc(p_graph_sizes->graphcontextSize, TIVX_MEM_EXTERNAL);
            graph_create_params.extMemSize = p_graph_sizes->graphcontextSize;
            graph_create_params.extMem = p_graph_ptrs->graphcontext;

            if(NULL == p_graph_ptrs->graphcontext)
            {
                status_v = VX_ERROR_NO_MEMORY;
            }
        }
        else
        {
            graph_create_params.extMemSize = 0;
            p_graph_sizes->graphcontextSize = 0;
        }
    }

    if(VX_SUCCESS == status_v)
    {
        status_b = BAM_createGraph(&graph_create_params, &p_graph_handle->bam_graph_handle);

        if(BAM_S_SUCCESS != status_b)
        {
            status_v = VX_FAILURE;
        }

        for(i = 1; i < num_nodes-1; i++)
        {
            tivxMemFree(graph_args.compute_kernel_args[i], frame_params->kernel_info[i].kernelArgSize, TIVX_MEM_EXTERNAL);
        }
    }

    if(VX_SUCCESS == status_v)
    {
        BAM_KernelCommonControlArgs cmd;
        BAM_KernelCommonControlFrameArgs ctrlArgs;

        cmd.cmdId = BAM_CTRL_CMD_ID_SET_FRAME_ARGS;
        cmd.payload = &ctrlArgs;

        for(i = 1; i < num_nodes-1; i++)
        {
            int32_t num_inputs = frame_params->kernel_info[i].numInputDataBlocks;
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
                    ctrlArgs.frameWidth = frame_params->buf_params[0]->dim_x-data_blocks[block_index].total_block_width_reduction;
                    ctrlArgs.frameHeight = frame_params->buf_params[0]->dim_y-data_blocks[block_index].total_block_height_reduction;
                    ctrlArgs.blockWidth = block_width-data_blocks[block_index].total_block_width_reduction;
                    ctrlArgs.blockHeight = block_height-data_blocks[block_index].total_block_height_reduction;
                }
            }

            status_b = BAM_controlNode(p_graph_handle->bam_graph_handle, i, &cmd);
        }
        /* Some kernels may not have control function, so ignore error here */
    }

    if(VX_SUCCESS == status_v)
    {
        EDMA_UTILS_setEdma3RmHandle(NULL);

        *graph_handle = p_graph_handle;
    }

    /* Clean up temporary memory */
    if( edge_params != NULL)
    {
        tivxMemFree(edge_params, sizeof(tivx_edge_params_t)*num_edges, TIVX_MEM_EXTERNAL);
    }
    if( data_blocks != NULL)
    {
        tivxMemFree(data_blocks, sizeof(tivx_data_block_params_t)*num_data_blocks, TIVX_MEM_EXTERNAL);
    }

    return status_v;
}

void tivxBamDestroyHandle(tivx_bam_graph_handle graph_handle)
{
    tivxBamFreeContextPtrs(graph_handle);
    tivxMemFree(graph_handle, sizeof(tivx_bam_graph_handle_t), TIVX_MEM_EXTERNAL);
}
