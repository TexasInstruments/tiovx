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
#include <tivx_kernel_channel_combine.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <stdio.h>

static tivx_target_kernel vx_channel_combine_target_kernel = NULL;

static vx_status VX_CALLBACK tivxChannelCombine(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxChannelCombineCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxChannelCombineDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxChannelCombine(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src0_desc;
    tivx_obj_desc_image_t *src1_desc;
    tivx_obj_desc_image_t *src2_desc;
    tivx_obj_desc_image_t *src3_desc;
    tivx_obj_desc_image_t *dst_desc;
    uint16_t plane_idx;

    if ((num_params != TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE0_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE1_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX]))
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        void *src0_desc_target_ptr;
        void *src1_desc_target_ptr;
        void *src2_desc_target_ptr = NULL;
        void *src3_desc_target_ptr = NULL;
        void *dst_desc_target_ptr[4U] = {NULL, NULL, NULL, NULL};

        src0_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE0_IDX];
        src1_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE1_IDX];
        src2_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE2_IDX];
        src3_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_PLANE3_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX];

        src0_desc_target_ptr = tivxMemShared2TargetPtr(&src0_desc->mem_ptr[0]);
        src1_desc_target_ptr = tivxMemShared2TargetPtr(&src1_desc->mem_ptr[0]);
        if( src2_desc != NULL)
        {
            src2_desc_target_ptr = tivxMemShared2TargetPtr(&src2_desc->mem_ptr[0]);
        }
        if( src3_desc != NULL)
        {
            src3_desc_target_ptr = tivxMemShared2TargetPtr(&src3_desc->mem_ptr[0]);
        }
        for(plane_idx=0; plane_idx<dst_desc->planes; plane_idx++)
        {
            dst_desc_target_ptr[plane_idx] = tivxMemShared2TargetPtr(&dst_desc->mem_ptr[plane_idx]);
        }

        tivxCheckStatus(&status, tivxMemBufferMap(src0_desc_target_ptr,
           src0_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferMap(src1_desc_target_ptr,
           src1_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        if( src2_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferMap(src2_desc_target_ptr,
               src2_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
        }
        if( src3_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferMap(src3_desc_target_ptr,
               src3_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
        }
        for(plane_idx=0; plane_idx<dst_desc->planes; plane_idx++)
        {
            tivxCheckStatus(&status, tivxMemBufferMap(dst_desc_target_ptr[plane_idx],
               dst_desc->mem_size[plane_idx], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }


        /* call kernel processing function */
        {
            VXLIB_bufParams2D_t *vxlib_src[3U];
            uint8_t *src_addr[3U];
            VXLIB_bufParams2D_t vxlib_src0;
            uint8_t *src0_addr = NULL;
            VXLIB_bufParams2D_t vxlib_src1;
            uint8_t *src1_addr = NULL;
            VXLIB_bufParams2D_t vxlib_src2;
            uint8_t *src2_addr = NULL;
            VXLIB_bufParams2D_t vxlib_src3;
            uint8_t *src3_addr = NULL;
            VXLIB_bufParams2D_t vxlib_dst;
            uint8_t *dst_addr[4U] = {NULL};

            tivxInitBufParams(src0_desc, &vxlib_src0);
            tivxSetPointerLocation(src0_desc, &src0_desc_target_ptr, &src0_addr);

            tivxInitBufParams(src1_desc, &vxlib_src1);
            tivxSetPointerLocation(src1_desc, &src1_desc_target_ptr, &src1_addr);

            if(src2_desc != NULL)
            {
                tivxInitBufParams(src2_desc, &vxlib_src2);
                tivxSetPointerLocation(src2_desc, &src2_desc_target_ptr, &src2_addr);
            }

            if(src3_desc != NULL)
            {
                tivxInitBufParams(src3_desc, &vxlib_src3);
                tivxSetPointerLocation(src3_desc, &src3_desc_target_ptr, &src3_addr);
            }

            src_addr[0] = src0_addr;
            src_addr[1] = src1_addr;
            src_addr[2] = src2_addr;
            vxlib_src[0] = &vxlib_src0;
            vxlib_src[1] = &vxlib_src1;
            vxlib_src[2] = &vxlib_src2;

            tivxSetPointerLocation(dst_desc, dst_desc_target_ptr, (uint8_t**)&dst_addr);

            if (   (dst_desc->format == (vx_df_image)VX_DF_IMAGE_RGB)
                || (dst_desc->format == (vx_df_image)VX_DF_IMAGE_RGBX)
                || (dst_desc->format == (vx_df_image)VX_DF_IMAGE_YUYV)
                || (dst_desc->format == (vx_df_image)VX_DF_IMAGE_UYVY)
                )
            {
                tivxInitBufParams(dst_desc, &vxlib_dst);

                if( dst_desc->format == (vx_df_image)VX_DF_IMAGE_RGB)
                {
                    vxlib_dst.data_type = (uint32_t)VXLIB_UINT24;

                    status = (vx_status)VXLIB_channelCombine_3to1_i8u_o8u(
                        src0_addr, &vxlib_src0,
                        src1_addr, &vxlib_src1,
                        src2_addr, &vxlib_src2,
                        dst_addr[0U], &vxlib_dst
                        );
                }
                else
                if( dst_desc->format == (vx_df_image)VX_DF_IMAGE_RGBX)
                {
                    vxlib_dst.data_type = (uint32_t)VXLIB_UINT32;

                    status = (vx_status)VXLIB_channelCombine_4to1_i8u_o8u(
                        src0_addr, &vxlib_src0,
                        src1_addr, &vxlib_src1,
                        src2_addr, &vxlib_src2,
                        src3_addr, &vxlib_src3,
                        dst_addr[0U], &vxlib_dst
                        );
                }
                else
                if( dst_desc->format == (vx_df_image)VX_DF_IMAGE_YUYV)
                {
                    vxlib_dst.data_type = (uint32_t)VXLIB_UINT16;

                    status = (vx_status)VXLIB_channelCombine_yuyv_i8u_o8u(
                        src0_addr, &vxlib_src0,
                        src1_addr, &vxlib_src1,
                        src2_addr, &vxlib_src2,
                        dst_addr[0U], &vxlib_dst,
                        0
                        );
                }
                else /* format is (vx_df_image)VX_DF_IMAGE_UYVY */
                {
                    vxlib_dst.data_type = (uint32_t)VXLIB_UINT16;

                    status = (vx_status)VXLIB_channelCombine_yuyv_i8u_o8u(
                        src0_addr, &vxlib_src0,
                        src1_addr, &vxlib_src1,
                        src2_addr, &vxlib_src2,
                        dst_addr[0U], &vxlib_dst,
                        1
                        );
                }
            }
            else
            if ((dst_desc->format == (vx_df_image)VX_DF_IMAGE_IYUV)
                || (dst_desc->format == (vx_df_image)VX_DF_IMAGE_YUV4)
                )
            {
                for(plane_idx=0; plane_idx<dst_desc->planes; plane_idx++)
                {
                    vxlib_dst.dim_x =
                        (dst_desc->valid_roi.end_x - dst_desc->valid_roi.start_x)
                        /dst_desc->imagepatch_addr[plane_idx].step_x;
                    vxlib_dst.dim_y =
                        (dst_desc->valid_roi.end_y - dst_desc->valid_roi.start_y)
                        /dst_desc->imagepatch_addr[plane_idx].step_y;
                    vxlib_dst.stride_y =
                        dst_desc->imagepatch_addr[plane_idx].stride_y;
                    vxlib_dst.data_type = (uint32_t)VXLIB_UINT8;

                    status = (vx_status)VXLIB_channelCopy_1to1_i8u_o8u(
                        src_addr[plane_idx], vxlib_src[plane_idx],
                        dst_addr[plane_idx], &vxlib_dst
                        );
                }
            }
            else
            if ((dst_desc->format == (vx_df_image)VX_DF_IMAGE_NV12)
                || (dst_desc->format == (vx_df_image)VX_DF_IMAGE_NV21)
                )
            {
                for(plane_idx=0; plane_idx<dst_desc->planes; plane_idx++)
                {
                    vxlib_dst.dim_x =
                        (dst_desc->valid_roi.end_x - dst_desc->valid_roi.start_x)
                        /dst_desc->imagepatch_addr[plane_idx].step_x;
                    vxlib_dst.dim_y =
                        (dst_desc->valid_roi.end_y - dst_desc->valid_roi.start_y)
                        /dst_desc->imagepatch_addr[plane_idx].step_y;
                    vxlib_dst.stride_y =
                        dst_desc->imagepatch_addr[plane_idx].stride_y;
                    vxlib_dst.data_type = (uint32_t)VXLIB_UINT8;

                    if(plane_idx==0U)
                    {
                        status = (vx_status)VXLIB_channelCopy_1to1_i8u_o8u(
                            src0_addr, &vxlib_src0,
                            dst_addr[plane_idx], &vxlib_dst
                            );
                    }
                    else
                    if(plane_idx==1U)
                    {
                        if(dst_desc->format == (vx_df_image)VX_DF_IMAGE_NV21)
                        {
                            status = (vx_status)VXLIB_channelCombine_2to1_i8u_o8u(
                                src2_addr, &vxlib_src2,
                                src1_addr, &vxlib_src1,
                                dst_addr[plane_idx], &vxlib_dst
                                );
                        }
                        else
                        {
                            status = (vx_status)VXLIB_channelCombine_2to1_i8u_o8u(
                                src1_addr, &vxlib_src1,
                                src2_addr, &vxlib_src2,
                                dst_addr[plane_idx], &vxlib_dst
                                );
                        }
                    }
                    else
                    {
                        status = (vx_status)VX_FAILURE;
                    }
                }
            }
            else
            {
                status = (vx_status)VX_FAILURE;
            }

            if ((vx_status)VXLIB_SUCCESS != status)
            {
                status = (vx_status)VX_FAILURE;
            }
        }


        /* kernel processing function complete */

        tivxCheckStatus(&status, tivxMemBufferUnmap(src0_desc_target_ptr,
           src0_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferUnmap(src1_desc_target_ptr,
           src1_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));
        if( src2_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(src2_desc_target_ptr,
               src2_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
        }
        if( src3_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(src3_desc_target_ptr,
               src3_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
        }
        for(plane_idx=0; plane_idx<dst_desc->planes; plane_idx++)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(dst_desc_target_ptr[plane_idx],
               dst_desc->mem_size[plane_idx], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }


    }

    return status;
}

static vx_status VX_CALLBACK tivxChannelCombineCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

static vx_status VX_CALLBACK tivxChannelCombineDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelChannelCombine(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_channel_combine_target_kernel = tivxAddTargetKernel(
                            (vx_enum)VX_KERNEL_CHANNEL_COMBINE,
                            target_name,
                            tivxChannelCombine,
                            tivxChannelCombineCreate,
                            tivxChannelCombineDelete,
                            NULL,
                            NULL);
    }
}

void tivxRemoveTargetKernelChannelCombine(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_channel_combine_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_channel_combine_target_kernel = NULL;
    }
}


