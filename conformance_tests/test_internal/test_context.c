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
    context->log_lock = lock2;
    dist=dist1;
    VX_CALL(vxReleaseDistribution(&dist));
    VX_CALL(vxRemoveKernel(kernel));
    ct_free_mem((void *)kernel_table);
}

/* Test to hit negative portion of ownContextGetUniqueKernels() */
TEST(tivxInternalContext, negativeTestOwnContextGetUniqueKernels)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint32 nkernels = 0;
    vx_kernel kernel   = 0;
    uint32_t i;

    uint32_t num_unique_kernels = context->num_unique_kernels;

    /* Reducing the num_unique_kernels by 1 */
    context->num_unique_kernels = (context->num_unique_kernels) - 1;

    VX_CALL(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, &nkernels, sizeof(nkernels)));

    vx_kernel_info_t *kernel_table = (vx_kernel_info_t *)ct_alloc_mem((nkernels + 1)*sizeof(vx_kernel_info_t));

    /* The follwing call should fail along with ownContextGetUniqueKernels(), as we have reduced the max kernels (num_unique_kernels) */
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_RESOURCES, vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNEL_TABLE, kernel_table, nkernels*sizeof(vx_kernel_info_t)));

    for (i = 0; i < nkernels; i++)
    {
        ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, kernel_table[i].enumeration), VX_TYPE_KERNEL);
        VX_CALL(vxReleaseKernel(&kernel));
        ASSERT_VX_OBJECT(kernel = vxGetKernelByName(context, kernel_table[i].name), VX_TYPE_KERNEL);
        VX_CALL(vxReleaseKernel(&kernel));
    }

    /* Resetting to the initial value */
    context->num_unique_kernels = num_unique_kernels;

    ct_free_mem(kernel_table);
}

/* Test to maximize the context references and to fail ownAddReferenceToContext() */
TEST(tivxInternalContext, negativeTestOwnAddReferenceToContext)
{
    vx_context context = context_->vx_context_;
    vx_uint32 width = 640, height = 480, add_ref_count, remove_ref_count;

    vx_image image = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB);

    /* Repeatedly add reference to the context with the same image reference to max out the references
    until it fails */
    for(add_ref_count = 0; add_ref_count < TIVX_CONTEXT_MAX_REFERENCES; add_ref_count++)
    {
        if (!ownAddReferenceToContext(context, (vx_reference)image))
        {
            break;
        }
    }

    /* Cleanup */
    for(remove_ref_count = 0; remove_ref_count < add_ref_count; remove_ref_count++)
    {
        if (!ownRemoveReferenceFromContext(context, (vx_reference)image))
        {
            break;
        }
    }
    VX_CALL(vxReleaseImage(&image));
}

/* Test to fail the setting of the reference name inside ownAddReferenceToContext() */
TEST(tivxInternalContext, negativeTestOwnAddReferenceToContext1)
{
    vx_context context = context_->vx_context_;
    vx_uint32 width = 640, height = 480, image_count, array_count, j;
    vx_image image;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);

    ASSERT((vx_bool)vx_false_e == ownAddReferenceToContext(NULL, (vx_reference)image));

    /* Manually setting the magic parameter to BAD, so that the vxSetReferenceName fails */
    image->base.magic = TIVX_BAD_MAGIC;

    /* Adding Reference to Context should fail, as vxSetReferenceName fails */
    ASSERT((vx_bool)vx_false_e == ownAddReferenceToContext(context, (vx_reference)image));

    /* Resetting the magic value */
    image->base.magic = TIVX_MAGIC;

    /* Cleanup */
    VX_CALL(vxReleaseImage(&image));
}

/* Test to cover negative portions of ownAddKernelToContext() */
TEST(tivxInternalContext, negativeTestOwnAddKernelToContext)
{
    vx_context context = context_->vx_context_;
    vx_kernel kernel;
    vx_uint32 numParams = 0;
    vx_enum kernel_id;

    VX_CALL(vxAllocateUserKernelId(context, &kernel_id));

    const vx_char kernel_name[VX_MAX_KERNEL_NAME] = "tempKernel";

    ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
                        context,
                        kernel_name,
                        kernel_id,
                        NULL,
                        numParams,
                        NULL,
                        NULL,
                        NULL), VX_TYPE_KERNEL);

    /* Kernel Reference Invalid, valid context */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownAddKernelToContext(context, NULL));
    /* Invalid Context and Invalid kernel reference*/
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownAddKernelToContext(NULL, kernel));
     /* Invalid Context and Kernel Reference Invalid */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownAddKernelToContext(NULL, NULL));

    /* Remove kernel */
    VX_CALL(vxRemoveKernel(kernel));
}

/* Test to hit negative portions of ownIsKernelInContext() */
TEST(tivxInternalContext, negativeTestOwnIsKernelInContext)
{
    vx_context context = context_->vx_context_;
    vx_bool is_found;
    const vx_char kernel_name[VX_MAX_KERNEL_NAME] = "NotRelevant";

    /* CONTEXT = NULL */
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownIsKernelInContext(NULL, 0, kernel_name, &is_found));
    /* is_found = NULL */
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownIsKernelInContext(context, 0, kernel_name, NULL));
    /* Kernel name = NULL */
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownIsKernelInContext(context, 0, NULL, &is_found));
}

/* Test to hit negative portion of ownRemoveKernelFromContext() */
TEST(tivxInternalContext, negativeTestOwnRemoveKernelFromContext)
{
    vx_context context = context_->vx_context_;
    vx_kernel kernel;
    tivx_mutex lock1;
    vx_uint32 numParams = 0;
    vx_enum kernel_id;

    lock1 = context->lock;

    VX_CALL(vxAllocateUserKernelId(context, &kernel_id));

    const vx_char kernel_name[VX_MAX_KERNEL_NAME] = "tempKernel";

   ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
                        context,
                        kernel_name,
                        kernel_id,
                        NULL,
                        numParams,
                        NULL,
                        NULL,
                        NULL), VX_TYPE_KERNEL);

    VX_CALL(ownAddKernelToContext(context, kernel));

    /* Kernel Reference Invalid, valid context */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownRemoveKernelFromContext(context, NULL));

    /* Invalid Context */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownRemoveKernelFromContext(NULL, kernel));

    /* Invalid Context and Kernel Reference Invalid */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownRemoveKernelFromContext(NULL, NULL));

    /* Removing lock */
    context->lock=NULL;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownRemoveKernelFromContext(context, kernel));

    /* Resetting the lock */
    context->lock=lock1;

    /* Removing actual context */
    VX_CALL(ownRemoveKernelFromContext(context, kernel));

    /* Invoking again with already removed kernel to hit negative portion */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownRemoveKernelFromContext(context, kernel));

    VX_CALL(vxRemoveKernel(kernel));
}

TEST(tivxInternalContext, negativeTestNullContextAndMisc)
{
    vx_context context = context_->vx_context_;
    vx_uint32 i = 0;

    ASSERT(vx_false_e == ownRemoveReferenceFromContext(NULL, NULL));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownContextSendControlCmd(NULL, 0, 0, 0, 0, 0, 0, 0));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownContextSendControlCmd(context, 0, 0, 0, 0, 0, TIVX_CMD_MAX_OBJ_DESCS, 0));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownContextSendCmd(NULL, 0, 0, 0, 0, 0));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownContextSendCmd(context, 0, 0, TIVX_CMD_MAX_OBJ_DESCS, 0, 0));

    uintptr_t obj_id;
    VX_CALL(tivxQueueGet(&context->free_queue, &obj_id, VX_TIMEOUT_WAIT_FOREVER));
    for (i=0; i<=TIVX_MAX_CTRL_CMD_OBJECTS; i++)
    {
        VX_CALL(tivxQueuePut(&context->free_queue, TIVX_MAX_CTRL_CMD_OBJECTS, VX_TIMEOUT_WAIT_FOREVER));
        VX_CALL(tivxQueueGet(&context->free_queue, &obj_id, VX_TIMEOUT_WAIT_FOREVER));
    }
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownContextSendControlCmd(context, 0, 0, 0, 0, 0, 0, 0));
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownContextSendCmd(context, 0, 0, 0, 0, 0));
}

TESTCASE_TESTS(
    tivxInternalContext,
    negativeTestContextLock,
    negativeTestOwnContextGetUniqueKernels,
    negativeTestOwnAddReferenceToContext,
    negativeTestOwnAddReferenceToContext1,
    negativeTestOwnAddKernelToContext,
    negativeTestOwnIsKernelInContext,
    negativeTestOwnRemoveKernelFromContext,
    negativeTestNullContextAndMisc
)