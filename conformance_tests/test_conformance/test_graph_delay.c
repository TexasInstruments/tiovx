/* 
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>

TESTCASE(GraphDelay, CT_VXContext, ct_setup_vx_context, 0)

TEST(GraphDelay, testSimple)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    int w = 128, h = 128;
    vx_df_image f = VX_DF_IMAGE_U8;
    vx_image images[3];
    vx_node nodes[3];
    vx_delay delay = 0;
    int i;
    vx_size delay_count = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(images[0] = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(images[1] = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(images[2] = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)images[0], 2), VX_TYPE_DELAY);

    VX_CALL(vxQueryDelay(delay, VX_DELAY_SLOTS, &delay_count, sizeof(delay_count)));
    ASSERT(delay_count == 2);

    ASSERT_VX_OBJECT(nodes[0] = vxBox3x3Node(graph, images[0], (vx_image)vxGetReferenceFromDelay(delay, 0)), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(nodes[1] = vxMedian3x3Node(graph, (vx_image)vxGetReferenceFromDelay(delay, -1), images[1]), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(nodes[2] = vxGaussian3x3Node(graph, (vx_image)vxGetReferenceFromDelay(delay, -1), images[2]), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxAgeDelay(delay));

    VX_CALL(vxProcessGraph(graph));

    for (i = 0; i < 3; i++)
    {
        VX_CALL(vxReleaseNode(&nodes[i]));
    }
    for (i = 0; i < 3; i++)
    {
        VX_CALL(vxReleaseImage(&images[i]));
    }
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseDelay(&delay));

    ASSERT(graph == 0);
    ASSERT(delay == 0);
}

TEST(GraphDelay, testPyramid)
{
    int w = 128, h = 128;
    vx_df_image f = VX_DF_IMAGE_U8;
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_image input = 0;
    vx_image output = 0;
    vx_image image_pyr = 0;
    vx_image image_node = 0;
    vx_pyramid pyr = 0;
    vx_delay delay = 0;
    vx_node node_0 = 0;
    vx_node node_1 = 0;
    vx_parameter param = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(input = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output = vxCreateImage(context, w / 4, h / 4, f), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, 3, VX_SCALE_PYRAMID_HALF, w, h, f), VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)pyr, 2), VX_TYPE_DELAY);
    VX_CALL(vxReleasePyramid(&pyr));
    ASSERT(pyr == 0);

    ASSERT_VX_OBJECT(pyr = (vx_pyramid)vxGetReferenceFromDelay(delay, 0), VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(node_0 = vxGaussianPyramidNode(graph, input, pyr), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(image_pyr = vxGetPyramidLevel(pyr, 2), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node_1 = vxMedian3x3Node(graph, image_pyr, output), VX_TYPE_NODE);
    VX_CALL(vxReleaseImage(&image_pyr));
    ASSERT(image_pyr == 0);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));
    VX_CALL(vxAgeDelay(delay));

    ASSERT_VX_OBJECT(pyr = (vx_pyramid)vxGetReferenceFromDelay(delay, 0), VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(image_pyr = vxGetPyramidLevel(pyr, 2), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(node_1, 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &image_node, sizeof(image_node)));
    VX_CALL(vxReleaseParameter(&param));

    EXPECT_EQ_PTR(image_pyr, image_node);

    VX_CALL(vxReleaseImage(&image_node));
    VX_CALL(vxReleaseImage(&image_pyr));
    ASSERT(image_node == 0);
    ASSERT(image_pyr == 0);

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node_0));
    VX_CALL(vxReleaseNode(&node_1));
    ASSERT(node_0 == 0);
    ASSERT(node_1 == 0);

    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleaseImage(&output));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(graph == 0);
    ASSERT(delay == 0);
    ASSERT(input == 0);
    ASSERT(output == 0);
}

TEST(GraphDelay, testRegisterAutoAging)
{
    int i, w = 128, h = 128;
    vx_df_image f = VX_DF_IMAGE_U8;
    vx_context context = context_->vx_context_;
    vx_graph graph_0 = 0;
    vx_graph graph_1 = 0;
    vx_image images[4];
    vx_node nodes[4];
    vx_delay delay = 0;
    vx_image delay_image_0 = 0;
    vx_image delay_image_1 = 0;
    vx_image node_image = 0;
    vx_parameter param = 0;

    ASSERT_VX_OBJECT(graph_0 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph_1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(images[0] = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(images[1] = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(images[2] = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(images[3] = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)images[0], 2), VX_TYPE_DELAY);

    ASSERT_VX_OBJECT(delay_image_0 = (vx_image)vxGetReferenceFromDelay(delay, 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(delay_image_1 = (vx_image)vxGetReferenceFromDelay(delay,-1), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(nodes[0] = vxBox3x3Node(graph_0, images[0], (vx_image)vxGetReferenceFromDelay(delay, 0)), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(nodes[1] = vxMedian3x3Node(graph_0, (vx_image)vxGetReferenceFromDelay(delay, -1), images[1]), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(nodes[2] = vxGaussian3x3Node(graph_0, (vx_image)vxGetReferenceFromDelay(delay, -1), images[2]), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(nodes[3] = vxGaussian3x3Node(graph_1, (vx_image)vxGetReferenceFromDelay(delay, 0), images[3]), VX_TYPE_NODE);

    VX_CALL(vxRegisterAutoAging(graph_0, delay));
    VX_CALL(vxRegisterAutoAging(graph_1, delay));
    VX_CALL(vxVerifyGraph(graph_0));
    VX_CALL(vxVerifyGraph(graph_1));

    VX_CALL(vxProcessGraph(graph_0));

    // check if delay was really aged
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[0], 1), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    VX_CALL(vxReleaseParameter(&param));
    EXPECT_EQ_PTR(delay_image_1, node_image);
    VX_CALL(vxReleaseImage(&node_image));
    ASSERT(node_image == 0);

    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[1], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    EXPECT_EQ_PTR(delay_image_0, node_image);
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[2], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    EXPECT_EQ_PTR(delay_image_0, node_image);
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[3], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    EXPECT_EQ_PTR(delay_image_1, node_image);
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    // check auto-aging multiple registration
    VX_CALL(vxRegisterAutoAging(graph_0, delay));
    VX_CALL(vxRegisterAutoAging(graph_0, delay));
    VX_CALL(vxVerifyGraph(graph_0));
    VX_CALL(vxProcessGraph(graph_0));

    // the delay must be aged once
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[0], 1), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    VX_CALL(vxReleaseParameter(&param));
    EXPECT_EQ_PTR(delay_image_0, node_image);
    VX_CALL(vxReleaseImage(&node_image));
    ASSERT(node_image == 0);

    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[1], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    EXPECT_EQ_PTR(delay_image_1, node_image);
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[2], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    EXPECT_EQ_PTR(delay_image_1, node_image);
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[3], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    EXPECT_EQ_PTR(delay_image_0, node_image);
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    // check second graph
    VX_CALL(vxProcessGraph(graph_1));

    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[0], 1), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    VX_CALL(vxReleaseParameter(&param));
    EXPECT_EQ_PTR(delay_image_1, node_image);
    VX_CALL(vxReleaseImage(&node_image));
    ASSERT(node_image == 0);

    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[1], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    EXPECT_EQ_PTR(delay_image_0, node_image);
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[2], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    EXPECT_EQ_PTR(delay_image_0, node_image);
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[3], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    EXPECT_EQ_PTR(delay_image_1, node_image);
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    for (i = 0; i < (sizeof(nodes)/sizeof(nodes[0])); i++)
    {
        VX_CALL(vxReleaseNode(&nodes[i]));
    }

    for (i = 0; i < (sizeof(images)/sizeof(images[0])); i++)
    {
        VX_CALL(vxReleaseImage(&images[i]));
    }
    
    VX_CALL(vxReleaseGraph(&graph_0));
    VX_CALL(vxReleaseGraph(&graph_1));
    VX_CALL(vxReleaseDelay(&delay));

    ASSERT(graph_0 == 0);
    ASSERT(graph_1 == 0);
    ASSERT(delay == 0);
}

TESTCASE_TESTS(
    GraphDelay,
    testSimple,
    testPyramid,
    testRegisterAutoAging
    )
