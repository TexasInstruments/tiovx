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
    tivx_queue test_queue1[10], test_queue2[10];
    uintptr_t test_queue1_mem, test_queue2_mem;
    tivx_queue *queue_1 = &test_queue1[0], *queue_2 = &test_queue2[0];
    uintptr_t *queue_mem_1=&test_queue1_mem, *queue_mem_2=&test_queue2_mem;
    uintptr_t data;
    uintptr_t dat;
    
    /*To create queue with block on queue put*/
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxQueueCreate(queue_1, 1, queue_mem_1, TIVX_QUEUE_FLAG_BLOCK_ON_GET));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxQueueCreate(queue_2, 1, queue_mem_2, TIVX_QUEUE_FLAG_BLOCK_ON_PUT));

   /*Putting elements on queue to reach the maximum number of elements*/
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxQueuePut(queue_1, 8, 0));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxQueuePut(queue_2, 8, 0));

    /*TO disable blocking on queue in tivxQueuePut */
    ASSERT_EQ_VX_STATUS(VX_FAILURE,tivxQueuePut(queue_1, 8, 1));

    /*To get queue element with block on queue put*/
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxQueueGet(queue_1, &data, 0));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxQueueGet(queue_2, &data, 0));

    /*Putting elements on queue before tivxQueuePeek*/
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxQueuePut(queue_1, 8, 0));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxQueuePeek((const tivx_queue *)queue_1, &dat));

    /*To Delete queue with block on queue put*/
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueueDelete(queue_2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxQueueDelete(queue_1));
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

TEST(tivxQueue, testQueueBranchCreatePutGetDelete)
{
    vx_status status = VX_SUCCESS;
    tivx_queue test_queue[10];
    uintptr_t test_qu_mem;
    tivx_queue *queue_t = NULL;
    uintptr_t *queue_mem_t=&test_qu_mem;
    uintptr_t data;
    uintptr_t dat;

    ASSERT_EQ_VX_STATUS(VX_FAILURE,tivxQueueCreate(queue_t, 8, queue_mem_t, TIVX_QUEUE_FLAG_BLOCK_ON_PUT));
    ASSERT_EQ_VX_STATUS(VX_FAILURE,tivxQueuePut(queue_t, 8, 0));
    ASSERT_EQ_VX_STATUS(VX_FAILURE,tivxQueueGet(queue_t, &data, 0));
    ASSERT_EQ_VX_STATUS(VX_FAILURE,tivxQueuePeek((const tivx_queue *)queue_t, &dat));
    ASSERT_EQ_VX_STATUS(vx_true_e,tivxQueueIsEmpty((const tivx_queue *)queue_t));
    ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxQueueDelete(queue_t));

    queue_t = &test_queue[0];
    queue_mem_t = NULL;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,tivxQueueCreate(queue_t, 8, queue_mem_t, TIVX_QUEUE_FLAG_BLOCK_ON_PUT));
    queue_mem_t=&test_qu_mem;
    queue_t->queue = queue_mem_t;
    queue_t->block_wr = NULL;
    queue_t->cur_rd = 0;
    queue_t->max_ele = 8;
    queue_t->flags = TIVX_QUEUE_FLAG_BLOCK_ON_PUT;
    queue_t->context = NULL;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,tivxQueueGet(queue_t, &data, 0));
    queue_t->count = 0;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,tivxQueuePeek((const tivx_queue *)queue_t, &dat));
    ASSERT_EQ_VX_STATUS(vx_true_e,tivxQueueIsEmpty((const tivx_queue *)queue_t));

    ASSERT_EQ_VX_STATUS(VX_FAILURE,tivxQueueCreate(queue_t, 0, queue_mem_t, TIVX_QUEUE_FLAG_BLOCK_ON_PUT));
}

TESTCASE_TESTS(tivxQueue,
        testQueueCreatePutGetDelete,
        testQueueCreateGet,
        testQueueBranchCreatePutGetDelete
)