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
#include <TI/tivx_obj_desc.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_target_kernels_utils.h>

void ownInitBufParams(
    tivx_obj_desc_image_t *obj_desc,
    vx_rectangle_t *rect,
    VXLIB_bufParams2D_t buf_params[],
    uint8_t *addr[],
    uint32_t lpad, uint32_t tpad, uint32_t rpad, uint32_t bpad)
{
    uint32_t i;

    if (NULL == rect)
    {
        rect = &obj_desc->valid_roi;
    }

    if ((rect->start_x == 0u) && (lpad != 0u))
    {
        lpad = 0;
    }
    if ((rect->start_y == 0u) && (tpad != 0u))
    {
        tpad = 0;
    }
    if ((rect->end_x == obj_desc->imagepatch_addr[0].dim_x) && (rpad != 0u))
    {
        rpad = 0;
    }
    if ((rect->end_y == obj_desc->imagepatch_addr[0].dim_y) && (bpad != 0u))
    {
        bpad = 0;
    }

    for (i = 0; i < obj_desc->planes; i ++)
    {
        buf_params[i].dim_x = (rect->end_x + rpad) - (rect->start_x - lpad);
        buf_params[i].dim_y = (rect->end_y + bpad) - (rect->start_y - tpad);
        buf_params[i].stride_y = obj_desc->imagepatch_addr[i].stride_y;

        if (512 == obj_desc->imagepatch_addr[1].scale_x)
        {
            buf_params[i].dim_x = buf_params[i].dim_x / 2;
            buf_params[i].dim_y = buf_params[i].dim_y / 2;
        }

        switch(obj_desc->format)
        {
            case VX_DF_IMAGE_NV12:
            case VX_DF_IMAGE_NV21:
            case VX_DF_IMAGE_UYVY:
            case VX_DF_IMAGE_YUYV:
            case VX_DF_IMAGE_IYUV:
            case VX_DF_IMAGE_YUV4:
            case VX_DF_IMAGE_U8:
                buf_params[i].data_type = VXLIB_UINT8;
                break;
            case VX_DF_IMAGE_U16:
                buf_params[i].data_type = VXLIB_UINT16;
                break;
            case VX_DF_IMAGE_S16:
                buf_params[i].data_type = VXLIB_INT16;
                break;
            case VX_DF_IMAGE_RGBX:
            case VX_DF_IMAGE_U32:
                buf_params[i].data_type = VXLIB_UINT32;
                break;
            case VX_DF_IMAGE_S32:
                buf_params[i].data_type = VXLIB_INT32;
                break;
            case VX_DF_IMAGE_RGB:
                buf_params[i].data_type = VXLIB_UINT24;
                break;
        }

        addr[i] = (uint8_t *)((uintptr_t)obj_desc->mem_ptr[i].target_ptr +
            ownComputePatchOffset(obj_desc->valid_roi.start_x,
            obj_desc->valid_roi.start_y,
            &obj_desc->imagepatch_addr[i]));
    }
}
