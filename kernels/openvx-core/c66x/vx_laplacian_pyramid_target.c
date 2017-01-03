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
#include <tivx_kernel_laplacian_pyramid.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_laplacian_pyramid_target_kernel = NULL;

typedef struct
{
    tivx_obj_desc_image_t *img_obj_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    uint8_t *gauss_output;
    uint8_t *hsg_output0;
    uint8_t *hsg_output1;
    uint32_t buff_size;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst, vxlib_low_out;
    VXLIB_bufParams2D_t vxlib_gauss0, vxlib_gauss1;
} tivxLaplacianPyramidParams;

static vx_status VX_CALLBACK tivxKernelLplPmdProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivxLaplacianPyramidParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst, *low_img;
    tivx_obj_desc_pyramid_t *pmd;
    uint8_t *src_addr;
    int16_t *dst_addr;
    uint32_t size, levels;

    if (num_params != TIVX_KERNEL_LPL_PMD_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_LPL_PMD_MAX_PARAMS; i ++)
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
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_LPL_PMD_IN_IMG_IDX];
        pmd = (tivx_obj_desc_pyramid_t *)obj_desc[
            TIVX_KERNEL_LPL_PMD_OUT_PMD_IDX];
        low_img = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_LPL_PMD_OUT_IMG_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (size == sizeof(tivxLaplacianPyramidParams)))
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
        low_img->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            low_img->mem_ptr[0].shared_ptr, low_img->mem_ptr[0].mem_type);
        tivxMemBufferMap(low_img->mem_ptr[0].target_ptr, low_img->mem_size[0],
            low_img->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        for (levels = 0; (levels < pmd->num_levels) && (VX_SUCCESS == status);
                levels ++)
        {
            src = (tivx_obj_desc_image_t *)obj_desc[
                TIVX_KERNEL_LPL_PMD_IN_IMG_IDX];

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
            dst_addr = (int16_t *)((uint32_t)dst->mem_ptr[0U].target_ptr +
                ownComputePatchOffset(0, 0, &dst->imagepatch_addr[0]));

            prms->vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
            prms->vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
            prms->vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
            prms->vxlib_src.data_type = VXLIB_UINT8;

            prms->vxlib_gauss0.dim_x = dst->imagepatch_addr[0].dim_x;
            prms->vxlib_gauss0.dim_y = dst->imagepatch_addr[0].dim_y;
            prms->vxlib_gauss0.stride_y = dst->imagepatch_addr[0].dim_y;
            prms->vxlib_gauss0.data_type = VXLIB_UINT8;

            prms->vxlib_gauss1.dim_x = dst->imagepatch_addr[0].dim_x;
            prms->vxlib_gauss1.dim_y = dst->imagepatch_addr[0].dim_y;
            prms->vxlib_gauss1.stride_y = dst->imagepatch_addr[0].dim_y * 2;
            prms->vxlib_gauss1.data_type = VXLIB_UINT8;

            prms->vxlib_dst.dim_x = dst->imagepatch_addr[0].dim_x;
            prms->vxlib_dst.dim_y = dst->imagepatch_addr[0].dim_y;
            prms->vxlib_dst.stride_y = dst->imagepatch_addr[0].stride_y;
            prms->vxlib_dst.data_type = VXLIB_INT16;

            prms->vxlib_low_out.dim_x = dst->imagepatch_addr[0].dim_x;
            prms->vxlib_low_out.dim_y = dst->imagepatch_addr[0].dim_y;
            prms->vxlib_low_out.stride_y = dst->imagepatch_addr[0].stride_y;
            prms->vxlib_low_out.data_type = VXLIB_INT16;

            if (0u == levels)
            {
                status = VXLIB_channelCopy_1to1_i8u_o8u(
                    src_addr, &prms->vxlib_src,
                    prms->hsg_output0, &prms->vxlib_gauss0);
            }
            else
            {
                /* First do half scale gaussian filter */
                status = VXLIB_halfScaleGaussian_5x5_br_i8u_o8u_o8u(
                    src_addr, &prms->vxlib_src,
                    prms->hsg_output0, &prms->vxlib_gauss0,
                    prms->hsg_output1, &prms->vxlib_gauss1, 0, 0);
            }

            if (VXLIB_SUCCESS == status)
            {
                status = VXLIB_gaussian_5x5_br_i8u_o8u(
                    prms->hsg_output0, &prms->vxlib_gauss0,
                    prms->gauss_output,&prms->vxlib_gauss0, 8, 0, 0);
            }

            if (VXLIB_SUCCESS == status)
            {
                status = VXLIB_subtract_i8u_i8u_o16s(
                    prms->hsg_output0, &prms->vxlib_gauss0,
                    prms->gauss_output, &prms->vxlib_gauss0,
                    dst_addr, &prms->vxlib_dst, VXLIB_CONVERT_POLICY_WRAP);
            }

            if ((VXLIB_SUCCESS == status) &&
                (levels == (pmd->num_levels - 1u)))
            {
                dst_addr = (int16_t *)(
                    (uint32_t)low_img->mem_ptr[0U].target_ptr +
                    ownComputePatchOffset(0, 0,
                    &low_img->imagepatch_addr[0]));

                status = VXLIB_convertDepth_i8u_o16s(prms->gauss_output,
                        &prms->vxlib_gauss0, dst_addr, &prms->vxlib_low_out,
                        0);
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

        tivxMemBufferUnmap(low_img->mem_ptr[0].target_ptr, low_img->mem_size[0],
            low_img->mem_ptr[0].mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelLplPmdCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_image_t *img;
    tivxLaplacianPyramidParams *prms = NULL;

    if (num_params != TIVX_KERNEL_LPL_PMD_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_LPL_PMD_MAX_PARAMS; i ++)
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
            TIVX_KERNEL_LPL_PMD_IN_IMG_IDX];

        prms = tivxMemAlloc(sizeof(tivxLaplacianPyramidParams));

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxLaplacianPyramidParams));

            prms->buff_size = (img->imagepatch_addr[0].stride_y *
                img->imagepatch_addr[0].dim_y);

            prms->gauss_output = tivxMemAlloc(prms->buff_size);

            if (NULL == prms->gauss_output)
            {
                status = VX_ERROR_NO_MEMORY;
            }

            if (VX_SUCCESS == status)
            {
                prms->hsg_output0 = tivxMemAlloc(prms->buff_size);

                if (NULL == prms->hsg_output0)
                {
                    status = VX_ERROR_NO_MEMORY;
                    tivxMemFree(prms->gauss_output, prms->buff_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->hsg_output1 = tivxMemAlloc(prms->buff_size);

                if (NULL == prms->hsg_output1)
                {
                    status = VX_ERROR_NO_MEMORY;
                    tivxMemFree(prms->gauss_output, prms->buff_size);
                    tivxMemFree(prms->hsg_output0, prms->buff_size);
                }
                else
                {
                    memset(prms->hsg_output0, 0, prms->buff_size);
                    memset(prms->hsg_output1, 0, prms->buff_size);
                    memset(prms->gauss_output, 0, prms->buff_size);
                }
            }
            if (VX_SUCCESS == status)
            {
                tivxSetTargetKernelInstanceContext(kernel, prms,
                    sizeof(tivxLaplacianPyramidParams));
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelLplPmdDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivxLaplacianPyramidParams *prms = NULL;

    if (num_params != TIVX_KERNEL_LPL_PMD_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_LPL_PMD_MAX_PARAMS; i ++)
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
            (sizeof(tivxLaplacianPyramidParams) == size))
        {
            if (NULL != prms->hsg_output0)
            {
                tivxMemFree(prms->hsg_output0, prms->buff_size);
            }
            if (NULL != prms->hsg_output1)
            {
                tivxMemFree(prms->hsg_output1, prms->buff_size);
            }
            if (NULL != prms->gauss_output)
            {
                tivxMemFree(prms->gauss_output, prms->buff_size);
            }

            tivxMemFree(prms, size);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelLplPmdControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelLaplacianPyramid()
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

        vx_laplacian_pyramid_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_LAPLACIAN_PYRAMID,
            target_name,
            tivxKernelLplPmdProcess,
            tivxKernelLplPmdCreate,
            tivxKernelLplPmdDelete,
            tivxKernelLplPmdControl,
            NULL);
    }
}


void tivxRemoveTargetKernelLaplacianPyramid()
{
    tivxRemoveTargetKernel(vx_laplacian_pyramid_target_kernel);
}
