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

#define SOURCE_NODE  0
#define COMPUTE_NODE 1
#define SINK_NODE    2

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


/* Contains context information for an instance of a BAM graph */
typedef struct _tivx_bam_graph_handle
{
    BAM_GraphHandle     bam_graph_handle;
    BAM_GraphMem        bam_graph_ptrs;
    BAM_GraphMemReq     bam_graph_sizes;
    uint32_t             one_shot_flag;

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
static int32_t VXLIB_TI_initKernelsArgs(void *args, BAM_BlockDimParams *blockDimParams)
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
        out_block_width -= ((frame_params->kernel_info.kernelExtraInfo.metaInfo >> 16) - 1);
        out_block_height -= ((frame_params->kernel_info.kernelExtraInfo.metaInfo & 0xFFFF) - 1);

        /* Some kernels are optimized if input width == output width.  With this enabled, we want to
         * increase the stride of the output buffer to match the input buffer, and make this output
         * width equal to the stride for the kernel processing */
        if(frame_params->kernel_info.kernelExtraInfo.optimizationInfo == 1) {
            optimize_x = (frame_params->kernel_info.kernelExtraInfo.metaInfo >> 16) - 1;
        }
    }

    /* Configure dma_read_autoinc_args for SOURCE_NODE */
    dma_read_autoinc_args->numInTransfers   = frame_params->kernel_info.numInputDataBlocks;
    dma_read_autoinc_args->transferType     = EDMA_UTILS_TRANSFER_IN;

    for(i=0; i<frame_params->kernel_info.numInputDataBlocks; i++)
    {
        num_bytes = VXLIB_sizeof(frame_params->buf_params[i]->data_type);

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
            0,/* intBlkIncrementX */
            0,/* intBlkIncrementY */
            0,/* roiOffset */
            0,/* blkOffset */
            NULL,/* extMemPtr : This will come during process call */
            frame_params->buf_params[i]->stride_y,/* extMemPtrStride */
            NULL,/* DMA node will be populating this field */
            compute_kernel_args[i].stride_y,/* interMemPtrStride */
            0 /* dmaQueNo */
            );
    }
    j = i;

    if((frame_params->kernel_info.nodeType == BAM_NODE_COMPUTE_FRAME_STATS_OP) ||
       (frame_params->kernel_info.nodeType == BAM_NODE_COMPUTE_MAP_TO_LIST_OP))
    {
        uint16_t numBlksHorz = ((frame_params->buf_params[0]->dim_x-1) / blockDimParams->blockWidth) + 1;
        uint16_t numBlksVert = ((frame_params->buf_params[0]->dim_y-1) / blockDimParams->blockHeight) + 1;

        /* Configure dma_write_autoinc_args for SINK_NODE */
        dma_write_oneshot_args->numOutTransfers        = frame_params->kernel_info.numOutputDataBlocks;
        dma_write_oneshot_args->transferType           = EDMA_UTILS_TRANSFER_OUT;
        dma_write_oneshot_args->numTotalBlocksInFrame  = numBlksHorz * numBlksVert;
        dma_write_oneshot_args->triggerBlockId         = dma_write_oneshot_args->numTotalBlocksInFrame - 1;

        for(i=0; i<frame_params->kernel_info.numOutputDataBlocks; i++)
        {
            num_bytes = VXLIB_sizeof(frame_params->buf_params[j]->data_type);

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
            num_bytes = VXLIB_sizeof(frame_params->buf_params[j]->data_type);

            compute_kernel_args[j].data_type = frame_params->buf_params[j]->data_type;
            compute_kernel_args[j].dim_x     = out_block_width + optimize_x;
            compute_kernel_args[j].dim_y     = out_block_height;
            compute_kernel_args[j].stride_y  = (out_block_width + optimize_x)*num_bytes;

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
    int32_t i, j;

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

        status= BAM_controlNode(p_handle->bam_graph_handle, SINK_NODE, &oneshotParams);
    }
    else
    {
        dma_update_params.transferType = EDMA_UTILS_TRANSFER_OUT;

        for( i = 0; i < num_outputs; i++)
        {
            dma_update_params.updateParams[i].extMemPtr = ptrs[j];
            j++;
        }

        status= BAM_controlNode(p_handle->bam_graph_handle, SINK_NODE, &dma_update_params);
    }

 Exit:
    if(BAM_S_SUCCESS != status)
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

                          
vx_status tivxBamCreateHandle(BAM_TI_KernelID kernel_id,
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

            memcpy(&startPtr[offset], compute_kernel_params, paramsSize);
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
        graph_create_params.initKernelsArgsFunc   = &VXLIB_TI_initKernelsArgs;
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

void tivxBamDestroyHandle(tivx_bam_graph_handle graph_handle)
{
    tivxBamFreeContextPtrs(graph_handle);
    tivxMemFree(graph_handle, sizeof(tivx_bam_graph_handle_t), TIVX_MEM_EXTERNAL);
}
