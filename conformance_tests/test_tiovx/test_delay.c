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
 * Copyright (c) 2022 Texas Instruments Incorporated
 */

#include <math.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx_config.h>

#include "test_engine/test.h"

TESTCASE(tivxDelay, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxDelay, negativeTestCreateDelay)
{
    vx_context context = context_->vx_context_;

    vx_delay delay = NULL;
    vx_scalar scalar = NULL;
    vx_uint32 tmp_value = 0;
    vx_size count = 0;

    ASSERT(NULL == vxCreateDelay(NULL, (vx_reference)(scalar), count));
    EXPECT_VX_ERROR(delay = vxCreateDelay(context, (vx_reference)(scalar), count), VX_ERROR_INVALID_REFERENCE);
    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    count = TIVX_DELAY_MAX_OBJECT + 1;
    EXPECT_VX_ERROR(delay = vxCreateDelay(context, (vx_reference)(scalar), count), VX_ERROR_NO_RESOURCES);
    VX_CALL(vxReleaseScalar(&scalar));
}

TEST(tivxDelay, negativeTestGetReferenceFromDelay)
{
    vx_context context = context_->vx_context_;

    vx_delay delay = NULL;
    vx_scalar scalar = NULL;
    vx_uint32 tmp_value = 0;
    vx_size count = 1;
    vx_int32 index = 1;
    vx_reference reference;

    ASSERT(NULL == vxGetReferenceFromDelay(delay, index));
    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)(scalar), count), VX_TYPE_DELAY);
    EXPECT_VX_ERROR(reference = vxGetReferenceFromDelay(delay, index), VX_ERROR_INVALID_PARAMETERS);
    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleaseScalar(&scalar));
}

TEST(tivxDelay, negativeTestQueryDelay)
{
    #define VX_DELAY_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_delay delay = NULL;
    vx_scalar scalar = NULL;
    vx_uint32 tmp_value = 0;
    vx_size count = 1, dtype = 0;
    vx_enum attribute = VX_DELAY_DEFAULT;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryDelay(delay, attribute, &count, count));
    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)(scalar), count), VX_TYPE_DELAY);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryDelay(delay, VX_DELAY_TYPE, &count, count));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryDelay(delay, VX_DELAY_TYPE, &dtype, sizeof(dtype)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryDelay(delay, VX_DELAY_SLOTS, &count, count));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryDelay(delay, attribute, &count, count));
    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleaseScalar(&scalar));
}

TEST(tivxDelay, negativeTestAgeDelay)
{
    vx_context context = context_->vx_context_;

    vx_delay delay = NULL;
    vx_scalar scalar = NULL;
    vx_uint32 tmp_value = 0;
    vx_object_array oarray = NULL;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxAgeDelay(delay));
    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(oarray = vxCreateObjectArray(context, (vx_reference)(scalar), 4), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)(oarray), 3), VX_TYPE_DELAY);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxAgeDelay(delay));
    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleaseObjectArray(&oarray));
    VX_CALL(vxReleaseScalar(&scalar));
}

TESTCASE_TESTS(
    tivxDelay,
    negativeTestCreateDelay,
    negativeTestGetReferenceFromDelay,
    negativeTestQueryDelay,
    negativeTestAgeDelay
)

