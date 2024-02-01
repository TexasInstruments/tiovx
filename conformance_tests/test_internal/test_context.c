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
 * Copyright (c) 2024 Texas Instruments Incorporated
 */
#include <vx_internal.h>
#include <tivx_objects.h>

#include "test_tiovx.h"
#include "test_engine/test.h"
#include "test_tiovx/test_tiovx.h"
#include <VX/vx.h>
#include <VX/vxu.h>

#include <vx_context.h>

TESTCASE(tivxInternalContext, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalContext, negativeTestContextLock)
{
    #define VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME "org.khronos.openvx.test.own_user"
    vx_context context = context_->vx_context_;
    vx_kernel kernel;
    vx_enum kernel_id;
    vx_size num_bins = 1;
    vx_int32 offset = 1;
    vx_uint32 range = 5;
    vx_distribution dist,dist1 = NULL;
    vx_size size = 3U;
    vx_uint32 nkernels = 0;
    tivx_mutex lock1,lock2;
    vx_enum enumeration = 0;
    vx_kernel_f func_ptr = NULL;
    vx_uint32 numParams = 0;
    vx_kernel_validate_f validate = NULL;
    vx_kernel_initialize_f initialize = NULL;
    vx_kernel_deinitialize_f deinitialize = NULL;
    
    lock1 = context->lock;
    lock2 = context->log_lock;
    VX_CALL(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, &nkernels, sizeof(nkernels)));
    vx_kernel_info_t *kernel_table = (vx_kernel_info_t *)ct_alloc_mem(nkernels*sizeof(vx_kernel_info_t));
    EXPECT_VX_OBJECT(dist = vxCreateDistribution(context, num_bins, offset, range), VX_TYPE_DISTRIBUTION);
    EXPECT_VX_OBJECT(kernel = vxAddUserKernel(context, "org.khronos.openvx.test1", enumeration, func_ptr, numParams, validate, initialize, deinitialize),VX_TYPE_KERNEL);
    dist1 = dist;
    context->lock=NULL;

    ASSERT(VX_TYPE_INVALID == vxRegisterUserStruct(context, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS,vxAllocateUserKernelId(context, &kernel_id));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS,vxAllocateUserKernelLibraryId(context, &kernel_id));
    ASSERT(NULL == vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME));
    ASSERT(NULL == vxGetKernelByEnum(context, VX_KERNEL_ADD));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS,vxSetImmediateModeTarget(context, VX_TARGET_ANY, NULL));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNEL_TABLE, kernel_table, nkernels*sizeof(vx_kernel_info_t)));
    ASSERT(NULL == vxCreateArray(context, VX_TYPE_KEYPOINT, 2));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE,vxReleaseDistribution(&dist));
    ASSERT(NULL == vxAddUserKernel(context, "org.khronos.openvx.test", enumeration, func_ptr, numParams, validate, initialize, deinitialize));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS,vxFinalizeKernel(kernel));
    vxRegisterLogCallback(context, NULL, 0);
    context->log_lock = NULL;
    vxAddLogEntry((vx_reference)dist1, VX_FAILURE, "hello world", 1, 2, 3);

    context->lock = lock1;
    dist=dist1;
    VX_CALL(vxReleaseDistribution(&dist));
    VX_CALL(vxRemoveKernel(kernel));
    ct_free_mem((void *)kernel_table);
}

TESTCASE_TESTS(
    tivxInternalContext,
    negativeTestContextLock
)