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
#include <tivx_kernel_fast_corners.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_fast_corners_target_kernel = NULL;

typedef struct
{
    uint32_t *corners;
    uint8_t  *strength;
    uint8_t  *scratch;
    uint32_t scratch_size;
} tivxFastCornersParams;

static vx_status VX_CALLBACK tivxKernelFastCProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivxFastCornersParams *prms = NULL;
    tivx_obj_desc_image_t *src;
    tivx_obj_desc_array_t *arr;
    uint8_t *src_addr;
    VXLIB_bufParams2D_t vxlib_src;
    uint32_t size, num_corners;
    tivx_obj_desc_scalar_t *sc_thr, *sc_nms, *sc_cnt;
    vx_keypoint_t *kp;

    if (num_params != TIVX_KERNEL_FASTC_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_FASTC_MAX_PARAMS; i ++)
        {
            if ((NULL == obj_desc[i]) &&
                (i != TIVX_KERNEL_FASTC_OUT_SC_CNT_IDX))
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_FASTC_IN_IMG_IDX];
        arr = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_FASTC_OUT_ARR_IDX];
        sc_thr = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_FASTC_IN_SC_THR_IDX];
        sc_nms = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_FASTC_IN_NMS_IDX];
        sc_cnt = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_FASTC_OUT_SC_CNT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxFastCornersParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);
        arr->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
            arr->mem_ptr.shared_ptr, arr->mem_ptr.mem_type);

        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(arr->mem_ptr.target_ptr, arr->mem_size,
            arr->mem_ptr.mem_type, VX_WRITE_ONLY);

        /* Valid rectangle is ignore here */
        src_addr = (uint8_t *)((uintptr_t)src->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(0, 0, &src->imagepatch_addr[0U]));

        vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
        vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
        vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
        vxlib_src.data_type = VXLIB_UINT8;

        status = VXLIB_fastCorners_i8u(
            src_addr, &vxlib_src, prms->corners, prms->strength,
            arr->capacity, (uint8_t)sc_thr->data.f32, &num_corners,
            sc_nms->data.boolean, prms->scratch, prms->scratch_size);

        if (status != VXLIB_SUCCESS)
        {
            status = VX_FAILURE;
        }
        else
        {
            if (NULL != sc_cnt)
            {
                sc_cnt->data.u32 = num_corners;
            }

            /* Copy array and other parameters */
            if (num_corners > arr->capacity)
            {
                num_corners = arr->capacity;
            }
            arr->num_items = num_corners;

            kp = (vx_keypoint_t *)arr->mem_ptr.target_ptr;
            for (i = 0; i < num_corners; i ++)
            {
                kp->x = prms->corners[i] & 0xFFFF;
                kp->y = (prms->corners[i] & 0xFFFF0000) >> 16u;
                kp->strength = prms->strength[i];
                kp->scale = 0.0f;
                kp->orientation = 0.0f;
                kp->tracking_status = 1;
                kp->error = 0.0f;

                kp ++;
            }
        }

        tivxMemBufferUnmap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(arr->mem_ptr.target_ptr, arr->mem_size,
            arr->mem_ptr.mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelFastCCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivx_obj_desc_image_t *img;
    tivx_obj_desc_array_t *arr;
    tivxFastCornersParams *prms = NULL;

    if (num_params != TIVX_KERNEL_FASTC_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_FASTC_MAX_PARAMS; i ++)
        {
            if ((NULL == obj_desc[i]) &&
                (i != TIVX_KERNEL_FASTC_OUT_SC_CNT_IDX))
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        img = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_FASTC_IN_IMG_IDX];
        arr = (tivx_obj_desc_array_t *)obj_desc[
            TIVX_KERNEL_FASTC_OUT_ARR_IDX];

        prms = tivxMemAlloc(sizeof(tivxFastCornersParams));
        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxFastCornersParams));

            prms->corners = tivxMemAlloc(arr->capacity * 4U);
            if (NULL == prms->corners)
            {
                status = VX_ERROR_NO_MEMORY;
            }

            if (VX_SUCCESS == status)
            {
                prms->strength = tivxMemAlloc(arr->capacity);
                if (NULL == prms->strength)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if (VX_SUCCESS == status)
            {
                if ((img->imagepatch_addr[0].dim_x*4u + arr->capacity + 30u) >
                        512u)
                {
                    size = img->imagepatch_addr[0].dim_x*4u +
                        arr->capacity + 30u;
                }
                else
                {
                    size = 512u;
                }

                prms->scratch_size = size;

                prms->scratch = tivxMemAlloc(size);
                if (NULL == prms->scratch)
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
                sizeof(tivxFastCornersParams));
        }
        else
        {
            if (NULL != prms)
            {
                if (prms->corners)
                {
                    tivxMemFree(prms->corners, arr->capacity * 4U);
                    prms->corners = NULL;
                }
                if (prms->strength)
                {
                    tivxMemFree(prms->strength, arr->capacity);
                    prms->strength = NULL;
                }
                if (prms->scratch)
                {
                    tivxMemFree(prms->scratch, prms->scratch_size);
                    prms->scratch = NULL;
                }

                tivxMemFree(prms, sizeof(tivxFastCornersParams));
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelFastCDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivx_obj_desc_array_t *arr;
    tivxFastCornersParams *prms = NULL;

    if (num_params != TIVX_KERNEL_FASTC_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_FASTC_MAX_PARAMS; i ++)
        {
            if ((NULL == obj_desc[i]) &&
                (i != TIVX_KERNEL_FASTC_OUT_SC_CNT_IDX))
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        arr = (tivx_obj_desc_array_t *)obj_desc[
            TIVX_KERNEL_FASTC_OUT_ARR_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxFastCornersParams) == size))
        {
            if (prms->corners)
            {
                tivxMemFree(prms->corners, arr->capacity * 4U);
                prms->corners = NULL;
            }
            if (prms->strength)
            {
                tivxMemFree(prms->strength, arr->capacity);
                prms->strength = NULL;
            }
            if (prms->scratch)
            {
                tivxMemFree(prms->scratch, prms->scratch_size);
                prms->scratch = NULL;
                prms->scratch_size = 0u;
            }

            tivxMemFree(prms, size);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelFastCControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelFastCorners()
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

        vx_fast_corners_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_FAST_CORNERS,
            target_name,
            tivxKernelFastCProcess,
            tivxKernelFastCCreate,
            tivxKernelFastCDelete,
            tivxKernelFastCControl,
            NULL);
    }
}


void tivxRemoveTargetKernelFastCorners()
{
    tivxRemoveTargetKernel(vx_fast_corners_target_kernel);
}
