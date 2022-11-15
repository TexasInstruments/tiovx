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

#include <math.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_khr_pipelining.h>
#include <TI/tivx_queue.h>
#include <TI/tivx_config.h>

#include "test_engine/test.h"

TESTCASE(tivxEventQueue, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxEventQueue, negativeTestEnableEvents)
{
    vx_context context = context_->vx_context_;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxEnableEvents(NULL));
}

TEST(tivxEventQueue, negativeTestDisableEvents)
{
    vx_context context = context_->vx_context_;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxDisableEvents(NULL));
}

TEST(tivxEventQueue, negativeTestSendUserEvent)
{
    vx_context context = context_->vx_context_;

    vx_uint32 appvalue = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSendUserEvent(NULL, appvalue, NULL));
}

TEST(tivxEventQueue, negativeTestWaitEvent)
{
    vx_context context = context_->vx_context_;

    vx_event_t event;
    vx_bool dnb = 0;
    vx_uint32 appvalue = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxWaitEvent(NULL, &event, dnb));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSendUserEvent(context, appvalue, NULL));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitEvent(context, NULL, dnb));
}

TESTCASE_TESTS(
    tivxEventQueue,
    negativeTestEnableEvents,
    negativeTestDisableEvents,
    negativeTestSendUserEvent,
    negativeTestWaitEvent
)

