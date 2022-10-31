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

#include "test_engine/test.h"

TESTCASE(tivxNode, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxNode, negativeTestCreateGenericNode)
{
    vx_context context = context_->vx_context_;

    vx_node node = NULL;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;

    ASSERT(NULL == (node = vxCreateGenericNode(graph, kernel)));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    EXPECT_VX_ERROR(node = vxCreateGenericNode(graph, kernel), VX_ERROR_INVALID_REFERENCE);
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxNode, negativeTestQueryNode)
{
    #define VX_NODE_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_node node = NULL;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_enum attribute = VX_NODE_DEFAULT;
    vx_uint32 udata = 0, np = 0;
    vx_size size = 0;
    vx_bool rflags[5] = {0};

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryNode(node, attribute, &udata, size));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryNode(node, VX_NODE_PERFORMANCE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryNode(node, VX_NODE_STATUS, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryNode(node, VX_NODE_STATUS, &udata, sizeof(vx_status)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryNode(node, VX_NODE_LOCAL_DATA_SIZE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryNode(node, VX_NODE_LOCAL_DATA_PTR, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryNode(node, VX_NODE_BORDER, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryNode(node, VX_NODE_PARAMETERS, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryNode(node, VX_NODE_IS_REPLICATED, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryNode(node, VX_NODE_IS_REPLICATED, &udata, sizeof(vx_bool)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryNode(node, VX_NODE_REPLICATE_FLAGS, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryNode(node, VX_NODE_PARAMETERS, &np, sizeof(vx_uint32)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryNode(node, VX_NODE_REPLICATE_FLAGS, rflags, (np * sizeof(vx_bool))));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryNode(node, VX_NODE_VALID_RECT_RESET, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryNode(node, TIVX_NODE_TARGET_STRING, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryNode(node, TIVX_NODE_TARGET_STRING, NULL, TIVX_TARGET_MAX_NAME));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryNode(node, VX_NODE_STATE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryNode(node, TIVX_NODE_TIMEOUT, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryNode(node, attribute, &udata, size));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxNode, negativeTestSetNodeAttribute)
{
    #define VX_NODE_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_node node = NULL;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_enum attribute = VX_NODE_DEFAULT;
    vx_uint32 udata = 0;
    vx_size size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetNodeAttribute(node, attribute, &udata, size));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_SIZE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_PTR, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetNodeAttribute(node, VX_NODE_BORDER, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetNodeAttribute(node, TIVX_NODE_TIMEOUT, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetNodeAttribute(node, attribute, &udata, size));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxNode, negativeTestRemoveNode)
{
    vx_context context = context_->vx_context_;

    vx_node node = NULL;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxRemoveNode(&node));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxRemoveNode(NULL));
}

static vx_action VX_CALLBACK tivxNode_Callback(vx_node node) {

    return VX_ACTION_CONTINUE;
}

TEST(tivxNode, negativeTestAssignNodeCallback)
{
    vx_context context = context_->vx_context_;

    vx_node node = NULL;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxAssignNodeCallback(node, tivxNode_Callback));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxAssignNodeCallback(node, tivxNode_Callback));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxAssignNodeCallback(node, tivxNode_Callback));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxNode, negativeTestRetrieveNodeCallback)
{
    vx_context context = context_->vx_context_;

    vx_node node = NULL;

    ASSERT(NULL == vxRetrieveNodeCallback(node));
}

TEST(tivxNode, negativeTestReplicateNode)
{
    vx_context context = context_->vx_context_;

    vx_node node = NULL;
    vx_graph graph1 = NULL, graph2 = NULL;
    vx_kernel kernel = NULL;
    vx_bool replicate[2] = {vx_false_e, vx_false_e};
    vx_uint32 nop = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxReplicateNode(graph1, node, replicate, nop));
    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxReplicateNode(graph1, node, replicate, nop));
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph1, kernel), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_FAILURE, vxReplicateNode(graph2, node, replicate, nop));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxReplicateNode(graph1, node, NULL, nop));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxReplicateNode(graph1, node, replicate, nop));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryNode(node, VX_NODE_PARAMETERS, &nop, sizeof(vx_uint32)));
    ASSERT_EQ_VX_STATUS(VX_FAILURE, vxReplicateNode(graph1, node, replicate, nop));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseGraph(&graph1));
}

TEST(tivxNode, negativeTestSetNodeTarget)
{
    vx_context context = context_->vx_context_;

    vx_node node = NULL;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_enum target = VX_TARGET_VENDOR_BEGIN;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetNodeTarget(node, target, "ABC"));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetNodeTarget(node, VX_TARGET_STRING, "ABC"));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetNodeTarget(node, target, "ABC"));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TESTCASE_TESTS(
    tivxNode,
    negativeTestCreateGenericNode,
    negativeTestQueryNode,
    negativeTestSetNodeAttribute,
    negativeTestRemoveNode,
    negativeTestAssignNodeCallback,
    negativeTestRetrieveNodeCallback,
    negativeTestReplicateNode,
    negativeTestSetNodeTarget
)

