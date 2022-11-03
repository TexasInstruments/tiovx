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

#include "test_engine/test.h"

TESTCASE(tivxPymd, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxPymd, negativeTestCreatePyramid)
{
    vx_context context = context_->vx_context_;

    vx_pyramid pymd = NULL;
    vx_size levels = 0;
    vx_float32 scale = 0.0f;
    vx_uint32 width = 0, height = 0;
    vx_df_image format = 0;

    ASSERT(NULL == (pymd = vxCreatePyramid(NULL, levels, scale, width, height, format)));
    ASSERT(NULL == (pymd = vxCreatePyramid(context, levels, scale, width, height, format)));
    ASSERT(NULL == (pymd = vxCreatePyramid(context, levels, 1.1f, width, height, format)));
    ASSERT_VX_OBJECT(pymd = vxCreatePyramid(context, 3, 0.9f, 3, 3, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    VX_CALL(vxReleasePyramid(&pymd));
}

TEST(tivxPymd, negativeTestGetPyramidLevel)
{
    vx_context context = context_->vx_context_;

    vx_pyramid pymd = NULL;
    vx_image img = NULL;
    vx_size levels = 3;
    vx_float32 scale = 0.9f;
    vx_uint32 index = 3, width = 3, height = 3;
    vx_df_image format = VX_DF_IMAGE_U8;

    EXPECT_VX_ERROR(img = vxGetPyramidLevel(pymd, index), VX_ERROR_NO_RESOURCES);
    ASSERT_VX_OBJECT(pymd = vxCreatePyramid(context, levels, scale, width, height, format), VX_TYPE_PYRAMID);
    ASSERT(NULL != (img = vxGetPyramidLevel(pymd, index)));
    VX_CALL(vxReleasePyramid(&pymd));
}

TEST(tivxPymd, negativeTestCreateVirtualPyramid)
{
    vx_context context = context_->vx_context_;

    vx_pyramid pymd = NULL;
    vx_graph graph = NULL;
    vx_size levels = 0;
    vx_float32 scale = 0.0f;
    vx_uint32 width = 0, height = 0;
    vx_df_image format = VX_DF_IMAGE_U8;

    ASSERT(NULL == (pymd = vxCreateVirtualPyramid(graph, levels, scale, width, height, format)));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT(NULL == (pymd = vxCreateVirtualPyramid(graph, levels, scale, width, height, format)));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxPymd, negativeTestQueryPyramid)
{
    #define VX_PYRAMID_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_pyramid pymd = NULL;
    vx_enum attribute = VX_PYRAMID_DEFAULT;
    vx_uint32 udata = 0, width = 3, height = 3;
    vx_size levels = 3, size = 0;
    vx_float32 scale = 0.9f;
    vx_df_image format = VX_DF_IMAGE_U8;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryPyramid(pymd, VX_PYRAMID_LEVELS, &udata, size));
    ASSERT_VX_OBJECT(pymd = vxCreatePyramid(context, levels, scale, width, height, format), VX_TYPE_PYRAMID);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryPyramid(pymd, VX_PYRAMID_LEVELS, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryPyramid(pymd, VX_PYRAMID_SCALE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryPyramid(pymd, VX_PYRAMID_WIDTH, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryPyramid(pymd, VX_PYRAMID_HEIGHT, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryPyramid(pymd, VX_PYRAMID_FORMAT, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryPyramid(pymd, attribute, &udata, size));
    VX_CALL(vxReleasePyramid(&pymd));
}

TESTCASE_TESTS(
    tivxPymd,
    negativeTestCreatePyramid,
    negativeTestGetPyramidLevel,
    negativeTestCreateVirtualPyramid,
    negativeTestQueryPyramid
)

