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
#include <tivx_kernel_channel_extract.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

static tivx_target_kernel vx_channel_extract_target_kernel = NULL;

vx_status tivxChannelExtractRgbRgbxInput(
            tivx_obj_desc_image_t *in_desc,
            tivx_obj_desc_image_t *out_desc,
            vx_enum channel_value,
            uint8_t *dst_addr,
            VXLIB_bufParams2D_t *vxlib_dst
            )
{
    VXLIB_bufParams2D_t vxlib_src;
    uint8_t *src_addr;
    uint8_t channel_offset;
    vx_rectangle_t rect;
    vx_status status = VX_SUCCESS;
    void *in_desc_target_ptr;

    in_desc_target_ptr = tivxMemShared2TargetPtr(
      in_desc->mem_ptr[0].shared_ptr, in_desc->mem_ptr[0].mem_heap_region);

    /* Get the correct offset of the images from the valid roi parameter,
     */
    rect = in_desc->valid_roi;

    src_addr = (uint8_t *)((uintptr_t)in_desc_target_ptr +
        tivxComputePatchOffset(rect.start_x, rect.start_y,
        &in_desc->imagepatch_addr[0U]));

    tivxInitBufParams(in_desc, &vxlib_src);

    switch(channel_value)
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
            if(in_desc->format == VX_DF_IMAGE_RGBX)
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

    if(status == VX_SUCCESS)
    {
        tivxMemBufferMap(in_desc_target_ptr,
           in_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        if(in_desc->format == VX_DF_IMAGE_RGB)
        {
            status = VXLIB_channelExtract_1of3_i8u_o8u(src_addr, &vxlib_src, dst_addr, vxlib_dst, channel_offset);
        }
        else
        if(in_desc->format == VX_DF_IMAGE_RGBX)
        {
            status = VXLIB_channelExtract_1of4_i8u_o8u(src_addr, &vxlib_src, dst_addr, vxlib_dst, channel_offset);
        }
        else
        {
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(in_desc_target_ptr,
           in_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
    }

    return status;
}

vx_status tivxChannelExtractYuyvUyvyInput(
            tivx_obj_desc_image_t *in_desc,
            tivx_obj_desc_image_t *out_desc,
            vx_enum channel_value,
            uint8_t *dst_addr,
            const VXLIB_bufParams2D_t *vxlib_dst
            )
{
    VXLIB_bufParams2D_t vxlib_src;
    uint8_t *src_addr;
    uint8_t channel_offset;
    vx_status status = VX_SUCCESS;
    void *in_desc_target_ptr;

    in_desc_target_ptr = tivxMemShared2TargetPtr(
      in_desc->mem_ptr[0].shared_ptr, in_desc->mem_ptr[0].mem_heap_region);

    tivxSetPointerLocation(in_desc, &in_desc_target_ptr, &src_addr);

    tivxInitBufParams(in_desc, &vxlib_src);

    switch(channel_value)
    {
        case VX_CHANNEL_Y:
            if(in_desc->format == VX_DF_IMAGE_YUYV)
            {
                channel_offset = 0;
            }
            else
            {
                channel_offset = 1;
            }
            break;
        case VX_CHANNEL_U:
            if(in_desc->format == VX_DF_IMAGE_YUYV)
            {
                channel_offset = 1;
            }
            else
            {
                channel_offset = 0;
            }
            break;
        case VX_CHANNEL_V:
            if(in_desc->format == VX_DF_IMAGE_YUYV)
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

    if(status == VX_SUCCESS)
    {
        tivxMemBufferMap(in_desc_target_ptr,
           in_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        if(channel_value == VX_CHANNEL_Y)
        {
            status = VXLIB_channelExtract_1of2_i8u_o8u(src_addr, &vxlib_src, dst_addr, vxlib_dst, channel_offset);
        }
        else
        { /* channel_value is VX_CHANNEL_U or VX_CHANNEL_V
           * consider plane to be 4 bytes per pixel, i.e half the width
           */
            vxlib_src.dim_x = vxlib_src.dim_x/2;

            status = VXLIB_channelExtract_1of4_i8u_o8u(src_addr, &vxlib_src, dst_addr, vxlib_dst, channel_offset);
        }

        tivxMemBufferUnmap(in_desc_target_ptr,
           in_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
    }

    return status;
}

vx_status tivxChannelExtractNv12Nv21Input(
            tivx_obj_desc_image_t *in_desc,
            tivx_obj_desc_image_t *out_desc,
            vx_enum channel_value,
            uint8_t *dst_addr,
            VXLIB_bufParams2D_t *vxlib_dst
            )
{
    VXLIB_bufParams2D_t vxlib_src;
    uint8_t *src_addr;
    uint8_t channel_offset, plane_idx;
    vx_rectangle_t rect;
    vx_status status = VX_SUCCESS;
    void *in_desc_target_ptr;

    if( channel_value == VX_CHANNEL_Y)
    {
        plane_idx = 0;
    }
    else
    if( channel_value == VX_CHANNEL_U)
    {
        plane_idx = 1;
    }
    else
    if( channel_value == VX_CHANNEL_V)
    {
        plane_idx = 1;
    }
    else
    {
        status = VX_FAILURE;
    }

    if(status == VX_SUCCESS)
    {
        in_desc_target_ptr = tivxMemShared2TargetPtr(
          in_desc->mem_ptr[plane_idx].shared_ptr, in_desc->mem_ptr[plane_idx].mem_heap_region);

        /* Get the correct offset of the images from the valid roi parameter,
         */
        rect = in_desc->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)in_desc_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &in_desc->imagepatch_addr[plane_idx]));

        /* Initialize vxLib Parameters with the input/output frame parameters */
        vxlib_src.dim_x = (in_desc->valid_roi.end_x - in_desc->valid_roi.start_x)/in_desc->imagepatch_addr[plane_idx].step_x;
        vxlib_src.dim_y = (in_desc->valid_roi.end_y - in_desc->valid_roi.start_y)/in_desc->imagepatch_addr[plane_idx].step_y;
        vxlib_src.stride_y = in_desc->imagepatch_addr[plane_idx].stride_y;
        vxlib_src.data_type = VXLIB_UINT8;

        switch(channel_value)
        {
            case VX_CHANNEL_Y:
                channel_offset = 0;
                break;
            case VX_CHANNEL_U:
                if(in_desc->format == VX_DF_IMAGE_NV12)
                {
                    channel_offset = 0;
                }
                else
                {
                    channel_offset = 1;
                }
                break;
            case VX_CHANNEL_V:
                if(in_desc->format == VX_DF_IMAGE_NV12)
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
    }

    if(status == VX_SUCCESS)
    {
        tivxMemBufferMap(in_desc_target_ptr,
           in_desc->mem_size[plane_idx], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        if(channel_value == VX_CHANNEL_Y)
        {
            status = VXLIB_channelCopy_1to1_i8u_o8u(src_addr, &vxlib_src, dst_addr, vxlib_dst);
        }
        else
        {
            status = VXLIB_channelExtract_1of2_i8u_o8u(src_addr, &vxlib_src, dst_addr, vxlib_dst, channel_offset);
        }

        tivxMemBufferUnmap(in_desc_target_ptr,
           in_desc->mem_size[plane_idx], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
    }

    return status;
}

vx_status tivxChannelExtractIyuvYuv4Input(
            tivx_obj_desc_image_t *in_desc,
            tivx_obj_desc_image_t *out_desc,
            vx_enum channel_value,
            uint8_t *dst_addr,
            VXLIB_bufParams2D_t *vxlib_dst
            )
{
    VXLIB_bufParams2D_t vxlib_src;
    uint8_t *src_addr;
    uint8_t plane_idx;
    vx_rectangle_t rect;
    vx_status status = VX_SUCCESS;
    void *in_desc_target_ptr;

    if( channel_value == VX_CHANNEL_Y)
    {
        plane_idx = 0;
    }
    else
    if( channel_value == VX_CHANNEL_U)
    {
        plane_idx = 1;
    }
    else
    if( channel_value == VX_CHANNEL_V)
    {
        plane_idx = 2;
    }
    else
    {
        status = VX_FAILURE;
    }

    if(status == VX_SUCCESS)
    {
        in_desc_target_ptr = tivxMemShared2TargetPtr(
          in_desc->mem_ptr[plane_idx].shared_ptr, in_desc->mem_ptr[plane_idx].mem_heap_region);

        /* Get the correct offset of the images from the valid roi parameter,
         */
        rect = in_desc->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)in_desc_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &in_desc->imagepatch_addr[plane_idx]));

        /* Initialize vxLib Parameters with the input/output frame parameters */
        vxlib_src.dim_x = (in_desc->valid_roi.end_x - in_desc->valid_roi.start_x)/in_desc->imagepatch_addr[plane_idx].step_x;
        vxlib_src.dim_y = (in_desc->valid_roi.end_y - in_desc->valid_roi.start_y)/in_desc->imagepatch_addr[plane_idx].step_y;
        vxlib_src.stride_y = in_desc->imagepatch_addr[plane_idx].stride_y;
        vxlib_src.data_type = VXLIB_UINT8;
    }

    if(status == VX_SUCCESS)
    {
        tivxMemBufferMap(in_desc_target_ptr,
           in_desc->mem_size[plane_idx], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        status = VXLIB_channelCopy_1to1_i8u_o8u(src_addr, &vxlib_src, dst_addr, vxlib_dst);

        tivxMemBufferUnmap(in_desc_target_ptr,
           in_desc->mem_size[plane_idx], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
    }

    return status;
}

vx_status VX_CALLBACK tivxChannelExtract(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *in_desc;
    tivx_obj_desc_scalar_t *channel_desc;
    tivx_obj_desc_image_t *out_desc;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        vx_enum channel_value;
        void *out_desc_target_ptr;

        in_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_IN_IDX];
        channel_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_CHANNEL_IDX];
        out_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_OUT_IDX];

        out_desc_target_ptr = tivxMemShared2TargetPtr(
          out_desc->mem_ptr[0].shared_ptr, out_desc->mem_ptr[0].mem_heap_region);

        tivxMemBufferMap(out_desc_target_ptr,
           out_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        channel_value = channel_desc->data.enm;

        /* call kernel processing function */
        {
            vx_rectangle_t rect;
            VXLIB_bufParams2D_t vxlib_dst;
            uint8_t *dst_addr;

            /* Get the correct offset of the images from the valid roi parameter,
             */
            rect = out_desc->valid_roi;

            dst_addr = (uint8_t *)((uintptr_t)out_desc_target_ptr +
                tivxComputePatchOffset(rect.start_x, rect.start_y,
                &out_desc->imagepatch_addr[0]));

            tivxInitBufParams(out_desc, &vxlib_dst);

            if (   (in_desc->format == VX_DF_IMAGE_RGB)
                || (in_desc->format == VX_DF_IMAGE_RGBX)
                )
            {
                status = tivxChannelExtractRgbRgbxInput(
                                in_desc, out_desc,
                                channel_value,
                                dst_addr, &vxlib_dst);
            }
            else
            if(    (in_desc->format == VX_DF_IMAGE_YUYV)
                || (in_desc->format == VX_DF_IMAGE_UYVY)
                )
            {
                status = tivxChannelExtractYuyvUyvyInput(
                                in_desc, out_desc,
                                channel_value,
                                dst_addr, &vxlib_dst);
            }
            else
            if(    (in_desc->format == VX_DF_IMAGE_NV12)
                || (in_desc->format == VX_DF_IMAGE_NV21)
                )
            {
                status = tivxChannelExtractNv12Nv21Input(
                                in_desc, out_desc,
                                channel_value,
                                dst_addr, &vxlib_dst);
            }
            else
            if(    (in_desc->format == VX_DF_IMAGE_IYUV)
                || (in_desc->format == VX_DF_IMAGE_YUV4)
                )
            {
                status = tivxChannelExtractIyuvYuv4Input(
                                in_desc, out_desc,
                                channel_value,
                                dst_addr, &vxlib_dst);
            }
            else
            {
                status = VX_FAILURE;
            }

            if (VXLIB_SUCCESS != status)
            {
                status = VX_FAILURE;
            }
        }
        /* kernel processing function complete */

        tivxMemBufferUnmap(out_desc_target_ptr,
           out_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);


    }

    return status;
}

vx_status VX_CALLBACK tivxChannelExtractCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status VX_CALLBACK tivxChannelExtractDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status VX_CALLBACK tivxChannelExtractControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelChannelExtract(void)
{
    vx_status status = VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == TIVX_CPU_ID_DSP1 )
    {
        strncpy(target_name, TIVX_TARGET_DSP1, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    if ( self_cpu == TIVX_CPU_ID_DSP2 )
    {
        strncpy(target_name, TIVX_TARGET_DSP2, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        vx_channel_extract_target_kernel = tivxAddTargetKernel(
                            VX_KERNEL_CHANNEL_EXTRACT,
                            target_name,
                            tivxChannelExtract,
                            tivxChannelExtractCreate,
                            tivxChannelExtractDelete,
                            tivxChannelExtractControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelChannelExtract(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_channel_extract_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_channel_extract_target_kernel = NULL;
    }
}


