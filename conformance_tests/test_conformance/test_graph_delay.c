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
    vx_image images[3];
    vx_node nodes[3];
    vx_delay delay = 0;
    vx_image delay_image_0 = 0;
    vx_image delay_image_1 = 0;
    vx_image node_image = 0;
    vx_parameter param = 0;
    vx_imagepatch_addressing_t addr;
    vx_uint8 *pdata = 0;
    vx_rectangle_t rect = {0, 0, 1, 1};
    vx_map_id map_id;

    ASSERT_VX_OBJECT(graph_0 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph_1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(images[0] = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(images[1] = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(images[2] = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)images[0], 2), VX_TYPE_DELAY);

    ASSERT_VX_OBJECT(delay_image_0 = (vx_image)vxGetReferenceFromDelay(delay, 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(delay_image_1 = (vx_image)vxGetReferenceFromDelay(delay,-1), VX_TYPE_IMAGE);


    /* image[0] gets 1 */
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(images[0], &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    *pdata = 1;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(images[0], map_id));

    /* Initialize the each delay slots with different values */
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(delay_image_0, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    /* Slot 0 gets 10 */
    *pdata = 10;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(delay_image_0, map_id));

    /* Slot -1 gets 2 */
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(delay_image_1, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    *pdata = 2;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(delay_image_1, map_id));

    ASSERT_VX_OBJECT(nodes[0] = vxAddNode(graph_0, images[0], (vx_image)vxGetReferenceFromDelay(delay, -1), VX_CONVERT_POLICY_WRAP, (vx_image)vxGetReferenceFromDelay(delay, 0)), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(nodes[1] = vxGaussian3x3Node(graph_0, (vx_image)vxGetReferenceFromDelay(delay, -1), images[1]), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(nodes[2] = vxGaussian3x3Node(graph_1, (vx_image)vxGetReferenceFromDelay(delay, 0), images[2]), VX_TYPE_NODE);

    VX_CALL(vxRegisterAutoAging(graph_0, delay));
    VX_CALL(vxRegisterAutoAging(graph_1, delay));
    VX_CALL(vxVerifyGraph(graph_0));
    VX_CALL(vxVerifyGraph(graph_1));

    /* 1 + 2 (slot -1) -> 3 (slot 0) */
    /* Ageing shifts slots: slot -1 = 3 ; slot 0 = 2 */
    VX_CALL(vxProcessGraph(graph_0));

    /* check if delay was really aged */

    /* Slot 0 */
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[0], 3), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(node_image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT(*pdata == 2);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(node_image, map_id));
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    /* Slot -1 */
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[0], 1), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(node_image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT(*pdata == 3);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(node_image, map_id));
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    /* Slot -1 */
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[1], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(node_image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT(*pdata == 3);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(node_image, map_id));
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    /* Slot 0 */
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[2], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(node_image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT(*pdata == 2);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(node_image, map_id));
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    /* check auto-aging multiple registration */
    VX_CALL(vxRegisterAutoAging(graph_0, delay)); /* Register auto-ageing a second time */
    VX_CALL(vxVerifyGraph(graph_0));
    VX_CALL(vxProcessGraph(graph_0));

    /* the delay must be aged once */
    /* 1 + 3 (slot -1) -> 4 (slot 0) */
    /* Ageing shifts slots: slot -1 = 4 ; slot 0 = 3 */

    /* Slot 0 */
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[0], 3), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    VX_CALL(vxReleaseParameter(&param));
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(node_image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT(*pdata == 3);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(node_image, map_id));
    VX_CALL(vxReleaseImage(&node_image));
    ASSERT(node_image == 0);

    /* Slot -1 */
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[0], 1), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(node_image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT(*pdata == 4);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(node_image, map_id));
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    /* Slot -1 */
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[1], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(node_image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT(*pdata == 4);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(node_image, map_id));
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    /* Slot 0 */
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[2], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(node_image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT(*pdata == 3);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(node_image, map_id));
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    /* check second graph */
    VX_CALL(vxProcessGraph(graph_1));

    /* the delay must be aged once more */
    /* Ageing shifts slots: slot -1 = 3 ; slot 0 = 4 */

    /* Slot 0 */
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[0], 3), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    VX_CALL(vxReleaseParameter(&param));
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(node_image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT(*pdata == 4);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(node_image, map_id));
    VX_CALL(vxReleaseImage(&node_image));
    ASSERT(node_image == 0);

    /* Slot -1 */
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[0], 1), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(node_image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT(*pdata == 3);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(node_image, map_id));
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    /* Slot -1 */
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[1], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(node_image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT(*pdata == 3);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(node_image, map_id));
    VX_CALL(vxReleaseImage(&node_image));
    VX_CALL(vxReleaseParameter(&param));
    ASSERT(node_image == 0);
    ASSERT(param == 0);

    /* Slot 0 */
    ASSERT_VX_OBJECT(param = vxGetParameterByIndex(nodes[2], 0), VX_TYPE_PARAMETER);
    VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &node_image, sizeof(node_image)));
    pdata = NULL;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(node_image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT(*pdata == 4);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(node_image, map_id));
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
