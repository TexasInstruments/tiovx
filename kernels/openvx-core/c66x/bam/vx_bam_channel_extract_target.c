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
#include <tivx_kernel_channel_extract.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>
#include "tivx_target_kernels_priv.h"

typedef struct
{
    uint8_t channel_offset, plane_idx;
    tivx_bam_graph_handle graph_handle;
    uint8_t bam_node_num;
} tivxBamChannelExtractParams;

static tivx_target_kernel vx_channel_extract_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelBamChannelExtractProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelBamChannelExtractCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelBamChannelExtractDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status tivxBamChannelExtractIyuvYuv4Input(
    tivx_target_kernel_instance kernel, tivxBamChannelExtractParams *prms,
    const tivx_obj_desc_image_t *src, const tivx_obj_desc_image_t *dst,
    const tivx_obj_desc_scalar_t *ch);
static vx_status tivxBamChannelExtractNv12Nv21Input(
    tivx_target_kernel_instance kernel, tivxBamChannelExtractParams *prms,
    const tivx_obj_desc_image_t *src, const tivx_obj_desc_image_t *dst,
    const tivx_obj_desc_scalar_t *ch);
static vx_status tivxBamChannelExtractYuyvUyvyInput(
    tivx_target_kernel_instance kernel, tivxBamChannelExtractParams *prms,
    const tivx_obj_desc_image_t *src, const tivx_obj_desc_image_t *dst,
    const tivx_obj_desc_scalar_t *ch);
static vx_status tivxBamChannelExtractRgbRgbxInput(
    tivx_target_kernel_instance kernel, tivxBamChannelExtractParams *prms,
    const tivx_obj_desc_image_t *src, const tivx_obj_desc_image_t *dst,
    const tivx_obj_desc_scalar_t *ch);
static vx_status tivxBamChannelExtractInBamGraphIyuvYuv4Input(
    tivxBamChannelExtractParams *prms, const tivx_obj_desc_image_t *src,
    const tivx_obj_desc_scalar_t *ch, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    const int32_t * bam_node_cnt, void * scratch, const int32_t *size);
static vx_status tivxBamChannelExtractInBamGraphNv12Nv21Input(
    tivxBamChannelExtractParams *prms, const tivx_obj_desc_image_t *src,
    const tivx_obj_desc_scalar_t *ch, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    const int32_t * bam_node_cnt, void * scratch, const int32_t *size);
static vx_status tivxBamChannelExtractInBamGraphYuyvUyvyInput(
    tivxBamChannelExtractParams *prms, const tivx_obj_desc_image_t *src,
    const tivx_obj_desc_scalar_t *ch, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    const int32_t * bam_node_cnt, void * scratch, const int32_t *size);
static vx_status tivxBamChannelExtractInBamGraphRgbRgbxInput(
    tivxBamChannelExtractParams *prms, const tivx_obj_desc_image_t *src,
    const tivx_obj_desc_scalar_t *ch, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    const int32_t * bam_node_cnt, void * scratch, const int32_t *size);

/* Supernode Callbacks */
static vx_status VX_CALLBACK tivxKernelChannelExtractCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size);
static vx_status VX_CALLBACK tivxKernelChannelExtractGetNodePort(
    tivx_target_kernel_instance kernel, uint8_t ovx_port, uint8_t plane,
    uint8_t *bam_node, uint8_t *bam_port);

static vx_status tivxBamChannelExtractIyuvYuv4Input(
    tivx_target_kernel_instance kernel, tivxBamChannelExtractParams *prms,
    const tivx_obj_desc_image_t *src, const tivx_obj_desc_image_t *dst,
    const tivx_obj_desc_scalar_t *ch)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint8_t channel_offset = 0, plane_idx;
    tivx_bam_kernel_details_t kernel_details;
    VXLIB_bufParams2D_t vxlib_src[3], vxlib_dst;
    VXLIB_bufParams2D_t *buf_params[2];

    tivxInitBufParams(src, &vxlib_src[0]);
    tivxInitBufParams(dst, &vxlib_dst);

    if ((vx_enum)VX_CHANNEL_Y == ch->data.enm)
    {
        plane_idx = 0;
        buf_params[0] = &vxlib_src[0];
    }
    else if ((vx_enum)VX_CHANNEL_U == ch->data.enm)
    {
        plane_idx = 1;
        buf_params[0] = &vxlib_src[1];
    }
    else
    {
        plane_idx = 2;
        buf_params[0] = &vxlib_src[2];
    }

    status = tivxBamInitKernelDetails(&kernel_details, 1, kernel);

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Fill in the frame level sizes of buffers here. If the port
         * is optionally disabled, put NULL */
        buf_params[1] = &vxlib_dst;

        BAM_VXLIB_channelCopy_1to1_i8u_o8u_getKernelInfo(NULL,
            &kernel_details.kernel_info);

        status = tivxBamCreateHandleSingleNode(
            BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U, buf_params,
            &kernel_details, &prms->graph_handle);
    }

    prms->channel_offset = channel_offset;
    prms->plane_idx = plane_idx;

    return (status);
}

static vx_status tivxBamChannelExtractNv12Nv21Input(
    tivx_target_kernel_instance kernel, tivxBamChannelExtractParams *prms,
    const tivx_obj_desc_image_t *src, const tivx_obj_desc_image_t *dst,
    const tivx_obj_desc_scalar_t *ch)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint8_t channel_offset = 0, plane_idx = 0;
    tivx_bam_kernel_details_t kernel_details;
    VXLIB_bufParams2D_t vxlib_src[2], vxlib_dst;
    VXLIB_bufParams2D_t *buf_params[2];

    tivxInitBufParams(src, &vxlib_src[0]);
    tivxInitBufParams(dst, &vxlib_dst);

    if ((vx_enum)VX_CHANNEL_Y == ch->data.enm)
    {
        plane_idx = 0;
        buf_params[0] = &vxlib_src[0];
    }
    else
    {
        plane_idx = 1;
        buf_params[0] = &vxlib_src[1];
    }

    switch(ch->data.enm)
    {
        case (vx_enum)VX_CHANNEL_Y:
            channel_offset = 0;
            break;
        case (vx_enum)VX_CHANNEL_U:
            if(src->format == (vx_df_image)VX_DF_IMAGE_NV12)
            {
                channel_offset = 0;
            }
            else
            {
                channel_offset = 1;
            }
            break;
        case (vx_enum)VX_CHANNEL_V:
            if(src->format == (vx_df_image)VX_DF_IMAGE_NV12)
            {
                channel_offset = 1;
            }
            else
            {
                channel_offset = 0;
            }
            break;
        default:
            status = (vx_status)VX_FAILURE;
            break;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxBamInitKernelDetails(&kernel_details, 1, kernel);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Fill in the frame level sizes of buffers here. If the port
         * is optionally disabled, put NULL */
        buf_params[1] = &vxlib_dst;

        if(ch->data.enm == (vx_enum)VX_CHANNEL_Y)
        {
            BAM_VXLIB_channelCopy_1to1_i8u_o8u_getKernelInfo(NULL,
                &kernel_details.kernel_info);

            status = tivxBamCreateHandleSingleNode(
                BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U, buf_params,
                &kernel_details, &prms->graph_handle);
        }
        else
        {
            BAM_VXLIB_channelExtract_1of2_i8u_o8u_params params;

            params.channel_offset = channel_offset;

            BAM_VXLIB_channelExtract_1of2_i8u_o8u_getKernelInfo(NULL,
                &kernel_details.kernel_info);

                kernel_details.kernel_info.kernelExtraInfo.\
                    vertSamplingFactor[BAM_VXLIB_CHANNELEXTRACT_1OF2_I8U_IO8U_INPUT_IMAGE_PORT] = 0.5f;
                kernel_details.kernel_info.kernelExtraInfo.\
                    horzSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELEXTRACT_1OF2_I8U_IO8U_NUM_INPUT_BLOCKS\
                                          + (uint32_t)BAM_VXLIB_CHANNELEXTRACT_1OF2_I8U_IO8U_OUTPUT_PORT] = 0.5f;
                kernel_details.kernel_info.kernelExtraInfo.\
                    vertSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELEXTRACT_1OF2_I8U_IO8U_NUM_INPUT_BLOCKS\
                                          + (uint32_t)BAM_VXLIB_CHANNELEXTRACT_1OF2_I8U_IO8U_OUTPUT_PORT] = 0.5f;
            kernel_details.compute_kernel_params = (void *)&params;

            status = tivxBamCreateHandleSingleNode(
                BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF2_I8U_O8U, buf_params,
                &kernel_details, &prms->graph_handle);
        }
    }

    prms->channel_offset = channel_offset;
    prms->plane_idx = plane_idx;

    return (status);
}

static vx_status tivxBamChannelExtractYuyvUyvyInput(
    tivx_target_kernel_instance kernel, tivxBamChannelExtractParams *prms,
    const tivx_obj_desc_image_t *src, const tivx_obj_desc_image_t *dst,
    const tivx_obj_desc_scalar_t *ch)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint8_t channel_offset = 0;
    tivx_bam_kernel_details_t kernel_details;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    VXLIB_bufParams2D_t *buf_params[2];

    tivxInitBufParams(src, &vxlib_src);
    tivxInitBufParams(dst, &vxlib_dst);

    switch(ch->data.enm)
    {
        case (vx_enum)VX_CHANNEL_Y:
            if(src->format == (vx_df_image)VX_DF_IMAGE_YUYV)
            {
                channel_offset = 0;
            }
            else
            {
                channel_offset = 1;
            }
            break;
        case (vx_enum)VX_CHANNEL_U:
            if(src->format == (vx_df_image)VX_DF_IMAGE_YUYV)
            {
                channel_offset = 1;
            }
            else
            {
                channel_offset = 0;
            }
            break;
        case (vx_enum)VX_CHANNEL_V:
            if(src->format == (vx_df_image)VX_DF_IMAGE_YUYV)
            {
                channel_offset = 3;
            }
            else
            {
                channel_offset = 2;
            }
            break;
        default:
            status = (vx_status)VX_FAILURE;
            break;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxBamInitKernelDetails(&kernel_details, 1, kernel);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Fill in the frame level sizes of buffers here. If the port
         * is optionally disabled, put NULL */
        buf_params[0] = &vxlib_src;
        buf_params[1] = &vxlib_dst;

        if(ch->data.enm == (vx_enum)VX_CHANNEL_Y)
        {
            BAM_VXLIB_channelExtract_1of2_i8u_o8u_params params;

            params.channel_offset = channel_offset;

            kernel_details.compute_kernel_params = (void *)&params;

            BAM_VXLIB_channelExtract_1of2_i8u_o8u_getKernelInfo(NULL,
                &kernel_details.kernel_info);

            status = tivxBamCreateHandleSingleNode(
                BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF2_I8U_O8U, buf_params,
                &kernel_details, &prms->graph_handle);
        }
        else
        {
            BAM_VXLIB_channelExtract_1of4_i8u_o8u_params params;

            params.channel_offset = channel_offset;

            kernel_details.compute_kernel_params = (void *)&params;

            BAM_VXLIB_channelExtract_1of4_i8u_o8u_getKernelInfo(NULL,
                &kernel_details.kernel_info);

            kernel_details.kernel_info.kernelExtraInfo.\
                horzSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELEXTRACT_1OF4_I8U_IO8U_NUM_INPUT_BLOCKS\
                                      + (uint32_t)BAM_VXLIB_CHANNELEXTRACT_1OF4_I8U_IO8U_OUTPUT_PORT] = 0.5f;

            status = tivxBamCreateHandleSingleNode(
                BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF4_I8U_O8U, buf_params,
                &kernel_details, &prms->graph_handle);
        }
    }

    prms->channel_offset = channel_offset;
    prms->plane_idx = 0;

    return (status);
}

static vx_status tivxBamChannelExtractRgbRgbxInput(
    tivx_target_kernel_instance kernel, tivxBamChannelExtractParams *prms,
    const tivx_obj_desc_image_t *src, const tivx_obj_desc_image_t *dst,
    const tivx_obj_desc_scalar_t *ch)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint8_t channel_offset = 0;
    tivx_bam_kernel_details_t kernel_details;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    VXLIB_bufParams2D_t *buf_params[2];

    tivxInitBufParams(src, &vxlib_src);
    tivxInitBufParams(dst, &vxlib_dst);

    switch(ch->data.enm)
    {
        case (vx_enum)VX_CHANNEL_0:
        case (vx_enum)VX_CHANNEL_R:
            channel_offset = 0;
            break;
        case (vx_enum)VX_CHANNEL_1:
        case (vx_enum)VX_CHANNEL_G:
            channel_offset = 1;
            break;
        case (vx_enum)VX_CHANNEL_2:
        case (vx_enum)VX_CHANNEL_B:
            channel_offset = 2;
            break;
        case (vx_enum)VX_CHANNEL_3:
        case (vx_enum)VX_CHANNEL_A:
            if(src->format == (vx_df_image)VX_DF_IMAGE_RGBX)
            {
                channel_offset = 3;
            }
            else
            {
                status = (vx_status)VX_FAILURE;
            }
            break;
        default:
            status = (vx_status)VX_FAILURE;
            break;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxBamInitKernelDetails(&kernel_details, 1, kernel);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Fill in the frame level sizes of buffers here. If the port
         * is optionally disabled, put NULL */
        buf_params[0] = &vxlib_src;
        buf_params[1] = &vxlib_dst;

        if(src->format == (vx_df_image)VX_DF_IMAGE_RGB)
        {
            vxlib_src.data_type = (uint32_t)VXLIB_UINT24;

            BAM_VXLIB_channelExtract_1of3_i8u_o8u_params params;

            params.channel_offset = channel_offset;

            kernel_details.compute_kernel_params = (void *)&params;

            BAM_VXLIB_channelExtract_1of3_i8u_o8u_getKernelInfo(NULL,
                &kernel_details.kernel_info);

            status = tivxBamCreateHandleSingleNode(
                BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF3_I8U_O8U, buf_params,
                &kernel_details, &prms->graph_handle);
        }
        else if(src->format == (vx_df_image)VX_DF_IMAGE_RGBX)
        {
            vxlib_src.data_type = (uint32_t)VXLIB_UINT32;

            BAM_VXLIB_channelExtract_1of4_i8u_o8u_params params;

            params.channel_offset = channel_offset;

            kernel_details.compute_kernel_params = (void *)&params;

            BAM_VXLIB_channelExtract_1of4_i8u_o8u_getKernelInfo(NULL,
                &kernel_details.kernel_info);

            status = tivxBamCreateHandleSingleNode(
                BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF4_I8U_O8U, buf_params,
                &kernel_details, &prms->graph_handle);
        }
        else
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    prms->channel_offset = channel_offset;
    prms->plane_idx = 0;

    return (status);
}

static vx_status tivxBamChannelExtractInBamGraphIyuvYuv4Input(
    tivxBamChannelExtractParams *prms, const tivx_obj_desc_image_t *src,
    const tivx_obj_desc_scalar_t *ch, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    const int32_t * bam_node_cnt, void * scratch, const int32_t *size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint8_t channel_offset = 0, plane_idx = 0;

    if ((vx_enum)VX_CHANNEL_Y == ch->data.enm)
    {
        plane_idx = 0;
    }
    else if ((vx_enum)VX_CHANNEL_U == ch->data.enm)
    {
        plane_idx = 1;
    }
    else
    {
        plane_idx = 2;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

        node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U;

        BAM_VXLIB_channelCopy_1to1_i8u_o8u_getKernelInfo(NULL,
            &kernel_details[*bam_node_cnt].kernel_info);

        /*IYUV's 3 output plains are scaled 1.0, 0.5, 0.5
          if you select plane_idx 1 or 2, the input and output images needs to be scaled*/
        if (src->format == (vx_df_image)VX_DF_IMAGE_IYUV)
        {
            if ((vx_enum)VX_CHANNEL_U == ch->data.enm)
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
            else if ((vx_enum)VX_CHANNEL_V == ch->data.enm)
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
            else
            {
                /* do nothing */
            }
        }
    }

    prms->channel_offset = channel_offset;
    prms->plane_idx = plane_idx;

    return (status);
}

static vx_status tivxBamChannelExtractInBamGraphNv12Nv21Input(
    tivxBamChannelExtractParams *prms, const tivx_obj_desc_image_t *src,
    const tivx_obj_desc_scalar_t *ch, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    const int32_t * bam_node_cnt, void * scratch, const int32_t *size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint8_t channel_offset = 0, plane_idx = 0;

    if ((vx_enum)VX_CHANNEL_Y == ch->data.enm)
    {
        plane_idx = 0;
    }
    else
    {
        plane_idx = 1;
    }

    switch(ch->data.enm)
    {
        case (vx_enum)VX_CHANNEL_Y:
            channel_offset = 0;
            break;
        case (vx_enum)VX_CHANNEL_U:
            if(src->format == (vx_df_image)VX_DF_IMAGE_NV12)
            {
                channel_offset = 0;
            }
            else
            {
                channel_offset = 1;
            }
            break;
        case (vx_enum)VX_CHANNEL_V:
            if(src->format == (vx_df_image)VX_DF_IMAGE_NV12)
            {
                channel_offset = 1;
            }
            else
            {
                channel_offset = 0;
            }
            break;
        default:
            VX_PRINT(VX_ZONE_ERROR,"tivxKernelChannelExtractCreateInBamGraph.tivxBamChannelExtractInBamGraphNv12Nv21Input: \
                                    Non existing channel selection\n");
            status = (vx_status)VX_FAILURE;
            break;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if(ch->data.enm == (vx_enum)VX_CHANNEL_Y)
        {
            kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

            node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U;

            BAM_VXLIB_channelCopy_1to1_i8u_o8u_getKernelInfo(NULL,
                &kernel_details[*bam_node_cnt].kernel_info);
        }
        else
        {
            BAM_VXLIB_channelExtract_1of2_i8u_o8u_params *kernel_params = (BAM_VXLIB_channelExtract_1of2_i8u_o8u_params*)scratch;

            if ((NULL != kernel_params) &&
                (*size >= (int32_t)sizeof(BAM_VXLIB_channelExtract_1of2_i8u_o8u_params)))
            {
                kernel_params->channel_offset = channel_offset;

                kernel_details[*bam_node_cnt].compute_kernel_params = (void *)kernel_params;

                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF2_I8U_O8U;

                BAM_VXLIB_channelExtract_1of2_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details[*bam_node_cnt].kernel_info);

                kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                    vertSamplingFactor[BAM_VXLIB_CHANNELEXTRACT_1OF2_I8U_IO8U_INPUT_IMAGE_PORT] = 0.5f;
                kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                    horzSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELEXTRACT_1OF2_I8U_IO8U_NUM_INPUT_BLOCKS\
                                          + (uint32_t)BAM_VXLIB_CHANNELEXTRACT_1OF2_I8U_IO8U_OUTPUT_PORT] = 0.5f;
                kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                    vertSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELEXTRACT_1OF2_I8U_IO8U_NUM_INPUT_BLOCKS\
                                          + (uint32_t)BAM_VXLIB_CHANNELEXTRACT_1OF2_I8U_IO8U_OUTPUT_PORT] = 0.5f;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,"tivxKernelChannelExtractCreateInBamGraph.tivxBamChannelExtractInBamGraphNv12Nv21Input: \
                                        channelExtract_1of2_i8u_o8u, kernel_params is null or the size is not as expected\n");
                status = (vx_status)VX_FAILURE;
            }
        }
    }

    prms->channel_offset = channel_offset;
    prms->plane_idx = plane_idx;

    return (status);
}

static vx_status tivxBamChannelExtractInBamGraphYuyvUyvyInput(
    tivxBamChannelExtractParams *prms, const tivx_obj_desc_image_t *src,
    const tivx_obj_desc_scalar_t *ch, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    const int32_t * bam_node_cnt, void * scratch, const int32_t *size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint8_t channel_offset = 0;

    switch(ch->data.enm)
    {
        case (vx_enum)VX_CHANNEL_Y:
            if(src->format == (vx_df_image)VX_DF_IMAGE_YUYV)
            {
                channel_offset = 0;
            }
            else
            {
                channel_offset = 1;
            }
            break;
        case (vx_enum)VX_CHANNEL_U:
            if(src->format == (vx_df_image)VX_DF_IMAGE_YUYV)
            {
                channel_offset = 1;
            }
            else
            {
                channel_offset = 0;
            }
            break;
        case (vx_enum)VX_CHANNEL_V:
            if(src->format == (vx_df_image)VX_DF_IMAGE_YUYV)
            {
                channel_offset = 3;
            }
            else
            {
                channel_offset = 2;
            }
            break;
        default:
            VX_PRINT(VX_ZONE_ERROR,"tivxKernelChannelExtractCreateInBamGraph.tivxBamChannelExtractInBamGraphYuyvUyvyInput: \
                                    Non existing channel selection\n");
            status = (vx_status)VX_FAILURE;
            break;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if(ch->data.enm == (vx_enum)VX_CHANNEL_Y)
        {
            BAM_VXLIB_channelExtract_1of2_i8u_o8u_params *kernel_params = (BAM_VXLIB_channelExtract_1of2_i8u_o8u_params*)scratch;

            if ((NULL != kernel_params) &&
                (*size >= (int32_t)sizeof(BAM_VXLIB_channelExtract_1of2_i8u_o8u_params)))
            {
                kernel_params->channel_offset = channel_offset;

                kernel_details[*bam_node_cnt].compute_kernel_params = (void *)kernel_params;

                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF2_I8U_O8U;

                BAM_VXLIB_channelExtract_1of2_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details[*bam_node_cnt].kernel_info);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,"tivxKernelChannelExtractCreateInBamGraph.tivxBamChannelExtractInBamGraphYuyvUyvyInput: \
                                        channelExtract_1of2_i8u_o8u, kernel_params is null or the size is not as expected\n");
                status = (vx_status)VX_FAILURE;
            }
        }
        else
        {
            BAM_VXLIB_channelExtract_1of4_i8u_o8u_params *kernel_params = (BAM_VXLIB_channelExtract_1of4_i8u_o8u_params*)scratch;

            if ((NULL != kernel_params) &&
                (*size >= (int32_t)sizeof(BAM_VXLIB_channelExtract_1of4_i8u_o8u_params)))
            {
                kernel_params->channel_offset = channel_offset;

                kernel_details[*bam_node_cnt].compute_kernel_params = (void *)kernel_params;

                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF4_I8U_O8U;

                BAM_VXLIB_channelExtract_1of4_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details[*bam_node_cnt].kernel_info);

                kernel_details[*bam_node_cnt].kernel_info.kernelExtraInfo.\
                    horzSamplingFactor[(uint32_t)BAM_VXLIB_CHANNELEXTRACT_1OF4_I8U_IO8U_NUM_INPUT_BLOCKS\
                                          + (uint32_t)BAM_VXLIB_CHANNELEXTRACT_1OF4_I8U_IO8U_OUTPUT_PORT] = 0.5f;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,"tivxKernelChannelExtractCreateInBamGraph.tivxBamChannelExtractInBamGraphYuyvUyvyInput: \
                                        channelExtract_1of4_i8u_o8u, kernel_params is null or the size is not as expected\n");
                status = (vx_status)VX_FAILURE;
            }
        }
    }

    prms->channel_offset = channel_offset;
    prms->plane_idx = 0;

    return (status);
}

static vx_status tivxBamChannelExtractInBamGraphRgbRgbxInput(
    tivxBamChannelExtractParams *prms, const tivx_obj_desc_image_t *src,
    const tivx_obj_desc_scalar_t *ch, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    const int32_t * bam_node_cnt, void * scratch, const int32_t *size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint8_t channel_offset = 0;

    switch(ch->data.enm)
    {
        case (vx_enum)VX_CHANNEL_0:
        case (vx_enum)VX_CHANNEL_R:
            channel_offset = 0;
            break;
        case (vx_enum)VX_CHANNEL_1:
        case (vx_enum)VX_CHANNEL_G:
            channel_offset = 1;
            break;
        case (vx_enum)VX_CHANNEL_2:
        case (vx_enum)VX_CHANNEL_B:
            channel_offset = 2;
            break;
        case (vx_enum)VX_CHANNEL_3:
        case (vx_enum)VX_CHANNEL_A:
            if(src->format == (vx_df_image)VX_DF_IMAGE_RGBX)
            {
                channel_offset = 3;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,"tivxKernelChannelExtractCreateInBamGraph.tivxBamChannelExtractInBamGraphRgbRgbxInput: \
                                        Non existing channel selection\n");
                status = (vx_status)VX_FAILURE;
            }
            break;
        default:
            VX_PRINT(VX_ZONE_ERROR,"tivxKernelChannelExtractCreateInBamGraph.tivxBamChannelExtractInBamGraphRgbRgbxInput: \
                                    Non existing channel selection\n");
            status = (vx_status)VX_FAILURE;
            break;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if(src->format == (vx_df_image)VX_DF_IMAGE_RGB)
        {
            BAM_VXLIB_channelExtract_1of3_i8u_o8u_params *kernel_params = (BAM_VXLIB_channelExtract_1of3_i8u_o8u_params*)scratch;

            if ((NULL != kernel_params) &&
                (*size >= (int32_t)sizeof(BAM_VXLIB_channelExtract_1of3_i8u_o8u_params)))
            {
                kernel_params->channel_offset = channel_offset;

                kernel_details[*bam_node_cnt].compute_kernel_params = (void *)kernel_params;

                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF3_I8U_O8U;

                BAM_VXLIB_channelExtract_1of3_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details[*bam_node_cnt].kernel_info);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,"tivxKernelChannelExtractCreateInBamGraph.tivxBamChannelExtractInBamGraphRgbRgbxInput: \
                                        channelExtract_1of3_i8u_o8u, kernel_params is null or the size is not as expected\n");
                status = (vx_status)VX_FAILURE;
            }
        }
        else if(src->format == (vx_df_image)VX_DF_IMAGE_RGBX)
        {
            BAM_VXLIB_channelExtract_1of4_i8u_o8u_params *kernel_params = (BAM_VXLIB_channelExtract_1of4_i8u_o8u_params*)scratch;

            if ((NULL != kernel_params) &&
                (*size >= (int32_t)sizeof(BAM_VXLIB_channelExtract_1of4_i8u_o8u_params)))
            {
                kernel_params->channel_offset = channel_offset;

                kernel_details[*bam_node_cnt].compute_kernel_params = (void *)kernel_params;

                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF4_I8U_O8U;

                BAM_VXLIB_channelExtract_1of4_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details[*bam_node_cnt].kernel_info);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,"tivxKernelChannelExtractCreateInBamGraph.tivxBamChannelExtractInBamGraphRgbRgbxInput: \
                                        channelExtract_1of4_i8u_o8u, kernel_params is null or the size is not as expected\n");
                status = (vx_status)VX_FAILURE;
            }
        }
        else
        {
            /* do nothing */
        }
    }

    prms->channel_offset = channel_offset;
    prms->plane_idx = 0;

    return (status);
}

static vx_status VX_CALLBACK tivxKernelBamChannelExtractProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint8_t plane_idx = 0;
    tivxBamChannelExtractParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    uint8_t *src_addr, *dst_addr;
    vx_rectangle_t rect;
    uint32_t size;
    void *img_ptrs[2];

    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CHANNEL_EXTRACT_INPUT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CHANNEL_EXTRACT_OUTPUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxBamChannelExtractParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *src_target_ptr;
        void *dst_target_ptr;
        plane_idx = prms->plane_idx;

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src->valid_roi;

        src_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[plane_idx]);
        src_addr = (uint8_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[plane_idx]));

        rect = dst->valid_roi;

        dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr[0]);
        dst_addr = (uint8_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0U]));

        img_ptrs[0] = src_addr;
        img_ptrs[1] = dst_addr;
        tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);
    }
    else
    {
        status = (vx_status)VX_ERROR_NO_MEMORY;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelBamChannelExtractCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{

    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_scalar_t *ch;
    tivxBamChannelExtractParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CHANNEL_EXTRACT_INPUT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CHANNEL_EXTRACT_OUTPUT_IDX];
        ch = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_CHANNEL_EXTRACT_CHANNEL_IDX];

        prms = tivxMemAlloc(sizeof(tivxBamChannelExtractParams),
            (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxBamChannelExtractParams));

            if ((src->format == (vx_df_image)VX_DF_IMAGE_RGB) ||
                (src->format == (vx_df_image)VX_DF_IMAGE_RGBX))
            {
                status = tivxBamChannelExtractRgbRgbxInput(kernel, prms, src, dst, ch);
            }
            else
            if((src->format == (vx_df_image)VX_DF_IMAGE_YUYV)||
               (src->format == (vx_df_image)VX_DF_IMAGE_UYVY))
            {
                status = tivxBamChannelExtractYuyvUyvyInput(kernel, prms, src, dst, ch);
            }
            else
            if ((src->format == (vx_df_image)VX_DF_IMAGE_NV12) ||
                (src->format == (vx_df_image)VX_DF_IMAGE_NV21))
            {
                status = tivxBamChannelExtractNv12Nv21Input(kernel, prms, src, dst, ch);
            }
            else
            if ((src->format == (vx_df_image)VX_DF_IMAGE_IYUV) ||
                (src->format == (vx_df_image)VX_DF_IMAGE_YUV4))
            {
                status = tivxBamChannelExtractIyuvYuv4Input(kernel, prms, src, dst, ch);
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
                sizeof(tivxBamChannelExtractParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxBamChannelExtractParams),
                    (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelBamChannelExtractDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t size;
    tivxBamChannelExtractParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxBamChannelExtractParams) == size))
        {
            if(NULL != prms->graph_handle)
            {
                tivxBamDestroyHandle(prms->graph_handle);
            }
            tivxMemFree(prms, sizeof(tivxBamChannelExtractParams),
                (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

void tivxAddTargetKernelBamChannelExtract(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_channel_extract_target_kernel = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_CHANNEL_EXTRACT,
            target_name,
            tivxKernelBamChannelExtractProcess,
            tivxKernelBamChannelExtractCreate,
            tivxKernelBamChannelExtractDelete,
            NULL,
            NULL);

        tivxEnableKernelForSuperNode(vx_channel_extract_target_kernel,
            tivxKernelChannelExtractCreateInBamGraph,
            tivxKernelChannelExtractGetNodePort,
            NULL,
            NULL,
            NULL,
            MAX4((int32_t)sizeof(BAM_VXLIB_channelExtract_1of2_i8u_o8u_params),
                 (int32_t)sizeof(BAM_VXLIB_channelExtract_1of3_i8u_o8u_params),
                 (int32_t)sizeof(BAM_VXLIB_channelExtract_1of4_i8u_o8u_params),
                 (int32_t)sizeof(BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U)),
            NULL);
    }
}


void tivxRemoveTargetKernelBamChannelExtract(void)
{
    tivxRemoveTargetKernel(vx_channel_extract_target_kernel);
}

static vx_status VX_CALLBACK tivxKernelChannelExtractCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size)
{

    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src;
    tivx_obj_desc_scalar_t *ch;
    tivxBamChannelExtractParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS);

    src = (tivx_obj_desc_image_t *)obj_desc[
        TIVX_KERNEL_CHANNEL_EXTRACT_INPUT_IDX];
    ch = (tivx_obj_desc_scalar_t *)obj_desc[
        TIVX_KERNEL_CHANNEL_EXTRACT_CHANNEL_IDX];

    if ((vx_status)VX_SUCCESS == status)
    {
        prms = tivxMemAlloc(sizeof(tivxBamChannelExtractParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxBamChannelExtractParams));

            node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
            node_list[*bam_node_cnt].kernelArgs = NULL;

            if ((src->format == (vx_df_image)VX_DF_IMAGE_RGB) ||
                (src->format == (vx_df_image)VX_DF_IMAGE_RGBX))
            {
                status = tivxBamChannelExtractInBamGraphRgbRgbxInput(prms, src, ch,
                            node_list, kernel_details, bam_node_cnt, scratch, size);
            }
            else if ((src->format == (vx_df_image)VX_DF_IMAGE_YUYV)||
                    (src->format == (vx_df_image)VX_DF_IMAGE_UYVY))
            {
                status = tivxBamChannelExtractInBamGraphYuyvUyvyInput(prms, src, ch,
                            node_list, kernel_details, bam_node_cnt, scratch, size);
            }
            else if ((src->format == (vx_df_image)VX_DF_IMAGE_NV12) ||
                    (src->format == (vx_df_image)VX_DF_IMAGE_NV21))
            {
                status = tivxBamChannelExtractInBamGraphNv12Nv21Input(prms, src, ch,
                            node_list, kernel_details, bam_node_cnt, scratch, size);
            }
            else if ((src->format == (vx_df_image)VX_DF_IMAGE_IYUV) ||
                    (src->format == (vx_df_image)VX_DF_IMAGE_YUV4))
            {
                status = tivxBamChannelExtractInBamGraphIyuvYuv4Input(prms, src, ch,
                            node_list, kernel_details, bam_node_cnt, scratch, size);
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
                sizeof(tivxBamChannelExtractParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxBamChannelExtractParams), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelChannelExtractGetNodePort(
    tivx_target_kernel_instance kernel,
    uint8_t ovx_port, uint8_t plane, uint8_t *bam_node, uint8_t *bam_port)
{
    tivxBamChannelExtractParams *prms = NULL;
    uint32_t size;

    vx_status status = tivxGetTargetKernelInstanceContext(kernel,
                        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
        (sizeof(tivxBamChannelExtractParams) == size))
    {
        switch (ovx_port)
        {
            case TIVX_KERNEL_CHANNEL_EXTRACT_INPUT_IDX:
                *bam_node = prms->bam_node_num;
                switch (plane)
                {
                    case 0 :
                        *bam_port = (prms->plane_idx == 0U) ? 0U : (uint8_t)TIVX_IMAGE_NULL_PLANE;
                        break;
                    case 1 :
                        *bam_port = (prms->plane_idx == 1U) ? 1U : (uint8_t)TIVX_IMAGE_NULL_PLANE;
                        break;
                    case 2 :
                        *bam_port = (prms->plane_idx == 2U) ? 2U : (uint8_t)TIVX_IMAGE_NULL_PLANE;
                        break;
                    default:
                        VX_PRINT(VX_ZONE_ERROR, "non existing index plane queried by tivxKernelSupernodeCreate.tivxGetNodePort()\n");
                        status = (vx_status)VX_FAILURE;
                        break;
                }
                break;
            case TIVX_KERNEL_CHANNEL_EXTRACT_OUTPUT_IDX:
                *bam_node = prms->bam_node_num;
                *bam_port = (uint8_t)BAM_VXLIB_CHANNELEXTRACT_1OF3_I8U_IO8U_OUTPUT_PORT;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "non existing index queried by tivxKernelSupernodeCreate.tivxGetNodePort()\n");
                status = (vx_status)VX_FAILURE;
                break;
        }
    }

    return status;
}
