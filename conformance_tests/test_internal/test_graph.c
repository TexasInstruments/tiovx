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


TESTCASE(tivxInternalGraph, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalGraph, negativeTestOwnUpdateGraphPerf)
{
    #define VX_GRAPH_DEFAULT 0

    vx_context context = context_->vx_context_;
    vx_graph graph;
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    EXPECT(VX_ERROR_INVALID_PARAMETERS == ownUpdateGraphPerf(graph, (graph->pipeline_depth+1)));
    vx_enum temp = (((vx_reference)graph)->type);
    (((vx_reference)graph)->type) = VX_GRAPH_DEFAULT;
    EXPECT(VX_ERROR_INVALID_PARAMETERS == ownUpdateGraphPerf(graph, (graph->pipeline_depth+1)));
    (((vx_reference)graph)->type) = temp;
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalGraph, negativeTestOwnGraphAddNode)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node node;
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    node = (vx_node)ownCreateReference(graph->base.context, (vx_enum)VX_TYPE_NODE, (vx_enum)VX_EXTERNAL, &graph->base);
    ASSERT(VX_ERROR_INVALID_PARAMETERS == ownGraphAddNode(graph, node, (int32_t)TIVX_GRAPH_MAX_NODES));
    ASSERT(VX_ERROR_INVALID_PARAMETERS == ownGraphAddNode(graph, node, 1));
    VX_CALL(ownReleaseReferenceInt((vx_reference*)&node, (vx_enum)VX_TYPE_NODE, (vx_enum)VX_EXTERNAL, NULL));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(VX_ERROR_INVALID_REFERENCE == ownGraphAddNode(graph, node, (int32_t)TIVX_GRAPH_MAX_NODES));
}

TEST(tivxInternalGraph, negativeTestOwnGraphRemoveNode)
{
    vx_graph graph = NULL;
    vx_node node = NULL;
    ASSERT(VX_ERROR_INVALID_REFERENCE == ownGraphRemoveNode(graph, node));
}

TEST(tivxInternalGraph, negativeTestOwnGraphSetReverify)
{
    vx_graph graph = NULL;
    ownGraphSetReverify(graph);
}

TEST(tivxInternalGraph, negativeTestOwnSendGraphCompletedEvent)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    ownSendGraphCompletedEvent(graph);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    vx_context temp = graph->base.context;
    graph->base.context = NULL;
    ownSendGraphCompletedEvent(graph);
    graph->base.context = temp;
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalGraph, negativeTestOwnGraphRegisterCompletionEvent)
{
    vx_graph graph =NULL;
    ASSERT(VX_ERROR_INVALID_REFERENCE == ownGraphRegisterCompletionEvent(graph, 1));
}

TEST(tivxInternalGraph, negativeTestOwnGraphRegisterParameterConsumedEvent)
{
    #define CONSUMED_EVENT_DEFAULT 0
    vx_context context = context_->vx_context_;
    vx_graph graph =NULL;
    ASSERT(VX_ERROR_INVALID_REFERENCE == ownGraphRegisterParameterConsumedEvent(graph, 0, CONSUMED_EVENT_DEFAULT));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT(VX_ERROR_INVALID_PARAMETERS == ownGraphRegisterParameterConsumedEvent(graph, 0, CONSUMED_EVENT_DEFAULT));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalGraph, negativeTestOwnSetGraphState)
{
    #define PIPELINE_ID 0
    #define GRAPH_STATE 0 
    vx_context context = context_->vx_context_;
    vx_graph graph =NULL;
    uint32_t id;
    ownSetGraphState(graph, PIPELINE_ID, GRAPH_STATE);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    id = graph->pipeline_depth;
    ownSetGraphState(graph, id, GRAPH_STATE);
    ownSetGraphState(graph, id-1, GRAPH_STATE);
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalGraph, negativeTestGraphGetNode)
{
    vx_context context = context_->vx_context_;
    vx_graph graph =NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    vx_bool temp_val = graph->verified;
    graph->verified = (vx_bool)vx_true_e;
    uint32_t temp = graph->num_nodes;
    graph->num_nodes = 1;
    ASSERT(NULL == tivxGraphGetNode(graph, 0));
    graph->verified = temp_val;
    graph->num_nodes = temp;
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalGraph, negativeTestOwnGraphGetFreeNodeIndex)
{
    vx_graph graph =NULL;
    ASSERT(ownGraphGetFreeNodeIndex(graph) < 0);
}

TEST(tivxInternalGraph, negativeTestOwnGraphScheduleGraphWrapper)
{
    vx_context context = context_->vx_context_;
    vx_graph graph =NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    graph->verified = (vx_bool)vx_true_e;
    graph->state = (vx_enum)VX_GRAPH_STATE_VERIFIED;
    graph->schedule_mode =(vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL;
    graph->is_streaming_enabled = (vx_bool)vx_true_e;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownGraphScheduleGraphWrapper(graph));

    /* To hit the else part by passing schedule mode other than VX_GRAPH_SCHEDULE_MODE_NORMAL or VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL */
    graph->schedule_mode =(vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownGraphScheduleGraphWrapper(graph));

    graph->verified = (vx_bool)vx_false_e;
    graph->state = (vx_enum)VX_GRAPH_STATE_UNVERIFIED;
    graph->schedule_mode = (vx_enum)VX_GRAPH_SCHEDULE_MODE_NORMAL;
    graph->is_streaming_enabled   = (vx_bool)vx_false_e;

    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalGraph, negativetestOwnGraphRemoveNode1)
{
    vx_context context = context_->vx_context_;
    vx_graph graph =NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* To fail 'i < graph->num_nodes' for-loop condition */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownGraphRemoveNode(graph, NULL));

    VX_CALL(vxReleaseGraph(&graph));
}

TESTCASE_TESTS(tivxInternalGraph,
    negativeTestOwnUpdateGraphPerf,
    negativeTestOwnGraphAddNode,
    negativeTestOwnGraphRemoveNode,
    negativeTestOwnGraphSetReverify,
    negativeTestOwnSendGraphCompletedEvent,
    negativeTestOwnGraphRegisterCompletionEvent,
    negativeTestOwnGraphRegisterParameterConsumedEvent,
    negativeTestOwnSetGraphState,
    negativeTestGraphGetNode,
    negativeTestOwnGraphGetFreeNodeIndex,
    negativeTestOwnGraphScheduleGraphWrapper,
    negativetestOwnGraphRemoveNode1
    )