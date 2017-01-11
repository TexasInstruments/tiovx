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
#include <tivx_kernel_canny.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

typedef struct
{
    /* Sobel parameters */
    uint8_t *sobel_x;
    uint8_t *sobel_y;
    uint32_t sobel_size;
    VXLIB_bufParams2D_t vxlib_sobx, vxlib_soby;


    /* Parameters for Norm */
    uint8_t *norm;
    uint32_t norm_size;
    VXLIB_bufParams2D_t vxlib_norm;

    /* Parameters for canny NMS */
    uint8_t *nms_edge;
    uint32_t nms_edge_size;
    VXLIB_bufParams2D_t vxlib_edge;

    uint32_t *edge_list;
    uint32_t edge_list_size;

    VXLIB_bufParams2D_t vxlib_src;
    VXLIB_bufParams2D_t vxlib_dst;
} tivxCannyParams;

static vx_status tivxCannyCalcSobel(tivxCannyParams *prms,
    uint8_t *src_addr, int32_t gs);
static vx_status tivxCannyCalcNorm(tivxCannyParams *prms, vx_enum norm,
    int32_t gs);
static vx_status tivxCannyCalcNms(tivxCannyParams *prms, int32_t gs);
static vx_status tivxCannyCalcDblThr(tivxCannyParams *prms,
    uint32_t *num_items, int32_t lower, int32_t upper, int32_t gs);

static void tivxCannyFreeMem(tivxCannyParams *prms);

static tivx_target_kernel vx_harris_corners_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelCannyProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivxCannyParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_threshold_t *thr;
    uint8_t *src_addr, *dst_addr;
    tivx_obj_desc_scalar_t *sc_gs, *sc_norm;
    vx_rectangle_t rect;
    uint32_t size, num_dbl_thr_items = 0, num_edge_trace_out = 0;

    if (num_params != TIVX_KERNEL_CNED_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_CNED_MAX_PARAMS; i ++)
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
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CNED_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CNED_OUT_IMG_IDX];
        thr = (tivx_obj_desc_threshold_t *)obj_desc[
            TIVX_KERNEL_CNED_IN_THR_IDX];
        sc_gs = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_CNED_IN_SC_GS_IDX];
        sc_norm = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_CNED_IN_SC_NORM_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxCannyParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);
        dst->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_type);

        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src->valid_roi;
        src_addr = (uint8_t *)((uintptr_t)src->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));
        dst_addr = (uint8_t *)((uintptr_t)dst->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0U]));

        prms->vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
        prms->vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
        prms->vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
        prms->vxlib_src.data_type = VXLIB_UINT8;

        prms->vxlib_dst.dim_x = dst->imagepatch_addr[0].dim_x;
        prms->vxlib_dst.dim_y = dst->imagepatch_addr[0].dim_y;
        prms->vxlib_dst.stride_y = dst->imagepatch_addr[0].stride_y;
        prms->vxlib_dst.data_type = VXLIB_UINT8;

        status = tivxCannyCalcSobel(prms, src_addr, sc_gs->data.s32);

        if (VXLIB_SUCCESS == status)
        {
            status = tivxCannyCalcNorm(prms, sc_norm->data.enm,
                sc_gs->data.s32);
        }
        if (VXLIB_SUCCESS == status)
        {
            status = tivxCannyCalcNms(prms, sc_gs->data.s32);
        }
        if (VXLIB_SUCCESS == status)
        {
            status = tivxCannyCalcDblThr(prms, &num_dbl_thr_items, thr->lower,
                thr->upper, sc_gs->data.s32);
        }
        if (VXLIB_SUCCESS == status)
        {
            status = VXLIB_edgeTracing_i8u(prms->nms_edge, &prms->vxlib_edge,
                prms->edge_list, prms->edge_list_size, num_dbl_thr_items,
                &num_edge_trace_out);
        }
        if (VXLIB_SUCCESS == status)
        {
            status = VXLIB_thresholdBinary_i8u_o8u(prms->nms_edge,
                &prms->vxlib_edge, dst_addr, &prms->vxlib_dst, 128, 255, 0);
        }

        if (status != VXLIB_SUCCESS)
        {
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelCannyCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_image_t *src, *dst;
    tivxCannyParams *prms = NULL;
    tivx_obj_desc_scalar_t *sc_gs;

    if (num_params != TIVX_KERNEL_CNED_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_CNED_MAX_PARAMS; i ++)
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
            TIVX_KERNEL_CNED_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CNED_OUT_IMG_IDX];
        sc_gs = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_CNED_IN_SC_GS_IDX];

        prms = tivxMemAlloc(sizeof(tivxCannyParams));
        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxCannyParams));

            prms->vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
            prms->vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
            prms->vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
            prms->vxlib_src.data_type = VXLIB_UINT8;

            prms->vxlib_dst.dim_x = dst->imagepatch_addr[0].dim_x;
            prms->vxlib_dst.dim_y = dst->imagepatch_addr[0].dim_y;
            prms->vxlib_dst.stride_y = dst->imagepatch_addr[0].stride_y;
            prms->vxlib_dst.data_type = VXLIB_UINT8;

            prms->vxlib_sobx.dim_x = src->imagepatch_addr[0].dim_x;
            prms->vxlib_sobx.dim_y = src->imagepatch_addr[0].dim_y -
                (sc_gs->data.s32 - 1u);
            prms->vxlib_sobx.stride_y = (src->imagepatch_addr[0].stride_y * 2u);
            prms->vxlib_sobx.data_type = VXLIB_UINT16;

            prms->vxlib_soby.dim_x = src->imagepatch_addr[0].dim_x;
            prms->vxlib_soby.dim_y = src->imagepatch_addr[0].dim_y -
                (sc_gs->data.s32 - 1u);
            prms->vxlib_soby.stride_y = (src->imagepatch_addr[0].stride_y * 2u);
            prms->vxlib_soby.data_type = VXLIB_UINT16;

            prms->sobel_size = prms->vxlib_sobx.stride_y *
                src->imagepatch_addr[0].dim_y;

            prms->sobel_x = tivxMemAlloc(prms->sobel_size);
            if (NULL == prms->sobel_x)
            {
                status = VX_ERROR_NO_MEMORY;
            }

            if (VX_SUCCESS == status)
            {
                prms->sobel_y = tivxMemAlloc(prms->sobel_size);
                if (NULL == prms->sobel_y)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->vxlib_norm.dim_x = src->imagepatch_addr[0].dim_x;
                prms->vxlib_norm.dim_y = src->imagepatch_addr[0].dim_y -
                    (sc_gs->data.s32 - 1u);
                prms->vxlib_norm.stride_y = (src->imagepatch_addr[0].stride_y *
                    2u);
                prms->vxlib_norm.data_type = VXLIB_UINT16;

                prms->norm_size = prms->vxlib_norm.stride_y *
                    src->imagepatch_addr[0].dim_y;

                prms->norm = tivxMemAlloc(prms->norm_size);
                if (NULL == prms->norm)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->vxlib_edge.dim_x = src->imagepatch_addr[0].dim_x;
                prms->vxlib_edge.dim_y = src->imagepatch_addr[0].dim_y -
                    (sc_gs->data.s32 - 1u) - 2u;
                prms->vxlib_edge.stride_y = src->imagepatch_addr[0].stride_y;
                prms->vxlib_edge.data_type = VXLIB_UINT8;

                prms->nms_edge_size = prms->vxlib_edge.stride_y *
                    src->imagepatch_addr[0].dim_y;

                prms->nms_edge = tivxMemAlloc(prms->nms_edge_size);
                if (NULL == prms->nms_edge)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->edge_list_size = ((prms->vxlib_edge.stride_y / 2) *
                    prms->vxlib_edge.dim_y);

                prms->edge_list = tivxMemAlloc(prms->edge_list_size * 4u);
                if (NULL == prms->edge_list)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxCannyParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxCannyFreeMem(prms);
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelCannyDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivxCannyParams *prms = NULL;

    if (num_params != TIVX_KERNEL_CNED_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_CNED_MAX_PARAMS; i ++)
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
            (sizeof(tivxCannyParams) == size))
        {
            tivxCannyFreeMem(prms);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelCannyControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelCannyEd()
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

        vx_harris_corners_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_CANNY_EDGE_DETECTOR,
            target_name,
            tivxKernelCannyProcess,
            tivxKernelCannyCreate,
            tivxKernelCannyDelete,
            tivxKernelCannyControl,
            NULL);
    }
}


void tivxRemoveTargetKernelCannyEd()
{
    tivxRemoveTargetKernel(vx_harris_corners_target_kernel);
}

static void tivxCannyFreeMem(tivxCannyParams *prms)
{
    if (NULL != prms)
    {
        if (NULL != prms->sobel_x)
        {
            tivxMemFree(prms->sobel_x, prms->sobel_size);
            prms->sobel_x = NULL;
        }
        if (NULL != prms->sobel_y)
        {
            tivxMemFree(prms->sobel_y, prms->sobel_size);
            prms->sobel_y = NULL;
        }
        if (NULL != prms->norm)
        {
            tivxMemFree(prms->norm, prms->norm_size);
            prms->norm = NULL;
        }
        if (NULL != prms->nms_edge)
        {
            tivxMemFree(prms->nms_edge, prms->nms_edge_size);
            prms->nms_edge = NULL;
        }
        if (NULL != prms->edge_list)
        {
            tivxMemFree(prms->edge_list, prms->edge_list_size * 4u);
            prms->edge_list = NULL;
        }

        tivxMemFree(prms, sizeof(tivxCannyParams));
    }
}

static vx_status tivxCannyCalcSobel(tivxCannyParams *prms,
    uint8_t *src_addr, int32_t gs)
{
    vx_status status = VX_FAILURE;

    int16_t *temp1_16, *temp2_16;

    if (NULL != prms)
    {
        switch(gs)
        {
            case 3:
                temp1_16 = (int16_t *)((uintptr_t)prms->sobel_x +
                    prms->vxlib_sobx.stride_y + 2u);

                temp2_16 = (int16_t *)((uintptr_t)prms->sobel_y +
                    prms->vxlib_soby.stride_y + 2u);

                status = VXLIB_sobel_3x3_i8u_o16s_o16s(src_addr,
                    &prms->vxlib_src, temp1_16, &prms->vxlib_sobx,
                    temp2_16, &prms->vxlib_soby);
                break;
            case 5:
                temp1_16 = (int16_t *)((uintptr_t)prms->sobel_x +
                    prms->vxlib_sobx.stride_y*2u + 4u);

                temp2_16 = (int16_t *)((uintptr_t)prms->sobel_y +
                    prms->vxlib_soby.stride_y*2u + 4u);

                status = VXLIB_sobel_5x5_i8u_o16s_o16s(src_addr,
                    &prms->vxlib_src, temp1_16, &prms->vxlib_sobx,
                    temp2_16, &prms->vxlib_soby);
                break;
            case 7:
            default:
                temp1_16 = (int16_t *)((uintptr_t)prms->sobel_x +
                    prms->vxlib_sobx.stride_y*3u + 6u);

                temp2_16 = (int16_t *)((uintptr_t)prms->sobel_y +
                    prms->vxlib_soby.stride_y*3u + 6u);

                status = VXLIB_sobel_7x7_i8u_o16s_o16s(src_addr,
                    &prms->vxlib_src, temp1_16, &prms->vxlib_sobx,
                    temp2_16, &prms->vxlib_soby);
                break;
        }
    }

    if (status != VXLIB_SUCCESS)
    {
        status = VX_FAILURE;
    }

    return (status);
}

static vx_status tivxCannyCalcNorm(tivxCannyParams *prms, vx_enum norm_enm,
    int32_t gs)
{
    vx_status status = VX_FAILURE;
    uint16_t *norm;
    int16_t *sobx, *soby;

    if (NULL != prms)
    {
        sobx = (int16_t *)((uintptr_t)prms->sobel_x + (prms->vxlib_sobx.stride_y *
            (gs / 2u)) + ((gs / 2u) * 2u));
        soby = (int16_t *)((uintptr_t)prms->sobel_y + (prms->vxlib_soby.stride_y *
            (gs / 2u)) + ((gs / 2u) * 2u));
        norm = (uint16_t *)((uintptr_t)prms->norm + (prms->vxlib_norm.stride_y *
            (gs / 2u)) + ((gs / 2u) * 2u));

        if (VX_NORM_L1 == norm_enm)
        {
            status = VXLIB_normL1_i16s_i16s_o16u(sobx, &prms->vxlib_sobx,
                soby, &prms->vxlib_soby, norm, &prms->vxlib_norm);
        }
        else
        {
            status = VXLIB_normL2_i16s_i16s_o16u(sobx, &prms->vxlib_sobx,
                soby, &prms->vxlib_soby, norm, &prms->vxlib_norm);
        }
    }

    if (status != VXLIB_SUCCESS)
    {
        status = VX_FAILURE;
    }

    return (status);
}

static vx_status tivxCannyCalcNms(tivxCannyParams *prms, int32_t gs)
{
    vx_status status = VX_FAILURE;
    int16_t *sobx, *soby;
    uint16_t *norm;
    uint8_t *edge;

    if (NULL != prms)
    {
        sobx = (int16_t *)((uintptr_t)prms->sobel_x + (prms->vxlib_sobx.stride_y *
            (gs / 2u)) + ((gs / 2u) * 2u));
        soby = (int16_t *)((uintptr_t)prms->sobel_y + (prms->vxlib_soby.stride_y *
            (gs / 2u)) + ((gs / 2u) * 2u));
        norm = (uint16_t *)((uintptr_t)prms->norm + (prms->vxlib_norm.stride_y *
            (gs / 2u)) + ((gs / 2u) * 2u));
        edge = (uint8_t *) ((uintptr_t)prms->nms_edge + (prms->vxlib_edge.stride_y *
            ((gs / 2u) + 1u)) + (gs / 2u) + 1u);

        status = VXLIB_cannyNMS_i16s_i16s_i16u_o8u(sobx, &prms->vxlib_sobx,
                soby, &prms->vxlib_soby, norm, &prms->vxlib_norm, edge,
                &prms->vxlib_edge);
    }
    if (status != VXLIB_SUCCESS)
    {
        status = VX_FAILURE;
    }

    return (status);
}

static vx_status tivxCannyCalcDblThr(tivxCannyParams *prms,
    uint32_t *num_items, int32_t lower, int32_t upper, int32_t gs)
{
    vx_status status = VX_FAILURE;
    uint32_t start_pos;
    uint16_t *norm;
    uint8_t *edge;
    VXLIB_bufParams2D_t vxlib_prms;

    if (NULL != prms)
    {
        start_pos = (prms->vxlib_edge.stride_y * ((gs / 2u) + 1u)) +
            ((gs / 2u) +1u);
        norm = (uint16_t*)((uintptr_t)prms->norm + (prms->vxlib_norm.stride_y *
            ((gs/2u)+1u)) + (((gs/2u)+1u)*2u));
        edge = (uint8_t*)((uintptr_t)prms->nms_edge + start_pos);

        vxlib_prms.dim_x = prms->vxlib_edge.dim_x - gs;
        vxlib_prms.dim_y = prms->vxlib_edge.dim_y;
        vxlib_prms.stride_y = prms->vxlib_edge.stride_y;
        vxlib_prms.data_type = prms->vxlib_edge.data_type;

        status = VXLIB_doubleThreshold_i16u_i8u(norm, &prms->vxlib_norm,
            edge, &vxlib_prms, prms->edge_list, prms->edge_list_size,
            num_items, start_pos, lower, upper);
    }
    if (status != VXLIB_SUCCESS)
    {
        status = VX_FAILURE;
    }

    return (status);
}
