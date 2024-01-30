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
#include <TI/tivx.h>

#include "test_engine/test.h"

/* Set module max to 13 instead of using TIVX_MODULE_MAX for maxing
 * out since some modules are already created during conformance test.
 */
#define MODULE_MAX 13u

void TestModuleRegister();
void TestModuleUnRegister();

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

TEST(tivxModule, negativeTestRegisterModule)
{
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxRegisterModule("tiovx",NULL,NULL));
}

TEST(tivxModule, negativeTestUnRegisterModule)
{
    ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxUnRegisterModule("tiovx"));
}

TEST(tivxModule, test_boundaryRegisterModule)
{
    for (int i = 0;i < MODULE_MAX; i++)
    {
        TestModuleRegister();
    }
    TestModuleRegister();

    for (int i = 0;i < MODULE_MAX; i++)
    {
        TestModuleUnRegister();
    }
}

TESTCASE_TESTS(
    tivxModule,
    negativeTestLoadKernels,
    negativeTestUnloadKernels,
    negativeTestRegisterModule,
    negativeTestUnRegisterModule,
    test_boundaryRegisterModule
)

