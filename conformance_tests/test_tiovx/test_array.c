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
 * Copyright (c) 2020 Texas Instruments Incorporated
 */

#include <math.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx_config.h>

#include "test_engine/test.h"

TESTCASE(tivxArray, CT_VXContext, ct_setup_vx_context, 0)

#define N 100
TEST(tivxArray, test_vxCopyArrayRangeWrite)
{
    vx_context context = context_->vx_context_;
    vx_coordinates2d_t localArrayInit[N];
    vx_coordinates2d_t localArray[N];
    vx_coordinates2d_t localArray2[N*3];
    vx_array array;
    vx_size num_items1, num_items2;
    int i;

    /* Initialization */
    for (i = 0; i < N; i++)
    {
        localArrayInit[i].x = 0;
        localArrayInit[i].y = 0;

        localArray[i].x = i;
        localArray[i].y = i;

        localArray2[3*i].x = 2*i;
        localArray2[3*i].y = 2*i;
        localArray2[3*i+1].x = 0;
        localArray2[3*i+1].y = 0;
        localArray2[3*i+2].x = 0;
        localArray2[3*i+2].y = 0;
    }

    ASSERT_VX_OBJECT(array = vxCreateArray(context, VX_TYPE_COORDINATES2D, N), VX_TYPE_ARRAY);
    VX_CALL(vxAddArrayItems(array, N, localArrayInit, sizeof(vx_coordinates2d_t)) );

    VX_CALL(vxQueryArray(array, VX_ARRAY_NUMITEMS, &num_items1, sizeof(num_items1)));

    ASSERT(N == num_items1);

    /* Write, COPY, No spacing */
    {
        vx_size stride = sizeof(vx_coordinates2d_t);
        vx_coordinates2d_t *p = &localArray[N/2];
        VX_CALL(vxCopyArrayRange(array, N/2, N, stride, (void *)p, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    }

    VX_CALL(vxQueryArray(array, VX_ARRAY_NUMITEMS, &num_items2, sizeof(num_items1)));

    ASSERT(N == num_items2);

    VX_CALL(vxReleaseArray(&array));
    ASSERT(array == 0);
}

TEST(tivxArray, test_vxCopyArrayRangeRead)
{
    vx_context context = context_->vx_context_;
    vx_coordinates2d_t localArrayInit[N];
    vx_coordinates2d_t localArray[N];
    vx_coordinates2d_t localArray2[N*3];
    vx_array array;
    vx_size num_items1, num_items2;
    int i;

    /* Initialization */
    for (i = 0; i < N; i++)
    {
        localArrayInit[i].x = i;
        localArrayInit[i].y = i;

        localArray[i].x = 0;
        localArray[i].y = 0;

        localArray2[3*i].x = 0;
        localArray2[3*i].y = 0;
        localArray2[3*i+1].x = 0;
        localArray2[3*i+1].y = 0;
        localArray2[3*i+2].x = 0;
        localArray2[3*i+2].y = 0;
    }

    ASSERT_VX_OBJECT(array = vxCreateArray(context, VX_TYPE_COORDINATES2D, N), VX_TYPE_ARRAY);
    VX_CALL(vxAddArrayItems(array, N, localArrayInit, sizeof(vx_coordinates2d_t)));

    VX_CALL(vxQueryArray(array, VX_ARRAY_NUMITEMS, &num_items1, sizeof(num_items1)));

    ASSERT(N == num_items1);

    /* READ, COPY, No spacing */
    {
        vx_size stride = sizeof(vx_coordinates2d_t);
        vx_coordinates2d_t *p = &localArray[N/2];
        VX_CALL(vxCopyArrayRange(array, N/2, N, stride, (void *)p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    }

    VX_CALL(vxQueryArray(array, VX_ARRAY_NUMITEMS, &num_items2, sizeof(num_items1)));

    ASSERT(N == num_items2);

    VX_CALL(vxReleaseArray(&array));
    ASSERT(array == 0);
}

TEST(tivxArray, test_vxMapArrayRangeWrite)
{
    vx_context context = context_->vx_context_;
    vx_coordinates2d_t localArrayInit[N];
    vx_array array;
    vx_size num_items1, num_items2, num_items3;
    int i;

    /* Initialization */
    for (i = 0; i < N; i++)
    {
        localArrayInit[i].x = i;
        localArrayInit[i].y = i;
    }

    ASSERT_VX_OBJECT(array = vxCreateArray(context, VX_TYPE_COORDINATES2D, N), VX_TYPE_ARRAY);
    VX_CALL(vxAddArrayItems(array, N, localArrayInit, sizeof(vx_coordinates2d_t)) );

    VX_CALL(vxQueryArray(array, VX_ARRAY_NUMITEMS, &num_items1, sizeof(num_items1)));

    ASSERT(N == num_items1);

    /* Map, READ_AND_WRITE mode*/
    {
        vx_size stride = 0;
        vx_coordinates2d_t *p = NULL;
        vx_map_id map_id;
        VX_CALL(vxMapArrayRange(array, N/2, N, &map_id, &stride, (void **)&p, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, VX_NOGAP_X));
        ASSERT(stride == sizeof(vx_coordinates2d_t));
        VX_CALL(vxUnmapArrayRange(array, map_id));

        VX_CALL(vxMapArrayRange(array, 0, N, &map_id, &stride, (void **)&p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X));
        VX_CALL(vxUnmapArrayRange(array, map_id));
    }

    VX_CALL(vxQueryArray(array, VX_ARRAY_NUMITEMS, &num_items2, sizeof(num_items1)));

    ASSERT(N == num_items2);

    /* Map, WRITE_ONLY mode */
    {
        vx_size stride = 0;
        vx_coordinates2d_t *p = NULL;
        vx_map_id map_id;
        VX_CALL(vxMapArrayRange(array, N/2, N, &map_id, &stride, (void **)&p, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X));
        ASSERT(stride == sizeof(vx_coordinates2d_t));
        /* Write into array */
        for (i = 0; i < N/2; i++)
        {
            p[i].x = 0;
            p[i].y = 0;
        }
        VX_CALL(vxUnmapArrayRange(array, map_id));
        /* Check */
        VX_CALL(vxMapArrayRange(array, 0, N, &map_id, &stride, (void **)&p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X));
        VX_CALL(vxUnmapArrayRange(array, map_id));
    }

    VX_CALL(vxQueryArray(array, VX_ARRAY_NUMITEMS, &num_items3, sizeof(num_items1)));

    ASSERT(N == num_items3);

    VX_CALL(vxReleaseArray(&array));
    ASSERT(array == 0);
}

TEST(tivxArray, negativeTestQueryArray)
{
    #define VX_ARRAY_DEFAULT (VX_ARRAY_ITEMSIZE + 1)

    vx_context context = context_->vx_context_;

    vx_array array = NULL;
    vx_size num_items = 0, size = 0;
    vx_enum item_type = VX_TYPE_KEYPOINT;
    vx_size capacity = 2;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryArray(array, VX_ARRAY_ITEMTYPE, &num_items, size));
    ASSERT_VX_OBJECT(array = vxCreateArray(context, item_type, capacity), VX_TYPE_ARRAY);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryArray(array, VX_ARRAY_ITEMTYPE, &num_items, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryArray(array, VX_ARRAY_NUMITEMS, &num_items, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryArray(array, VX_ARRAY_CAPACITY, &num_items, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryArray(array, VX_ARRAY_ITEMSIZE, &num_items, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryArray(array, VX_ARRAY_DEFAULT, &num_items, size));
    VX_CALL(vxReleaseArray(&array));
}

TEST(tivxArray, negativeTestAddArrayItems)
{
    vx_context context = context_->vx_context_;

    vx_array array = NULL;
    vx_size count = 2;
    void *array_items = NULL;
    vx_size stride = 1;
    vx_enum item_type = VX_TYPE_KEYPOINT;
    vx_size capacity = 1;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxAddArrayItems(array, count, array_items, stride));
    ASSERT_VX_OBJECT(array = vxCreateArray(context, item_type, capacity), VX_TYPE_ARRAY);
    ASSERT_EQ_VX_STATUS(VX_FAILURE, vxAddArrayItems(array, count, array_items, stride));
    VX_CALL(vxReleaseArray(&array));
}

TEST(tivxArray, negativeTestTruncateArray)
{
    #define KA 2

    vx_context context = context_->vx_context_;

    vx_array array = NULL;
    vx_size new_num_items = KA + 1;
    vx_enum item_type = VX_TYPE_KEYPOINT;
    vx_size capacity = 1;
    int i = 0;
    vx_keypoint_t keypoint_array[KA];

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxTruncateArray(array, new_num_items));
    ASSERT_VX_OBJECT(array = vxCreateArray(context, item_type, (KA * sizeof(vx_keypoint_t))), VX_TYPE_ARRAY);
    for (i = 0; i < KA; i++) {
        memset(&keypoint_array[i], 0, sizeof(vx_keypoint_t));
    }
    VX_CALL(vxAddArrayItems(array, KA, keypoint_array, sizeof(vx_keypoint_t)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxTruncateArray(array, new_num_items));
    VX_CALL(vxReleaseArray(&array));
}

TEST(tivxArray, negativeTestCopyArrayRange)
{
    #define KA 2

    vx_context context = context_->vx_context_;

    vx_array array = NULL;
    vx_size range_start = 0, range_end = 0, stride = 0;
    void *array_items = NULL;
    vx_enum usage = VX_READ_ONLY, user_mem_type = VX_MEMORY_TYPE_NONE, item_type = VX_TYPE_KEYPOINT;
    int i = 0;
    vx_keypoint_t keypoint_array[KA];

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxCopyArrayRange(array, range_start, range_end, stride, array_items, usage, user_mem_type));
    ASSERT_VX_OBJECT(array = vxCreateArray(context, item_type, (KA * sizeof(vx_keypoint_t))), VX_TYPE_ARRAY);
    for (i = 0; i < KA; i++) {
        memset(&keypoint_array[i], 0, sizeof(vx_keypoint_t));
    }
    VX_CALL(vxAddArrayItems(array, KA, keypoint_array, sizeof(vx_keypoint_t)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxCopyArrayRange(array, range_start, range_end, stride, array_items, usage, user_mem_type));
    VX_CALL(vxReleaseArray(&array));
}

TEST(tivxArray, negativeTestMapUnmapArrayRange)
{
    #define KA 2

    vx_context context = context_->vx_context_;

    vx_array array = NULL;
    vx_size range_start = 0, range_end = 0, stride = 0;
    vx_map_id map_id = 0;
    void *ptr = NULL;
    vx_enum usage = VX_READ_ONLY, mem_type = VX_MEMORY_TYPE_HOST, item_type = VX_TYPE_KEYPOINT;
    vx_uint32 flags = 0;
    int i = 0;
    vx_keypoint_t keypoint_array[KA];

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxMapArrayRange(array, range_start, range_end, &map_id, &stride, &ptr, usage, mem_type, flags));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxUnmapArrayRange(array, map_id));
    ASSERT_VX_OBJECT(array = vxCreateArray(context, item_type, (KA * sizeof(vx_keypoint_t))), VX_TYPE_ARRAY);
    for (i = 0; i < KA; i++) {
        memset(&keypoint_array[i], 0, sizeof(vx_keypoint_t));
    }
    VX_CALL(vxAddArrayItems(array, KA, keypoint_array, sizeof(vx_keypoint_t)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxMapArrayRange(array, range_start, range_end, &map_id, NULL, &ptr, usage, mem_type, flags));
    range_start = 1;
    range_end = 2;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapArrayRange(array, range_start, range_end, &map_id, &stride, NULL, usage, mem_type, flags));
    for (i = 0; i < TIVX_ARRAY_MAX_MAPS + 1; i++) {
        stride = 0;
        if (i == TIVX_ARRAY_MAX_MAPS) {
            ASSERT_EQ_VX_STATUS(VX_ERROR_NO_RESOURCES, vxMapArrayRange(array, range_start, range_end, &map_id, &stride, &ptr, usage, mem_type, flags));
        } else {
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapArrayRange(array, range_start, range_end, &map_id, &stride, &ptr, usage, mem_type, flags));
        }
    }
    map_id = TIVX_ARRAY_MAX_MAPS;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxUnmapArrayRange(array, map_id));
    VX_CALL(vxReleaseArray(&array));
}

TEST(tivxArray, negativeTestCreateArray)
{
    vx_context context = context_->vx_context_;

    vx_enum item_type = VX_TYPE_KEYPOINT;
    vx_size capacity = 0;

    ASSERT(NULL == vxCreateArray(NULL, item_type, capacity));
    ASSERT(NULL == vxCreateArray(context, item_type, capacity));
}

TEST(tivxArray, negativeTestCreateVirtualArray)
{
    vx_context context = context_->vx_context_;

    vx_enum item_type = VX_TYPE_KEYPOINT;
    vx_size capacity = 0;

    ASSERT(NULL == vxCreateVirtualArray((vx_graph)(context), item_type, capacity));
}

TESTCASE_TESTS(
    tivxArray,
    test_vxCopyArrayRangeRead,
    test_vxCopyArrayRangeWrite,
    test_vxMapArrayRangeWrite,
    negativeTestQueryArray,
    negativeTestAddArrayItems,
    negativeTestTruncateArray,
    negativeTestCopyArrayRange,
    negativeTestMapUnmapArrayRange,
    negativeTestCreateArray,
    negativeTestCreateVirtualArray
)

