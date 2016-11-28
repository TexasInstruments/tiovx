/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_lut.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_lut_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelLutProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_lut_t *lut;
    vx_uint8 *src_addr, *dst_addr;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    vx_rectangle_t rect;

    if (num_params != TIVX_KERNEL_LUT_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0; i < TIVX_KERNEL_LUT_MAX_PARAMS; i ++)
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
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_LUT_IN_IMG_IDX];
        lut = (tivx_obj_desc_lut_t *)obj_desc[TIVX_KERNEL_LUT_IN_LUT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_LUT_OUT_IMG_IDX];

        src->mem_ptr[0U].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0U].shared_ptr, src->mem_ptr[0U].mem_type);
        dst->mem_ptr[0U].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0U].shared_ptr, dst->mem_ptr[0U].mem_type);
        lut->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
            lut->mem_ptr.shared_ptr, lut->mem_ptr.mem_type);

        tivxMemBufferMap(src->mem_ptr[0U].target_ptr, src->mem_size[0],
            src->mem_ptr[0U].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(lut->mem_ptr.target_ptr, lut->mem_size,
            lut->mem_ptr.mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst->mem_ptr[0U].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0U].mem_type, VX_WRITE_ONLY);

        /* Get the correct offset of the images from teh valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uint32_t)src->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));
        /* TODO: Do we require to move pointer even for destination image */
        dst_addr = (uint8_t *)((uint32_t)dst->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0]));

        vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
        vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
        vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;

        if (src->format == VX_DF_IMAGE_U8)
        {
            vxlib_src.data_type = VXLIB_UINT8;
        }
        else
        {
            vxlib_src.data_type = VXLIB_INT16;
        }

        vxlib_dst.dim_x = dst->imagepatch_addr[0].dim_x;
        vxlib_dst.dim_y = dst->imagepatch_addr[0].dim_y;
        vxlib_dst.stride_y = dst->imagepatch_addr[0].stride_y;
        vxlib_dst.data_type = vxlib_src.data_type;

        if (src->format == VX_DF_IMAGE_U8)
        {
            status = VXLIB_tableLookup_i8u_o8u(src_addr, &vxlib_src,
                dst_addr, &vxlib_dst, lut->mem_ptr.target_ptr, lut->num_items);
        }
        else
        {
            status = VXLIB_tableLookup_i16s_o16s((int16_t *)src_addr,
                &vxlib_src, (int16_t *)dst_addr, &vxlib_dst,
                lut->mem_ptr.target_ptr, lut->num_items, 32768U);
        }

        tivxMemBufferUnmap(dst->mem_ptr[0U].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0U].mem_type, VX_WRITE_ONLY);
    }

    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelLutCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelLutDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelLutControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelLut()
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

        vx_lut_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_TABLE_LOOKUP,
            target_name,
            tivxKernelLutProcess,
            tivxKernelLutCreate,
            tivxKernelLutDelete,
            tivxKernelLutControl,
            NULL);
    }
}


void tivxRemoveTargetKernelLut()
{
    tivxRemoveTargetKernel(vx_lut_target_kernel);
}
