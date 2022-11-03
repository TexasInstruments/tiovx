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

#include <VX/vx_khr_pipelining.h>

#include "test_engine/test.h"

TESTCASE(tivxStreamGraph, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxStreamGraph, negativeTestEnableGraphStreaming)
{
    vx_context context = context_->vx_context_;

    vx_node node1 = NULL, node2 = NULL;
    vx_graph graph1 = NULL, graph2 = NULL;
    vx_kernel kernel1 = NULL, kernel2 = NULL;

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel1 = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node1 = vxCreateGenericNode(graph1, kernel1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel2 = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node2 = vxCreateGenericNode(graph2, kernel2), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxEnableGraphStreaming(graph1, node2));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseKernel(&kernel2));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseKernel(&kernel1));
    VX_CALL(vxReleaseGraph(&graph1));
}

TESTCASE_TESTS(
    tivxStreamGraph,
    negativeTestEnableGraphStreaming
)

