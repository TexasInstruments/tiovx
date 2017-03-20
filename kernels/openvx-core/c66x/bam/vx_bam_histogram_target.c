/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_histogram.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>
#include <vx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;
} tivxHistogramParams;

static tivx_target_kernel vx_histogram_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelHistogramProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelHistogramCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelHistogramDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelHistogramControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelHistogramProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxHistogramParams *prms = NULL;
    uint32_t i;
    tivx_obj_desc_image_t *src;
    tivx_obj_desc_distribution_t *dst;
    uint8_t *src_addr;
    vx_rectangle_t rect;
    uint32_t size;

    if (num_params != TIVX_KERNEL_HISTOGRAM_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HISTOGRAM_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HISTOGRAM_IN_IMG_IDX];
        dst = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_HISTOGRAM_OUT_IMG_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxHistogramParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        void *img_ptrs[2];

        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);
        dst->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr.shared_ptr, dst->mem_ptr.mem_type);

        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst->mem_ptr.target_ptr, dst->mem_size,
            dst->mem_ptr.mem_type, VX_WRITE_ONLY);

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));

        img_ptrs[0] = src_addr;
        img_ptrs[1] = dst->mem_ptr.target_ptr;
        tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);

        tivxMemBufferUnmap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(dst->mem_ptr.target_ptr, dst->mem_size,
            dst->mem_ptr.mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelHistogramCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src;
    tivx_obj_desc_distribution_t *dst;
    tivxHistogramParams *prms = NULL;
    uint32_t i;

    if (num_params != TIVX_KERNEL_HISTOGRAM_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HISTOGRAM_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_HISTOGRAM_IN_IMG_IDX];
        dst = (tivx_obj_desc_distribution_t *)obj_desc[
            TIVX_KERNEL_HISTOGRAM_OUT_IMG_IDX];

        prms = tivxMemAlloc(sizeof(tivxHistogramParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            tivx_bam_frame_params_t frame_params;
            BAM_VXLIB_histogram_i8u_o32u_params kernel_params;
            VXLIB_bufParams2D_t vxlib_src, vxlib_dst;

            memset(prms, 0, sizeof(tivxHistogramParams));

            vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
            vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
            vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
            vxlib_src.data_type = VXLIB_UINT8;

            vxlib_dst.dim_x = dst->mem_size;
            vxlib_dst.dim_y = 1;
            vxlib_dst.stride_y = 0;
            vxlib_dst.data_type = VXLIB_UINT8;

            /* Fill in the frame level sizes of buffers here. If the port
             * is optionally disabled, put NULL */
            frame_params.buf_params[0] = &vxlib_src;
            frame_params.buf_params[1] = &vxlib_dst;

            kernel_params.offset          = dst->offset;
            kernel_params.range           = dst->range;
            kernel_params.numBins         = dst->num_bins;

            status = BAM_VXLIB_histogram_i8u_o32u_getKernelInfo( &kernel_params,
                                                                 &frame_params.kernel_info);

            if (VX_SUCCESS == status)
            {
                status = tivxBamCreateHandleSingleNode(BAM_KERNELID_VXLIB_HISTOGRAM_I8U_O32U,
                                                       &frame_params, (void*)&kernel_params,
                                                       &prms->graph_handle);
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxHistogramParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxHistogramParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelHistogramDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxHistogramParams *prms = NULL;
    uint32_t i;

    if (num_params != TIVX_KERNEL_HISTOGRAM_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HISTOGRAM_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxHistogramParams) == size))
        {
            tivxBamDestroyHandle(prms->graph_handle);
            tivxMemFree(prms, sizeof(tivxHistogramParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelHistogramControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBamHistogram(void)
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

        vx_histogram_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_HISTOGRAM,
            target_name,
            tivxKernelHistogramProcess,
            tivxKernelHistogramCreate,
            tivxKernelHistogramDelete,
            tivxKernelHistogramControl,
            NULL);
    }
}


void tivxRemoveTargetKernelBamHistogram(void)
{
    tivxRemoveTargetKernel(vx_histogram_target_kernel);
}
