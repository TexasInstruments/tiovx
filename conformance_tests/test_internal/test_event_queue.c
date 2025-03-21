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
/*
 * Copyright (c) 2024 Texas Instruments Incorporated
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

#define INVALID_QUEUE_TYPE -1

TESTCASE(tivxInternalEventQueue, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalEventQueue, negativeTestownRegisterEvent)
{
    vx_context context = context_->vx_context_;
    vx_reference ref2 = ownCreateReference(context, (vx_enum)VX_TYPE_GRAPH, (vx_enum)VX_EXTERNAL, &context->base);
    vx_reference ref3 = ownCreateReference(context, (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_EXTERNAL, &context->base);

    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, ownRegisterEvent(ref2, INVALID_QUEUE_TYPE, VX_EVENT_NODE_COMPLETED, 0, 0, vx_true_e));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, ownRegisterEvent(ref2, INVALID_QUEUE_TYPE, VX_EVENT_NODE_COMPLETED, 0, 0, vx_true_e));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownRegisterEvent(ref3, INVALID_QUEUE_TYPE, VX_EVENT_NODE_COMPLETED, 0, 0, vx_true_e));

    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, ownRegisterEvent(ref2, INVALID_QUEUE_TYPE, VX_EVENT_GRAPH_COMPLETED, 0, 0, vx_true_e));

    ownReleaseReferenceInt(&ref2, (vx_enum)VX_TYPE_GRAPH, (vx_enum)VX_EXTERNAL, NULL);
    ownReleaseReferenceInt(&ref3, (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_EXTERNAL, NULL);

    vx_graph graph = vxCreateGraph(context);
    vx_image input  = vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8);
    vx_image output = vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8);
    vx_node node = vxNotNode(graph, input,  output);
    /* graph param not exisiting at all */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownRegisterEvent((vx_reference)graph, INVALID_QUEUE_TYPE, VX_EVENT_GRAPH_PARAMETER_CONSUMED, 0, 222, vx_true_e));
    vx_parameter p = vxGetParameterByIndex(node, 0);
    vxAddParameterToGraph(graph, p);
    vxReleaseParameter(&p);   
    vx_graph_parameter_queue_params_t graph_param = 
        {.graph_parameter_index = 0, .refs_list = (vx_reference *)&output, .refs_list_size = 1};
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetGraphScheduleConfig(graph, VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO, 1, &graph_param));
    /* wrong queue type */
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, ownRegisterEvent((vx_reference)graph, INVALID_QUEUE_TYPE, VX_EVENT_GRAPH_PARAMETER_CONSUMED, 0, 222, vx_true_e));      
    vxReleaseNode(&node);
    vxReleaseImage(&input);
    vxReleaseImage(&output);
    vxReleaseGraph(&graph);
}

TEST(tivxInternalEventQueue, negativeTestOwnEventQueueAddEvent)
{
    tivx_event_queue_t *event_q = NULL;

    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownEventQueueAddEvent(event_q, 0, 0, 0, 0, 0, 0));
}

TESTCASE_TESTS(tivxInternalEventQueue,
    negativeTestownRegisterEvent,
    negativeTestOwnEventQueueAddEvent
    )
