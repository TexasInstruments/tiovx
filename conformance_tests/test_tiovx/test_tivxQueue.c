/*

* Copyright (c) 2012-2020 The Khronos Group Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
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

#include <tivx_utils_ipc_ref_xfer.h>
#include <TI/tivx_queue.h>
#include "test_tiovx.h"

TESTCASE(tivxQueue, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxQueue, testQueueCreatePutGetDelete)
{

    vx_status status = VX_SUCCESS;
    tivx_queue test_queue[10];
    uintptr_t test_qu_mem;
    tivx_queue *queue_t = &test_queue[0];
    uintptr_t *queue_mem_t=&test_qu_mem;
    uintptr_t data;
    uintptr_t dat;
    
    /*To create queue with block on queue put*/
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxQueueCreate(queue_t, 8, queue_mem_t, TIVX_QUEUE_FLAG_BLOCK_ON_PUT));

   /*Putting elements on queue to reach the maximum number of elements*/
    queue_t->max_ele =1;
    queue_t->count =0;   
    queue_t->flags =TIVX_QUEUE_FLAG_BLOCK_ON_GET;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxQueuePut(queue_t, 8, 0));

    /*TO disable blocking on queue in tivxQueuePut */
    ASSERT_EQ_VX_STATUS(VX_FAILURE,tivxQueuePut(queue_t, 8, 1));

    /*To get queue element with block on queue put*/
    queue_t->flags =TIVX_QUEUE_FLAG_BLOCK_ON_PUT;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxQueueGet(queue_t, &data, 0));

    /*Putting elements on queue before tivxQueuePeek*/
    queue_t->flags =TIVX_QUEUE_FLAG_BLOCK_ON_GET;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxQueuePut(queue_t, 8, 0));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxQueuePeek((const tivx_queue *)queue_t, &dat));

    /*To Delete queue with block on queue put*/
    queue_t->flags =TIVX_QUEUE_FLAG_BLOCK_ON_PUT;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueueDelete(queue_t));
}

TEST(tivxQueue, testQueueCreateGet)
{
    vx_status status = VX_SUCCESS;
    tivx_queue test_queue[10];
    uintptr_t test_qu_mem;
    tivx_queue *queue_t = &test_queue[0];
    uintptr_t *queue_mem_t=&test_qu_mem;
    uintptr_t data;
    
    /*To create queue with block on queue put*/
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxQueueCreate(queue_t, 8, queue_mem_t, TIVX_QUEUE_FLAG_BLOCK_ON_PUT));

    queue_t->max_ele =1;
    queue_t->count =0;  
    /*TO disable blocking on queue in tivxQueueGet */
    ASSERT_EQ_VX_STATUS(VX_FAILURE,tivxQueueGet(queue_t, &data, 1));

    /*To Delete queue with block on queue put*/
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueueDelete(queue_t));
}

TESTCASE_TESTS(tivxQueue,
        testQueueCreatePutGetDelete,
        testQueueCreateGet
)