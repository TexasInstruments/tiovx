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
}

TEST(tivxImage, negativeTestCreateImageFromChannel)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL, imgcnl = NULL;
    vx_uint32 width = 1, height = 1;
    vx_df_image format = VX_DF_IMAGE_RGB;
    vx_enum channel = VX_CHANNEL_0;

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
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, negativeTestCreateVirtualImage)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL;

    img = vxCreateVirtualImage(NULL, 0, 0, VX_DF_IMAGE_RGB);
}

TEST(tivxImage, negativeTestCreateUniformImage)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    vx_df_image format = TIVX_DF_IMAGE_RGB565;
    vx_pixel_value_t pvalue;

    EXPECT_VX_ERROR(img = vxCreateUniformImage(context, 0, 0, format, NULL), VX_ERROR_INVALID_PARAMETERS);
    ASSERT_VX_OBJECT(img = vxCreateUniformImage(context, 1, 1, format, &pvalue), VX_TYPE_IMAGE);
    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxImage, negativeTestReleaseImage)
{
    vx_context context = context_->vx_context_;

    vx_image img = NULL;
    vx_status status;

    status = vxReleaseImage(NULL);
    status = vxReleaseImage(&img);
}

TEST(tivxImage, negativeTestFormatImagePatchAddress1d)
{
    vx_context context = context_->vx_context_;

    vx_uint8 *p = NULL;
    void *vp = {0};
    vx_imagepatch_addressing_t paddr;

    paddr.dim_x = 1;
    paddr.dim_y = 1;
    p = vxFormatImagePatchAddress1d(NULL, 0, &paddr);
}

TEST(tivxImage, negativeTestFormatImagePatchAddress2d)
{
    vx_context context = context_->vx_context_;

    vx_uint8 *p = NULL;
    void *vp = {0};
    vx_imagepatch_addressing_t paddr;

    paddr.dim_x = 1;
    paddr.dim_y = 1;
    p = vxFormatImagePatchAddress2d(NULL, 0, 0, &paddr);
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
    negativeTestFormatImagePatchAddress2d
)

