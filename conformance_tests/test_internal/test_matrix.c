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

TESTCASE(tivxInternalMatrix, CT_VXContext, ct_setup_vx_context, 0)

/* Testcase to fail ownCreateReference() inside vxCreateMatrixFromPattern() by setting context->lock to NULL*/
TEST(tivxInternalMatrix, negativeTestCreateMatrixPattern)
{
    vx_context context = context_->vx_context_;
    vx_size columns = 5, rows = 5;
    tivx_mutex lock = NULL;
    lock = context->lock;

    context->lock = NULL;
    EXPECT(NULL == vxCreateMatrixFromPattern(context, VX_PATTERN_OTHER, columns, rows));
    context->lock = lock;
}

TESTCASE_TESTS(tivxInternalMatrix,
    negativeTestCreateMatrixPattern)