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
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_capture.h>

#include "test_engine/test.h"

#define VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE1_NAME1 "org.khronos.openvx.test.user_source_1"

TESTCASE(tivxNodeApi, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxNodeApi, negativeTestWarpAffineNode)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_matrix matrix = NULL;
    vx_enum type = 0;
    vx_image output = NULL;

    ASSERT(NULL == vxWarpAffineNode(graph, input, matrix, type, output));
}

TEST(tivxNodeApi, negativeTestWarpPerspectiveNode)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_matrix matrix = NULL;
    vx_enum type = 0;
    vx_image output = NULL;

    ASSERT(NULL == vxWarpPerspectiveNode(graph, input, matrix, type, output));
}

TEST(tivxNodeApi, negativeTestRemapNode)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_remap table = NULL;
    vx_enum policy = 0;
    vx_image output = NULL;

    ASSERT(NULL == vxRemapNode(graph, input, table, policy, output));
}

/* tivxCreateNodeByKernelName , if(kernel!=NULL)*/
TEST(tivxNodeApi, negativeTesttivxCreateNodeByKernelName)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_remap table = NULL;
    vx_reference prms[]= {NULL};
    ASSERT(NULL == tivxCreateNodeByKernelName(graph, NULL, prms,dimof(prms)));
}

/* vxCreateNodeByStructure , #line 71*/
TEST(tivxNodeApi, negativeTesttivxCreateNodeByStructure)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_remap table = NULL;
    vx_reference prms[]= {NULL};
    vx_enum kernel_id = 10;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxAllocateUserKernelId(context, &kernel_id));
    vx_kernel kernel = vxAddUserKernel(
                    context,
                    "com.ti.test_kernels.cmd_timeout_test",
                    kernel_id,
                    NULL,
                    3,
                    NULL,
                    NULL,
                    NULL);

    ASSERT(NULL == tivxCreateNodeByKernelRef(graph, kernel, prms,3));
    VX_CALL(vxReleaseKernel(&kernel));
}

TEST(tivxNodeApi, negativeTesttivxCreateNodeByStructure1)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_image image;

    ASSERT_VX_OBJECT(image   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    vx_reference params[2] =
    {
        (vx_reference)image
    };
    ASSERT(NULL == tivxCreateNodeByKernelEnum(graph, VX_KERNEL_ABSDIFF, params, 2));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxNodeApi, negativeTestvxScaleImageNode)
{
    vx_graph graph = NULL;
    vx_image src = NULL;
    vx_image dst = NULL;
    vx_enum type = (vx_enum)0;

    ASSERT(NULL == vxScaleImageNode(graph, src, dst, type));
}

TEST(tivxNodeApi, negativeTestvxAddNode)
{
    vx_graph graph = NULL;
    vx_image in1 = NULL;
    vx_image in2 = NULL;
    vx_enum policy = (vx_enum)0;
    vx_image out = NULL;

    ASSERT(NULL == vxAddNode(graph, in1, in2, policy, out));
}

TEST(tivxNodeApi, negativeTestvxCannyEdgeDetectorNode)
{
    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_threshold hyst = NULL;
    vx_int32 gradient_size = (vx_int32)0;
    vx_enum norm_type = (vx_enum)0;
    vx_image output = NULL;

    ASSERT(NULL == vxCannyEdgeDetectorNode(graph, input, hyst, gradient_size, norm_type, output));
}

TEST(tivxNodeApi, negativeTestvxChannelExtractNode)
{
    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_enum channelNum = (vx_enum)0;
    vx_image output = NULL;

    ASSERT(NULL == vxChannelExtractNode(graph, input, channelNum, output));
}

TEST(tivxNodeApi, negativeTestvxConvertDepthNode)
{
    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_image output = NULL;
    vx_enum policy = (vx_enum)0;
    vx_scalar shift = NULL;

    ASSERT(NULL == vxConvertDepthNode(graph, input, output, policy, shift));
}

TEST(tivxNodeApi, negativeTestvxFastCornersNode)
{
    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_scalar strength_thresh = NULL;
    vx_bool nonmax_suppression = false;
    vx_array corners = NULL;
    vx_scalar num_corners = NULL;

    ASSERT(NULL == vxFastCornersNode(graph, input, strength_thresh, nonmax_suppression, corners, num_corners));
}

TEST(tivxNodeApi, negativeTestvxHalfScaleGaussianNode)
{
    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_image output = NULL;
    vx_int32 kernel_size = (vx_int32)0;

    ASSERT(NULL == vxHalfScaleGaussianNode(graph, input, output, kernel_size));
}

TEST(tivxNodeApi, negativeTestvxHarrisCornersNode)
{
    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_scalar strength_thresh = NULL;
    vx_scalar min_distance = NULL;
    vx_scalar sensitivity = NULL;
    vx_int32 gradient_size = (vx_int32)0;
    vx_int32 block_size = (vx_int32)0;
    vx_array corners = NULL;
    vx_scalar num_corners = NULL;

    ASSERT(NULL == vxHarrisCornersNode(graph, input, strength_thresh, min_distance, sensitivity, gradient_size, block_size, corners, num_corners));

}

TEST(tivxNodeApi, negativeTestvxMultiplyNode)
{
    vx_graph graph = NULL;
    vx_image in1 = NULL;
    vx_image in2 = NULL;
    vx_scalar scale = NULL;
    vx_enum overflow_policy = (vx_enum)0;
    vx_enum rounding_policy = (vx_enum)0;
    vx_image out = NULL;

    ASSERT(NULL == vxMultiplyNode(graph, in1, in2, scale, overflow_policy, rounding_policy, out));
}

TEST(tivxNodeApi, negativeTestvxNonLinearFilterNode)
{
    vx_graph graph = NULL;
    vx_enum function = (vx_enum)0; 
    vx_image input = NULL;
    vx_matrix mask = NULL;
    vx_image output = NULL;

    ASSERT(NULL == vxNonLinearFilterNode(graph, function, input, mask, output));
}

TEST(tivxNodeApi, negativeTestvxOpticalFlowPyrLKNode)
{
    vx_graph graph = NULL;
    vx_pyramid old_images = NULL;
    vx_pyramid new_images = NULL;
    vx_array old_points = NULL;
    vx_array new_points_estimates = NULL;
    vx_array new_points = NULL;
    vx_enum termination = (vx_enum)0;
    vx_scalar epsilon = NULL;
    vx_scalar num_iterations = NULL;
    vx_scalar use_initial_estimate = NULL;
    vx_size window_dimension= (vx_int32)0;

    ASSERT(NULL == vxOpticalFlowPyrLKNode(graph, old_images, new_images, old_points, new_points_estimates,
                                            new_points, termination, epsilon, num_iterations, use_initial_estimate,
                                            window_dimension));

}

TEST(tivxNodeApi, negativeTestvxSubtractNode)
{
    vx_graph graph = NULL;
    vx_image in1 = NULL;
    vx_image in2 = NULL;
    vx_enum policy = (vx_enum)0;
    vx_image out = NULL;

    ASSERT(NULL == vxSubtractNode(graph, in1, in2, policy, out));
}

TEST(tivxNodeApi, negativeTestvxAccumulateWeightedImageNodeX)
{
    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_image accum = NULL;
    vx_float32 alpha = (vx_float32)0;

    ASSERT(NULL == vxAccumulateWeightedImageNodeX(graph, input, alpha, accum));
}

TEST(tivxNodeApi, negativeTestvxAccumulateSquareImageNodeX)
{
    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_image accum = NULL;
    vx_uint32 shift = (vx_uint32)0;

    ASSERT(NULL == vxAccumulateSquareImageNodeX(graph, input, shift, accum));
}

TESTCASE_TESTS(
    tivxNodeApi,
    negativeTestWarpAffineNode,
    negativeTestWarpPerspectiveNode,
    negativeTestRemapNode,
    negativeTesttivxCreateNodeByKernelName,
    negativeTesttivxCreateNodeByStructure,
    negativeTesttivxCreateNodeByStructure1,
    negativeTestvxScaleImageNode,
    negativeTestvxAddNode,
    negativeTestvxCannyEdgeDetectorNode,
    negativeTestvxChannelExtractNode,
    negativeTestvxConvertDepthNode,
    negativeTestvxFastCornersNode,
    negativeTestvxHalfScaleGaussianNode,
    negativeTestvxHarrisCornersNode,
    negativeTestvxMultiplyNode,
    negativeTestvxNonLinearFilterNode,
    negativeTestvxOpticalFlowPyrLKNode,
    negativeTestvxSubtractNode,
    negativeTestvxAccumulateWeightedImageNodeX,
    negativeTestvxAccumulateSquareImageNodeX
)