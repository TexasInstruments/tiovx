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

#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx_config.h>

#include "test_engine/test.h"

TESTCASE(tivxModule, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxModule, negativeTestLoadKernels)
{
    vx_context context = context_->vx_context_;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxLoadKernels(context, "tiovx"));
}

TEST(tivxModule, negativeTestUnloadKernels)
{
    vx_context context = context_->vx_context_;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxUnloadKernels(context, "tiovx"));
}

TESTCASE_TESTS(
    tivxModule,
    negativeTestLoadKernels,
    negativeTestUnloadKernels
)

