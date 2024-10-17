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

#include "test_tiovx.h"

#include <TI/tivx_test_kernels.h>
#include <TI/tivx_config.h>
#include <TI/tivx_capture.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include <string.h>
#include <math.h>

#include <vx_internal.h>

#include "shared_functions.h"

TESTCASE(tivxInternalArray, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalArray, negativeTestInitVirtualArray)
{
    vx_context context = context_->vx_context_;
    vx_enum item_type = VX_TYPE_INVALID;
    vx_array array = NULL;
    vx_size capacity = 2;
    tivx_obj_desc_t *obj_desc = NULL;

    ASSERT_VX_OBJECT(array = vxCreateArray(context, VX_TYPE_KEYPOINT, capacity), VX_TYPE_ARRAY);

    EXPECT_EQ_VX_STATUS(ownInitVirtualArray(NULL,item_type,capacity), VX_FAILURE);

    obj_desc = array->base.obj_desc;
    array->base.obj_desc = NULL;

    EXPECT_EQ_VX_STATUS(ownInitVirtualArray(array,item_type,capacity), VX_FAILURE);
    array->base.obj_desc = obj_desc;

    /* to hit branch conditions */
    EXPECT_EQ_VX_STATUS(ownInitVirtualArray(array,VX_TYPE_INVALID,0), VX_FAILURE);
    EXPECT_EQ_VX_STATUS(ownInitVirtualArray(array,VX_TYPE_KEYPOINT,0), VX_FAILURE);
    EXPECT_EQ_VX_STATUS(ownInitVirtualArray(array,VX_TYPE_KEYPOINT,2), VX_FAILURE);

    VX_CALL(vxReleaseArray(&array));
}

TEST(tivxInternalArray, negativeTestUnmapArrayRange)
{
    vx_context context = context_->vx_context_;
    vx_array array;
    vx_enum item_type = VX_TYPE_BOOL;
    vx_size capacity = 10;
    vx_size num_items = 10;

    void* array_items = 0;
    vx_size item_size = sizeof(vx_bool);
    vx_size stride = 0;
    void* ptr = 0;
    vx_map_id map_id;

    ASSERT_VX_OBJECT(array = vxCreateArray(context, item_type, capacity), VX_TYPE_ARRAY);

    array_items = ct_alloc_mem(num_items * item_size);

    for (int i = 0; i < num_items; i++)
    {
        ((vx_bool*)array_items)[i] = (vx_bool)(i & 1 ? vx_true_e : vx_false_e);
    }

    VX_CALL(vxAddArrayItems(array, num_items, array_items, item_size));

    VX_CALL(vxMapArrayRange(array, 0, num_items, &map_id, &stride, &ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));

    uint8_t *map_addr = array->maps[map_id].map_addr;
    array->maps[map_id].map_addr = NULL;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxUnmapArrayRange(array, map_id));

    array->maps[map_id].map_addr = map_addr;
    VX_CALL(vxUnmapArrayRange(array, map_id));

    VX_CALL(vxReleaseArray(&array));
}

TEST(tivxInternalArray, negativeTestTruncateArrayAndCopyArrayRange)
{
    vx_context context = context_->vx_context_;
    vx_enum item_type = VX_TYPE_INVALID;
    vx_array array = NULL;
    vx_size capacity = 2;
    vx_size new_num_items = 0u;
    int64_t host_ptr = 0;
    vx_bool is_virtual = (vx_bool)vx_false_e;
    vx_bool is_accessible = (vx_bool)vx_false_e;

    ASSERT_VX_OBJECT(array = vxCreateArray(context, VX_TYPE_KEYPOINT, capacity), VX_TYPE_ARRAY);

    capacity = ((tivx_obj_desc_array_t *)array->base.obj_desc)->capacity;
    ((tivx_obj_desc_array_t *)array->base.obj_desc)->capacity = 0u;

    host_ptr = ((tivx_obj_desc_array_t *)array->base.obj_desc)->mem_ptr.host_ptr;
    ((tivx_obj_desc_array_t *)array->base.obj_desc)->mem_ptr.host_ptr = 1;

    is_virtual = array->base.is_virtual;
    array->base.is_virtual = (vx_bool)vx_true_e;

    is_accessible = array->base.is_accessible;
    array->base.is_accessible = (vx_bool)vx_false_e;

    EXPECT_EQ_VX_STATUS(vxTruncateArray(array, new_num_items), VX_ERROR_INVALID_PARAMETERS);

    EXPECT_EQ_VX_STATUS(vxCopyArrayRange(array, 0u , 0u, 0u, NULL, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_NONE), VX_ERROR_INVALID_PARAMETERS);

    ((tivx_obj_desc_array_t *)array->base.obj_desc)->capacity = capacity;
    ((tivx_obj_desc_array_t *)array->base.obj_desc)->mem_ptr.host_ptr = host_ptr;
    array->base.is_virtual = is_virtual;
    array->base.is_accessible = is_accessible;

    VX_CALL(vxReleaseArray(&array));
}

TEST(tivxInternalArray, negativeMapArrayRangeAndUnmapArrayRange)
{
    vx_context context = context_->vx_context_;
    vx_enum item_type = VX_TYPE_INVALID;
    vx_array array = NULL;
    vx_size capacity = 2;
    vx_size new_num_items = 0u;
    int64_t host_ptr = 0;
    vx_bool is_virtual = (vx_bool)vx_false_e;
    vx_bool is_accessible = (vx_bool)vx_false_e;

    ASSERT_VX_OBJECT(array = vxCreateArray(context, VX_TYPE_KEYPOINT, capacity), VX_TYPE_ARRAY);

    host_ptr = ((tivx_obj_desc_array_t *)array->base.obj_desc)->mem_ptr.host_ptr;
    ((tivx_obj_desc_array_t *)array->base.obj_desc)->mem_ptr.host_ptr = 1;

    is_virtual = array->base.is_virtual;
    array->base.is_virtual = (vx_bool)vx_true_e;

    is_accessible = array->base.is_accessible;
    array->base.is_accessible = (vx_bool)vx_false_e;

    EXPECT_EQ_VX_STATUS(vxMapArrayRange(array, 0u, 0u, NULL, NULL, NULL, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_NONE, 0u), VX_ERROR_INVALID_PARAMETERS);
    EXPECT_EQ_VX_STATUS(vxUnmapArrayRange(array, TIVX_ARRAY_MAX_MAPS), VX_ERROR_INVALID_PARAMETERS);

    ((tivx_obj_desc_array_t *)array->base.obj_desc)->mem_ptr.host_ptr = host_ptr;
    array->base.is_virtual = is_virtual;
    array->base.is_accessible = is_accessible;

    VX_CALL(vxReleaseArray(&array));
}

TESTCASE_TESTS(tivxInternalArray,
    negativeTestInitVirtualArray,
    negativeTestUnmapArrayRange,
    negativeTestTruncateArrayAndCopyArrayRange,
    negativeMapArrayRangeAndUnmapArrayRange
)