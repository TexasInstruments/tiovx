/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_channel_extract.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

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

    in_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
      in_desc->mem_ptr[0].shared_ptr, in_desc->mem_ptr[0].mem_type);

    /* Get the correct offset of the images from the valid roi parameter,
     */
    rect = in_desc->valid_roi;

    src_addr = (uint8_t *)((uint32_t)in_desc->mem_ptr[0U].target_ptr +
        ownComputePatchOffset(rect.start_x, rect.start_y,
        &in_desc->imagepatch_addr[0U]));

    /* Initialize vxLib Parameters with the input/output frame parameters */
    vxlib_src.dim_x = in_desc->imagepatch_addr[0U].dim_x;
    vxlib_src.dim_y = in_desc->imagepatch_addr[0U].dim_y;
    vxlib_src.stride_y = in_desc->imagepatch_addr[0U].stride_y;
    vxlib_src.data_type = VXLIB_UINT8;

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
        tivxMemBufferMap(in_desc->mem_ptr[0].target_ptr,
           in_desc->mem_size[0], in_desc->mem_ptr[0].mem_type,
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

        tivxMemBufferUnmap(in_desc->mem_ptr[0].target_ptr,
           in_desc->mem_size[0], in_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
    }

    return status;
}

vx_status tivxChannelExtractYuyvUyvyInput(
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

    in_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
      in_desc->mem_ptr[0].shared_ptr, in_desc->mem_ptr[0].mem_type);

    /* Get the correct offset of the images from the valid roi parameter,
     */
    rect = in_desc->valid_roi;

    src_addr = (uint8_t *)((uint32_t)in_desc->mem_ptr[0U].target_ptr +
        ownComputePatchOffset(rect.start_x, rect.start_y,
        &in_desc->imagepatch_addr[0U]));

    /* Initialize vxLib Parameters with the input/output frame parameters */
    vxlib_src.dim_x = in_desc->imagepatch_addr[0U].dim_x;
    vxlib_src.dim_y = in_desc->imagepatch_addr[0U].dim_y;
    vxlib_src.stride_y = in_desc->imagepatch_addr[0U].stride_y;
    vxlib_src.data_type = VXLIB_UINT8;

    switch(channel_value)
    {
        case VX_CHANNEL_Y:
            if(in_desc->format == VX_DF_IMAGE_YUYV)
                channel_offset = 0;
            else
                channel_offset = 1;
            break;
        case VX_CHANNEL_U:
            if(in_desc->format == VX_DF_IMAGE_YUYV)
                channel_offset = 1;
            else
                channel_offset = 0;
            break;
        case VX_CHANNEL_V:
            if(in_desc->format == VX_DF_IMAGE_YUYV)
                channel_offset = 3;
            else
                channel_offset = 2;
            break;
        default:
            status = VX_FAILURE;
            break;
    }

    if(status == VX_SUCCESS)
    {
        tivxMemBufferMap(in_desc->mem_ptr[0].target_ptr,
           in_desc->mem_size[0], in_desc->mem_ptr[0].mem_type,
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

        tivxMemBufferUnmap(in_desc->mem_ptr[0].target_ptr,
           in_desc->mem_size[0], in_desc->mem_ptr[0].mem_type,
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
        in_desc->mem_ptr[plane_idx].target_ptr = tivxMemShared2TargetPtr(
          in_desc->mem_ptr[plane_idx].shared_ptr, in_desc->mem_ptr[plane_idx].mem_type);

        /* Get the correct offset of the images from the valid roi parameter,
         */
        rect = in_desc->valid_roi;

        src_addr = (uint8_t *)((uint32_t)in_desc->mem_ptr[plane_idx].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &in_desc->imagepatch_addr[plane_idx]));

        /* Initialize vxLib Parameters with the input/output frame parameters */
        vxlib_src.dim_x = in_desc->imagepatch_addr[plane_idx].dim_x/in_desc->imagepatch_addr[plane_idx].step_x;
        vxlib_src.dim_y = in_desc->imagepatch_addr[plane_idx].dim_y/in_desc->imagepatch_addr[plane_idx].step_y;
        vxlib_src.stride_y = in_desc->imagepatch_addr[plane_idx].stride_y;
        vxlib_src.data_type = VXLIB_UINT8;

        switch(channel_value)
        {
            case VX_CHANNEL_Y:
                channel_offset = 0;
                break;
            case VX_CHANNEL_U:
                if(in_desc->format == VX_DF_IMAGE_NV12)
                    channel_offset = 0;
                else
                    channel_offset = 1;
                break;
            case VX_CHANNEL_V:
                if(in_desc->format == VX_DF_IMAGE_NV12)
                    channel_offset = 1;
                else
                    channel_offset = 0;
                break;
            default:
                status = VX_FAILURE;
                break;
        }
    }

    if(status == VX_SUCCESS)
    {
        tivxMemBufferMap(in_desc->mem_ptr[plane_idx].target_ptr,
           in_desc->mem_size[plane_idx], in_desc->mem_ptr[plane_idx].mem_type,
            VX_READ_ONLY);

        if(channel_value == VX_CHANNEL_Y)
        {
            status = VXLIB_channelCopy_1to1_i8u_o8u(src_addr, &vxlib_src, dst_addr, vxlib_dst);
        }
        else
        {
            status = VXLIB_channelExtract_1of2_i8u_o8u(src_addr, &vxlib_src, dst_addr, vxlib_dst, channel_offset);
        }

        tivxMemBufferUnmap(in_desc->mem_ptr[plane_idx].target_ptr,
           in_desc->mem_size[plane_idx], in_desc->mem_ptr[plane_idx].mem_type,
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
        in_desc->mem_ptr[plane_idx].target_ptr = tivxMemShared2TargetPtr(
          in_desc->mem_ptr[plane_idx].shared_ptr, in_desc->mem_ptr[plane_idx].mem_type);

        /* Get the correct offset of the images from the valid roi parameter,
         */
        rect = in_desc->valid_roi;

        src_addr = (uint8_t *)((uint32_t)in_desc->mem_ptr[plane_idx].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &in_desc->imagepatch_addr[plane_idx]));

        /* Initialize vxLib Parameters with the input/output frame parameters */
        vxlib_src.dim_x = in_desc->imagepatch_addr[plane_idx].dim_x/in_desc->imagepatch_addr[plane_idx].step_x;
        vxlib_src.dim_y = in_desc->imagepatch_addr[plane_idx].dim_y/in_desc->imagepatch_addr[plane_idx].step_y;
        vxlib_src.stride_y = in_desc->imagepatch_addr[plane_idx].stride_y;
        vxlib_src.data_type = VXLIB_UINT8;
    }

    if(status == VX_SUCCESS)
    {
        tivxMemBufferMap(in_desc->mem_ptr[plane_idx].target_ptr,
           in_desc->mem_size[plane_idx], in_desc->mem_ptr[plane_idx].mem_type,
            VX_READ_ONLY);

        status = VXLIB_channelCopy_1to1_i8u_o8u(src_addr, &vxlib_src, dst_addr, vxlib_dst);

        tivxMemBufferUnmap(in_desc->mem_ptr[plane_idx].target_ptr,
           in_desc->mem_size[plane_idx], in_desc->mem_ptr[plane_idx].mem_type,
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

    if ( num_params != TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_CHANNEL_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_OUT_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        vx_enum channel_value;

        in_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_IN_IDX];
        channel_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_CHANNEL_IDX];
        out_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_OUT_IDX];

        out_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
          out_desc->mem_ptr[0].shared_ptr, out_desc->mem_ptr[0].mem_type);

        tivxMemBufferMap(out_desc->mem_ptr[0].target_ptr,
           out_desc->mem_size[0], out_desc->mem_ptr[0].mem_type,
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

            /* TODO: Do we require to move pointer even for destination image */
            dst_addr = (uint8_t *)((uint32_t)out_desc->mem_ptr[0U].target_ptr +
                ownComputePatchOffset(rect.start_x, rect.start_y,
                &out_desc->imagepatch_addr[0]));

            vxlib_dst.dim_x = out_desc->imagepatch_addr[0U].dim_x;
            vxlib_dst.dim_y = out_desc->imagepatch_addr[0U].dim_y;
            vxlib_dst.stride_y = out_desc->imagepatch_addr[0U].stride_y;
            vxlib_dst.data_type = VXLIB_UINT8;

            if (   in_desc->format == VX_DF_IMAGE_RGB
                || in_desc->format == VX_DF_IMAGE_RGBX
                )
            {
                status = tivxChannelExtractRgbRgbxInput(
                                in_desc, out_desc,
                                channel_value,
                                dst_addr, &vxlib_dst);
            }
            else
            if(    in_desc->format == VX_DF_IMAGE_YUYV
                || in_desc->format == VX_DF_IMAGE_UYVY
                )
            {
                status = tivxChannelExtractYuyvUyvyInput(
                                in_desc, out_desc,
                                channel_value,
                                dst_addr, &vxlib_dst);
            }
            else
            if(    in_desc->format == VX_DF_IMAGE_NV12
                || in_desc->format == VX_DF_IMAGE_NV21
                )
            {
                status = tivxChannelExtractNv12Nv21Input(
                                in_desc, out_desc,
                                channel_value,
                                dst_addr, &vxlib_dst);
            }
            else
            if(    in_desc->format == VX_DF_IMAGE_IYUV
                || in_desc->format == VX_DF_IMAGE_YUV4
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

        tivxMemBufferUnmap(out_desc->mem_ptr[0].target_ptr,
           out_desc->mem_size[0], out_desc->mem_ptr[0].mem_type,
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

void tivxAddTargetKernelChannelExtract()
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

void tivxRemoveTargetKernelChannelExtract()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_channel_extract_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_channel_extract_target_kernel = NULL;
    }
}


