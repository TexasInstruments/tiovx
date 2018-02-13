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

#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>

TESTCASE(Graph, CT_VXContext, ct_setup_vx_context, 0)

TEST(Graph, testTwoNodes)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, interm_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(interm_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src_image, interm_image), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxIntegralImageNode(graph, interm_image, dst_image), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&interm_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node1 == 0); ASSERT(node2 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(interm_image == 0); ASSERT(src_image == 0);
}

TEST(Graph, testVirtualImage)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, interm_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(interm_image = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src_image, interm_image), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxIntegralImageNode(graph, interm_image, dst_image), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&interm_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node1 == 0); ASSERT(node2 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(interm_image == 0); ASSERT(src_image == 0);
}

#ifndef dimof
#  define dimof(arr) (sizeof(arr)/sizeof((arr)[0]))
#endif

// example from specification wrapped into asserts
static void testCornersGraphFactory(vx_context context, vx_graph* graph_out)
{
    vx_uint32 i;
    vx_float32 strength_thresh = 10000.0f;
    vx_float32 r = 1.5f;
    vx_float32 sensitivity = 0.14f;
    vx_int32 window_size = 3;
    vx_int32 block_size = 3;
    vx_graph graph = 0;
    vx_enum channel = VX_CHANNEL_Y;

    *graph_out = NULL;
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    // if (vxGetStatus((vx_reference)graph) == VX_SUCCESS)
    {
        vx_image virts[] = {
            vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT),
            vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT)
        };

        vx_kernel kernels[] = {
            vxGetKernelByEnum(context, VX_KERNEL_CHANNEL_EXTRACT),
            vxGetKernelByEnum(context, VX_KERNEL_MEDIAN_3x3),
            vxGetKernelByEnum(context, VX_KERNEL_HARRIS_CORNERS)
        };

        vx_node nodes[dimof(kernels)] = {
            vxCreateGenericNode(graph, kernels[0]),
            vxCreateGenericNode(graph, kernels[1]),
            vxCreateGenericNode(graph, kernels[2])
        };

        vx_scalar scalars[] = {
            vxCreateScalar(context, VX_TYPE_ENUM, &channel),
            vxCreateScalar(context, VX_TYPE_FLOAT32, &strength_thresh),
            vxCreateScalar(context, VX_TYPE_FLOAT32, &r),
            vxCreateScalar(context, VX_TYPE_FLOAT32, &sensitivity),
            vxCreateScalar(context, VX_TYPE_INT32, &window_size),
            vxCreateScalar(context, VX_TYPE_INT32, &block_size)
        };

        vx_parameter parameters[] = {
            vxGetParameterByIndex(nodes[0], 0),
            vxGetParameterByIndex(nodes[2], 6)
        };

        for (i = 0; i < dimof(virts); i++)
            ASSERT_VX_OBJECT(virts[i], VX_TYPE_IMAGE);
        for (i = 0; i < dimof(kernels); i++)
            ASSERT_VX_OBJECT(kernels[i], VX_TYPE_KERNEL);
        for (i = 0; i < dimof(nodes); i++)
            ASSERT_VX_OBJECT(nodes[i], VX_TYPE_NODE);
        for (i = 0; i < dimof(scalars); i++)
            ASSERT_VX_OBJECT(scalars[i], VX_TYPE_SCALAR);
        for (i = 0; i < dimof(parameters); i++)
            ASSERT_VX_OBJECT(parameters[i], VX_TYPE_PARAMETER);

        // Channel Extract
        VX_CALL(vxAddParameterToGraph(graph, parameters[0]));
        VX_CALL(vxSetParameterByIndex(nodes[0], 1, (vx_reference)scalars[0]));
        VX_CALL(vxSetParameterByIndex(nodes[0], 2, (vx_reference)virts[0]));
        // Median Filter
        VX_CALL(vxSetParameterByIndex(nodes[1], 0, (vx_reference)virts[0]));
        VX_CALL(vxSetParameterByIndex(nodes[1], 1, (vx_reference)virts[1]));
        // Harris Corners
        VX_CALL(vxSetParameterByIndex(nodes[2], 0, (vx_reference)virts[1]));
        VX_CALL(vxSetParameterByIndex(nodes[2], 1, (vx_reference)scalars[1]));
        VX_CALL(vxSetParameterByIndex(nodes[2], 2, (vx_reference)scalars[2]));
        VX_CALL(vxSetParameterByIndex(nodes[2], 3, (vx_reference)scalars[3]));
        VX_CALL(vxSetParameterByIndex(nodes[2], 4, (vx_reference)scalars[4]));
        VX_CALL(vxSetParameterByIndex(nodes[2], 5, (vx_reference)scalars[5]));
        VX_CALL(vxAddParameterToGraph(graph, parameters[1]));

        for (i = 0; i < dimof(scalars); i++)
        {
            VX_CALL(vxReleaseScalar(&scalars[i]));
            ASSERT(scalars[i] == NULL);
        }
        for (i = 0; i < dimof(virts); i++)
        {
            VX_CALL(vxReleaseImage(&virts[i]));
            ASSERT(virts[i] == NULL);
        }
        for (i = 0; i < dimof(kernels); i++)
        {
            VX_CALL(vxReleaseKernel(&kernels[i]));
            ASSERT(kernels[i] == NULL);
        }
        for (i = 0; i < dimof(nodes); i++)
        {
            VX_CALL(vxReleaseNode(&nodes[i]));
            ASSERT(nodes[i] == NULL);
        }
        for (i = 0; i < dimof(parameters); i++)
        {
            VX_CALL(vxReleaseParameter(&parameters[i]));
            ASSERT(parameters[i] == NULL);
        }
    }

    *graph_out = graph;
}

TEST(Graph, testGraphFactory)
{
    vx_context context = context_->vx_context_;
    vx_graph   graph;
    vx_image   source;
    vx_array   points;
    vx_parameter points_param;

    ASSERT_NO_FAILURE(testCornersGraphFactory(context, &graph));

    ASSERT_VX_OBJECT(source = vxCreateImage(context, 640, 480, VX_DF_IMAGE_YUV4), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(points = vxCreateArray(context, VX_TYPE_KEYPOINT, 100), VX_TYPE_ARRAY);

    ASSERT_NO_FAILURE(ct_fill_image_random(source, &CT()->seed_));

    VX_CALL(vxSetGraphParameterByIndex(graph, 0, (vx_reference)source));

    ASSERT_VX_OBJECT(points_param = vxGetGraphParameterByIndex(graph, 1), VX_TYPE_PARAMETER);
    VX_CALL(vxSetParameterByReference(points_param, (vx_reference)points));

    VX_CALL(vxReleaseParameter(&points_param));
    ASSERT(points_param == NULL);
    VX_CALL(vxReleaseImage(&source));
    ASSERT(source == NULL);

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseArray(&points));
    ASSERT(points == NULL);

    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(graph == NULL);
}

#define VX_KERNEL_CONFORMANCE_TEST_TAKE10_NAME "org.khronos.openvx.test.array_take_10"

static vx_status VX_CALLBACK take10_ParameterValidator(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    vx_size capacity = 10;
    vx_enum type = VX_TYPE_KEYPOINT;

    // are we really required to set these attributes???
    VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(metas[1], VX_ARRAY_CAPACITY, &capacity, sizeof(capacity)));
    VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(metas[1], VX_ARRAY_ITEMTYPE, &type, sizeof(type)));

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK take10_Kernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_array input, output;
    vx_size len = 0;
    vx_size stride = 0;
    void* data = 0;
    vx_map_id map_id;
    ASSERT_(return VX_FAILURE, num == 2);
    ASSERT_VX_OBJECT_(return VX_FAILURE, input  = (vx_array)parameters[0], VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT_(return VX_FAILURE, output = (vx_array)parameters[1], VX_TYPE_ARRAY);

    VX_CALL_(return VX_FAILURE, vxTruncateArray(output, 0));
    VX_CALL_(return VX_FAILURE, vxQueryArray(input, VX_ARRAY_NUMITEMS, &len, sizeof(len)));

    if (len > 10) len = 10;

    VX_CALL_(return VX_FAILURE, vxMapArrayRange(input,  0, len, &map_id, &stride, &data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X));
    VX_CALL_(return VX_FAILURE, vxAddArrayItems(output, len, data, stride));
    VX_CALL_(return VX_FAILURE, vxUnmapArrayRange(input, map_id));

    return VX_SUCCESS;
}

static void take10_node(vx_graph graph, vx_array in, vx_array out)
{
    vx_kernel kernel = 0;
    vx_node node = 0;
    vx_context context = vxGetContext((vx_reference)graph);
    vx_enum take10_enumId = 0u;

    ASSERT_VX_OBJECT(context, VX_TYPE_CONTEXT);

    VX_CALL(vxAllocateUserKernelId(context, &take10_enumId));
    ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
            context,
            VX_KERNEL_CONFORMANCE_TEST_TAKE10_NAME,
            take10_enumId,
            take10_Kernel,
            2,
            take10_ParameterValidator,
            NULL,
            NULL), VX_TYPE_KERNEL);

    VX_CALL(vxAddParameterToKernel(kernel, 0, VX_INPUT,  VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED));
    VX_CALL(vxAddParameterToKernel(kernel, 1, VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED));
    VX_CALL(vxFinalizeKernel(kernel));

    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)in));
    VX_CALL(vxSetParameterByIndex(node, 1, (vx_reference)out));

    VX_CALL(vxReleaseNode(&node));
    ASSERT(node == 0);

    VX_CALL(vxReleaseKernel(&kernel));
    ASSERT(kernel == 0);
}

TEST(Graph, testVirtualArray)
{
    vx_bool    use_estimations;
    vx_uint32  num_iter;
    vx_float32 threshold_f;
    vx_float32 eps;
    vx_scalar  fast_thresh;
    vx_scalar  flow_eps;
    vx_scalar  flow_num_iter;
    vx_scalar  flow_use_estimations;
    vx_size    window;
    vx_array   corners;
    vx_array   new_corners, corners10;
    vx_image   frame0 = 0;
    vx_image   frame1 = 0;
    vx_pyramid p0;
    vx_pyramid p1;
    vx_node    n1;
    vx_node    n2;
    vx_node    n3;
    vx_node    n4;
    vx_graph   graph;
    vx_context context;
    vx_kernel  kernel;
    CT_Image   src0 = 0, src1 = 0;
    vx_scalar  scalar_fastCorners = 0;
    vx_size    fastCorners = 0;

    ASSERT_VX_OBJECT(context = context_->vx_context_, VX_TYPE_CONTEXT);

    ASSERT_NO_FAILURE(src0 = ct_read_image("optflow_00.bmp", 1));
    ASSERT_NO_FAILURE(src1 = ct_read_image("optflow_01.bmp", 1));

    ASSERT_VX_OBJECT(frame0 = ct_image_to_vx_image(src0, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(frame1 = ct_image_to_vx_image(src1, context), VX_TYPE_IMAGE);

    threshold_f = 80;
    eps = 0.01f;
    num_iter = 10;
    use_estimations = vx_false_e;
    window = 3;

    ASSERT_VX_OBJECT(fast_thresh = vxCreateScalar(context, VX_TYPE_FLOAT32, &threshold_f), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(flow_eps = vxCreateScalar(context, VX_TYPE_FLOAT32, &eps), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(flow_num_iter = vxCreateScalar(context, VX_TYPE_UINT32, &num_iter), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(flow_use_estimations = vxCreateScalar(context, VX_TYPE_BOOL, &use_estimations), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    // Don't use zero capacity here
    ASSERT_VX_OBJECT(corners = vxCreateVirtualArray(graph, 0, 100), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(new_corners = vxCreateVirtualArray(graph, 0, 0), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(corners10   = vxCreateArray(context, VX_TYPE_KEYPOINT, 10), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(scalar_fastCorners = vxCreateScalar(context, VX_TYPE_SIZE, &fastCorners), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = vxFastCornersNode(graph, frame0, fast_thresh, vx_true_e, corners, scalar_fastCorners), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(p0 = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 0, 0, VX_DF_IMAGE_VIRT), VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(p1 = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 0, 0, VX_DF_IMAGE_VIRT), VX_TYPE_PYRAMID);

    ASSERT_VX_OBJECT(n2 = vxGaussianPyramidNode(graph, frame0, p0), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n3 = vxGaussianPyramidNode(graph, frame1, p1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n4 = vxOpticalFlowPyrLKNode(graph, p0, p1, corners, corners, new_corners, VX_TERM_CRITERIA_BOTH, flow_eps, flow_num_iter, flow_use_estimations, window), VX_TYPE_NODE);

    take10_node(graph, new_corners, corners10);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxCopyScalar(scalar_fastCorners, &fastCorners, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    ASSERT(fastCorners > 0);

    VX_CALL(vxReleaseScalar(&scalar_fastCorners));
    VX_CALL(vxReleaseScalar(&fast_thresh));
    VX_CALL(vxReleaseScalar(&flow_eps));
    VX_CALL(vxReleaseScalar(&flow_num_iter));
    VX_CALL(vxReleaseScalar(&flow_use_estimations));
    VX_CALL(vxReleaseArray(&corners));
    VX_CALL(vxReleaseArray(&new_corners));
    VX_CALL(vxReleaseArray(&corners10));
    VX_CALL(vxReleasePyramid(&p0));
    VX_CALL(vxReleasePyramid(&p1));
    VX_CALL(vxReleaseImage(&frame0));
    VX_CALL(vxReleaseImage(&frame1));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n3));
    VX_CALL(vxReleaseNode(&n4));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT_VX_OBJECT(kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_TAKE10_NAME), VX_TYPE_KERNEL);
    VX_CALL(vxRemoveKernel(kernel));
}


TEST(Graph, testNodeRemove)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, interm_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_uint32 num_nodes = 0;

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(interm_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src_image, interm_image), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxIntegralImageNode(graph, interm_image, dst_image), VX_TYPE_NODE);

    VX_CALL(vxQueryGraph(graph, VX_GRAPH_NUMNODES, &num_nodes, sizeof(num_nodes)));
    ASSERT(num_nodes == 2);

    VX_CALL(vxRemoveNode(&node2));
    ASSERT(node2 == 0);

    VX_CALL(vxQueryGraph(graph, VX_GRAPH_NUMNODES, &num_nodes, sizeof(num_nodes)));
    ASSERT(num_nodes == 1);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&interm_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node1 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(interm_image == 0); ASSERT(src_image == 0);
}

TEST(Graph, testNodeFromEnum)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_kernel kernel = 0;
    vx_node node = 0;
    vx_uint32 num_params = 0;
    vx_parameter parameter = 0;
    vx_image p_image = 0;

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);

    VX_CALL(vxQueryKernel(kernel, VX_KERNEL_PARAMETERS, &num_params, sizeof(num_params)));
    ASSERT_EQ_INT(2, num_params);

    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)src_image));

    ASSERT_VX_OBJECT(parameter = vxGetParameterByIndex(node, 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_REF, &p_image, sizeof(p_image)));
    ASSERT(p_image == src_image);
    VX_CALL(vxReleaseImage(&p_image));
    VX_CALL(vxReleaseParameter(&parameter));

    {
        /* check set node parameter by reference */
        ASSERT_VX_OBJECT(parameter = vxGetParameterByIndex(node, 1), VX_TYPE_PARAMETER);

        /* parameter was not set yet */
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_REF, &p_image, sizeof(p_image)));
        ASSERT(p_image != dst_image);
        VX_CALL(vxSetParameterByReference(parameter, (vx_reference)dst_image));
        /* expect parameter is set to know value */
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_REF, &p_image, sizeof(p_image)));
        ASSERT(p_image == dst_image);
    }

    VX_CALL(vxReleaseImage(&p_image));
    VX_CALL(vxReleaseParameter(&parameter));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node == 0);
    ASSERT(kernel == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(src_image == 0);
}

TEST(Graph, testTwoNodesWithSameDst)
{
    vx_context context = context_->vx_context_;
    vx_image src1_image = 0, src2_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;

    ASSERT_VX_OBJECT(src1_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src1_image, dst_image), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxMedian3x3Node(graph, src2_image, dst_image), VX_TYPE_NODE);

    EXPECT_NE_VX_STATUS(vxVerifyGraph(graph), VX_SUCCESS);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src1_image));
    VX_CALL(vxReleaseImage(&src2_image));

    ASSERT(node1 == 0); ASSERT(node2 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(src1_image == 0); ASSERT(src2_image == 0);
}

TEST(Graph, testCycle)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src_image, dst_image), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxMedian3x3Node(graph, dst_image, src_image), VX_TYPE_NODE);

    EXPECT_NE_VX_STATUS(vxVerifyGraph(graph), VX_SUCCESS);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node1 == 0); ASSERT(node2 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(src_image == 0);
}

TEST(Graph, testCycle2)
{
    vx_context context = context_->vx_context_;
    vx_image src1_image = 0, src2_image = 0, interm_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;

    ASSERT_VX_OBJECT(src1_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(interm_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxAddNode(graph, src1_image, src2_image, VX_CONVERT_POLICY_SATURATE, interm_image), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxBox3x3Node(graph, interm_image, dst_image), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxMedian3x3Node(graph, dst_image, src2_image), VX_TYPE_NODE);

    EXPECT_NE_VX_STATUS(vxVerifyGraph(graph), VX_SUCCESS);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src1_image));
    VX_CALL(vxReleaseImage(&src2_image));
    VX_CALL(vxReleaseImage(&interm_image));

    ASSERT(node1 == 0); ASSERT(node2 == 0); ASSERT(node3 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(src1_image == 0); ASSERT(src2_image == 0); ASSERT(interm_image == 0);
}


TEST(Graph, testMultipleRun)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, interm_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    CT_Image res1 = 0, res2 = 0;
    vx_border_t border = { VX_BORDER_UNDEFINED };

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src_image, &CT()->seed_));
    ASSERT_VX_OBJECT(interm_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src_image, interm_image), VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    ASSERT_VX_OBJECT(node2 = vxIntegralImageNode(graph, interm_image, dst_image), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));
    ASSERT_NO_FAILURE(res1 = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(ct_fill_image_random(dst_image, &CT()->seed_));

    VX_CALL(vxProcessGraph(graph));
    ASSERT_NO_FAILURE(res2 = ct_image_from_vx_image(dst_image));

    ct_adjust_roi(res1, 1, 1, 1, 1);
    ct_adjust_roi(res2, 1, 1, 1, 1);

    ASSERT_EQ_CTIMAGE(res1, res2);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&interm_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node1 == 0); ASSERT(node2 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(interm_image == 0); ASSERT(src_image == 0);
}


TEST(Graph, testMultipleRunAsync)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, interm_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    CT_Image res1 = 0, res2 = 0;
    vx_border_t border = { VX_BORDER_UNDEFINED };

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src_image, &CT()->seed_));
    ASSERT_VX_OBJECT(interm_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src_image, interm_image), VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    ASSERT_VX_OBJECT(node2 = vxIntegralImageNode(graph, interm_image, dst_image), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxScheduleGraph(graph));
    VX_CALL(vxWaitGraph(graph));
    ASSERT_NO_FAILURE(res1 = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(ct_fill_image_random(dst_image, &CT()->seed_));

    VX_CALL(vxScheduleGraph(graph));
    VX_CALL(vxWaitGraph(graph));
    ASSERT_NO_FAILURE(res2 = ct_image_from_vx_image(dst_image));

    ct_adjust_roi(res1, 1, 1, 1, 1);
    ct_adjust_roi(res2, 1, 1, 1, 1);

    ASSERT_EQ_CTIMAGE(res1, res2);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&interm_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node1 == 0); ASSERT(node2 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(interm_image == 0); ASSERT(src_image == 0);
}


TEST(Graph, testAsyncWaitWithoutSchedule)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, interm_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(interm_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src_image, interm_image), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxIntegralImageNode(graph, interm_image, dst_image), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&interm_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node1 == 0); ASSERT(node2 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(interm_image == 0); ASSERT(src_image == 0);
}


TEST(Graph, testNodePerformance)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, interm_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf;
    vx_perf_t perf2;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(interm_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src_image, interm_image), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxIntegralImageNode(graph, interm_image, dst_image), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf, sizeof(perf)));

    ASSERT(perf.num == 1);
    ASSERT(perf.beg > 0);
    ASSERT(perf.min > 0);

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf2, sizeof(perf)));

    ASSERT(perf2.num == 2);
    ASSERT(perf2.beg > perf.end);
    ASSERT(perf2.min > 0);
    ASSERT(perf2.sum > perf.sum);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&interm_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node1 == 0); ASSERT(node2 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(interm_image == 0); ASSERT(src_image == 0);
}


TEST(Graph, testGraphPerformance)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, interm_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf;
    vx_perf_t perf2;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(interm_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src_image, interm_image), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxIntegralImageNode(graph, interm_image, dst_image), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf, sizeof(perf)));

    ASSERT(perf.num == 1);
    ASSERT(perf.beg > 0);
    ASSERT(perf.min > 0);

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf2, sizeof(perf)));

    ASSERT(perf2.num == 2);
    ASSERT(perf2.beg >= perf.end);
    ASSERT(perf2.min > 0);
    ASSERT(perf2.sum >= perf.sum);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&interm_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node1 == 0); ASSERT(node2 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(interm_image == 0); ASSERT(src_image == 0);
}

typedef struct
{
    char* name;
    vx_enum kernel_id;
} kernel_name_arg;

TEST_WITH_ARG(Graph, testKernelName, kernel_name_arg,
    ARG("org.khronos.openvx.color_convert",         VX_KERNEL_COLOR_CONVERT),
    ARG("org.khronos.openvx.channel_extract",       VX_KERNEL_CHANNEL_EXTRACT),
    ARG("org.khronos.openvx.channel_combine",       VX_KERNEL_CHANNEL_COMBINE),
    ARG("org.khronos.openvx.sobel_3x3",             VX_KERNEL_SOBEL_3x3),
    ARG("org.khronos.openvx.magnitude",             VX_KERNEL_MAGNITUDE),
    ARG("org.khronos.openvx.phase",                 VX_KERNEL_PHASE),
    ARG("org.khronos.openvx.scale_image",           VX_KERNEL_SCALE_IMAGE),
    ARG("org.khronos.openvx.table_lookup",          VX_KERNEL_TABLE_LOOKUP),
    ARG("org.khronos.openvx.histogram",             VX_KERNEL_HISTOGRAM),
    ARG("org.khronos.openvx.equalize_histogram",    VX_KERNEL_EQUALIZE_HISTOGRAM),
    ARG("org.khronos.openvx.absdiff",               VX_KERNEL_ABSDIFF),
    ARG("org.khronos.openvx.mean_stddev",           VX_KERNEL_MEAN_STDDEV),
    ARG("org.khronos.openvx.threshold",             VX_KERNEL_THRESHOLD),
    ARG("org.khronos.openvx.integral_image",        VX_KERNEL_INTEGRAL_IMAGE),
    ARG("org.khronos.openvx.dilate_3x3",            VX_KERNEL_DILATE_3x3),
    ARG("org.khronos.openvx.erode_3x3",             VX_KERNEL_ERODE_3x3),
    ARG("org.khronos.openvx.median_3x3",            VX_KERNEL_MEDIAN_3x3),
    ARG("org.khronos.openvx.box_3x3",               VX_KERNEL_BOX_3x3),
    ARG("org.khronos.openvx.gaussian_3x3",          VX_KERNEL_GAUSSIAN_3x3),
    ARG("org.khronos.openvx.custom_convolution",    VX_KERNEL_CUSTOM_CONVOLUTION),
    ARG("org.khronos.openvx.gaussian_pyramid",      VX_KERNEL_GAUSSIAN_PYRAMID),
    ARG("org.khronos.openvx.accumulate",            VX_KERNEL_ACCUMULATE),
    ARG("org.khronos.openvx.accumulate_weighted",   VX_KERNEL_ACCUMULATE_WEIGHTED),
    ARG("org.khronos.openvx.accumulate_square",     VX_KERNEL_ACCUMULATE_SQUARE),
    ARG("org.khronos.openvx.minmaxloc",             VX_KERNEL_MINMAXLOC),
    ARG("org.khronos.openvx.convertdepth",          VX_KERNEL_CONVERTDEPTH),
    ARG("org.khronos.openvx.canny_edge_detector",   VX_KERNEL_CANNY_EDGE_DETECTOR),
    ARG("org.khronos.openvx.and",                   VX_KERNEL_AND),
    ARG("org.khronos.openvx.or",                    VX_KERNEL_OR),
    ARG("org.khronos.openvx.xor",                   VX_KERNEL_XOR),
    ARG("org.khronos.openvx.not",                   VX_KERNEL_NOT),
    ARG("org.khronos.openvx.multiply",              VX_KERNEL_MULTIPLY),
    ARG("org.khronos.openvx.add",                   VX_KERNEL_ADD),
    ARG("org.khronos.openvx.subtract",              VX_KERNEL_SUBTRACT),
    ARG("org.khronos.openvx.warp_affine",           VX_KERNEL_WARP_AFFINE),
    ARG("org.khronos.openvx.warp_perspective",      VX_KERNEL_WARP_PERSPECTIVE),
    ARG("org.khronos.openvx.harris_corners",        VX_KERNEL_HARRIS_CORNERS),
    ARG("org.khronos.openvx.fast_corners",          VX_KERNEL_FAST_CORNERS),
    ARG("org.khronos.openvx.optical_flow_pyr_lk",   VX_KERNEL_OPTICAL_FLOW_PYR_LK),
    ARG("org.khronos.openvx.remap",                 VX_KERNEL_REMAP),
    ARG("org.khronos.openvx.halfscale_gaussian",    VX_KERNEL_HALFSCALE_GAUSSIAN),
    )
{
    vx_context context = context_->vx_context_;
    vx_kernel kernel   = 0;
    vx_enum   kernel_id = 0;

    EXPECT_VX_OBJECT(kernel = vxGetKernelByName(context, arg_->name), VX_TYPE_KERNEL);

    if (CT_HasFailure())
    {
        vx_char name[VX_MAX_KERNEL_NAME] = {0};

        ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, arg_->kernel_id), VX_TYPE_KERNEL);
        VX_CALL(vxQueryKernel(kernel, VX_KERNEL_NAME, &name, sizeof(name)));
        printf("\tExpected kernel name is: %s\n", arg_->name);
        printf("\tActual kernel name is:   %-*s\n", VX_MAX_KERNEL_NAME, name);
    }
    else
    {
        VX_CALL(vxQueryKernel(kernel, VX_KERNEL_ENUM, &kernel_id, sizeof(kernel_id)));
        EXPECT_EQ_INT(arg_->kernel_id, kernel_id);
    }

    VX_CALL(vxReleaseKernel(&kernel));
}

TEST(Graph, testAllocateUserKernelId)
{
    vx_context context = context_->vx_context_;
    vx_enum   kernel_id = 0;

    ASSERT_EQ_VX_STATUS(vxAllocateUserKernelId(NULL, &kernel_id), VX_ERROR_INVALID_REFERENCE);
    ASSERT_NE_VX_STATUS(vxAllocateUserKernelId(context, NULL), VX_SUCCESS);

    VX_CALL(vxAllocateUserKernelId(context, &kernel_id));

    ASSERT(kernel_id >= VX_KERNEL_BASE(VX_ID_USER, 0));
    ASSERT(kernel_id < (VX_KERNEL_BASE(VX_ID_USER, 0) + 4096));
}

TEST(Graph, testAllocateUserKernelLibraryId)
{
    vx_context context = context_->vx_context_;
    vx_enum   library_id = 0;

    ASSERT_EQ_VX_STATUS(vxAllocateUserKernelLibraryId(NULL, &library_id), VX_ERROR_INVALID_REFERENCE);
    ASSERT_NE_VX_STATUS(vxAllocateUserKernelLibraryId(context, NULL), VX_SUCCESS);

    VX_CALL(vxAllocateUserKernelLibraryId(context, &library_id));

    ASSERT(library_id >= 1);
    ASSERT(library_id <= 255);
}

void test_case_1(vx_context context, vx_uint32 width, vx_uint32 height)
{
    vx_image src1 = 0;
    vx_image src2 = 0;
    vx_image d1 = 0;
    vx_image d2 = 0;
    vx_image res = 0;
    vx_graph graph = 0;
    vx_node N1 = 0;
    vx_node N2 = 0;
    vx_node node3 = 0;
    vx_node node4 = 0;
    vx_pixel_value_t src1_val = {{ 1 }};
    vx_pixel_value_t src2_val = {{ 2 }};
    vx_rectangle_t roi_rect = { width / 2, height / 2, width, height };
    vx_enum convert_policy = VX_CONVERT_POLICY_SATURATE;
    vx_scalar mean1 = 0;
    vx_scalar stddev1 = 0;
    vx_float32 mean_val1 = 0.0f;
    vx_float32 stddev_val1 = 0.0f;
    vx_scalar mean2 = 0;
    vx_scalar stddev2 = 0;
    vx_float32 mean_val2 = 0.0f;
    vx_float32 stddev_val2 = 0.0f;

    ASSERT_VX_OBJECT(src1 = vxCreateUniformImage(context, width, height, VX_DF_IMAGE_U8, &src1_val), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateUniformImage(context, width, height, VX_DF_IMAGE_U8, &src2_val), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d1 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d2 = vxCreateImageFromROI(d1, &roi_rect), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(res = vxCreateImage(context, width / 2, height / 2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(mean1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean_val1), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(stddev1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev_val1), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(mean2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean_val2), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(stddev2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev_val2), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* add nodes in arbitrary order */
    ASSERT_VX_OBJECT(node3 = vxMeanStdDevNode(graph, res, mean1, stddev1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(N2 = vxNotNode(graph, d2, res), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxMeanStdDevNode(graph, d1, mean2, stddev2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(N1 = vxAddNode(graph, src1, src2, convert_policy, d1), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxCopyScalar(mean1, &mean_val1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyScalar(stddev1, &stddev_val1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyScalar(mean2, &mean_val2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyScalar(stddev2, &stddev_val2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    EXPECT_EQ_INT((vx_uint8)( ~(src1_val.U8 + src2_val.U8) ), (vx_uint8)mean_val1);
    EXPECT_EQ_INT((vx_uint8)((src1_val.U8 + src2_val.U8)), (vx_uint8)mean_val2);

    VX_CALL(vxReleaseNode(&N1));
    VX_CALL(vxReleaseNode(&N2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseImage(&d2));
    VX_CALL(vxReleaseImage(&res));
    VX_CALL(vxReleaseScalar(&mean1));
    VX_CALL(vxReleaseScalar(&stddev1));
    VX_CALL(vxReleaseScalar(&mean2));
    VX_CALL(vxReleaseScalar(&stddev2));

    ASSERT(N1 == 0);
    ASSERT(N2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(graph == 0);
    ASSERT(src1 == 0);
    ASSERT(src2 == 0);
    ASSERT(d1 == 0);
    ASSERT(d2 == 0);
    ASSERT(res == 0);
    ASSERT(mean1 == 0);
    ASSERT(stddev1 == 0);
    ASSERT(mean2 == 0);
    ASSERT(stddev2 == 0);

    return;
}


void test_case_2(vx_context context, vx_uint32 width, vx_uint32 height)
{
    vx_image d2 = 0;
    vx_image src1 = 0;
    vx_image src2 = 0;
    vx_image d1 = 0;
    vx_image res = 0;
    vx_graph graph = 0;
    vx_node N1 = 0;
    vx_node N2 = 0;
    vx_node node3 = 0;
    vx_node node4 = 0;
    vx_pixel_value_t d2_val = {{ 1 }};
    vx_pixel_value_t src1_val = {{ 2 }};
    vx_pixel_value_t src2_val = {{ 3 }};
    vx_rectangle_t img_rect = { 0, 0, width, height };
    vx_rectangle_t roi_rect = { width / 2, height / 2, width, height };
    vx_imagepatch_addressing_t addr = { 0 };
    vx_uint8* base_ptr = NULL;
    vx_enum convert_policy = VX_CONVERT_POLICY_SATURATE;
    vx_scalar mean1 = 0;
    vx_scalar stddev1 = 0;
    vx_float32 mean_val1 = 0.0f;
    vx_float32 stddev_val1 = 0.0f;
    vx_scalar mean2 = 0;
    vx_scalar stddev2 = 0;
    vx_float32 mean_val2 = 0.0f;
    vx_float32 stddev_val2 = 0.0f;
    vx_map_id map_id;
    vx_uint32 x, y;

    ASSERT_VX_OBJECT(mean1   = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean_val1), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(stddev1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev_val1), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(mean2   = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean_val2), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(stddev2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev_val2), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(d2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    VX_CALL(vxMapImagePatch(d2, &img_rect, 0, &map_id, &addr, (void**)&base_ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    for ( y = 0; y < addr.dim_y; y += addr.step_y)
    {
        for ( x = 0; x < addr.dim_x; x += addr.step_x)
        {
            vx_uint8* ptr = vxFormatImagePatchAddress2d(base_ptr, x, y, &addr);
            *ptr = d2_val.U8;
        }
    }
    VX_CALL(vxUnmapImagePatch(d2, map_id));

    ASSERT_VX_OBJECT(d1 = vxCreateImageFromROI(d2, &roi_rect), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateUniformImage(context, width / 2, height / 2, VX_DF_IMAGE_U8, &src1_val), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateUniformImage(context, width / 2, height / 2, VX_DF_IMAGE_U8, &src2_val), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(res = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* add nodes in arbitrary order */
    ASSERT_VX_OBJECT(node3 = vxMeanStdDevNode(graph, d1, mean2, stddev2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(N2 = vxMeanStdDevNode(graph, res, mean1, stddev1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxNotNode(graph, d2, res), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(N1 = vxAddNode(graph, src1, src2, convert_policy, d1), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxCopyScalar(mean1, &mean_val1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyScalar(stddev1, &stddev_val1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyScalar(mean2, &mean_val2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyScalar(stddev2, &stddev_val2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    EXPECT_EQ_INT((vx_uint8)(~(src1_val.U8 + src2_val.U8) / 2), (vx_uint8)mean_val1);
    EXPECT_EQ_INT((vx_uint8)((src1_val.U8 + src2_val.U8)), (vx_uint8)mean_val2);

    VX_CALL(vxReleaseNode(&N1));
    VX_CALL(vxReleaseNode(&N2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseImage(&d2));
    VX_CALL(vxReleaseImage(&res));
    VX_CALL(vxReleaseScalar(&mean1));
    VX_CALL(vxReleaseScalar(&stddev1));
    VX_CALL(vxReleaseScalar(&mean2));
    VX_CALL(vxReleaseScalar(&stddev2));

    ASSERT(N1 == 0);
    ASSERT(N2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(graph == 0);
    ASSERT(src1 == 0);
    ASSERT(src2 == 0);
    ASSERT(d1 == 0);
    ASSERT(d2 == 0);
    ASSERT(res == 0);
    ASSERT(mean1 == 0);
    ASSERT(stddev1 == 0);
    ASSERT(mean2 == 0);
    ASSERT(stddev2 == 0);

    return;
}


void test_case_3(vx_context context, vx_uint32 width, vx_uint32 height)
{
    vx_image image = 0;
    vx_image src1 = 0;
    vx_image src2 = 0;
    vx_image d1 = 0;
    vx_image d2 = 0;
    vx_graph graph = 0;
    vx_node N1 = 0;
    vx_node N2 = 0;
    vx_node node = 0;
    vx_pixel_value_t val = {{ 1 }};
    vx_pixel_value_t src1_val = {{ 2 }};
    vx_pixel_value_t src2_val = {{ 3 }};
    vx_rectangle_t img_rect = { 0, 0, width, height };
    vx_rectangle_t roi_rect1 = { width / 2, height / 2, width, height };
    vx_rectangle_t roi_rect2 = { 0, height / 2, width, height };
    vx_imagepatch_addressing_t addr = { 0 };
    vx_uint8* base_ptr = NULL;
    vx_enum convert_policy = VX_CONVERT_POLICY_SATURATE;
    vx_scalar mean1 = 0;
    vx_scalar stddev1 = 0;
    vx_float32 mean_val1 = 0.0f;
    vx_float32 stddev_val1 = 0.0f;
    vx_scalar mean2 = 0;
    vx_scalar stddev2 = 0;
    vx_float32 mean_val2 = 0.0f;
    vx_float32 stddev_val2 = 0.0f;
    vx_map_id map_id;
    vx_uint32 x, y;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    VX_CALL(vxMapImagePatch(image, &img_rect, 0, &map_id, &addr, (void**)&base_ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    for ( y = 0; y < addr.dim_y; y += addr.step_y)
    {
        for ( x = 0; x < addr.dim_x; x += addr.step_x)
        {
            vx_uint8* ptr = vxFormatImagePatchAddress2d(base_ptr, x, y, &addr);
            *ptr = val.U8;
        }
    }
    VX_CALL(vxUnmapImagePatch(image, map_id));

    ASSERT_VX_OBJECT(src1 = vxCreateUniformImage(context, width / 2, height / 2, VX_DF_IMAGE_U8, &src1_val), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateUniformImage(context, width / 2, height / 2, VX_DF_IMAGE_U8, &src2_val), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d1 = vxCreateImageFromROI(image, &roi_rect1), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d2 = vxCreateImageFromROI(image, &roi_rect2), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(mean1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean_val1), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(stddev1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev_val1), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(mean2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean_val2), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(stddev2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev_val2), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* add nodes in arbitrary order */
    ASSERT_VX_OBJECT(node = vxMeanStdDevNode(graph, d1, mean2, stddev2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(N2 = vxMeanStdDevNode(graph, d2, mean1, stddev1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(N1 = vxAddNode(graph, src1, src2, convert_policy, d1), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxCopyScalar(mean1, &mean_val1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyScalar(stddev1, &stddev_val1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyScalar(mean2, &mean_val2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyScalar(stddev2, &stddev_val2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    EXPECT_EQ_INT((vx_uint8)((val.U8 + src1_val.U8 + src2_val.U8) / 2), (vx_uint8)mean_val1);
    EXPECT_EQ_INT((vx_uint8)((src1_val.U8 + src2_val.U8)), (vx_uint8)mean_val2);

    VX_CALL(vxReleaseNode(&N1));
    VX_CALL(vxReleaseNode(&N2));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&d1));
    VX_CALL(vxReleaseImage(&d2));
    VX_CALL(vxReleaseScalar(&mean1));
    VX_CALL(vxReleaseScalar(&stddev1));
    VX_CALL(vxReleaseScalar(&mean2));
    VX_CALL(vxReleaseScalar(&stddev2));

    ASSERT(N1 == 0);
    ASSERT(N2 == 0);
    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(image == 0);
    ASSERT(src1 == 0);
    ASSERT(src2 == 0);
    ASSERT(d1 == 0);
    ASSERT(d2 == 0);
    ASSERT(mean1 == 0);
    ASSERT(stddev1 == 0);
    ASSERT(mean2 == 0);
    ASSERT(stddev2 == 0);

    return;
}


void test_case_4(vx_context context, vx_uint32 width, vx_uint32 height)
{
    vx_image image = 0;
    vx_pyramid d1 = 0;
    vx_image d2 = 0;
    vx_graph graph = 0;
    vx_node N1 = 0;
    vx_node N2 = 0;
    vx_pixel_value_t val = {{ 5 }};
    vx_size levels = 3;
    vx_float32 scale = VX_SCALE_PYRAMID_HALF;
    vx_scalar mean1 = 0;
    vx_scalar stddev1 = 0;
    vx_float32 mean_val1 = 0.0f;
    vx_float32 stddev_val1 = 0.0f;

    ASSERT_VX_OBJECT(image = vxCreateUniformImage(context, width, height, VX_DF_IMAGE_U8, &val), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(d1 = vxCreatePyramid(context, levels, scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(d2 = vxGetPyramidLevel(d1, 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(mean1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean_val1), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(stddev1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev_val1), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* add nodes in arbitrary order */
    ASSERT_VX_OBJECT(N2 = vxMeanStdDevNode(graph, d2, mean1, stddev1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(N1 = vxGaussianPyramidNode(graph, image, d1), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxCopyScalar(mean1, &mean_val1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyScalar(stddev1, &stddev_val1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    EXPECT_EQ_INT(val.U8, (vx_uint8)mean_val1);

    VX_CALL(vxReleaseNode(&N1));
    VX_CALL(vxReleaseNode(&N2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleasePyramid(&d1));
    VX_CALL(vxReleaseImage(&d2));
    VX_CALL(vxReleaseScalar(&mean1));
    VX_CALL(vxReleaseScalar(&stddev1));

    ASSERT(N1 == 0);
    ASSERT(N2 == 0);
    ASSERT(graph == 0);
    ASSERT(image == 0);
    ASSERT(d1 == 0);
    ASSERT(d2 == 0);
    ASSERT(mean1 == 0);
    ASSERT(stddev1 == 0);

    return;
}


TEST(Graph, testImageContainmentRelationship)
{
    vx_context context = context_->vx_context_;
    vx_uint32 width = 128;
    vx_uint32 height = 128;
    test_case_1(context, width, height);
    test_case_2(context, width, height);
    test_case_3(context, width, height);
    test_case_4(context, width, height);
}


/* test replicate node */

static CT_Image own_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
        image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

typedef enum
{
    ADD = 0,
    SUB,
    MUL,
    LUT

} OWN_OPERATION_TYPE;

typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int width;
    int height;
    vx_size levels;
    vx_float32 scale;
    vx_df_image format;
    OWN_OPERATION_TYPE op;
} Test_Replicate_Arg;


#define ADD_SIZE_SET(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=256x256", __VA_ARGS__, 256, 256)), \
    CT_EXPAND(nextmacro(testArgName "/sz=640x480", __VA_ARGS__, 640, 480))

#define ADD_VX_LEVELS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/REPLICAS=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/REPLICAS=4", __VA_ARGS__, 4))

#define ADD_VX_SCALE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/PYRAMID:SCALE_HALF", __VA_ARGS__, VX_SCALE_PYRAMID_HALF)), \
    CT_EXPAND(nextmacro(testArgName "/PYRAMID:SCALE_ORB", __VA_ARGS__, VX_SCALE_PYRAMID_ORB)), \
    CT_EXPAND(nextmacro(testArgName "/OBJECT_ARRAY", __VA_ARGS__, -1))

#define ADD_VX_FORMAT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U8", __VA_ARGS__, VX_DF_IMAGE_U8))

#define ADD_VX_OPERATION(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/Add", __VA_ARGS__, ADD)),  \
    CT_EXPAND(nextmacro(testArgName "/Sub", __VA_ARGS__, SUB)),  \
    CT_EXPAND(nextmacro(testArgName "/Mul", __VA_ARGS__, MUL)),  \
    CT_EXPAND(nextmacro(testArgName "/LUT", __VA_ARGS__, LUT))

#define TEST_REPLICATE_PARAMETERS \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_SET, ADD_VX_LEVELS, ADD_VX_SCALE, ADD_VX_FORMAT, ADD_VX_OPERATION, ARG, own_generate_random, NULL)

static void ref_replicate_op(vx_context context, vx_reference input1, vx_reference input2, vx_reference output, OWN_OPERATION_TYPE op)
{
    vx_uint32 i, k;
    vx_size levels = 0;
    vx_enum type = VX_TYPE_INVALID;

    VX_CALL(vxQueryReference(input1, VX_REFERENCE_TYPE, &type, sizeof(type)));

    if (type == VX_TYPE_PYRAMID)
    {
        VX_CALL(vxQueryPyramid((vx_pyramid)input1, VX_PYRAMID_LEVELS, &levels, sizeof(vx_size)));

        // add, sub, mul, lut
        for (k = 0; k < levels; k++)
        {
            vx_image src1 = 0;
            vx_image src2 = 0;
            vx_image dst = 0;
            vx_enum policy = VX_CONVERT_POLICY_SATURATE;
            vx_float32 scale_val = 1.0f;
            vx_enum rounding = VX_ROUND_POLICY_TO_ZERO;

            ASSERT_VX_OBJECT(src1 = vxGetPyramidLevel((vx_pyramid)input1, k), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(src2 = vxGetPyramidLevel((vx_pyramid)input2, k), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(dst = vxGetPyramidLevel((vx_pyramid)output, k), VX_TYPE_IMAGE);

            switch (op)
            {
            case ADD:
                VX_CALL(vxuAdd(context, src1, src2, policy, dst));
                break;

            case SUB:
                VX_CALL(vxuSubtract(context, src1, src2, policy, dst));
                break;

            case MUL:
                VX_CALL(vxuMultiply(context, src1, src2, scale_val, policy, rounding, dst));
                break;

            case LUT:
            {
                vx_lut lut = 0;
                vx_uint8* data = 0;
                vx_map_id map_id;
                ASSERT_VX_OBJECT(lut = vxCreateLUT(context, VX_TYPE_UINT8, 256), VX_TYPE_LUT);
                VX_CALL(vxMapLUT(lut, &map_id, (void **)&data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
                for (i = 0; i < 256; i++)
                    data[i] = 255 - i;
                VX_CALL(vxUnmapLUT(lut, map_id));
                VX_CALL(vxuTableLookup(context, src1, lut, dst));
                VX_CALL(vxReleaseLUT(&lut));
                break;
            }

            default:
                ASSERT(0);
            }

            VX_CALL(vxReleaseImage(&src1));
            VX_CALL(vxReleaseImage(&src2));
            VX_CALL(vxReleaseImage(&dst));
        }
    }
    else if (type == VX_TYPE_OBJECT_ARRAY)
    {
        VX_CALL(vxQueryObjectArray((vx_object_array)input1, VX_OBJECT_ARRAY_NUMITEMS, &levels, sizeof(vx_size)));

        // add, sub, mul, lut
        for (k = 0; k < levels; k++)
        {
            vx_image src1 = 0;
            vx_image src2 = 0;
            vx_image dst = 0;
            vx_enum policy = VX_CONVERT_POLICY_SATURATE;
            vx_float32 scale_val = 1.0f;
            vx_enum rounding = VX_ROUND_POLICY_TO_ZERO;

            ASSERT_VX_OBJECT(src1 = (vx_image)vxGetObjectArrayItem((vx_object_array)input1, k), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(src2 = (vx_image)vxGetObjectArrayItem((vx_object_array)input2, k), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(dst = (vx_image)vxGetObjectArrayItem((vx_object_array)output, k), VX_TYPE_IMAGE);

            switch (op)
            {
            case ADD:
                VX_CALL(vxuAdd(context, src1, src2, policy, dst));
                break;

            case SUB:
                VX_CALL(vxuSubtract(context, src1, src2, policy, dst));
                break;

            case MUL:
                VX_CALL(vxuMultiply(context, src1, src2, scale_val, policy, rounding, dst));
                break;

            case LUT:
            {
                vx_lut lut = 0;
                vx_uint8* data = 0;
                vx_map_id map_id;
                ASSERT_VX_OBJECT(lut = vxCreateLUT(context, VX_TYPE_UINT8, 256), VX_TYPE_LUT);
                VX_CALL(vxMapLUT(lut, &map_id, (void **)&data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
                for (i = 0; i < 256; i++)
                    data[i] = 255 - i - k;
                VX_CALL(vxUnmapLUT(lut, map_id));
                VX_CALL(vxuTableLookup(context, src1, lut, dst));
                VX_CALL(vxReleaseLUT(&lut));
                break;
            }

            default:
                ASSERT(0);
            }

            VX_CALL(vxReleaseImage(&src1));
            VX_CALL(vxReleaseImage(&src2));
            VX_CALL(vxReleaseImage(&dst));
        }
    }
    else
        ASSERT(0);

    return;
}

static void tst_replicate_op(vx_context context, vx_reference input1, vx_reference input2, vx_reference output, OWN_OPERATION_TYPE op)
{
    vx_graph graph = 0;
    vx_node node = 0;
    vx_object_array object_array = 0;
    vx_image src1 = 0;
    vx_image src2 = 0;
    vx_image dst = 0;
    vx_enum policy = VX_CONVERT_POLICY_SATURATE;
    vx_float32 scale_val = 1.0f;
    vx_scalar scale = 0;
    vx_enum rounding = VX_ROUND_POLICY_TO_ZERO;
    vx_size levels = 0;
    vx_enum type = VX_TYPE_INVALID;

    VX_CALL(vxQueryReference(input1, VX_REFERENCE_TYPE, &type, sizeof(type)));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    if (type == VX_TYPE_PYRAMID)
    {
        ASSERT_VX_OBJECT(src1 = vxGetPyramidLevel((vx_pyramid)input1, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src2 = vxGetPyramidLevel((vx_pyramid)input2, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst = vxGetPyramidLevel((vx_pyramid)output, 0), VX_TYPE_IMAGE);

        VX_CALL(vxQueryPyramid((vx_pyramid)input1, VX_PYRAMID_LEVELS, &levels, sizeof(levels)));

        // add, sub, mul, lut
        switch (op)
        {
        case ADD:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(node = vxAddNode(graph, src1, src2, policy, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node, replicate, 4));
            break;
        }

        case SUB:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(node = vxSubtractNode(graph, src1, src2, policy, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node, replicate, 4));
            break;
        }

        case MUL:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scale_val), VX_TYPE_SCALAR);
            ASSERT_VX_OBJECT(node = vxMultiplyNode(graph, src1, src2, scale, policy, rounding, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node, replicate, 6));
            VX_CALL(vxReleaseScalar(&scale));
            break;
        }

        case LUT:
        {
            vx_uint32 i;
            vx_lut lut = 0;
            vx_uint8* data = 0;
            vx_bool replicate[] = { vx_true_e, vx_false_e, vx_true_e };
            vx_map_id map_id;
            ASSERT_VX_OBJECT(lut = vxCreateLUT(context, VX_TYPE_UINT8, 256), VX_TYPE_LUT);
            VX_CALL(vxMapLUT(lut, &map_id, (void **)&data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
            for (i = 0; i < 256; i++)
                data[i] = 255 - i;
            VX_CALL(vxUnmapLUT(lut, map_id));
            ASSERT_VX_OBJECT(node = vxTableLookupNode(graph, src1, lut, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node, replicate, 3));
            VX_CALL(vxReleaseLUT(&lut));
            break;
        }

        default:
            ASSERT(0);
        }
    }
    else if (type == VX_TYPE_OBJECT_ARRAY)
    {
        ASSERT_VX_OBJECT(src1 = (vx_image)vxGetObjectArrayItem((vx_object_array)input1, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src2 = (vx_image)vxGetObjectArrayItem((vx_object_array)input2, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst = (vx_image)vxGetObjectArrayItem((vx_object_array)output, 0), VX_TYPE_IMAGE);

        VX_CALL(vxQueryObjectArray((vx_object_array)input1, VX_OBJECT_ARRAY_NUMITEMS, &levels, sizeof(levels)));

        // add, sub, mul, lut
        switch (op)
        {
        case ADD:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(node = vxAddNode(graph, src1, src2, policy, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node, replicate, 4));
            break;
        }

        case SUB:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(node = vxSubtractNode(graph, src1, src2, policy, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node, replicate, 4));
            break;
        }

        case MUL:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scale_val), VX_TYPE_SCALAR);
            ASSERT_VX_OBJECT(node = vxMultiplyNode(graph, src1, src2, scale, policy, rounding, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node, replicate, 6));
            VX_CALL(vxReleaseScalar(&scale));
            break;
        }

        case LUT:
        {
            vx_uint32 i, k;
            vx_lut lut = 0;
            vx_uint8* data = 0;
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_true_e };
            vx_map_id map_id;
            ASSERT_VX_OBJECT(lut = vxCreateLUT(context, VX_TYPE_UINT8, 256), VX_TYPE_LUT);
            ASSERT_VX_OBJECT(object_array = vxCreateObjectArray(context, (vx_reference)lut, levels), VX_TYPE_OBJECT_ARRAY);
            VX_CALL(vxReleaseLUT(&lut));

            for (k = 0; k < levels; ++k)
            {
                ASSERT_VX_OBJECT(lut = (vx_lut)vxGetObjectArrayItem(object_array, k), VX_TYPE_LUT);
                VX_CALL(vxMapLUT(lut, &map_id, (void **)&data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
                for (i = 0; i < 256; i++)
                    data[i] = 255 - i - k;
                VX_CALL(vxUnmapLUT(lut, map_id));
                VX_CALL(vxReleaseLUT(&lut));
            }

            ASSERT_VX_OBJECT(lut = (vx_lut)vxGetObjectArrayItem(object_array, 0), VX_TYPE_LUT);
            ASSERT_VX_OBJECT(node = vxTableLookupNode(graph, src1, lut, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node, replicate, 3));
            VX_CALL(vxReleaseLUT(&lut));
            break;
        }

        default:
            ASSERT(0);
        }
    }
    else
        ASSERT(0);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&dst));

    if (object_array)
        VX_CALL(vxReleaseObjectArray(&object_array));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    return;
}

static void check_replicas(vx_reference ref, vx_reference tst, vx_border_t border)
{
    vx_uint32 i;
    vx_size ref_levels = 0;
    vx_size tst_levels = 0;
    vx_enum type = VX_TYPE_INVALID;

    VX_CALL(vxQueryReference(ref, VX_REFERENCE_TYPE, &type, sizeof(type)));

    if (type == VX_TYPE_PYRAMID)
    {
        vx_float32 scale;
        VX_CALL(vxQueryPyramid((vx_pyramid)ref, VX_PYRAMID_LEVELS, &ref_levels, sizeof(vx_size)));
        VX_CALL(vxQueryPyramid((vx_pyramid)tst, VX_PYRAMID_LEVELS, &tst_levels, sizeof(vx_size)));
        VX_CALL(vxQueryPyramid((vx_pyramid)ref, VX_PYRAMID_SCALE, &scale, sizeof(scale)));
        EXPECT_EQ_INT(ref_levels, tst_levels);

        for (i = 0; i < ref_levels; i++)
        {
            vx_image src1 = 0;
            vx_image src2 = 0;
            CT_Image img1 = 0;
            CT_Image img2 = 0;

            ASSERT_VX_OBJECT(src1 = vxGetPyramidLevel((vx_pyramid)ref, i), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(src2 = vxGetPyramidLevel((vx_pyramid)tst, i), VX_TYPE_IMAGE);

            ASSERT_NO_FAILURE(img1 = ct_image_from_vx_image(src1));
            ASSERT_NO_FAILURE(img2 = ct_image_from_vx_image(src2));

            if (VX_BORDER_UNDEFINED == border.mode)
            {
                if (i > 0)
                {
                    if (VX_SCALE_PYRAMID_ORB == scale)
                    {
                        ct_adjust_roi(img1, 2, 2, 2, 2);
                        ct_adjust_roi(img2, 2, 2, 2, 2);
                    }
                    else if (VX_SCALE_PYRAMID_HALF == scale)
                    {
                        ct_adjust_roi(img1, 1, 1, 1, 1);
                        ct_adjust_roi(img2, 1, 1, 1, 1);
                    }
                }
            }

            EXPECT_EQ_CTIMAGE(img1, img2);

            VX_CALL(vxReleaseImage(&src1));
            VX_CALL(vxReleaseImage(&src2));
        }
    }
    else if (type == VX_TYPE_OBJECT_ARRAY)
    {
        VX_CALL(vxQueryObjectArray((vx_object_array)ref, VX_OBJECT_ARRAY_NUMITEMS, &ref_levels, sizeof(vx_size)));
        VX_CALL(vxQueryObjectArray((vx_object_array)tst, VX_OBJECT_ARRAY_NUMITEMS, &tst_levels, sizeof(vx_size)));
        EXPECT_EQ_INT(ref_levels, tst_levels);

        for (i = 0; i < ref_levels; i++)
        {
            vx_image src1 = 0;
            vx_image src2 = 0;
            CT_Image img1 = 0;
            CT_Image img2 = 0;

            ASSERT_VX_OBJECT(src1 = (vx_image)vxGetObjectArrayItem((vx_object_array)ref, i), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(src2 = (vx_image)vxGetObjectArrayItem((vx_object_array)tst, i), VX_TYPE_IMAGE);

            ASSERT_NO_FAILURE(img1 = ct_image_from_vx_image(src1));
            ASSERT_NO_FAILURE(img2 = ct_image_from_vx_image(src2));
            EXPECT_EQ_CTIMAGE(img1, img2);

            VX_CALL(vxReleaseImage(&src1));
            VX_CALL(vxReleaseImage(&src2));
        }
    }
    else
        ASSERT(0);

    return;
}


TEST_WITH_ARG(Graph, testReplicateNode, Test_Replicate_Arg, TEST_REPLICATE_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    CT_Image src = 0;
    vx_reference src1 = 0;
    vx_reference src2 = 0;
    vx_reference ref = 0;
    vx_reference tst = 0;
    vx_image input1 = 0;
    vx_image input2 = 0;
    vx_pixel_value_t value = {{ 2 }};
    vx_border_t border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    if (arg_->scale < 0)
    {
        vx_size i;
        ASSERT_VX_OBJECT(input1 = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(src1 = (vx_reference)vxCreateObjectArray(context, (vx_reference)input1, arg_->levels), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(src2 = (vx_reference)vxCreateObjectArray(context, (vx_reference)input1, arg_->levels), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(ref = (vx_reference)vxCreateObjectArray(context, (vx_reference)input1, arg_->levels), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(tst = (vx_reference)vxCreateObjectArray(context, (vx_reference)input1, arg_->levels), VX_TYPE_OBJECT_ARRAY);

        /* Initialize array items */
        for (i = 0; i < arg_->levels; ++i)
        {
            vx_image item = 0;
            value.U8 = (vx_uint8)i;
            ASSERT_VX_OBJECT(input2 = vxCreateUniformImage(context, arg_->width, arg_->height, arg_->format, &value), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(item = (vx_image)vxGetObjectArrayItem((vx_object_array)src1, (vx_uint32)i), VX_TYPE_IMAGE);
            VX_CALL(vxuAdd(context, input1, input2, VX_CONVERT_POLICY_WRAP, item));
            VX_CALL(vxReleaseImage(&item));
            VX_CALL(vxReleaseImage(&input2));
        }

        VX_CALL(vxReleaseImage(&input1));
    }
    else
    {
        ASSERT_VX_OBJECT(input1 = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(input2 = vxCreateUniformImage(context, arg_->width, arg_->height, arg_->format, &value), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(src1 = (vx_reference)vxCreatePyramid(context, arg_->levels, arg_->scale, arg_->width, arg_->height, arg_->format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(src2 = (vx_reference)vxCreatePyramid(context, arg_->levels, arg_->scale, arg_->width, arg_->height, arg_->format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(ref = (vx_reference)vxCreatePyramid(context, arg_->levels, arg_->scale, arg_->width, arg_->height, arg_->format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(tst = (vx_reference)vxCreatePyramid(context, arg_->levels, arg_->scale, arg_->width, arg_->height, arg_->format), VX_TYPE_PYRAMID);

        /* Initialize pyramids */
        VX_CALL(vxuGaussianPyramid(context, input1, (vx_pyramid)src1));
        VX_CALL(vxuGaussianPyramid(context, input2, (vx_pyramid)src2));

        VX_CALL(vxReleaseImage(&input1));
        VX_CALL(vxReleaseImage(&input2));
    }

    ref_replicate_op(context, src1, src2, ref, arg_->op);
    tst_replicate_op(context, src1, src2, tst, arg_->op);

    VX_CALL(vxQueryContext(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border)));
    check_replicas(ref, tst, border);

    VX_CALL(vxReleaseReference(&src1));
    VX_CALL(vxReleaseReference(&src2));
    VX_CALL(vxReleaseReference(&ref));
    VX_CALL(vxReleaseReference(&tst));
}


static void test_halfscalegaussian(vx_context context)
{
    vx_uint32 nrefs_before = 0;
    vx_uint32 nrefs_after = 0;
    vx_uint32 nrefs_dangling = 0;

    VX_CALL(vxQueryContext(context, VX_CONTEXT_REFERENCES, &nrefs_before, sizeof(nrefs_before)));

    {
        int count;
        vx_image src = 0;
        vx_image dst = 0;
        vx_graph graph = 0;
        vx_node node = 0;

        ASSERT_VX_OBJECT(src = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst = vxCreateImage(context, 160, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = vxHalfScaleGaussianNode(graph, src, dst, 3), VX_TYPE_NODE);

        for (count = 0; count < 2; count++)
        {
            VX_CALL(vxVerifyGraph(graph));
        }

        VX_CALL(vxReleaseImage(&src));
        VX_CALL(vxReleaseImage(&dst));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
    }

    VX_CALL(vxQueryContext(context, VX_CONTEXT_REFERENCES, &nrefs_after, sizeof(nrefs_after)));

    nrefs_dangling = nrefs_after - nrefs_before;
    ASSERT_EQ_INT(0, nrefs_dangling);

    return;
}


static void test_cannyedgedetector(vx_context context)
{
    vx_uint32 nrefs_before = 0;
    vx_uint32 nrefs_after = 0;
    vx_uint32 nrefs_dangling = 0;
    vx_uint32 nkernels_before = 0;
    vx_uint32 nkernels_after = 0;

    VX_CALL(vxQueryContext(context, VX_CONTEXT_REFERENCES, &nrefs_before, sizeof(nrefs_before)));
    VX_CALL(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, &nkernels_before, sizeof(nkernels_before)));

    {
        int count;
        vx_image src = 0;
        vx_image dst = 0;
        vx_graph graph = 0;
        vx_threshold threshold = 0;
        vx_int32 val1 = 16;
        vx_int32 val2 = 32;
        vx_node node = 0;

        ASSERT_VX_OBJECT(src = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(threshold = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8), VX_TYPE_THRESHOLD);
        VX_CALL(vxSetThresholdAttribute(threshold, VX_THRESHOLD_THRESHOLD_LOWER, &val1, sizeof(val1)));
        VX_CALL(vxSetThresholdAttribute(threshold, VX_THRESHOLD_THRESHOLD_UPPER, &val2, sizeof(val2)));
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = vxCannyEdgeDetectorNode(graph, src, threshold, 3, VX_NORM_L1, dst), VX_TYPE_NODE);

        for (count = 0; count < 10; count++)
        {
            VX_CALL(vxVerifyGraph(graph));
        }

        VX_CALL(vxReleaseImage(&src));
        VX_CALL(vxReleaseImage(&dst));
        VX_CALL(vxReleaseThreshold(&threshold));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
    }

    VX_CALL(vxQueryContext(context, VX_CONTEXT_REFERENCES, &nrefs_after, sizeof(nrefs_after)));
    VX_CALL(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, &nkernels_after, sizeof(nkernels_after)));

    nrefs_dangling = nrefs_after - nrefs_before;

    if(nkernels_after != nkernels_before)
        nrefs_dangling -= (nkernels_after - nkernels_before);

    ASSERT_EQ_INT(0, nrefs_dangling);

    return;
}


static void test_harriscorners(vx_context context)
{
    vx_uint32 nrefs_before = 0;
    vx_uint32 nrefs_after = 0;
    vx_uint32 nrefs_dangling = 0;
    vx_uint32 nkernels_before = 0;
    vx_uint32 nkernels_after = 0;

    VX_CALL(vxQueryContext(context, VX_CONTEXT_REFERENCES, &nrefs_before, sizeof(nrefs_before)));
    VX_CALL(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, &nkernels_before, sizeof(nkernels_before)));

    {
        int count;
        vx_image src = 0;
        vx_array dst = 0;
        vx_graph graph = 0;
        vx_size capacity = 1000;
        vx_scalar strength = 0;
        vx_float32 strength_val = 0.5f;
        vx_scalar min_distance = 0;
        vx_float32 min_distance_val = 0.5f;
        vx_scalar sensitivity = 0;
        vx_float32 sensitivity_val = 0.1f;
        vx_scalar num_corners = 0;
        vx_size num_corners_val = 10;
        vx_node node = 0;

        ASSERT_VX_OBJECT(src = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst = vxCreateArray(context, VX_TYPE_KEYPOINT, capacity), VX_TYPE_ARRAY);
        ASSERT_VX_OBJECT(strength = vxCreateScalar(context, VX_TYPE_FLOAT32, &strength_val), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(min_distance = vxCreateScalar(context, VX_TYPE_FLOAT32, &min_distance_val), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(sensitivity = vxCreateScalar(context, VX_TYPE_FLOAT32, &sensitivity_val), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(num_corners = vxCreateScalar(context, VX_TYPE_SIZE, &num_corners_val), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = vxHarrisCornersNode(graph, src, strength, min_distance, sensitivity, 3, 3, dst, num_corners), VX_TYPE_NODE);

        for (count = 0; count < 2; count++)
        {
            VX_CALL(vxVerifyGraph(graph));
        }

        VX_CALL(vxReleaseImage(&src));
        VX_CALL(vxReleaseArray(&dst));
        VX_CALL(vxReleaseScalar(&strength));
        VX_CALL(vxReleaseScalar(&min_distance));
        VX_CALL(vxReleaseScalar(&sensitivity));
        VX_CALL(vxReleaseScalar(&num_corners));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
    }

    VX_CALL(vxQueryContext(context, VX_CONTEXT_REFERENCES, &nrefs_after, sizeof(nrefs_after)));
    VX_CALL(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, &nkernels_after, sizeof(nkernels_after)));

    nrefs_dangling = nrefs_after - nrefs_before;

    if (nkernels_after != nkernels_before)
        nrefs_dangling -= (nkernels_after - nkernels_before);

    ASSERT_EQ_INT(0, nrefs_dangling);

    return;
}


static void test_gaussianpyramid(vx_context context)
{
    vx_uint32 nrefs_before = 0;
    vx_uint32 nrefs_after = 0;
    vx_uint32 nrefs_dangling = 0;
    vx_uint32 nkernels_before = 0;
    vx_uint32 nkernels_after = 0;

    VX_CALL(vxQueryContext(context, VX_CONTEXT_REFERENCES, &nrefs_before, sizeof(nrefs_before)));
    VX_CALL(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, &nkernels_before, sizeof(nkernels_before)));

    {
        int count;
        vx_image src = 0;
        vx_pyramid dst = 0;
        vx_size levels = 2;
        vx_float32 scale = VX_SCALE_PYRAMID_HALF;
        vx_graph graph = 0;
        vx_node node = 0;

        ASSERT_VX_OBJECT(src = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst = vxCreatePyramid(context, levels, scale, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = vxGaussianPyramidNode(graph, src, dst), VX_TYPE_NODE);

        for (count = 0; count < 2; count++)
        {
            VX_CALL(vxVerifyGraph(graph));
        }

        VX_CALL(vxReleaseImage(&src));
        VX_CALL(vxReleasePyramid(&dst));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
    }

    VX_CALL(vxQueryContext(context, VX_CONTEXT_REFERENCES, &nrefs_after, sizeof(nrefs_after)));
    VX_CALL(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, &nkernels_after, sizeof(nkernels_after)));

    nrefs_dangling = nrefs_after - nrefs_before;

    if (nkernels_after != nkernels_before)
        nrefs_dangling -= (nkernels_after - nkernels_before);

    ASSERT_EQ_INT(0, nrefs_dangling);

    return;
}


static void test_laplacianpyramid(vx_context context)
{
    vx_uint32 nrefs_before = 0;
    vx_uint32 nrefs_after = 0;
    vx_uint32 nrefs_dangling = 0;
    vx_uint32 nkernels_before = 0;
    vx_uint32 nkernels_after = 0;

    VX_CALL(vxQueryContext(context, VX_CONTEXT_REFERENCES, &nrefs_before, sizeof(nrefs_before)));
    VX_CALL(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, &nkernels_before, sizeof(nkernels_before)));

    {
        int count;
        vx_image src = 0;
        vx_image out = 0;
        vx_pyramid dst = 0;
        vx_size levels = 1;
        vx_float32 scale = VX_SCALE_PYRAMID_HALF;
        vx_graph graph = 0;
        vx_node node = 0;

        ASSERT_VX_OBJECT(src = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out = vxCreateImage(context, 160, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst = vxCreatePyramid(context, levels, scale, 320, 240, VX_DF_IMAGE_S16), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = vxLaplacianPyramidNode(graph, src, dst, out), VX_TYPE_NODE);

        for (count = 0; count < 2; count++)
        {
            VX_CALL(vxVerifyGraph(graph));
        }

        VX_CALL(vxReleaseImage(&src));
        VX_CALL(vxReleaseImage(&out));
        VX_CALL(vxReleasePyramid(&dst));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
    }

    VX_CALL(vxQueryContext(context, VX_CONTEXT_REFERENCES, &nrefs_after, sizeof(nrefs_after)));
    VX_CALL(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, &nkernels_after, sizeof(nkernels_after)));

    nrefs_dangling = nrefs_after - nrefs_before;

    if (nkernels_after != nkernels_before)
        nrefs_dangling -= (nkernels_after - nkernels_before);

    ASSERT_EQ_INT(0, nrefs_dangling);

    return;
}


static void test_laplacianreconstruct(vx_context context)
{
    vx_uint32 nrefs_before = 0;
    vx_uint32 nrefs_after = 0;
    vx_uint32 nrefs_dangling = 0;
    vx_uint32 nkernels_before = 0;
    vx_uint32 nkernels_after = 0;

    VX_CALL(vxQueryContext(context, VX_CONTEXT_REFERENCES, &nrefs_before, sizeof(nrefs_before)));
    VX_CALL(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, &nkernels_before, sizeof(nkernels_before)));

    {
        int count;
        vx_pyramid src = 0;
        vx_image in = 0;
        vx_image dst = 0;
        vx_size levels = 1;
        vx_float32 scale = VX_SCALE_PYRAMID_HALF;
        vx_graph graph = 0;
        vx_node node = 0;

        ASSERT_VX_OBJECT(src = vxCreatePyramid(context, levels, scale, 320, 240, VX_DF_IMAGE_S16), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(in = vxCreateImage(context, 160, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = vxLaplacianReconstructNode(graph, src, in, dst), VX_TYPE_NODE);

        for (count = 0; count < 2; count++)
        {
            VX_CALL(vxVerifyGraph(graph));
        }

        VX_CALL(vxReleasePyramid(&src));
        VX_CALL(vxReleaseImage(&in));
        VX_CALL(vxReleaseImage(&dst));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
    }

    VX_CALL(vxQueryContext(context, VX_CONTEXT_REFERENCES, &nrefs_after, sizeof(nrefs_after)));
    VX_CALL(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, &nkernels_after, sizeof(nkernels_after)));

    nrefs_dangling = nrefs_after - nrefs_before;

    if (nkernels_after != nkernels_before)
        nrefs_dangling -= (nkernels_after - nkernels_before);

    ASSERT_EQ_INT(0, nrefs_dangling);

    return;
}


static void test_opticalflowpyrlk(vx_context context)
{
    vx_uint32 nrefs_before = 0;
    vx_uint32 nrefs_after = 0;
    vx_uint32 nrefs_dangling = 0;

    VX_CALL(vxQueryContext(context, VX_CONTEXT_REFERENCES, &nrefs_before, sizeof(nrefs_before)));

    {
        int count;
        vx_size levels = 2;
        vx_float32 scale = VX_SCALE_PYRAMID_HALF;
        vx_pyramid old_images = 0;
        vx_pyramid new_images = 0;
        vx_size capacity = 1000;
        vx_array old_points = 0;
        vx_array new_points_estimates = 0;
        vx_array new_points = 0;
        vx_graph graph = 0;
        vx_scalar epsilon = 0;
        vx_float32 epsilon_val = 0.5f;
        vx_scalar num_iter = 0;
        vx_uint32 num_iter_val = 5;
        vx_scalar use_initial_estimate = 0;
        vx_bool use_initial_estimate_val = vx_true_e;
        vx_size window_dims = 10;
        vx_node node = 0;

        ASSERT_VX_OBJECT(old_images = vxCreatePyramid(context, levels, scale, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(new_images = vxCreatePyramid(context, levels, scale, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(old_points = vxCreateArray(context, VX_TYPE_KEYPOINT, capacity), VX_TYPE_ARRAY);
        ASSERT_VX_OBJECT(new_points_estimates = vxCreateArray(context, VX_TYPE_KEYPOINT, capacity), VX_TYPE_ARRAY);
        ASSERT_VX_OBJECT(new_points = vxCreateArray(context, VX_TYPE_KEYPOINT, capacity), VX_TYPE_ARRAY);
        ASSERT_VX_OBJECT(epsilon = vxCreateScalar(context, VX_TYPE_FLOAT32, &epsilon_val), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(num_iter = vxCreateScalar(context, VX_TYPE_UINT32, &num_iter_val), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(use_initial_estimate = vxCreateScalar(context, VX_TYPE_BOOL, &use_initial_estimate_val), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = vxOpticalFlowPyrLKNode(graph,
            old_images, new_images,
            old_points, new_points_estimates, new_points,
            VX_TERM_CRITERIA_BOTH, epsilon, num_iter, use_initial_estimate, window_dims), VX_TYPE_NODE);

        for (count = 0; count < 2; count++)
        {
            VX_CALL(vxVerifyGraph(graph));
        }

        VX_CALL(vxReleasePyramid(&old_images));
        VX_CALL(vxReleasePyramid(&new_images));
        VX_CALL(vxReleaseArray(&old_points));
        VX_CALL(vxReleaseArray(&new_points_estimates));
        VX_CALL(vxReleaseArray(&new_points));
        VX_CALL(vxReleaseScalar(&epsilon));
        VX_CALL(vxReleaseScalar(&num_iter));
        VX_CALL(vxReleaseScalar(&use_initial_estimate));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
    }

    VX_CALL(vxQueryContext(context, VX_CONTEXT_REFERENCES, &nrefs_after, sizeof(nrefs_after)));

    nrefs_dangling = nrefs_after - nrefs_before;
    ASSERT_EQ_INT(0, nrefs_dangling);

    return;
}


typedef enum
{
    fn_vxOpticalFlowPyrLKNode = 0,
    fn_vxGaussianPyramidNode,
    fn_vxCannyEdgeDetectorNode,
    fn_vxHarrisCornersNode,
    fn_vxHalfScaleGaussianNode,
    fn_vxLaplacianPyramidNode,
    fn_vxLaplacianReconstructNode

}  test_case;


typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    test_case test_id;

} ResourceLeakage_Arg;


#define ADD_TEST_CASES(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/vxOpticalFlowPyrLKNode",     __VA_ARGS__, fn_vxOpticalFlowPyrLKNode)), \
    CT_EXPAND(nextmacro(testArgName "/vxGaussianPyramidNode",      __VA_ARGS__, fn_vxGaussianPyramidNode)), \
    CT_EXPAND(nextmacro(testArgName "/vxCannyEdgeDetectorNode",    __VA_ARGS__, fn_vxCannyEdgeDetectorNode)), \
    CT_EXPAND(nextmacro(testArgName "/vxHarrisCornersNode",        __VA_ARGS__, fn_vxHarrisCornersNode)), \
    CT_EXPAND(nextmacro(testArgName "/vxHalfScaleGausianNode",     __VA_ARGS__, fn_vxHalfScaleGaussianNode)), \
    CT_EXPAND(nextmacro(testArgName "/vxLaplacianPyramidNode",     __VA_ARGS__, fn_vxLaplacianPyramidNode)), \
    CT_EXPAND(nextmacro(testArgName "/vxLaplacianReconstructNode", __VA_ARGS__, fn_vxLaplacianReconstructNode))

#define RESOURCE_LEAKAGE_PARAMETERS \
    CT_GENERATE_PARAMETERS("func", ADD_TEST_CASES, ARG, NULL)

TEST_WITH_ARG(Graph, testVerifyGraphLeak, ResourceLeakage_Arg, RESOURCE_LEAKAGE_PARAMETERS)
{
    vx_context context = context_->vx_context_;

    switch (arg_->test_id)
    {
    case fn_vxOpticalFlowPyrLKNode:     test_opticalflowpyrlk(context); break;
    case fn_vxGaussianPyramidNode:      test_gaussianpyramid(context);  break;
    case fn_vxCannyEdgeDetectorNode:    test_cannyedgedetector(context); break;
    case fn_vxHarrisCornersNode:        test_harriscorners(context); break;
    case fn_vxHalfScaleGaussianNode:    test_halfscalegaussian(context); break;
    case fn_vxLaplacianPyramidNode:     test_laplacianpyramid(context); break;
    case fn_vxLaplacianReconstructNode: test_laplacianreconstruct(context); break;
    default: break;
    }
    return;
}


static vx_graph graph_to_abandon = NULL;
static vx_status saved_graph_state_status = VX_FAILURE;
static vx_bool saved_graph_state_queried = vx_false_e;
static vx_enum saved_graph_state = VX_GRAPH_STATE_UNVERIFIED;

static vx_action VX_CALLBACK abandon_graph(vx_node node)
{
    vx_action action = VX_ACTION_ABANDON;

    if (graph_to_abandon != NULL)
    {
        saved_graph_state_status = vxQueryGraph(graph_to_abandon, VX_GRAPH_STATE, &saved_graph_state, sizeof(saved_graph_state));
        saved_graph_state_queried = vx_true_e;
    }

    return action;
}

TEST(Graph, testGraphState)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, interm_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_enum graph_state = VX_GRAPH_STATE_UNVERIFIED;
    int phase;

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(interm_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    VX_CALL(vxQueryGraph(graph, VX_GRAPH_STATE, &graph_state, sizeof(graph_state)));
    ASSERT(graph_state == VX_GRAPH_STATE_UNVERIFIED);

    ASSERT_VX_OBJECT(node1 = vxBox3x3Node(graph, src_image, interm_image), VX_TYPE_NODE);

    VX_CALL(vxQueryGraph(graph, VX_GRAPH_STATE, &graph_state, sizeof(graph_state)));
    ASSERT(graph_state == VX_GRAPH_STATE_UNVERIFIED);

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxQueryGraph(graph, VX_GRAPH_STATE, &graph_state, sizeof(graph_state)));
    ASSERT(graph_state == VX_GRAPH_STATE_VERIFIED);

    ASSERT_VX_OBJECT(node2 = vxIntegralImageNode(graph, interm_image, dst_image), VX_TYPE_NODE);

    VX_CALL(vxQueryGraph(graph, VX_GRAPH_STATE, &graph_state, sizeof(graph_state)));
    ASSERT(graph_state == VX_GRAPH_STATE_UNVERIFIED);

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxQueryGraph(graph, VX_GRAPH_STATE, &graph_state, sizeof(graph_state)));
    ASSERT(graph_state == VX_GRAPH_STATE_VERIFIED);

    for (phase = 0; phase < 2; ++phase)
    {
        vx_bool do_async = (vx_bool)(phase != 0);

        if (do_async)
        {
            VX_CALL(vxScheduleGraph(graph));
            VX_CALL(vxWaitGraph(graph));
        }
        else
        {
            VX_CALL(vxProcessGraph(graph));
        }

        VX_CALL(vxQueryGraph(graph, VX_GRAPH_STATE, &graph_state, sizeof(graph_state)));
        ASSERT(graph_state == VX_GRAPH_STATE_COMPLETED);

        graph_to_abandon = graph;
        saved_graph_state_status = VX_FAILURE;
        saved_graph_state_queried = vx_false_e;
        saved_graph_state = VX_GRAPH_STATE_UNVERIFIED;

        VX_CALL(vxAssignNodeCallback(node1, &abandon_graph));

        if (do_async)
        {
            VX_CALL(vxScheduleGraph(graph));
            ASSERT_EQ_VX_STATUS(VX_ERROR_GRAPH_ABANDONED, vxWaitGraph(graph));
        }
        else
        {
            ASSERT_EQ_VX_STATUS(VX_ERROR_GRAPH_ABANDONED, vxProcessGraph(graph));
        }

        ASSERT(saved_graph_state_queried == vx_true_e);
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, saved_graph_state_status);
        ASSERT(saved_graph_state == VX_GRAPH_STATE_RUNNING);

        VX_CALL(vxQueryGraph(graph, VX_GRAPH_STATE, &graph_state, sizeof(graph_state)));
        ASSERT(graph_state == VX_GRAPH_STATE_ABANDONED);

        VX_CALL(vxAssignNodeCallback(node1, NULL));
    }

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&interm_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node1 == 0); ASSERT(node2 == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0); ASSERT(interm_image == 0); ASSERT(src_image == 0);
}


TESTCASE_TESTS(Graph,
        testTwoNodes,
        testGraphFactory,
        testVirtualImage,
        testVirtualArray,
        testNodeRemove,
        testNodeFromEnum,
        testTwoNodesWithSameDst,
        testCycle,
        testCycle2,
        testMultipleRun,
        testMultipleRunAsync,
        DISABLED_testAsyncWaitWithoutSchedule,
        testNodePerformance,
        testGraphPerformance,
        testKernelName,
        testAllocateUserKernelId,
        testAllocateUserKernelLibraryId,
        testReplicateNode,
        testImageContainmentRelationship,
        testVerifyGraphLeak,
        testGraphState
        )
