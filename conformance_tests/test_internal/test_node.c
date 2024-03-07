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


TESTCASE(tivxInternalNode, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalNode, negativeTestSetNodeParameterNumBufByIndex)
{
    uint32_t get_num_buf = TIVX_OBJ_DESC_QUEUE_MAX_DEPTH;
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d1, d2;
    vx_node  n1;
    uint32_t width = 64;
    uint32_t height = 48;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(d1    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d2    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1, d2), VX_TYPE_NODE);

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS,(tivxSetNodeParameterNumBufByIndex(n1, 1, get_num_buf))); // for number of buffers condition
    get_num_buf = 1; 
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS,(tivxSetNodeParameterNumBufByIndex(n1, 0, get_num_buf))); // for parameter directions condition
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseImage(&d2));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalNode, negativeTestNodeGetParameterNumBuf)
{
    vx_uint32 idx = 1;
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image d1, d2;
    vx_node  n1;
    uint32_t width = 64;
    uint32_t height = 48;

    ASSERT(0 == ownNodeGetParameterNumBuf(NULL, 1));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(d1    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d2    = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(n1    = vxNotNode(graph, d1, d2), VX_TYPE_NODE);

    ASSERT(0 == ownNodeGetParameterNumBuf(n1, 100)); //more number of parameters condition
    ASSERT(0 == ownNodeGetParameterNumBuf(n1, 0)); //parameter direction condition
    VX_CALL(vxReleaseNode(&n1));    
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseImage(&d2));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalNode, negativeTestInitNodeObjDesc)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    kernel->num_targets = 0;
    ASSERT(NULL == vxCreateGenericNode(graph, kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalNode, negativeTestAddInOutNode)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL, node1 = NULL;
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node1 = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    node->obj_desc[0]->num_in_nodes = TIVX_NODE_MAX_IN_NODES;
    ASSERT(VX_ERROR_NO_RESOURCES == ownNodeAddInNode(node, node1));
    node->obj_desc[0]->num_out_nodes = TIVX_NODE_MAX_OUT_NODES;
    ASSERT(VX_ERROR_NO_RESOURCES == ownNodeAddOutNode(node, node1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph));
}

TESTCASE_TESTS(tivxInternalNode,
    negativeTestSetNodeParameterNumBufByIndex,
    negativeTestNodeGetParameterNumBuf,
    negativeTestInitNodeObjDesc,
    negativeTestAddInOutNode
    )