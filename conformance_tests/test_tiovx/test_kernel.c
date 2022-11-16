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

#include <TI/tivx.h>
#include <TI/tivx_config.h>
#include <VX/vx_khr_pipelining.h>

#include "test_engine/test.h"

TESTCASE(tivxKernel, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxKernel, negativeTestReleaseKernel)
{
    vx_context context = context_->vx_context_;

    vx_kernel kern = NULL;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxReleaseKernel(&kern));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxReleaseKernel(NULL));
}

TEST(tivxKernel, negativeTestQueryKernel)
{
    #define VX_KERNEL_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_kernel kern = NULL;
    vx_enum attribute = VX_KERNEL_DEFAULT;
    vx_uint32 udata = 0;
    vx_char kname[VX_MAX_KERNEL_NAME] = {0};
    vx_enum kenum;
    vx_size size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryKernel(kern, attribute, &udata, size));
    ASSERT_VX_OBJECT(kern = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryKernel(kern, VX_KERNEL_PARAMETERS, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryKernel(kern, VX_KERNEL_NAME, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryKernel(kern, VX_KERNEL_ENUM, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryKernel(kern, VX_KERNEL_LOCAL_DATA_SIZE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryKernel(kern, VX_KERNEL_PIPEUP_OUTPUT_DEPTH, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryKernel(kern, TIVX_KERNEL_TIMEOUT, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryKernel(kern, attribute, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryKernel(kern, VX_KERNEL_PARAMETERS, &udata, sizeof(vx_uint32)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryKernel(kern, VX_KERNEL_NAME, &kname, sizeof(kname)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryKernel(kern, VX_KERNEL_NAME, NULL, sizeof(kname)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryKernel(kern, VX_KERNEL_ENUM, &kenum, sizeof(vx_enum)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryKernel(kern, VX_KERNEL_LOCAL_DATA_SIZE, &size, sizeof(vx_size)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryKernel(kern, VX_KERNEL_PIPEUP_OUTPUT_DEPTH, &udata, sizeof(vx_uint32)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryKernel(kern, TIVX_KERNEL_TIMEOUT, &udata, sizeof(vx_uint32)));
    VX_CALL(vxReleaseKernel(&kern));
}

TEST(tivxKernel, negativeTestSetKernelAttribute)
{
    #define VX_KERNEL_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_kernel kern = NULL;
    vx_enum attribute = VX_KERNEL_DEFAULT;
    vx_uint32 udata = 0;
    vx_size size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetKernelAttribute(kern, attribute, &udata, size));
    ASSERT_VX_OBJECT(kern = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetKernelAttribute(kern, VX_KERNEL_LOCAL_DATA_SIZE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetKernelAttribute(kern, VX_KERNEL_PIPEUP_OUTPUT_DEPTH, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetKernelAttribute(kern, TIVX_KERNEL_TIMEOUT, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetKernelAttribute(kern, attribute, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetKernelAttribute(kern, TIVX_KERNEL_TIMEOUT, &udata, sizeof(vx_uint32)));
    udata = 1;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetKernelAttribute(kern, TIVX_KERNEL_TIMEOUT, &udata, sizeof(vx_uint32)));
    VX_CALL(vxReleaseKernel(&kern));
}

TEST(tivxKernel, negativeTestRemoveKernel)
{
    vx_context context = context_->vx_context_;

    vx_kernel kern = NULL;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxRemoveKernel(kern));
}

TEST(tivxKernel, negativeTestAddParameterToKernel)
{
    #define VX_PARAMETER_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_kernel kern = NULL;
    vx_uint32 index = 0;
    vx_enum dir = VX_BIDIRECTIONAL, data_type = VX_TYPE_INVALID, state = VX_PARAMETER_DEFAULT;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxAddParameterToKernel(kern, index, dir, data_type, state));
    ASSERT_VX_OBJECT(kern = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxAddParameterToKernel(kern, index, dir, data_type, state));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxAddParameterToKernel(kern, index, dir, VX_TYPE_DELAY, VX_PARAMETER_STATE_REQUIRED));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryKernel(kern, VX_KERNEL_PARAMETERS, &index, sizeof(vx_uint32)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxAddParameterToKernel(kern, index, dir, data_type, state));
    VX_CALL(vxReleaseKernel(&kern));
}

TEST(tivxKernel, negativeTestAddUserKernel)
{
    vx_context context = context_->vx_context_;

    vx_kernel kern = NULL;
    vx_enum enumeration = 0;
    vx_kernel_f func_ptr = NULL;
    vx_uint32 numParams = 0;
    vx_kernel_validate_f validate = NULL;
    vx_kernel_initialize_f initialize = NULL;
    vx_kernel_deinitialize_f deinitialize = NULL;

    ASSERT(NULL == (kern = vxAddUserKernel(NULL, NULL, enumeration, func_ptr, numParams, validate, initialize, deinitialize)));
    EXPECT_VX_ERROR(vxAddUserKernel(context, NULL, enumeration, func_ptr, numParams, validate, initialize, deinitialize), VX_ERROR_INVALID_PARAMETERS);
}

TEST(tivxKernel, negativeTestFinalizeKernel)
{
    vx_context context = context_->vx_context_;

    vx_kernel kern = NULL;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxFinalizeKernel(kern));
}

TEST(tivxKernel, negativeTestAddKernelTarget)
{
    vx_context context = context_->vx_context_;

    vx_kernel kern = NULL;
    char tname[] = {'t', 'i', 'o', 'v', 'x'};
    uint8_t i = 0, num_remaining_targets;

    /* Note: this kernel is registered on 2 DSP's for J721E
     * All other platforms have only 1 DSP target */
    #if defined(SOC_J721E)
    num_remaining_targets = TIVX_MAX_TARGETS_PER_KERNEL-2;
    #else
    num_remaining_targets = TIVX_MAX_TARGETS_PER_KERNEL-1;
    #endif

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, tivxAddKernelTarget(kern, tname));
    ASSERT_VX_OBJECT(kern = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    for (i=0; i<num_remaining_targets; i++) {
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxAddKernelTarget(kern, tname));
    }
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_RESOURCES, tivxAddKernelTarget(kern, tname));
    VX_CALL(vxReleaseKernel(&kern));
}

TEST(tivxKernel, negativeTestSetKernelSinkDepth)
{
    vx_context context = context_->vx_context_;

    vx_kernel kern = NULL;
    uint32_t nsb = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, tivxSetKernelSinkDepth(kern, nsb));
}

TESTCASE_TESTS(
    tivxKernel,
    negativeTestReleaseKernel,
    negativeTestQueryKernel,
    negativeTestSetKernelAttribute,
    negativeTestRemoveKernel,
    negativeTestAddParameterToKernel,
    negativeTestAddUserKernel,
    negativeTestFinalizeKernel,
    negativeTestAddKernelTarget,
    negativeTestSetKernelSinkDepth
)

