/*
 *
 * Copyright (c) 2017-2018 Texas Instruments Incorporated
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
#ifdef BUILD_DMPAC_DOF

#include <VX/vx.h>
#include <TI/tivx.h>
#include <string.h>
#include "test_engine/test.h"
#include "test_tiovx/utils/test_utils_file_bmp_rd_wr.h"
#include "tivx_utils_checksum.h"
#include "test_hwa_common.h"

TESTCASE(tivxHwaDmpacDof, CT_VXContext, ct_setup_vx_context, 0)

#define MAX_ABS_FILENAME   (1024u)

static void make_filename(char *abs_filename, char *filename_prefix, uint32_t level)
{
    snprintf(abs_filename, MAX_ABS_FILENAME, "%s/%s%d.bmp",
        ct_get_test_file_path(), filename_prefix, level);
}

static vx_status load_image_into_pyramid_level(vx_pyramid pyr, uint32_t level, char *filename_prefix)
{
    char filename[MAX_ABS_FILENAME];
    vx_image image;
    vx_status status = 0;

    make_filename(filename, filename_prefix, level);
    image = vxGetPyramidLevel(pyr, level);
    status = test_utils_load_vximage_from_bmpfile(image, filename, vx_true_e);
    vxReleaseImage(&image);
    return status;
}

static vx_status save_image_from_dof(vx_image flow_vector_img, vx_image confidence_img, char *filename_prefix)
{
    char filename[MAX_ABS_FILENAME];
    vx_status status;

    snprintf(filename, MAX_ABS_FILENAME, "%s/%s_flow_img.bmp",
        ct_get_test_file_path(), filename_prefix);

    status = test_utils_save_vximage_to_bmpfile(filename, flow_vector_img);
    if(status == VX_SUCCESS)
    {
        snprintf(filename, MAX_ABS_FILENAME, "%s/%s_confidence_img.bmp",
            ct_get_test_file_path(), filename_prefix);

        status = test_utils_save_vximage_to_bmpfile(filename, confidence_img);
    }

    return status;
}

/*
    test_num=
    0: Every line has 1/4 of original pixels (alternating lines have different mask pixel positions)
    1: Even lines have 1/4 of original pixels, odd lines have 0 pixels (odd lines are trash data in HW, SW should ignore, or don't do this)
    2: 2 lines have 1/4 of original pixels, alternating 2 lines have 0 pixels (not supported in HW)
*/
static void initialize_sof_mask(vx_image sof_mask, uint32_t width, uint32_t height, uint32_t *flow_width, uint32_t *flow_height, uint32_t test_num)
{
    vx_rectangle_t              rect;
    vx_imagepatch_addressing_t  image_addr;
    vx_map_id                   map_id;
    uint8_t                    *data_ptr;
    int                         i, j;

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;

    if(NULL != sof_mask)
    {
        VX_CALL(vxMapImagePatch(sof_mask,
            &rect,
            0,
            &map_id,
            &image_addr,
            (void**) &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            ));

        for(j = 0; j < height; j+=4)
        {
            /* Line 0 */
            for(i = 0; i < width; i++)
            {
                data_ptr[j*image_addr.stride_y + i] = (uint8_t)0x11; /* 2/8 : Every 4th pixel even lines */
            }

            /* Line 1 */
            for(i = 0; i < width; i++)
            {
                if(test_num == 0 || test_num == 2)
                {
                    data_ptr[(j+1)*image_addr.stride_y + i] = (uint8_t)0x18; /* 2/8 : 2 pixels next to each other odd lines*/
                }
                else if (test_num == 1)
                {
                    data_ptr[(j+1)*image_addr.stride_y + i] = (uint8_t)0x0; /* 0/8 : empty odd lines*/
                }
            }

            /* Line 2 */
            for(i = 0; i < width; i++)
            {
                if(test_num == 0 || test_num == 1)
                {
                    data_ptr[(j+2)*image_addr.stride_y + i] = (uint8_t)0x11; /* 2/8 : Every 4th pixel even lines */
                }
                else if(test_num == 2)
                {
                    data_ptr[(j+2)*image_addr.stride_y + i] = (uint8_t)0x0; /* 0/8 : empty odd lines*/
                }
            }

            /* Line 3 */
            for(i = 0; i < width; i++)
            {
                if(test_num == 0)
                {
                    data_ptr[(j+3)*image_addr.stride_y + i] = (uint8_t)0x18; /* 2/8 : 2 pixels next to each other odd lines*/
                }
                else if(test_num == 1 || test_num == 2)
                {
                    data_ptr[(j+3)*image_addr.stride_y + i] = (uint8_t)0x0; /* 0/8 : empty odd lines*/
                }
            }
        }

        VX_CALL(vxUnmapImagePatch(sof_mask, map_id));

        *flow_width = width * 8 / 4;
        if(test_num == 2)
        {
            *flow_height = height/2;
        }
        else
        {
            *flow_height = height;
        }
    }

    return;
}

static vx_status copy_flow_image(vx_image in_image, uint32_t width, uint32_t height, vx_image out_image)
{
    vx_status                   status = VX_FAILURE;
    vx_rectangle_t              rect;
    vx_imagepatch_addressing_t  in_image_addr;
    vx_imagepatch_addressing_t  out_image_addr;
    vx_map_id                   in_map_id;
    vx_map_id                   out_map_id;
    uint32_t                   *in_data_ptr;
    uint32_t                   *out_data_ptr;

    int                         i, j;

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;

    if(NULL != out_image)
    {
        status = vxMapImagePatch(in_image,
            &rect,
            0,
            &in_map_id,
            &in_image_addr,
            (void**) &in_data_ptr,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        status |= vxMapImagePatch(out_image,
            &rect,
            0,
            &out_map_id,
            &out_image_addr,
            (void**) &out_data_ptr,
            VX_READ_AND_WRITE,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        if (VX_SUCCESS == status)
        {
            for(j = 0; j < height; j++)
            {
                for(i = 0; i < width; i++)
                {
                    out_data_ptr[j*out_image_addr.stride_y/4 + i] = in_data_ptr[j*in_image_addr.stride_y/4 + i];
                }
            }
        }

        status |= vxUnmapImagePatch(in_image, in_map_id);
        status |= vxUnmapImagePatch(out_image, out_map_id);
    }

    return status;
}

static vx_status addParamByNodeIndex(vx_graph  graph,
                                     vx_node   node,
                                     vx_uint32 nodeParamIndex)
{
    vx_parameter    param;
    vx_status       vxStatus;

    vxStatus = VX_SUCCESS;
    param = vxGetParameterByIndex(node, nodeParamIndex);

    if (param == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "[%s:%d] vxGetParameterByIndex() failed\n",
                    __FUNCTION__, __LINE__);

        vxStatus = VX_FAILURE;
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        vxStatus = vxAddParameterToGraph(graph, param);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "[%s:%d] vxAddParameterToGraph() failed\n",
                        __FUNCTION__, __LINE__);
        }
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        vxStatus = vxReleaseParameter(&param);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "[%s:%d] vxReleaseParameter() failed\n",
                        __FUNCTION__, __LINE__);
        }
    }

    return vxStatus;
}

typedef struct {
    const char* testName;
    int median_filter;
    int motion_smoothness;
    int vertical_range;
    int horizontal_range;
    int iir_filter;
    int enable_lk;
    int enable_sof;
} Arg;

static uint32_t dof_checksums_ref[3*3*3*3*2*2*2] = {
    0xfd80331c, 0xd1f0212f, 0xe1c9e2bb, 0x3c0842ab, 0xfd80331c, 0xd1f0212f, 0xe1ca0611, 0x3c084b52,
    0xfd80331c, 0xd1f0212f, 0xe1c9e8de, 0x3c08444b, 0xfd80331c, 0xd1f0212f, 0xe1c9e2bb, 0x3c0842ab,
    0xfd80331c, 0xd1f0212f, 0xe1ca0611, 0x3c084b52, 0xfd80331c, 0xd1f0212f, 0xe1c9e8de, 0x3c08444b,
    0xfd80331c, 0xd1f0212f, 0xe1c9e2bb, 0x3c0842ab, 0xfd80331c, 0xd1f0212f, 0xe1ca0611, 0x3c084b52,
    0xfd80331c, 0xd1f0212f, 0xe1c9e8de, 0x3c08444b, 0xfd80331c, 0xd1f0212f, 0xe1c9e2bb, 0x3c0842ab,
    0xfd80331c, 0xd1f0212f, 0xe1ca0611, 0x3c084b52, 0xfd80331c, 0xd1f0212f, 0xe1c9e8de, 0x3c08444b,
    0xfd80331c, 0xd1f0212f, 0xe1c9e2bb, 0x3c0842ab, 0xfd80331c, 0xd1f0212f, 0xe1ca0611, 0x3c084b52,
    0xfd80331c, 0xd1f0212f, 0xe1c9e8de, 0x3c08444b, 0xfd80331c, 0xd1f0212f, 0xe1c9e2bb, 0x3c0842ab,
    0xfd80331c, 0xd1f0212f, 0xe1ca0611, 0x3c084b52, 0xfd80331c, 0xd1f0212f, 0xe1c9e8de, 0x3c08444b,
    0xfd80331c, 0xd1f0212f, 0xe1c9e2bb, 0x3c0842ab, 0xfd80331c, 0xd1f0212f, 0xe1ca0611, 0x3c084b52,
    0xfd80331c, 0xd1f0212f, 0xe1c9e8de, 0x3c08444b, 0xfd80331c, 0xd1f0212f, 0xe1c9e2bb, 0x3c0842ab,
    0xfd80331c, 0xd1f0212f, 0xe1ca0611, 0x3c084b52, 0xfd80331c, 0xd1f0212f, 0xe1c9e8de, 0x3c08444b,
    0xfd80331c, 0xd1f0212f, 0xe1c9e2bb, 0x3c0842ab, 0xfd80331c, 0xd1f0212f, 0xe1ca0611, 0x3c084b52,
    0xfd80331c, 0xd1f0212f, 0xe1c9e8de, 0x3c08444b, 0xcb938688, 0x4c5ef73e, 0x72a02c94, 0x28186490,
    0xcb938688, 0x4c5ef73e, 0x72a04780, 0x2818680d, 0xcb938688, 0x4c5ef73e, 0x72a025dd, 0x28185fa8,
    0xcb938688, 0x4c5ef73e, 0x72a02c94, 0x28186490, 0xcb938688, 0x4c5ef73e, 0x72a04780, 0x2818680d,
    0xcb938688, 0x4c5ef73e, 0x72a025dd, 0x28185fa8, 0xcb938688, 0x4c5ef73e, 0x72a02c94, 0x28186490,
    0xcb938688, 0x4c5ef73e, 0x72a04780, 0x2818680d, 0xcb938688, 0x4c5ef73e, 0x72a025dd, 0x28185fa8,
    0xcb938688, 0x4c5ef73e, 0x72a02c94, 0x28186490, 0xcb938688, 0x4c5ef73e, 0x72a04780, 0x2818680d,
    0xcb938688, 0x4c5ef73e, 0x72a025dd, 0x28185fa8, 0xcb938688, 0x4c5ef73e, 0x72a02c94, 0x28186490,
    0xcb938688, 0x4c5ef73e, 0x72a04780, 0x2818680d, 0xcb938688, 0x4c5ef73e, 0x72a025dd, 0x28185fa8,
    0xcb938688, 0x4c5ef73e, 0x72a02c94, 0x28186490, 0xcb938688, 0x4c5ef73e, 0x72a04780, 0x2818680d,
    0xcb938688, 0x4c5ef73e, 0x72a025dd, 0x28185fa8, 0xcb938688, 0x4c5ef73e, 0x72a02c94, 0x28186490,
    0xcb938688, 0x4c5ef73e, 0x72a04780, 0x2818680d, 0xcb938688, 0x4c5ef73e, 0x72a025dd, 0x28185fa8,
    0xcb938688, 0x4c5ef73e, 0x72a02c94, 0x28186490, 0xcb938688, 0x4c5ef73e, 0x72a04780, 0x2818680d,
    0xcb938688, 0x4c5ef73e, 0x72a025dd, 0x28185fa8, 0xcb938688, 0x4c5ef73e, 0x72a02c94, 0x28186490,
    0xcb938688, 0x4c5ef73e, 0x72a04780, 0x2818680d, 0xcb938688, 0x4c5ef73e, 0x72a025dd, 0x28185fa8,
    0xb2b7226c, 0x082d935d, 0x2f2601b9, 0x142b5598, 0xb2b7226c, 0x082d935d, 0x2f26192d, 0x142b5a3b,
    0xb2b7226c, 0x082d935d, 0x2f25f41c, 0x142b50f8, 0xb2b7226c, 0x082d935d, 0x2f2601b9, 0x142b5598,
    0xb2b7226c, 0x082d935d, 0x2f26192d, 0x142b5a3b, 0xb2b7226c, 0x082d935d, 0x2f25f41c, 0x142b50f8,
    0xb2b7226c, 0x082d935d, 0x2f2601b9, 0x142b5598, 0xb2b7226c, 0x082d935d, 0x2f26192d, 0x142b5a3b,
    0xb2b7226c, 0x082d935d, 0x2f25f41c, 0x142b50f8, 0xb2b7226c, 0x082d935d, 0x2f2601b9, 0x142b5598,
    0xb2b7226c, 0x082d935d, 0x2f26192d, 0x142b5a3b, 0xb2b7226c, 0x082d935d, 0x2f25f41c, 0x142b50f8,
    0xb2b7226c, 0x082d935d, 0x2f2601b9, 0x142b5598, 0xb2b7226c, 0x082d935d, 0x2f26192d, 0x142b5a3b,
    0xb2b7226c, 0x082d935d, 0x2f25f41c, 0x142b50f8, 0xb2b7226c, 0x082d935d, 0x2f2601b9, 0x142b5598,
    0xb2b7226c, 0x082d935d, 0x2f26192d, 0x142b5a3b, 0xb2b7226c, 0x082d935d, 0x2f25f41c, 0x142b50f8,
    0xb2b7226c, 0x082d935d, 0x2f2601b9, 0x142b5598, 0xb2b7226c, 0x082d935d, 0x2f26192d, 0x142b5a3b,
    0xb2b7226c, 0x082d935d, 0x2f25f41c, 0x142b50f8, 0xb2b7226c, 0x082d935d, 0x2f2601b9, 0x142b5598,
    0xb2b7226c, 0x082d935d, 0x2f26192d, 0x142b5a3b, 0xb2b7226c, 0x082d935d, 0x2f25f41c, 0x142b50f8,
    0xb2b7226c, 0x082d935d, 0x2f2601b9, 0x142b5598, 0xb2b7226c, 0x082d935d, 0x2f26192d, 0x142b5a3b,
    0xb2b7226c, 0x082d935d, 0x2f25f41c, 0x142b50f8, 0xf46585e0, 0x9e5b3913, 0xb1553ba4, 0x3680c24a,
    0xf46585e0, 0x9e5b3913, 0xb154e6c1, 0x3680a9cf, 0xf46585e0, 0x9e5b3913, 0xb154c872, 0x3680a273,
    0xf46585e0, 0x9e5b3913, 0xb1553ba4, 0x3680c24a, 0xf46585e0, 0x9e5b3913, 0xb154e6c1, 0x3680a9cf,
    0xf46585e0, 0x9e5b3913, 0xb154c872, 0x3680a273, 0xf46585e0, 0x9e5b3913, 0xb1553ba4, 0x3680c24a,
    0xf46585e0, 0x9e5b3913, 0xb154e6c1, 0x3680a9cf, 0xf46585e0, 0x9e5b3913, 0xb154c872, 0x3680a273,
    0xf46585e0, 0x9e5b3913, 0xb1553ba4, 0x3680c24a, 0xf46585e0, 0x9e5b3913, 0xb154e6c1, 0x3680a9cf,
    0xf46585e0, 0x9e5b3913, 0xb154c872, 0x3680a273, 0xf46585e0, 0x9e5b3913, 0xb1553ba4, 0x3680c24a,
    0xf46585e0, 0x9e5b3913, 0xb154e6c1, 0x3680a9cf, 0xf46585e0, 0x9e5b3913, 0xb154c872, 0x3680a273,
    0xf46585e0, 0x9e5b3913, 0xb1553ba4, 0x3680c24a, 0xf46585e0, 0x9e5b3913, 0xb154e6c1, 0x3680a9cf,
    0xf46585e0, 0x9e5b3913, 0xb154c872, 0x3680a273, 0xf46585e0, 0x9e5b3913, 0xb1553ba4, 0x3680c24a,
    0xf46585e0, 0x9e5b3913, 0xb154e6c1, 0x3680a9cf, 0xf46585e0, 0x9e5b3913, 0xb154c872, 0x3680a273,
    0xf46585e0, 0x9e5b3913, 0xb1553ba4, 0x3680c24a, 0xf46585e0, 0x9e5b3913, 0xb154e6c1, 0x3680a9cf,
    0xf46585e0, 0x9e5b3913, 0xb154c872, 0x3680a273, 0xf46585e0, 0x9e5b3913, 0xb1553ba4, 0x3680c24a,
    0xf46585e0, 0x9e5b3913, 0xb154e6c1, 0x3680a9cf, 0xf46585e0, 0x9e5b3913, 0xb154c872, 0x3680a273,
    0x5a78ff33, 0x2a4fc649, 0x472cf22a, 0x16f498e6, 0x5a78ff33, 0x2a4fc649, 0x472cbf09, 0x16f489f5,
    0x5a78ff33, 0x2a4fc649, 0x472c9ca4, 0x16f4816c, 0x5a78ff33, 0x2a4fc649, 0x472cf22a, 0x16f498e6,
    0x5a78ff33, 0x2a4fc649, 0x472cbf09, 0x16f489f5, 0x5a78ff33, 0x2a4fc649, 0x472c9ca4, 0x16f4816c,
    0x5a78ff33, 0x2a4fc649, 0x472cf22a, 0x16f498e6, 0x5a78ff33, 0x2a4fc649, 0x472cbf09, 0x16f489f5,
    0x5a78ff33, 0x2a4fc649, 0x472c9ca4, 0x16f4816c, 0x5a78ff33, 0x2a4fc649, 0x472cf22a, 0x16f498e6,
    0x5a78ff33, 0x2a4fc649, 0x472cbf09, 0x16f489f5, 0x5a78ff33, 0x2a4fc649, 0x472c9ca4, 0x16f4816c,
    0x5a78ff33, 0x2a4fc649, 0x472cf22a, 0x16f498e6, 0x5a78ff33, 0x2a4fc649, 0x472cbf09, 0x16f489f5,
    0x5a78ff33, 0x2a4fc649, 0x472c9ca4, 0x16f4816c, 0x5a78ff33, 0x2a4fc649, 0x472cf22a, 0x16f498e6,
    0x5a78ff33, 0x2a4fc649, 0x472cbf09, 0x16f489f5, 0x5a78ff33, 0x2a4fc649, 0x472c9ca4, 0x16f4816c,
    0x5a78ff33, 0x2a4fc649, 0x472cf22a, 0x16f498e6, 0x5a78ff33, 0x2a4fc649, 0x472cbf09, 0x16f489f5,
    0x5a78ff33, 0x2a4fc649, 0x472c9ca4, 0x16f4816c, 0x5a78ff33, 0x2a4fc649, 0x472cf22a, 0x16f498e6,
    0x5a78ff33, 0x2a4fc649, 0x472cbf09, 0x16f489f5, 0x5a78ff33, 0x2a4fc649, 0x472c9ca4, 0x16f4816c,
    0x5a78ff33, 0x2a4fc649, 0x472cf22a, 0x16f498e6, 0x5a78ff33, 0x2a4fc649, 0x472cbf09, 0x16f489f5,
    0x5a78ff33, 0x2a4fc649, 0x472c9ca4, 0x16f4816c, 0x23faa0b6, 0x0b2d9f6c, 0x50c194dc, 0x161dac68,
    0x23faa0b6, 0x0b2d9f6c, 0x50c153e9, 0x161d9b81, 0x23faa0b6, 0x0b2d9f6c, 0x50c12d3d, 0x161d91ff,
    0x23faa0b6, 0x0b2d9f6c, 0x50c194dc, 0x161dac68, 0x23faa0b6, 0x0b2d9f6c, 0x50c153e9, 0x161d9b81,
    0x23faa0b6, 0x0b2d9f6c, 0x50c12d3d, 0x161d91ff, 0x23faa0b6, 0x0b2d9f6c, 0x50c194dc, 0x161dac68,
    0x23faa0b6, 0x0b2d9f6c, 0x50c153e9, 0x161d9b81, 0x23faa0b6, 0x0b2d9f6c, 0x50c12d3d, 0x161d91ff,
    0x23faa0b6, 0x0b2d9f6c, 0x50c194dc, 0x161dac68, 0x23faa0b6, 0x0b2d9f6c, 0x50c153e9, 0x161d9b81,
    0x23faa0b6, 0x0b2d9f6c, 0x50c12d3d, 0x161d91ff, 0x23faa0b6, 0x0b2d9f6c, 0x50c194dc, 0x161dac68,
    0x23faa0b6, 0x0b2d9f6c, 0x50c153e9, 0x161d9b81, 0x23faa0b6, 0x0b2d9f6c, 0x50c12d3d, 0x161d91ff,
    0x23faa0b6, 0x0b2d9f6c, 0x50c194dc, 0x161dac68, 0x23faa0b6, 0x0b2d9f6c, 0x50c153e9, 0x161d9b81,
    0x23faa0b6, 0x0b2d9f6c, 0x50c12d3d, 0x161d91ff, 0x23faa0b6, 0x0b2d9f6c, 0x50c194dc, 0x161dac68,
    0x23faa0b6, 0x0b2d9f6c, 0x50c153e9, 0x161d9b81, 0x23faa0b6, 0x0b2d9f6c, 0x50c12d3d, 0x161d91ff,
    0x23faa0b6, 0x0b2d9f6c, 0x50c194dc, 0x161dac68, 0x23faa0b6, 0x0b2d9f6c, 0x50c153e9, 0x161d9b81,
    0x23faa0b6, 0x0b2d9f6c, 0x50c12d3d, 0x161d91ff, 0x23faa0b6, 0x0b2d9f6c, 0x50c194dc, 0x161dac68,
    0x23faa0b6, 0x0b2d9f6c, 0x50c153e9, 0x161d9b81, 0x23faa0b6, 0x0b2d9f6c, 0x50c12d3d, 0x161d91ff
};

static uint32_t get_checksum(uint16_t median, uint16_t motion, uint16_t vert,
    uint16_t horiz, uint16_t iir, uint16_t lk, uint16_t sof)
{
    uint16_t a, b, c, d, e, f, g;
    a = median;
    b = motion / 15U;
    c = (vert - 28U) / 14U;
    d = (horiz - 85U) / 40U;
    e = (iir - 1U) / 127U;
    f = lk;
    g = sof;
    return dof_checksums_ref[(3U * 3U * 3U * 3U * 2U * 2U * a) + (3U * 3U * 3U * 2U * 2U * b) + (3U * 3U * 2U * 2U * c) +
        (3U * 2U * 2U * d) + (2U*2U*e) + (2U*f) + g];
}

#define ADD_MEDIAN_FILTER(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/median=OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/median=ON", __VA_ARGS__, 1))

#define ADD_MOTION_SMOOTHNESS_FACTOR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/motion_smooth=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/motion_smooth=16", __VA_ARGS__, 16)), \
    CT_EXPAND(nextmacro(testArgName "/motion_smooth=31", __VA_ARGS__, 31))

#define ADD_VERTICAL_SEARCH_RANGE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/vert_search_range=28", __VA_ARGS__, 28)), \
    CT_EXPAND(nextmacro(testArgName "/vert_search_range=42", __VA_ARGS__, 42)), \
    CT_EXPAND(nextmacro(testArgName "/vert_search_range=56", __VA_ARGS__, 56))

#define ADD_HORIZONTAL_SEARCH_RANGE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/horz_search_range=85", __VA_ARGS__, 85)), \
    CT_EXPAND(nextmacro(testArgName "/horz_search_range=130", __VA_ARGS__, 130)), \
    CT_EXPAND(nextmacro(testArgName "/horz_search_range=170", __VA_ARGS__, 170))

#define ADD_IIR_FILTER_ALPHA(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/iir=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/iir=128", __VA_ARGS__, 128)), \
    CT_EXPAND(nextmacro(testArgName "/iir=255", __VA_ARGS__, 255))

#define ADD_ENABLE_LK(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/output=U16", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/output=U32", __VA_ARGS__, 1))

#define ADD_ENABLE_SOF(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sof=OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/sof=ON", __VA_ARGS__, 1))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("dof_real_input", ADD_MEDIAN_FILTER, ADD_MOTION_SMOOTHNESS_FACTOR, ADD_VERTICAL_SEARCH_RANGE, ADD_HORIZONTAL_SEARCH_RANGE, ADD_IIR_FILTER_ALPHA, ADD_ENABLE_LK, ADD_ENABLE_SOF, ARG)


TEST_WITH_ARG(tivxHwaDmpacDof, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_pyramid input_current = NULL, input_reference = NULL;
    vx_image flow_vector_in = NULL, flow_vector_out = NULL;
    vx_image flow_vector_out_img = NULL, confidence_img = NULL;
    vx_image sof_mask = NULL;
    vx_distribution confidence_histogram = NULL;
    tivx_dmpac_dof_params_t params;
    vx_user_data_object param_obj;
    vx_user_data_object cs_obj;
    vx_user_data_object sof_config_obj = NULL;
    vx_graph graph = 0;
    vx_node node_dof = 0;
    vx_node node_dof_vis = 0;
    vx_status status;
    vx_rectangle_t rect;
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    char output_file[256];
    vx_reference ref[1];
    vx_enum flowVectorType = VX_DF_IMAGE_U32;

    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_DOF));

    {
        uint32_t width = 256, height = 128;
        uint32_t flow_width = width;
        uint32_t flow_height = height;
        uint32_t levels = 2, i;
        vx_enum format = VX_DF_IMAGE_U8;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        tivx_dmpac_dof_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_dmpac_dof_params_t", sizeof(tivx_dmpac_dof_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.vertical_search_range[0] = arg_->vertical_range;
        params.vertical_search_range[1] = arg_->vertical_range;
        params.horizontal_search_range = arg_->horizontal_range;
        params.median_filter_enable = arg_->median_filter;
        params.motion_smoothness_factor = arg_->motion_smoothness;
        params.motion_direction = 1; /* 1: forward direction */
        params.iir_filter_alpha = arg_->iir_filter;

        /* Turn off temporal predictor in this test ... will test in separate test */
        params.base_predictor[0] = TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED;

        if(arg_->enable_lk == 0)
        {
            flowVectorType = VX_DF_IMAGE_U16;
        }
        if(arg_->enable_sof == 1)
        {
            tivx_dmpac_dof_sof_params_t sof_params;
            ASSERT_VX_OBJECT(sof_mask = vxCreateImage(context, width/8, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            initialize_sof_mask(sof_mask, width/8, height, &flow_width, &flow_height, 0);
            sof_params.sof_max_pix_in_row = flow_width;
            sof_params.sof_fv_height = flow_height;
            ASSERT_VX_OBJECT(sof_config_obj = vxCreateUserDataObject(context, "tivx_dmpac_dof_sof_params_t", sizeof(tivx_dmpac_dof_sof_params_t), &sof_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        }

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_dmpac_dof_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(input_current = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(input_reference = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(flow_vector_out = vxCreateImage(context, flow_width, flow_height, flowVectorType), VX_TYPE_IMAGE);

        if(arg_->enable_lk == 1)
        {
            ASSERT_VX_OBJECT(confidence_histogram = vxCreateDistribution(context, 16, 0, 16), VX_TYPE_DISTRIBUTION);
            ASSERT_VX_OBJECT(flow_vector_out_img = vxCreateImage(context, flow_width, flow_height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(confidence_img = vxCreateImage(context, flow_width, flow_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node_dof = tivxDmpacDofNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        input_current,
                        input_reference,
                        NULL,
                        sof_config_obj,
                        sof_mask,
                        flow_vector_out,
                        confidence_histogram), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_dof, VX_TARGET_STRING, TIVX_TARGET_DMPAC_DOF));

        if(arg_->enable_lk == 1)
        {
            ASSERT_VX_OBJECT(node_dof_vis = tivxDofVisualizeNode(graph,
                            flow_vector_out,
                            NULL,
                            flow_vector_out_img,
                            confidence_img), VX_TYPE_NODE);
            VX_CALL(vxSetNodeTarget(node_dof_vis, VX_TARGET_STRING, TIVX_TARGET_DSP1));
        }

        VX_CALL(vxVerifyGraph(graph));

        /* Configure confidence score tree params */
        {
            tivx_dmpac_dof_cs_tree_params_t cs_tree_params;
            tivx_dmpac_dof_cs_tree_params_init(&cs_tree_params);

            ASSERT_VX_OBJECT(cs_obj = vxCreateUserDataObject(context, "tivx_dmpac_dof_cs_tree_params_t",
                                                sizeof(tivx_dmpac_dof_cs_tree_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(cs_obj, 0, sizeof(tivx_dmpac_dof_cs_tree_params_t), &cs_tree_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

            ref[0] = (vx_reference) cs_obj;
            VX_CALL(tivxNodeSendCommand(node_dof, 0, TIVX_DMPAC_DOF_CMD_CS_PARAMS, ref, 1));
            VX_CALL(vxReleaseUserDataObject(&cs_obj));
        }

        for(i=0; i<levels; i++)
        {
            status = load_image_into_pyramid_level(input_current, i, "tivx/dof/tivx_test_ofTestCase1_10_pl");
            ASSERT(status==VX_SUCCESS);
            status = load_image_into_pyramid_level(input_reference, i, "tivx/dof/tivx_test_ofTestCase1_11_pl");
            ASSERT(status==VX_SUCCESS);
        }
        VX_CALL(vxProcessGraph(graph));

#if 0 /* Disabling file output writes */
        if(arg_->enable_lk == 1)
        {
            sprintf(output_file, "output/tivx_test_ofTestCase1_%d_%d", arg_->median_filter, arg_->motion_smoothness);
            status = save_image_from_dof(flow_vector_out_img, confidence_img, output_file);
            ASSERT(status==VX_SUCCESS);
        }
#endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = flow_width;
        rect.end_y = flow_height;

        checksum_expected = get_checksum(arg_->median_filter, arg_->motion_smoothness, arg_->vertical_range,
            arg_->horizontal_range, arg_->iir_filter, arg_->enable_lk, arg_->enable_sof);
        checksum_actual = tivx_utils_simple_image_checksum(flow_vector_out, 0, rect);
        ASSERT(checksum_expected == checksum_actual);

        VX_CALL(vxReleaseNode(&node_dof));

        if(arg_->enable_lk == 1)
        {
            VX_CALL(vxReleaseDistribution(&confidence_histogram));
            VX_CALL(vxReleaseNode(&node_dof_vis));
            VX_CALL(vxReleaseImage(&flow_vector_out_img));
            VX_CALL(vxReleaseImage(&confidence_img));
        }
        if(arg_->enable_sof == 1)
        {
            VX_CALL(vxReleaseUserDataObject(&sof_config_obj));
            VX_CALL(vxReleaseImage(&sof_mask));
        }
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleasePyramid(&input_current));
        VX_CALL(vxReleasePyramid(&input_reference));
        VX_CALL(vxReleaseImage(&flow_vector_out));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node_dof == 0);
        ASSERT(node_dof_vis == 0);
        ASSERT(graph == 0);
        ASSERT(input_current == 0);
        ASSERT(input_reference == 0);
        ASSERT(flow_vector_in == 0);
        ASSERT(flow_vector_out == 0);
        ASSERT(sof_mask == 0);
        ASSERT(sof_config_obj == 0);
        ASSERT(confidence_histogram == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

typedef struct {
    const char* testName;
    int inter_predictor2;
    int inter_predictor1;
    int base_predictor2;
    int base_predictor1;
} ArgPredictors;

static uint32_t dof_predictor_checksums_ref[] = {
    0x1af9d081, 0x7808c689, 0x1af9d081, 0x823387e2, 0x4b189064, 0x7808c689, 0x7808c689, 0x7808c689,
    0x5fc1c786, 0x272b0914, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x823387e2, 0x4b189064, 0x823387e2,
    0x5fc1c786, 0x823387e2, 0x823387e2, 0x3c9f6976, 0x4b189064, 0x272b0914, 0x4b189064, 0x3c9f6976,
    0x4b189064, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x79e08274, 0x448cb9b8, 0x7808c689, 0x7808c689,
    0x7808c689, 0x6231bff6, 0x336b28eb, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x79e08274, 0x448cb9b8,
    0x79e08274, 0x6231bff6, 0x79e08274, 0x79e08274, 0x3b6c67b5, 0x448cb9b8, 0x336b28eb, 0x448cb9b8,
    0x3b6c67b5, 0x448cb9b8, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x888a8e69, 0x53a521a1, 0x7808c689,
    0x7808c689, 0x7808c689, 0x6f0abc42, 0x3faa30c3, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x888a8e69,
    0x53a521a1, 0x888a8e69, 0x6f0abc42, 0x888a8e69, 0x888a8e69, 0x48a5c889, 0x53a521a1, 0x3faa30c3,
    0x53a521a1, 0x48a5c889, 0x53a521a1, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x918adc12, 0x4aebba3c,
    0x7808c689, 0x7808c689, 0x7808c689, 0x6a8c32a5, 0x43710a30, 0x1af9d081, 0x7808c689, 0x1af9d081,
    0x918adc12, 0x4aebba3c, 0x918adc12, 0x6a8c32a5, 0x918adc12, 0x918adc12, 0x44e6e85f, 0x4aebba3c,
    0x43710a30, 0x4aebba3c, 0x44e6e85f, 0x4aebba3c, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x79e08274,
    0x448cb9b8, 0x7808c689, 0x7808c689, 0x7808c689, 0x6231bff6, 0x336b28eb, 0x1af9d081, 0x7808c689,
    0x1af9d081, 0x79e08274, 0x448cb9b8, 0x79e08274, 0x6231bff6, 0x79e08274, 0x79e08274, 0x3b6c67b5,
    0x448cb9b8, 0x336b28eb, 0x448cb9b8, 0x3b6c67b5, 0x448cb9b8, 0x1af9d081, 0x7808c689, 0x1af9d081,
    0x79e08274, 0x448cb9b8, 0x7808c689, 0x7808c689, 0x7808c689, 0x6231bff6, 0x336b28eb, 0x1af9d081,
    0x7808c689, 0x1af9d081, 0x79e08274, 0x448cb9b8, 0x79e08274, 0x6231bff6, 0x79e08274, 0x79e08274,
    0x3b6c67b5, 0x448cb9b8, 0x336b28eb, 0x448cb9b8, 0x3b6c67b5, 0x448cb9b8, 0x1af9d081, 0x7808c689,
    0x1af9d081, 0x83115f15, 0x57d81054, 0x7808c689, 0x7808c689, 0x7808c689, 0x62a2a24d, 0x464a4b5e,
    0x1af9d081, 0x7808c689, 0x1af9d081, 0x83115f15, 0x57d81054, 0x83115f15, 0x62a2a24d, 0x83115f15,
    0x83115f15, 0x4861abd9, 0x57d81054, 0x464a4b5e, 0x57d81054, 0x4861abd9, 0x57d81054, 0x1af9d081,
    0x7808c689, 0x1af9d081, 0x8037b14d, 0x56e1b154, 0x7808c689, 0x7808c689, 0x7808c689, 0x66ec130a,
    0x4adc1203, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x8037b14d, 0x56e1b154, 0x8037b14d, 0x66ec130a,
    0x8037b14d, 0x8037b14d, 0x527bddb3, 0x56e1b154, 0x4adc1203, 0x56e1b154, 0x527bddb3, 0x56e1b154,
    0x1af9d081, 0x7808c689, 0x1af9d081, 0x888a8e69, 0x53a521a1, 0x7808c689, 0x7808c689, 0x7808c689,
    0x6f0abc42, 0x3faa30c3, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x888a8e69, 0x53a521a1, 0x888a8e69,
    0x6f0abc42, 0x888a8e69, 0x888a8e69, 0x48a5c889, 0x53a521a1, 0x3faa30c3, 0x53a521a1, 0x48a5c889,
    0x53a521a1, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x83115f15, 0x57d81054, 0x7808c689, 0x7808c689,
    0x7808c689, 0x62a2a24d, 0x464a4b5e, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x83115f15, 0x57d81054,
    0x83115f15, 0x62a2a24d, 0x83115f15, 0x83115f15, 0x4861abd9, 0x57d81054, 0x464a4b5e, 0x57d81054,
    0x4861abd9, 0x57d81054, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x888a8e69, 0x53a521a1, 0x7808c689,
    0x7808c689, 0x7808c689, 0x6f0abc42, 0x3faa30c3, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x888a8e69,
    0x53a521a1, 0x888a8e69, 0x6f0abc42, 0x888a8e69, 0x888a8e69, 0x48a5c889, 0x53a521a1, 0x3faa30c3,
    0x53a521a1, 0x48a5c889, 0x53a521a1, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x8931d6ff, 0x4f323c68,
    0x7808c689, 0x7808c689, 0x7808c689, 0x605128e5, 0x46677192, 0x1af9d081, 0x7808c689, 0x1af9d081,
    0x8931d6ff, 0x4f323c68, 0x8931d6ff, 0x605128e5, 0x8931d6ff, 0x8931d6ff, 0x49d16e9a, 0x4f323c68,
    0x46677192, 0x4f323c68, 0x49d16e9a, 0x4f323c68, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x918adc12,
    0x4aebba3c, 0x7808c689, 0x7808c689, 0x7808c689, 0x6a8c32a5, 0x43710a30, 0x1af9d081, 0x7808c689,
    0x1af9d081, 0x918adc12, 0x4aebba3c, 0x918adc12, 0x6a8c32a5, 0x918adc12, 0x918adc12, 0x44e6e85f,
    0x4aebba3c, 0x43710a30, 0x4aebba3c, 0x44e6e85f, 0x4aebba3c, 0x1af9d081, 0x7808c689, 0x1af9d081,
    0x8037b14d, 0x56e1b154, 0x7808c689, 0x7808c689, 0x7808c689, 0x66ec130a, 0x4adc1203, 0x1af9d081,
    0x7808c689, 0x1af9d081, 0x8037b14d, 0x56e1b154, 0x8037b14d, 0x66ec130a, 0x8037b14d, 0x8037b14d,
    0x527bddb3, 0x56e1b154, 0x4adc1203, 0x56e1b154, 0x527bddb3, 0x56e1b154, 0x1af9d081, 0x7808c689,
    0x1af9d081, 0x8931d6ff, 0x4f323c68, 0x7808c689, 0x7808c689, 0x7808c689, 0x605128e5, 0x46677192,
    0x1af9d081, 0x7808c689, 0x1af9d081, 0x8931d6ff, 0x4f323c68, 0x8931d6ff, 0x605128e5, 0x8931d6ff,
    0x8931d6ff, 0x49d16e9a, 0x4f323c68, 0x46677192, 0x4f323c68, 0x49d16e9a, 0x4f323c68, 0x1af9d081,
    0x7808c689, 0x1af9d081, 0x918adc12, 0x4aebba3c, 0x7808c689, 0x7808c689, 0x7808c689, 0x6a8c32a5,
    0x43710a30, 0x1af9d081, 0x7808c689, 0x1af9d081, 0x918adc12, 0x4aebba3c, 0x918adc12, 0x6a8c32a5,
    0x918adc12, 0x918adc12, 0x44e6e85f, 0x4aebba3c, 0x43710a30, 0x4aebba3c, 0x44e6e85f, 0x4aebba3c
};

static uint32_t get_predictor_checksum(uint16_t base_pred1, uint16_t base_pred2,
    uint16_t inter_pred1, uint16_t inter_pred2)
{
    uint16_t a, b, c, d;
    a = (inter_pred2 > 1) ? inter_pred2-1 : inter_pred2;
    b = (inter_pred1 > 1) ? inter_pred1-1 : inter_pred1;
    c = base_pred2;
    d = base_pred1;

    return dof_predictor_checksums_ref[(4U * 5U * 5U * a) + (5U * 5U * b) + (5U*c) + d];
}

static uint32_t dof_predictor2_checksums_ref[] = {
    0x109b3df7, 0x5a5ded98, 0x53f00bc2, 0x433ba56d,
    0x109b3df7, 0x5a5ded98, 0x56fedd73, 0x2b223be4,
    0x109b3df7, 0x5a5ded98, 0x5c07edac, 0x460846aa,
    0x109b3df7, 0x5a5ded98, 0x5407526d, 0x4232d38c,
    0x109b3df7, 0x5a5ded98, 0x5b869bef, 0x40c16bb2,
    0x109b3df7, 0x5a5ded98, 0x4f524859, 0x435c74ea,
    0x109b3df7, 0x5a5ded98, 0x591404bb, 0x3bf60922
};

static uint32_t get_predictor2_checksum(uint16_t base_pred1, uint16_t base_pred2,
    uint16_t inter_pred1, uint16_t inter_pred2)
{
    uint32_t index = 0, inter = 0;

    if(      (base_pred1 == TIVX_DMPAC_DOF_PREDICTOR_NONE) ||
             (base_pred2 == TIVX_DMPAC_DOF_PREDICTOR_NONE) ||
            ((base_pred1 == TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL) &&
             (base_pred2 == TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL)))

    {
        index = 0;
    }
    else if( (base_pred1 == TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT) ||
             (base_pred2 == TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT))
    {
        index = 1;
    }
    else if( (base_pred1 == TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT) ||
             (base_pred2 == TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT))
    {
        index = 2;
    }
    else if( (base_pred1 == TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED) ||
             (base_pred2 == TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED))
    {
        index = 3;
    }

    if(     ((inter_pred1 == TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT) ||
             (inter_pred2 == TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT)) &&
            ((inter_pred1 == TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT) ||
             (inter_pred2 == TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT)))
    {
        inter = 4;
    }
    else if(((inter_pred1 == TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED) ||
             (inter_pred2 == TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED)) &&
            ((inter_pred1 == TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT) ||
             (inter_pred2 == TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT)))
    {
        inter = 5;
    }
    else if(((inter_pred1 == TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED) ||
             (inter_pred2 == TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED)) &&
            ((inter_pred1 == TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT) ||
             (inter_pred2 == TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT)))
    {
        inter = 6;
    }
    else if( (inter_pred1 == TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT) ||
             (inter_pred2 == TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT))
    {
        inter = 1;
    }
    else if( (inter_pred1 == TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT) ||
             (inter_pred2 == TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT))
    {
        inter = 2;
    }
    else if( (inter_pred1 == TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED) ||
             (inter_pred2 == TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED))
    {
        inter = 3;
    }

    return dof_predictor2_checksums_ref[inter*4 + index];
}

#define ADD_BASE_PRED1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/base_pred1=NONE", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_NONE)),     \
    CT_EXPAND(nextmacro(testArgName "/base_pred1=DELAY_LEFT", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT)), \
    CT_EXPAND(nextmacro(testArgName "/base_pred1=TEMPORAL", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL)),   \
    CT_EXPAND(nextmacro(testArgName "/base_pred1=PYR_LEFT", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT)),   \
    CT_EXPAND(nextmacro(testArgName "/base_pred1=PYR_COL", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED))

#define ADD_BASE_PRED2(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/base_pred2=NONE", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_NONE)),     \
    CT_EXPAND(nextmacro(testArgName "/base_pred2=DELAY_LEFT", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT)), \
    CT_EXPAND(nextmacro(testArgName "/base_pred2=TEMPORAL", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL)),   \
    CT_EXPAND(nextmacro(testArgName "/base_pred2=PYR_LEFT", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT)),   \
    CT_EXPAND(nextmacro(testArgName "/base_pred2=PYR_COL", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED))

#define ADD_INTER_PRED1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/inter_pred1=NONE", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_NONE)),     \
    CT_EXPAND(nextmacro(testArgName "/inter_pred1=DELAY_LEFT", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT)), \
    CT_EXPAND(nextmacro(testArgName "/inter_pred1=PYR_LEFT", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT)),   \
    CT_EXPAND(nextmacro(testArgName "/inter_pred1=PYR_COL", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED))

#define ADD_INTER_PRED2(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/inter_pred2=NONE", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_NONE)),     \
    CT_EXPAND(nextmacro(testArgName "/inter_pred2=DELAY_LEFT", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT)), \
    CT_EXPAND(nextmacro(testArgName "/inter_pred2=PYR_LEFT", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_PYR_LEFT)),   \
    CT_EXPAND(nextmacro(testArgName "/inter_pred2=PYR_COL", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED))

#define PREDICTOR_PARAMETERS \
    CT_GENERATE_PARAMETERS("dof_real_input", ADD_INTER_PRED2, ADD_INTER_PRED1, ADD_BASE_PRED2, ADD_BASE_PRED1, ARG)

TEST_WITH_ARG(tivxHwaDmpacDof, testPredictors, ArgPredictors,
    PREDICTOR_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_pyramid input_current = NULL, input_reference = NULL;
    vx_image flow_vector_in = NULL, flow_vector_out = NULL;
    vx_image flow_vector_out_img = NULL, confidence_img = NULL;
    vx_image sof_mask = NULL;
    vx_distribution confidence_histogram = NULL;
    tivx_dmpac_dof_params_t params;
    vx_user_data_object param_obj;
    vx_user_data_object cs_obj;
    vx_graph graph = 0;
    vx_node node_dof = 0;
    vx_node node_dof_vis = 0;
    vx_status status;
    vx_rectangle_t rect;
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    char output_file[256];
    vx_reference ref[1];
    vx_enum flowVectorType = VX_DF_IMAGE_U32;

    ASSERT((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_DOF)));

    {
        uint32_t width = 256, height = 128;
        uint32_t flow_width = width;
        uint32_t flow_height = height;
        uint32_t levels = 3, i;
        uint32_t temporal_pred_flag = 0;
        vx_enum format = VX_DF_IMAGE_U8;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        tivx_dmpac_dof_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_dmpac_dof_params_t", sizeof(tivx_dmpac_dof_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.base_predictor[0] = arg_->base_predictor1;
        params.base_predictor[1] = arg_->base_predictor2;
        params.inter_predictor[0] = arg_->inter_predictor1;
        params.inter_predictor[1] = arg_->inter_predictor2;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_dmpac_dof_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(input_current = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(input_reference = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);

        if((params.base_predictor[0] == TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL) ||
           (params.base_predictor[1] == TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL))
        {
            temporal_pred_flag = 1;
            ASSERT_VX_OBJECT(flow_vector_in = vxCreateImage(context, flow_width, flow_height, flowVectorType), VX_TYPE_IMAGE);
        }
        ASSERT_VX_OBJECT(flow_vector_out = vxCreateImage(context, flow_width, flow_height, flowVectorType), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(confidence_histogram = vxCreateDistribution(context, 16, 0, 16), VX_TYPE_DISTRIBUTION);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node_dof = tivxDmpacDofNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        input_current,
                        input_reference,
                        flow_vector_in,
                        NULL,
                        NULL,
                        flow_vector_out,
                        confidence_histogram), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_dof, VX_TARGET_STRING, TIVX_TARGET_DMPAC_DOF));

        VX_CALL(vxVerifyGraph(graph));

        /* Configure confidence score tree params */
        {
            tivx_dmpac_dof_cs_tree_params_t cs_tree_params;
            tivx_dmpac_dof_cs_tree_params_init(&cs_tree_params);

            ASSERT_VX_OBJECT(cs_obj = vxCreateUserDataObject(context, "tivx_dmpac_dof_cs_tree_params_t",
                                                sizeof(tivx_dmpac_dof_cs_tree_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(cs_obj, 0, sizeof(tivx_dmpac_dof_cs_tree_params_t), &cs_tree_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

            ref[0] = (vx_reference) cs_obj;
            VX_CALL(tivxNodeSendCommand(node_dof, 0, TIVX_DMPAC_DOF_CMD_CS_PARAMS, ref, 1));
            VX_CALL(vxReleaseUserDataObject(&cs_obj));
        }

        for(i=0; i<levels; i++)
        {
            status = load_image_into_pyramid_level(input_current, i, "tivx/dof/tivx_test_ofTestCase1_10_pl");
            ASSERT(status==VX_SUCCESS);
            status = load_image_into_pyramid_level(input_reference, i, "tivx/dof/tivx_test_ofTestCase1_11_pl");
            ASSERT(status==VX_SUCCESS);
        }
        VX_CALL(vxProcessGraph(graph));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = flow_width;
        rect.end_y = flow_height;

        checksum_expected = get_predictor_checksum(arg_->base_predictor1, arg_->base_predictor2,
            arg_->inter_predictor1, arg_->inter_predictor2);
        checksum_actual = tivx_utils_simple_image_checksum(flow_vector_out, 0, rect);
        ASSERT(checksum_expected == checksum_actual);

        if(temporal_pred_flag == 1)
        {
            copy_flow_image(flow_vector_out, flow_width, flow_height, flow_vector_in);
            VX_CALL(vxProcessGraph(graph));
            checksum_expected = get_predictor2_checksum(arg_->base_predictor1, arg_->base_predictor2,
                arg_->inter_predictor1, arg_->inter_predictor2);
            checksum_actual = tivx_utils_simple_image_checksum(flow_vector_out, 0, rect);
            ASSERT(checksum_expected == checksum_actual);
            VX_CALL(vxReleaseImage(&flow_vector_in));
        }

        VX_CALL(vxReleaseNode(&node_dof));

        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleasePyramid(&input_current));
        VX_CALL(vxReleasePyramid(&input_reference));
        VX_CALL(vxReleaseImage(&flow_vector_out));
        VX_CALL(vxReleaseDistribution(&confidence_histogram));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node_dof == 0);
        ASSERT(node_dof_vis == 0);
        ASSERT(graph == 0);
        ASSERT(input_current == 0);
        ASSERT(input_reference == 0);
        ASSERT(flow_vector_in == 0);
        ASSERT(flow_vector_out == 0);
        ASSERT(sof_mask == 0);
        ASSERT(confidence_histogram == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}


typedef struct {
    const char* testName;
    int negative_test;
    int condition;
} ArgNegative;

#define ADD_NEGATIVE_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/negative_test=vertical-search_range_0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=vertical-search_range_1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=horizontal_search_range", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=max_horizontal_with_vertical-search_range", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=max_vertical_with_horizontal-search_range", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=median_filter_enable", __VA_ARGS__, 5)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=motion_smoothness_factor", __VA_ARGS__, 6)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=motion_direction", __VA_ARGS__, 7)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=iir_filter_alpha", __VA_ARGS__, 8)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=output_format", __VA_ARGS__, 9)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=pyramid_divisibility", __VA_ARGS__, 10))


#define ADD_NEGATIVE_CONDITION(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_positive", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_positive", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_negative", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_negative", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/condition=middle_negative", __VA_ARGS__, 4))

#define PARAMETERS_NEGATIVE \
    CT_GENERATE_PARAMETERS("testNegative", ADD_NEGATIVE_TEST, ADD_NEGATIVE_CONDITION, ARG)


TEST_WITH_ARG(tivxHwaDmpacDof, testNegativeGraph, ArgNegative,
    PARAMETERS_NEGATIVE)
{
    vx_context context = context_->vx_context_;
    vx_pyramid input_current = NULL, input_reference = NULL;
    vx_image flow_vector_in = NULL, flow_vector_out = NULL;
    vx_distribution confidence_histogram = NULL;
    tivx_dmpac_dof_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node_dof = 0;
    vx_status status;
    vx_enum flowVectorType = VX_DF_IMAGE_U32;

    ASSERT((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_DOF)));

    {
        uint32_t width = 256, height = 128;
        uint32_t levels = 5;
        vx_enum format = VX_DF_IMAGE_U8;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        tivx_dmpac_dof_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_dmpac_dof_params_t", sizeof(tivx_dmpac_dof_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.vertical_search_range[0] = 48;
        params.vertical_search_range[1] = 48;
        params.horizontal_search_range = 191;
        params.median_filter_enable = 1;
        params.motion_smoothness_factor = 24;
        params.motion_direction = 1; /* 1: forward direction */

        switch (arg_->negative_test)
        {
            case 0:
            {
                if (0U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 62;
                }
                else if (2U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 63;
                }
                else if (3U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 63;
                }
                else
                {
                    params.vertical_search_range[0U] = 63;
                }
                break;
            }
            case 1:
            {
                if (0U == arg_->condition)
                {
                    params.vertical_search_range[1U] = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.vertical_search_range[1U] = 62;
                }
                else if (2U == arg_->condition)
                {
                    params.vertical_search_range[1U] = 63;
                }
                else if (3U == arg_->condition)
                {
                    params.vertical_search_range[1U] = 63;
                }
                else
                {
                    params.vertical_search_range[1U] = 63;
                }
                break;
            }
            case 2:
            {
                if (0U == arg_->condition)
                {
                    params.horizontal_search_range = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.horizontal_search_range = 191;
                }
                else if (2U == arg_->condition)
                {
                    params.horizontal_search_range = 192;
                }
                else if (3U == arg_->condition)
                {
                    params.horizontal_search_range = 192;
                }
                else
                {
                    params.horizontal_search_range = 192;
                }
                break;
            }
            case 3:
            {
                params.horizontal_search_range = 191;
                if (0U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 0;
                    params.vertical_search_range[1U] = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 56;
                    params.vertical_search_range[1U] = 56;
                }
                else if (2U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 57;
                    params.vertical_search_range[1U] = 57;
                }
                else if (3U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 57;
                    params.vertical_search_range[1U] = 57;
                }
                else
                {
                    params.vertical_search_range[0U] = 57;
                    params.vertical_search_range[1U] = 57;
                }
                break;
            }
            case 4:
            {
                params.vertical_search_range[0U] = 62;
                params.vertical_search_range[1U] = 62;
                if (0U == arg_->condition)
                {
                    params.horizontal_search_range = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.horizontal_search_range = 170;
                }
                else if (2U == arg_->condition)
                {
                    params.horizontal_search_range = 171;
                }
                else if (3U == arg_->condition)
                {
                    params.horizontal_search_range = 171;
                }
                else
                {
                    params.horizontal_search_range = 171;
                }
                break;
            }
            case 5:
            {
                if (0U == arg_->condition)
                {
                    params.median_filter_enable = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.median_filter_enable = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.median_filter_enable = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.median_filter_enable = 2;
                }
                else
                {
                    params.median_filter_enable = 2;
                }
                break;
            }
            case 6:
            {
                if (0U == arg_->condition)
                {
                    params.motion_smoothness_factor = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.motion_smoothness_factor = 31;
                }
                else if (2U == arg_->condition)
                {
                    params.motion_smoothness_factor = 32;
                }
                else if (3U == arg_->condition)
                {
                    params.motion_smoothness_factor = 32;
                }
                else
                {
                    params.motion_smoothness_factor = 32;
                }
                break;
            }
            case 7:
            {
                if (0U == arg_->condition)
                {
                    params.motion_direction = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.motion_direction = 3;
                }
                else if (2U == arg_->condition)
                {
                    params.motion_direction = 4;
                }
                else if (3U == arg_->condition)
                {
                    params.motion_direction = 4;
                }
                else
                {
                    params.motion_direction = 4;
                }
                break;
            }
            case 8:
            {
                if (0U == arg_->condition)
                {
                    params.iir_filter_alpha = 1;
                }
                else if (1U == arg_->condition)
                {
                    params.iir_filter_alpha = 255;
                }
                else if (2U == arg_->condition)
                {
                    params.iir_filter_alpha = 0;
                }
                else if (3U == arg_->condition)
                {
                    params.iir_filter_alpha = 256;
                }
                else
                {
                    params.iir_filter_alpha = 256;
                }
                break;
            }
            case 9:
            {
                if (0U == arg_->condition)
                {
                    flowVectorType = VX_DF_IMAGE_U16;
                }
                else if (1U == arg_->condition)
                {
                    flowVectorType = VX_DF_IMAGE_U32;
                }
                else if (2U == arg_->condition)
                {
                    flowVectorType = VX_DF_IMAGE_NV12;
                }
                else if (3U == arg_->condition)
                {
                    flowVectorType = VX_DF_IMAGE_YUYV;
                }
                else
                {
                    flowVectorType = VX_DF_IMAGE_RGB;
                }
                break;
            }
            case 10:
            {
                if (0U == arg_->condition)
                {
                    height = 64;
                }
                else if (1U == arg_->condition)
                {
                    height = 128;
                }
                else if (2U == arg_->condition)
                {
                    height = 144;
                }
                else if (3U == arg_->condition)
                {
                    height = 144;
                }
                else
                {
                    height = 144;
                }
                break;
            }
        }

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_dmpac_dof_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(input_current = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(input_reference = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(flow_vector_in = vxCreateImage(context, width, height, flowVectorType), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(flow_vector_out = vxCreateImage(context, width, height, flowVectorType), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(confidence_histogram = vxCreateDistribution(context, 16, 0, 16), VX_TYPE_DISTRIBUTION);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node_dof = tivxDmpacDofNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        input_current,
                        input_reference,
                        flow_vector_in,
                        NULL,
                        NULL,
                        flow_vector_out,
                        confidence_histogram), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_dof, VX_TARGET_STRING, TIVX_TARGET_DMPAC_DOF));

        if(2 > arg_->condition)
        {
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        }
        else
        {
            ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
        }
        VX_CALL(vxReleaseNode(&node_dof));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleasePyramid(&input_current));
        VX_CALL(vxReleasePyramid(&input_reference));
        VX_CALL(vxReleaseImage(&flow_vector_in));
        VX_CALL(vxReleaseImage(&flow_vector_out));
        VX_CALL(vxReleaseDistribution(&confidence_histogram));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node_dof == 0);
        ASSERT(graph == 0);
        ASSERT(input_current == 0);
        ASSERT(input_reference == 0);
        ASSERT(flow_vector_in == 0);
        ASSERT(flow_vector_out == 0);
        ASSERT(confidence_histogram == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

typedef struct {
    const char* testName;
    int base_predictor1;
    int base_predictor2;
    int flow_vec_delay;
    int enable_pipeline;
    int vec_in_present;
} ArgConfig;

#define ADD_CONFIG_VEC_IN_ABSENT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/vec_in_present=0", __VA_ARGS__, 0))

#define ADD_CONFIG_VEC_IN_PRESENT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/vec_in_present=1", __VA_ARGS__, 1))

#define ADD_CONFIG_PIPELINE_OFF(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/enable_pipeline=0", __VA_ARGS__, 0))

#define ADD_CONFIG_PIPELINE_ON(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/enable_pipeline=1", __VA_ARGS__, 1))

#define ADD_CONFIG_BASE_PRED1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/base_pred1=TEMPORAL", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL))

#define ADD_CONFIG_BASE_PRED2(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/base_pred2=TEMPORAL", __VA_ARGS__, TIVX_DMPAC_DOF_PREDICTOR_TEMPORAL))

#define ADD_CONFIG_VEC_DELAY_0(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/flow_vec_delay=0", __VA_ARGS__, 0))

#define ADD_CONFIG_VEC_DELAY_1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/flow_vec_delay=0", __VA_ARGS__, 1))

#define ADD_CONFIG_VEC_DELAY_EXCEED_MAX(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/flow_vec_delay=exceed_max_value", __VA_ARGS__, (TIVX_DMPAC_DOF_MAX_FLOW_VECTOR_DELAY+1)))

/* The following negative tests are defined below, all with temporal predictors ON.
 * 1) Flow vector invalid delay value + pipeline OFF + input flow vector absent
 * 2) Flow vector non-zero delay value + pipeline OFF + input flow vector absent
 * 3) Flow vector invalid delay value + pipeline ON + input flow vector absent
 * 4) Flow vector zero delay value + pipeline ON + input flow vector absent
 * 5) Flow vector non-zero delay value + pipeline ON + input flow vector present
 */
#define CONFIG_PARAMETERS \
    CT_GENERATE_PARAMETERS("dof_config", ADD_CONFIG_BASE_PRED1, ADD_CONFIG_BASE_PRED2, \
                           ADD_CONFIG_VEC_DELAY_EXCEED_MAX, ADD_CONFIG_PIPELINE_OFF, ADD_CONFIG_VEC_IN_ABSENT, ARG), \
    CT_GENERATE_PARAMETERS("dof_config", ADD_CONFIG_BASE_PRED1, ADD_CONFIG_BASE_PRED2, \
                           ADD_CONFIG_VEC_DELAY_1, ADD_CONFIG_PIPELINE_OFF, ADD_CONFIG_VEC_IN_ABSENT, ARG), \
    CT_GENERATE_PARAMETERS("dof_config", ADD_CONFIG_BASE_PRED1, ADD_CONFIG_BASE_PRED2, \
                           ADD_CONFIG_VEC_DELAY_EXCEED_MAX, ADD_CONFIG_PIPELINE_ON, ADD_CONFIG_VEC_IN_ABSENT, ARG), \
    CT_GENERATE_PARAMETERS("dof_config", ADD_CONFIG_BASE_PRED1, ADD_CONFIG_BASE_PRED2, \
                           ADD_CONFIG_VEC_DELAY_0, ADD_CONFIG_PIPELINE_ON, ADD_CONFIG_VEC_IN_ABSENT, ARG), \
    CT_GENERATE_PARAMETERS("dof_config", ADD_CONFIG_BASE_PRED1, ADD_CONFIG_BASE_PRED2, \
                           ADD_CONFIG_VEC_DELAY_1, ADD_CONFIG_PIPELINE_ON, ADD_CONFIG_VEC_IN_PRESENT, ARG)


TEST_WITH_ARG(tivxHwaDmpacDof, testNegativeConfig, ArgConfig,
    CONFIG_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_pyramid input_current = NULL, input_reference = NULL;
    vx_image flow_vector_in = NULL, flow_vector_out = NULL;
    vx_image flow_vector_out_img = NULL, confidence_img = NULL;
    vx_image sof_mask = NULL;
    vx_distribution confidence_histogram = NULL;
    tivx_dmpac_dof_params_t params;
    vx_user_data_object param_obj;
    vx_user_data_object cs_obj;
    vx_graph graph = 0;
    vx_node node_dof = 0;
    vx_node node_dof_vis = 0;
    vx_status status;
    vx_rectangle_t rect;
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    char output_file[256];
    vx_reference ref[1];
    vx_enum flowVectorType = VX_DF_IMAGE_U32;

    ASSERT((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_DOF)));

    {
        uint32_t width = 256, height = 128;
        uint32_t flow_width = width;
        uint32_t flow_height = height;
        uint32_t levels = 3, i;
        vx_enum format = VX_DF_IMAGE_U8;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        tivx_dmpac_dof_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_dmpac_dof_params_t", sizeof(tivx_dmpac_dof_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.base_predictor[0] = arg_->base_predictor1;
        params.base_predictor[1] = arg_->base_predictor2;
        params.inter_predictor[0] = TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT;
        params.inter_predictor[1] = TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT;
        params.flow_vector_internal_delay_num = arg_->flow_vec_delay;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_dmpac_dof_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(input_current = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(input_reference = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);

        if (arg_->vec_in_present == 1)
        {
            ASSERT_VX_OBJECT(flow_vector_in = vxCreateImage(context, width, height, flowVectorType), VX_TYPE_IMAGE);
        }

        ASSERT_VX_OBJECT(flow_vector_out = vxCreateImage(context, flow_width, flow_height, flowVectorType), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(confidence_histogram = vxCreateDistribution(context, 16, 0, 16), VX_TYPE_DISTRIBUTION);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node_dof = tivxDmpacDofNode(graph,
                                                     param_obj,
                                                     NULL,
                                                     NULL,
                                                     input_current,
                                                     input_reference,
                                                     flow_vector_in,
                                                     NULL,
                                                     NULL,
                                                     flow_vector_out,
                                                     confidence_histogram), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_dof, VX_TARGET_STRING, TIVX_TARGET_DMPAC_DOF));

        if (arg_->enable_pipeline == 1)
        {
            vx_graph_parameter_queue_params_t   q[2];
            uint32_t                            cnt = 0;
            uint32_t                            i;
            uint32_t                            pipelineDepth = 1;
            vx_status                           vxStatus;

            vxStatus = addParamByNodeIndex(graph, node_dof, 0);
            ASSERT(vxStatus == VX_SUCCESS);

            q[cnt++].refs_list = (vx_reference*)&param_obj;

            if (arg_->vec_in_present == 1)
            {
                vxStatus = addParamByNodeIndex(graph, node_dof, 5);
                ASSERT(vxStatus == VX_SUCCESS);
                q[cnt++].refs_list = (vx_reference*)&flow_vector_in;
            }

            for (i = 0; i < cnt; i++)
            {
                q[i].graph_parameter_index = i;
                q[i].refs_list_size = pipelineDepth;
            }

            vxStatus = vxSetGraphScheduleConfig(graph,
                                                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                                                cnt,
                                                q);

            ASSERT(vxStatus == VX_SUCCESS);

            /* explicitly set graph pipeline depth */
            vxStatus = tivxSetGraphPipelineDepth(graph,
                                                 pipelineDepth);

            ASSERT(vxStatus == VX_SUCCESS);

        }

        ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node_dof));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleasePyramid(&input_current));
        VX_CALL(vxReleasePyramid(&input_reference));
        VX_CALL(vxReleaseImage(&flow_vector_out));
        VX_CALL(vxReleaseDistribution(&confidence_histogram));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        if (flow_vector_in != NULL)
        {
            VX_CALL(vxReleaseImage(&flow_vector_in));
        }

        ASSERT(node_dof == 0);
        ASSERT(node_dof_vis == 0);
        ASSERT(graph == 0);
        ASSERT(input_current == 0);
        ASSERT(input_reference == 0);
        ASSERT(flow_vector_in == 0);
        ASSERT(flow_vector_out == 0);
        ASSERT(sof_mask == 0);
        ASSERT(confidence_histogram == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaDmpacDof, testGraphProcessing, testPredictors, testNegativeGraph, testNegativeConfig)

#endif /* BUILD_DMPAC_DOF */