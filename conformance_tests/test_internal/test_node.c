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

TEST(tivxInternalNode, negativeTestGetNextNode)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;

    ASSERT(NULL == ownNodeGetNextNode(node, 0));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    tivx_obj_desc_node_t *desc = node->obj_desc[0];
    node->obj_desc[0] = NULL;
    ASSERT(NULL == ownNodeGetNextNode(node, 0));
    node->obj_desc[0] = desc;
    node->obj_desc[0]->num_out_nodes = 0;
    ASSERT(NULL == ownNodeGetNextNode(node, 0));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalNode, negativeTestGetNextInNode)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;

    ASSERT(NULL == ownNodeGetNextInNode(node, 0));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    tivx_obj_desc_node_t *desc = node->obj_desc[0];
    node->obj_desc[0] = NULL;
    ASSERT(NULL == ownNodeGetNextInNode(node, 0));
    node->obj_desc[0] = desc;
    node->obj_desc[0]->num_in_nodes = 0;
    ASSERT(NULL == ownNodeGetNextInNode(node, 0));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalNode, negativeTestNodeClearExecuteState)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;

    ownNodeClearExecuteState(node, 0);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    ownNodeClearExecuteState(node, TIVX_GRAPH_MAX_PIPELINE_DEPTH);
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalNode, negativeTestNodeIsPrmReplicated)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;

    ASSERT(vx_false_e == ownNodeIsPrmReplicated(node, 0));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    tivx_obj_desc_node_t *desc = node->obj_desc[0];
    node->obj_desc[0] = NULL;
    ASSERT(vx_false_e == ownNodeIsPrmReplicated(node, 0));
    node->obj_desc[0] = desc;

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalNode, negativeTestDestructNode)
{ 
    #define VX_DEFAULT 0
    vx_context context = context_->vx_context_;
    vx_delay   src_delay;
    vx_graph   graph = 0;
    vx_image   image, image1;
    vx_node    box_node;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_delay = vxCreateDelay(context, (vx_reference)image, 2), VX_TYPE_DELAY);
    ASSERT_VX_OBJECT(box_node = vxBox3x3Node(graph, image1, (vx_image)vxGetReferenceFromDelay(src_delay, 0)), VX_TYPE_NODE);

    VX_CALL(vxReleaseDelay(&src_delay));
    VX_CALL(vxReleaseImage(&image1));

    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseNode(&box_node));

    vx_reference value = ((graph->nodes[0])->parameters[1]);
    vx_bool res = ownRemoveAssociationToDelay(value, (graph->nodes[0]), 1);
    ASSERT(res == (vx_bool)vx_true_e);
    VX_CALL(vxReleaseGraph(&graph));

}

TEST(tivxInternalNode, negativeTestNodeRemoveNode)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    vx_graph g1 = node->graph;
    node->graph = NULL;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxRemoveNode(&node));
    node->graph = g1;
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalNode, negativeTestNodeCreateUserCallbackCommand)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownNodeCreateUserCallbackCommand(node, 0));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownNodeCreateUserCallbackCommand(node, TIVX_GRAPH_MAX_PIPELINE_DEPTH));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalNode, negativeTestNodeSetParameter)
{
    #define NODE0_COMPLETED_EVENT     (2u)
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;
    vx_reference src = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(src = (vx_reference)vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)src));
    vx_reference tmp = (vx_reference)(node->parameters[0]);
    uint32_t tmp_magic = tmp->magic;
    tmp->magic = TIVX_BAD_MAGIC;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownNodeSetParameter(node, 0 , (vx_reference)src));
    tmp->magic = tmp_magic;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownNodeSetParameter(node, 0, (vx_reference)src));

    VX_CALL(vxReleaseReference(&src));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalNode, negativeTestNodeCheckAndSendErrorEvent)
{
    #define NODE0_COMPLETED_EVENT     (2u)
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;
    vx_reference src = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    VX_CALL(vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, NODE0_COMPLETED_EVENT));
    VX_CALL(vxDisableEvents(((&(node->base))->context)));
    tivx_obj_desc_node_t *node_obj_desc = (tivx_obj_desc_node_t *)node->obj_desc[0];
    ownNodeCheckAndSendErrorEvent(node_obj_desc, 0, (vx_status)node_obj_desc->exe_status);

    VX_CALL(vxEnableEvents(((&(node->base))->context)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownRegisterEvent(((vx_reference)node), TIVX_EVENT_GRAPH_QUEUE, VX_EVENT_NODE_ERROR, 0, NODE0_COMPLETED_EVENT));
    ownEventQueueEnableEvents(&node->graph->event_queue, (vx_bool)vx_false_e);
    ownNodeCheckAndSendErrorEvent(node_obj_desc, 0, (vx_status)node_obj_desc->exe_status);

    node_obj_desc->base.host_ref = 0;
    ownNodeCheckAndSendErrorEvent(node_obj_desc, 0, (vx_status)node_obj_desc->exe_status);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalNode, negativeTestNodeCheckAndSendCompletionEvent)
{
    #define NODE0_COMPLETED_EVENT     (2u)
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;
    vx_reference src = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    VX_CALL(vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_COMPLETED, 0, NODE0_COMPLETED_EVENT));
    VX_CALL(vxDisableEvents(((&(node->base))->context)));
    tivx_obj_desc_node_t *node_obj_desc = (tivx_obj_desc_node_t *)node->obj_desc[0];
    ownNodeCheckAndSendCompletionEvent(node_obj_desc, 0);

    VX_CALL(vxEnableEvents(((&(node->base))->context)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownRegisterEvent(((vx_reference)node), TIVX_EVENT_GRAPH_QUEUE, VX_EVENT_NODE_COMPLETED, 0, NODE0_COMPLETED_EVENT));
    ownEventQueueEnableEvents(&node->graph->event_queue, (vx_bool)vx_false_e);
    ownNodeCheckAndSendCompletionEvent(node_obj_desc, 0);

    node_obj_desc->base.host_ref = 0;
    ownNodeCheckAndSendCompletionEvent(node_obj_desc, 0);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalNode, negativeTestNodeReplaceOut)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL, node1 = NULL, node2 = NULL, node3 = NULL, node4 = NULL;
    vx_image image1, image2;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);

    // Creating a node
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    // Creating a new node
    ASSERT_VX_OBJECT(node1 = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    // Creating three nodes for output node
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, image1, image2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxNotNode(graph, image1, image2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxNotNode(graph, image1, image2), VX_TYPE_NODE);

    // Adding output nodes to node
    ASSERT(VX_SUCCESS == ownNodeAddOutNode(node, node2));
    ASSERT(VX_SUCCESS == ownNodeAddOutNode(node, node3));
    ASSERT(VX_SUCCESS == ownNodeAddOutNode(node, node4));
    ASSERT(VX_SUCCESS == ownNodeReplaceOutNode(node, node2, node1));
    ASSERT(VX_SUCCESS == ownNodeReplaceOutNode(node, node3, node1));

    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalNode, negativeTestNodeReplaceIn)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL, node1 = NULL, node2 = NULL, node3 = NULL, node4 = NULL;
    vx_image image1, image2;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);

    // Creating a node
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    // Creating a new node
    ASSERT_VX_OBJECT(node1 = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    // Creating three nodes for input node
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, image1, image2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxNotNode(graph, image1, image2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxNotNode(graph, image1, image2), VX_TYPE_NODE);

    // Adding input nodes to node
    ASSERT(VX_SUCCESS == ownNodeAddInNode(node, node2));
    ASSERT(VX_SUCCESS == ownNodeAddInNode(node, node3));
    ASSERT(VX_SUCCESS == ownNodeAddInNode(node, node4));
    ASSERT(VX_SUCCESS == ownNodeReplaceInNode(node, node2, node1));
    ASSERT(VX_SUCCESS == ownNodeReplaceInNode(node, node3, node1));

    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseGraph(&graph));
}


TEST(tivxInternalNode, negativeTestNodeAllocObjDescForPipeline)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;
    vx_reference src = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    node->pipeline_depth = 0;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownNodeAllocObjDescForPipeline(node, 2));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TESTCASE_TESTS(tivxInternalNode,
    negativeTestSetNodeParameterNumBufByIndex,
    negativeTestNodeGetParameterNumBuf,
    negativeTestInitNodeObjDesc,
    negativeTestAddInOutNode,
    negativeTestGetNextNode,
    negativeTestGetNextInNode,
    negativeTestNodeClearExecuteState,
    negativeTestNodeIsPrmReplicated,
    negativeTestDestructNode,
    negativeTestNodeRemoveNode,
    negativeTestNodeCreateUserCallbackCommand,
    negativeTestNodeSetParameter,
    negativeTestNodeCheckAndSendErrorEvent,
    negativeTestNodeCheckAndSendCompletionEvent,
    negativeTestNodeReplaceOut,
    negativeTestNodeReplaceIn,
    negativeTestNodeAllocObjDescForPipeline
    )