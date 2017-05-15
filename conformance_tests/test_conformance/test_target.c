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


#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_kernels.h>


TESTCASE(Target, CT_VXContext, ct_setup_vx_context, 0)


typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    vx_enum target_enum;
    const char* target_string;

} SetTarget_Arg;


#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_TARGET_ANY", __VA_ARGS__, VX_TARGET_ANY, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TARGET_STRING=any", __VA_ARGS__, VX_TARGET_STRING, "any")), \
    CT_EXPAND(nextmacro(testArgName "/VX_TARGET_STRING=aNy", __VA_ARGS__, VX_TARGET_STRING, "aNy")), \
    CT_EXPAND(nextmacro(testArgName "/VX_TARGET_STRING=ANY", __VA_ARGS__, VX_TARGET_STRING, "ANY"))

#define SET_NODE_TARGET_PARAMETERS \
    CT_GENERATE_PARAMETERS("target", ADD_SET_TARGET_PARAMETERS, ARG, NULL)

#define SET_IMM_MODE_TARGET_PARAMETERS \
    CT_GENERATE_PARAMETERS("target", ADD_SET_TARGET_PARAMETERS, ARG, NULL)

TEST_WITH_ARG(Target, testvxSetNodeTarget, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;

    vx_image src1 = 0;
    vx_image src2 = 0;
    vx_image dst = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst  = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = vxAddNode(graph, src1, src2, VX_CONVERT_POLICY_WRAP, dst), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node, arg_->target_enum, arg_->target_string));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&dst));

    return;
}


TEST_WITH_ARG(Target, testvxSetImmediateModeTarget, SetTarget_Arg, SET_IMM_MODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;

    vx_image src1 = 0;
    vx_image src2 = 0;
    vx_image dst = 0;

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst  = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    VX_CALL(vxSetImmediateModeTarget(context, arg_->target_enum, arg_->target_string));

    VX_CALL(vxuAdd(context, src1, src2, VX_CONVERT_POLICY_WRAP, dst));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&dst));

    return;
}


TESTCASE_TESTS(Target,
        testvxSetNodeTarget,
        testvxSetImmediateModeTarget
        )
