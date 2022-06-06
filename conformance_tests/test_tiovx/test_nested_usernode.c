/*

 * Copyright (c) 2012-2017 The Khronos Group Inc.
 * Copyright (c) 2022 Texas Instruments Inc.
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

#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx.h>

#define TIVX_KERNEL_CONFORMANCE_TEST_NESTED_USER_NAME "com.ti.test.own_nested_user"

static enum vx_type_e type = VX_TYPE_IMAGE;

TESTCASE(tivxNestedUserNode, CT_VXContext, ct_setup_vx_context, 0)

typedef enum _own_params_e
{
    OWN_PARAM_INPUT = 0,
    OWN_PARAM_OUTPUT,
} own_params_e;

static vx_status VX_CALLBACK own_Validator(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);

    vx_reference input = parameters[OWN_PARAM_INPUT];

    vx_meta_format meta = metas[OWN_PARAM_OUTPUT];

    vx_enum format = VX_DF_IMAGE_U8;
    vx_uint32 src_width = 128, src_height = 128;
    vx_uint32 dst_width = 128, dst_height = 128;

    vx_enum actual_format = VX_TYPE_INVALID;
    vx_uint32 actual_src_width = 128, actual_src_height = 128;
    vx_uint32 actual_dst_width = 128, actual_dst_height = 128;

    VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_FORMAT, &actual_format, sizeof(vx_enum)));
    VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_WIDTH, &actual_src_width, sizeof(vx_uint32)));
    VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_HEIGHT, &actual_src_height, sizeof(vx_uint32)));

    if (format == actual_format && src_width == actual_src_width && src_height == actual_src_height)
    {
        VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_IMAGE_FORMAT, &format, sizeof(vx_enum)));
        VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_IMAGE_WIDTH, &src_width, sizeof(vx_uint32)));
        VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_IMAGE_HEIGHT, &src_height, sizeof(vx_uint32)));
    }
    else
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK own_Kernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_context context;

    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);
    EXPECT(parameters != NULL);
    EXPECT(num == 2);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }

    context = vxCreateContext();

    status = vxuNot(context, (vx_image)parameters[0], (vx_image)parameters[1]);

    vxReleaseContext(&context);

    return status;
}

static vx_status VX_CALLBACK own_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);
    EXPECT(parameters != NULL);
    EXPECT(num == 2);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }
   return VX_SUCCESS;
}

static vx_status VX_CALLBACK own_Deinitialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    EXPECT(node != 0);
    EXPECT(parameters != NULL);
    EXPECT(num == 2);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }
    return VX_SUCCESS;
}

static void own_register_kernel(vx_context context, vx_kernel *kernel)
{
    vx_kernel local_kernel = 0;
    vx_enum kernel_id;
    vx_status status;

    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    ASSERT_VX_OBJECT(local_kernel = vxAddUserKernel(
        context,
        TIVX_KERNEL_CONFORMANCE_TEST_NESTED_USER_NAME,
        kernel_id,
        own_Kernel,
        2,
        own_Validator,
        own_Initialize,
        own_Deinitialize), VX_TYPE_KERNEL);

    tivxAddKernelTarget(local_kernel, TIVX_TARGET_A72_0);
    tivxAddKernelTarget(local_kernel, TIVX_TARGET_A72_1);
    tivxAddKernelTarget(local_kernel, TIVX_TARGET_A72_2);
    tivxAddKernelTarget(local_kernel, TIVX_TARGET_A72_3);

    VX_CALL(vxAddParameterToKernel(local_kernel, OWN_PARAM_INPUT, VX_INPUT, type, VX_PARAMETER_STATE_REQUIRED));
    VX_CALL(vxAddParameterToKernel(local_kernel, OWN_PARAM_OUTPUT, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    VX_CALL(vxFinalizeKernel(local_kernel));
    *kernel = local_kernel;
}

static void fillSquence(CT_Image dst, uint32_t seq_init)
{
    uint32_t i, j;
    uint32_t val = seq_init;

    ASSERT(dst);
    ASSERT(dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = ++val;
}

static void referenceNot(CT_Image src, CT_Image dst)
{
    uint32_t i, j;

    ASSERT(src && dst);
    ASSERT(src->width == dst->width);
    ASSERT(src->height == dst->height);
    ASSERT(src->format == dst->format && src->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = ~src->data.y[i * src->stride + j];
}

TEST(tivxNestedUserNode, testNestedUserKernel)
{
    vx_context context = context_->vx_context_;
    vx_reference src = 0, dst = 0;
    CT_Image ref_src, refdst, vxdst;
    vx_graph graph = 0;
    vx_kernel user_kernel = 0;
    vx_node node = 0;

    vx_enum format = VX_DF_IMAGE_U8;
    vx_uint32 src_width = 128, src_height = 128;
    vx_uint32 dst_width = 128, dst_height = 128;

    ASSERT_NO_FAILURE({
        ref_src = ct_allocate_image(src_width, src_height, VX_DF_IMAGE_U8);
        fillSquence(ref_src, (uint32_t)CT()->seed_);
        src = (vx_reference)ct_image_to_vx_image(ref_src, context);
    });
    ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateImage(context, src_width, src_height, format), type);

    ASSERT_NO_FAILURE(own_register_kernel(context, &user_kernel));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, user_kernel), VX_TYPE_NODE);

    VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)src));
    VX_CALL(vxSetParameterByIndex(node, 1, (vx_reference)dst));

    /* This must be different than TIVX_TARGET_A72_0, which is the HOST thread */
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_A72_1));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    ASSERT_NO_FAILURE({
        vxdst = ct_image_from_vx_image((vx_image)dst);
        refdst = ct_allocate_image(dst_width, dst_height, VX_DF_IMAGE_U8);
        referenceNot(ref_src, refdst);
    });

    ASSERT_EQ_CTIMAGE(refdst, vxdst);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    VX_CALL(vxRemoveKernel(user_kernel));

    VX_CALL(vxReleaseReference(&dst));
    VX_CALL(vxReleaseReference(&src));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(dst == 0);
    ASSERT(src == 0);
}


TESTCASE_TESTS(tivxNestedUserNode,
        testNestedUserKernel
        )
