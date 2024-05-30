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

TESTCASE(tivxInternalLUT, CT_VXContext, ct_setup_vx_context, 0)

/* Testcase to fail vxQueryLUT() by passing an invalid type*/
TEST(tivxInternalLUT, negativeTestQueryLUT)
{
    vx_context context = context_->vx_context_;
    vx_lut lut = NULL;
    tivx_obj_desc_lut_t *obj_desc = NULL;
    vx_uint32 lut_offset = 0;

    ASSERT_VX_OBJECT(lut = vxCreateLUT(context, VX_TYPE_FLOAT64, 1), VX_TYPE_LUT);

    obj_desc = (tivx_obj_desc_lut_t *)lut->base.obj_desc;
    obj_desc->item_type = -1; /* Send invalid item_type*/
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryLUT(lut, VX_LUT_OFFSET, &lut_offset, sizeof(lut_offset)));

    obj_desc->item_type = VX_TYPE_FLOAT64;
    VX_CALL(vxReleaseLUT(&lut));
}

TESTCASE_TESTS(tivxInternalLUT,
    negativeTestQueryLUT)