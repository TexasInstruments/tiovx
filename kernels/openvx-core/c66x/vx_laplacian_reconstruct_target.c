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
#include <tivx_kernel_laplacian_reconstruct.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_laplacian_reconstruct_target_kernel = NULL;

typedef struct
{
    tivx_obj_desc_image_t *img_obj_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    int16_t *add_output;
    int16_t *scale_output;
    uint32_t buff_size;

    VXLIB_bufParams2D_t vxlib_src0, vxlib_src1, vxlib_add;
    VXLIB_bufParams2D_t vxlib_scale;
    VXLIB_bufParams2D_t vxlib_dst;
} tivxLaplacianReconstructParams;

static vx_status VX_CALLBACK tivxKernelLplRcstrctProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivxLaplacianReconstructParams *prms = NULL;
    tivx_obj_desc_image_t *low_img, *out_img, *pyd_level;
    tivx_obj_desc_pyramid_t *pmd;
    int16_t *src0_addr, *src1_addr;
    uint8_t *dst_addr;
    uint32_t size;

    if (num_params != TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS; i ++)
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
        low_img = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_LPL_RCNSTR_IN_IMG_IDX];
        pmd = (tivx_obj_desc_pyramid_t *)obj_desc[
            TIVX_KERNEL_LPL_RCNSTR_IN_PMD_IDX];
        out_img = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_LPL_RCNSTR_OUT_IMG_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (size == sizeof(tivxLaplacianReconstructParams)))
        {
            tivxGetObjDescList(pmd->obj_desc_id,
                (tivx_obj_desc_t **)prms->img_obj_desc, pmd->num_levels);

            for (i = 0U; i < pmd->num_levels; i ++)
            {
                if (NULL == prms->img_obj_desc[i])
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
        out_img->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            out_img->mem_ptr[0].shared_ptr, out_img->mem_ptr[0].mem_type);

        tivxMemBufferMap(low_img->mem_ptr[0].target_ptr, low_img->mem_size[0],
            low_img->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(out_img->mem_ptr[0].target_ptr, out_img->mem_size[0],
            out_img->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        for (i = 0; (i < pmd->num_levels) && (VX_SUCCESS == status);
                i ++)
        {
            pyd_level = prms->img_obj_desc[(pmd->num_levels - 1) - i];

            pyd_level->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
                pyd_level->mem_ptr[0].shared_ptr,
                pyd_level->mem_ptr[0].mem_type);
            tivxMemBufferMap(pyd_level->mem_ptr[0].target_ptr,
                pyd_level->mem_size[0], pyd_level->mem_ptr[0].mem_type,
                VX_READ_ONLY);

            if (0 == i)
            {
                /* Valid rectangle is ignore here */
                src0_addr = (int16_t *)pyd_level->mem_ptr[0].target_ptr;
                src1_addr = (int16_t *)low_img->mem_ptr[0U].target_ptr;
            }
            else
            {
                src0_addr = (int16_t *)pyd_level->mem_ptr[0].target_ptr;
                src1_addr = (int16_t *)prms->scale_output;
            }

            prms->vxlib_src0.dim_x = pyd_level->imagepatch_addr[0].dim_x;
            prms->vxlib_src0.dim_y = pyd_level->imagepatch_addr[0].dim_y;
            prms->vxlib_src0.stride_y = pyd_level->imagepatch_addr[0].stride_y;
            prms->vxlib_src0.data_type = VXLIB_INT16;

            prms->vxlib_src1.dim_x = pyd_level->imagepatch_addr[0].dim_x;
            prms->vxlib_src1.dim_y = pyd_level->imagepatch_addr[0].dim_y;
            prms->vxlib_src1.stride_y = pyd_level->imagepatch_addr[0].stride_y;
            prms->vxlib_src1.data_type = VXLIB_INT16;

            prms->vxlib_add.dim_x = pyd_level->imagepatch_addr[0].dim_x;
            prms->vxlib_add.dim_y = pyd_level->imagepatch_addr[0].dim_y;
            prms->vxlib_add.stride_y = pyd_level->imagepatch_addr[0].stride_y;
            prms->vxlib_add.data_type = VXLIB_INT16;

            status = VXLIB_add_i16s_i16s_o16s(src0_addr, &prms->vxlib_src0,
                src1_addr, &prms->vxlib_src1,
                prms->add_output, &prms->vxlib_add, VX_CONVERT_POLICY_SATURATE);

            if (VXLIB_SUCCESS == status)
            {
                if ((pmd->num_levels - 1) - i == 0)
                {
                    prms->vxlib_dst.dim_x = out_img->imagepatch_addr[0].dim_x;
                    prms->vxlib_dst.dim_y = out_img->imagepatch_addr[0].dim_y;
                    prms->vxlib_dst.stride_y =
                        out_img->imagepatch_addr[0].stride_y;
                    prms->vxlib_dst.data_type = VXLIB_UINT8;

                    dst_addr = out_img->mem_ptr[0].target_ptr;
                    status = VXLIB_convertDepth_i16s_o8u(
                        prms->add_output, &prms->vxlib_add,
                        dst_addr, &prms->vxlib_dst,
                        VX_CONVERT_POLICY_SATURATE, 0);
                }
                else
                {
                    prms->vxlib_scale.dim_x =
                        pyd_level->imagepatch_addr[0].dim_x * 2;
                    prms->vxlib_scale.dim_y =
                        pyd_level->imagepatch_addr[0].dim_y * 2;
                    prms->vxlib_scale.stride_y =
                        pyd_level->imagepatch_addr[0].stride_y * 2;
                    prms->vxlib_scale.data_type = VXLIB_INT16;

                    #if 0
                    status = VXLIB_scaleImageNearest_i16s_o16s(
                        prms->add_output, &prms->vxlib_add,
                        prms->scale_output, &prms->vxlib_scale,
                        2.0, 2.0, 0, 0);
                    #endif
                }
            }


            tivxMemBufferUnmap(pyd_level->mem_ptr[0].target_ptr,
                pyd_level->mem_size[0], pyd_level->mem_ptr[0].mem_type,
                VX_READ_ONLY);

            if (status != VXLIB_SUCCESS)
            {
                status = VX_FAILURE;
                break;
            }
        }

        tivxMemBufferUnmap(low_img->mem_ptr[0].target_ptr, low_img->mem_size[0],
            low_img->mem_ptr[0].mem_type, VX_WRITE_ONLY);
        tivxMemBufferUnmap(out_img->mem_ptr[0].target_ptr, out_img->mem_size[0],
            out_img->mem_ptr[0].mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelLplRcstrctCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_pyramid_t *pmd;
    tivxLaplacianReconstructParams *prms = NULL;

    if (num_params != TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS; i ++)
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
            TIVX_KERNEL_LPL_RCNSTR_IN_PMD_IDX];

        prms = tivxMemAlloc(sizeof(tivxLaplacianReconstructParams));

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxLaplacianReconstructParams));

            prms->buff_size = (pmd->width * pmd->height * 2u);

            prms->add_output = tivxMemAlloc(prms->buff_size);

            if (NULL == prms->add_output)
            {
                status = VX_ERROR_NO_MEMORY;
            }

            if (VX_SUCCESS == status)
            {
                prms->scale_output = tivxMemAlloc(prms->buff_size);

                if (NULL == prms->scale_output)
                {
                    status = VX_ERROR_NO_MEMORY;
                    tivxMemFree(prms->add_output, prms->buff_size);
                }
                else
                {
                    memset(prms->add_output, 0, prms->buff_size);
                    memset(prms->scale_output, 0, prms->buff_size);
                }
            }

            if (VX_SUCCESS == status)
            {
                tivxSetTargetKernelInstanceContext(kernel, prms,
                    sizeof(tivxLaplacianReconstructParams));
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelLplRcstrctDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivxLaplacianReconstructParams *prms = NULL;

    if (num_params != TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS; i ++)
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
            (sizeof(tivxLaplacianReconstructParams) == size))
        {
            if (NULL != prms->add_output)
            {
                tivxMemFree(prms->add_output, prms->buff_size);
            }
            if (NULL != prms->scale_output)
            {
                tivxMemFree(prms->scale_output, prms->buff_size);
            }

            tivxMemFree(prms, size);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelLplRcstrctControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelLaplacianReconstruct()
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

        vx_laplacian_reconstruct_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_LAPLACIAN_RECONSTRUCT,
            target_name,
            tivxKernelLplRcstrctProcess,
            tivxKernelLplRcstrctCreate,
            tivxKernelLplRcstrctDelete,
            tivxKernelLplRcstrctControl,
            NULL);
    }
}


void tivxRemoveTargetKernelLaplacianReconstruct()
{
    tivxRemoveTargetKernel(vx_laplacian_reconstruct_target_kernel);
}
