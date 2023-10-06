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

#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx_config.h>
#include <TI/tivx.h>
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_capture.h>

#include "test_engine/test.h"

#define VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE1_NAME1 "org.khronos.openvx.test.user_source_1"

TESTCASE(tivxNodeApi, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxNodeApi, negativeTestWarpAffineNode)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_matrix matrix = NULL;
    vx_enum type = 0;
    vx_image output = NULL;

    ASSERT(NULL == vxWarpAffineNode(graph, input, matrix, type, output));
}

TEST(tivxNodeApi, negativeTestWarpPerspectiveNode)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_matrix matrix = NULL;
    vx_enum type = 0;
    vx_image output = NULL;

    ASSERT(NULL == vxWarpPerspectiveNode(graph, input, matrix, type, output));
}

TEST(tivxNodeApi, negativeTestRemapNode)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_remap table = NULL;
    vx_enum policy = 0;
    vx_image output = NULL;

    ASSERT(NULL == vxRemapNode(graph, input, table, policy, output));
}

/* tivxCreateNodeByKernelName , if(kernel!=NULL)*/
TEST(tivxNodeApi, negativeTesttivxCreateNodeByKernelName)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_remap table = NULL;
    vx_reference prms[]= {NULL};
    ASSERT(NULL == tivxCreateNodeByKernelName(graph, NULL, prms,dimof(prms)));
}

/* vxCreateNodeByStructure , #line 71*/
TEST(tivxNodeApi, negativeTesttivxCreateNodeByStructure)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_remap table = NULL;
    vx_reference prms[]= {NULL};
    vx_enum kernel_id = 10;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxAllocateUserKernelId(context, &kernel_id));
    vx_kernel kernel = vxAddUserKernel(
                    context,
                    "com.ti.test_kernels.cmd_timeout_test",
                    kernel_id,
                    NULL,
                    3,
                    NULL,
                    NULL,
                    NULL);

    ASSERT(NULL == tivxCreateNodeByKernelRef(graph, kernel, prms,3));
    VX_CALL(vxReleaseKernel(&kernel));
}

TEST(tivxNodeApi, negativeTesttivxCreateNodeByStructure1)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image image;

    ASSERT_VX_OBJECT(image   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    vx_reference params[2] =
    {
        (vx_reference)image
    };
    ASSERT(NULL == tivxCreateNodeByKernelEnum(graph, VX_KERNEL_ABSDIFF, params, 2));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseGraph(&graph));
}

TESTCASE_TESTS(
    tivxNodeApi,
    negativeTestWarpAffineNode,
    negativeTestWarpPerspectiveNode,
    negativeTestRemapNode,
    negativeTesttivxCreateNodeByKernelName,
    negativeTesttivxCreateNodeByStructure,
    negativeTesttivxCreateNodeByStructure1
)