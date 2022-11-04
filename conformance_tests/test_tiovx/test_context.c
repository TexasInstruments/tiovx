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
#include <TI/tivx.h>
#include <TI/tivx_config.h>
#include <TI/tivx_tensor.h>

#include "test_engine/test.h"

TESTCASE(tivxContext, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxContext, negativeTestReleaseContext)
{
    vx_context context = context_->vx_context_;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxReleaseContext(NULL));
}

TEST(tivxContext, negativeTestQueryContext)
{
    #define VX_CONTEXT_DEFAULT_ATTRIBUTE 0

    vx_context context = context_->vx_context_;

    vx_enum attribute = VX_CONTEXT_DEFAULT_ATTRIBUTE;
    void *ptr = NULL;
    vx_size size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryContext(NULL, attribute, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_VENDOR_ID, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_VERSION, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_MODULES, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_REFERENCES, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_IMPLEMENTATION, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_IMPLEMENTATION, ptr, (VX_MAX_IMPLEMENTATION_NAME + 1)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_EXTENSIONS_SIZE, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_EXTENSIONS, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_EXTENSIONS, ptr, 3));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_CONVOLUTION_MAX_DIMENSION, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_MAX_TENSOR_DIMS, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_NONLINEAR_MAX_DIMENSION, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_OPTICAL_FLOW_MAX_WINDOW_DIMENSION, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_IMMEDIATE_BORDER, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_IMMEDIATE_BORDER_POLICY, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNEL_TABLE, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryContext(context, VX_CONTEXT_DEFAULT_ATTRIBUTE, ptr, size));
}

TEST(tivxContext, negativeTestSetContextAttribute)
{
    #define VX_CONTEXT_DEFAULT_ATTRIBUTE 0

    vx_context context = context_->vx_context_;

    vx_enum attribute = VX_CONTEXT_DEFAULT_ATTRIBUTE;
    void *ptr = NULL;
    vx_border_t border;
    vx_size size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetContextAttribute(NULL, attribute, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_VALUE, vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(vx_border_t)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER, ptr, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetContextAttribute(context, attribute, ptr, size));
}

TEST(tivxContext, negativeTestDirective)
{
    #define VX_DIRECTIVE_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_enum directive = VX_DIRECTIVE_DEFAULT;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxDirective(NULL, directive));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxDirective((vx_reference)(context), VX_DIRECTIVE_DISABLE_PERFORMANCE));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxDirective((vx_reference)(context), VX_DIRECTIVE_ENABLE_PERFORMANCE));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxDirective((vx_reference)(context), directive));
}

TEST(tivxContext, negativeTestRegisterUserStruct)
{
    vx_context context = context_->vx_context_;

    vx_size size = 0;
    vx_enum item_type = VX_TYPE_INVALID;

    item_type = vxRegisterUserStruct(NULL, size);
    ASSERT(item_type == VX_TYPE_INVALID);
}

TEST(tivxContext, negativeTestSetImmediateModeTarget)
{
    #define VX_TARGET_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_enum target_enum = VX_TARGET_DEFAULT;
    char *target_string = NULL;

    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetImmediateModeTarget(context, VX_TARGET_STRING, target_string));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetImmediateModeTarget(context, target_enum, target_string));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetImmediateModeTarget(NULL, target_enum, target_string));
}

TEST(tivxContext, negativeTestGetKernelByName)
{
    vx_context context = context_->vx_context_;

    vx_kernel user_kernel;
    vx_char kernel_name[10];

    ASSERT(NULL == (user_kernel = vxGetKernelByName(NULL, kernel_name)));
}

TEST(tivxContext, negativeTestGetKernelByEnum)
{
    #define VX_KERNEL_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_kernel user_kernel;
    vx_enum kernelenum = VX_KERNEL_DEFAULT;

    ASSERT(NULL == (user_kernel = vxGetKernelByEnum(NULL, kernelenum)));
    ASSERT(NULL == (user_kernel = vxGetKernelByEnum(context, kernelenum)));
}

TEST(tivxContext, negativeTestAllocateUserKernelId)
{
    vx_context context = context_->vx_context_;

    vx_enum keID = 0;
    vx_uint32 i = 0;
    vx_status status = VX_SUCCESS;

    vx_uint32 num_user_kernel_id = 0;

    VX_CALL(vxQueryContext(context, TIVX_CONTEXT_NUM_USER_KERNEL_ID, &num_user_kernel_id, sizeof(num_user_kernel_id)));

    for (i = 0; i < TIVX_MAX_KERNEL_ID; i++) {
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxAllocateUserKernelId(context, &keID));
        VX_CALL(vxQueryContext(context, TIVX_CONTEXT_NUM_USER_KERNEL_ID, &num_user_kernel_id, sizeof(num_user_kernel_id)));
        ASSERT(num_user_kernel_id==(i+1));
    }
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_RESOURCES, vxAllocateUserKernelId(context, &keID));
}

TESTCASE_TESTS(
    tivxContext,
    negativeTestReleaseContext,
    negativeTestQueryContext,
    negativeTestSetContextAttribute,
    negativeTestDirective,
    negativeTestRegisterUserStruct,
    negativeTestSetImmediateModeTarget,
    negativeTestGetKernelByName,
    negativeTestGetKernelByEnum,
    negativeTestAllocateUserKernelId
)

