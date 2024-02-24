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
 * Copyright (c) 2024 Texas Instruments Incorporated
 */
#include <vx_internal.h>
#include <tivx_objects.h>

#include "test_tiovx.h"
#include "test_engine/test.h"
#include "test_tiovx/test_tiovx.h"
#include <VX/vx.h>
#include <VX/vxu.h>

TESTCASE(tivxInternalGraphStream, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalGraphStream, negativeTestGraphEvent)
{
    vx_graph graph = NULL;
    vx_event_t event;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, tivxWaitGraphEvent(graph, &event, (vx_bool)vx_false_e));
}

TEST(tivxInternalGraphStream, negativeTestGraphVerfiyStreamingNode)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownGraphVerifyStreamingMode(graph));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    vx_bool temp = graph->is_streaming_enabled;
    graph->is_streaming_enabled = (vx_bool)vx_true_e;
    vx_enum mode = graph->schedule_mode;
    graph->schedule_mode = (vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL;
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, ownGraphVerifyStreamingMode(graph));
    graph->schedule_mode = (vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO;
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, ownGraphVerifyStreamingMode(graph));
    graph->is_streaming_enabled = temp;
    graph->schedule_mode = mode;
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalGraphStream, negativeTestGraphAllocForStreaming)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;

    ASSERT(VX_ERROR_INVALID_REFERENCE == ownGraphAllocForStreaming(graph));
}

TESTCASE_TESTS(
    tivxInternalGraphStream,
    negativeTestGraphEvent,
    negativeTestGraphVerfiyStreamingNode,
    negativeTestGraphAllocForStreaming
)

