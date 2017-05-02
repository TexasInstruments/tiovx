/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_halfscale_gaussian.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>
#include <vx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;
} tivxHalfScaleGaussianParams;

static tivx_target_kernel vx_halfscale_gaussian_target_kernel = NULL;

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxHalfScaleGaussianParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_scalar_t *gsize;
    vx_uint8 *src_addr, *dst_addr;
    vx_rectangle_t rect;
    uint32_t size, gsize_value = 1;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_SRC_IDX];
        gsize = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_GSIZE_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_DST_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxHalfScaleGaussianParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        void *img_ptrs[2];

        src->mem_ptr[0U].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0U].shared_ptr, src->mem_ptr[0U].mem_type);
        dst->mem_ptr[0U].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0U].shared_ptr, dst->mem_ptr[0U].mem_type);

        tivxMemBufferMap(src->mem_ptr[0U].target_ptr, src->mem_size[0],
            src->mem_ptr[0U].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst->mem_ptr[0U].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0U].mem_type, VX_WRITE_ONLY);

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));

        dst_addr = (uint8_t *)((uintptr_t)dst->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0]));

        if( gsize != NULL)
        {
            gsize_value = gsize->data.s32;
        }

        if (gsize_value == 1)
        {
        }
        else if (gsize_value == 3)
        {
        }
        else if (gsize_value == 5)
        {
            dst_addr = (vx_uint8*)(dst_addr + dst->imagepatch_addr[0].stride_y + 1);
        }
        else
        {
        }

        img_ptrs[0] = src_addr;
        img_ptrs[1] = dst_addr;
        tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);

        tivxMemBufferUnmap(src->mem_ptr[0U].target_ptr, src->mem_size[0],
            src->mem_ptr[0U].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(dst->mem_ptr[0U].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0U].mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_scalar_t *gsize;
    tivxHalfScaleGaussianParams *prms = NULL;
    int32_t gsize_value = 1;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_HALFSCALE_GAUSSIAN_SRC_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_HALFSCALE_GAUSSIAN_DST_IDX];
        gsize = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HALFSCALE_GAUSSIAN_GSIZE_IDX];

        prms = tivxMemAlloc(sizeof(tivxHalfScaleGaussianParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            tivx_bam_kernel_details_t kernel_details;
            VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
            VXLIB_bufParams2D_t *buf_params[2];

            memset(prms, 0, sizeof(tivxHalfScaleGaussianParams));

            vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
            vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
            vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
            vxlib_src.data_type = VXLIB_UINT8;

            vxlib_dst.dim_x = dst->imagepatch_addr[0].dim_x;
            vxlib_dst.dim_y = dst->imagepatch_addr[0].dim_y;
            vxlib_dst.stride_y = dst->imagepatch_addr[0].stride_y;
            vxlib_dst.data_type = VXLIB_UINT8;

            if( gsize != NULL)
            {
                gsize_value = gsize->data.s32;
            }

            /* Fill in the frame level sizes of buffers here. If the port
             * is optionally disabled, put NULL */
            buf_params[0] = &vxlib_src;
            buf_params[1] = &vxlib_dst;

            if (1 == gsize_value)
            {
                BAM_VXLIB_scaleImageNearest_i8u_o8u_params kernel_params;

                kernel_params.xScale = 2;
                kernel_params.yScale = 2;
                kernel_params.srcOffsetX = 0;
                kernel_params.srcOffsetY = 0;
                kernel_params.dstOffsetX = 0;
                kernel_params.dstOffsetY = 0;

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                BAM_VXLIB_scaleImageNearest_i8u_o8u_getKernelInfo(
                    &kernel_params, &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_SCALEIMAGENEARES_I8U_O8U,
                    buf_params, &kernel_details,
                    &prms->graph_handle);
            }
            else if ((gsize_value == 3) || (gsize_value == 5))
            {
                if (gsize_value == 5)
                {
                    vxlib_dst.dim_x -= 2;
                    vxlib_dst.dim_y -= 2;

                    kernel_details.compute_kernel_params = NULL;

                    BAM_VXLIB_halfScaleGaussian_5x5_i8u_o8u_getKernelInfo(
                        NULL, &kernel_details.kernel_info);

                    status = tivxBamCreateHandleSingleNode(
                        BAM_KERNELID_VXLIB_HALFSCALEGAUSSIAN_5x5_I8U_O8U,
                        buf_params, &kernel_details,
                        &prms->graph_handle);
                }
            }
            else
            {
                status = VX_FAILURE;
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxHalfScaleGaussianParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxHalfScaleGaussianParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxHalfScaleGaussianParams *prms = NULL;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxHalfScaleGaussianParams) == size))
        {
            tivxBamDestroyHandle(prms->graph_handle);
            tivxMemFree(prms, sizeof(tivxHalfScaleGaussianParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxBamKernelHalfScaleGaussianControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBamHalfscaleGaussian(void)
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

        vx_halfscale_gaussian_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_HALFSCALE_GAUSSIAN,
            target_name,
            tivxBamKernelHalfScaleGaussianProcess,
            tivxBamKernelHalfScaleGaussianCreate,
            tivxBamKernelHalfScaleGaussianDelete,
            tivxBamKernelHalfScaleGaussianControl,
            NULL);
    }
}


void tivxRemoveTargetKernelBamHalfscaleGaussian(void)
{
    tivxRemoveTargetKernel(vx_halfscale_gaussian_target_kernel);
}
