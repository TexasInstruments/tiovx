/*
*
* Copyright (c) 2017-2019 Texas Instruments Incorporated
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
#include <tivx_kernel_color_convert.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>
#include "tivx_target_kernels_priv.h"

typedef struct
{
    tivx_bam_graph_handle graph_handle;
    uint8_t bam_node_num;
} tivxColorConvertParams;

static tivx_target_kernel vx_color_convert_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelBamColorConvertProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelBamColorConvertCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelBamColorConvertDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

/* Supernode Callbacks */
static vx_status VX_CALLBACK tivxKernelColorConvertCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size);

static vx_status VX_CALLBACK tivxKernelColorConvertGetNodePort(
    tivx_target_kernel_instance kernel, uint8_t ovx_port, uint8_t plane,
    uint8_t *bam_node, uint8_t *bam_port);

static vx_status VX_CALLBACK tivxKernelBamColorConvertProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    tivxColorConvertParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    uint8_t *src_addr[4] = {NULL}, *dst_addr[4] = {NULL};
    uint32_t size;
    void *img_ptrs[8] = {NULL};


    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_COLOR_CONVERT_INPUT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_COLOR_CONVERT_OUTPUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxColorConvertParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *src_target_ptr[TIVX_IMAGE_MAX_PLANES];
        void *dst_target_ptr[TIVX_IMAGE_MAX_PLANES];

        for (i = 0; i < src->planes; i++)
        {
            src_target_ptr[i] = tivxMemShared2TargetPtr(&src->mem_ptr[i]);
        }
        tivxSetPointerLocation(src, src_target_ptr, (uint8_t**)&src_addr);

        for (i = 0; i < dst->planes; i++)
        {
            dst_target_ptr[i] = tivxMemShared2TargetPtr(&dst->mem_ptr[i]);
        }
        tivxSetPointerLocation(dst, dst_target_ptr, (uint8_t**)&dst_addr);

        if ((((vx_df_image)VX_DF_IMAGE_RGB == src->format) &&
             ((vx_df_image)VX_DF_IMAGE_YUV4 == dst->format)) ||
            (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) &&
             ((vx_df_image)VX_DF_IMAGE_YUV4 == dst->format)))
        {
            img_ptrs[0] = src_addr[0];
            img_ptrs[1] = dst_addr[0];
            img_ptrs[2] = dst_addr[1];
            img_ptrs[3] = dst_addr[2];
            tivxBamUpdatePointers(prms->graph_handle, 1U, 3U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else if ((((vx_df_image)VX_DF_IMAGE_RGB == src->format) &&
             ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format)) ||
            (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) &&
             ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format)))
        {
            img_ptrs[0] = src_addr[0];
            img_ptrs[1] = dst_addr[0];
            img_ptrs[2] = dst_addr[1];
            tivxBamUpdatePointers(prms->graph_handle, 1U, 2U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else if ((((vx_df_image)VX_DF_IMAGE_RGB == src->format) &&
             ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format)) ||
            (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) &&
             ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format)))
        {
            img_ptrs[0] = src_addr[0];
            img_ptrs[1] = dst_addr[0];
            img_ptrs[2] = dst_addr[1];
            img_ptrs[3] = dst_addr[2];
            tivxBamUpdatePointers(prms->graph_handle, 1U, 3U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else if ((((vx_df_image)VX_DF_IMAGE_RGB == src->format) &&
                  ((vx_df_image)VX_DF_IMAGE_RGBX == dst->format)) ||
                 (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) &&
                  ((vx_df_image)VX_DF_IMAGE_RGB == dst->format)))
        {
            img_ptrs[0] = src_addr[0];
            img_ptrs[1] = dst_addr[0];
            tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else if ((((vx_df_image)VX_DF_IMAGE_NV12 == src->format) ||
                  ((vx_df_image)VX_DF_IMAGE_NV21 == src->format)) &&
                 (((vx_df_image)VX_DF_IMAGE_RGB == dst->format) ||
                  ((vx_df_image)VX_DF_IMAGE_RGBX == dst->format)))
        {
            img_ptrs[0] = src_addr[0];
            img_ptrs[1] = src_addr[1];
            img_ptrs[2] = dst_addr[0];
            tivxBamUpdatePointers(prms->graph_handle, 2U, 1U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else if ((((vx_df_image)VX_DF_IMAGE_NV12 == src->format) ||
                  ((vx_df_image)VX_DF_IMAGE_NV21 == src->format)) &&
                 (((vx_df_image)VX_DF_IMAGE_YUV4 == dst->format) ||
                  ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format)))
        {
            img_ptrs[0] = src_addr[0];
            img_ptrs[1] = src_addr[1];
            img_ptrs[2] = dst_addr[0];
            img_ptrs[3] = dst_addr[1];
            img_ptrs[4] = dst_addr[2];
            tivxBamUpdatePointers(prms->graph_handle, 2U, 3U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else if ((((vx_df_image)VX_DF_IMAGE_YUYV == src->format) ||
                  ((vx_df_image)VX_DF_IMAGE_UYVY == src->format)) &&
                 (((vx_df_image)VX_DF_IMAGE_RGB == dst->format) ||
                  ((vx_df_image)VX_DF_IMAGE_RGBX == dst->format)))
        {
            img_ptrs[0] = src_addr[0];
            img_ptrs[1] = dst_addr[0];

            tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else if ((((vx_df_image)VX_DF_IMAGE_YUYV == src->format) ||
                  ((vx_df_image)VX_DF_IMAGE_UYVY == src->format)) &&
                 ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format))
        {
            img_ptrs[0] = src_addr[0];
            img_ptrs[1] = dst_addr[0];
            img_ptrs[2] = dst_addr[1];

            tivxBamUpdatePointers(prms->graph_handle, 1U, 2U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else if ((((vx_df_image)VX_DF_IMAGE_YUYV == src->format) ||
                  ((vx_df_image)VX_DF_IMAGE_UYVY == src->format)) &&
                 ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format))
        {
            img_ptrs[0] = src_addr[0];
            img_ptrs[1] = dst_addr[0];
            img_ptrs[2] = dst_addr[1];
            img_ptrs[3] = dst_addr[2];

            tivxBamUpdatePointers(prms->graph_handle, 1U, 3U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else if (((vx_df_image)VX_DF_IMAGE_IYUV == src->format) &&
                 (((vx_df_image)VX_DF_IMAGE_RGB == dst->format) ||
                  ((vx_df_image)VX_DF_IMAGE_RGBX == dst->format)))
        {
            img_ptrs[0] = src_addr[0];
            img_ptrs[1] = src_addr[1];
            img_ptrs[2] = src_addr[2];
            img_ptrs[3] = dst_addr[0];

            tivxBamUpdatePointers(prms->graph_handle, 3U, 1U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else if (((vx_df_image)VX_DF_IMAGE_IYUV == src->format) &&
                 ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format))
        {
            img_ptrs[0] = src_addr[0];
            img_ptrs[1] = src_addr[1];
            img_ptrs[2] = src_addr[2];
            img_ptrs[3] = dst_addr[0];
            img_ptrs[4] = dst_addr[1];

            tivxBamUpdatePointers(prms->graph_handle, 3U, 2U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else if (((vx_df_image)VX_DF_IMAGE_IYUV == src->format) &&
                 ((vx_df_image)VX_DF_IMAGE_YUV4 == dst->format))
        {
            img_ptrs[0] = src_addr[0];
            img_ptrs[1] = src_addr[1];
            img_ptrs[2] = src_addr[2];
            img_ptrs[3] = dst_addr[0];
            img_ptrs[4] = dst_addr[1];
            img_ptrs[5] = dst_addr[2];

            tivxBamUpdatePointers(prms->graph_handle, 3U, 3U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else
        {
            /* do nothing */
        }
    }
    else
    {
        status = (vx_status)VX_ERROR_NO_MEMORY;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelBamColorConvertCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{

    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    tivxColorConvertParams *prms = NULL;
    tivx_bam_kernel_details_t kernel_details;
    VXLIB_bufParams2D_t vxlib_src[4], vxlib_dst[4];
    VXLIB_bufParams2D_t *buf_params[6];

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxBamInitKernelDetails(&kernel_details, 1, kernel);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_COLOR_CONVERT_INPUT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_COLOR_CONVERT_OUTPUT_IDX];

        prms = tivxMemAlloc(sizeof(tivxColorConvertParams), (vx_enum)TIVX_MEM_EXTERNAL);

        /* Allocate Scratch memory */
        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxColorConvertParams));
        }

        if (NULL != prms)
        {
            tivxInitBufParams(src, (VXLIB_bufParams2D_t*)&vxlib_src);
            tivxInitBufParams(dst, (VXLIB_bufParams2D_t*)&vxlib_dst);

            if (((vx_df_image)VX_DF_IMAGE_RGB == src->format) &&
                ((vx_df_image)VX_DF_IMAGE_YUV4 == dst->format))
            {
                vxlib_src[0].data_type = (uint32_t)VXLIB_UINT24;
                vxlib_dst[0].data_type = (uint32_t)VXLIB_UINT8;
                vxlib_dst[1].data_type = (uint32_t)VXLIB_UINT8;
                vxlib_dst[2].data_type = (uint32_t)VXLIB_UINT8;

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_dst[0];
                buf_params[2] = &vxlib_dst[1];
                buf_params[3] = &vxlib_dst[2];

                BAM_VXLIB_colorConvert_RGBtoYUV4_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoYUV4_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGB == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format))
            {
                vxlib_src[0].data_type = (uint32_t)VXLIB_UINT24;
                vxlib_dst[0].data_type = (uint32_t)VXLIB_UINT8;
                vxlib_dst[1].data_type = (uint32_t)VXLIB_UINT8;

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_dst[0];
                buf_params[2] = &vxlib_dst[1];

                BAM_VXLIB_colorConvert_RGBtoNV12_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoNV12_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGB == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format))
            {
                vxlib_src[0].data_type = (uint32_t)VXLIB_UINT24;
                vxlib_dst[0].data_type = (uint32_t)VXLIB_UINT8;
                vxlib_dst[1].data_type = (uint32_t)VXLIB_UINT8;
                vxlib_dst[2].data_type = (uint32_t)VXLIB_UINT8;

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_dst[0];
                buf_params[2] = &vxlib_dst[1];
                buf_params[3] = &vxlib_dst[2];

                BAM_VXLIB_colorConvert_RGBtoIYUV_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoIYUV_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGB == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_RGBX == dst->format))
            {
                vxlib_src[0].data_type = (uint32_t)VXLIB_UINT24;
                vxlib_dst[0].data_type = (uint32_t)VXLIB_UINT32;

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_dst[0];

                BAM_VXLIB_colorConvert_RGBtoRGBX_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoRGBX_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_RGB == dst->format))
            {
                vxlib_src[0].data_type = (uint32_t)VXLIB_UINT32;
                vxlib_dst[0].data_type = (uint32_t)VXLIB_UINT24;

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_dst[0];

                BAM_VXLIB_colorConvert_RGBXtoRGB_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoRGB_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_YUV4 == dst->format))
            {
                vxlib_src[0].data_type = (uint32_t)VXLIB_UINT32;
                vxlib_dst[0].data_type = (uint32_t)VXLIB_UINT8;
                vxlib_dst[1].data_type = (uint32_t)VXLIB_UINT8;
                vxlib_dst[2].data_type = (uint32_t)VXLIB_UINT8;

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_dst[0];
                buf_params[2] = &vxlib_dst[1];
                buf_params[3] = &vxlib_dst[2];

                BAM_VXLIB_colorConvert_RGBXtoYUV4_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoYUV4_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format))
            {
                vxlib_src[0].data_type = (uint32_t)VXLIB_UINT32;
                vxlib_dst[0].data_type = (uint32_t)VXLIB_UINT8;
                vxlib_dst[1].data_type = (uint32_t)VXLIB_UINT8;

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_dst[0];
                buf_params[2] = &vxlib_dst[1];

                BAM_VXLIB_colorConvert_RGBXtoNV12_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoNV12_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format))
            {
                vxlib_src[0].data_type = (uint32_t)VXLIB_UINT32;
                vxlib_dst[0].data_type = (uint32_t)VXLIB_UINT8;
                vxlib_dst[1].data_type = (uint32_t)VXLIB_UINT8;
                vxlib_dst[2].data_type = (uint32_t)VXLIB_UINT8;

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_dst[0];
                buf_params[2] = &vxlib_dst[1];
                buf_params[3] = &vxlib_dst[2];

                BAM_VXLIB_colorConvert_RGBXtoIYUV_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoIYUV_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if ((((vx_df_image)VX_DF_IMAGE_NV12 == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_NV21 == src->format)) &&
                     ((vx_df_image)VX_DF_IMAGE_RGB == dst->format))
            {
                BAM_VXLIB_colorConvert_NVXXtoRGB_i8u_o8u_params params;

                params.u_pix = (src->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;
                params.src_space = (uint8_t)src->color_space -
                    (uint8_t)VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);

                vxlib_src[1].dim_x = src->imagepatch_addr[1].dim_x;

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_src[1];
                buf_params[2] = &vxlib_dst[0];

                kernel_details.compute_kernel_params = &params;

                BAM_VXLIB_colorConvert_NVXXtoRGB_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoRGB_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if ((((vx_df_image)VX_DF_IMAGE_NV12 == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_NV21 == src->format)) &&
                     ((vx_df_image)VX_DF_IMAGE_RGBX == dst->format))
            {
                BAM_VXLIB_colorConvert_NVXXtoRGBX_i8u_o8u_params params;

                params.u_pix = (src->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;
                params.src_space = (uint8_t)src->color_space -
                    (uint8_t)VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);

                vxlib_src[1].dim_x = src->imagepatch_addr[1].dim_x;

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_src[1];
                buf_params[2] = &vxlib_dst[0];

                kernel_details.compute_kernel_params = &params;

                BAM_VXLIB_colorConvert_NVXXtoRGBX_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoRGBX_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if ((((vx_df_image)VX_DF_IMAGE_NV12 == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_NV21 == src->format)) &&
                     ((vx_df_image)VX_DF_IMAGE_YUV4 == dst->format))
            {
                BAM_VXLIB_colorConvert_NVXXtoYUV4_i8u_o8u_params params;

                params.u_pix = (src->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;

                vxlib_src[1].dim_x = src->imagepatch_addr[1].dim_x;

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_src[1];
                buf_params[2] = &vxlib_dst[0];
                buf_params[3] = &vxlib_dst[1];
                buf_params[4] = &vxlib_dst[2];

                kernel_details.compute_kernel_params = &params;

                BAM_VXLIB_colorConvert_NVXXtoYUV4_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoYUV4_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if ((((vx_df_image)VX_DF_IMAGE_NV12 == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_NV21 == src->format)) &&
                     ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format))
            {
                BAM_VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u_params params;

                params.u_pix = (src->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_src[1];
                buf_params[2] = &vxlib_dst[0];
                buf_params[3] = &vxlib_dst[1];
                buf_params[4] = &vxlib_dst[2];

                kernel_details.compute_kernel_params = &params;

                BAM_VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoIYUV_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if ((((vx_df_image)VX_DF_IMAGE_YUYV == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_UYVY == src->format)) &&
                     ((vx_df_image)VX_DF_IMAGE_RGB == dst->format))
            {
                BAM_VXLIB_colorConvert_YUVXtoRGB_i8u_o8u_params params;

                if ((vx_df_image)VX_DF_IMAGE_YUYV == src->format)
                {
                    params.x_value = 0;
                }
                else
                {
                    params.x_value = 1;
                }
                params.src_space = (uint8_t)src->color_space -
                    (uint8_t)VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_dst[0];

                kernel_details.compute_kernel_params = &params;

                BAM_VXLIB_colorConvert_YUVXtoRGB_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoRGB_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if ((((vx_df_image)VX_DF_IMAGE_YUYV == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_UYVY == src->format))&&
                     ((vx_df_image)VX_DF_IMAGE_RGBX == dst->format))
            {
                BAM_VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u_params params;

                if ((vx_df_image)VX_DF_IMAGE_YUYV == src->format)
                {
                    params.x_value = 0;
                }
                else
                {
                    params.x_value = 1;
                }
                params.src_space = (uint8_t)src->color_space -
                    (uint8_t)VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_dst[0];

                kernel_details.compute_kernel_params = &params;

                BAM_VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoRGBX_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if ((((vx_df_image)VX_DF_IMAGE_YUYV == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_UYVY == src->format))&&
                     ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format))
            {
                BAM_VXLIB_colorConvert_YUVXtoNV12_i8u_o8u_params params;

                if ((vx_df_image)VX_DF_IMAGE_YUYV == src->format)
                {
                    params.x_value = 0;
                }
                else
                {
                    params.x_value = 1;
                }

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_dst[0];
                buf_params[2] = &vxlib_dst[1];

                kernel_details.compute_kernel_params = &params;

                BAM_VXLIB_colorConvert_YUVXtoNV12_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoNV12_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if ((((vx_df_image)VX_DF_IMAGE_YUYV == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_UYVY == src->format)) &&
                     ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format))
            {
                BAM_VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u_params params;

                if ((vx_df_image)VX_DF_IMAGE_YUYV == src->format)
                {
                    params.x_value = 0;
                }
                else
                {
                    params.x_value = 1;
                }

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_dst[0];
                buf_params[2] = &vxlib_dst[1];
                buf_params[3] = &vxlib_dst[2];

                kernel_details.compute_kernel_params = &params;

                BAM_VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoIYUV_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if (((vx_df_image)VX_DF_IMAGE_IYUV == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_RGB == dst->format))
            {
                BAM_VXLIB_colorConvert_IYUVtoRGB_i8u_o8u_params params;

                params.src_space = (uint8_t)src->color_space -
                    (uint8_t)VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_src[1];
                buf_params[2] = &vxlib_src[2];
                buf_params[3] = &vxlib_dst[0];

                kernel_details.compute_kernel_params = &params;

                BAM_VXLIB_colorConvert_IYUVtoRGB_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoRGB_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if (((vx_df_image)VX_DF_IMAGE_IYUV == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_RGBX == dst->format))
            {
                BAM_VXLIB_colorConvert_IYUVtoRGBX_i8u_o8u_params params;

                params.src_space = (uint8_t)src->color_space -
                    (uint8_t)VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);

                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_src[1];
                buf_params[2] = &vxlib_src[2];
                buf_params[3] = &vxlib_dst[0];

                kernel_details.compute_kernel_params = &params;

                BAM_VXLIB_colorConvert_IYUVtoRGBX_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoRGBX_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if (((vx_df_image)VX_DF_IMAGE_IYUV == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format))
            {
                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_src[1];
                buf_params[2] = &vxlib_src[2];
                buf_params[3] = &vxlib_dst[0];
                buf_params[4] = &vxlib_dst[1];

                BAM_VXLIB_colorConvert_IYUVtoNV12_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoNV12_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else if (((vx_df_image)VX_DF_IMAGE_IYUV == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_YUV4 == dst->format))
            {
                buf_params[0] = &vxlib_src[0];
                buf_params[1] = &vxlib_src[1];
                buf_params[2] = &vxlib_src[2];
                buf_params[3] = &vxlib_dst[0];
                buf_params[4] = &vxlib_dst[1];
                buf_params[5] = &vxlib_dst[2];

                vxlib_dst[1].dim_x = dst->imagepatch_addr[1].dim_x;

                BAM_VXLIB_colorConvert_IYUVtoYUV4_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoYUV4_I8U_O8U,
                    buf_params, &kernel_details, &prms->graph_handle);
            }
            else
            {
                /* do nothing */
            }
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxColorConvertParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxColorConvertParams),
                    (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelBamColorConvertDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t size;
    tivxColorConvertParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxColorConvertParams) == size))
        {
            if(NULL != prms->graph_handle)
            {
                tivxBamDestroyHandle(prms->graph_handle);
            }
            tivxMemFree(prms, sizeof(tivxColorConvertParams), (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

void tivxAddTargetKernelBamColorConvert(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == (vx_enum)TIVX_CPU_ID_DSP1) || (self_cpu == (vx_enum)TIVX_CPU_ID_DSP2))
    {
        if (self_cpu == (vx_enum)TIVX_CPU_ID_DSP1)
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
            (vx_enum)VX_KERNEL_COLOR_CONVERT,
            target_name,
            tivxKernelBamColorConvertProcess,
            tivxKernelBamColorConvertCreate,
            tivxKernelBamColorConvertDelete,
            NULL,
            NULL);

        tivxEnableKernelForSuperNode(vx_color_convert_target_kernel,
            tivxKernelColorConvertCreateInBamGraph,
            tivxKernelColorConvertGetNodePort,
            NULL,
            NULL,
            NULL,
            MAX10((int32_t)sizeof(BAM_VXLIB_colorConvert_NVXXtoRGB_i8u_o8u_params),
                 (int32_t)sizeof(BAM_VXLIB_colorConvert_NVXXtoRGBX_i8u_o8u_params),
                 (int32_t)sizeof(BAM_VXLIB_colorConvert_NVXXtoYUV4_i8u_o8u_params),
                 (int32_t)sizeof(BAM_VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u_params),
                 (int32_t)sizeof(BAM_VXLIB_colorConvert_YUVXtoRGB_i8u_o8u_params),
                 (int32_t)sizeof(BAM_VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u_params),
                 (int32_t)sizeof(BAM_VXLIB_colorConvert_YUVXtoNV12_i8u_o8u_params),
                 (int32_t)sizeof(BAM_VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u_params),
                 (int32_t)sizeof(BAM_VXLIB_colorConvert_IYUVtoRGB_i8u_o8u_params),
                 (int32_t)sizeof(BAM_VXLIB_colorConvert_IYUVtoRGBX_i8u_o8u_params)),
            NULL);
    }
}


void tivxRemoveTargetKernelBamColorConvert(void)
{
    tivxRemoveTargetKernel(vx_color_convert_target_kernel);
}

static vx_status VX_CALLBACK tivxKernelColorConvertCreateInBamGraph (
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size)
{

    vx_status status = (vx_status)VX_SUCCESS;
    tivxColorConvertParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_COLOR_CONVERT_INPUT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_COLOR_CONVERT_OUTPUT_IDX];

        prms = tivxMemAlloc(sizeof(tivxColorConvertParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxColorConvertParams));

            node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
            node_list[*bam_node_cnt].kernelArgs = NULL;

            if (((vx_df_image)VX_DF_IMAGE_RGB == src->format) &&
                ((vx_df_image)VX_DF_IMAGE_YUV4 == dst->format))
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoYUV4_I8U_O8U;

                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

                BAM_VXLIB_colorConvert_RGBtoYUV4_i8u_o8u_getKernelInfo(NULL,
                                                             &kernel_details[*bam_node_cnt].kernel_info);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGB == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format))
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoNV12_I8U_O8U;

                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

                BAM_VXLIB_colorConvert_RGBtoNV12_i8u_o8u_getKernelInfo(NULL,
                                                              &kernel_details[*bam_node_cnt].kernel_info);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGB == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format))
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoIYUV_I8U_O8U;

                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

                BAM_VXLIB_colorConvert_RGBtoIYUV_i8u_o8u_getKernelInfo(NULL,
                                                                &kernel_details[*bam_node_cnt].kernel_info);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGB == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_RGBX == dst->format))
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoRGBX_I8U_O8U;

                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

                BAM_VXLIB_colorConvert_RGBtoRGBX_i8u_o8u_getKernelInfo(NULL,
                                                                &kernel_details[*bam_node_cnt].kernel_info);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_RGB == dst->format))
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoRGB_I8U_O8U;

                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

                BAM_VXLIB_colorConvert_RGBXtoRGB_i8u_o8u_getKernelInfo(NULL,
                                                                &kernel_details[*bam_node_cnt].kernel_info);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_YUV4 == dst->format))
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoYUV4_I8U_O8U;

                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

                BAM_VXLIB_colorConvert_RGBXtoYUV4_i8u_o8u_getKernelInfo(NULL,
                                                                &kernel_details[*bam_node_cnt].kernel_info);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_YUV4 == dst->format))
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoYUV4_I8U_O8U;

                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

                BAM_VXLIB_colorConvert_RGBXtoYUV4_i8u_o8u_getKernelInfo(NULL,
                                                                &kernel_details[*bam_node_cnt].kernel_info);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format))
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoNV12_I8U_O8U;

                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

                BAM_VXLIB_colorConvert_RGBXtoNV12_i8u_o8u_getKernelInfo(NULL,
                                                                &kernel_details[*bam_node_cnt].kernel_info);
            }
            else if (((vx_df_image)VX_DF_IMAGE_RGBX == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format))
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoIYUV_I8U_O8U;

                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

                BAM_VXLIB_colorConvert_RGBXtoIYUV_i8u_o8u_getKernelInfo(NULL,
                                                                &kernel_details[*bam_node_cnt].kernel_info);
            }
            else if ((((vx_df_image)VX_DF_IMAGE_NV12 == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_NV21 == src->format)) &&
                     ((vx_df_image)VX_DF_IMAGE_RGB == dst->format))
            {
                BAM_VXLIB_colorConvert_NVXXtoRGB_i8u_o8u_params *kernel_params = (BAM_VXLIB_colorConvert_NVXXtoRGB_i8u_o8u_params*)scratch;

                if ((NULL != kernel_params) &&
                    (*size >= (int32_t)sizeof(BAM_VXLIB_colorConvert_NVXXtoRGB_i8u_o8u_params)))
                {
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoRGB_I8U_O8U;

                    kernel_params->u_pix = (src->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;
                    kernel_params->src_space = (uint8_t)src->color_space -
                    (uint8_t)VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);

                    kernel_details[*bam_node_cnt].compute_kernel_params = (void*)kernel_params;

                    BAM_VXLIB_colorConvert_NVXXtoRGB_i8u_o8u_getKernelInfo(NULL,
                                                                   &kernel_details[*bam_node_cnt].kernel_info);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "colorConvert_NVXXtoRGB_i8u_o8u, kernel_params is null or the size is not as expected\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
            else if ((((vx_df_image)VX_DF_IMAGE_NV12 == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_NV21 == src->format)) &&
                     ((vx_df_image)VX_DF_IMAGE_RGBX == dst->format))
            {
                BAM_VXLIB_colorConvert_NVXXtoRGBX_i8u_o8u_params *kernel_params = (BAM_VXLIB_colorConvert_NVXXtoRGBX_i8u_o8u_params*)scratch;

                if ((NULL != kernel_params) &&
                    (*size >= (int32_t)sizeof(BAM_VXLIB_colorConvert_NVXXtoRGBX_i8u_o8u_params)))
                {
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoRGBX_I8U_O8U;

                    kernel_params->u_pix = (src->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;
                    kernel_params->src_space = (uint8_t)src->color_space -
                        (uint8_t)VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);

                    kernel_details[*bam_node_cnt].compute_kernel_params = (void*)kernel_params;

                    BAM_VXLIB_colorConvert_NVXXtoRGBX_i8u_o8u_getKernelInfo(NULL,
                                                                   &kernel_details[*bam_node_cnt].kernel_info);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "colorConvert_NVXXtoRGBX_i8u_o8u, kernel_params is null or the size is not as expected\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
            else if ((((vx_df_image)VX_DF_IMAGE_NV12 == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_NV21 == src->format)) &&
                     ((vx_df_image)VX_DF_IMAGE_YUV4 == dst->format))
            {
                BAM_VXLIB_colorConvert_NVXXtoYUV4_i8u_o8u_params *kernel_params = (BAM_VXLIB_colorConvert_NVXXtoYUV4_i8u_o8u_params*)scratch;

                if ((NULL != kernel_params) &&
                    (*size >= (int32_t)sizeof(BAM_VXLIB_colorConvert_NVXXtoYUV4_i8u_o8u_params)))
                {
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoYUV4_I8U_O8U;

                    kernel_params->u_pix = (src->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;

                    kernel_details[*bam_node_cnt].compute_kernel_params = (void*)kernel_params;

                    BAM_VXLIB_colorConvert_NVXXtoYUV4_i8u_o8u_getKernelInfo(NULL,
                                                                   &kernel_details[*bam_node_cnt].kernel_info);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "colorConvert_NVXXtoYUV4_i8u_o8u, kernel_params is null or the size is not as expected\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
            else if ((((vx_df_image)VX_DF_IMAGE_NV12 == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_NV21 == src->format)) &&
                     ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format))
            {
                BAM_VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u_params *kernel_params = (BAM_VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u_params*)scratch;

                if ((NULL != kernel_params) &&
                    (*size >= (int32_t)sizeof(BAM_VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u_params)))
                {
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoIYUV_I8U_O8U;

                    kernel_params->u_pix = (src->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;

                    kernel_details[*bam_node_cnt].compute_kernel_params = (void*)kernel_params;

                    BAM_VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u_getKernelInfo(NULL,
                                                                   &kernel_details[*bam_node_cnt].kernel_info);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "colorConvert_NVXXtoIYUV_i8u_o8u, kernel_params is null or the size is not as expected\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
            else if ((((vx_df_image)VX_DF_IMAGE_YUYV == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_UYVY == src->format)) &&
                     ((vx_df_image)VX_DF_IMAGE_RGB == dst->format))
            {
                BAM_VXLIB_colorConvert_YUVXtoRGB_i8u_o8u_params *kernel_params = (BAM_VXLIB_colorConvert_YUVXtoRGB_i8u_o8u_params*)scratch;

                if ((NULL != kernel_params) &&
                    (*size >= (int32_t)sizeof(BAM_VXLIB_colorConvert_YUVXtoRGB_i8u_o8u_params)))
                {
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoRGB_I8U_O8U;

                    if ((vx_df_image)VX_DF_IMAGE_YUYV == src->format)
                    {
                        kernel_params->x_value = 0;
                    }
                    else
                    {
                        kernel_params->x_value = 1;
                    }
                    kernel_params->src_space = (uint8_t)src->color_space -
                        (uint8_t)VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);

                    kernel_details[*bam_node_cnt].compute_kernel_params = (void*)kernel_params;

                    BAM_VXLIB_colorConvert_YUVXtoRGB_i8u_o8u_getKernelInfo(NULL,
                                                                   &kernel_details[*bam_node_cnt].kernel_info);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "colorConvert_YUVXtoRGB_i8u_o8u, kernel_params is null or the size is not as expected\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
            else if ((((vx_df_image)VX_DF_IMAGE_YUYV == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_UYVY == src->format)) &&
                     ((vx_df_image)VX_DF_IMAGE_RGBX == dst->format))
            {
                BAM_VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u_params *kernel_params = (BAM_VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u_params*)scratch;

                if ((NULL != kernel_params) &&
                    (*size >= (int32_t)sizeof(BAM_VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u_params)))
                {
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoRGBX_I8U_O8U;

                    if ((vx_df_image)VX_DF_IMAGE_YUYV == src->format)
                    {
                        kernel_params->x_value = 0;
                    }
                    else
                    {
                        kernel_params->x_value = 1;
                    }
                    kernel_params->src_space = (uint8_t)src->color_space -
                        (uint8_t)VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);

                    kernel_details[*bam_node_cnt].compute_kernel_params = (void*)kernel_params;

                    BAM_VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u_getKernelInfo(NULL,
                                                                   &kernel_details[*bam_node_cnt].kernel_info);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "colorConvert_YUVXtoRGBX_i8u_o8u, kernel_params is null or the size is not as expected\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
            else if ((((vx_df_image)VX_DF_IMAGE_YUYV == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_UYVY == src->format))&&
                     ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format))
            {
                BAM_VXLIB_colorConvert_YUVXtoNV12_i8u_o8u_params *kernel_params = (BAM_VXLIB_colorConvert_YUVXtoNV12_i8u_o8u_params*)scratch;

                if ((NULL != kernel_params) &&
                    (*size >= (int32_t)sizeof(BAM_VXLIB_colorConvert_YUVXtoNV12_i8u_o8u_params)))
                {
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoNV12_I8U_O8U;

                    if ((vx_df_image)VX_DF_IMAGE_YUYV == src->format)
                    {
                        kernel_params->x_value = 0;
                    }
                    else
                    {
                        kernel_params->x_value = 1;
                    }

                    kernel_details[*bam_node_cnt].compute_kernel_params = (void*)kernel_params;

                    BAM_VXLIB_colorConvert_YUVXtoNV12_i8u_o8u_getKernelInfo(NULL,
                                                                   &kernel_details[*bam_node_cnt].kernel_info);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "colorConvert_YUVXtoNV12_i8u_o8u, kernel_params is null or the size is not as expected\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
            else if ((((vx_df_image)VX_DF_IMAGE_YUYV == src->format) ||
                      ((vx_df_image)VX_DF_IMAGE_UYVY == src->format)) &&
                     ((vx_df_image)VX_DF_IMAGE_IYUV == dst->format))
            {
                BAM_VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u_params *kernel_params = (BAM_VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u_params*)scratch;

                if ((NULL != kernel_params) &&
                    (*size >= (int32_t)sizeof(BAM_VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u_params)))
                {
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoIYUV_I8U_O8U;

                    if ((vx_df_image)VX_DF_IMAGE_YUYV == src->format)
                    {
                        kernel_params->x_value = 0;
                    }
                    else
                    {
                        kernel_params->x_value = 1;
                    }

                    kernel_details[*bam_node_cnt].compute_kernel_params = (void*)kernel_params;

                    BAM_VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u_getKernelInfo(NULL,
                                                                   &kernel_details[*bam_node_cnt].kernel_info);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "colorConvert_YUVXtoIYUV_i8u_o8u, kernel_params is null or the size is not as expected\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
            else if (((vx_df_image)VX_DF_IMAGE_IYUV == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_RGB == dst->format))
            {
                BAM_VXLIB_colorConvert_IYUVtoRGB_i8u_o8u_params *kernel_params = (BAM_VXLIB_colorConvert_IYUVtoRGB_i8u_o8u_params*)scratch;

                if ((NULL != kernel_params) &&
                    (*size >= (int32_t)sizeof(BAM_VXLIB_colorConvert_IYUVtoRGB_i8u_o8u_params)))
                {
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoRGB_I8U_O8U;

                    kernel_params->src_space = (uint8_t)src->color_space -
                        (uint8_t)VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);

                    kernel_details[*bam_node_cnt].compute_kernel_params = (void*)kernel_params;

                    BAM_VXLIB_colorConvert_IYUVtoRGB_i8u_o8u_getKernelInfo(NULL,
                                                                   &kernel_details[*bam_node_cnt].kernel_info);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "colorConvert_IYUVtoRGB_i8u_o8u, kernel_params is null or the size is not as expected\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
            else if (((vx_df_image)VX_DF_IMAGE_IYUV == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_RGBX == dst->format))
            {
                BAM_VXLIB_colorConvert_IYUVtoRGBX_i8u_o8u_params *kernel_params = (BAM_VXLIB_colorConvert_IYUVtoRGBX_i8u_o8u_params*)scratch;

                if ((NULL != kernel_params) &&
                    (*size >= (int32_t)sizeof(BAM_VXLIB_colorConvert_IYUVtoRGBX_i8u_o8u_params)))
                {
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoRGBX_I8U_O8U;

                    kernel_params->src_space = (uint8_t)src->color_space -
                        (uint8_t)VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);

                    kernel_details[*bam_node_cnt].compute_kernel_params = (void*)kernel_params;

                    BAM_VXLIB_colorConvert_IYUVtoRGBX_i8u_o8u_getKernelInfo(NULL,
                                                                   &kernel_details[*bam_node_cnt].kernel_info);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "colorConvert_IYUVtoRGBX_i8u_o8u, kernel_params is null or the size is not as expected\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
            else if (((vx_df_image)VX_DF_IMAGE_IYUV == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_NV12 == dst->format))
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoNV12_I8U_O8U;

                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

                BAM_VXLIB_colorConvert_IYUVtoNV12_i8u_o8u_getKernelInfo(NULL,
                                                               &kernel_details[*bam_node_cnt].kernel_info);
            }
            else if (((vx_df_image)VX_DF_IMAGE_IYUV == src->format) &&
                     ((vx_df_image)VX_DF_IMAGE_YUV4 == dst->format))
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoYUV4_I8U_O8U;

                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

                BAM_VXLIB_colorConvert_IYUVtoYUV4_i8u_o8u_getKernelInfo(NULL,
                                                               &kernel_details[*bam_node_cnt].kernel_info);
            }
            else
            {
                /* do nothing */
            }
            prms->bam_node_num = (uint8_t)*bam_node_cnt;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "prms mem allocation failed\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxColorConvertParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxColorConvertParams), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelColorConvertGetNodePort(
    tivx_target_kernel_instance kernel,
    uint8_t ovx_port, uint8_t plane, uint8_t *bam_node, uint8_t *bam_port)
{
    tivxColorConvertParams *prms = NULL;
    uint32_t size;

    vx_status status = tivxGetTargetKernelInstanceContext(kernel,
                        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
        (sizeof(tivxColorConvertParams) == size))
    {
        *bam_node = prms->bam_node_num;

        switch (ovx_port)
        {
            case TIVX_KERNEL_COLOR_CONVERT_INPUT_IDX:
                switch (plane)
                {
                    case 0 :
                        *bam_port = 0;
                        break;
                    case 1 :
                        *bam_port = 1;
                        break;
                    case 2 :
                        *bam_port = 2;
                        break;
                    default:
                        VX_PRINT(VX_ZONE_ERROR, "non existing index plane queried by tivxKernelSupernodeCreate.tivxGetNodePort()\n");
                        status = (vx_status)VX_FAILURE;
                        break;
                }
                break;
            case TIVX_KERNEL_COLOR_CONVERT_OUTPUT_IDX:
                switch (plane)
                {
                    case 0 :
                        *bam_port = 0;
                        break;
                    case 1 :
                        *bam_port = 1;
                        break;
                    case 2 :
                        *bam_port = 2;
                        break;
                    default:
                        VX_PRINT(VX_ZONE_ERROR, "non existing index plane queried by tivxKernelSupernodeCreate.tivxGetNodePort()\n");
                        status = (vx_status)VX_FAILURE;
                        break;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "non existing index queried by tivxKernelSupernodeCreate.tivxGetNodePort()\n");
                status = (vx_status)VX_FAILURE;
                break;
        }
    }

    return status;
}
