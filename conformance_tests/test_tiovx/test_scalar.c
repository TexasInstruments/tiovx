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

#include "test_engine/test.h"

TESTCASE(tivxScalar, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxScalar, negativeTestCreateScalar)
{
    vx_context context = context_->vx_context_;

    vx_scalar scalar = NULL;
    vx_uint32 udata = 0;

    ASSERT(NULL != (scalar = vxCreateScalar(context, VX_TYPE_INVALID, &udata)));
}

TEST(tivxScalar, negativeTestQueryScalar)
{
    #define VX_SCALAR_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_scalar scalar = NULL;
    vx_uint32 udata = 0;
    vx_enum attribute = VX_SCALAR_DEFAULT;
    vx_size size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryScalar(scalar, VX_SCALAR_TYPE, &udata, size));
    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT32, &udata), VX_TYPE_SCALAR);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryScalar(scalar, VX_SCALAR_TYPE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryScalar(scalar, attribute, &udata, size));
    VX_CALL(vxReleaseScalar(&scalar));
}

TEST(tivxScalar, negativeTestCopyScalar)
{
    vx_context context = context_->vx_context_;

    vx_scalar scalar = NULL;
    vx_uint32 udata = 0;
    vx_enum usage = VX_READ_ONLY, user_mem_type = VX_MEMORY_TYPE_NONE;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxCopyScalar(scalar, &udata, usage, user_mem_type));
    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT32, &udata), VX_TYPE_SCALAR);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxCopyScalar(scalar, &udata, usage, user_mem_type));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxCopyScalar(scalar, NULL, usage, VX_MEMORY_TYPE_HOST));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxCopyScalar(scalar, &udata, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxReleaseScalar(&scalar));
}

TESTCASE_TESTS(
    tivxScalar,
    negativeTestCreateScalar,
    negativeTestQueryScalar,
    negativeTestCopyScalar
)

