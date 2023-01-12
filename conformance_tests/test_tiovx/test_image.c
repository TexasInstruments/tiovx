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

#include "test_engine/test.h"

TESTCASE(tivxImage, CT_VXContext, ct_setup_vx_context, 0)

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

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxMapImagePatch(img, &rect, plane_index, NULL, NULL, NULL, usage, mem_type, flags));
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

    vx_image img = NULL, imgcnl = NULL;
    void *new_ptrs[VX_PLANE_MAX] = {0, 0, 0, 0};
    void *prev_ptrs[VX_PLANE_MAX] = {0, 0, 0, 0};
    vx_uint8 ndata[256];
    vx_uint8 pdata[256];
    vx_size num_planes = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSwapImageHandle(img, NULL, NULL, num_planes));
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSwapImageHandle(img, NULL, NULL, num_planes));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSwapImageHandle(img, new_ptrs, NULL, 1));
    VX_CALL(vxReleaseImage(&img));
    new_ptrs[0] = new_ptrs[1] = new_ptrs[2] = ndata;
    prev_ptrs[0] = prev_ptrs[1] = prev_ptrs[2] = pdata;
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 2, 2, VX_DF_IMAGE_IYUV), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(imgcnl = vxCreateImageFromChannel(img, VX_CHANNEL_U), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSwapImageHandle(img, new_ptrs, prev_ptrs, 3));
    VX_CALL(vxReleaseImage(&imgcnl));
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, testQueryImage)
{
    vx_context context = context_->vx_context_;

    vx_image image = NULL;
    vx_imagepatch_addressing_t image_addr;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(image, TIVX_IMAGE_IMAGEPATCH_ADDRESSING,  &image_addr,  sizeof(image_addr)));

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
    testQueryImage
)

