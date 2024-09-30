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
/*
 * Copyright (c) 2024 Texas Instruments Incorporated
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

TESTCASE(tivxInternalSafeCasts, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalSafeCasts, negativeTestGetRefAsBranch)
{
    vx_array array;
    vx_reference ref;
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

    ASSERT_VX_OBJECT(array = vxCreateArray(context_->vx_context_, VX_TYPE_UINT8, 4), VX_TYPE_ARRAY);
    ref = vxCastRefFromArray(array);

    /* Pass 'NULL' status to fail 'if(status)' condition */
    ASSERT(array == vxGetRefAsArray(&ref, NULL));
    ownDecrementReference(ref, (vx_enum)VX_EXTERNAL);

    /* To fail '(vx_status)VX_SUCCESS == *status)' condition */
    ASSERT(array == vxGetRefAsArray(&ref, &status));
    ownDecrementReference(ref, (vx_enum)VX_EXTERNAL);

    /* To fail 'ref->magic == TIVX_MAGIC' condition */
    ref->magic = TIVX_BAD_MAGIC;
    status = (vx_status)VX_SUCCESS;
    vxGetRefAsArray(&ref, &status);
    ASSERT(status == (vx_status)VX_ERROR_INVALID_REFERENCE);
    ref->magic = TIVX_MAGIC;

    vxReleaseArray(&array);
}

TEST(tivxInternalSafeCasts, negativeTestCastRefAsBranch)
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_array array = vxCreateArray(context_->vx_context_, VX_TYPE_UINT8, 4);
    vx_reference ref = vxCastRefFromArray(array);

    /* To fail 'ref->magic == TIVX_MAGIC' condition */
    ref->magic = TIVX_BAD_MAGIC;
    vxCastRefAsArray(ref, &status);
    ASSERT(status == (vx_status)VX_ERROR_INVALID_REFERENCE);
    ref->magic = TIVX_MAGIC;

    vxReleaseArray(&array);
}

TESTCASE_TESTS(tivxInternalSafeCasts,
    negativeTestGetRefAsBranch,
    negativeTestCastRefAsBranch)