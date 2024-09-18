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
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownDataRefQueueSendRefConsumedEvent(NULL, 0));
}

TEST(tivxInternalDataRefQueue, negativeTestOwnDataRefQueueDataRefQueueEnqueueReadyRef)
{
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownDataRefQueueEnqueueReadyRef(NULL, NULL));
}

TEST(tivxInternalDataRefQueue, negativeTestOwnDataRefQueueDequeueDoneRef)
{
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownDataRefQueueDequeueDoneRef(NULL, NULL));
}

TEST(tivxInternalDataRefQueue, negativeTestOwnDataRefQueueWaitDoneRef)
{
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownDataRefQueueWaitDoneRef(NULL, 0));
}

TEST(tivxInternalDataRefQueue, negativeTestOwnDataRefQueueGetDoneQueueCount)
{
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownDataRefQueueGetDoneQueueCount(NULL, NULL));
}

TESTCASE_TESTS(
    tivxInternalDataRefQueue,
    negativeTestOwnDataRefQueueGetReadyQueueCount,
    negativeTestTivxDataRefQueueCreate,
    negativeTestOwnDataRefQueueSendRefConsumedEvent,
    negativeTestOwnDataRefQueueDataRefQueueEnqueueReadyRef,
    negativeTestOwnDataRefQueueDequeueDoneRef,
    negativeTestOwnDataRefQueueWaitDoneRef,
    negativeTestOwnDataRefQueueGetDoneQueueCount
)