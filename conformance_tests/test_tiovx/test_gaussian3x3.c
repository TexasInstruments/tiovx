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
#include <VX/vx.h>

#define MAX_NODES 10

TESTCASE(tivxGaussian3x3, CT_VXContext, ct_setup_vx_context, 0)


// Generate input to cover these requirements:
// There should be a image with randomly generated pixel intensities.
static CT_Image gaussian3x3_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static int32_t gaussian_get(int32_t *values)
{
    int32_t res = (values[0] + values[2] + values[6] + values[8]) * 1 +
                (values[1] + values[3] + values[5] + values[7]) * 2 +
                (values[4]) * 4;
    res = res >> 4;
    return res;
}

static uint8_t gaussian3x3_calculate(CT_Image src, uint32_t x, uint32_t y)
{
    int32_t values[9] = {
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 1)
    };
    return (uint8_t)gaussian_get(values);
}

static uint8_t gaussian3x3_calculate_replicate(CT_Image src, uint32_t x_, uint32_t y_)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 1)
    };
    return (uint8_t)gaussian_get(values);
}

static uint8_t gaussian3x3_calculate_constant(CT_Image src, uint32_t x_, uint32_t y_, vx_uint32 constant_value)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 1, constant_value)
    };
    return (uint8_t)gaussian_get(values);
}


static CT_Image gaussian3x3_create_reference_image(CT_Image src, vx_border_t border)
{
    CT_Image dst;

    CT_ASSERT_(return NULL, src->format == VX_DF_IMAGE_U8);

    dst = ct_allocate_image(src->width, src->height, src->format);

    if (border.mode == VX_BORDER_UNDEFINED)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                if (x >= 1 && y >= 1 && x < src->width - 1 && y < src->height - 1)
                {
                    uint8_t res = gaussian3x3_calculate(src, x, y);
                    *dst_data = res;
                });
    }
    else if (border.mode == VX_BORDER_REPLICATE)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = gaussian3x3_calculate_replicate(src, x, y);
                    *dst_data = res;
                });
    }
    else if (border.mode == VX_BORDER_CONSTANT)
    {
        vx_uint32 constant_value = border.constant_value.U32;
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = gaussian3x3_calculate_constant(src, x, y, constant_value);
                    *dst_data = res;
                });
    }
    else
    {
        ASSERT_(return 0, 0);
    }
    return dst;
}


static void gaussian3x3_check(CT_Image src, CT_Image dst, vx_border_t border)
{
    CT_Image dst_ref = NULL;

    ASSERT(src && dst);

    ASSERT_NO_FAILURE(dst_ref = gaussian3x3_create_reference_image(src, border));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst,  1, 1, 1, 1);
            ct_adjust_roi(dst_ref, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
}

static void sequential_gaussian3x3_check(CT_Image src, CT_Image dst, vx_border_t border)
{
    CT_Image dst_ref = NULL, virt_ref = NULL;

    ASSERT(src && dst);

    ASSERT_NO_FAILURE(virt_ref = gaussian3x3_create_reference_image(src, border));

    if (NULL != virt_ref)
    {
        ASSERT_NO_FAILURE(dst_ref = gaussian3x3_create_reference_image(virt_ref, border));

        ASSERT_NO_FAILURE(
            if (border.mode == VX_BORDER_UNDEFINED)
            {
                ct_adjust_roi(virt_ref, 1, 1, 1, 1);
                ct_adjust_roi(dst,  2, 2, 2, 2);
                ct_adjust_roi(dst_ref, 2, 2, 2, 2);
            }
        );

        EXPECT_CTIMAGE_NEAR(dst_ref, dst, 1);
    }
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    vx_border_t border;
    int width, height;
} Filter_Arg;

#define GAUSSIAN_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_18x18, ARG, gaussian3x3_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_644x258, ARG, gaussian3x3_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_1600x1200, ARG, gaussian3x3_generate_random, NULL)

TEST_WITH_ARG(tivxGaussian3x3, testGraphProcessing, Filter_Arg,
    GAUSSIAN_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    node1 = vxGaussian3x3Node(graph, src_image, virt);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    node2 = vxGaussian3x3Node(graph, virt, dst_image);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(sequential_gaussian3x3_check(src, dst, border));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TEST_WITH_ARG(tivxGaussian3x3, testValidRegion, Filter_Arg,
    GAUSSIAN_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;
    vx_rectangle_t rect;
    vx_bool valid_rect;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    node1 = vxGaussian3x3Node(graph, src_image, virt);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    node2 = vxGaussian3x3Node(graph, virt, dst_image);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(sequential_gaussian3x3_check(src, dst, border));

    vxGetValidRegionImage(dst_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

#define GAUSSIAN_PARAMETERS_NEGATIVE \
    CT_GENERATE_PARAMETERS("bi_level", ADD_VX_BORDERS_REQUIRE_REPLICATE_ONLY, ADD_SIZE_18x18, ARG, gaussian3x3_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("bi_level", ADD_VX_BORDERS_REQUIRE_CONSTANT_ONLY, ADD_SIZE_1600x1200, ARG, gaussian3x3_generate_random, NULL)

TEST_WITH_ARG(tivxGaussian3x3, negativeTestBorderMode, Filter_Arg,
    GAUSSIAN_PARAMETERS_NEGATIVE
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = ct_create_similar_image(src_image), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxGaussian3x3Node(graph, src_image, virt), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxGaussian3x3Node(graph, virt, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_EQ_VX_STATUS(vxVerifyGraph(graph), VX_ERROR_NOT_SUPPORTED);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

#define SUPERNODE_GAUSSIAN_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_644x258, ARG, gaussian3x3_generate_random, NULL)

TEST_WITH_ARG(tivxGaussian3x3, testGaussian3x3Supernode, Filter_Arg,
    SUPERNODE_GAUSSIAN_PARAMETERS
)
{
    int node_count = 2;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_rectangle_t rect;
    vx_bool valid_rect;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    node1 = vxGaussian3x3Node(graph, src_image, virt);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    node2 = vxGaussian3x3Node(graph, virt, dst_image);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_NO_FAILURE(node_list[0] = node1); 
    ASSERT_NO_FAILURE(node_list[1] = node2);
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);
    vxQueryNode(node2, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(sequential_gaussian3x3_check(src, dst, border));

    vxGetValidRegionImage(dst_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_super_node, arg_->width * arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
}

#ifdef BUILD_BAM
#define testGaussian3x3Supernode testGaussian3x3Supernode
#else
#define testGaussian3x3Supernode DISABLED_testGaussian3x3Supernode
#endif

TESTCASE_TESTS(tivxGaussian3x3, 
               testGraphProcessing, 
               testValidRegion, 
               negativeTestBorderMode,
               testGaussian3x3Supernode)
