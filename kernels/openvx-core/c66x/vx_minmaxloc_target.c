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
#include <tivx_kernel_minmaxloc.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_mml_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelMmlProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src;
    vx_uint8 *src_addr;
    VXLIB_bufParams2D_t vxlib_src;
    vx_rectangle_t rect;
    tivx_obj_desc_scalar_t *sc[4U];
    tivx_obj_desc_array_t *arr[2U];
    uint32_t min_cnt = 0, max_cnt = 0, min_cap = 0, max_cap = 0;
    uint32_t *min_loc = NULL, *max_loc = NULL;

    if (num_params != TIVX_KERNEL_MML_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        if ((NULL == obj_desc[TIVX_KERNEL_MML_IN_IMG_IDX]) ||
            (NULL == obj_desc[TIVX_KERNEL_MML_OUT_MIN_SC_IDX]) ||
            (NULL == obj_desc[TIVX_KERNEL_MML_OUT_MAX_SC_IDX]))
        {
            status = VX_ERROR_NO_MEMORY;
        }
    }

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_MML_IN_IMG_IDX];
        sc[0U] = (tivx_obj_desc_scalar_t*)obj_desc[TIVX_KERNEL_MML_OUT_MIN_SC_IDX];
        sc[1U] = (tivx_obj_desc_scalar_t*)obj_desc[TIVX_KERNEL_MML_OUT_MAX_SC_IDX];
        sc[2U] = (tivx_obj_desc_scalar_t*)obj_desc[TIVX_KERNEL_MML_OUT_MIN_SC_C_IDX];
        sc[3U] = (tivx_obj_desc_scalar_t*)obj_desc[TIVX_KERNEL_MML_OUT_MAX_SC_C_IDX];
        arr[0U] = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_MML_OUT_MIN_ARR_IDX];
        arr[1U] = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_MML_OUT_MAX_ARR_IDX];

        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);
        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);

        if (NULL != arr[0u])
        {
            arr[0U]->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
                arr[0U]->mem_ptr.shared_ptr, arr[0U]->mem_ptr.mem_type);
            tivxMemBufferMap(arr[0U]->mem_ptr.target_ptr, arr[0U]->mem_size,
                arr[0U]->mem_ptr.mem_type, VX_WRITE_ONLY);

            min_loc = arr[0U]->mem_ptr.target_ptr;
            min_cap = arr[0U]->mem_size / arr[0u]->item_size;
        }

        if (NULL != arr[1u])
        {
            arr[1U]->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
                arr[1U]->mem_ptr.shared_ptr, arr[1U]->mem_ptr.mem_type);
            tivxMemBufferMap(arr[1U]->mem_ptr.target_ptr, arr[1U]->mem_size,
                arr[1U]->mem_ptr.mem_type, VX_WRITE_ONLY);

            max_loc = arr[1U]->mem_ptr.target_ptr;
            max_cap = arr[1U]->mem_size / arr[1u]->item_size;
        }


        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uint32_t)src->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));

        vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
        vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
        vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
        if (VX_DF_IMAGE_U8 == src->format)
        {
            vxlib_src.data_type = VXLIB_UINT8;
        }
        else
        {
            vxlib_src.data_type = VXLIB_INT16;
        }

        if (VX_DF_IMAGE_U8 == src->format)
        {
            sc[0U]->data.u08 = 0xFF;
            sc[1U]->data.u08 = 0;
            status = VXLIB_minMaxLoc_i8u(src_addr, &vxlib_src,
                &sc[0U]->data.u08, &sc[1U]->data.u08,
                &min_cnt, &max_cnt,
                min_cap, max_cap,
                min_loc, max_loc, 0, 0);
        }
        else
        {
            sc[0U]->data.s16 = 0x7fff;
            sc[1U]->data.s16 = 0x8000;
            status = VXLIB_minMaxLoc_i16s((int16_t*)src_addr, &vxlib_src,
                &sc[0U]->data.s16, &sc[1U]->data.s16,
                &min_cnt, &max_cnt,
                min_cap, max_cap,
                min_loc, max_loc, 0, 0);
        }
        if (status != VXLIB_SUCCESS)
        {
            status = VX_FAILURE;
        }

        if (NULL != sc[2U])
        {
             sc[2U]->data.u32 = min_cnt;
        }
        if (NULL != sc[3U])
        {
             sc[3U]->data.u32 = max_cnt;
        }
        if (NULL != arr[0u])
        {
            if (min_cnt > min_cap)
            {
                arr[0u]->num_items = min_cap;
            }
            else
            {
                arr[0u]->num_items = min_cnt;
            }
        }
        if (NULL != arr[1u])
        {
            if (max_cnt > max_cap)
            {
                arr[1u]->num_items = max_cap;
            }
            else
            {
                arr[1u]->num_items = max_cnt;
            }
        }

        tivxMemBufferUnmap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        if (NULL != arr[0u])
        {
            tivxMemBufferUnmap(arr[0U]->mem_ptr.target_ptr, arr[0U]->mem_size,
                arr[0U]->mem_ptr.mem_type, VX_WRITE_ONLY);
        }
        if (NULL != arr[1u])
        {
            tivxMemBufferUnmap(arr[1U]->mem_ptr.target_ptr, arr[1U]->mem_size,
                arr[1U]->mem_ptr.mem_type, VX_WRITE_ONLY);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelMmlCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelMmlDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelMmlControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelMinMaxLoc()
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

        vx_mml_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_MINMAXLOC,
            target_name,
            tivxKernelMmlProcess,
            tivxKernelMmlCreate,
            tivxKernelMmlDelete,
            tivxKernelMmlControl,
            NULL);
    }
}


void tivxRemoveTargetKernelMinMaxLoc()
{
    tivxRemoveTargetKernel(vx_mml_target_kernel);
}
