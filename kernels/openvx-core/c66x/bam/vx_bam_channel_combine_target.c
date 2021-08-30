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
#include <tivx_kernel_channel_combine.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>
#include <edma_utils_memcpy.h>
#include "tivx_target_kernels_priv.h"

#define SOURCE_NODE1      0
#define CHCOPY_NODE0      1
#define CHCOPY_NODE1      2
#define CHCOPY_NODE2      3
#define SINK_NODE1        4

typedef struct
{
    tivx_bam_graph_handle graph_handle;
    uint8_t bam_node_num;
    uint8_t is_iyuv_yuv4;
    uint8_t switch_buffers;
    uint8_t is_nv12;
} tivxChannelCombineParams;

static tivx_target_kernel vx_channel_combine_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelChannelCombineProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelChannelCombineCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelChannelCombineDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

/* Supernode Callbacks */
static vx_status VX_CALLBACK tivxKernelChannelCombineCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size);

static vx_status VX_CALLBACK tivxKernelChannelCombineGetNodePort(
    tivx_target_kernel_instance kernel, uint8_t ovx_port, uint8_t plane,
    uint8_t *bam_node, uint8_t *bam_port);


static vx_status VX_CALLBACK tivxKernelChannelCombineProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxChannelCombineParams *prms = NULL;
    tivx_obj_desc_image_t *src0, *src1, *src2, *src3, *dst;
    uint8_t *src0_addr = NULL, *src1_addr = NULL, *src2_addr = NULL, *src3_addr = NULL, *dst_addr[4U] = {NULL};
    uint32_t size;
    uint16_t plane_idx;

    if ((num_params != TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE0_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE1_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX]))
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src0 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE0_IDX];
        src1 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE1_IDX];
        src2 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE2_IDX];
        src3 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE3_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxChannelCombineParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        void *img_ptrs[6];
        void *src0_target_ptr;
        void *src1_target_ptr;
        void *src2_target_ptr = NULL;
        void *src3_target_ptr = NULL;
        void *dst_target_ptr[4];

        /* Get the correct offset of the images from the valid roi parameter */
        src0_target_ptr = tivxMemShared2TargetPtr(&src0->mem_ptr[0]);
        tivxSetPointerLocation(src0, &src0_target_ptr, &src0_addr);

        src1_target_ptr = tivxMemShared2TargetPtr(&src1->mem_ptr[0]);
        tivxSetPointerLocation(src1, &src1_target_ptr, &src1_addr);
        if( src2 != NULL)
        {
            src2_target_ptr = tivxMemShared2TargetPtr(&src2->mem_ptr[0]);
            tivxSetPointerLocation(src2, &src2_target_ptr, &src2_addr);
        }
        if( src3 != NULL)
        {
            src3_target_ptr = tivxMemShared2TargetPtr(&src3->mem_ptr[0]);
            tivxSetPointerLocation(src3, &src3_target_ptr, &src3_addr);
        }
        for(plane_idx=0; plane_idx<dst->planes; plane_idx++)
        {
            dst_target_ptr[plane_idx] = tivxMemShared2TargetPtr(&dst->mem_ptr[plane_idx]);
        }
        tivxSetPointerLocation(dst, dst_target_ptr, (uint8_t**)&dst_addr);

        if ( (src2 != NULL) && (src3 != NULL) )
        {
            img_ptrs[0] = src0_addr;
            img_ptrs[1] = src1_addr;
            img_ptrs[2] = src2_addr;
            img_ptrs[3] = src3_addr;
            img_ptrs[4] = dst_addr[0U];
            tivxBamUpdatePointers(prms->graph_handle, 4U, 1U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else if (src2 != NULL)
        {
            if ((dst->format == (vx_df_image)VX_DF_IMAGE_RGB) ||
                (dst->format == (vx_df_image)VX_DF_IMAGE_YUYV) ||
                (dst->format == (vx_df_image)VX_DF_IMAGE_UYVY))
            {
                img_ptrs[0] = src0_addr;
                img_ptrs[1] = src1_addr;
                img_ptrs[2] = src2_addr;
                img_ptrs[3] = dst_addr[0U];
                tivxBamUpdatePointers(prms->graph_handle, 3U, 1U, img_ptrs);
                status  = tivxBamProcessGraph(prms->graph_handle);
            }
            else if (dst->format == (vx_df_image)VX_DF_IMAGE_YUV4 )
            {
                img_ptrs[0] = src0_addr;
                img_ptrs[1] = dst_addr[0U];
                tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);
                status  = tivxBamProcessGraph(prms->graph_handle);
                img_ptrs[0] = src1_addr;
                img_ptrs[1] = dst_addr[1U];
                tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);
                status  = tivxBamProcessGraph(prms->graph_handle);
                img_ptrs[0] = src2_addr;
                img_ptrs[1] = dst_addr[2U];
                tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);
                status  = tivxBamProcessGraph(prms->graph_handle);
            }
            else if (dst->format == (vx_df_image)VX_DF_IMAGE_IYUV )
            {
                img_ptrs[0] = src0_addr;
                img_ptrs[1] = src1_addr;
                img_ptrs[2] = src2_addr;
                img_ptrs[3] = dst_addr[0U];
                img_ptrs[4] = dst_addr[1U];
                img_ptrs[5] = dst_addr[2U];
                tivxBamUpdatePointers(prms->graph_handle, 3U, 3U, img_ptrs);
                status  = tivxBamProcessGraph(prms->graph_handle);
            }
            else if (dst->format == (vx_df_image)VX_DF_IMAGE_NV12 )
            {
                img_ptrs[0] = src1_addr;
                img_ptrs[1] = src2_addr;
                img_ptrs[2] = dst_addr[1U];
                EDMA_UTILS_memcpy2D(dst_addr[0U], src0_addr, (uint16_t)src0->imagepatch_addr[0U].dim_x,
                                    (uint16_t)src0->imagepatch_addr[0U].dim_y, (int16_t)dst->imagepatch_addr[0U].stride_y,
                                    (int16_t)src0->imagepatch_addr[0U].stride_y);
                tivxBamUpdatePointers(prms->graph_handle, 2U, 1U, img_ptrs);
                status  = tivxBamProcessGraph(prms->graph_handle);
            }
            else if(dst->format == (vx_df_image)VX_DF_IMAGE_NV21)
            {
                img_ptrs[0] = src2_addr;
                img_ptrs[1] = src1_addr;
                img_ptrs[2] = dst_addr[1U];
                EDMA_UTILS_memcpy2D(dst_addr[0U], src0_addr, (uint16_t)src0->imagepatch_addr[0U].dim_x,
                                    (uint16_t)src0->imagepatch_addr[0U].dim_y, (int16_t)dst->imagepatch_addr[0U].stride_y,
                                    (int16_t)src0->imagepatch_addr[0U].stride_y);
                tivxBamUpdatePointers(prms->graph_handle, 2U, 1U, img_ptrs);
                status  = tivxBamProcessGraph(prms->graph_handle);
            }
            else
            {
                /* do nothing */
            }
        }
        else
        {
            img_ptrs[0] = src0_addr;
            img_ptrs[1] = src1_addr;
            img_ptrs[2] = dst_addr[0U];
            tivxBamUpdatePointers(prms->graph_handle, 2U, 1U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
    }
    else
    {
        status = (vx_status)VX_ERROR_NO_MEMORY;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelChannelCombineCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src0, *src1, *src2, *src3, *dst;
    tivxChannelCombineParams *prms = NULL;
    tivx_bam_kernel_details_t kernel_details;
    uint16_t plane_idx = 0;

    if ((num_params != TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE0_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE1_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX]))
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxBamInitKernelDetails(&kernel_details, 1, kernel);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src0 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE0_IDX];
        src1 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE1_IDX];
        src2 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE2_IDX];
        src3 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE3_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX];

        prms = tivxMemAlloc(sizeof(tivxChannelCombineParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            VXLIB_bufParams2D_t vxlib_src0, vxlib_src1, vxlib_src2, vxlib_src3, vxlib_dst[3], vxlib_dst1;
            VXLIB_bufParams2D_t *buf_params[6];

            memset(prms, 0, sizeof(tivxChannelCombineParams));

            tivxInitBufParams(src0, &vxlib_src0);
            tivxInitBufParams(src1, &vxlib_src1);
            tivxInitBufParams(dst, &vxlib_dst[0]);

            /* Fill in the frame level sizes of buffers here. If the port
             * is optionally disabled, put NULL */
            buf_params[0] = &vxlib_src0;
            buf_params[1] = &vxlib_src1;

            if(src2 != NULL)
            {
                tivxInitBufParams(src2, &vxlib_src2);
                buf_params[2] = &vxlib_src2;
            }

            if(src3 != NULL)
            {
                tivxInitBufParams(src3, &vxlib_src3);
                buf_params[3] = &vxlib_src3;
            }

            if (   (dst->format == (vx_df_image)VX_DF_IMAGE_RGB)
                || (dst->format == (vx_df_image)VX_DF_IMAGE_RGBX)
                || (dst->format == (vx_df_image)VX_DF_IMAGE_YUYV)
                || (dst->format == (vx_df_image)VX_DF_IMAGE_UYVY)
                )
            {
                tivxInitBufParams(dst, &vxlib_dst[0]);

                if( dst->format == (vx_df_image)VX_DF_IMAGE_RGB)
                {
                    vxlib_dst[0].data_type = (uint32_t)VXLIB_UINT24;
                    buf_params[3] = &vxlib_dst[0];

                    BAM_VXLIB_channelCombine_3to1_i8u_o8u_getKernelInfo(NULL,
                        &kernel_details.kernel_info);

                    status = tivxBamCreateHandleSingleNode(
                        BAM_KERNELID_VXLIB_CHANNEL_COMBINE_3TO1_I8U_O8U, buf_params,
                        &kernel_details, &prms->graph_handle);
                }
                else
                if( dst->format == (vx_df_image)VX_DF_IMAGE_RGBX)
                {
                    vxlib_dst[0].data_type = (uint32_t)VXLIB_UINT32;
                    buf_params[4] = &vxlib_dst[0];

                    BAM_VXLIB_channelCombine_4to1_i8u_o8u_getKernelInfo(NULL,
                        &kernel_details.kernel_info);

                    status = tivxBamCreateHandleSingleNode(
                        BAM_KERNELID_VXLIB_CHANNEL_COMBINE_4TO1_I8U_O8U, buf_params,
                        &kernel_details, &prms->graph_handle);
                }
                else
                if( dst->format == (vx_df_image)VX_DF_IMAGE_YUYV)
                {
                    vxlib_dst[0].data_type = (uint32_t)VXLIB_UINT16;

                    BAM_VXLIB_channelCombine_yuyv_i8u_o8u_params kernel_params;

                    kernel_params.yidx = 0;

                    kernel_details.compute_kernel_params = (void*)&kernel_params;

                    buf_params[3] = &vxlib_dst[0];

                    BAM_VXLIB_channelCombine_yuyv_i8u_o8u_getKernelInfo(NULL,
                        &kernel_details.kernel_info);

                    status = tivxBamCreateHandleSingleNode(
                        BAM_KERNELID_VXLIB_CHANNEL_COMBINE_YUYV_I8U_O8U, buf_params,
                        &kernel_details, &prms->graph_handle);
                }
                else /* format is (vx_df_image)VX_DF_IMAGE_UYVY */
                {
                    vxlib_dst[0].data_type = (uint32_t)VXLIB_UINT16;

                    BAM_VXLIB_channelCombine_yuyv_i8u_o8u_params kernel_params;

                    kernel_params.yidx = 1;

                    kernel_details.compute_kernel_params = (void*)&kernel_params;

                    buf_params[3] = &vxlib_dst[0];

                    BAM_VXLIB_channelCombine_yuyv_i8u_o8u_getKernelInfo(NULL,
                        &kernel_details.kernel_info);

                    status = tivxBamCreateHandleSingleNode(
                        BAM_KERNELID_VXLIB_CHANNEL_COMBINE_YUYV_I8U_O8U, buf_params,
                        &kernel_details, &prms->graph_handle);
                }
            }
            else
            if (dst->format == (vx_df_image)VX_DF_IMAGE_YUV4)
            {
                for(plane_idx=0; plane_idx<dst->planes; plane_idx++)
                {
                    if (plane_idx==0U)
                    {
                        buf_params[0] = &vxlib_src0;
                    }
                    else if (plane_idx==1U)
                    {
                        buf_params[0] = &vxlib_src1;
                    }
                    else if (plane_idx==2U)
                    {
                        buf_params[0] = &vxlib_src2;
                    }
                    else
                    {
                        /* do nothing */
                    }

                    vxlib_dst[0].dim_x =
                        (dst->valid_roi.end_x - dst->valid_roi.start_x)
                        /dst->imagepatch_addr[plane_idx].step_x;
                    vxlib_dst[0].dim_y =
                        (dst->valid_roi.end_y - dst->valid_roi.start_y)
                        /dst->imagepatch_addr[plane_idx].step_y;
                    vxlib_dst[0].stride_y =
                        dst->imagepatch_addr[plane_idx].stride_y;
                    vxlib_dst[0].data_type = (uint32_t)VXLIB_UINT8;

                    buf_params[1] = &vxlib_dst[0];

                    BAM_VXLIB_channelCopy_1to1_i8u_o8u_getKernelInfo(NULL,
                        &kernel_details.kernel_info);

                    status = tivxBamCreateHandleSingleNode(
                        BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U, buf_params,
                        &kernel_details, &prms->graph_handle);
                }
            }
            else
            if (dst->format == (vx_df_image)VX_DF_IMAGE_IYUV)
            {
                tivx_bam_kernel_details_t multi_kernel_details[6];
                BAM_NodeParams node_list[] = { \
                    {SOURCE_NODE1, (uint32_t)BAM_KERNELID_DMAREAD_AUTOINCREMENT, NULL}, \
                    {CHCOPY_NODE0, (uint32_t)BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U, NULL}, \
                    {CHCOPY_NODE1, (uint32_t)BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U, NULL}, \
                    {CHCOPY_NODE2, (uint32_t)BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U, NULL}, \
                    {SINK_NODE1, (uint32_t)BAM_KERNELID_DMAWRITE_AUTOINCREMENT, NULL}, \
                    {BAM_END_NODE_MARKER,   0,                          NULL},\
                };

                BAM_EdgeParams edge_list[]= {\
                    {{SOURCE_NODE1, 0},
                        {CHCOPY_NODE0, (uint8_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT}},\

                    {{SOURCE_NODE1, 1},
                        {CHCOPY_NODE1, (uint8_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT}},\

                    {{SOURCE_NODE1, 2},
                        {CHCOPY_NODE2, (uint8_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT}},\

                    {{CHCOPY_NODE0, (uint8_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT},
                        {SINK_NODE1, 0}},\

                    {{CHCOPY_NODE1, (uint8_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT},
                        {SINK_NODE1, 1}},\

                    {{CHCOPY_NODE2, (uint8_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT},
                        {SINK_NODE1, 2}},\

                    {{BAM_END_NODE_MARKER, 0},
                        {BAM_END_NODE_MARKER, 0}},\
                };

                status = tivxBamInitKernelDetails(&multi_kernel_details[0], 6, kernel);

                if ((vx_status)VX_SUCCESS == status)
                {
                    BAM_VXLIB_channelCopy_1to1_i8u_o8u_getKernelInfo(NULL,
                        &multi_kernel_details[CHCOPY_NODE0].kernel_info);

                    BAM_VXLIB_channelCopy_1to1_i8u_o8u_getKernelInfo(NULL,
                        &multi_kernel_details[CHCOPY_NODE1].kernel_info);

                    multi_kernel_details[CHCOPY_NODE1].kernel_info.kernelExtraInfo.\
                        horzSamplingFactor[BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT] = 0.5f;
                    multi_kernel_details[CHCOPY_NODE1].kernel_info.kernelExtraInfo.\
                        vertSamplingFactor[BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT] = 0.5f;
                    multi_kernel_details[CHCOPY_NODE1].kernel_info.kernelExtraInfo.\
                        horzSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_NUM_INPUT_BLOCKS\
                                              + (uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT] = 0.5f;
                    multi_kernel_details[CHCOPY_NODE1].kernel_info.kernelExtraInfo.\
                        vertSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_NUM_INPUT_BLOCKS\
                                              + (uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT] = 0.5f;

                    BAM_VXLIB_channelCopy_1to1_i8u_o8u_getKernelInfo(NULL,
                        &multi_kernel_details[CHCOPY_NODE2].kernel_info);

                    multi_kernel_details[CHCOPY_NODE2].kernel_info.kernelExtraInfo.\
                        horzSamplingFactor[BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT] = 0.5f;
                    multi_kernel_details[CHCOPY_NODE2].kernel_info.kernelExtraInfo.\
                        vertSamplingFactor[BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT] = 0.5f;
                    multi_kernel_details[CHCOPY_NODE2].kernel_info.kernelExtraInfo.\
                        horzSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_NUM_INPUT_BLOCKS\
                                              + (uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT] = 0.5f;
                    multi_kernel_details[CHCOPY_NODE2].kernel_info.kernelExtraInfo.\
                        vertSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_NUM_INPUT_BLOCKS\
                                              + (uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT] = 0.5f;

                    multi_kernel_details[SOURCE_NODE1].compute_kernel_params = NULL;
                    multi_kernel_details[CHCOPY_NODE0].compute_kernel_params = NULL;
                    multi_kernel_details[CHCOPY_NODE1].compute_kernel_params = NULL;
                    multi_kernel_details[CHCOPY_NODE2].compute_kernel_params = NULL;
                    multi_kernel_details[SINK_NODE1].compute_kernel_params = NULL;

                    buf_params[0] = &vxlib_src0;
                    buf_params[1] = &vxlib_src1;
                    buf_params[2] = &vxlib_src2;
                    buf_params[3] = &vxlib_dst[0];
                    buf_params[4] = &vxlib_dst[1];
                    buf_params[5] = &vxlib_dst[2];

                    status = tivxBamCreateHandleMultiNode(node_list,
                        sizeof(node_list)/sizeof(BAM_NodeParams),
                        edge_list,
                        sizeof(edge_list)/sizeof(BAM_EdgeParams),
                        buf_params, multi_kernel_details,
                        &prms->graph_handle);
                }
            }
            else
            if ((dst->format == (vx_df_image)VX_DF_IMAGE_NV12)
                || (dst->format == (vx_df_image)VX_DF_IMAGE_NV21)
                )
            {
                buf_params[2] = &vxlib_dst1;

                vxlib_dst1.dim_x =
                    (dst->valid_roi.end_x - dst->valid_roi.start_x);
                vxlib_dst1.dim_y =
                    (dst->valid_roi.end_y - dst->valid_roi.start_y)/2U;
                vxlib_dst1.stride_y =
                    dst->imagepatch_addr[1].stride_y;
                vxlib_dst1.data_type = (uint32_t)VXLIB_UINT8;

                if(dst->format == (vx_df_image)VX_DF_IMAGE_NV21)
                {
                    buf_params[0] = &vxlib_src2;
                    buf_params[1] = &vxlib_src1;
                }
                else
                {
                    buf_params[0] = &vxlib_src1;
                    buf_params[1] = &vxlib_src2;
                }

                BAM_VXLIB_channelCombine_2to1_i8u_o8u_getKernelInfo(NULL,
                        &kernel_details.kernel_info);

                kernel_details.kernel_info.kernelExtraInfo.\
                    horzSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELCOMBINE_2TO1_I8U_O8U_NUM_INPUT_BLOCKS\
                                          + (uint32_t)BAM_VXLIB_CHANNELCOMBINE_2TO1_I8U_O8U_OUTPUT_PORT] = 2.0f;

                status = tivxBamCreateHandleSingleNode(
                        BAM_KERNELID_VXLIB_CHANNEL_COMBINE_2TO1_I8U_O8U, buf_params,
                        &kernel_details, &prms->graph_handle);
            }
            else
            {
                status = (vx_status)VX_FAILURE;
            }
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxChannelCombineParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxChannelCombineParams), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }
    return status;
}

static vx_status VX_CALLBACK tivxKernelChannelCombineDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t size;
    tivxChannelCombineParams *prms = NULL;

    if ((num_params != TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE0_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE1_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX]))
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxChannelCombineParams) == size))
        {
            if(NULL != prms->graph_handle)
            {
                tivxBamDestroyHandle(prms->graph_handle);
            }
            tivxMemFree(prms, sizeof(tivxChannelCombineParams), (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

void tivxAddTargetKernelBamChannelCombine(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_channel_combine_target_kernel = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_CHANNEL_COMBINE,
            target_name,
            tivxKernelChannelCombineProcess,
            tivxKernelChannelCombineCreate,
            tivxKernelChannelCombineDelete,
            NULL,
            NULL);

        tivxEnableKernelForSuperNode(vx_channel_combine_target_kernel,
            tivxKernelChannelCombineCreateInBamGraph,
            tivxKernelChannelCombineGetNodePort,
            NULL,
            NULL,
            NULL,
            (int32_t)sizeof(BAM_VXLIB_channelCombine_yuyv_i8u_o8u_params),
            NULL);
    }
}


void tivxRemoveTargetKernelBamChannelCombine(void)
{
    tivxRemoveTargetKernel(vx_channel_combine_target_kernel);
}

static vx_status VX_CALLBACK tivxKernelChannelCombineCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *dst;
    tivxChannelCombineParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    if ((num_params != TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE0_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE1_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX]))
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX];

        prms = tivxMemAlloc(sizeof(tivxChannelCombineParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxChannelCombineParams));

            if (   (dst->format == (vx_df_image)VX_DF_IMAGE_RGB)
                || (dst->format == (vx_df_image)VX_DF_IMAGE_RGBX)
                || (dst->format == (vx_df_image)VX_DF_IMAGE_YUYV)
                || (dst->format == (vx_df_image)VX_DF_IMAGE_UYVY)
                )
            {
                node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
                node_list[*bam_node_cnt].kernelArgs = NULL;

                if( dst->format == (vx_df_image)VX_DF_IMAGE_RGB)
                {
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNEL_COMBINE_3TO1_I8U_O8U;

                    BAM_VXLIB_channelCombine_3to1_i8u_o8u_getKernelInfo(NULL,
                        &kernel_details[*bam_node_cnt].kernel_info);

                    kernel_details[*bam_node_cnt].compute_kernel_params = NULL;
                }
                else
                if( dst->format == (vx_df_image)VX_DF_IMAGE_RGBX)
                {
                    node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNEL_COMBINE_4TO1_I8U_O8U;

                    BAM_VXLIB_channelCombine_4to1_i8u_o8u_getKernelInfo(NULL,
                        &kernel_details[*bam_node_cnt].kernel_info);

                    kernel_details[*bam_node_cnt].compute_kernel_params = NULL;
                }
                else
                if( (dst->format == (vx_df_image)VX_DF_IMAGE_YUYV) || (dst->format == (vx_df_image)VX_DF_IMAGE_UYVY))
                {
                    BAM_VXLIB_channelCombine_yuyv_i8u_o8u_params *kernel_params = (BAM_VXLIB_channelCombine_yuyv_i8u_o8u_params*)scratch;

                    if ((NULL != kernel_params) &&
                        (*size >= (int32_t)sizeof(BAM_VXLIB_channelCombine_yuyv_i8u_o8u_params)))
                    {
                        node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNEL_COMBINE_YUYV_I8U_O8U;

                        if (dst->format == (vx_df_image)VX_DF_IMAGE_YUYV) {
                            kernel_params->yidx = 0;
                        }
                        else
                        {
                            kernel_params->yidx = 1;
                        }
                        kernel_details[*bam_node_cnt].compute_kernel_params = (void*)kernel_params;

                        BAM_VXLIB_channelCombine_yuyv_i8u_o8u_getKernelInfo(NULL,
                                                                       &kernel_details[*bam_node_cnt].kernel_info);
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "channelCombine_yuyv_i8u_o8u, kernel_params is null or the size is not as expected\n");
                        status = (vx_status)VX_FAILURE;
                    }

                }
                else
                {
                    /* do nothing */
                }
                prms->bam_node_num = (uint8_t)*bam_node_cnt;
            }
            else if ((dst->format == (vx_df_image)VX_DF_IMAGE_YUV4) || (dst->format == (vx_df_image)VX_DF_IMAGE_IYUV))
            {
                node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U;
                node_list[*bam_node_cnt].kernelArgs = NULL;
                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;
                BAM_VXLIB_channelCopy_1to1_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details[*bam_node_cnt].kernel_info);
                *bam_node_cnt = *bam_node_cnt + 1;

                node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U;
                node_list[*bam_node_cnt].kernelArgs = NULL;
                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;
                BAM_VXLIB_channelCopy_1to1_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details[*bam_node_cnt].kernel_info);

                if (dst->format == (vx_df_image)VX_DF_IMAGE_IYUV)
                {
                    kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                        horzSamplingFactor[BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT] = 0.5f;
                    kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                        vertSamplingFactor[BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT] = 0.5f;
                    kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                        horzSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_NUM_INPUT_BLOCKS\
                                              + (uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT] = 0.5f;
                    kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                        vertSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_NUM_INPUT_BLOCKS\
                                              + (uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT] = 0.5f;
                }
                *bam_node_cnt = *bam_node_cnt + 1;

                node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U;
                node_list[*bam_node_cnt].kernelArgs = NULL;
                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;
                BAM_VXLIB_channelCopy_1to1_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details[*bam_node_cnt].kernel_info);

                if (dst->format == (vx_df_image)VX_DF_IMAGE_IYUV)
                {
                    kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                        horzSamplingFactor[BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT] = 0.5f;
                    kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                        vertSamplingFactor[BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT] = 0.5f;
                    kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                        horzSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_NUM_INPUT_BLOCKS\
                                              + (uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT] = 0.5f;
                    kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                        vertSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_NUM_INPUT_BLOCKS\
                                              + (uint32_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT] = 0.5f;
                }
                prms->bam_node_num = (uint8_t)*bam_node_cnt;
                prms->is_iyuv_yuv4 = 1;
            }
            else if ((dst->format == (vx_df_image)VX_DF_IMAGE_NV12) ||
                     (dst->format == (vx_df_image)VX_DF_IMAGE_NV21))
            {
                node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U;
                node_list[*bam_node_cnt].kernelArgs = NULL;
                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;
                BAM_VXLIB_channelCopy_1to1_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details[*bam_node_cnt].kernel_info);
                *bam_node_cnt = *bam_node_cnt + 1;

                node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
                node_list[*bam_node_cnt].kernelArgs = NULL;
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNEL_COMBINE_2TO1_I8U_O8U;
                kernel_details[*bam_node_cnt].compute_kernel_params = NULL;
                BAM_VXLIB_channelCombine_2to1_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details[*bam_node_cnt].kernel_info);

                kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                    horzSamplingFactor[BAM_VXLIB_CHANNELCOMBINE_2TO1_I8U_O8U_INPUT0_IMAGE_PORT] = 0.5f;
                kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                    vertSamplingFactor[BAM_VXLIB_CHANNELCOMBINE_2TO1_I8U_O8U_INPUT0_IMAGE_PORT] = 0.5f;
                kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                    horzSamplingFactor[BAM_VXLIB_CHANNELCOMBINE_2TO1_I8U_O8U_INPUT1_IMAGE_PORT] = 0.5f;
                kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                    vertSamplingFactor[BAM_VXLIB_CHANNELCOMBINE_2TO1_I8U_O8U_INPUT1_IMAGE_PORT] = 0.5f;
                kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                    vertSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELCOMBINE_2TO1_I8U_O8U_NUM_INPUT_BLOCKS\
                                          + (uint32_t)BAM_VXLIB_CHANNELCOMBINE_2TO1_I8U_O8U_OUTPUT_PORT] = 0.5f;
                kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                    typeOutputElmt[BAM_VXLIB_CHANNELCOMBINE_2TO1_I8U_O8U_OUTPUT_PORT] = (uint8_t)VXLIB_UINT8;

                if( dst->format == (vx_df_image)VX_DF_IMAGE_NV12)
                {
                    prms->switch_buffers = 0U;
                }
                else
                {
                    prms->switch_buffers = 1U;
                }
                prms->bam_node_num = (uint8_t)*bam_node_cnt;
                prms->is_nv12 = 1U;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "unsupported output format found\n");
                status = (vx_status)VX_FAILURE;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "prms mem allocation failed\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxChannelCombineParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxChannelCombineParams), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}
/*
static vx_status VX_CALLBACK tivxKernelChannelCombineAppendInternalEdges(
    tivx_target_kernel_instance kernel, BAM_EdgeParams edge_list[], int32_t *bam_edge_cnt)
{
    tivxChannelCombineParams *prms = NULL;
    uint32_t size;

    vx_status status = tivxGetTargetKernelInstanceContext(kernel,
                        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
        (sizeof(tivxChannelCombineParams) == size))
    {
                        BAM_EdgeParams edge_list[]= {\
                        {{SOURCE_NODE1, 0},
                            {CHCOPY_NODE0, BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT}},\

                        {{SOURCE_NODE1, 1},
                            {CHCOPY_NODE1, BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT}},\

                        {{SOURCE_NODE1, 2},
                            {CHCOPY_NODE2, BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT}},\

                        {{CHCOPY_NODE0, BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT},
                            {SINK_NODE1, 0}},\

                        {{CHCOPY_NODE1, BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT},
                            {SINK_NODE1, 1}},\

                        {{CHCOPY_NODE2, BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT},
                            {SINK_NODE1, 2}},\

                    edge_list[*bam_edge_cnt].upStreamNode.id = prms->bam_node_num - 2;
                    edge_list[*bam_edge_cnt].upStreamNode.port);
                    edge_list[*bam_edge_cnt].downStreamNode.id = prms->bam_node_num - 2;
                    edge_list[*bam_edge_cnt].downStreamNode.port = BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT;
                    *bam_edge_cnt = *bam_edge_cnt + 1;

    }

    return status;
}
*/

static vx_status VX_CALLBACK tivxKernelChannelCombineGetNodePort(
    tivx_target_kernel_instance kernel,
    uint8_t ovx_port, uint8_t plane, uint8_t *bam_node, uint8_t *bam_port)
{
    tivxChannelCombineParams *prms = NULL;
    uint32_t size;

    vx_status status = tivxGetTargetKernelInstanceContext(kernel,
                        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
        (sizeof(tivxChannelCombineParams) == size))
    {
        switch (ovx_port)
        {
            case TIVX_KERNEL_CHANNEL_COMBINE_PLANE0_IDX:

                *bam_node = prms->bam_node_num;
                *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOMBINE_4TO1_I8U_O8U_INPUT0_IMAGE_PORT;

                if (prms->is_iyuv_yuv4 != 0U)
                {
                    *bam_node = prms->bam_node_num - 2U;
                    *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT;
                }

                if (prms->is_nv12 != 0U)
                {
                    *bam_node = prms->bam_node_num - 1U;
                    *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT;
                }
                break;
            case TIVX_KERNEL_CHANNEL_COMBINE_PLANE1_IDX:

                *bam_node = prms->bam_node_num;
                *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOMBINE_4TO1_I8U_O8U_INPUT1_IMAGE_PORT;

                if (prms->is_iyuv_yuv4 != 0U)
                {
                    *bam_node = prms->bam_node_num - 1U;
                    *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT;
                }

                if (prms->is_nv12 != 0U)
                {
                    if (prms->switch_buffers != 0U)
                    {
                        *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOMBINE_2TO1_I8U_O8U_INPUT1_IMAGE_PORT;
                    }
                    else
                    {
                        *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOMBINE_2TO1_I8U_O8U_INPUT0_IMAGE_PORT;
                    }
                }
                break;
            case TIVX_KERNEL_CHANNEL_COMBINE_PLANE2_IDX:

                *bam_node = prms->bam_node_num;
                *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOMBINE_4TO1_I8U_O8U_INPUT2_IMAGE_PORT;

                if (prms->is_iyuv_yuv4 != 0U)
                {
                    *bam_node = prms->bam_node_num;
                    *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_INPUT_IMAGE_PORT;
                }

                if (prms->is_nv12 != 0U)
                {
                    if (prms->switch_buffers != 0U)
                    {
                        *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOMBINE_2TO1_I8U_O8U_INPUT0_IMAGE_PORT;
                    }
                    else
                    {
                        *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOMBINE_2TO1_I8U_O8U_INPUT1_IMAGE_PORT;
                    }
                }
                break;
            case TIVX_KERNEL_CHANNEL_COMBINE_PLANE3_IDX:

                *bam_node = prms->bam_node_num;
                *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOMBINE_4TO1_I8U_O8U_INPUT3_IMAGE_PORT;
                break;
            case TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX:

                *bam_node = prms->bam_node_num;

                switch (plane)
                {
                    case 0 :

                        *bam_port = 0;

                        if (prms->is_iyuv_yuv4 != 0U)
                        {
                            *bam_node = prms->bam_node_num - 2U;
                            *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT;
                        }
                        if (prms->is_nv12 != 0U)
                        {
                            *bam_node = prms->bam_node_num - 1U;
                            *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT;
                        }
                        break;
                    case 1 :

                        *bam_port = 1;

                        if (prms->is_iyuv_yuv4 != 0U)
                        {
                            *bam_node = prms->bam_node_num - 1U;
                            *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT;
                        }
                        if (prms->is_nv12 != 0U)
                        {
                            *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOMBINE_2TO1_I8U_O8U_OUTPUT_PORT;
                        }
                        break;
                    case 2 :

                        *bam_port = 2;

                        if (prms->is_iyuv_yuv4 != 0U)
                        {
                            *bam_port = (uint8_t)BAM_VXLIB_CHANNELCOPY_1TO1_I8U_IO8U_OUTPUT_PORT;
                        }
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
