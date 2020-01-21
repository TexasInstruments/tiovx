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

        stride_xby2 = (image_addr.stride_x == 0) ? 3U : ((uint32_t)image_addr.stride_x*2U);

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

        vxUnmapImagePatch(image, map_id);
    }

    return sum;
}
