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
#ifdef BUILD_DMPAC_SDE 

#include <VX/vx.h>
#include <TI/tivx.h>
#include "test_engine/test.h"
#include <string.h>
#include "tivx_utils_file_rd_wr.h"
#include "tivx_utils_checksum.h"
#include "test_hwa_common.h"

#define MAX_ABS_FILENAME (1024)

static vx_status dump_binary_to_file(vx_image image, uint32_t width, uint32_t height, char* filename)
{
    FILE                       *pFile;
    vx_status                   status = VX_SUCCESS;
    vx_rectangle_t              rect;
    vx_imagepatch_addressing_t  image_addr;
    vx_map_id                   map_id;
    uint16_t                   *data_ptr;
    char                        full_filename[MAX_ABS_FILENAME];

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;


    snprintf(full_filename, MAX_ABS_FILENAME, "%s/%s",
        ct_get_test_file_path(), filename);

    if(NULL != image)
    {
        status = vxMapImagePatch(image,
            &rect,
            0,
            &map_id,
            &image_addr,
            (void**) &data_ptr,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        if (VX_SUCCESS == status)
        {
            pFile = fopen(full_filename,"wb");

            if (pFile){
                fwrite(data_ptr, image_addr.stride_y * image_addr.dim_y, 1, pFile);
                fclose(pFile);
            }
            else
            {
                status = VX_FAILURE;
                printf(" ERROR: Failed to open file %s!\n", filename);
            }
        }
        else
        {
            status = VX_FAILURE;
            printf(" ERROR: Failed to map image!\n");
        }
    }
    else
    {
        status = VX_FAILURE;
        printf(" ERROR: Null image!\n");
    }

    return status;
}

static vx_status convert_s16_to_u8(vx_context context, vx_image in_image,
    uint32_t width, uint32_t height, vx_image out_image)
{
    vx_status                   status = VX_FAILURE;
    vx_rectangle_t              rect;
    vx_imagepatch_addressing_t  in_image_addr;
    vx_imagepatch_addressing_t  out_image_addr;
    vx_map_id                   in_map_id;
    vx_map_id                   out_map_id;
    uint16_t                   *in_data_ptr;
    uint8_t                    *out_data_ptr;

    int                         i;

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
            for(i = 0; i < width*height; i++)
            {
                out_data_ptr[i] = (uint8_t)((in_data_ptr[i] & 0x7FFF) >> 7);
            }
        }

        status |= vxUnmapImagePatch(in_image, in_map_id);
        status |= vxUnmapImagePatch(out_image, out_map_id);
    }

    return status;
}

static vx_status crop_image(vx_context context, vx_image in_image,
    uint32_t in_width, uint32_t in_height, uint32_t crop_width,
    uint32_t crop_height, vx_image out_image)
{
    vx_status                   status = VX_FAILURE;
    vx_rectangle_t              in_rect;
    vx_rectangle_t              out_rect;
    vx_imagepatch_addressing_t  in_image_addr;
    vx_imagepatch_addressing_t  out_image_addr;
    vx_map_id                   in_map_id;
    vx_map_id                   out_map_id;
    uint8_t                    *in_data_ptr;
    uint8_t                    *out_data_ptr;

    int                         i;
    int                         j;

    if ((NULL != in_image) &&
        (NULL != out_image))
    {
        in_rect.start_x = 0;
        in_rect.start_y = 0;
        in_rect.end_x = in_width;
        in_rect.end_y = in_height;

        out_rect.start_x = 0;
        out_rect.start_y = 0;
        out_rect.end_x = crop_width;
        out_rect.end_y = crop_height;

        status = vxMapImagePatch(in_image,
            &in_rect,
            0,
            &in_map_id,
            &in_image_addr,
            (void**) &in_data_ptr,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        status |= vxMapImagePatch(out_image,
            &out_rect,
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
            for (i = 0U; i< crop_height * out_image_addr.stride_y; i += out_image_addr.stride_y)
            {
                for (j = 0U; j < crop_width * out_image_addr.stride_x; j++)
                {
                    out_data_ptr[i + j] = in_data_ptr[i + j];
                }
            }
        }

        status |= vxUnmapImagePatch(in_image, in_map_id);
        status |= vxUnmapImagePatch(out_image, out_map_id);
    }

    return status;
}

static vx_status load_image_to_sde(vx_image image, char* filename)
{
    vx_status status;
    char full_filename[MAX_ABS_FILENAME];

    snprintf(full_filename, MAX_ABS_FILENAME, "%s/%s",
        ct_get_test_file_path(), filename);

    status = tivx_utils_load_vximage_from_bmpfile(image, full_filename, vx_false_e);
    return status;
}

static vx_status save_image_from_sde(vx_image image, char* file_path)
{
    vx_status status;
    char filename[MAX_ABS_FILENAME];

    snprintf(filename, MAX_ABS_FILENAME, "%s/%s",
        ct_get_test_file_path(), file_path);

    status = tivx_utils_save_vximage_to_bmpfile(filename, image);
    return status;
}

TESTCASE(tivxHwaDmpacSde, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxHwaDmpacSde, testNodeCreation)
{
    vx_context context = context_->vx_context_;
    vx_image left_image = 0, right_image = 0, dst_image = 0;
    tivx_dmpac_sde_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_SDE));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(left_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(right_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

        memset(&params, 0, sizeof(tivx_dmpac_sde_params_t));
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_dmpac_sde_params_t",
                                                            sizeof(tivx_dmpac_sde_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxDmpacSdeNode(graph, param_obj, left_image, right_image, dst_image, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_DMPAC_SDE));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&right_image));
        VX_CALL(vxReleaseImage(&left_image));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(dst_image == 0);
        ASSERT(right_image == 0);
        ASSERT(left_image == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}


static CT_Image convolve_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static CT_Image convolve_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileNameLeft;
    const char* fileNameRight;
    uint32_t dispMin, dispMax;
    vx_int32 median;
    vx_int32 texture;
    uint32_t hist_output;
    vx_border_t border;
    int width, height;
} Arg;

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int negative_test;
    int condition;
    vx_border_t border;
} ArgNegative;

static uint32_t sde_checksums_ref[2*3*2*2*2] = {
    (uint32_t) 0xd3d68387, (uint32_t) 0xd3d68387, (uint32_t) 0xd3d68387, (uint32_t) 0xd3d68387,
    (uint32_t) 0xe7db447f, (uint32_t) 0xe7db447f, (uint32_t) 0xe7db447f, (uint32_t) 0xe7db447f,
    (uint32_t) 0x2dafc07d, (uint32_t) 0x2dafc07d, (uint32_t) 0x2dafc07d, (uint32_t) 0x2dafc07d,
    (uint32_t) 0x353aff75, (uint32_t) 0x353aff75, (uint32_t) 0x353aff75, (uint32_t) 0x353aff75,
    (uint32_t) 0x7a6607fd, (uint32_t) 0x7a6607fd, (uint32_t) 0x7a6607fd, (uint32_t) 0x7a6607fd,
    (uint32_t) 0xd2bcaf9d, (uint32_t) 0xd2bcaf9d, (uint32_t) 0xd2bcaf9d, (uint32_t) 0xd2bcaf9d,
    (uint32_t) 0x153492ab, (uint32_t) 0x153492ab, (uint32_t) 0x153492ab, (uint32_t) 0x153492ab,
    (uint32_t) 0xec8aff9b, (uint32_t) 0xec8aff9b, (uint32_t) 0xec8aff9b, (uint32_t) 0xec8aff9b,
    (uint32_t) 0xfcd8367d, (uint32_t) 0xfcd8367d, (uint32_t) 0xfcd8367d, (uint32_t) 0xfcd8367d,
    (uint32_t) 0x850d6b35, (uint32_t) 0x850d6b35, (uint32_t) 0x850d6b35, (uint32_t) 0x850d6b35,
    (uint32_t) 0x57bb0d44, (uint32_t) 0x57bb0d44, (uint32_t) 0x57bb0d44, (uint32_t) 0x57bb0d44,
    (uint32_t) 0xb46bad5c, (uint32_t) 0xb46bad5c, (uint32_t) 0xb46bad5c, (uint32_t) 0xb46bad5c
};

static uint32_t sde_hist_ref[2*3*2*2][128] = {
    {362910U, 3514U, 5632U, 3919U, 5926U, 5184U, 4302U, 1066U, 7071U, 2632U, 3723U, 1123U, 3227U, 1479U, 775U, 3U, 8411U, 312U, 926U, 795U, 1229U, 1448U, 760U, 75U, 2175U, 450U, 975U, 91U, 837U, 171U, 59U, 0U, 3441U, 7U, 100U, 119U, 294U, 538U, 87U, 3U, 849U, 57U, 361U, 5U, 163U, 27U, 4U, 0U, 1669U, 1U, 15U, 21U, 58U, 202U, 15U, 0U, 430U, 14U, 108U, 0U, 37U, 2U, 0U, 0U, 2852U, 0U, 0U, 2U, 15U, 56U, 3U, 0U, 215U, 5U, 41U, 0U, 8U, 0U, 0U, 0U, 592U, 0U, 0U, 0U, 4U, 12U, 0U, 0U, 98U, 0U, 10U, 0U, 4U, 0U, 0U, 0U, 413U, 0U, 0U, 0U, 2U, 5U, 0U, 0U, 46U, 0U, 4U, 0U, 1U, 0U, 0U, 0U, 276U, 0U, 0U, 0U, 1U, 3U, 0U, 0U, 31U, 0U, 0U, 0U, 0U, 0U, 0U, 3322U},
    {362910U, 3514U, 5632U, 3919U, 5926U, 5184U, 4302U, 1066U, 7071U, 2632U, 3723U, 1123U, 3227U, 1479U, 775U, 3U, 8411U, 312U, 926U, 795U, 1229U, 1448U, 760U, 75U, 2175U, 450U, 975U, 91U, 837U, 171U, 59U, 0U, 3441U, 7U, 100U, 119U, 294U, 538U, 87U, 3U, 849U, 57U, 361U, 5U, 163U, 27U, 4U, 0U, 1669U, 1U, 15U, 21U, 58U, 202U, 15U, 0U, 430U, 14U, 108U, 0U, 37U, 2U, 0U, 0U, 2852U, 0U, 0U, 2U, 15U, 56U, 3U, 0U, 215U, 5U, 41U, 0U, 8U, 0U, 0U, 0U, 592U, 0U, 0U, 0U, 4U, 12U, 0U, 0U, 98U, 0U, 10U, 0U, 4U, 0U, 0U, 0U, 413U, 0U, 0U, 0U, 2U, 5U, 0U, 0U, 46U, 0U, 4U, 0U, 1U, 0U, 0U, 0U, 276U, 0U, 0U, 0U, 1U, 3U, 0U, 0U, 31U, 0U, 0U, 0U, 0U, 0U, 0U, 3322U},
    {362910U, 3514U, 5632U, 3919U, 5926U, 5184U, 4302U, 1066U, 7071U, 2632U, 3723U, 1123U, 3227U, 1479U, 775U, 3U, 8411U, 312U, 926U, 795U, 1229U, 1448U, 760U, 75U, 2175U, 450U, 975U, 91U, 837U, 171U, 59U, 0U, 3441U, 7U, 100U, 119U, 294U, 538U, 87U, 3U, 849U, 57U, 361U, 5U, 163U, 27U, 4U, 0U, 1669U, 1U, 15U, 21U, 58U, 202U, 15U, 0U, 430U, 14U, 108U, 0U, 37U, 2U, 0U, 0U, 2852U, 0U, 0U, 2U, 15U, 56U, 3U, 0U, 215U, 5U, 41U, 0U, 8U, 0U, 0U, 0U, 592U, 0U, 0U, 0U, 4U, 12U, 0U, 0U, 98U, 0U, 10U, 0U, 4U, 0U, 0U, 0U, 413U, 0U, 0U, 0U, 2U, 5U, 0U, 0U, 46U, 0U, 4U, 0U, 1U, 0U, 0U, 0U, 276U, 0U, 0U, 0U, 1U, 3U, 0U, 0U, 31U, 0U, 0U, 0U, 0U, 0U, 0U, 3322U},
    {362910U, 3514U, 5632U, 3919U, 5926U, 5184U, 4302U, 1066U, 7071U, 2632U, 3723U, 1123U, 3227U, 1479U, 775U, 3U, 8411U, 312U, 926U, 795U, 1229U, 1448U, 760U, 75U, 2175U, 450U, 975U, 91U, 837U, 171U, 59U, 0U, 3441U, 7U, 100U, 119U, 294U, 538U, 87U, 3U, 849U, 57U, 361U, 5U, 163U, 27U, 4U, 0U, 1669U, 1U, 15U, 21U, 58U, 202U, 15U, 0U, 430U, 14U, 108U, 0U, 37U, 2U, 0U, 0U, 2852U, 0U, 0U, 2U, 15U, 56U, 3U, 0U, 215U, 5U, 41U, 0U, 8U, 0U, 0U, 0U, 592U, 0U, 0U, 0U, 4U, 12U, 0U, 0U, 98U, 0U, 10U, 0U, 4U, 0U, 0U, 0U, 413U, 0U, 0U, 0U, 2U, 5U, 0U, 0U, 46U, 0U, 4U, 0U, 1U, 0U, 0U, 0U, 276U, 0U, 0U, 0U, 1U, 3U, 0U, 0U, 31U, 0U, 0U, 0U, 0U, 0U, 0U, 3322U},
    {370474U, 3076U, 5382U, 3745U, 5651U, 5049U, 3985U, 897U, 6713U, 2361U, 3448U, 898U, 2969U, 1215U, 625U, 0U, 8180U, 223U, 724U, 659U, 1007U, 1292U, 575U, 56U, 1940U, 345U, 870U, 72U, 653U, 130U, 38U, 0U, 3154U, 2U, 75U, 86U, 193U, 455U, 62U, 1U, 763U, 41U, 288U, 1U, 109U, 13U, 0U, 0U, 1494U, 0U, 7U, 13U, 43U, 140U, 11U, 0U, 346U, 7U, 82U, 0U, 29U, 0U, 0U, 0U, 2842U, 0U, 0U, 3U, 15U, 38U, 1U, 0U, 157U, 1U, 23U, 0U, 3U, 0U, 0U, 0U, 505U, 0U, 0U, 0U, 0U, 13U, 0U, 0U, 68U, 0U, 9U, 0U, 2U, 0U, 0U, 0U, 347U, 0U, 0U, 1U, 0U, 3U, 0U, 0U, 21U, 0U, 3U, 0U, 2U, 0U, 0U, 0U, 227U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 20U, 0U, 0U, 0U, 0U, 0U, 0U, 2876U},
    {370474U, 3076U, 5382U, 3745U, 5651U, 5049U, 3985U, 897U, 6713U, 2361U, 3448U, 898U, 2969U, 1215U, 625U, 0U, 8180U, 223U, 724U, 659U, 1007U, 1292U, 575U, 56U, 1940U, 345U, 870U, 72U, 653U, 130U, 38U, 0U, 3154U, 2U, 75U, 86U, 193U, 455U, 62U, 1U, 763U, 41U, 288U, 1U, 109U, 13U, 0U, 0U, 1494U, 0U, 7U, 13U, 43U, 140U, 11U, 0U, 346U, 7U, 82U, 0U, 29U, 0U, 0U, 0U, 2842U, 0U, 0U, 3U, 15U, 38U, 1U, 0U, 157U, 1U, 23U, 0U, 3U, 0U, 0U, 0U, 505U, 0U, 0U, 0U, 0U, 13U, 0U, 0U, 68U, 0U, 9U, 0U, 2U, 0U, 0U, 0U, 347U, 0U, 0U, 1U, 0U, 3U, 0U, 0U, 21U, 0U, 3U, 0U, 2U, 0U, 0U, 0U, 227U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 20U, 0U, 0U, 0U, 0U, 0U, 0U, 2876U},
    {370474U, 3076U, 5382U, 3745U, 5651U, 5049U, 3985U, 897U, 6713U, 2361U, 3448U, 898U, 2969U, 1215U, 625U, 0U, 8180U, 223U, 724U, 659U, 1007U, 1292U, 575U, 56U, 1940U, 345U, 870U, 72U, 653U, 130U, 38U, 0U, 3154U, 2U, 75U, 86U, 193U, 455U, 62U, 1U, 763U, 41U, 288U, 1U, 109U, 13U, 0U, 0U, 1494U, 0U, 7U, 13U, 43U, 140U, 11U, 0U, 346U, 7U, 82U, 0U, 29U, 0U, 0U, 0U, 2842U, 0U, 0U, 3U, 15U, 38U, 1U, 0U, 157U, 1U, 23U, 0U, 3U, 0U, 0U, 0U, 505U, 0U, 0U, 0U, 0U, 13U, 0U, 0U, 68U, 0U, 9U, 0U, 2U, 0U, 0U, 0U, 347U, 0U, 0U, 1U, 0U, 3U, 0U, 0U, 21U, 0U, 3U, 0U, 2U, 0U, 0U, 0U, 227U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 20U, 0U, 0U, 0U, 0U, 0U, 0U, 2876U},
    {370474U, 3076U, 5382U, 3745U, 5651U, 5049U, 3985U, 897U, 6713U, 2361U, 3448U, 898U, 2969U, 1215U, 625U, 0U, 8180U, 223U, 724U, 659U, 1007U, 1292U, 575U, 56U, 1940U, 345U, 870U, 72U, 653U, 130U, 38U, 0U, 3154U, 2U, 75U, 86U, 193U, 455U, 62U, 1U, 763U, 41U, 288U, 1U, 109U, 13U, 0U, 0U, 1494U, 0U, 7U, 13U, 43U, 140U, 11U, 0U, 346U, 7U, 82U, 0U, 29U, 0U, 0U, 0U, 2842U, 0U, 0U, 3U, 15U, 38U, 1U, 0U, 157U, 1U, 23U, 0U, 3U, 0U, 0U, 0U, 505U, 0U, 0U, 0U, 0U, 13U, 0U, 0U, 68U, 0U, 9U, 0U, 2U, 0U, 0U, 0U, 347U, 0U, 0U, 1U, 0U, 3U, 0U, 0U, 21U, 0U, 3U, 0U, 2U, 0U, 0U, 0U, 227U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 20U, 0U, 0U, 0U, 0U, 0U, 0U, 2876U},
    {374164U, 2855U, 5225U, 3584U, 5518U, 4930U, 3792U, 789U, 6603U, 2219U, 3346U, 835U, 2856U, 1089U, 512U, 0U, 8043U, 174U, 621U, 601U, 934U, 1213U, 493U, 43U, 1885U, 295U, 826U, 51U, 559U, 108U, 27U, 0U, 2992U, 3U, 51U, 68U, 158U, 427U, 50U, 0U, 708U, 28U, 241U, 1U, 90U, 14U, 0U, 0U, 1397U, 0U, 2U, 13U, 40U, 107U, 8U, 0U, 309U, 5U, 69U, 0U, 21U, 0U, 0U, 0U, 2864U, 0U, 0U, 2U, 10U, 23U, 0U, 0U, 139U, 1U, 19U, 0U, 2U, 0U, 0U, 0U, 480U, 0U, 0U, 0U, 0U, 10U, 0U, 0U, 62U, 0U, 7U, 0U, 2U, 0U, 0U, 0U, 323U, 0U, 0U, 1U, 0U, 2U, 0U, 0U, 11U, 0U, 3U, 0U, 1U, 0U, 0U, 0U, 214U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 18U, 0U, 0U, 0U, 0U, 0U, 0U, 2661U},
    {374164U, 2855U, 5225U, 3584U, 5518U, 4930U, 3792U, 789U, 6603U, 2219U, 3346U, 835U, 2856U, 1089U, 512U, 0U, 8043U, 174U, 621U, 601U, 934U, 1213U, 493U, 43U, 1885U, 295U, 826U, 51U, 559U, 108U, 27U, 0U, 2992U, 3U, 51U, 68U, 158U, 427U, 50U, 0U, 708U, 28U, 241U, 1U, 90U, 14U, 0U, 0U, 1397U, 0U, 2U, 13U, 40U, 107U, 8U, 0U, 309U, 5U, 69U, 0U, 21U, 0U, 0U, 0U, 2864U, 0U, 0U, 2U, 10U, 23U, 0U, 0U, 139U, 1U, 19U, 0U, 2U, 0U, 0U, 0U, 480U, 0U, 0U, 0U, 0U, 10U, 0U, 0U, 62U, 0U, 7U, 0U, 2U, 0U, 0U, 0U, 323U, 0U, 0U, 1U, 0U, 2U, 0U, 0U, 11U, 0U, 3U, 0U, 1U, 0U, 0U, 0U, 214U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 18U, 0U, 0U, 0U, 0U, 0U, 0U, 2661U},
    {374164U, 2855U, 5225U, 3584U, 5518U, 4930U, 3792U, 789U, 6603U, 2219U, 3346U, 835U, 2856U, 1089U, 512U, 0U, 8043U, 174U, 621U, 601U, 934U, 1213U, 493U, 43U, 1885U, 295U, 826U, 51U, 559U, 108U, 27U, 0U, 2992U, 3U, 51U, 68U, 158U, 427U, 50U, 0U, 708U, 28U, 241U, 1U, 90U, 14U, 0U, 0U, 1397U, 0U, 2U, 13U, 40U, 107U, 8U, 0U, 309U, 5U, 69U, 0U, 21U, 0U, 0U, 0U, 2864U, 0U, 0U, 2U, 10U, 23U, 0U, 0U, 139U, 1U, 19U, 0U, 2U, 0U, 0U, 0U, 480U, 0U, 0U, 0U, 0U, 10U, 0U, 0U, 62U, 0U, 7U, 0U, 2U, 0U, 0U, 0U, 323U, 0U, 0U, 1U, 0U, 2U, 0U, 0U, 11U, 0U, 3U, 0U, 1U, 0U, 0U, 0U, 214U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 18U, 0U, 0U, 0U, 0U, 0U, 0U, 2661U},
    {374164U, 2855U, 5225U, 3584U, 5518U, 4930U, 3792U, 789U, 6603U, 2219U, 3346U, 835U, 2856U, 1089U, 512U, 0U, 8043U, 174U, 621U, 601U, 934U, 1213U, 493U, 43U, 1885U, 295U, 826U, 51U, 559U, 108U, 27U, 0U, 2992U, 3U, 51U, 68U, 158U, 427U, 50U, 0U, 708U, 28U, 241U, 1U, 90U, 14U, 0U, 0U, 1397U, 0U, 2U, 13U, 40U, 107U, 8U, 0U, 309U, 5U, 69U, 0U, 21U, 0U, 0U, 0U, 2864U, 0U, 0U, 2U, 10U, 23U, 0U, 0U, 139U, 1U, 19U, 0U, 2U, 0U, 0U, 0U, 480U, 0U, 0U, 0U, 0U, 10U, 0U, 0U, 62U, 0U, 7U, 0U, 2U, 0U, 0U, 0U, 323U, 0U, 0U, 1U, 0U, 2U, 0U, 0U, 11U, 0U, 3U, 0U, 1U, 0U, 0U, 0U, 214U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 18U, 0U, 0U, 0U, 0U, 0U, 0U, 2661U},
    {363023U, 3507U, 5654U, 3959U, 5937U, 5187U, 4305U, 1056U, 7083U, 2602U, 3707U, 1086U, 3250U, 1468U, 773U, 2U, 8387U, 304U, 945U, 804U, 1232U, 1424U, 747U, 73U, 2203U, 462U, 975U, 92U, 824U, 173U, 68U, 0U, 3480U, 7U, 100U, 124U, 284U, 542U, 91U, 2U, 830U, 53U, 359U, 5U, 162U, 28U, 2U, 0U, 1668U, 1U, 14U, 19U, 65U, 196U, 15U, 0U, 413U, 12U, 100U, 0U, 38U, 2U, 0U, 0U, 2851U, 0U, 0U, 2U, 14U, 52U, 3U, 0U, 217U, 2U, 40U, 0U, 8U, 1U, 0U, 0U, 577U, 0U, 0U, 1U, 3U, 13U, 0U, 0U, 97U, 0U, 9U, 0U, 2U, 0U, 0U, 0U, 394U, 0U, 0U, 1U, 2U, 6U, 0U, 0U, 39U, 0U, 4U, 0U, 0U, 0U, 0U, 0U, 272U, 0U, 0U, 0U, 1U, 2U, 0U, 0U, 29U, 0U, 0U, 0U, 0U, 0U, 0U, 3282U},
    {363023U, 3507U, 5654U, 3959U, 5937U, 5187U, 4305U, 1056U, 7083U, 2602U, 3707U, 1086U, 3250U, 1468U, 773U, 2U, 8387U, 304U, 945U, 804U, 1232U, 1424U, 747U, 73U, 2203U, 462U, 975U, 92U, 824U, 173U, 68U, 0U, 3480U, 7U, 100U, 124U, 284U, 542U, 91U, 2U, 830U, 53U, 359U, 5U, 162U, 28U, 2U, 0U, 1668U, 1U, 14U, 19U, 65U, 196U, 15U, 0U, 413U, 12U, 100U, 0U, 38U, 2U, 0U, 0U, 2851U, 0U, 0U, 2U, 14U, 52U, 3U, 0U, 217U, 2U, 40U, 0U, 8U, 1U, 0U, 0U, 577U, 0U, 0U, 1U, 3U, 13U, 0U, 0U, 97U, 0U, 9U, 0U, 2U, 0U, 0U, 0U, 394U, 0U, 0U, 1U, 2U, 6U, 0U, 0U, 39U, 0U, 4U, 0U, 0U, 0U, 0U, 0U, 272U, 0U, 0U, 0U, 1U, 2U, 0U, 0U, 29U, 0U, 0U, 0U, 0U, 0U, 0U, 3282U},
    {363023U, 3507U, 5654U, 3959U, 5937U, 5187U, 4305U, 1056U, 7083U, 2602U, 3707U, 1086U, 3250U, 1468U, 773U, 2U, 8387U, 304U, 945U, 804U, 1232U, 1424U, 747U, 73U, 2203U, 462U, 975U, 92U, 824U, 173U, 68U, 0U, 3480U, 7U, 100U, 124U, 284U, 542U, 91U, 2U, 830U, 53U, 359U, 5U, 162U, 28U, 2U, 0U, 1668U, 1U, 14U, 19U, 65U, 196U, 15U, 0U, 413U, 12U, 100U, 0U, 38U, 2U, 0U, 0U, 2851U, 0U, 0U, 2U, 14U, 52U, 3U, 0U, 217U, 2U, 40U, 0U, 8U, 1U, 0U, 0U, 577U, 0U, 0U, 1U, 3U, 13U, 0U, 0U, 97U, 0U, 9U, 0U, 2U, 0U, 0U, 0U, 394U, 0U, 0U, 1U, 2U, 6U, 0U, 0U, 39U, 0U, 4U, 0U, 0U, 0U, 0U, 0U, 272U, 0U, 0U, 0U, 1U, 2U, 0U, 0U, 29U, 0U, 0U, 0U, 0U, 0U, 0U, 3282U},
    {363023U, 3507U, 5654U, 3959U, 5937U, 5187U, 4305U, 1056U, 7083U, 2602U, 3707U, 1086U, 3250U, 1468U, 773U, 2U, 8387U, 304U, 945U, 804U, 1232U, 1424U, 747U, 73U, 2203U, 462U, 975U, 92U, 824U, 173U, 68U, 0U, 3480U, 7U, 100U, 124U, 284U, 542U, 91U, 2U, 830U, 53U, 359U, 5U, 162U, 28U, 2U, 0U, 1668U, 1U, 14U, 19U, 65U, 196U, 15U, 0U, 413U, 12U, 100U, 0U, 38U, 2U, 0U, 0U, 2851U, 0U, 0U, 2U, 14U, 52U, 3U, 0U, 217U, 2U, 40U, 0U, 8U, 1U, 0U, 0U, 577U, 0U, 0U, 1U, 3U, 13U, 0U, 0U, 97U, 0U, 9U, 0U, 2U, 0U, 0U, 0U, 394U, 0U, 0U, 1U, 2U, 6U, 0U, 0U, 39U, 0U, 4U, 0U, 0U, 0U, 0U, 0U, 272U, 0U, 0U, 0U, 1U, 2U, 0U, 0U, 29U, 0U, 0U, 0U, 0U, 0U, 0U, 3282U},
    {370742U, 3043U, 5382U, 3754U, 5662U, 5056U, 3959U, 886U, 6751U, 2338U, 3442U, 871U, 2970U, 1203U, 622U, 0U, 8156U, 222U, 721U, 653U, 1022U, 1272U, 563U, 56U, 1953U, 341U, 867U, 75U, 637U, 129U, 39U, 0U, 3194U, 2U, 77U, 89U, 178U, 460U, 65U, 0U, 744U, 41U, 278U, 1U, 107U, 13U, 0U, 0U, 1471U, 0U, 7U, 12U, 43U, 130U, 12U, 0U, 344U, 7U, 76U, 0U, 27U, 0U, 0U, 0U, 2847U, 0U, 0U, 3U, 15U, 35U, 1U, 0U, 157U, 0U, 25U, 0U, 4U, 0U, 0U, 0U, 478U, 0U, 0U, 0U, 1U, 13U, 0U, 0U, 67U, 0U, 8U, 0U, 2U, 0U, 0U, 0U, 333U, 0U, 0U, 1U, 0U, 4U, 0U, 0U, 19U, 0U, 2U, 0U, 2U, 0U, 0U, 0U, 232U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 19U, 0U, 0U, 0U, 0U, 0U, 0U, 2814U},
    {370742U, 3043U, 5382U, 3754U, 5662U, 5056U, 3959U, 886U, 6751U, 2338U, 3442U, 871U, 2970U, 1203U, 622U, 0U, 8156U, 222U, 721U, 653U, 1022U, 1272U, 563U, 56U, 1953U, 341U, 867U, 75U, 637U, 129U, 39U, 0U, 3194U, 2U, 77U, 89U, 178U, 460U, 65U, 0U, 744U, 41U, 278U, 1U, 107U, 13U, 0U, 0U, 1471U, 0U, 7U, 12U, 43U, 130U, 12U, 0U, 344U, 7U, 76U, 0U, 27U, 0U, 0U, 0U, 2847U, 0U, 0U, 3U, 15U, 35U, 1U, 0U, 157U, 0U, 25U, 0U, 4U, 0U, 0U, 0U, 478U, 0U, 0U, 0U, 1U, 13U, 0U, 0U, 67U, 0U, 8U, 0U, 2U, 0U, 0U, 0U, 333U, 0U, 0U, 1U, 0U, 4U, 0U, 0U, 19U, 0U, 2U, 0U, 2U, 0U, 0U, 0U, 232U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 19U, 0U, 0U, 0U, 0U, 0U, 0U, 2814U},
    {370742U, 3043U, 5382U, 3754U, 5662U, 5056U, 3959U, 886U, 6751U, 2338U, 3442U, 871U, 2970U, 1203U, 622U, 0U, 8156U, 222U, 721U, 653U, 1022U, 1272U, 563U, 56U, 1953U, 341U, 867U, 75U, 637U, 129U, 39U, 0U, 3194U, 2U, 77U, 89U, 178U, 460U, 65U, 0U, 744U, 41U, 278U, 1U, 107U, 13U, 0U, 0U, 1471U, 0U, 7U, 12U, 43U, 130U, 12U, 0U, 344U, 7U, 76U, 0U, 27U, 0U, 0U, 0U, 2847U, 0U, 0U, 3U, 15U, 35U, 1U, 0U, 157U, 0U, 25U, 0U, 4U, 0U, 0U, 0U, 478U, 0U, 0U, 0U, 1U, 13U, 0U, 0U, 67U, 0U, 8U, 0U, 2U, 0U, 0U, 0U, 333U, 0U, 0U, 1U, 0U, 4U, 0U, 0U, 19U, 0U, 2U, 0U, 2U, 0U, 0U, 0U, 232U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 19U, 0U, 0U, 0U, 0U, 0U, 0U, 2814U},
    {370742U, 3043U, 5382U, 3754U, 5662U, 5056U, 3959U, 886U, 6751U, 2338U, 3442U, 871U, 2970U, 1203U, 622U, 0U, 8156U, 222U, 721U, 653U, 1022U, 1272U, 563U, 56U, 1953U, 341U, 867U, 75U, 637U, 129U, 39U, 0U, 3194U, 2U, 77U, 89U, 178U, 460U, 65U, 0U, 744U, 41U, 278U, 1U, 107U, 13U, 0U, 0U, 1471U, 0U, 7U, 12U, 43U, 130U, 12U, 0U, 344U, 7U, 76U, 0U, 27U, 0U, 0U, 0U, 2847U, 0U, 0U, 3U, 15U, 35U, 1U, 0U, 157U, 0U, 25U, 0U, 4U, 0U, 0U, 0U, 478U, 0U, 0U, 0U, 1U, 13U, 0U, 0U, 67U, 0U, 8U, 0U, 2U, 0U, 0U, 0U, 333U, 0U, 0U, 1U, 0U, 4U, 0U, 0U, 19U, 0U, 2U, 0U, 2U, 0U, 0U, 0U, 232U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 19U, 0U, 0U, 0U, 0U, 0U, 0U, 2814U},
    {374353U, 2832U, 5216U, 3599U, 5499U, 4959U, 3782U, 779U, 6641U, 2214U, 3349U, 819U, 2842U, 1089U, 508U, 0U, 8030U, 170U, 620U, 589U, 939U, 1202U, 476U, 42U, 1911U, 293U, 808U, 51U, 555U, 113U, 28U, 0U, 3028U, 3U, 49U, 66U, 147U, 420U, 49U, 0U, 694U, 31U, 235U, 1U, 87U, 14U, 0U, 0U, 1376U, 0U, 3U, 12U, 38U, 102U, 9U, 0U, 314U, 5U, 65U, 0U, 21U, 0U, 0U, 0U, 2866U, 0U, 0U, 2U, 10U, 22U, 0U, 0U, 137U, 0U, 18U, 0U, 2U, 0U, 0U, 0U, 467U, 0U, 0U, 0U, 0U, 10U, 0U, 0U, 60U, 0U, 6U, 0U, 2U, 0U, 0U, 0U, 317U, 0U, 0U, 1U, 0U, 3U, 0U, 0U, 9U, 0U, 2U, 0U, 1U, 0U, 0U, 0U, 209U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 17U, 0U, 0U, 0U, 0U, 0U, 0U, 2609U},
    {374353U, 2832U, 5216U, 3599U, 5499U, 4959U, 3782U, 779U, 6641U, 2214U, 3349U, 819U, 2842U, 1089U, 508U, 0U, 8030U, 170U, 620U, 589U, 939U, 1202U, 476U, 42U, 1911U, 293U, 808U, 51U, 555U, 113U, 28U, 0U, 3028U, 3U, 49U, 66U, 147U, 420U, 49U, 0U, 694U, 31U, 235U, 1U, 87U, 14U, 0U, 0U, 1376U, 0U, 3U, 12U, 38U, 102U, 9U, 0U, 314U, 5U, 65U, 0U, 21U, 0U, 0U, 0U, 2866U, 0U, 0U, 2U, 10U, 22U, 0U, 0U, 137U, 0U, 18U, 0U, 2U, 0U, 0U, 0U, 467U, 0U, 0U, 0U, 0U, 10U, 0U, 0U, 60U, 0U, 6U, 0U, 2U, 0U, 0U, 0U, 317U, 0U, 0U, 1U, 0U, 3U, 0U, 0U, 9U, 0U, 2U, 0U, 1U, 0U, 0U, 0U, 209U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 17U, 0U, 0U, 0U, 0U, 0U, 0U, 2609U},
    {374353U, 2832U, 5216U, 3599U, 5499U, 4959U, 3782U, 779U, 6641U, 2214U, 3349U, 819U, 2842U, 1089U, 508U, 0U, 8030U, 170U, 620U, 589U, 939U, 1202U, 476U, 42U, 1911U, 293U, 808U, 51U, 555U, 113U, 28U, 0U, 3028U, 3U, 49U, 66U, 147U, 420U, 49U, 0U, 694U, 31U, 235U, 1U, 87U, 14U, 0U, 0U, 1376U, 0U, 3U, 12U, 38U, 102U, 9U, 0U, 314U, 5U, 65U, 0U, 21U, 0U, 0U, 0U, 2866U, 0U, 0U, 2U, 10U, 22U, 0U, 0U, 137U, 0U, 18U, 0U, 2U, 0U, 0U, 0U, 467U, 0U, 0U, 0U, 0U, 10U, 0U, 0U, 60U, 0U, 6U, 0U, 2U, 0U, 0U, 0U, 317U, 0U, 0U, 1U, 0U, 3U, 0U, 0U, 9U, 0U, 2U, 0U, 1U, 0U, 0U, 0U, 209U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 17U, 0U, 0U, 0U, 0U, 0U, 0U, 2609U},
    {374353U, 2832U, 5216U, 3599U, 5499U, 4959U, 3782U, 779U, 6641U, 2214U, 3349U, 819U, 2842U, 1089U, 508U, 0U, 8030U, 170U, 620U, 589U, 939U, 1202U, 476U, 42U, 1911U, 293U, 808U, 51U, 555U, 113U, 28U, 0U, 3028U, 3U, 49U, 66U, 147U, 420U, 49U, 0U, 694U, 31U, 235U, 1U, 87U, 14U, 0U, 0U, 1376U, 0U, 3U, 12U, 38U, 102U, 9U, 0U, 314U, 5U, 65U, 0U, 21U, 0U, 0U, 0U, 2866U, 0U, 0U, 2U, 10U, 22U, 0U, 0U, 137U, 0U, 18U, 0U, 2U, 0U, 0U, 0U, 467U, 0U, 0U, 0U, 0U, 10U, 0U, 0U, 60U, 0U, 6U, 0U, 2U, 0U, 0U, 0U, 317U, 0U, 0U, 1U, 0U, 3U, 0U, 0U, 9U, 0U, 2U, 0U, 1U, 0U, 0U, 0U, 209U, 0U, 0U, 0U, 0U, 1U, 0U, 0U, 17U, 0U, 0U, 0U, 0U, 0U, 0U, 2609U},
};

static uint32_t get_checksum(uint16_t a, uint16_t b, uint16_t c, uint16_t d, uint16_t e)
{
    return sde_checksums_ref[(24U * a) + (8U * b) + (4U * c) + (2U * d) + (e)];
}

static vx_status check_histogram(vx_distribution histogram,
    uint16_t a, uint16_t b, uint16_t c, uint16_t d)
{
    vx_status                   status = VX_SUCCESS;
    vx_map_id map_id;
    uint32_t *hist_ptr;
    uint16_t index;
    uint16_t i;

    index = (12U * a) + (4U * b) + (2U * c) + d;

    status = vxMapDistribution(histogram,
                &map_id,
                (void**) &hist_ptr,
                VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST,
                0);

    if (VX_SUCCESS == status)
    {
        for (i = 0U; i < 128U; i++)
        {
            if (sde_hist_ref[index][i] != hist_ptr[i])
            {
                status = VX_FAILURE;
                printf(" ERROR: Mismatch at index %d (%d != %d)\n", i, hist_ptr[i], sde_hist_ref[index][i]);
                break;
            }
        }
    }
    else
    {
        printf(" ERROR: Could not map distribution!\n");
    }

    return status;
}

#define ADD_DISPMIN(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dispMin=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/dispMin=-3", __VA_ARGS__, 1))

#define ADD_DISPMAX(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dispMax=min+63", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/dispMax=min+127", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/dispMax=min+191", __VA_ARGS__, 2))

#define ADD_MEDIAN(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/median=OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/median=ON", __VA_ARGS__, 1))

#define ADD_TEXTURE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/texture=OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/texture=ON", __VA_ARGS__, 1))

#define ADD_OUTPUT_HISTOGRAM(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/hist_output=OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/hist_output=ON", __VA_ARGS__, 1))

#define ADD_NEGATIVE_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/negative_test=median_filter_enable", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=reduced_range_search_enable", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=disparity_min", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=disparity_max", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=threshold_left_right", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=texture_filter_enable", __VA_ARGS__, 5)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=threshold_texture_disabled", __VA_ARGS__, 6)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=threshold_texture_enabled", __VA_ARGS__, 7)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=aggregation_penalty_p1", __VA_ARGS__, 8)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=aggregation_penalty_p2", __VA_ARGS__, 9)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=confidence_score_map", __VA_ARGS__, 10)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=width", __VA_ARGS__, 11)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=height", __VA_ARGS__, 12)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=available_sl2", __VA_ARGS__, 13)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=histogram_max_6", __VA_ARGS__, 14)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=histogram_max_7", __VA_ARGS__, 15)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=histogram_increasing", __VA_ARGS__, 16))

#define ADD_NEGATIVE_CONDITION(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_positive", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_positive", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/condition=negative", __VA_ARGS__, 2))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("real", ADD_DISPMIN, ADD_DISPMAX, ADD_MEDIAN, ADD_TEXTURE, ADD_OUTPUT_HISTOGRAM, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, convolve_read_image, "left_rect.bmp", "right_rect.bmp")

#define PARAMETERS_NEGATIVE \
    CT_GENERATE_PARAMETERS("randomInput", ADD_NEGATIVE_TEST, ADD_NEGATIVE_CONDITION, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ARG, convolve_generate_random, NULL)

TEST_WITH_ARG(tivxHwaDmpacSde, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image left_image = 0, right_image = 0, dst_image = 0, left_crop_image = 0, right_crop_image = 0, out_image = 0;
    vx_distribution histogram = 0;
    tivx_dmpac_sde_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_rectangle_t rect;
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    int i;

    CT_Image srcL = NULL;
    CT_Image srcR = NULL;
    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_SDE));

    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 640;
        rect.end_y = 716;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(srcL = arg_->generator(arg_->fileNameLeft, arg_->width, arg_->height));
        ASSERT_NO_FAILURE(srcR = arg_->generator(arg_->fileNameRight, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(left_image = ct_image_to_vx_image(srcL, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(right_image = ct_image_to_vx_image(srcR, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(left_crop_image = vxCreateImage(context, srcL->width / 2, srcL->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(right_crop_image = vxCreateImage(context, srcL->width / 2, srcL->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_NO_FAILURE(crop_image(context, left_image, srcL->width, srcL->height, srcL->width / 2, srcL->height, left_crop_image));
        ASSERT_NO_FAILURE(crop_image(context, right_image, srcL->width, srcL->height, srcL->width / 2, srcL->height, right_crop_image));
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, srcL->width / 2, srcL->height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out_image = vxCreateImage(context, srcL->width / 2, srcL->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        if(arg_->hist_output) {
            ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 128, 0, 4096), VX_TYPE_DISTRIBUTION);
        }

        memset(&params, 0, sizeof(tivx_dmpac_sde_params_t));
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_dmpac_sde_params_t",
                                                            sizeof(tivx_dmpac_sde_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.median_filter_enable = arg_->median;
        params.disparity_min = arg_->dispMin;
        params.disparity_max = arg_->dispMax;
        params.texture_filter_enable = arg_->texture;
        for(i = 0; i < 8; i++) {
            params.confidence_score_map[i] = i*8;
        }
        params.threshold_left_right = 0;
        params.threshold_texture = 0;
        params.aggregation_penalty_p1 = 0;
        params.aggregation_penalty_p2 = 0;
        params.reduced_range_search_enable = 0;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_dmpac_sde_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxDmpacSdeNode(graph, param_obj, left_crop_image, right_crop_image, dst_image, histogram), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_DMPAC_SDE));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

#if 0
        char output_file[256];
        sprintf(output_file, "output/sde_output_%d_%d_%d_%d.bmp", (int32_t) arg_->median, arg_->dispMin, arg_->dispMax, (int32_t) arg_->texture);
        VX_CALL(convert_s16_to_u8(context, dst_image, srcL->width / 2, srcL->height, out_image));
        VX_CALL(save_image_from_sde(out_image, output_file));
#endif

        checksum_expected = get_checksum(arg_->dispMin, arg_->dispMax, arg_->median, arg_->texture, arg_->hist_output);
        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
        ASSERT(checksum_expected == checksum_actual);

        if(arg_->hist_output) {
            VX_CALL(check_histogram(histogram, arg_->dispMin, arg_->dispMax, arg_->median, arg_->texture));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&out_image));
        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&right_crop_image));
        VX_CALL(vxReleaseImage(&left_crop_image));
        VX_CALL(vxReleaseImage(&right_image));
        VX_CALL(vxReleaseImage(&left_image));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        if(arg_->hist_output) {
            VX_CALL(vxReleaseDistribution(&histogram));
        }

        ASSERT(out_image == 0);
        ASSERT(dst_image == 0);
        ASSERT(right_crop_image == 0);
        ASSERT(left_crop_image == 0);
        ASSERT(right_image == 0);
        ASSERT(left_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(histogram == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaDmpacSde, testNegativeGraphProcessing, ArgNegative,
    PARAMETERS_NEGATIVE
)
{
    vx_context context = context_->vx_context_;
    vx_image left_image = 0, right_image = 0, dst_image = 0;
    vx_distribution histogram = 0;
    tivx_dmpac_sde_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    int width;
    int height;
    int i;

    CT_Image srcL = NULL;
    CT_Image srcR = NULL;
    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_SDE));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        memset(&params, 0, sizeof(tivx_dmpac_sde_params_t));
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_dmpac_sde_params_t",
                                                            sizeof(tivx_dmpac_sde_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        width = 640;
        height = 480;

        params.median_filter_enable = 0;
        params.disparity_min = 0;
        params.disparity_max = 0;
        params.texture_filter_enable = 0;
        for(i = 0; i < 8; i++) {
            params.confidence_score_map[i] = i*8;
        }
        params.threshold_left_right = 0;
        params.threshold_texture = 0;
        params.aggregation_penalty_p1 = 0;
        params.aggregation_penalty_p2 = 0;
        params.reduced_range_search_enable = 0;

        switch (arg_->negative_test)
        {
            case 0:
            {
                params.median_filter_enable = arg_->condition;
                break;
            }
            case 1:
            {
                params.reduced_range_search_enable = arg_->condition;
                break;
            }
            case 2:
            {
                params.disparity_min = arg_->condition;
                break;
            }
            case 3:
            {
                width = 256;
                height = 256;
                params.disparity_max = arg_->condition;
                if (params.disparity_max != 0)
                {
                    params.disparity_max++;
                }
                break;
            }
            case 4:
            {
                params.threshold_left_right = 255 * arg_->condition;
                if (2 == arg_->condition)
                {
                    params.threshold_left_right = 256;
                }
                break;
            }
            case 5:
            {
                params.texture_filter_enable = arg_->condition;
                break;
            }
            case 6:
            {
                params.texture_filter_enable = 0;
                params.threshold_texture = arg_->condition * 255;
                if (2 == arg_->condition)
                {
                    params.texture_filter_enable = 1;
                    params.threshold_texture = 256;
                }
                break;
            }
            case 7:
            {
                params.texture_filter_enable = 1;
                params.threshold_texture = arg_->condition * 255;
                if (2 == arg_->condition)
                {
                    params.threshold_texture = 256;
                }
                break;
            }
            case 8:
            {
                params.aggregation_penalty_p1 = 127 * arg_->condition;
                if (2 == arg_->condition)
                {
                    params.threshold_left_right = 128;
                }
                break;
            }
            case 9:
            {
                params.aggregation_penalty_p2 = 255 * arg_->condition;
                if (2 == arg_->condition)
                {
                    params.threshold_left_right = 256;
                }
                break;
            }
            case 10:
            {
                params.confidence_score_map[0] = 4088 * arg_->condition;
                if (2 == arg_->condition)
                {
                    params.confidence_score_map[0] = 4089;
                }
                for(i = 1; i < 8; i++)
                {
                    params.confidence_score_map[i] = params.confidence_score_map[i-1] + 1;
                }
                break;
            }
            case 11:
            {
                if (0 == arg_->condition)
                {
                    width = 128;
                }
                else if (1 == arg_->condition)
                {
                    width = 2048;
                }
                else
                {
                    width = 2064;
                }
                break;
            }
            case 12:
            {
                if (0 == arg_->condition)
                {
                    height = 64;
                }
                else if (1 == arg_->condition)
                {
                    height = 1024;
                }
                else
                {
                    height = 1040;
                }
                break;
            }
            case 13:
            {
                width = 1280;
                height = 720;
                if (0 == arg_->condition)
                {
                    params.disparity_max = 0;
                }
                else if (1 == arg_->condition)
                {
                    params.disparity_max = 1;
                }
                else
                {
                    params.disparity_max = 2;
                }
                break;
            }
            case 14:
            {
                if (0 == arg_->condition)
                {
                    for(i = 0; i < 8; i++)
                    {
                        params.confidence_score_map[i] = i;
                    }
                }
                else if (1 == arg_->condition)
                {
                    params.confidence_score_map[6] = 126;
                }
                else
                {
                    params.confidence_score_map[6] = 127;
                }
                break;
            }
            case 15:
            {
                if (0 == arg_->condition)
                {
                    for(i = 0; i < 8; i++)
                    {
                        params.confidence_score_map[i] = i;
                    }
                }
                else if (1 == arg_->condition)
                {
                    params.confidence_score_map[7] = 127;
                }
                else
                {
                    params.confidence_score_map[7] = 128;
                }
                break;
            }
            case 16:
            {
                if (0 == arg_->condition)
                {
                    params.confidence_score_map[0] = 0;
                    params.confidence_score_map[1] = 1;
                    params.confidence_score_map[2] = 3;
                    params.confidence_score_map[3] = 7;
                    params.confidence_score_map[4] = 15;
                    params.confidence_score_map[5] = 31;
                    params.confidence_score_map[6] = 63;
                    params.confidence_score_map[7] = 127;
                }
                else if (1 == arg_->condition)
                {
                    for(i = 0; i < 8; i++)
                    {
                        params.confidence_score_map[i] = i;
                    }
                }
                else
                {
                    for(i = 0; i < 8; i++)
                    {
                        params.confidence_score_map[i] = 63;
                    }
                }
                break;
            }
        }

        ASSERT_NO_FAILURE(srcL = arg_->generator(arg_->fileName, width, height));
        ASSERT_NO_FAILURE(srcR = arg_->generator(arg_->fileName, width, height));
        ASSERT_VX_OBJECT(left_image = ct_image_to_vx_image(srcL, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(right_image = ct_image_to_vx_image(srcR, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, srcL->width, srcL->height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_dmpac_sde_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxDmpacSdeNode(graph, param_obj, left_image, right_image, dst_image, histogram), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_DMPAC_SDE));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

        if(2 != arg_->condition)
        {
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
            ASSERT_NO_FAILURE(vxProcessGraph(graph));
        }
        else if ((11 == arg_->negative_test) ||
            (12 == arg_->negative_test))
        {
            ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
        }
        else if  (13 == arg_->negative_test)
        {
#if defined(PC)
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
            ASSERT_NO_FAILURE(vxProcessGraph(graph));
#else
            ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
#endif
        }
        else
        {
            ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&right_image));
        VX_CALL(vxReleaseImage(&left_image));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(dst_image == 0);
        ASSERT(right_image == 0);
        ASSERT(left_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(histogram == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST(tivxHwaDmpacSde, testRealGraphProcessing)
{
    vx_context context = context_->vx_context_;
    vx_image left_image = 0, right_image = 0, dst_image = 0, out_image = 0;
    vx_distribution histogram = 0;
    tivx_dmpac_sde_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    uint32_t width = 1280;
    uint32_t height  = 720;
    int i;

    vx_border_t border;
    border.mode = VX_BORDER_UNDEFINED;

    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_SDE));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(left_image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(right_image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, width, height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out_image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 128, 0, 4096), VX_TYPE_DISTRIBUTION);

        ASSERT_NO_FAILURE(load_image_to_sde(left_image, "left_rect.bmp"));
        ASSERT_NO_FAILURE(load_image_to_sde(right_image, "right_rect.bmp"));

        memset(&params, 0, sizeof(tivx_dmpac_sde_params_t));
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_dmpac_sde_params_t",
                                                            sizeof(tivx_dmpac_sde_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.median_filter_enable = 0;
        params.disparity_min = 0;
        params.disparity_max = 1;
        params.texture_filter_enable = 1;
        for(i = 0; i < 8; i++) {
            params.confidence_score_map[i] = i*8;
        }
        params.threshold_left_right = 3;
        params.threshold_texture = 100;
        params.aggregation_penalty_p1 = 32;
        params.aggregation_penalty_p2 = 64;
        params.reduced_range_search_enable = 0;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_dmpac_sde_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxDmpacSdeNode(graph, param_obj, left_image, right_image, dst_image, histogram), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_DMPAC_SDE));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

#if 0
        VX_CALL(convert_s16_to_u8(context, dst_image, 1280, 720, out_image));
        VX_CALL(save_image_from_sde(out_image, "output/sde_output.bmp"));
#endif

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&right_image));
        VX_CALL(vxReleaseImage(&left_image));
        VX_CALL(vxReleaseImage(&out_image));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseDistribution(&histogram));

        ASSERT(out_image == 0);
        ASSERT(dst_image == 0);
        ASSERT(right_image == 0);
        ASSERT(left_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(histogram == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaDmpacSde, testNodeCreation, testGraphProcessing, testRealGraphProcessing, testNegativeGraphProcessing)

#endif /* BUILD_DMPAC_SDE */