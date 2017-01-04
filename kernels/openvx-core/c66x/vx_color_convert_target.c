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
#include <tivx_kernel_color_convert.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_color_convert_target_kernel = NULL;

static vx_status tivxKernelColorConvert(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    vx_enum kern_type)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc, *dst_desc;
    uint32_t i;
    void *src_addr[4], *dst_addr[4];
    vx_rectangle_t rect;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    VXLIB_bufParams2D_t vxlib_src1, vxlib_dst1;
    VXLIB_bufParams2D_t vxlib_src2, vxlib_dst2;
    void *scratch;
    uint32_t scratch_size;

    if (num_params != TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS; i ++)
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
        src_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_COLOR_CONVERT_IN_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_COLOR_CONVERT_OUT_IMG_IDX];

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src image */
        rect = src_desc->valid_roi;

        for (i = 0; i < src_desc->planes; i++)
        {
            src_desc->mem_ptr[i].target_ptr = tivxMemShared2TargetPtr(
                src_desc->mem_ptr[i].shared_ptr, src_desc->mem_ptr[i].mem_type);
            tivxMemBufferMap(src_desc->mem_ptr[i].target_ptr, src_desc->mem_size[i],
                src_desc->mem_ptr[i].mem_type, VX_READ_ONLY);

            src_addr[i] = (uint8_t *)((uint32_t)src_desc->mem_ptr[i].target_ptr +
                ownComputePatchOffset(rect.start_x, rect.start_y,
                &src_desc->imagepatch_addr[i]));
        }

        for (i = 0; i < dst_desc->planes; i++)
        {
            dst_desc->mem_ptr[i].target_ptr = tivxMemShared2TargetPtr(
                dst_desc->mem_ptr[i].shared_ptr, dst_desc->mem_ptr[i].mem_type);
            tivxMemBufferMap(dst_desc->mem_ptr[i].target_ptr, dst_desc->mem_size[i],
                dst_desc->mem_ptr[i].mem_type, VX_WRITE_ONLY);

            /* TODO: Do we require to move pointer even for destination image */
            dst_addr[i] = (uint8_t *)((uint32_t)dst_desc->mem_ptr[i].target_ptr +
                ownComputePatchOffset(rect.start_x, rect.start_y,
                &dst_desc->imagepatch_addr[i]));
        }

        vxlib_src.dim_x = src_desc->imagepatch_addr[0].dim_x;
        vxlib_src.dim_y = src_desc->imagepatch_addr[0].dim_y;
        vxlib_src.stride_y = src_desc->imagepatch_addr[0].stride_y;
        vxlib_src.data_type = VXLIB_UINT8;

        vxlib_src1.dim_x = src_desc->imagepatch_addr[1].dim_x;
        vxlib_src1.dim_y = src_desc->imagepatch_addr[1].dim_y;
        vxlib_src1.stride_y = src_desc->imagepatch_addr[1].stride_y;
        vxlib_src1.data_type = VXLIB_UINT8;

        if (512 == src_desc->imagepatch_addr[1].scale_x)
        {
            vxlib_src1.dim_x = src_desc->imagepatch_addr[1].dim_x / 2;
            vxlib_src1.dim_y = src_desc->imagepatch_addr[1].dim_y / 2;
        }

        vxlib_src2.dim_x = src_desc->imagepatch_addr[2].dim_x;
        vxlib_src2.dim_y = src_desc->imagepatch_addr[2].dim_y;
        vxlib_src2.stride_y = src_desc->imagepatch_addr[2].stride_y;
        vxlib_src2.data_type = VXLIB_UINT8;

        if (512 == src_desc->imagepatch_addr[2].scale_x)
        {
            vxlib_src2.dim_x = src_desc->imagepatch_addr[2].dim_x / 2;
            vxlib_src2.dim_y = src_desc->imagepatch_addr[2].dim_y / 2;
        }

        vxlib_dst.dim_x = dst_desc->imagepatch_addr[0].dim_x;
        vxlib_dst.dim_y = dst_desc->imagepatch_addr[0].dim_y;
        vxlib_dst.stride_y = dst_desc->imagepatch_addr[0].stride_y;
        vxlib_dst.data_type = VXLIB_UINT8;

        vxlib_dst1.dim_x = dst_desc->imagepatch_addr[1].dim_x;
        vxlib_dst1.dim_y = dst_desc->imagepatch_addr[1].dim_y;
        vxlib_dst1.stride_y = dst_desc->imagepatch_addr[1].stride_y;
        vxlib_dst1.data_type = VXLIB_UINT8;

        if (512 == dst_desc->imagepatch_addr[1].scale_x)
        {
            vxlib_dst1.dim_x = dst_desc->imagepatch_addr[1].dim_x / 2;
            vxlib_dst1.dim_y = dst_desc->imagepatch_addr[1].dim_y / 2;
        }

        vxlib_dst2.dim_x = dst_desc->imagepatch_addr[2].dim_x;
        vxlib_dst2.dim_y = dst_desc->imagepatch_addr[2].dim_y;
        vxlib_dst2.stride_y = dst_desc->imagepatch_addr[2].stride_y;
        vxlib_dst2.data_type = VXLIB_UINT8;

        if (512 == dst_desc->imagepatch_addr[2].scale_x)
        {
            vxlib_dst2.dim_x = dst_desc->imagepatch_addr[2].dim_x / 2;
            vxlib_dst2.dim_y = dst_desc->imagepatch_addr[2].dim_y / 2;
        }

        if ((VX_DF_IMAGE_RGB == src_desc->format) && (VX_DF_IMAGE_RGBX == dst_desc->format))
        {
            status = VXLIB_colorConvert_RGBtoRGBX_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst);
        }
        else if ((VX_DF_IMAGE_RGB == src_desc->format) && (VX_DF_IMAGE_NV12 == dst_desc->format))
        {
            status = tivxGetTargetKernelInstanceContext(kernel, &scratch, &scratch_size);

            vxlib_dst1.dim_x = dst_desc->imagepatch_addr[1].dim_x;
            if (VX_SUCCESS == status)
            {
                status = VXLIB_colorConvert_RGBtoNV12_i8u_o8u((uint8_t *)src_addr[0],
                    &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst, (uint8_t *)dst_addr[1], &vxlib_dst1,
                    scratch, scratch_size);
            }
        }
        else if ((VX_DF_IMAGE_RGB == src_desc->format) && (VX_DF_IMAGE_YUV4 == dst_desc->format))
        {
            status = VXLIB_colorConvert_RGBtoYUV4_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst, (uint8_t *)dst_addr[1], &vxlib_dst1,
                (uint8_t *)dst_addr[2], &vxlib_dst2);
        }
        else if ((VX_DF_IMAGE_RGB == src_desc->format) && (VX_DF_IMAGE_IYUV == dst_desc->format))
        {
            status = tivxGetTargetKernelInstanceContext(kernel, &scratch, &scratch_size);

            if (VX_SUCCESS == status)
            {
                status = VXLIB_colorConvert_RGBtoIYUV_i8u_o8u((uint8_t *)src_addr[0],
                    &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst, (uint8_t *)dst_addr[1],
                    &vxlib_dst1, (uint8_t *)dst_addr[2], &vxlib_dst2, scratch, scratch_size);
            }
        }
        else if ((VX_DF_IMAGE_RGBX == src_desc->format) && (VX_DF_IMAGE_RGB == dst_desc->format))
        {
            status = VXLIB_colorConvert_RGBXtoRGB_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst);
        }
        else if ((VX_DF_IMAGE_RGBX == src_desc->format) && (VX_DF_IMAGE_NV12 == dst_desc->format))
        {
            status = tivxGetTargetKernelInstanceContext(kernel, &scratch, &scratch_size);

            vxlib_dst1.dim_x = dst_desc->imagepatch_addr[1].dim_x;
            if (VX_SUCCESS == status)
            {
                status = VXLIB_colorConvert_RGBXtoNV12_i8u_o8u((uint8_t *)src_addr[0],
                    &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst, (uint8_t *)dst_addr[1],
                    &vxlib_dst1, scratch, scratch_size);
            }
        }
        else if ((VX_DF_IMAGE_RGBX == src_desc->format) && (VX_DF_IMAGE_YUV4 == dst_desc->format))
        {
            status = VXLIB_colorConvert_RGBXtoYUV4_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst, (uint8_t *)dst_addr[1],
                &vxlib_dst1, (uint8_t *)dst_addr[2], &vxlib_dst2);
        }
        else if ((VX_DF_IMAGE_RGBX == src_desc->format) && (VX_DF_IMAGE_IYUV == dst_desc->format))
        {
            status = tivxGetTargetKernelInstanceContext(kernel, &scratch, &scratch_size);

            if (VX_SUCCESS == status)
            {
                status = VXLIB_colorConvert_RGBXtoIYUV_i8u_o8u((uint8_t *)src_addr[0],
                    &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst, (uint8_t *)dst_addr[1],
                    &vxlib_dst1, (uint8_t *)dst_addr[2], &vxlib_dst2, scratch, scratch_size);
            }
        }
        else if ( ( (VX_DF_IMAGE_NV12 == src_desc->format) || (VX_DF_IMAGE_NV21 == src_desc->format) ) &&
                 (VX_DF_IMAGE_RGB == dst_desc->format))
        {
            int u_pix = src_desc->format == VX_DF_IMAGE_NV12 ? 0 : 1;

            vxlib_src1.dim_x = src_desc->imagepatch_addr[1].dim_x;
            status = VXLIB_colorConvert_NVXXtoRGB_i8u_o8u((uint8_t *)src_addr[0], &vxlib_src,
                (uint8_t *)src_addr[1], &vxlib_src1, (uint8_t *)dst_addr[0], &vxlib_dst, u_pix,
                src_desc->color_space - VX_ENUM_BASE(VX_ID_KHRONOS, VX_ENUM_COLOR_SPACE));
        }
        else if ( ( (VX_DF_IMAGE_NV12 == src_desc->format) || (VX_DF_IMAGE_NV21 == src_desc->format) ) &&
                 (VX_DF_IMAGE_RGBX == dst_desc->format))
        {
            int u_pix = src_desc->format == VX_DF_IMAGE_NV12 ? 0 : 1;

            vxlib_src1.dim_x = src_desc->imagepatch_addr[1].dim_x;
            status = VXLIB_colorConvert_NVXXtoRGBX_i8u_o8u((uint8_t *)src_addr[0], &vxlib_src,
                (uint8_t *)src_addr[1], &vxlib_src1, (uint8_t *)dst_addr[0],
                &vxlib_dst, u_pix, src_desc->color_space - VX_ENUM_BASE(VX_ID_KHRONOS, VX_ENUM_COLOR_SPACE));
        }
        else if ( ( (VX_DF_IMAGE_NV12 == src_desc->format) || (VX_DF_IMAGE_NV21 == src_desc->format) ) &&
                 (VX_DF_IMAGE_YUV4 == dst_desc->format))
        {
            int u_pix = src_desc->format == VX_DF_IMAGE_NV12 ? 0 : 1;

            vxlib_src1.dim_x = src_desc->imagepatch_addr[1].dim_x;
            status = VXLIB_colorConvert_NVXXtoYUV4_i8u_o8u((uint8_t *)src_addr[0], &vxlib_src,
                (uint8_t *)src_addr[1], &vxlib_src1, (uint8_t *)dst_addr[0], &vxlib_dst, (uint8_t *)dst_addr[1],
                &vxlib_dst1, (uint8_t *)dst_addr[2], &vxlib_dst2, u_pix);
        }
        else if ( ( (VX_DF_IMAGE_NV12 == src_desc->format) || (VX_DF_IMAGE_NV21 == src_desc->format) ) &&
                 (VX_DF_IMAGE_IYUV == dst_desc->format))
        {
            int u_pix = src_desc->format == VX_DF_IMAGE_NV12 ? 0 : 1;

            vxlib_src1.dim_x = src_desc->imagepatch_addr[1].dim_x;
            status = VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u((uint8_t *)src_addr[0], &vxlib_src,
                (uint8_t *)src_addr[1], &vxlib_src1, (uint8_t *)dst_addr[0], &vxlib_dst, (uint8_t *)dst_addr[1],
                &vxlib_dst1, (uint8_t *)dst_addr[2], &vxlib_dst2, u_pix);
        }
        else if ((VX_DF_IMAGE_YUYV == src_desc->format) && (VX_DF_IMAGE_RGB == dst_desc->format))
        {
            status = VXLIB_colorConvert_YUVXtoRGB_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst, 0,
                src_desc->color_space - VX_ENUM_BASE(VX_ID_KHRONOS, VX_ENUM_COLOR_SPACE));
        }
        else if ((VX_DF_IMAGE_YUYV == src_desc->format) && (VX_DF_IMAGE_RGBX == dst_desc->format))
        {
            status = VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst, 0,
                src_desc->color_space - VX_ENUM_BASE(VX_ID_KHRONOS, VX_ENUM_COLOR_SPACE));
        }
        else if ((VX_DF_IMAGE_YUYV == src_desc->format) && (VX_DF_IMAGE_NV12 == dst_desc->format))
        {
            vxlib_dst1.dim_x = dst_desc->imagepatch_addr[1].dim_x;
            status = VXLIB_colorConvert_YUVXtoNV12_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst, (uint8_t *)dst_addr[1],
                &vxlib_dst1, 0);
        }
        else if ((VX_DF_IMAGE_YUYV == src_desc->format) && (VX_DF_IMAGE_IYUV == dst_desc->format))
        {
            status = VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst, (uint8_t *)dst_addr[1], &vxlib_dst1,
                (uint8_t *)dst_addr[2], &vxlib_dst2, 0);
        }
        else if ((VX_DF_IMAGE_UYVY == src_desc->format) && (VX_DF_IMAGE_RGB == dst_desc->format))
        {
            status = VXLIB_colorConvert_YUVXtoRGB_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst, 1,
                src_desc->color_space - VX_ENUM_BASE(VX_ID_KHRONOS, VX_ENUM_COLOR_SPACE));
        }
        else if ((VX_DF_IMAGE_UYVY == src_desc->format) && (VX_DF_IMAGE_RGBX == dst_desc->format))
        {
            status = VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst, 1,
                src_desc->color_space - VX_ENUM_BASE(VX_ID_KHRONOS, VX_ENUM_COLOR_SPACE));
        }
        else if ((VX_DF_IMAGE_UYVY == src_desc->format) && (VX_DF_IMAGE_NV12 == dst_desc->format))
        {
            vxlib_dst1.dim_x = dst_desc->imagepatch_addr[1].dim_x;
            status = VXLIB_colorConvert_YUVXtoNV12_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst, (uint8_t *)dst_addr[1],
                &vxlib_dst1, 1);
        }
        else if ((VX_DF_IMAGE_UYVY == src_desc->format) && (VX_DF_IMAGE_IYUV == dst_desc->format))
        {
            status = VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src, (uint8_t *)dst_addr[0], &vxlib_dst, (uint8_t *)dst_addr[1],
                &vxlib_dst1, (uint8_t *)dst_addr[2], &vxlib_dst2, 1);
        }
        else if ((VX_DF_IMAGE_IYUV == src_desc->format) && (VX_DF_IMAGE_RGB == dst_desc->format))
        {
            status = VXLIB_colorConvert_IYUVtoRGB_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src, (uint8_t *)src_addr[1], &vxlib_src1, (uint8_t *)src_addr[2],
                &vxlib_src2, (uint8_t *)dst_addr[0], &vxlib_dst,
                src_desc->color_space - VX_ENUM_BASE(VX_ID_KHRONOS, VX_ENUM_COLOR_SPACE));
        }
        else if ((VX_DF_IMAGE_IYUV == src_desc->format) && (VX_DF_IMAGE_RGBX == dst_desc->format))
        {
            status = VXLIB_colorConvert_IYUVtoRGBX_i8u_o8u((uint8_t *)src_addr[0],
                 &vxlib_src, (uint8_t *)src_addr[1],  &vxlib_src1, (uint8_t *)src_addr[2],
                 &vxlib_src2, (uint8_t *)dst_addr[0], &vxlib_dst,
                 src_desc->color_space - VX_ENUM_BASE(VX_ID_KHRONOS, VX_ENUM_COLOR_SPACE));
        }
        else if ((VX_DF_IMAGE_IYUV == src_desc->format) && (VX_DF_IMAGE_NV12 == dst_desc->format))
        {
            vxlib_dst1.dim_x = dst_desc->imagepatch_addr[1].dim_x;
            status = VXLIB_colorConvert_IYUVtoNV12_i8u_o8u((uint8_t *)src_addr[0], &vxlib_src,
                (uint8_t *)src_addr[1], &vxlib_src1, (uint8_t *)src_addr[2], &vxlib_src2,
                (uint8_t *)dst_addr[0], &vxlib_dst, (uint8_t *)dst_addr[1], &vxlib_dst1);
        }
        else if ((VX_DF_IMAGE_IYUV == src_desc->format) && (VX_DF_IMAGE_YUV4 == dst_desc->format))
        {
            status = VXLIB_colorConvert_IYUVtoYUV4_i8u_o8u((uint8_t *)src_addr[0], &vxlib_src,
                (uint8_t *)src_addr[1], &vxlib_src1, (uint8_t *)src_addr[2], &vxlib_src2,
                (uint8_t *)dst_addr[0],  &vxlib_dst, (uint8_t *)dst_addr[1],  &vxlib_dst1,
                (uint8_t *)dst_addr[2], &vxlib_dst2);
        }

        if (VXLIB_SUCCESS != status)
        {
            status = VX_FAILURE;
        }

        for (i = 0; i < src_desc->planes; i++)
        {
            tivxMemBufferUnmap(src_desc->mem_ptr[i].target_ptr,
                src_desc->mem_size[i], src_desc->mem_ptr[i].mem_type,
                VX_READ_ONLY);
        }

        for (i = 0; i < dst_desc->planes; i++)
        {
            tivxMemBufferUnmap(dst_desc->mem_ptr[i].target_ptr,
                dst_desc->mem_size[i], dst_desc->mem_ptr[i].mem_type,
                VX_WRITE_ONLY);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelColorConvertCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;
    tivx_obj_desc_image_t *dst, *src;

    if (num_params != TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS; i ++)
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
        src = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_COLOR_CONVERT_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_COLOR_CONVERT_OUT_IMG_IDX];

        /* scenarios where scratch memory is needed */
        if ( ((VX_DF_IMAGE_RGB == src->format) && (VX_DF_IMAGE_NV12 == dst->format)) ||
             ((VX_DF_IMAGE_RGB == src->format) && (VX_DF_IMAGE_IYUV == dst->format)) ||
             ((VX_DF_IMAGE_RGBX == src->format) && (VX_DF_IMAGE_NV12 == dst->format)) ||
             ((VX_DF_IMAGE_RGBX == src->format) && (VX_DF_IMAGE_IYUV == dst->format)) )
        {
            uint32_t temp_ptr_size;

            temp_ptr_size = 4 * dst->imagepatch_addr[0].stride_y * sizeof(uint8_t);

            temp_ptr = tivxMemAlloc(temp_ptr_size);

            if (NULL == temp_ptr)
            {
                status = VX_ERROR_NO_MEMORY;
            }
            else
            {
                memset(temp_ptr, 0, temp_ptr_size);
                tivxSetTargetKernelInstanceContext(kernel, temp_ptr, temp_ptr_size);
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelColorConvertDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;
    uint32_t temp_ptr_size;
    tivx_obj_desc_image_t *dst, *src;

    if (num_params != TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS; i ++)
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
        src = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_COLOR_CONVERT_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_COLOR_CONVERT_OUT_IMG_IDX];

        /* scenarios where scratch memory is needed */
        if ( ((VX_DF_IMAGE_RGB == src->format) && (VX_DF_IMAGE_NV12 == dst->format)) ||
             ((VX_DF_IMAGE_RGB == src->format) && (VX_DF_IMAGE_IYUV == dst->format)) ||
             ((VX_DF_IMAGE_RGBX == src->format) && (VX_DF_IMAGE_NV12 == dst->format)) ||
             ((VX_DF_IMAGE_RGBX == src->format) && (VX_DF_IMAGE_IYUV == dst->format)) )
        {
            status = tivxGetTargetKernelInstanceContext(kernel, &temp_ptr, &temp_ptr_size);

            if (VXLIB_SUCCESS != status)
            {
                status = VX_ERROR_NO_MEMORY;
            }
            else
            {
                tivxMemFree(temp_ptr, temp_ptr_size);
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelColorConvertControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelColorConvertProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status;

    status = tivxKernelColorConvert(kernel, obj_desc, num_params, VX_KERNEL_COLOR_CONVERT);

    return (status);
}

void tivxAddTargetKernelColorConvert()
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

        vx_color_convert_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_COLOR_CONVERT,
            target_name,
            tivxKernelColorConvertProcess,
            tivxKernelColorConvertCreate,
            tivxKernelColorConvertDelete,
            tivxKernelColorConvertControl,
            NULL);
    }
}


void tivxRemoveTargetKernelColorConvert()
{
    tivxRemoveTargetKernel(vx_color_convert_target_kernel);
}

