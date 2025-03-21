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

#if defined(SOC_AM62A)
#define TIVX_TARGET_MCU TIVX_TARGET_MCU1_0
#else
#define TIVX_TARGET_MCU TIVX_TARGET_MCU2_0
#endif

#include <vx_internal.h>

#include "shared_functions.h"

#define VX_KERNEL_CONFORMANCE_TEST_OWN_USER (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 2)
#define VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME "org.khronos.openvx.test.own_user"
typedef enum _own_params_e
{
    OWN_PARAM_INPUT = 0,
    OWN_PARAM_OUTPUT,
} own_params_e;
static vx_status set_local_size_status_init = VX_SUCCESS;
static vx_status set_local_ptr_status_init = VX_SUCCESS;
static vx_size local_size_auto_alloc = 0;
static vx_size local_size_kernel_alloc = 0;
static vx_status query_local_size_status_deinit = VX_SUCCESS;
static vx_status query_local_ptr_status_deinit = VX_SUCCESS;
static vx_status set_local_size_status_deinit = VX_SUCCESS;
static vx_status set_local_ptr_status_deinit = VX_SUCCESS;
static vx_bool is_kernel_called = vx_false_e;
static enum vx_type_e type = VX_TYPE_IMAGE;

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
    ASSERT((vx_enum)VX_ERROR_INVALID_VALUE == ((tivx_error_t *) vxCreateGenericNode(graph, kernel))->status);
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
    tivx_obj_desc_node_t *desc = node->obj_desc[0];
    node->obj_desc[0] = NULL;
    ownNodeClearExecuteState(node, 0);
    node->obj_desc[0] = desc;
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

    uint16_t obj_id = node->obj_desc[0]->base.obj_desc_id;
    node->obj_desc[0]->base.obj_desc_id = TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxRemoveNode(&graph->nodes[0]));
    node->obj_desc[0]->base.obj_desc_id = obj_id;

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
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownNodeCreateUserCallbackCommand(node, 0));

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
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownRegisterEvent(((vx_reference)node), TIVX_EVENT_GRAPH_STREAMING_QUEUE, VX_EVENT_NODE_ERROR, 0, NODE0_COMPLETED_EVENT, vx_true_e));
    ownEventQueueEnableEvents(&node->graph->streaming_event_queue, (vx_bool)vx_false_e);
    ownNodeCheckAndSendErrorEvent(node_obj_desc, 0, (vx_status)node_obj_desc->exe_status);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownRegisterEvent(((vx_reference)node), TIVX_EVENT_GRAPH_QUEUE, VX_EVENT_NODE_ERROR, 0, NODE0_COMPLETED_EVENT, vx_true_e));
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
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownRegisterEvent(((vx_reference)node), TIVX_EVENT_GRAPH_STREAMING_QUEUE, VX_EVENT_NODE_COMPLETED, 0, NODE0_COMPLETED_EVENT, vx_true_e));
    ownEventQueueEnableEvents(&node->graph->streaming_event_queue, (vx_bool)vx_false_e);
    ownNodeCheckAndSendCompletionEvent(node_obj_desc, 0);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownRegisterEvent(((vx_reference)node), TIVX_EVENT_GRAPH_QUEUE, VX_EVENT_NODE_COMPLETED, 0, NODE0_COMPLETED_EVENT, vx_true_e));
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
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownNodeAllocObjDescForPipeline(node, 2));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Testcase to hit failure condition of ownReleaseReferenceInt() called by vxRemoveNode*/
TEST(tivxInternalNode, negativeTestNodeRemoveNode1)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;
    tivx_mutex lock = NULL;

    lock = context->lock;
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    context->lock = NULL;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxRemoveNode(&node));
    context->lock = lock;

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Testcase to hit conditions node->kernel = NULL and num_refs > TIVX_CMD_MAX_OBJ_DESCS.*/
TEST(tivxInternalNode, negativeTestNodeSendCommand)
{
    vx_context context = context_->vx_context_;
    vx_scalar scalar[1];
    vx_node  node[1];
    vx_uint8 scalar_val = 0;
    vx_graph graph;
    vx_kernel kernel;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(scalar[0] = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(node[0] = tivxScalarSourceNode(graph, scalar[0]), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node[0], VX_TARGET_STRING, TIVX_TARGET_MCU));

    VX_CALL(vxVerifyGraph(graph));
    kernel = node[0]->kernel;
    node[0]->kernel = NULL;
    ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxNodeSendCommand(node[0], 0u, 0x01000000u, (vx_reference *)scalar, 1u));
    node[0]->kernel = kernel;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS,tivxNodeSendCommand(node[0], 0u, 0x01000000u, (vx_reference *)scalar, TIVX_CMD_MAX_OBJ_DESCS+1));

    VX_CALL(vxReleaseScalar(&scalar[0]));
    VX_CALL(vxReleaseNode(&node[0]));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);

}

/* Testcase to make vxGetParameterIndex called by vxReplicateNode() to return NULL by failing the lock condition
   and also to hit invalid reference type error by setting node->kernel->signature type to default */
TEST(tivxInternalNode, negativeTestNodeReplicate)
{
    #define VX_TYPE_DEFAULT 0
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownNodeCreateUserCallbackCommand(node, 0));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    vx_bool replicate2[2] = {vx_true_e, vx_true_e};
    node->kernel->signature.types[0] = (vx_enum)VX_TYPE_DEFAULT;
    ASSERT_EQ_VX_STATUS(VX_FAILURE, vxReplicateNode(graph, node, replicate2, 2));
    node->parameters[0] = NULL;
    ASSERT_EQ_VX_STATUS(VX_FAILURE, vxReplicateNode(graph, node, replicate2, 2));
    tivx_mutex lock1 = node->base.context->lock;
    node->base.context->lock = NULL;
    ASSERT_EQ_VX_STATUS(VX_FAILURE, vxReplicateNode(graph, node, replicate2, 2));
    node->base.context->lock = lock1;
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

static vx_status VX_CALLBACK own_Kernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    is_kernel_called = vx_true_e;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);
    EXPECT(parameters != NULL);
    EXPECT(num == 2);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK own_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_size size = 0;
    void *ptr = NULL;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }
    else
    {
        return VX_FAILURE;
    }
    if (local_size_kernel_alloc > 0)
    {
        size = local_size_kernel_alloc;
        ptr = ct_calloc(1, local_size_kernel_alloc);
    }
    set_local_size_status_init = vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_SIZE, &size, sizeof(size));
    set_local_ptr_status_init = vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_PTR, &ptr, sizeof(ptr));
    return VX_SUCCESS;
}

static vx_status VX_CALLBACK own_Deinitialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_size size = 0;
    void *ptr = NULL;
    EXPECT(node != 0);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }
    else
    {
        return VX_FAILURE;
    }
    query_local_size_status_deinit = vxQueryNode(node, VX_NODE_LOCAL_DATA_SIZE, &size, sizeof(size));
    query_local_ptr_status_deinit = vxQueryNode(node, VX_NODE_LOCAL_DATA_PTR, &ptr, sizeof(ptr));
    if (local_size_kernel_alloc > 0)
    {
        size = 0;
        if (ptr != NULL)
        {
            ct_free_mem(ptr);
            ptr = NULL;
        }
    }
    set_local_size_status_deinit = vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_SIZE, &size, sizeof(size));
    set_local_ptr_status_deinit = vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_PTR, &ptr, sizeof(ptr));
    return VX_SUCCESS;
}

static void own_register_kernel(vx_context context, vx_kernel *kernel)
{
    vx_kernel local_kernel = 0;
    vx_enum kernel_id;
    vx_status status;
    vx_size size = local_size_auto_alloc;
    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    ASSERT_VX_OBJECT(local_kernel = vxAddUserKernel(
        context,
        VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME,
        kernel_id,
        own_Kernel,
        2,
        NULL,
        own_Initialize,
        own_Deinitialize), VX_TYPE_KERNEL);

    VX_CALL(vxAddParameterToKernel(local_kernel, OWN_PARAM_INPUT, VX_INPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(local_kernel, OWN_PARAM_INPUT), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_INPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(local_kernel, OWN_PARAM_OUTPUT, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(local_kernel, OWN_PARAM_OUTPUT), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxSetKernelAttribute(local_kernel, VX_KERNEL_LOCAL_DATA_SIZE, &size, sizeof(size)));
    VX_CALL(vxFinalizeKernel(local_kernel));
    *kernel = local_kernel;
}

/* Testcase to fail node->kernel->initialize callback by setting node->kernel->signature.num_parameter = 1 instead of expected value '2'
 * and to make params=NULL by setting parent_ref [ i ] -> type to array object other than the object array or pyramid.
 */
TEST(tivxInternalNode, negativeTestKernelInit)
{
    vx_context context = context_->vx_context_;
    vx_reference src = 0, dst = 0;
    vx_graph graph = 0;
    vx_kernel user_kernel = 0;
    vx_node node = 0;

    vx_enum format = VX_DF_IMAGE_U8;
    vx_uint32 src_width = 128, src_height = 128;
    vx_uint32 dst_width = 256, dst_height = 256;

    ASSERT_VX_OBJECT(src = (vx_reference)vxCreateImage(context, src_width, src_height, format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateImage(context, src_width, src_height, format), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(own_register_kernel(context, &user_kernel));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, user_kernel), VX_TYPE_NODE);

    VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)src));
    VX_CALL(vxSetParameterByIndex(node, 1, (vx_reference)dst));

    node->kernel->signature.num_parameters = 1;

    vx_pyramid src_pyr, dst_pyr;
    vx_reference src_arr;
    vx_image src_image, dst_image;
    vx_enum item_type = VX_TYPE_UINT8;
    vx_size capacity = 20;
    vx_node src_node;

    ASSERT_VX_OBJECT(src_pyr = vxCreatePyramid(context, 2, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(dst_pyr = vxCreatePyramid(context, 2, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_VX_OBJECT(src_arr = (vx_reference)vxCreateArray(context, item_type, capacity), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(src_image = vxGetPyramidLevel((vx_pyramid)src_pyr, 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxGetPyramidLevel((vx_pyramid)dst_pyr, 0), VX_TYPE_IMAGE);

    vx_bool replicate[] = { vx_true_e, vx_true_e };
    ASSERT_VX_OBJECT(src_node = vxBox3x3Node(graph, src_image, dst_image), VX_TYPE_NODE);
    VX_CALL(vxReplicateNode(graph, src_node, replicate, 2));

    vx_reference temp = src_node->parameters[0];
    src_node->parameters[0] = src_arr;

    ASSERT(VX_FAILURE == ownNodeKernelInit(node));

    ASSERT(VX_ERROR_INVALID_PARAMETERS == ownNodeKernelInit(src_node));

    node->kernel->signature.num_parameters = 2;
    src_node->parameters[0] = temp;

    // finalization
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseNode(&src_node));
    VX_CALL(vxReleasePyramid(&src_pyr));
    VX_CALL(vxReleasePyramid(&dst_pyr));
    VX_CALL(vxRemoveKernel(user_kernel));
    VX_CALL(vxReleaseImage(&src_image));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseReference(&src_arr));
    VX_CALL(vxReleaseReference(&dst));
    VX_CALL(vxReleaseReference(&src));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Testcase to fail node->kernel->deinitialize callback by setting node->kernel->signature.num_parameter = 1
 * instead of expected value '2' and to hit target kernel deinitialision when replicate flags are set.
 */
TEST(tivxInternalNode, negativeTestKernelDeinit)
{
#if defined(BUILD_CORE_KERNELS)
    vx_context context = context_->vx_context_;
    vx_reference src = 0, dst = 0;
    vx_graph graph = 0;
    vx_kernel user_kernel = 0;
    vx_node node = 0;

    vx_enum format = VX_DF_IMAGE_U8;
   	vx_uint32 src_width = 128, src_height = 128;
    vx_uint32 dst_width = 256, dst_height = 256;

	ASSERT_VX_OBJECT(src = (vx_reference)vxCreateImage(context, src_width, src_height, format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateImage(context, src_width, src_height, format), VX_TYPE_IMAGE);

	ASSERT_NO_FAILURE(own_register_kernel(context, &user_kernel));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, user_kernel), VX_TYPE_NODE);

    VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)src));
    VX_CALL(vxSetParameterByIndex(node, 1, (vx_reference)dst));

    ASSERT(VX_SUCCESS == ownNodeKernelInit(node));

    node->kernel->signature.num_parameters = 1;
    vx_pyramid src_pyr;
    vx_image src_image, dst_image;
    vx_node src_node;
    vx_object_array obj_arr;
    vx_image image;

    ASSERT_VX_OBJECT(src_pyr = vxCreatePyramid(context, 2, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(obj_arr = vxCreateObjectArray(context, (vx_reference)image, 2), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(dst_image = (vx_image)vxGetObjectArrayItem(obj_arr, 0), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src_image = vxGetPyramidLevel((vx_pyramid)src_pyr, 0), VX_TYPE_IMAGE);

    vx_bool replicate[] = { vx_true_e, vx_true_e };
    ASSERT_VX_OBJECT(src_node = vxBox3x3Node(graph, src_image, dst_image), VX_TYPE_NODE);
    VX_CALL(vxReplicateNode(graph, src_node, replicate, 2));

    ASSERT(VX_SUCCESS == ownNodeKernelInit(src_node));
    ASSERT(VX_FAILURE == ownNodeKernelDeinit(node));

    src_node->kernel->deinitialize = node->kernel->deinitialize;

    ASSERT(VX_SUCCESS == ownNodeKernelDeinit(src_node));
    src_node->kernel->deinitialize = NULL;

    node->kernel->signature.num_parameters = 2;

    // finalization
    VX_CALL(vxReleaseNode(&src_node));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseObjectArray(&obj_arr));
    VX_CALL(vxReleasePyramid(&src_pyr));
	VX_CALL(vxRemoveKernel(user_kernel));
    VX_CALL(vxReleaseImage(&src_image));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseReference(&dst));
    VX_CALL(vxReleaseReference(&src));
    VX_CALL(vxReleaseGraph(&graph));
#endif /* #if defined(BUILD_CORE_KERNELS) */
}

TEST(tivxInternalNode, TestBranchownNodeGetNextNode)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    node->obj_desc[0]->num_out_nodes = 3;
    node->obj_desc[0]->out_node_id[0] =  TIVX_OBJ_DESC_INVALID;
    ASSERT(NULL == ownNodeGetNextNode(node, 0));

    node->obj_desc[0]->num_in_nodes = 3;
    node->obj_desc[0]->in_node_id[0] =  TIVX_OBJ_DESC_INVALID;
    ASSERT(NULL == ownNodeGetNextInNode(node, 0));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalNode, TestBranchownNodeUserKernelExecute)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    kernel = node->kernel;
    node->kernel = NULL;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownNodeUserKernelExecute(node, NULL));
    node->kernel = kernel;
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
    negativeTestNodeAllocObjDescForPipeline,
    negativeTestNodeRemoveNode1,
    negativeTestNodeSendCommand,
    negativeTestNodeReplicate,
    negativeTestKernelInit,
    negativeTestKernelDeinit,
    TestBranchownNodeGetNextNode,
    TestBranchownNodeUserKernelExecute
    )