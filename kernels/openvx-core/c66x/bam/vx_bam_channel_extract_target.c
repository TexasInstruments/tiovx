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
#include <tivx_kernel_channel_extract.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>

typedef struct
{
    uint8_t channel_offset, plane_idx;
    tivx_bam_graph_handle graph_handle;
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

static vx_status VX_CALLBACK tivxKernelBamChannelExtractControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status tivxBamChannelExtractIyuvYuv4Input(
    tivxBamChannelExtractParams *prms, tivx_obj_desc_image_t *src,
    tivx_obj_desc_image_t *dst, tivx_obj_desc_scalar_t *ch)
{
    vx_status status = VX_SUCCESS;
    uint8_t channel_offset = 0, plane_idx;
    tivx_bam_kernel_details_t kernel_details;
    VXLIB_bufParams2D_t vxlib_src[3], vxlib_dst;
    VXLIB_bufParams2D_t *buf_params[2];

    tivxInitBufParams(src, &vxlib_src[0]);
    tivxInitBufParams(dst, &vxlib_dst);

    if (VX_CHANNEL_Y == ch->data.enm)
    {
        plane_idx = 0;
        buf_params[0] = &vxlib_src[0];
    }
    else if (VX_CHANNEL_U == ch->data.enm)
    {
        plane_idx = 1;
        buf_params[0] = &vxlib_src[1];
    }
    else
    {
        plane_idx = 2;
        buf_params[0] = &vxlib_src[2];
    }

    if (VX_SUCCESS == status)
    {
        /* Fill in the frame level sizes of buffers here. If the port
         * is optionally disabled, put NULL */
        buf_params[1] = &vxlib_dst;

        kernel_details.compute_kernel_params = NULL;

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
    tivxBamChannelExtractParams *prms, tivx_obj_desc_image_t *src,
    tivx_obj_desc_image_t *dst, tivx_obj_desc_scalar_t *ch)
{
    vx_status status = VX_SUCCESS;
    uint8_t channel_offset = 0, plane_idx = 0;
    tivx_bam_kernel_details_t kernel_details;
    VXLIB_bufParams2D_t vxlib_src[2], vxlib_dst;
    VXLIB_bufParams2D_t *buf_params[2];

    tivxInitBufParams(src, &vxlib_src[0]);
    tivxInitBufParams(dst, &vxlib_dst);

    if (VX_CHANNEL_Y == ch->data.enm)
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
        case VX_CHANNEL_Y:
            channel_offset = 0;
            break;
        case VX_CHANNEL_U:
            if(src->format == VX_DF_IMAGE_NV12)
            {
                channel_offset = 0;
            }
            else
            {
                channel_offset = 1;
            }
            break;
        case VX_CHANNEL_V:
            if(src->format == VX_DF_IMAGE_NV12)
            {
                channel_offset = 1;
            }
            else
            {
                channel_offset = 0;
            }
            break;
        default:
            status = VX_FAILURE;
            break;
    }

    if (VX_SUCCESS == status)
    {
        /* Fill in the frame level sizes of buffers here. If the port
         * is optionally disabled, put NULL */
        buf_params[1] = &vxlib_dst;

        kernel_details.compute_kernel_params = NULL;

        if(ch->data.enm == VX_CHANNEL_Y)
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

            vxlib_src[1].data_type = VXLIB_UINT16;

            params.channel_offset = channel_offset;

            kernel_details.compute_kernel_params = (void *)&params;

            BAM_VXLIB_channelExtract_1of2_i8u_o8u_getKernelInfo(NULL,
                &kernel_details.kernel_info);

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
    tivxBamChannelExtractParams *prms, tivx_obj_desc_image_t *src,
    tivx_obj_desc_image_t *dst, tivx_obj_desc_scalar_t *ch)
{
    vx_status status = VX_SUCCESS;
    uint8_t channel_offset = 0;
    tivx_bam_kernel_details_t kernel_details;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    VXLIB_bufParams2D_t *buf_params[2];

    tivxInitBufParams(src, &vxlib_src);
    tivxInitBufParams(dst, &vxlib_dst);
    vxlib_src.data_type = VXLIB_UINT32;

    switch(ch->data.enm)
    {
        case VX_CHANNEL_Y:
            if(src->format == VX_DF_IMAGE_YUYV)
            {
                channel_offset = 0;
            }
            else
            {
                channel_offset = 1;
            }
            break;
        case VX_CHANNEL_U:
            if(src->format == VX_DF_IMAGE_YUYV)
            {
                channel_offset = 1;
            }
            else
            {
                channel_offset = 0;
            }
            break;
        case VX_CHANNEL_V:
            if(src->format == VX_DF_IMAGE_YUYV)
            {
                channel_offset = 3;
            }
            else
            {
                channel_offset = 2;
            }
            break;
        default:
            status = VX_FAILURE;
            break;
    }

    if (VX_SUCCESS == status)
    {
        /* Fill in the frame level sizes of buffers here. If the port
         * is optionally disabled, put NULL */
        buf_params[0] = &vxlib_src;
        buf_params[1] = &vxlib_dst;

        kernel_details.compute_kernel_params = NULL;

        if(ch->data.enm == VX_CHANNEL_Y)
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

            /* channel_value is VX_CHANNEL_U or VX_CHANNEL_V
             * consider plane to be 4 bytes per pixel, i.e half the width
             */
            vxlib_src.dim_x = vxlib_src.dim_x/2;

            params.channel_offset = channel_offset;

            kernel_details.compute_kernel_params = (void *)&params;

            BAM_VXLIB_channelExtract_1of4_i8u_o8u_getKernelInfo(NULL,
                &kernel_details.kernel_info);

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
    tivxBamChannelExtractParams *prms, tivx_obj_desc_image_t *src,
    tivx_obj_desc_image_t *dst, tivx_obj_desc_scalar_t *ch)
{
    vx_status status = VX_SUCCESS;
    uint8_t channel_offset = 0;
    tivx_bam_kernel_details_t kernel_details;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    VXLIB_bufParams2D_t *buf_params[2];

    tivxInitBufParams(src, &vxlib_src);
    tivxInitBufParams(dst, &vxlib_dst);

    switch(ch->data.enm)
    {
        case VX_CHANNEL_0:
        case VX_CHANNEL_R:
            channel_offset = 0;
            break;
        case VX_CHANNEL_1:
        case VX_CHANNEL_G:
            channel_offset = 1;
            break;
        case VX_CHANNEL_2:
        case VX_CHANNEL_B:
            channel_offset = 2;
            break;
        case VX_CHANNEL_3:
        case VX_CHANNEL_A:
            if(src->format == VX_DF_IMAGE_RGBX)
            {
                channel_offset = 3;
            }
            else
            {
                status = VX_FAILURE;
            }
            break;
        default:
            status = VX_FAILURE;
            break;
    }

    if (VX_SUCCESS == status)
    {
        /* Fill in the frame level sizes of buffers here. If the port
         * is optionally disabled, put NULL */
        buf_params[0] = &vxlib_src;
        buf_params[1] = &vxlib_dst;

        kernel_details.compute_kernel_params = NULL;

        if(src->format == VX_DF_IMAGE_RGB)
        {
            vxlib_src.data_type = VXLIB_UINT24;

            BAM_VXLIB_channelExtract_1of3_i8u_o8u_params params;

            params.channel_offset = channel_offset;

            kernel_details.compute_kernel_params = (void *)&params;

            BAM_VXLIB_channelExtract_1of3_i8u_o8u_getKernelInfo(NULL,
                &kernel_details.kernel_info);

            status = tivxBamCreateHandleSingleNode(
                BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF3_I8U_O8U, buf_params,
                &kernel_details, &prms->graph_handle);
        }
        else if(src->format == VX_DF_IMAGE_RGBX)
        {
            vxlib_src.data_type = VXLIB_UINT32;

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
            status = VX_FAILURE;
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
    vx_status status = VX_SUCCESS;
    uint8_t plane_idx = 0;
    tivxBamChannelExtractParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    uint8_t *src_addr, *dst_addr;
    vx_rectangle_t rect;
    uint32_t size;
    void *img_ptrs[2];

    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CHANNEL_EXTRACT_IN_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CHANNEL_EXTRACT_OUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxBamChannelExtractParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        void *src_target_ptr;
        void *dst_target_ptr;
        plane_idx = prms->plane_idx;

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src->valid_roi;

        src_target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[plane_idx].shared_ptr,
            src->mem_ptr[plane_idx].mem_heap_region);
        src_addr = (uint8_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[plane_idx]));

        rect = dst->valid_roi;

        dst_target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_heap_region);
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
        status = VX_ERROR_NO_MEMORY;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelBamChannelExtractCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{

    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_scalar_t *ch;
    tivxBamChannelExtractParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CHANNEL_EXTRACT_IN_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CHANNEL_EXTRACT_OUT_IDX];
        ch = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_CHANNEL_EXTRACT_CHANNEL_IDX];

        prms = tivxMemAlloc(sizeof(tivxBamChannelExtractParams),
            TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxBamChannelExtractParams));

            if ((src->format == VX_DF_IMAGE_RGB) ||
                (src->format == VX_DF_IMAGE_RGBX))
            {
                status = tivxBamChannelExtractRgbRgbxInput(prms, src, dst, ch);
            }
            else
            if((src->format == VX_DF_IMAGE_YUYV)||
               (src->format == VX_DF_IMAGE_UYVY))
            {
                status = tivxBamChannelExtractYuyvUyvyInput(prms, src, dst, ch);
            }
            else
            if ((src->format == VX_DF_IMAGE_NV12) ||
                (src->format == VX_DF_IMAGE_NV21))
            {
                status = tivxBamChannelExtractNv12Nv21Input(prms, src, dst, ch);
            }
            else
            if ((src->format == VX_DF_IMAGE_IYUV) ||
                (src->format == VX_DF_IMAGE_YUV4))
            {
                status = tivxBamChannelExtractIyuvYuv4Input(prms, src, dst, ch);
            }
            else
            {
                status = VX_FAILURE;
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxBamChannelExtractParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxBamChannelExtractParams),
                    TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelBamChannelExtractDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxBamChannelExtractParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxBamChannelExtractParams) == size))
        {
            tivxBamDestroyHandle(prms->graph_handle);
            tivxMemFree(prms, sizeof(tivxBamChannelExtractParams),
                TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelBamChannelExtractControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBamChannelExtract(void)
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

        vx_channel_extract_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_CHANNEL_EXTRACT,
            target_name,
            tivxKernelBamChannelExtractProcess,
            tivxKernelBamChannelExtractCreate,
            tivxKernelBamChannelExtractDelete,
            tivxKernelBamChannelExtractControl,
            NULL);
    }
}


void tivxRemoveTargetKernelBamChannelExtract(void)
{
    tivxRemoveTargetKernel(vx_channel_extract_target_kernel);
}
