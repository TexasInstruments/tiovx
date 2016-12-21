/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_gaussian_pyramid.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_gaussian_pyramid_target_kernel = NULL;

typedef struct
{
    tivx_obj_desc_image_t *img_obj_desc[TIVX_PYRAMID_MAX_LEVELS];
    uint8_t *interm_output;
    uint32_t interm_output_size;
} tivxGassPyrmdParams;

static vx_status VX_CALLBACK tivxKernelGsnPmdProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivxGassPyrmdParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_pyramid_t *pmd;
    uint8_t *src_addr;
    uint8_t *dst_addr;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    VXLIB_bufParams2D_t vxlib_gauss;
    uint8_t *temp_buf = NULL;
    uint32_t size, levels;

    if (num_params != TIVX_KERNEL_G_PYD_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_G_PYD_MAX_PARAMS; i ++)
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
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_G_PYD_IN_IMG_IDX];
        pmd = (tivx_obj_desc_pyramid_t *)obj_desc[
            TIVX_KERNEL_G_PYD_OUT_PYT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (size == sizeof(tivxGassPyrmdParams)))
        {
            tivxGetObjDescList(pmd->obj_desc_id,
                (tivx_obj_desc_t **)prms->img_obj_desc, pmd->num_levels);

            for (levels = 0U; levels < pmd->num_levels; levels ++)
            {
                if (NULL == prms->img_obj_desc[levels])
                {
                    status = VX_FAILURE;
                    break;
                }
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        if (pmd->scale != 0.5f)
        {
            if (NULL == prms->interm_output)
            {
                status = VX_FAILURE;
            }
        }

        for (levels = 0; (levels < pmd->num_levels) && (VX_SUCCESS == status);
                levels ++)
        {
            if (0 == levels)
            {
                src = (tivx_obj_desc_image_t *)obj_desc[
                    TIVX_KERNEL_G_PYD_IN_IMG_IDX];
            }
            else
            {
                src = prms->img_obj_desc[levels - 1U];
            }
            dst = prms->img_obj_desc[levels];

            src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
                src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);
            dst->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
                dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_type);

            tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
                src->mem_ptr[0].mem_type, VX_READ_ONLY);
            tivxMemBufferMap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
                dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);

            /* Valid rectangle is ignore here */
            src_addr = (uint8_t *)((uint32_t)src->mem_ptr[0U].target_ptr +
                ownComputePatchOffset(0, 0, &src->imagepatch_addr[0U]));
            dst_addr = (uint8_t *)((uint32_t)dst->mem_ptr[0U].target_ptr +
                ownComputePatchOffset(0, 0, &dst->imagepatch_addr[0]));

            vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
            vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
            vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
            vxlib_src.data_type = VXLIB_UINT8;

            vxlib_gauss.dim_x = src->imagepatch_addr[0].dim_x;
            vxlib_gauss.dim_y = src->imagepatch_addr[0].dim_y;
            vxlib_gauss.stride_y = src->imagepatch_addr[0].stride_y;
            vxlib_gauss.data_type = VXLIB_UINT8;

            vxlib_dst.dim_x = dst->imagepatch_addr[0].dim_x;
            vxlib_dst.dim_y = dst->imagepatch_addr[0].dim_y;
            vxlib_dst.stride_y = dst->imagepatch_addr[0].stride_y;
            vxlib_dst.data_type = VXLIB_UINT8;

            if (0u == levels)
            {
                status = VXLIB_channelCopy_1to1_i8u_o8u(
                    src_addr, &vxlib_src, dst_addr, &vxlib_dst);
            }
            else
            {
                if(pmd->scale == 0.5f)
                {
                    dst_addr = (uint8_t *)(
                        (uint32_t)dst->mem_ptr[0U].target_ptr +
                        ownComputePatchOffset(
                        1u, 1u, &dst->imagepatch_addr[0]));

                    vxlib_dst.dim_x -= 2;
                    vxlib_dst.dim_y -= 2;

                    status = VXLIB_halfScaleGaussian_5x5_i8u_o8u(
                        (uint8_t*)src_addr, &vxlib_src, dst_addr, &vxlib_dst);
                }
                else
                {
                    vxlib_gauss.dim_x = vxlib_src.dim_x;
                    vxlib_gauss.dim_y = vxlib_src.dim_y - 4U;
                    vxlib_gauss.stride_y = src->imagepatch_addr[0].stride_y;

                    temp_buf = (uint8_t*)(prms->interm_output +
                        2U*vxlib_gauss.stride_y + 2U);

                    status = VXLIB_gaussian_5x5_i8u_o8u(
                        src_addr, &vxlib_src, temp_buf, &vxlib_gauss, 8);

                    vxlib_gauss.dim_y = vxlib_src.dim_y;
                    status |= VXLIB_scaleImageNearest_i8u_o8u(
                        prms->interm_output, &vxlib_gauss,
                        dst_addr, &vxlib_dst,
                        (VXLIB_F32)vxlib_src.dim_x/(VXLIB_F32)vxlib_dst.dim_x,
                        (VXLIB_F32)vxlib_src.dim_y/(VXLIB_F32)vxlib_dst.dim_y,
                        0, 0, 0, 0);
                }
            }

            tivxMemBufferUnmap(src->mem_ptr[0].target_ptr, src->mem_size[0],
                src->mem_ptr[0].mem_type, VX_READ_ONLY);
            tivxMemBufferUnmap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
                dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);

            if (status != VXLIB_SUCCESS)
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelGsnPmdCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivx_obj_desc_image_t *img;
    tivx_obj_desc_pyramid_t *pmd;
    void *temp_ptr;
    tivxGassPyrmdParams *prms = NULL;

    if (num_params != TIVX_KERNEL_G_PYD_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_G_PYD_MAX_PARAMS; i ++)
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
        img = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_G_PYD_IN_IMG_IDX];
        pmd = (tivx_obj_desc_pyramid_t *)obj_desc[
            TIVX_KERNEL_G_PYD_OUT_PYT_IDX];

        prms = tivxMemAlloc(sizeof(tivxGassPyrmdParams));

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxGassPyrmdParams));

            if (0.5f != pmd->scale)
            {
                size = img->imagepatch_addr[0].stride_y *
                    img->imagepatch_addr[0].dim_y;

                temp_ptr = tivxMemAlloc(size);

                if (NULL == temp_ptr)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
                else
                {
                    memset(temp_ptr, 0, size);
                    prms->interm_output = temp_ptr;
                    prms->interm_output_size = size;
                }
            }

            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxGassPyrmdParams));
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelGsnPmdDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivx_obj_desc_pyramid_t *pmd;
    tivxGassPyrmdParams *prms = NULL;

    if (num_params != TIVX_KERNEL_G_PYD_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_G_PYD_MAX_PARAMS; i ++)
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
        pmd = (tivx_obj_desc_pyramid_t *)obj_desc[
            TIVX_KERNEL_G_PYD_OUT_PYT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxGassPyrmdParams) == size))
        {
            if (0.5f != pmd->scale)
            {
                if (NULL == prms->interm_output)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
                else
                {
                    tivxMemFree(prms->interm_output,
                        prms->interm_output_size);
                    prms->interm_output = NULL;
                    prms->interm_output_size = 0U;
                }
            }

            tivxMemFree(prms, size);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelGsnPmdControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelGaussianPyramid()
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

        vx_gaussian_pyramid_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_GAUSSIAN_PYRAMID,
            target_name,
            tivxKernelGsnPmdProcess,
            tivxKernelGsnPmdCreate,
            tivxKernelGsnPmdDelete,
            tivxKernelGsnPmdControl,
            NULL);
    }
}


void tivxRemoveTargetKernelGaussianPyramid()
{
    tivxRemoveTargetKernel(vx_gaussian_pyramid_target_kernel);
}
