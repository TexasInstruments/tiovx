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

TEST(tivxInternaldelay, testOwnIsValidObject)
{
    vx_context context = context_->vx_context_;
    vx_array array = NULL;
    vx_matrix matrix = 0;
    vx_size capacity = 2;
    vx_size count = 1;
    vx_distribution dist = NULL;
    vx_delay delay = NULL;
    vx_threshold hyst = NULL;
    const vx_enum matrix_type = VX_TYPE_INT8;
    const vx_size matrix_rows = 3;
    const vx_size matrix_cols = 2;
    vx_uint32 src_width = 16;
    vx_uint32 src_height = 32;
    vx_uint32 dst_width = 128;
    vx_uint32 dst_height = 64;
    vx_remap map = 0;
    vx_lut lut = 0;
    vx_user_data_object src_user_data;
    vx_char test_name[] = {'t', 'e', 's', 't', 'i', 'n', 'g'};
    vx_uint32 udata = 0;

    ASSERT_VX_OBJECT(array = vxCreateArray(context, VX_TYPE_KEYPOINT, capacity), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)array, count), VX_TYPE_DELAY);
    VX_CALL(vxReleaseDelay(&delay));
    

    ASSERT_VX_OBJECT(dist = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)dist, count), VX_TYPE_DELAY);
    VX_CALL(vxReleaseDelay(&delay));

    ASSERT_VX_OBJECT(hyst = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8), VX_TYPE_THRESHOLD);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)hyst, count), VX_TYPE_DELAY);
    VX_CALL(vxReleaseDelay(&delay));

    ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, matrix_type, matrix_cols, matrix_rows), VX_TYPE_MATRIX);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)matrix, count), VX_TYPE_DELAY);
    VX_CALL(vxReleaseDelay(&delay));

    ASSERT_VX_OBJECT(map = vxCreateRemap(context, src_width, src_height, dst_width, dst_height), VX_TYPE_REMAP);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)map, count), VX_TYPE_DELAY);
    VX_CALL(vxReleaseDelay(&delay));

    ASSERT_VX_OBJECT(lut = vxCreateLUT(context, VX_TYPE_UINT8, 256), VX_TYPE_LUT);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)lut, count), VX_TYPE_DELAY);
    VX_CALL(vxReleaseDelay(&delay));

    ASSERT_VX_OBJECT(src_user_data = vxCreateUserDataObject(context, test_name, sizeof(vx_uint32), &udata), VX_TYPE_USER_DATA_OBJECT);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)src_user_data, count), VX_TYPE_DELAY);
    VX_CALL(vxReleaseDelay(&delay));

    VX_CALL(vxReleaseArray(&array));
    VX_CALL(vxReleaseDistribution(&dist));
    VX_CALL(vxReleaseThreshold(&hyst));
    VX_CALL(vxReleaseMatrix(&matrix));
    VX_CALL(vxReleaseRemap(&map));
    VX_CALL(vxReleaseLUT(&lut));
    VX_CALL(vxReleaseUserDataObject(&src_user_data));

}
TESTCASE_TESTS(
    tivxInternaldelay,
    negativeTestVxAgeDelay,
    testOwnIsValidObject
)

