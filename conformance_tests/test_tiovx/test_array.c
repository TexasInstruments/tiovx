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

TESTCASE_TESTS(
    tivxArray,
    test_vxCopyArrayRangeRead,
    test_vxCopyArrayRangeWrite,
    test_vxMapArrayRangeWrite)
