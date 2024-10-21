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
 * Copyright (c) 2024 Texas Instruments Incorporated
 */
#include "test_tiovx.h"
#include "test_engine/test.h"
#include "test_tiovx/test_tiovx.h"
#include <VX/vx.h>
#include <VX/vxu.h>

#include <vx_internal.h>

#include "shared_functions.h"

TESTCASE(tivxInternalImage, CT_VXContext, ct_setup_vx_context, 0)

/* Testcase to fail valid_roi.start_x <= obj_desc -> valid_roi.end_x & valid_roi.start_y <= obj_desc -> valid_roi.end_y*/
TEST(tivxInternalImage, negativeTestGetValidRegionImage)
{
    vx_context context = context_->vx_context_;
    vx_image img = NULL;
    vx_rectangle_t rect;
    tivx_obj_desc_image_t *obj_desc = NULL;
    

    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    obj_desc = (tivx_obj_desc_image_t *)img->base.obj_desc;
    obj_desc->valid_roi.start_x = 16;
    obj_desc->valid_roi.end_x = 0;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxGetValidRegionImage(img, &rect));
    obj_desc->valid_roi.start_x = 0;
    obj_desc->valid_roi.end_x = 16;
    obj_desc->valid_roi.start_y = 16;
    obj_desc->valid_roi.end_y = 0;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxGetValidRegionImage(img, &rect));

    VX_CALL(vxReleaseImage(&img));
}

/* Testcase to hit branch conditions of vxUnmapImagePatch() */
TEST(tivxInternalImage, negativeTestUnmapImagePatch)
{
    vx_context context = context_->vx_context_;
    vx_image image = NULL;
    uint8_t *map_addr;
    vx_size  map_size;
    vx_map_id map_id = 0;
    vx_rectangle_t rect = {0, 0, 1, 1};
    vx_imagepatch_addressing_t addr = { 0 };
    vx_uint8* base_ptr = NULL;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(image, &rect, 0, &map_id, &addr, (void**)&base_ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));

    map_addr = image->maps[0].map_addr;
    map_size = image->maps[0].map_size;
    image->maps[0].map_addr=NULL;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxUnmapImagePatch(image, map_id));
    image->maps[0].map_addr = map_addr;
    image->maps[0].map_size=0;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxUnmapImagePatch(image, map_id));
    image->maps[0].map_size = map_size;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(image, map_id));

    VX_CALL(vxReleaseImage(&image));
}

TEST(tivxInternalImage, negativeTestOwnInitVirtualImage)
{
    vx_context context = context_->vx_context_;
    vx_image image = NULL;
    tivx_obj_desc_t *obj_desc= NULL;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownInitVirtualImage(NULL,0,0,VX_DF_IMAGE_U8));
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownInitVirtualImage(image,0,0,VX_DF_IMAGE_U8));
    /* to hit branch conditions*/
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownInitVirtualImage(image,16,0,VX_DF_IMAGE_U8));
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownInitVirtualImage(image,16,16,VX_DF_IMAGE_U8));

    obj_desc = image->base.obj_desc;
    image->base.obj_desc = NULL;
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownInitVirtualImage(image,16,16,VX_DF_IMAGE_U8));
    image->base.obj_desc = obj_desc;

    VX_CALL(vxReleaseImage(&image));

}

/* Testcase to hit fail ownCopyAndMapCheckParams() by passing virtual image by forcefully setting obj_desc->create_type*/
TEST(tivxInternalImage, negativeTestCopyAndMapCheckParam)
{
    vx_context context = context_->vx_context_;
    vx_image img = NULL;
    tivx_obj_desc_image_t *obj_desc = NULL;
    vx_size  map_size;
    vx_map_id map_id = 0;
    vx_rectangle_t rect = {0, 0, 1, 1};
    vx_imagepatch_addressing_t addr = { 0 };
    vx_uint8* base_ptr = NULL;

    ASSERT_VX_OBJECT(img = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    obj_desc = (tivx_obj_desc_image_t *)img->base.obj_desc;
    obj_desc->create_type = TIVX_IMAGE_VIRTUAL;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxMapImagePatch(img, &rect, 0, &map_id, &addr, (void**)&base_ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    obj_desc->create_type = TIVX_IMAGE_NORMAL;

    VX_CALL(vxReleaseImage(&img));
}

TEST(tivxInternalImage, negativeTestBranchOwnSizeofChannel)
{
    vx_context context = context_->vx_context_;
    vx_image image = NULL;
    vx_graph graph = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image = vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownInitVirtualImage(image,16,16,0));

    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalImage, TestBranchownCopyAndMapCheckParams)
{
    vx_context context = context_->vx_context_;
    vx_image image = NULL;
    vx_map_id map_id = 0;
    vx_rectangle_t rect = {0, 0, 1, 1};
    vx_imagepatch_addressing_t addr = { 0 };
    vx_uint8* base_ptr = NULL;
    vx_pixel_value_t pvalue;

    ASSERT_VX_OBJECT(image = vxCreateUniformImage(context,640 , 480, VX_DF_IMAGE_YUYV, &pvalue), VX_TYPE_IMAGE);
    image->base.is_virtual = (vx_bool)vx_false_e;
    image->base.is_accessible = (vx_bool)vx_true_e;
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxMapImagePatch(image, &rect, 0, &map_id, &addr, (void**)&base_ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxUnmapImagePatch(image, map_id));

    VX_CALL(vxReleaseImage(&image));
}

TESTCASE_TESTS(
    tivxInternalImage,
    negativeTestGetValidRegionImage,
    negativeTestUnmapImagePatch,
    negativeTestOwnInitVirtualImage,
    negativeTestCopyAndMapCheckParam,
    negativeTestBranchOwnSizeofChannel,
    TestBranchownCopyAndMapCheckParams
)