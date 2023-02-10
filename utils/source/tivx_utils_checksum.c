/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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

#include <TI/tivx_mem.h>
#include <TI/tivx_debug.h>
#include <tivx_utils_checksum.h>

uint32_t tivx_utils_simple_image_checksum(vx_image image, uint8_t plane_id, vx_rectangle_t rect)
{
    vx_status                   status = (vx_status)VX_FAILURE;
    vx_imagepatch_addressing_t  image_addr;
    vx_map_id                   map_id;
    uint32_t                   *data_ptr;
    uint32_t                    sum = 0U;
    uint32_t                    stride_xby2;
    int32_t                    i;
    int32_t                    j;

    if(NULL != image)
    {
        status = vxMapImagePatch(image,
            &rect,
            plane_id,
            &map_id,
            &image_addr,
            (void**) &data_ptr,
            (vx_enum)VX_READ_ONLY,
            (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_NOGAP_X
            );

        stride_xby2 = (image_addr.stride_x == 0) ? 3U : (((uint32_t)image_addr.stride_x*2U)/(uint32_t)image_addr.step_x);

        VX_PRINT(VX_ZONE_INFO, "plane_id = %d \n", plane_id);
        VX_PRINT(VX_ZONE_INFO, "stride_xby2 = %d \n", stride_xby2);
        VX_PRINT(VX_ZONE_INFO, "image_addr.dim_x = %d \n", image_addr.dim_x);
        VX_PRINT(VX_ZONE_INFO, "image_addr.dim_y = %d \n", image_addr.dim_y);
        VX_PRINT(VX_ZONE_INFO, "image_addr.stride_x = %d \n", image_addr.stride_x);
        VX_PRINT(VX_ZONE_INFO, "image_addr.stride_y = %d \n", image_addr.stride_y);

        if ((vx_status)VX_SUCCESS == status)
        {
            for (i = 0; i < (((int32_t)image_addr.dim_y * image_addr.stride_y) / 4); i += (image_addr.stride_y / 4))
            {
                int32_t temp_bytes = (((int32_t)image_addr.dim_x * (int32_t)stride_xby2) / 2);

                for (j = 0; j < (temp_bytes / 4); j++)
                {
                    sum += data_ptr[i + j];
                }

                if (0 != (temp_bytes % 4))
                {
                    uint32_t bitshift = (4U - ((uint32_t)temp_bytes % 4U)) * 8U;
                    sum += (data_ptr[i + j] << bitshift) >> bitshift;
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "vxMapImagePatch failed \n");
        }

        vxUnmapImagePatch(image, map_id);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null Image pointer \n");
    }

    return sum;
}

uint32_t tivx_utils_user_data_object_checksum(vx_user_data_object user_data_object, uint32_t offset_byte, uint32_t num_bytes)
{
    vx_status                   status = (vx_status)VX_FAILURE;
    vx_map_id                   map_id;
    uint32_t                   *data_ptr;
    uint32_t                    sum = 0U;
    int32_t                    j;

    if(NULL != user_data_object)
    {
        status = vxMapUserDataObject(user_data_object,
            offset_byte,
            num_bytes,
            &map_id,
            (void**) &data_ptr,
            (vx_enum)VX_READ_ONLY,
            (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_NOGAP_X
            );

        if ((vx_status)VX_SUCCESS == status)
        {
            for (j = 0; j < (num_bytes / 4); j++)
            {
                sum += data_ptr[j];
            }

            if (0 != (num_bytes % 4))
            {
                uint32_t bitshift = (4U - ((uint32_t)num_bytes % 4U)) * 8U;
                sum += (data_ptr[j] << bitshift) >> bitshift;
            }
        }

        vxUnmapUserDataObject(user_data_object, map_id);
    }

    return sum;
}

uint32_t tivx_utils_tensor_checksum(vx_tensor tensor_object, vx_size number_of_dimensions, vx_size * view_start, vx_size * view_end, vx_size * user_stride)
{
    vx_status  status = (vx_status)VX_FAILURE;
    vx_map_id  map_id;
    uint32_t   *data_ptr;
    uint32_t   sum = 0U;

    if((number_of_dimensions != 0) && (NULL != tensor_object))
    {
        status = tivxMapTensorPatch(tensor_object,
                                    number_of_dimensions,
                                    view_start,
                                    view_end,
                                    &map_id,
                                    user_stride,
                                    (void**) &data_ptr,
                                    (vx_enum)VX_READ_ONLY,
                                    (vx_enum)VX_MEMORY_TYPE_HOST);

        if ((vx_status)VX_SUCCESS == status)
        {
            vx_int32 dim0, dim1, dim2, dim3;
            vx_uint32 dim0_count = 1;
            vx_uint32 dim1_count = 1;
            vx_uint32 dim2_count = 1;
            vx_uint32 dim3_count = 1;
            vx_uint32 offset = 0;
            vx_uint32 num_bytes;

            if (number_of_dimensions >= 1) dim0_count = view_end[0];
            if (number_of_dimensions >= 2) dim1_count = view_end[1];
            if (number_of_dimensions >= 3) dim2_count = view_end[2];
            if (number_of_dimensions >= 4) dim3_count = view_end[3];

            for(dim3 = 0; dim3 < dim3_count; dim3++)
            {
                for(dim2 = 0; dim2 < dim2_count; dim2++)
                {
                    for(dim1 = 0; dim1 < dim1_count; dim1++)
                    {
                        offset += (user_stride[1]);
                        num_bytes = dim0_count * user_stride[0];
                        for (dim0 = 0; dim0 < (num_bytes/4); dim0++)
                        {
                            sum += *((uint32_t*)((uint8_t*)data_ptr+offset) + dim0);
                        }
                        if (0 != (num_bytes % 4))
                        {
                            uint32_t bitshift = (4U - ((uint32_t)num_bytes % 4U)) * 8U;
                            sum += ( (*((uint32_t*)((uint8_t*)data_ptr+offset) + dim0)) << bitshift) >> bitshift;
                        }
                    }
                }
            }
        }
        tivxUnmapTensorPatch(tensor_object, map_id);
    }

    return sum;
}
