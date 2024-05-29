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

TESTCASE_TESTS(tivxInternalArray,
    negativeTestInitVirtualArray
)