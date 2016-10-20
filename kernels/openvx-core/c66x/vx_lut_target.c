/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
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

    if ((num_params != 3U) || (NULL == obj_desc[0]) || (NULL == obj_desc[1])
        || (NULL == obj_desc[2]))
    {
        status = VX_FAILURE;
    }
    else
    {
        src = (tivx_obj_desc_image_t *)tivxObjDescGet(obj_desc[0U]->obj_desc_id);
        lut = (tivx_obj_desc_lut_t *)tivxObjDescGet(obj_desc[1U]->obj_desc_id);
        dst = (tivx_obj_desc_image_t *)tivxObjDescGet(obj_desc[2U]->obj_desc_id);

        tivxMemBufferMap(src->mem_ptr[0].host_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        if (src->format == VX_DF_IMAGE_U8)
        {
            s8 = (uint8_t *)src->mem_ptr[0].host_ptr;
            d8 = (uint8_t *)dst->mem_ptr[0].host_ptr;
            l8 = (uint8_t *)lut->mem_ptr.host_ptr;

            for (y = 0; (y < src->height) && (status == VX_SUCCESS); y++)
            {
                for (x = 0; x < src->width; x++)
                {
                    if ((*s8) < lut->num_items)
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
            s16 = (int16_t *)src->mem_ptr[0].host_ptr;
            d16 = (int16_t *)dst->mem_ptr[0].host_ptr;
            l16 = (int16_t *)lut->mem_ptr.host_ptr;

            for (y = 0; (y < src->height) && (status == VX_SUCCESS); y++)
            {
                for (x = 0; x < src->width; x++)
                {
                    if (((unsigned short)(*s16)) < lut->num_items)
                    {
                        *d16 = l16[32768 + (*s16)];
                    }
                    s16 ++;
                    d16 ++;
                }
            }
        }

        tivxMemBufferUnmap(dst->mem_ptr[0].host_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);

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
    vx_lut_target_kernel = tivxAddTargetKernel(
        VX_KERNEL_TABLE_LOOKUP,
        TIVX_TARGET_DSP1,
        tivxKernelLutProcess,
        tivxKernelLutCreate,
        tivxKernelLutDelete,
        tivxKernelLutControl);
}


void tivxRemoveTargetKernelLut()
{
    tivxRemoveTargetKernel(vx_lut_target_kernel);
}
