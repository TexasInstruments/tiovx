/*

 * Copyright (c) 2012-2020 The Khronos Group Inc.
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
 *
 * Copyright (c) 2024 Texas Instruments Incorporated
 *
 */
#if defined(A72) || defined(A53) || defined(PC)
#include "test_tiovx.h"
#include <TI/tivx_event.h>
#include <tivx_platform_posix.h>

#define INVALID_TYPE -1

TESTCASE(tivxPosixObjects, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxPosixObjects, testPosixObjectFree)
{
    tivx_event *event = NULL;
    tivx_mutex *mutex = NULL;

    event = (tivx_event *)ct_alloc_mem(sizeof(tivx_event));
    mutex = (tivx_mutex *)ct_alloc_mem(sizeof(tivx_mutex));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventCreate(event));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxMutexCreate(mutex));

    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownPosixObjectFree((uint8_t *)event, TIVX_POSIX_TYPE_MUTEX));
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownPosixObjectFree((uint8_t *)event, TIVX_POSIX_TYPE_QUEUE));
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownPosixObjectFree((uint8_t *)event, TIVX_POSIX_TYPE_TASK));
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownPosixObjectFree((uint8_t *)mutex, TIVX_POSIX_TYPE_EVENT));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownPosixObjectFree((uint8_t *)event, INVALID_TYPE));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventDelete(event));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxMutexDelete(mutex));

    ct_free_mem((void *)event);
    ct_free_mem((void *)mutex);
}

TEST(tivxPosixObjects, negativeTestPosixObjectAlloc)
{
    ASSERT(NULL == ownPosixObjectAlloc(INVALID_TYPE));
}

TEST(tivxPosixObjects, negativeTestPosixObjectFree)
{
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownPosixObjectFree(NULL, INVALID_TYPE));
}

TESTCASE_TESTS(tivxPosixObjects,
               testPosixObjectFree,
               negativeTestPosixObjectAlloc,
               negativeTestPosixObjectFree
)
#endif