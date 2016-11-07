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

static tivx_target_kernel vx_lut_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelLutProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params)
{
    vx_status status = VX_SUCCESS;
    uint32_t x, y;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_lut_t *lut;
    uint8_t *s8, *d8, *l8;
    int16_t *s16, *d16, *l16;
    uint32_t temp;

    if ((num_params != 3U) || (NULL == obj_desc[0]) || (NULL == obj_desc[1])
        || (NULL == obj_desc[2]))
    {
        status = VX_FAILURE;
    }
    else
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_LUT_IN_IMG_IDX];
        lut = (tivx_obj_desc_lut_t *)obj_desc[TIVX_KERNEL_LUT_IN_LUT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_LUT_OUT_IMG_IDX];

        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);
        dst->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_type);
        lut->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
            lut->mem_ptr.shared_ptr, lut->mem_ptr.mem_type);


        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        tivxMemBufferMap(lut->mem_ptr.target_ptr, lut->mem_size,
            lut->mem_ptr.mem_type, VX_WRITE_ONLY);

        if (src->format == VX_DF_IMAGE_U8)
        {
            s8 = (uint8_t *)src->mem_ptr[0].target_ptr;
            d8 = (uint8_t *)dst->mem_ptr[0].target_ptr;
            l8 = (uint8_t *)lut->mem_ptr.target_ptr;

            for (y = 0; (y < src->height) && (status == VX_SUCCESS); y++)
            {
                for (x = 0; x < src->width; x++)
                {
                    temp = ((uint32_t)(*s8) & 0xff);
                    if (temp < lut->num_items)
                    {
                        *d8 = l8[(*s8)];
                    }

                    s8 ++;
                    d8 ++;
                }
            }
        }
        else if (src->format == VX_DF_IMAGE_S16)
        {
            s16 = (int16_t *)src->mem_ptr[0].target_ptr;
            d16 = (int16_t *)dst->mem_ptr[0].target_ptr;
            l16 = (int16_t *)lut->mem_ptr.target_ptr;

            for (y = 0; (y < src->height) && (status == VX_SUCCESS); y++)
            {
                for (x = 0; x < src->width; x++)
                {
                    temp = ((uint32_t)(*s16) & 0xFFFF);
                    if (temp < lut->num_items)
                    {
                        *d16 = l16[32768 + (*s16)];
                    }

                    s16 ++;
                    d16 ++;
                }
            }
        }

        tivxMemBufferUnmap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        tivxMemBufferUnmap(lut->mem_ptr.target_ptr, lut->mem_size,
            lut->mem_ptr.mem_type, VX_WRITE_ONLY);
    }

    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelLutCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelLutDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelLutControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params)
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
            tivxKernelLutControl);
    }
}


void tivxRemoveTargetKernelLut()
{
    tivxRemoveTargetKernel(vx_lut_target_kernel);
}
