/*

 * Copyright (c) 2015-2017 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * Copyright (c) 2022 Texas Instruments Incorporated
 */

#include <math.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx.h>
#include <TI/tivx_config.h>
#include "test_utils_mem_operations.h"

#include "test_engine/test.h"

TESTCASE(tivxImage, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tivxImage2, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxImage, negativeTestCreateImage)
{
    #define VX_DF_IMAGE_DEFAULT VX_DF_IMAGE('D','E','F','A')

    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    vx_uint32 width = 0, height = 0;
    vx_df_image format = VX_DF_IMAGE_VIRT;

    EXPECT_VX_ERROR(img = vxCreateImage(context, width, height, format), VX_ERROR_INVALID_PARAMETERS);
    EXPECT_VX_ERROR(img = vxCreateImage(context, width + 1, height, format), VX_ERROR_INVALID_PARAMETERS);
    EXPECT_VX_ERROR(img = vxCreateImage(context, width + 1, height + 1, VX_DF_IMAGE_DEFAULT), VX_ERROR_INVALID_PARAMETERS);
    EXPECT_VX_ERROR(img = vxCreateImage(NULL, width + 1, height + 1, VX_DF_IMAGE_IYUV), VX_ERROR_INVALID_PARAMETERS);
}

TEST(tivxImage, negativeTestCreateImageFromHandle)
{
    #define VX_PLANE_MAX 4
    #define VX_DF_IMAGE_DEFAULT VX_DF_IMAGE('D','E','F','A')

    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    vx_imagepatch_addressing_t ipa[VX_PLANE_MAX] = {VX_IMAGEPATCH_ADDR_INIT, VX_IMAGEPATCH_ADDR_INIT, VX_IMAGEPATCH_ADDR_INIT, VX_IMAGEPATCH_ADDR_INIT};
    void *mptrs[VX_PLANE_MAX] = {0, 0, 0, 0};
    vx_uint32 width = 0, height = 0;
    vx_df_image color = VX_DF_IMAGE_VIRT;

    EXPECT_VX_ERROR(img = vxCreateImageFromHandle(context, color, ipa, mptrs, VX_MEMORY_TYPE_HOST), VX_ERROR_INVALID_PARAMETERS);
    ipa[0].dim_x = 1;
    EXPECT_VX_ERROR(img = vxCreateImageFromHandle(context, color, ipa, mptrs, VX_MEMORY_TYPE_HOST), VX_ERROR_INVALID_PARAMETERS);
    ipa[0].dim_y = 1;
    EXPECT_VX_ERROR(img = vxCreateImageFromHandle(context, VX_DF_IMAGE_DEFAULT, ipa, mptrs, VX_MEMORY_TYPE_HOST), VX_ERROR_INVALID_PARAMETERS);
    ipa[0].stride_x = 1;
    EXPECT_VX_ERROR(img = vxCreateImageFromHandle(context, TIVX_DF_IMAGE_P12, ipa, mptrs, VX_MEMORY_TYPE_HOST), VX_ERROR_INVALID_PARAMETERS);
    ipa[0].stride_x = 0;
    EXPECT_VX_ERROR(img = vxCreateImageFromHandle(context, TIVX_DF_IMAGE_NV12_P12, ipa, mptrs, VX_MEMORY_TYPE_HOST), VX_ERROR_INVALID_PARAMETERS);
    EXPECT_VX_ERROR(img = vxCreateImageFromHandle(context, VX_DF_IMAGE_RGB, ipa, mptrs, VX_MEMORY_TYPE_HOST), VX_ERROR_INVALID_PARAMETERS);
    ipa[0].stride_x = -1;
    EXPECT_VX_ERROR(img = vxCreateImageFromHandle(context, VX_DF_IMAGE_RGB, ipa, mptrs, VX_MEMORY_TYPE_HOST), VX_ERROR_INVALID_PARAMETERS);
    ipa[0].stride_x = ipa[0].stride_y = 1;
    ASSERT_VX_OBJECT(img = vxCreateImageFromHandle(context, VX_DF_IMAGE_RGB, ipa, mptrs, VX_MEMORY_TYPE_HOST), VX_TYPE_IMAGE);
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, negativeTestCreateImageFromChannel)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL, imgcnl = NULL;
    vx_uint32 width = 1, height = 1;
    vx_df_image format = VX_DF_IMAGE_RGB;
    vx_enum channel = VX_CHANNEL_0;

    ASSERT(NULL == vxCreateImageFromChannel(img, channel));
    ASSERT_VX_OBJECT(img = vxCreateImage(context, width, height, format), VX_TYPE_IMAGE);
    EXPECT_VX_ERROR(imgcnl = vxCreateImageFromChannel(img, VX_CHANNEL_Y), VX_ERROR_INVALID_PARAMETERS);
    EXPECT_VX_ERROR(imgcnl = vxCreateImageFromChannel(img, VX_CHANNEL_U), VX_ERROR_INVALID_PARAMETERS);
    EXPECT_VX_ERROR(imgcnl = vxCreateImageFromChannel(img, channel), VX_ERROR_INVALID_PARAMETERS);
    VX_CALL(vxReleaseImage(&img));

    ASSERT_VX_OBJECT(img = vxCreateImage(context, 2, 2, VX_DF_IMAGE_IYUV), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(imgcnl = vxCreateImageFromChannel(img, VX_CHANNEL_U), VX_TYPE_IMAGE);
    VX_CALL(vxReleaseImage(&imgcnl));
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, negativeTestCreateImageFromROI)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL, imgroi = NULL;
    vx_uint32 width = 1, height = 1;
    vx_df_image format = VX_DF_IMAGE_RGB;
    vx_rectangle_t rect1 = {3, 3, 2, 2};
    vx_rectangle_t rect2 = {2, 3, 2, 2};
    vx_rectangle_t rect3 = {2, 2, 2, 2};

    ASSERT_VX_OBJECT(img = vxCreateImage(context, width, height, format), VX_TYPE_IMAGE);
    EXPECT_VX_ERROR(imgroi = vxCreateImageFromROI(img, NULL), VX_ERROR_INVALID_PARAMETERS);
    EXPECT_VX_ERROR(imgroi = vxCreateImageFromROI(img, &rect1), VX_ERROR_INVALID_PARAMETERS);
    EXPECT_VX_ERROR(imgroi = vxCreateImageFromROI(img, &rect2), VX_ERROR_INVALID_PARAMETERS);
    EXPECT_VX_ERROR(imgroi = vxCreateImageFromROI(img, &rect3), VX_ERROR_INVALID_PARAMETERS);
    ASSERT(NULL == vxCreateImageFromROI((vx_image)(context), &rect3));
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, negativeTestCreateVirtualImage)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL;

    ASSERT(NULL == (img = vxCreateVirtualImage(NULL, 0, 0, VX_DF_IMAGE_RGB)));
}

TEST(tivxImage, negativeTestCreateUniformImage)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    vx_df_image format = TIVX_DF_IMAGE_RGB565;
    vx_pixel_value_t pvalue;

    EXPECT_VX_ERROR(img = vxCreateUniformImage(context, 0, 0, format, NULL), VX_ERROR_INVALID_PARAMETERS);
    EXPECT_VX_ERROR(img = vxCreateUniformImage(context, 0, 0, format, &pvalue), VX_ERROR_INVALID_PARAMETERS);
    ASSERT_VX_OBJECT(img = vxCreateUniformImage(context, 1, 1, format, &pvalue), VX_TYPE_IMAGE);
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, negativeTestReleaseImage)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    vx_status status;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxReleaseImage(NULL));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxReleaseImage(&img));
}

TEST(tivxImage, negativeTestFormatImagePatchAddress1d)
{
    vx_context context = context_->vx_context_;

    vx_uint8 *p = NULL;
    void *vp = {0};
    vx_imagepatch_addressing_t paddr;

    paddr.dim_x = 1;
    paddr.dim_y = 1;
    ASSERT(NULL == (p = vxFormatImagePatchAddress1d(NULL, 0, &paddr)));
}

TEST(tivxImage, negativeTestFormatImagePatchAddress2d)
{
    vx_context context = context_->vx_context_;

    vx_uint8 *p = NULL, d = 0;
    void *vp = {0};
    vx_imagepatch_addressing_t paddr;

    paddr.dim_x = 1;
    paddr.dim_y = 1;
    ASSERT(NULL == (p = vxFormatImagePatchAddress2d(NULL, 0, 0, &paddr)));
    ASSERT(NULL == (p = vxFormatImagePatchAddress2d((void *)(&d), 1, 0, &paddr)));
}

TEST(tivxImage, negativeTestGetValidRegionImage)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    vx_rectangle_t rect;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxGetValidRegionImage(img, NULL));
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxGetValidRegionImage(img, NULL));
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, negativeTestSetImageValidRectangle)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    vx_rectangle_t rect1 = {2, 2, 1, 1};
    vx_rectangle_t rect2 = {2, 2, 17, 17};
    vx_rectangle_t rect3 = {1, 2, 2, 1};

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetImageValidRectangle(img, NULL));
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageValidRectangle(img, NULL));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetImageValidRectangle(img, &rect1));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetImageValidRectangle(img, &rect2));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetImageValidRectangle(img, &rect3));
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, negativeTestComputeImagePatchSize)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    vx_rectangle_t rect1 = {1, 1, 2, 2};
    vx_rectangle_t rect2 = {1, 1, 2, 17};
    vx_rectangle_t rect3 = {1, 1, 17, 2};
    vx_rectangle_t rect4 = {1, 2, 2, 1};
    vx_rectangle_t rect5 = {2, 1, 1, 2};
    vx_uint32 plane_index = 2;
    vx_size size = 0;

    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT(0 == (size = vxComputeImagePatchSize(img, &rect1, plane_index)));
    ASSERT(0 == (size = vxComputeImagePatchSize(img, &rect2, plane_index)));
    ASSERT(0 == (size = vxComputeImagePatchSize(img, &rect3, plane_index)));
    ASSERT(0 == (size = vxComputeImagePatchSize(img, &rect4, plane_index)));
    ASSERT(0 == (size = vxComputeImagePatchSize(img, &rect5, plane_index)));
    ASSERT(0 == (size = vxComputeImagePatchSize(img, NULL, plane_index)));
    ASSERT(0 == (size = vxComputeImagePatchSize(NULL, NULL, plane_index)));
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, negativeTestQueryImage)
{
    #define VX_IMAGE_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    vx_enum attribute = VX_IMAGE_DEFAULT;
    vx_size size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryImage(img, attribute, &size, size));
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryImage(img, VX_IMAGE_FORMAT, &size, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryImage(img, VX_IMAGE_WIDTH, &size, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryImage(img, VX_IMAGE_HEIGHT, &size, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryImage(img, VX_IMAGE_PLANES, &size, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryImage(img, VX_IMAGE_SPACE, &size, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryImage(img, VX_IMAGE_RANGE, &size, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryImage(img, VX_IMAGE_SIZE, &size, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryImage(img, VX_IMAGE_MEMORY_TYPE, &size, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryImage(img, attribute, &size, size));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(img, VX_IMAGE_SIZE, &size, sizeof(vx_size)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryImage(img, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &size, size));
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, negativeTestSetImageAttribute)
{
    #define VX_IMAGE_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    vx_enum attribute = VX_IMAGE_DEFAULT;
    vx_size size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetImageAttribute(img, attribute, &size, size));
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetImageAttribute(img, VX_IMAGE_SPACE, &size, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetImageAttribute(img, attribute, &size, size));
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, negativeTestCopyImagePatch)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    vx_rectangle_t rect = {1, 1, 2, 2};
    vx_uint32 plane_index = 0;
    vx_imagepatch_addressing_t user_addr;
    vx_uint8 user_ptr[256];
    vx_enum usage = VX_READ_ONLY, mem_type = VX_MEMORY_TYPE_HOST;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxCopyImagePatch(img, NULL, plane_index, NULL, NULL, VX_READ_AND_WRITE, mem_type));
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    user_addr.stride_x = 0;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxCopyImagePatch(img, &rect, plane_index, &user_addr, user_ptr, usage, mem_type));
    user_addr.stride_x = 2;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyImagePatch(img, &rect, plane_index, &user_addr, user_ptr, usage, mem_type));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyImagePatch(img, &rect, plane_index, &user_addr, user_ptr, VX_WRITE_ONLY, mem_type));
    VX_CALL(vxReleaseImage(&img));
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyImagePatch(img, &rect, plane_index, &user_addr, user_ptr, VX_WRITE_ONLY, mem_type));
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, negativeTestMapImagePatch)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    vx_rectangle_t rect = {1, 1, 2, 2};
    vx_uint32 plane_index = 0;
    vx_map_id map_id = 0;
    vx_imagepatch_addressing_t user_addr;
    vx_uint8 user_ptr[256];
    vx_enum usage = VX_READ_ONLY, mem_type = VX_MEMORY_TYPE_HOST;
    vx_uint32 flags = 0;
    void *base;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxMapImagePatch(img, &rect, plane_index, NULL, NULL, NULL, usage, mem_type, flags));
    //-ve Testcase for rect == NULL
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxMapImagePatch(img, NULL, plane_index, &map_id, &user_addr, &base, usage, mem_type, flags));
    VX_CALL(vxReleaseImage(&img));
    //-ve Testcase for wrong image type
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxMapImagePatch(NULL, &rect, plane_index, &map_id, &user_addr, &base, usage, mem_type, flags));
    plane_index = 2; // Create a -ve condition for plane index being > than No.of Img planes
    //-ve case to check condition start_y>end_y
    rect.start_y = 1;
    rect.end_y = 0;
    //-ve case to check condition start_x>end_x
    rect.start_x = 1;
    rect.end_x = 0;
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxMapImagePatch(img, &rect, plane_index, &map_id, &user_addr, &base, usage, mem_type, flags));
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, negativeTestUnmapImagePatch)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    vx_graph graph = NULL;
    vx_map_id map_id = TIVX_IMAGE_MAX_MAPS;
    uint8_t map_addr = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxUnmapImagePatch(img, map_id));
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxUnmapImagePatch(img, map_id));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxUnmapImagePatch(img, 0));
    VX_CALL(vxReleaseImage(&img));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(img = vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_OPTIMIZED_AWAY, vxUnmapImagePatch(img, 0));
    VX_CALL(vxReleaseImage(&img));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxImage, negativeTestSwapImageHandle)
{
    #define VX_PLANE_MAX 4

    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    void *new_ptrs[VX_PLANE_MAX] = {0, 0, 0, 0};
    void *prev_ptrs[VX_PLANE_MAX] = {0, 0, 0, 0};
    vx_size num_planes = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSwapImageHandle(img, NULL, NULL, num_planes));
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSwapImageHandle(img, NULL, NULL, num_planes));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSwapImageHandle(img, new_ptrs, NULL, 1));
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, testQueryImage)
{
    vx_context context = context_->vx_context_;

    vx_image image = NULL;
    vx_imagepatch_addressing_t image_addr;
    vx_uint32 stride_y_alignment;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(image, TIVX_IMAGE_IMAGEPATCH_ADDRESSING,  &image_addr,  sizeof(image_addr)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(image, TIVX_IMAGE_STRIDE_Y_ALIGNMENT,  &stride_y_alignment,  sizeof(stride_y_alignment)));

    /* Default stride alignment is 16 as defined in TIVX_DEFAULT_STRIDE_Y_ALIGN */
    ASSERT_EQ_INT(16, stride_y_alignment);
    ASSERT_EQ_INT(640, image_addr.dim_x);
    ASSERT_EQ_INT(480, image_addr.dim_y);
    ASSERT_EQ_INT(1, image_addr.stride_x);
    ASSERT_EQ_INT(640, image_addr.stride_y);
    ASSERT_EQ_INT(1024, image_addr.scale_x);
    ASSERT_EQ_INT(1024, image_addr.scale_y);
    ASSERT_EQ_INT(1, image_addr.step_x);
    ASSERT_EQ_INT(1, image_addr.step_y);

    VX_CALL(vxReleaseImage(&image));
}

TEST(tivxImage, testSetImageStride)
{
    #define TEST1_STRIDE_Y_ALIGNMENT 64
    #define TEST2_STRIDE_Y_ALIGNMENT 8
    vx_context context = context_->vx_context_;

    vx_image image = NULL;
    vx_imagepatch_addressing_t image_addr;
    vx_uint32 stride_y_alignment, set_stride_y_alignment = TEST1_STRIDE_Y_ALIGNMENT;
    vx_size img_size;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 648, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(image, TIVX_IMAGE_IMAGEPATCH_ADDRESSING,  &image_addr,  sizeof(image_addr)));

    /* Originally aligned to 16 for stride, so confirming here */
    ASSERT_EQ_INT(656, image_addr.stride_y);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(image, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(image, TIVX_IMAGE_IMAGEPATCH_ADDRESSING,  &image_addr,  sizeof(image_addr)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(image, VX_IMAGE_SIZE,  &img_size,  sizeof(img_size)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(image, TIVX_IMAGE_STRIDE_Y_ALIGNMENT,  &stride_y_alignment,  sizeof(stride_y_alignment)));

    ASSERT_EQ_INT(704*480, img_size);
    ASSERT_EQ_INT(TEST1_STRIDE_Y_ALIGNMENT, stride_y_alignment);
    ASSERT_EQ_INT(704, image_addr.stride_y);

    /* Now setting to an alignment of 8 and querying */
    set_stride_y_alignment = TEST2_STRIDE_Y_ALIGNMENT;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(image, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(image, TIVX_IMAGE_IMAGEPATCH_ADDRESSING,  &image_addr,  sizeof(image_addr)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(image, VX_IMAGE_SIZE,  &img_size,  sizeof(img_size)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(image, TIVX_IMAGE_STRIDE_Y_ALIGNMENT,  &stride_y_alignment,  sizeof(stride_y_alignment)));

    ASSERT_EQ_INT(648*480, img_size);
    ASSERT_EQ_INT(TEST2_STRIDE_Y_ALIGNMENT, stride_y_alignment);
    ASSERT_EQ_INT(648, image_addr.stride_y);

    VX_CALL(vxReleaseImage(&image));
}

TEST(tivxImage, testSetObjArrImageStride)
{
    #define NUM_OBJ_ARR_IMAGES 4

    vx_context context = context_->vx_context_;
    vx_image image_exemplar = NULL, img = NULL;
    vx_object_array obj_arr = NULL;
    vx_uint32 i, stride_y_alignment, set_stride_y_alignment = 8;

    ASSERT_VX_OBJECT(image_exemplar = vxCreateImage(context, 648, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(image_exemplar, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment)));
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)(image_exemplar), NUM_OBJ_ARR_IMAGES), VX_TYPE_OBJECT_ARRAY);
    VX_CALL(vxReleaseImage(&image_exemplar));

    for (i = 0; i < NUM_OBJ_ARR_IMAGES; i++)
    {
        ASSERT_VX_OBJECT(img = (vx_image)vxGetObjectArrayItem(obj_arr, i), VX_TYPE_IMAGE);

        ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetImageAttribute(img,TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment)));

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(img, TIVX_IMAGE_STRIDE_Y_ALIGNMENT,  &stride_y_alignment,  sizeof(stride_y_alignment)));

        ASSERT(stride_y_alignment==set_stride_y_alignment);

        VX_CALL(vxReleaseImage(&img));
    }

    VX_CALL(vxReleaseObjectArray(&obj_arr));
}

TEST(tivxImage, negativeTestSetImageStride)
{
    #define TEST_STRIDE_Y_ALIGNMENT 64
    vx_context context = context_->vx_context_;

    vx_image image = NULL;
    vx_uint32 stride_y_alignment;
    vx_size set_stride_y_alignment = TEST_STRIDE_Y_ALIGNMENT;
    vx_uint32 invalid_set_stride_y_alignment = 17;

    vx_pixel_value_t val = {{ 0xAB }};
    vx_size memsz;
    vx_size count_pixels = 0;
    vx_uint32 i;
    vx_uint32 j;
    vx_uint8* buffer;
    vx_uint8* buffer0;
    vx_rectangle_t rect             = { 0, 0, 640, 480 };
    vx_imagepatch_addressing_t addr = { 640, 480, 1, 640 };

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    /* This is expected to fail due to incorrect type parameter for setting the stride */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetImageAttribute(image, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment)));

    /* This is expected to fail since it is not a multiple of 8 */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetImageAttribute(image, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &invalid_set_stride_y_alignment, sizeof(invalid_set_stride_y_alignment)));

    memsz = vxComputeImagePatchSize(image, &rect, 0);
    ASSERT(memsz >= 640*480);

    ASSERT(buffer = ct_alloc_mem(memsz));
    buffer0 = buffer;

    // copy image data to our buffer
    VX_CALL(vxCopyImagePatch(image, &rect, 0, &addr, buffer, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    /* This is expected to fail since the image has already been allocated */
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetImageAttribute(image, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &stride_y_alignment, sizeof(stride_y_alignment)));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(image, TIVX_IMAGE_STRIDE_Y_ALIGNMENT,  &stride_y_alignment,  sizeof(stride_y_alignment)));

    /* Ensure this has not been updated since the previous calls failed */
    ASSERT_EQ_INT(16, stride_y_alignment);

    VX_CALL(vxReleaseImage(&image));
    ct_free_mem(buffer);
}

TEST(tivxImage, negativeTestSubmage)
{
    vx_context context = context_->vx_context_;

    vx_image image = NULL;
    vx_image img1=NULL;
    vx_image img[TIVX_IMAGE_MAX_SUBIMAGES+1]; // Added  an extra subimage to create overflow condition
    int i;
    vx_uint32 set_stride_y_alignment = TEST_STRIDE_Y_ALIGNMENT;
    
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_YUV4), VX_TYPE_IMAGE);

    for (i= 0; i <=TIVX_IMAGE_MAX_SUBIMAGES; i++)
    { // Max out the SubImage allocation space
        if(i<TIVX_IMAGE_MAX_SUBIMAGES)
        {
            //MaxOut the buffer 
            ASSERT_VX_OBJECT(img[i]=vxCreateImageFromChannel(image, VX_CHANNEL_V),VX_TYPE_IMAGE);
        }
        else //Create the error condition
        {
            EXPECT_VX_ERROR(img[i] = vxCreateImageFromChannel(image, VX_CHANNEL_V), VX_ERROR_NO_RESOURCES);
        }
        
    }

    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetImageAttribute(image, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetImageAttribute(img[0], TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment)));

    for (int j = 0; j < i; j++)
    { // Max out the SubImage allocation space
        if(NULL!=img[j])
        {
            vxReleaseImage(&img[j]);
        }
    }
    VX_CALL(vxReleaseImage(&image));
}
TEST(tivxImage, negativeTestIsValidDimension)
{
    vx_context context = context_->vx_context_;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_DIMENSION, vxGetStatus((vx_reference)vxCreateImage(context, 641, 480, VX_DF_IMAGE_IYUV)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxGetStatus((vx_reference)vxCreateImage(context, 641, 480, VX_REFERENCE_TYPE)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_DIMENSION, vxGetStatus((vx_reference)vxCreateImage(context, 641, 480, VX_DF_IMAGE_YUYV)));
}

TEST(tivxImage, negativeTestQueryImage1)
{
    vx_context context = context_->vx_context_;
    vx_image img = NULL;
    vx_enum attribute = TIVX_IMAGE_IMAGEPATCH_ADDRESSING;
    vx_size size = 0;
    vx_imagepatch_addressing_t patch[5];

    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryImage(img, attribute, NULL, size));

    /* to hit num_dims > (vx_size)TIVX_IMAGE_MAX_PLANE for the attribute TIVX_IMAGE_IMAGEPATCH_ADDRESSING */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(img, attribute, &patch, sizeof(patch)));

    VX_CALL(vxReleaseImage(&img));
}

/* Testcase to create image of TIVX_DF_IMAGE_BGRX format*/
TEST(tivxImage, testCreateImage)
{
    vx_context context = context_->vx_context_;
    vx_image img = NULL;
    vx_uint32 width = 640, height = 480;

    ASSERT_VX_OBJECT(img = vxCreateImage(context, width, height, TIVX_DF_IMAGE_BGRX), VX_TYPE_IMAGE);

    VX_CALL(vxReleaseImage(&img));
}

/* Testcase to fail ownIsSupportedFourcc() inside ownCreateImageInt API */
TEST(tivxImage, negativeTestCreateImage1)
{
    #define VX_DF_IMAGE_DEFAULT VX_DF_IMAGE('D','E','F','A')

    vx_context context = context_->vx_context_;
    vx_image img = NULL;
    vx_graph graph = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    EXPECT_VX_ERROR(img = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_DEFAULT), VX_ERROR_INVALID_FORMAT);

    VX_CALL(vxReleaseGraph(&graph));
}

/* Testcase to fail imagepatch_addr->stride_x != 0 condition for the attribute TIVX_IMAGE_STRIDE_Y_ALIGNMENT inside vxSetImageAttribute API */
TEST(tivxImage, testSetImageAttribute)
{
    vx_context context = context_->vx_context_;
    vx_image img = NULL;
    vx_uint32 width = 640, height = 480;
    vx_uint32 set_stride_y_alignment = TEST1_STRIDE_Y_ALIGNMENT;

    ASSERT_VX_OBJECT(img = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12 ), VX_TYPE_IMAGE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(img, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment)));
    VX_CALL(vxReleaseImage(&img));
}

/* Testcase to hit error condition inside ownCopyAndMapCheckParams() for invalid access mode(other than VX_READ_ONLY) for uniform image */
TEST(tivxImage, negativeTestCopyAndMapCheckParam1)
{
    vx_context context = context_->vx_context_;
    vx_image img = NULL;
    vx_size  map_size;
    vx_map_id map_id = 0;
    vx_rectangle_t rect = {0, 0, 1, 1};
    vx_imagepatch_addressing_t addr = { 0 };
    vx_uint8* base_ptr = NULL;
    vx_pixel_value_t pvalue;

    ASSERT_VX_OBJECT(img = vxCreateUniformImage(context,640 , 480, VX_DF_IMAGE_YUYV, &pvalue), VX_TYPE_IMAGE);

    ASSERT_EQ_VX_STATUS( VX_ERROR_NOT_SUPPORTED, vxMapImagePatch(img, &rect, 0, &map_id, &addr, (void**)&base_ptr, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0));

    VX_CALL(vxReleaseImage(&img));
}

/* Testcase To fail vxMapImagePatch() inside vxCreateUniformImage API by failing tivxMemBufferAlloc() by maxing out heap memory */
TEST(tivxImage, negativeTestCreateUniformImage1)
{
    vx_context context = context_->vx_context_;
    vx_image image = NULL;
    vx_pixel_value_t pvalue;

    tivx_shared_mem_info_t *tivx_shared_mem_info_array;
    uint32_t num_chunks;
    vx_enum mheap_region = TIVX_MEM_EXTERNAL;

    /* Allocating all the memory under heap region TIVX_MEM_EXTERNAL using test-utils mem api*/
    VX_CALL(test_utils_max_out_heap_mem(&tivx_shared_mem_info_array, &num_chunks, mheap_region));

    /* vxCreateUniformImage should fail due to tivxMemBufferAlloc failure */
    EXPECT_VX_ERROR(image = vxCreateUniformImage(context,640 , 480, VX_DF_IMAGE_YUYV, &pvalue), VX_FAILURE);

    /* Freeing all the previously allocated memory */
    VX_CALL(test_utils_release_maxed_out_heap_mem(tivx_shared_mem_info_array, num_chunks));
}

TEST(tivxImage, negativeTestCreateImageFromChannelROIBoundary)
{
    vx_context context = context_->vx_context_;
    vx_image image[TIVX_IMAGE_MAX_SUBIMAGE_DEPTH + 1], img1;
    vx_uint32 width = 640, height = 480;
    vx_df_image format = VX_DF_IMAGE_YUV4;
    vx_rectangle_t rect = {0, 0, width, height/2};
    int i =0;

    ASSERT_VX_OBJECT(image[0] = vxCreateImage(context, width, height, format), VX_TYPE_IMAGE);

    for(i = 0;i<TIVX_IMAGE_MAX_SUBIMAGE_DEPTH;i++)
    {
        ASSERT_VX_OBJECT(image[i+1] = vxCreateImageFromROI(image[i], &rect), VX_TYPE_IMAGE);
    }
    /* To hit condition parent subimages >= TIVX_IMAGE_MAX_SUBIMAGE_DEPTH */
    EXPECT_VX_ERROR(img1 = vxCreateImageFromROI(image[i], &rect), VX_ERROR_NO_RESOURCES);

    EXPECT_VX_ERROR(img1 = vxCreateImageFromChannel(image[i], VX_CHANNEL_V), VX_ERROR_NO_RESOURCES);

    for(i = 0;i<=TIVX_IMAGE_MAX_SUBIMAGE_DEPTH;i++)
    {
        VX_CALL(vxReleaseImage(&image[i]));
    }
}

/*
// Allocates image plane pointers from user controlled memory according to format, width, height params
// and initialize with some value
*/
static void own_allocate_image_pointers(
    vx_df_image format, int width, int height,
    vx_uint32* nplanes, void* ptrs[], vx_imagepatch_addressing_t addr[],
    vx_pixel_value_t* val)
{
    unsigned int p;
    int channel[VX_PLANE_MAX] = { 0, 0, 0, 0 };

    channel[0] = VX_CHANNEL_Y;
    channel[1] = VX_CHANNEL_U;
    channel[2] = VX_CHANNEL_V;


    ASSERT_NO_FAILURE(*nplanes = ct_get_num_planes(format));

    for (p = 0; p < *nplanes; p++)
    {
        size_t plane_size = 0;

        vx_uint32 subsampling_x = 2;
        vx_uint32 subsampling_y = 2;

        addr[p].dim_x    = width  / subsampling_x;
        addr[p].dim_y    = height / subsampling_y;
        addr[p].stride_x = 1;
        addr[p].stride_y = width;

        plane_size = addr[p].stride_y * addr[p].dim_y;

        if (plane_size != 0)
        {
            ptrs[p] = ct_alloc_mem(plane_size);
            /* init memory */
            ct_memset(ptrs[p], val->reserved[p], plane_size);
        }
    }

    return;
}

/* To hit condition 'si_obj_desc -> create_type == ( vx_enum ) TIVX_IMAGE_FROM_CHANNEL' inside ownSwapSubImage() */
TEST(tivxImage, testOwnSwapSubImageChannel)
{
    vx_context context = context_->vx_context_;
    vx_uint32 n;
    vx_image image;
    vx_image subimage;
    vx_imagepatch_addressing_t addr1[VX_PLANE_MAX] =
    {
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT
    };

    vx_uint32 nplanes1 = 0;

    vx_pixel_value_t val1;
    void* mem1_ptrs[VX_PLANE_MAX] = { 0, 0, 0, 0 };

    val1.reserved[0] = 0x11;
    val1.reserved[1] = 0x22;
    val1.reserved[2] = 0x33;
    val1.reserved[3] = 0x44;

    own_allocate_image_pointers(VX_DF_IMAGE_NV12, 16, 16, &nplanes1, mem1_ptrs, addr1, &val1);

    EXPECT_VX_OBJECT(image = vxCreateImageFromHandle(context, VX_DF_IMAGE_NV12, addr1, mem1_ptrs, VX_MEMORY_TYPE_HOST), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(subimage = vxCreateImageFromChannel(image, VX_CHANNEL_Y), VX_TYPE_IMAGE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS,vxSwapImageHandle(image, mem1_ptrs, NULL, nplanes1));

    VX_CALL(vxReleaseImage(&subimage));

    for (n = 0; n < VX_PLANE_MAX; n++)
    {
        if (mem1_ptrs[n] != NULL)
        {
            ct_free_mem(mem1_ptrs[n]);
            mem1_ptrs[n] = NULL;
        }
    }

    VX_CALL(vxReleaseImage(&image));
}

TEST(tivxImage2, negativeTestBranchQueryImage)
{
    vx_context context = context_->vx_context_;
    vx_image img = NULL;
    vx_uint8* base_ptr = NULL;

    ASSERT_VX_OBJECT(img = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryImage(img, (vx_enum)TIVX_IMAGE_IMAGEPATCH_ADDRESSING,(void *)&base_ptr, 0));

    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage2, negativeTestBranchCreateImageFromHandle)
{
    vx_context context = context_->vx_context_;
    vx_image image = NULL;
    vx_imagepatch_addressing_t addr[10];

    addr[0].dim_x = 1;
    addr[0].dim_y = 1;
    ASSERT(NULL == vxCreateImageFromHandle(NULL, VX_DF_IMAGE_RGB, addr,NULL,VX_MEMORY_TYPE_HOST));
}

TEST(tivxImage2, negativeTestBranchOwnValidDimensions)
{
    vx_context context = context_->vx_context_;
    vx_image image = NULL;
    vx_image img = NULL;
    vx_imagepatch_addressing_t addr[10];

    EXPECT_VX_ERROR(image = vxCreateImage(context, 1,1,(vx_df_image)VX_DF_IMAGE_UYVY),VX_ERROR_INVALID_DIMENSION);
    EXPECT_VX_ERROR(img = vxCreateImage(context, 1,1,(vx_df_image)VX_DF_IMAGE_NV12),VX_ERROR_INVALID_DIMENSION);
}

TESTCASE_TESTS(
    tivxImage,
    negativeTestCreateImage,
    negativeTestCreateImageFromHandle,
    negativeTestCreateImageFromChannel,
    negativeTestCreateImageFromROI,
    negativeTestCreateVirtualImage,
    negativeTestCreateUniformImage,
    negativeTestReleaseImage,
    negativeTestFormatImagePatchAddress1d,
    negativeTestFormatImagePatchAddress2d,
    negativeTestGetValidRegionImage,
    negativeTestSetImageValidRectangle,
    negativeTestComputeImagePatchSize,
    negativeTestQueryImage,
    negativeTestSetImageAttribute,
    negativeTestCopyImagePatch,
    negativeTestMapImagePatch,
    negativeTestUnmapImagePatch,
    negativeTestSwapImageHandle,
    negativeTestIsValidDimension,
    negativeTestSubmage,
    negativeTestSetImageStride,
    negativeTestQueryImage1,
    negativeTestCreateImage1,
    negativeTestCopyAndMapCheckParam1,
    negativeTestCreateUniformImage1,
    testQueryImage,
    testSetImageStride,
    testSetObjArrImageStride,
    testCreateImage,
    testSetImageAttribute,
    negativeTestCreateImageFromChannelROIBoundary,
    testOwnSwapSubImageChannel
    )

TESTCASE_TESTS(
    tivxImage2,
    negativeTestBranchQueryImage,
    negativeTestBranchCreateImageFromHandle,
    negativeTestBranchOwnValidDimensions
    )
