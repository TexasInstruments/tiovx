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


TESTCASE(tivxInternalParameter, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalParameter, negativeTestOwnIsValidTypeMatch)
{
    ASSERT(vx_true_e == ownIsValidTypeMatch(VX_TYPE_PARAMETER, VX_TYPE_PARAMETER));
    ASSERT(vx_false_e == ownIsValidTypeMatch(VX_TYPE_PARAMETER, VX_TYPE_PYRAMID));
}

TEST(tivxInternalParameter, negativeTestGetKernelParameterByIndex)
{
    vx_context context = context_->vx_context_;

    vx_parameter param = NULL;
    vx_kernel kern = NULL;
    vx_uint32 index = 1;
    tivx_mutex lock1,lock2;

    ASSERT_VX_OBJECT(kern = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    lock1 = (kern->base.context)->lock;
    (kern->base.context)->lock = NULL;
    ASSERT(NULL == (param = vxGetKernelParameterByIndex(kern, index)));
    (kern->base.context)->lock = lock1;
    VX_CALL(vxReleaseKernel(&kern));
}

TEST(tivxInternalParameter, negativeTestGetParameterByIndex)
{
    vx_context context = context_->vx_context_;

    vx_parameter param = NULL;
    vx_kernel kern = NULL, kernel = NULL;
    vx_uint32 index = 1;
    vx_node node = NULL;
    vx_graph graph = NULL;
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kern = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kern), VX_TYPE_NODE);
    kernel = node->kernel;
    node->kernel = NULL;
    param = vxGetParameterByIndex(node, index);
    ASSERT(((vx_reference)param)->type == VX_TYPE_ERROR);
    node->kernel = kernel;

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kern));
    VX_CALL(vxReleaseGraph(&graph));
}

TESTCASE_TESTS(tivxInternalParameter,
    negativeTestOwnIsValidTypeMatch,
    negativeTestGetKernelParameterByIndex,
    negativeTestGetParameterByIndex
    )