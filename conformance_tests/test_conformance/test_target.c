/* 
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
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
