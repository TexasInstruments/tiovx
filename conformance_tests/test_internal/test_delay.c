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
#include <tivx_raw_image.h>
#include <VX/vx.h>
#include <VX/vxu.h>

TESTCASE(tivxInternaldelay, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternaldelay, negativeTestVxAgeDelay)
{
    vx_context context = context_->vx_context_;
    vx_pyramid pyr = NULL;
    vx_delay delay1 = NULL;
    vx_delay delay2 = NULL;
    vx_object_array oarray = NULL;
    vx_size count = 1;
    uint32_t width = 16;
    uint32_t height = 16;
    vx_size pyr_num_levels_temp = 0u;
    vx_size obj_arr_num_items_temp = 0u;
    vx_delay pyr_delay_temp = NULL;

    ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(oarray = vxCreateObjectArray(context, (vx_reference)(pyr), 4), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(delay1 = vxCreateDelay(context, (vx_reference)(pyr), count), VX_TYPE_DELAY);
    ASSERT_VX_OBJECT(delay2 = vxCreateDelay(context, (vx_reference)(oarray), count), VX_TYPE_DELAY);
    
    pyr_num_levels_temp = delay1->pyr_num_levels;
    obj_arr_num_items_temp = delay2->obj_arr_num_items;
    pyr_delay_temp = delay1->pyr_delay[0];

    delay1->pyr_num_levels = 0u;
    delay2->obj_arr_num_items = 0u;
    VX_CALL(vxAgeDelay(delay1));
    VX_CALL(vxAgeDelay(delay2));

    delay1->pyr_num_levels = 1u;
    delay1->pyr_delay[0] = NULL;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxAgeDelay(delay1));

    delay1->pyr_num_levels = pyr_num_levels_temp;
    delay2->obj_arr_num_items = obj_arr_num_items_temp;
    delay1->pyr_delay[0] = pyr_delay_temp;

    VX_CALL(vxReleaseDelay(&delay1));
    VX_CALL(vxReleaseDelay(&delay2));
    VX_CALL(vxReleaseObjectArray(&oarray));
    VX_CALL(vxReleasePyramid(&pyr));

}

TESTCASE_TESTS(
    tivxInternaldelay,
    negativeTestVxAgeDelay
)

