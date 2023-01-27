/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/



#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_color_convert.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

static tivx_target_kernel vx_color_convert_target_kernel = NULL;

static vx_status tivxKernelColorConvert(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    vx_enum kern_type);
static vx_status VX_CALLBACK tivxKernelColorConvertCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelColorConvertDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelColorConvertProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status tivxKernelColorConvert(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    vx_enum kern_type)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc, *dst_desc;
    uint32_t i;
    void *src_addr[4] = {NULL}, *dst_addr[4] = {NULL};
    VXLIB_bufParams2D_t vxlib_src[4], vxlib_dst[4];
    void *scratch;
    uint32_t scratch_size;

    if (num_params != TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        void *src_desc_target_ptr[TIVX_IMAGE_MAX_PLANES] = {NULL, NULL, NULL};
        void *dst_desc_target_ptr[TIVX_IMAGE_MAX_PLANES] = {NULL, NULL, NULL};

        src_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_COLOR_CONVERT_INPUT_IDX];
        dst_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_COLOR_CONVERT_OUTPUT_IDX];

        for (i = 0; i < src_desc->planes; i++)
        {
            src_desc_target_ptr[i] = tivxMemShared2TargetPtr(&src_desc->mem_ptr[i]);
            tivxCheckStatus(&status, tivxMemBufferMap(src_desc_target_ptr[i], src_desc->mem_size[i],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
        tivxSetPointerLocation(src_desc, src_desc_target_ptr, (uint8_t**)&src_addr);

        for (i = 0; i < dst_desc->planes; i++)
        {
            dst_desc_target_ptr[i] = tivxMemShared2TargetPtr(&dst_desc->mem_ptr[i]);
            tivxCheckStatus(&status, tivxMemBufferMap(dst_desc_target_ptr[i], dst_desc->mem_size[i],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
        }
        tivxSetPointerLocation(dst_desc, dst_desc_target_ptr, (uint8_t**)&dst_addr);

        tivxInitBufParams(src_desc, (VXLIB_bufParams2D_t*)&vxlib_src);

        tivxInitBufParams(dst_desc, (VXLIB_bufParams2D_t*)&vxlib_dst);

        if (((vx_df_image)VX_DF_IMAGE_RGB == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_RGBX == dst_desc->format))
        {
            status = (vx_status)VXLIB_colorConvert_RGBtoRGBX_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0]);
        }
        else if (((vx_df_image)VX_DF_IMAGE_RGB == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == dst_desc->format))
        {
            status = tivxGetTargetKernelInstanceContext(kernel, &scratch, &scratch_size);

            if ((vx_status)VX_SUCCESS == status)
            {
                status = (vx_status)VXLIB_colorConvert_RGBtoNV12_i8u_o8u((uint8_t *)src_addr[0],
                    &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t *)dst_addr[1], &vxlib_dst[1],
                    scratch, scratch_size);
            }
        }
        else if (((vx_df_image)VX_DF_IMAGE_RGB == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_YUV4 == dst_desc->format))
        {
            status = (vx_status)VXLIB_colorConvert_RGBtoYUV4_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t *)dst_addr[1], &vxlib_dst[1],
                (uint8_t *)dst_addr[2], &vxlib_dst[2]);
        }
        else if (((vx_df_image)VX_DF_IMAGE_RGB == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_IYUV == dst_desc->format))
        {
            status = tivxGetTargetKernelInstanceContext(kernel, &scratch, &scratch_size);

            if ((vx_status)VX_SUCCESS == status)
            {
                status = (vx_status)VXLIB_colorConvert_RGBtoIYUV_i8u_o8u((uint8_t *)src_addr[0],
                    &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t *)dst_addr[1],
                    &vxlib_dst[1], (uint8_t *)dst_addr[2], &vxlib_dst[2], scratch, scratch_size);
            }
        }
        else if (((vx_df_image)VX_DF_IMAGE_RGBX == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_RGB == dst_desc->format))
        {
            status = (vx_status)VXLIB_colorConvert_RGBXtoRGB_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0]);
        }
        else if (((vx_df_image)VX_DF_IMAGE_RGBX == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == dst_desc->format))
        {
            status = tivxGetTargetKernelInstanceContext(kernel, &scratch, &scratch_size);

            if ((vx_status)VX_SUCCESS == status)
            {
                status = (vx_status)VXLIB_colorConvert_RGBXtoNV12_i8u_o8u((uint8_t *)src_addr[0],
                    &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t *)dst_addr[1],
                    &vxlib_dst[1], scratch, scratch_size);
            }
        }
        else if (((vx_df_image)VX_DF_IMAGE_RGBX == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_YUV4 == dst_desc->format))
        {
            status = (vx_status)VXLIB_colorConvert_RGBXtoYUV4_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t *)dst_addr[1],
                &vxlib_dst[1], (uint8_t *)dst_addr[2], &vxlib_dst[2]);
        }
        else if (((vx_df_image)VX_DF_IMAGE_RGBX == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_IYUV == dst_desc->format))
        {
            status = tivxGetTargetKernelInstanceContext(kernel, &scratch, &scratch_size);

            if ((vx_status)VX_SUCCESS == status)
            {
                status = (vx_status)VXLIB_colorConvert_RGBXtoIYUV_i8u_o8u((uint8_t *)src_addr[0],
                    &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t *)dst_addr[1],
                    &vxlib_dst[1], (uint8_t *)dst_addr[2], &vxlib_dst[2], scratch, scratch_size);
            }
        }
        else if ( ( ((vx_df_image)VX_DF_IMAGE_NV12 == src_desc->format) || ((vx_df_image)VX_DF_IMAGE_NV21 == src_desc->format) ) &&
                 ((vx_df_image)VX_DF_IMAGE_RGB == dst_desc->format))
        {
            uint8_t u_pix = (src_desc->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;
            vx_enum temp0 = (vx_enum)src_desc->color_space - VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);
            status = (vx_status)VXLIB_colorConvert_NVXXtoRGB_i8u_o8u((uint8_t *)src_addr[0], &vxlib_src[0],
                (uint8_t *)src_addr[1], &vxlib_src[1], (uint8_t *)dst_addr[0], &vxlib_dst[0], u_pix, (uint8_t)temp0);
        }
        else if ( ( ((vx_df_image)VX_DF_IMAGE_NV12 == src_desc->format) || ((vx_df_image)VX_DF_IMAGE_NV21 == src_desc->format) ) &&
                 ((vx_df_image)VX_DF_IMAGE_RGBX == dst_desc->format))
        {
            uint8_t u_pix = (src_desc->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;
            vx_enum temp1 = (vx_enum)src_desc->color_space - VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);
            status = (vx_status)VXLIB_colorConvert_NVXXtoRGBX_i8u_o8u((uint8_t *)src_addr[0], &vxlib_src[0],
                (uint8_t *)src_addr[1], &vxlib_src[1], (uint8_t *)dst_addr[0],
                &vxlib_dst[0], u_pix, (uint8_t)temp1);
        }
        else if ( ( ((vx_df_image)VX_DF_IMAGE_NV12 == src_desc->format) || ((vx_df_image)VX_DF_IMAGE_NV21 == src_desc->format) ) &&
                 ((vx_df_image)VX_DF_IMAGE_YUV4 == dst_desc->format))
        {
            uint8_t u_pix = (src_desc->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;

            status = (vx_status)VXLIB_colorConvert_NVXXtoYUV4_i8u_o8u((uint8_t *)src_addr[0], &vxlib_src[0],
                (uint8_t *)src_addr[1], &vxlib_src[1], (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t *)dst_addr[1],
                &vxlib_dst[1], (uint8_t *)dst_addr[2], &vxlib_dst[2], u_pix);
        }
        else if ( ( ((vx_df_image)VX_DF_IMAGE_NV12 == src_desc->format) || ((vx_df_image)VX_DF_IMAGE_NV21 == src_desc->format) ) &&
                 ((vx_df_image)VX_DF_IMAGE_IYUV == dst_desc->format))
        {
            uint8_t u_pix = (src_desc->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;

            status = (vx_status)VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u((uint8_t *)src_addr[0], &vxlib_src[0],
                (uint8_t *)src_addr[1], &vxlib_src[1], (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t *)dst_addr[1],
                &vxlib_dst[1], (uint8_t *)dst_addr[2], &vxlib_dst[2], u_pix);
        }
        else if (((vx_df_image)VX_DF_IMAGE_YUYV == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_RGB == dst_desc->format))
        {
            vx_enum temp2 = (vx_enum)src_desc->color_space - VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);
            status = (vx_status)VXLIB_colorConvert_YUVXtoRGB_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0], 0, (uint8_t)temp2);
        }
        else if (((vx_df_image)VX_DF_IMAGE_YUYV == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_RGBX == dst_desc->format))
        {
            vx_enum temp3 = (vx_enum)src_desc->color_space - VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);
            status = (vx_status)VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0], 0, (uint8_t)temp3);
        }
        else if (((vx_df_image)VX_DF_IMAGE_YUYV == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == dst_desc->format))
        {
            status = (vx_status)VXLIB_colorConvert_YUVXtoNV12_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t *)dst_addr[1],
                &vxlib_dst[1], 0);
        }
        else if (((vx_df_image)VX_DF_IMAGE_YUYV == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_IYUV == dst_desc->format))
        {
            status = (vx_status)VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t *)dst_addr[1], &vxlib_dst[1],
                (uint8_t *)dst_addr[2], &vxlib_dst[2], 0);
        }
        else if (((vx_df_image)VX_DF_IMAGE_UYVY == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_RGB == dst_desc->format))
        {
            vx_enum temp4 = (vx_enum)src_desc->color_space - VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);
            status = (vx_status)VXLIB_colorConvert_YUVXtoRGB_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0], 1, (uint8_t)temp4);
        }
        else if (((vx_df_image)VX_DF_IMAGE_UYVY == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_RGBX == dst_desc->format))
        {
            vx_enum temp5 = (vx_enum)src_desc->color_space - VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);
            status = (vx_status)VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0], 1, (uint8_t)temp5);
        }
        else if (((vx_df_image)VX_DF_IMAGE_UYVY == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == dst_desc->format))
        {
            status = (vx_status)VXLIB_colorConvert_YUVXtoNV12_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t *)dst_addr[1],
                &vxlib_dst[1], 1);
        }
        else if (((vx_df_image)VX_DF_IMAGE_UYVY == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_IYUV == dst_desc->format))
        {
            status = (vx_status)VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src[0], (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t *)dst_addr[1],
                &vxlib_dst[1], (uint8_t *)dst_addr[2], &vxlib_dst[2], 1);
        }
        else if (((vx_df_image)VX_DF_IMAGE_IYUV == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_RGB == dst_desc->format))
        {
            vx_enum temp6 = (vx_enum)src_desc->color_space - VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);
            status = (vx_status)VXLIB_colorConvert_IYUVtoRGB_i8u_o8u((uint8_t *)src_addr[0],
                &vxlib_src[0], (uint8_t *)src_addr[1], &vxlib_src[1], (uint8_t *)src_addr[2],
                &vxlib_src[2], (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t)temp6);
        }
        else if (((vx_df_image)VX_DF_IMAGE_IYUV == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_RGBX == dst_desc->format))
        {
            vx_enum temp7 = (vx_enum)src_desc->color_space - VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);
            status = (vx_status)VXLIB_colorConvert_IYUVtoRGBX_i8u_o8u((uint8_t *)src_addr[0],
                 &vxlib_src[0], (uint8_t *)src_addr[1],  &vxlib_src[1], (uint8_t *)src_addr[2],
                 &vxlib_src[2], (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t)temp7);
        }
        else if (((vx_df_image)VX_DF_IMAGE_IYUV == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == dst_desc->format))
        {
            status = (vx_status)VXLIB_colorConvert_IYUVtoNV12_i8u_o8u((uint8_t *)src_addr[0], &vxlib_src[0],
                (uint8_t *)src_addr[1], &vxlib_src[1], (uint8_t *)src_addr[2], &vxlib_src[2],
                (uint8_t *)dst_addr[0], &vxlib_dst[0], (uint8_t *)dst_addr[1], &vxlib_dst[1]);
            tivxEnableL1DandL2CacheWb();
        }
        else if (((vx_df_image)VX_DF_IMAGE_IYUV == src_desc->format) && ((vx_df_image)VX_DF_IMAGE_YUV4 == dst_desc->format))
        {
            status = (vx_status)VXLIB_colorConvert_IYUVtoYUV4_i8u_o8u((uint8_t *)src_addr[0], &vxlib_src[0],
                (uint8_t *)src_addr[1], &vxlib_src[1], (uint8_t *)src_addr[2], &vxlib_src[2],
                (uint8_t *)dst_addr[0],  &vxlib_dst[0], (uint8_t *)dst_addr[1],  &vxlib_dst[1],
                (uint8_t *)dst_addr[2], &vxlib_dst[2]);
            tivxEnableL1DandL2CacheWb();
        }
        else
        {
            status = (vx_status)VX_FAILURE;
        }

        if ((vx_status)VXLIB_SUCCESS != status)
        {
            status = (vx_status)VX_FAILURE;
        }

        for (i = 0; i < src_desc->planes; i++)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(src_desc_target_ptr[i],
                src_desc->mem_size[i], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
        }

        for (i = 0; i < dst_desc->planes; i++)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(dst_desc_target_ptr[i],
                dst_desc->mem_size[i], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelColorConvertCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;
    tivx_obj_desc_image_t *dst, *src;

    if (num_params != TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_COLOR_CONVERT_INPUT_IDX];
        dst = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_COLOR_CONVERT_OUTPUT_IDX];

        /* scenarios where scratch memory is needed */
        if ( (((vx_df_image)VX_DF_IMAGE_RGB == src->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format)) ||
             (((vx_df_image)VX_DF_IMAGE_RGB == src->format) && ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format)) ||
             (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format)) ||
             (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) && ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format)) )
        {
            uint32_t temp_ptr_size;

            temp_ptr_size = 4U * dst->imagepatch_addr[0].dim_x * sizeof(uint8_t);

            temp_ptr = tivxMemAlloc(temp_ptr_size, (vx_enum)TIVX_MEM_EXTERNAL);

            if (NULL == temp_ptr)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
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
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;
    uint32_t temp_ptr_size;
    tivx_obj_desc_image_t *dst, *src;

    if (num_params != TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_COLOR_CONVERT_INPUT_IDX];
        dst = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_COLOR_CONVERT_OUTPUT_IDX];

        /* scenarios where scratch memory is needed */
        if ( (((vx_df_image)VX_DF_IMAGE_RGB == src->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format)) ||
             (((vx_df_image)VX_DF_IMAGE_RGB == src->format) && ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format)) ||
             (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format)) ||
             (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) && ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format)) )
        {
            status = tivxGetTargetKernelInstanceContext(kernel, &temp_ptr, &temp_ptr_size);

            if ((vx_status)VXLIB_SUCCESS != status)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
            else
            {
                tivxMemFree(temp_ptr, temp_ptr_size, (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelColorConvertProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status;

    status = tivxKernelColorConvert(kernel, obj_desc, num_params, (vx_enum)VX_KERNEL_COLOR_CONVERT);

    return (status);
}

void tivxAddTargetKernelColorConvert(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_color_convert_target_kernel = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_COLOR_CONVERT,
            target_name,
            tivxKernelColorConvertProcess,
            tivxKernelColorConvertCreate,
            tivxKernelColorConvertDelete,
            NULL,
            NULL);
    }
}


void tivxRemoveTargetKernelColorConvert(void)
{
    tivxRemoveTargetKernel(vx_color_convert_target_kernel);
}

