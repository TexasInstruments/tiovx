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

TESTCASE(tivxReferenceLock, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxReferenceLock, negativeTestOwnRemoveNodeInt)
{
    vx_context context = context_->vx_context_;
    tivx_mutex lock1, lock2;
    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_kernel kernel = NULL;
    vx_enum kernel_id = VX_KERNEL_SOBEL_3x3;
    
    /* For the reference lock failure check in ownRemoveNodeInt that is invoked
    when a node is removed from the parent Graph */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, kernel_id), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    lock1 = (&node->graph->base)->lock;
    (&node->graph->base)->lock = NULL;
    lock2 = (&node->graph->base)->context->base.lock;
    (&node->graph->base)->context->base.lock = NULL;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS,vxRemoveNode(&node));
    (&node->graph->base)->lock = lock1;
    (&node->graph->base)->context->base.lock = lock2;
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxReferenceLock, negativeTestScalarHostMem)
{
    vx_context context = context_->vx_context_;
    tivx_mutex lock1, lock2;
    vx_uint32 udata = 0;
    vx_scalar scalar;
    /* For the reference lock failure check in ownScalarToHostMem and ownHostMemToScalar that is invoked
    in vxCopyScalar */
    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT32, &udata), VX_TYPE_SCALAR);
    lock1 = (&scalar->base)->lock;
    (&scalar->base)->lock=NULL;
    lock2 = (&scalar->base)->context->base.lock;
    (&scalar->base)->context->base.lock = NULL;
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_RESOURCES, vxCopyScalar(scalar, &udata, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_RESOURCES, vxCopyScalar(scalar, &udata, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    (&scalar->base)->lock = lock1;
    (&scalar->base)->context->base.lock = lock2;
    VX_CALL(vxReleaseScalar(&scalar));
}

TEST(tivxReferenceLock, negativeTestCreateGenericNode)
{
    vx_context context = context_->vx_context_;
    tivx_mutex lock1, lock2;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_enum kernel_id = VX_KERNEL_SOBEL_3x3;
    /* For the reference lock failure check in vxCreateGenericNode*/
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, kernel_id), VX_TYPE_KERNEL);
    lock1 = (&graph->base)->lock;
    (&graph->base)->lock = NULL;
    lock2 = (&graph->base)->context->base.lock;
    (&graph->base)->context->base.lock = NULL;
    ASSERT(NULL == vxCreateGenericNode(graph, kernel));
    (&graph->base)->lock = lock1;
    (&graph->base)->context->base.lock = lock2;
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxReferenceLock, negativeTestIncDecReference)
{
    vx_context context = context_->vx_context_;
    tivx_mutex lock1, lock2;
    vx_image img = NULL;
    vx_uint32 result = 0;
    /* For the reference lock failure check in ownDecrementReference and ownIncrementReference*/
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_reference img1 = (vx_reference)img;
    lock1 = (img1)->lock;
    img1->lock = NULL;
    lock2 = (img1)->context->base.lock;
    img1->context->base.lock = NULL;
    result = ownDecrementReference(img1, VX_EXTERNAL);
    ASSERT(result != 0);
    result = ownIncrementReference(img1, VX_EXTERNAL);
    ASSERT(result == 0);
    img1->lock = lock1;
    img1->context->base.lock = lock2;
    VX_CALL(vxReleaseImage(&img));
}

TESTCASE_TESTS(
    tivxReferenceLock,
    negativeTestOwnRemoveNodeInt,
    negativeTestScalarHostMem,
    negativeTestCreateGenericNode,
    negativeTestIncDecReference
)