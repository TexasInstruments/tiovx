/*

 * Copyright (c) 2012-2017 The Khronos Group Inc.
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

#include "test_tiovx.h"

#include <TI/tivx_test_kernels.h>
#include <TI/tivx_config.h>
#include <TI/tivx_capture.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include <string.h>
#include <math.h>

#include <vx_internal.h>

#include "shared_functions.h"


TESTCASE(tivxInternalPyramid, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalPyramid, negativeTestOwnInitVirtualPyramid)
{
    #define VX_FORMAT_DEFAULT 0
    vx_context context = context_->vx_context_;
    vx_pyramid pymd=NULL;
    vx_size levels = 3;
    vx_float32 scale = 0.9f;
    vx_uint32 index = 3, width = 0, height = 0;
    vx_df_image format = VX_FORMAT_DEFAULT;
    vx_graph graph = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(pymd = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    pymd->base.is_virtual = vx_false_e;
    ASSERT(VX_FAILURE == ownInitVirtualPyramid(pymd,width,height,format));

    VX_CALL(vxReleasePyramid(&pymd));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalPyramid, negativeTestOwnInitVirtualPyramidType)
{
    #define VX_FORMAT_DEFAULT 0
    #define VX_REFERENCE_TYPE 0
    vx_context context = context_->vx_context_;
    vx_pyramid pymd=NULL;
    vx_size levels = 3;
    vx_float32 scale = 0.9f;
    vx_uint32 index = 3, width = 0, height = 0;
    vx_df_image format = VX_FORMAT_DEFAULT;
    vx_graph graph = 0;
    tivx_obj_desc_t *obj_desc = NULL;
    vx_enum type;
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(pymd = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    type = (((vx_reference)pymd)->type);
    (((vx_reference)pymd)->type) = VX_REFERENCE_TYPE;
    ASSERT(VX_FAILURE == ownInitVirtualPyramid(pymd, width, height, format));
    (((vx_reference)pymd)->type) = type;
    VX_CALL(vxReleasePyramid(&pymd));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalPyramid, negativeTestOwnInitVirtualPyramidDesc)
{
    #define VX_FORMAT_DEFAULT 0
    #define VX_REFERENCE_TYPE 0
    vx_context context = context_->vx_context_;
    vx_pyramid pymd=NULL;
    vx_size levels = 3;
    vx_float32 scale = 0.9f;
    vx_uint32 index = 3, width = 0, height = 0;
    vx_df_image format = VX_FORMAT_DEFAULT;
    vx_graph graph = 0;
    tivx_obj_desc_t *obj_desc = NULL;
    vx_enum type;
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(pymd = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    obj_desc = pymd->base.obj_desc;
    pymd->base.obj_desc = NULL;
    ASSERT(VX_FAILURE == ownInitVirtualPyramid(pymd, width, height, format));
    pymd->base.obj_desc = obj_desc;
    VX_CALL(vxReleasePyramid(&pymd));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Testcase to fail ownIsValidContext() API for invalid pyramid reference passed */
TEST(tivxInternalPyramid, negativeTestGetPyramidLevel)
{
    vx_context context = context_->vx_context_;
    vx_pyramid pymd = NULL;
    vx_image img = NULL;
    vx_size levels = 3;
    vx_float32 scale = 0.9f;
    vx_uint32 index = 3, width = 3, height = 3;
    vx_df_image format = VX_DF_IMAGE_U8;

    ASSERT_VX_OBJECT(pymd = vxCreatePyramid(context, levels, scale, width, height, format), VX_TYPE_PYRAMID);

    vx_reference ref = (vx_reference)pymd;
    /* To fail initial condition for valid pyramid reference check, ref->type is forcefully set to a type other than PYRAMID */
    ref->type = VX_TYPE_ARRAY;
    /* To fail ownIsValidContext() API */
    context->base.magic = TIVX_BAD_MAGIC;

    ASSERT(NULL == vxGetPyramidLevel(pymd, index));
    ref->type = VX_TYPE_PYRAMID;
    context->base.magic = TIVX_MAGIC;

    VX_CALL(vxReleasePyramid(&pymd));
}

TESTCASE_TESTS(tivxInternalPyramid,
    negativeTestOwnInitVirtualPyramid,
    negativeTestOwnInitVirtualPyramidType,
    negativeTestOwnInitVirtualPyramidDesc,
    negativeTestGetPyramidLevel
    )