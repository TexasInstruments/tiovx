/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_channel_combine.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_channel_combine_target_kernel = NULL;

vx_status tivxChannelCombine(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src0_desc;
    tivx_obj_desc_image_t *src1_desc;
    tivx_obj_desc_image_t *src2_desc;
    tivx_obj_desc_image_t *src3_desc;
    tivx_obj_desc_image_t *dst_desc;
    uint16_t plane_idx;

    if ( num_params != TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_SRC0_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_SRC1_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {

        src0_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_SRC0_IDX];
        src1_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_SRC1_IDX];
        src2_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_SRC2_IDX];
        src3_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_SRC3_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX];

        src0_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
          src0_desc->mem_ptr[0].shared_ptr, src0_desc->mem_ptr[0].mem_type);
        src1_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
          src1_desc->mem_ptr[0].shared_ptr, src1_desc->mem_ptr[0].mem_type);
        if( src2_desc != NULL)
        {
            src2_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
              src2_desc->mem_ptr[0].shared_ptr, src2_desc->mem_ptr[0].mem_type);
        }
        if( src3_desc != NULL)
        {
            src3_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
              src3_desc->mem_ptr[0].shared_ptr, src3_desc->mem_ptr[0].mem_type);
        }
        for(plane_idx=0; plane_idx<dst_desc->planes; plane_idx++)
        {
            dst_desc->mem_ptr[plane_idx].target_ptr = tivxMemShared2TargetPtr(
              dst_desc->mem_ptr[plane_idx].shared_ptr, dst_desc->mem_ptr[plane_idx].mem_type);
        }

        tivxMemBufferMap(src0_desc->mem_ptr[0].target_ptr,
           src0_desc->mem_size[0], src0_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferMap(src1_desc->mem_ptr[0].target_ptr,
           src1_desc->mem_size[0], src1_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        if( src2_desc != NULL)
        {
            tivxMemBufferMap(src2_desc->mem_ptr[0].target_ptr,
               src2_desc->mem_size[0], src2_desc->mem_ptr[0].mem_type,
                VX_READ_ONLY);
        }
        if( src3_desc != NULL)
        {
            tivxMemBufferMap(src3_desc->mem_ptr[0].target_ptr,
               src3_desc->mem_size[0], src3_desc->mem_ptr[0].mem_type,
                VX_READ_ONLY);
        }
        for(plane_idx=0; plane_idx<dst_desc->planes; plane_idx++)
        {
            tivxMemBufferMap(dst_desc->mem_ptr[plane_idx].target_ptr,
               dst_desc->mem_size[plane_idx], dst_desc->mem_ptr[plane_idx].mem_type,
                VX_WRITE_ONLY);
        }


        /* call kernel processing function */
        {
            VXLIB_bufParams2D_t *vxlib_src[3U];
            uint8_t *src_addr[3U];
            VXLIB_bufParams2D_t vxlib_src0;
            uint8_t *src0_addr;
            VXLIB_bufParams2D_t vxlib_src1;
            uint8_t *src1_addr;
            VXLIB_bufParams2D_t vxlib_src2;
            uint8_t *src2_addr;
            VXLIB_bufParams2D_t vxlib_src3;
            uint8_t *src3_addr;
            VXLIB_bufParams2D_t vxlib_dst;
            uint8_t *dst_addr[4U];
            vx_rectangle_t rect;

            vxlib_src0.dim_x = src0_desc->imagepatch_addr[0U].dim_x;
            vxlib_src0.dim_y = src0_desc->imagepatch_addr[0U].dim_y;
            vxlib_src0.stride_y = src0_desc->imagepatch_addr[0U].stride_y;
            vxlib_src0.data_type = VXLIB_UINT8;

            /* Get the correct offset of the images from the valid roi parameter,
             */
            rect = src0_desc->valid_roi;

            src0_addr = (uint8_t *)((uint32_t)src0_desc->mem_ptr[0U].target_ptr +
                ownComputePatchOffset(rect.start_x, rect.start_y,
                &src0_desc->imagepatch_addr[0U]));

            vxlib_src1.dim_x = src1_desc->imagepatch_addr[0U].dim_x;
            vxlib_src1.dim_y = src1_desc->imagepatch_addr[0U].dim_y;
            vxlib_src1.stride_y = src1_desc->imagepatch_addr[0U].stride_y;
            vxlib_src1.data_type = VXLIB_UINT8;

            /* Get the correct offset of the images from the valid roi parameter,
             */
            rect = src1_desc->valid_roi;

            src1_addr = (uint8_t *)((uint32_t)src1_desc->mem_ptr[0U].target_ptr +
                ownComputePatchOffset(rect.start_x, rect.start_y,
                &src1_desc->imagepatch_addr[0U]));

            if(src2_desc != NULL)
            {
                vxlib_src2.dim_x = src2_desc->imagepatch_addr[0U].dim_x;
                vxlib_src2.dim_y = src2_desc->imagepatch_addr[0U].dim_y;
                vxlib_src2.stride_y = src2_desc->imagepatch_addr[0U].stride_y;
                vxlib_src2.data_type = VXLIB_UINT8;

                /* Get the correct offset of the images from the valid roi parameter,
                 */
                rect = src2_desc->valid_roi;

                src2_addr = (uint8_t *)((uint32_t)src2_desc->mem_ptr[0U].target_ptr +
                    ownComputePatchOffset(rect.start_x, rect.start_y,
                    &src2_desc->imagepatch_addr[0U]));
            }

            if(src3_desc != NULL)
            {
                /* Get the correct offset of the images from the valid roi parameter,
                 */
                rect = src3_desc->valid_roi;

                vxlib_src3.dim_x = src3_desc->imagepatch_addr[0U].dim_x;
                vxlib_src3.dim_y = src3_desc->imagepatch_addr[0U].dim_y;
                vxlib_src3.stride_y = src3_desc->imagepatch_addr[0U].stride_y;
                vxlib_src3.data_type = VXLIB_UINT8;

                src3_addr = (uint8_t *)((uint32_t)src3_desc->mem_ptr[0U].target_ptr +
                    ownComputePatchOffset(rect.start_x, rect.start_y,
                    &src3_desc->imagepatch_addr[0U]));
            }

            src_addr[0] = src0_addr;
            src_addr[1] = src1_addr;
            src_addr[2] = src2_addr;
            vxlib_src[0] = &vxlib_src0;
            vxlib_src[1] = &vxlib_src1;
            vxlib_src[2] = &vxlib_src2;

            /* Get the correct offset of the images from the valid roi parameter,
             */
            rect = dst_desc->valid_roi;

            for(plane_idx=0; plane_idx<dst_desc->planes; plane_idx++)
            {
                dst_addr[plane_idx] = (uint8_t *)((uint32_t)dst_desc->mem_ptr[plane_idx].target_ptr +
                    ownComputePatchOffset(rect.start_x, rect.start_y,
                        &dst_desc->imagepatch_addr[plane_idx]));
            }

            if (   dst_desc->format == VX_DF_IMAGE_RGB
                || dst_desc->format == VX_DF_IMAGE_RGBX
                || dst_desc->format == VX_DF_IMAGE_YUYV
                || dst_desc->format == VX_DF_IMAGE_UYVY
                )
            {
                vxlib_dst.dim_x = dst_desc->imagepatch_addr[0U].dim_x;
                vxlib_dst.dim_y = dst_desc->imagepatch_addr[0U].dim_y;
                vxlib_dst.stride_y = dst_desc->imagepatch_addr[0U].stride_y;
                vxlib_dst.data_type = VXLIB_UINT8;

                if( dst_desc->format == VX_DF_IMAGE_RGB)
                {
                    status = VXLIB_channelCombine_3to1_i8u_o8u(
                        src0_addr, &vxlib_src0,
                        src1_addr, &vxlib_src1,
                        src2_addr, &vxlib_src2,
                        dst_addr[0U], &vxlib_dst
                        );
                }
                else
                if( dst_desc->format == VX_DF_IMAGE_RGBX)
                {
                    status = VXLIB_channelCombine_4to1_i8u_o8u(
                        src0_addr, &vxlib_src0,
                        src1_addr, &vxlib_src1,
                        src2_addr, &vxlib_src2,
                        src3_addr, &vxlib_src3,
                        dst_addr[0U], &vxlib_dst
                        );
                }
                else
                if( dst_desc->format == VX_DF_IMAGE_YUYV)
                {
                    status = VXLIB_channelCombine_yuyv_i8u_o8u(
                        src0_addr, &vxlib_src0,
                        src1_addr, &vxlib_src1,
                        src2_addr, &vxlib_src2,
                        dst_addr[0U], &vxlib_dst,
                        0
                        );
                }
                else
                if( dst_desc->format == VX_DF_IMAGE_UYVY)
                {
                    status = VXLIB_channelCombine_yuyv_i8u_o8u(
                        src0_addr, &vxlib_src0,
                        src1_addr, &vxlib_src1,
                        src2_addr, &vxlib_src2,
                        dst_addr[0U], &vxlib_dst,
                        1
                        );
                }
            }
            else
            if (   dst_desc->format == VX_DF_IMAGE_IYUV
                || dst_desc->format == VX_DF_IMAGE_YUV4
                )
            {
                for(plane_idx=0; plane_idx<dst_desc->planes; plane_idx++)
                {
                    vxlib_dst.dim_x =
                        dst_desc->imagepatch_addr[plane_idx].dim_x
                        /dst_desc->imagepatch_addr[plane_idx].step_x;
                    vxlib_dst.dim_y =
                        dst_desc->imagepatch_addr[plane_idx].dim_y
                        /dst_desc->imagepatch_addr[plane_idx].step_y;
                    vxlib_dst.stride_y =
                        dst_desc->imagepatch_addr[plane_idx].stride_y;
                    vxlib_dst.data_type = VXLIB_UINT8;

                    status = VXLIB_channelCopy_1to1_i8u_o8u(
                        src_addr[plane_idx], vxlib_src[plane_idx],
                        dst_addr[plane_idx], &vxlib_dst
                        );
                }
            }
            else
            if (   dst_desc->format == VX_DF_IMAGE_NV12
                || dst_desc->format == VX_DF_IMAGE_NV21
                )
            {
                for(plane_idx=0; plane_idx<dst_desc->planes; plane_idx++)
                {
                    vxlib_dst.dim_x =
                        dst_desc->imagepatch_addr[plane_idx].dim_x
                        /dst_desc->imagepatch_addr[plane_idx].step_x;
                    vxlib_dst.dim_y =
                        dst_desc->imagepatch_addr[plane_idx].dim_y
                        /dst_desc->imagepatch_addr[plane_idx].step_y;
                    vxlib_dst.stride_y =
                        dst_desc->imagepatch_addr[plane_idx].stride_y;
                    vxlib_dst.data_type = VXLIB_UINT8;

                    if(plane_idx==0)
                    {
                        status = VXLIB_channelCopy_1to1_i8u_o8u(
                            src0_addr, &vxlib_src0,
                            dst_addr[plane_idx], &vxlib_dst
                            );
                    }
                    else
                    if(plane_idx==1)
                    {
                        if(dst_desc->format == VX_DF_IMAGE_NV21)
                        {
                            status = VXLIB_channelCombine_2to1_i8u_o8u(
                                src1_addr, &vxlib_src1,
                                src2_addr, &vxlib_src2,
                                dst_addr[plane_idx], &vxlib_dst,
                                0 /* Index of V component in C plane */
                                );
                        }
                        else
                        {
                            status = VXLIB_channelCombine_2to1_i8u_o8u(
                                src2_addr, &vxlib_src2,
                                src1_addr, &vxlib_src1,
                                dst_addr[plane_idx], &vxlib_dst,
                                0 /* Index of V component in C plane */
                                );
                        }
                    }
                }
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

        tivxMemBufferUnmap(src0_desc->mem_ptr[0].target_ptr,
           src0_desc->mem_size[0], src0_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferUnmap(src1_desc->mem_ptr[0].target_ptr,
           src1_desc->mem_size[0], src1_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        if( src2_desc != NULL)
        {
            tivxMemBufferUnmap(src2_desc->mem_ptr[0].target_ptr,
               src2_desc->mem_size[0], src2_desc->mem_ptr[0].mem_type,
                VX_READ_ONLY);
        }
        if( src3_desc != NULL)
        {
            tivxMemBufferUnmap(src3_desc->mem_ptr[0].target_ptr,
               src3_desc->mem_size[0], src3_desc->mem_ptr[0].mem_type,
                VX_READ_ONLY);
        }
        for(plane_idx=0; plane_idx<dst_desc->planes; plane_idx++)
        {
            tivxMemBufferUnmap(dst_desc->mem_ptr[plane_idx].target_ptr,
               dst_desc->mem_size[plane_idx], dst_desc->mem_ptr[plane_idx].mem_type,
                VX_WRITE_ONLY);
        }


    }

    return status;
}

vx_status tivxChannelCombineCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxChannelCombineDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxChannelCombineControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelChannelCombine()
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
        vx_channel_combine_target_kernel = tivxAddTargetKernel(
                            VX_KERNEL_CHANNEL_COMBINE,
                            target_name,
                            tivxChannelCombine,
                            tivxChannelCombineCreate,
                            tivxChannelCombineDelete,
                            tivxChannelCombineControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelChannelCombine()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_channel_combine_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_channel_combine_target_kernel = NULL;
    }
}


