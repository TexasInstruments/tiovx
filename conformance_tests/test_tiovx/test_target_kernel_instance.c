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

#include <TI/tivx_target_kernel.h>

#include "test_engine/test.h"

TESTCASE(tivxTgKrnlInst, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxTgKrnlInst, negativeTestSetTargetKernelInstanceContext)
{
    vx_context context = context_->vx_context_;

    tivx_target_kernel_instance tki = NULL;
    vx_uint32 kernel_context;
    uint32_t kernel_context_size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxSetTargetKernelInstanceContext(tki, &kernel_context, kernel_context_size));
}

TEST(tivxTgKrnlInst, negativeTestGetTargetKernelInstanceState)
{
    vx_context context = context_->vx_context_;

    tivx_target_kernel_instance tki = NULL;
    vx_enum state = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxGetTargetKernelInstanceState(tki, &state));
}

TEST(tivxTgKrnlInst, negativeTestGetTargetKernelInstanceContext)
{
    vx_context context = context_->vx_context_;

    tivx_target_kernel_instance tki = NULL;
    vx_uint32 *kernel_context = NULL;
    uint32_t kernel_context_size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxGetTargetKernelInstanceContext(tki, (void *)(&kernel_context), &kernel_context_size));
}

TEST(tivxTgKrnlInst, negativeTestGetTargetKernelInstanceBorderMode)
{
    vx_context context = context_->vx_context_;

    tivx_target_kernel_instance tki = NULL;

    tivxGetTargetKernelInstanceBorderMode(tki, NULL);
}

TEST(tivxTgKrnlInst, negativeTestIsTargetKernelInstanceReplicated)
{
    vx_context context = context_->vx_context_;

    tivx_target_kernel_instance tki = NULL;

    tivxIsTargetKernelInstanceReplicated(tki);
}

TESTCASE_TESTS(
    tivxTgKrnlInst,
    negativeTestSetTargetKernelInstanceContext,
    negativeTestGetTargetKernelInstanceState,
    negativeTestGetTargetKernelInstanceContext,
    negativeTestGetTargetKernelInstanceBorderMode,
    negativeTestIsTargetKernelInstanceReplicated
)

