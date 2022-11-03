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

#include <TI/tivx_config.h>

#include "test_engine/test.h"

TESTCASE(tivxParameter, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxParameter, negativeTestGetKernelParameterByIndex)
{
    vx_context context = context_->vx_context_;

    vx_parameter param = NULL;
    vx_kernel kern = NULL;
    vx_uint32 index = TIVX_KERNEL_MAX_PARAMS + 1;

    ASSERT(NULL == (param = vxGetKernelParameterByIndex(kern, index)));
    ASSERT_VX_OBJECT(kern = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT(NULL != (param = vxGetKernelParameterByIndex(kern, index)));
    VX_CALL(vxReleaseKernel(&kern));
}

TEST(tivxParameter, negativeTestGetParameterByIndex)
{
    vx_context context = context_->vx_context_;

    vx_parameter param = NULL;
    vx_node node = NULL;
    vx_graph graph = NULL;
    vx_kernel kern = NULL;
    vx_uint32 index = TIVX_KERNEL_MAX_PARAMS + 1;

    ASSERT(NULL == (param = vxGetParameterByIndex(node, index)));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kern = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kern), VX_TYPE_NODE);
    ASSERT(NULL != (param = vxGetParameterByIndex(node, index)));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kern));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxParameter, negativeTestSetParameterByIndex)
{
    vx_context context = context_->vx_context_;

    vx_node node = NULL;
    vx_graph graph = NULL;
    vx_kernel kern = NULL;
    vx_reference value = NULL;
    vx_scalar scalar = NULL;
    vx_image img = NULL;
    vx_uint32 index = TIVX_KERNEL_MAX_PARAMS + 1, udata = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetParameterByIndex(node, index, value));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kern = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kern), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_VALUE, vxSetParameterByIndex(node, index, value));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_TYPE, vxSetParameterByIndex(node, 1, (vx_reference)(graph)));
    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT32, &udata), VX_TYPE_SCALAR);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_TYPE, vxSetParameterByIndex(node, 1, (vx_reference)(scalar)));
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetParameterByIndex(node, 1, (vx_reference)(img)));
    VX_CALL(vxReleaseImage(&img));
    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kern));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxParameter, negativeTestSetParameterByReference)
{
    vx_context context = context_->vx_context_;

    vx_parameter param = NULL;
    vx_kernel kern = NULL;
    vx_reference value = NULL;
    vx_uint32 index = 1;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetParameterByReference(param, value));
    ASSERT_VX_OBJECT(kern = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(param = vxGetKernelParameterByIndex(kern, index), VX_TYPE_PARAMETER);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetParameterByReference(param, value));
    VX_CALL(vxReleaseParameter(&param));
    VX_CALL(vxReleaseKernel(&kern));
}

TEST(tivxParameter, negativeTestQueryParameter)
{
    #define VX_PARAMETER_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_parameter param = NULL;
    vx_kernel kern = NULL;
    vx_enum attribute = VX_PARAMETER_DEFAULT;
    vx_uint32 udata = 0, index = 1;
    vx_size size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryParameter(param, attribute, &udata, size));
    ASSERT_VX_OBJECT(kern = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(param = vxGetKernelParameterByIndex(kern, index), VX_TYPE_PARAMETER);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryParameter(param, VX_PARAMETER_DIRECTION, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryParameter(param, VX_PARAMETER_INDEX, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryParameter(param, VX_PARAMETER_INDEX, &udata, sizeof(vx_uint32)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryParameter(param, VX_PARAMETER_TYPE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryParameter(param, VX_PARAMETER_STATE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryParameter(param, VX_PARAMETER_REF, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryParameter(param, VX_PARAMETER_REF, &udata, sizeof(vx_reference)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryParameter(param, attribute, &udata, size));
    VX_CALL(vxReleaseParameter(&param));
    VX_CALL(vxReleaseKernel(&kern));
}

TESTCASE_TESTS(
    tivxParameter,
    negativeTestGetKernelParameterByIndex,
    negativeTestGetParameterByIndex,
    negativeTestSetParameterByIndex,
    negativeTestSetParameterByReference,
    negativeTestQueryParameter
)

