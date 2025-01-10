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

#include <vx_context.h>

TESTCASE(tivxInternalDataRefQueue, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalDataRefQueue, negativeTestOwnDataRefQueueGetReadyQueueCount)
{
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownDataRefQueueGetReadyQueueCount(NULL, NULL));
}

TEST(tivxInternalDataRefQueue, negativeTestTivxDataRefQueueCreate)
{
    ASSERT_EQ_PTR(NULL, tivxDataRefQueueCreate(NULL, NULL));
}

TEST(tivxInternalDataRefQueue, negativeTestOwnDataRefQueueSendRefConsumedEvent)
{
    vx_context context = context_->vx_context_;
    tivx_data_ref_queue ref = NULL;
    uint32_t pipeline_id = 0u;
    vx_graph graph = NULL;
    tivx_data_ref_queue_create_params_t prms;
    prms.pipeline_depth = 1;
    tivx_event wait_done_ref_available_event_local;
    vx_bool is_enable_send_ref_consumed_event_local;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownDataRefQueueSendRefConsumedEvent(NULL, 0));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(ref = tivxDataRefQueueCreate(graph, &prms), (vx_enum)TIVX_TYPE_DATA_REF_Q);

    wait_done_ref_available_event_local = ref->wait_done_ref_available_event;
    is_enable_send_ref_consumed_event_local = ref->is_enable_send_ref_consumed_event;
    ref->wait_done_ref_available_event = NULL;
    ref->is_enable_send_ref_consumed_event = 0;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownDataRefQueueSendRefConsumedEvent(ref, 0));
    ref->wait_done_ref_available_event = wait_done_ref_available_event_local;
    ref->is_enable_send_ref_consumed_event = is_enable_send_ref_consumed_event_local;

    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(ownDataRefQueueRelease(&ref));
}

TEST(tivxInternalDataRefQueue, negativeTestOwnDataRefQueueWaitDoneRef)
{
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownDataRefQueueWaitDoneRef(NULL, 0));
}

TEST(tivxInternalDataRefQueue, negativeTestOwnDataRefQueueGetDoneQueueCount)
{
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownDataRefQueueGetDoneQueueCount(NULL, NULL));
}

TEST(tivxInternalDataRefQueue, negativeTestownDataRefQueueGetObjDescId)
{
    vx_context context = context_->vx_context_;
    tivx_data_ref_queue ref = NULL;
    uint32_t pipeline_id = 0u;
    vx_graph graph = NULL;
    tivx_data_ref_queue_create_params_t prms;
    prms.pipeline_depth = 1;

    ASSERT_EQ_VX_STATUS(TIVX_OBJ_DESC_INVALID, ownDataRefQueueGetObjDescId(ref, pipeline_id));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(ref = tivxDataRefQueueCreate(graph, &prms), (vx_enum)TIVX_TYPE_DATA_REF_Q);

    pipeline_id = 0;
    ref->pipeline_depth = 0;
    ASSERT_EQ_VX_STATUS(TIVX_OBJ_DESC_INVALID, ownDataRefQueueGetObjDescId(ref, pipeline_id));

    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(ownDataRefQueueRelease(&ref));
}

TESTCASE_TESTS(
    tivxInternalDataRefQueue,
    negativeTestOwnDataRefQueueGetReadyQueueCount,
    negativeTestTivxDataRefQueueCreate,
    negativeTestOwnDataRefQueueSendRefConsumedEvent,
    negativeTestOwnDataRefQueueWaitDoneRef,
    negativeTestOwnDataRefQueueGetDoneQueueCount,
    negativeTestownDataRefQueueGetObjDescId
)