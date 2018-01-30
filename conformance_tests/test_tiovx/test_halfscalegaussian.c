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

#include <math.h> // floor

#define HALFSCALEGAUSSIAN_TOLERANCE 1

TESTCASE(tivxHalfScaleGaussian, CT_VXContext, ct_setup_vx_context, 0)

static CT_Image halfScaleGaussian_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static int32_t simple_get(CT_Image src, int32_t x, int32_t y, vx_border_t border,
        vx_df_image dst_format)
{
    int32_t value = 0;
    int valid_values = 0;

    ASSERT_NO_FAILURE_(return -1,
            valid_values = ct_image_read_rect_S32(src, &value, x, y, x, y, border));

    if (valid_values == 0)
        return INT32_MIN;

    if (dst_format == VX_DF_IMAGE_U8)
    {
        if (value < 0) value = 0;
        else if (value > UINT8_MAX) value = UINT8_MAX;
    }

    return value;
}

#define MAX_CONV_SIZE 15

static int32_t convolve_get(CT_Image src, int32_t x, int32_t y, vx_border_t border,
        int cols, int rows, vx_int16* data, vx_int32 scale, vx_df_image dst_format)
{
    int i;
    int32_t sum = 0, value = 0;
    int32_t src_data[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 };
    int valid_values = 0;

    ASSERT_(return -1, cols <= MAX_CONV_SIZE);
    ASSERT_(return -1, rows <= MAX_CONV_SIZE);

    ASSERT_NO_FAILURE_(return -1,
            valid_values = ct_image_read_rect_S32(src, src_data, x - cols / 2, y - rows / 2, x + cols / 2, y + rows / 2, border));

    if (valid_values == 0)
        return INT32_MIN;

    for (i = 0; i < cols * rows; ++i)
        sum += src_data[i] * data[i];

    value = sum / scale;

    if (dst_format == VX_DF_IMAGE_U8)
    {
        if (value < 0) value = 0;
        else if (value > UINT8_MAX) value = UINT8_MAX;
    }

    return value;
}

static vx_int16 gaussian3x3_kernel[9] = {
        1, 2, 1,
        2, 4, 2,
        1, 2, 1
};
static const vx_int32 gaussian3x3_scale = 16;

static vx_int16 gaussian5x5_kernel[25] = {
        1,  4,  6,  4, 1,
        4, 16, 24, 16, 4,
        6, 24, 36, 24, 6,
        4, 16, 24, 16, 4,
        1,  4,  6,  4, 1
};
static const vx_int32 gaussian5x5_scale = 256;

static int32_t halfScaleGaussian_get_pixel(CT_Image src, int x, int y, vx_int32 kernel_size, vx_border_t border)
{
    if (kernel_size == 1)
    {
        int32_t res = simple_get(src, x, y, border, VX_DF_IMAGE_U8);
        return res;
    }
    else if (kernel_size == 3)
    {
        int32_t res = convolve_get(src, x, y, border, 3, 3, gaussian3x3_kernel, gaussian3x3_scale, VX_DF_IMAGE_U8);
        return res;
    }
    else if (kernel_size == 5)
    {
        int32_t res = convolve_get(src, x, y, border, 5, 5, gaussian5x5_kernel, gaussian5x5_scale, VX_DF_IMAGE_U8);
        return res;
    }
    CT_FAIL_(return -1, "Invalid kernel size");
}

static int halfScaleGaussian_check_pixel(CT_Image src, CT_Image dst, int x, int y, vx_int32 kernel_size, vx_border_t border)
{
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(dst, x, y);
    vx_float64 x_src = (((vx_float64)x + 0.5f) * (vx_float64)src->width / (vx_float64)dst->width) - 0.5f;
    vx_float64 y_src = (((vx_float64)y + 0.5f) * (vx_float64)src->height / (vx_float64)dst->height) - 0.5f;
    int x_min = (int)floor(x_src), y_min = (int)floor(y_src);
    int sx, sy;
    for (sy = -1; sy <= 1; sy++)
    {
        for (sx = -1; sx <= 1; sx++)
        {
            vx_int32 candidate = 0;
            ASSERT_NO_FAILURE_(return 0, candidate = halfScaleGaussian_get_pixel(src, x_min + sx, y_min + sy, kernel_size, border));
            if (candidate < 0 || abs(candidate - res) <= HALFSCALEGAUSSIAN_TOLERANCE)
                return 1;
        }
    }
#if 0
    for (sy = -1; sy <= 1; sy++)
    {
        for (sx = -1; sx <= 1; sx++)
        {
            vx_int32 candidate = 0;
            ASSERT_NO_FAILURE_(return 0, candidate = halfScaleGaussian_get_pixel(src, x_min + sx, y_min + sy, kernel_size, border));
            printf("Check failed for pixel (%d+%d, %d+%d): %d\n", x, sx, y, sy, (int)candidate);
        }
    }
#endif
    CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
}

static void halfScaleGaussian_validate(CT_Image src, CT_Image dst, vx_int32 kernel_size, vx_border_t border)
{
    ASSERT(src && dst);
    CT_FILL_IMAGE_8U(, dst,
            {
                ASSERT_NO_FAILURE(halfScaleGaussian_check_pixel(src, dst, x, y, kernel_size, border));
            });
}

static void halfScaleGaussian_check(CT_Image src, CT_Image int_image, CT_Image dst0, CT_Image dst1, vx_int32 kernel_size, vx_border_t border)
{
    ASSERT(src && int_image && dst0 && dst1);
    EXPECT_EQ_INT((int)ceil(src->width * 0.25), dst0->width);
    EXPECT_EQ_INT((int)ceil(src->height * 0.25), dst0->height);
    halfScaleGaussian_validate(src, int_image, kernel_size, border);
    halfScaleGaussian_validate(int_image, dst0, kernel_size, border);
    halfScaleGaussian_validate(int_image, dst1, kernel_size, border);
#if 0
    if (CT_HasFailure())
    {
        printf("=== SRC ===\n");
        ct_dump_image_info_ex(src, 16, 8);
        printf("=== DST ===\n");
        ct_dump_image_info_ex(dst, 16, 8);
    }
#endif
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int width, height;
    vx_int32 kernel_size;
    vx_border_t border;
} Arg;


#define ADD_KERNEL_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/k=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/k=5", __VA_ARGS__, 5))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_18x18, ADD_KERNEL_SIZE, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ARG, halfScaleGaussian_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_644x258, ADD_KERNEL_SIZE, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ARG, halfScaleGaussian_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_1600x1200, ADD_KERNEL_SIZE, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ARG, halfScaleGaussian_generate_random, NULL)


TEST_WITH_ARG(tivxHalfScaleGaussian, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    int virt_width = 0, virt_height = 0, dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst0_image = 0, dst1_image = 0, virt, int_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_node4, perf_graph;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    CT_Image src = NULL, dst0 = NULL, dst1 = NULL, int_ctimage = NULL;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    virt_width = (src->width + 1) / 2;
    virt_height = (src->height + 1) / 2;
    dst_width = (virt_width + 1) / 2;
    dst_height = (virt_height + 1) / 2;

    ASSERT_VX_OBJECT(int_image = vxCreateImage(context, virt_width, virt_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst0_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst1_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, virt_width, virt_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxHalfScaleGaussianNode(graph, src_image, virt, arg_->kernel_size), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxHalfScaleGaussianNode(graph, virt, dst0_image, arg_->kernel_size), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxHalfScaleGaussianNode(graph, src_image, int_image, arg_->kernel_size), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxHalfScaleGaussianNode(graph, int_image, dst1_image, arg_->kernel_size), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));
    VX_CALL(vxSetNodeAttribute(node3, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));
    VX_CALL(vxSetNodeAttribute(node4, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxGetValidRegionImage(src_image, &src_rect);
    vxGetValidRegionImage(dst1_image, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), arg_->width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), arg_->height);

    if (arg_->kernel_size == 1)
    {
        ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), dst_width);
        ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), dst_height);
    }
    else if (arg_->kernel_size == 3)
    {
        ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), (dst_width-3));
        ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), (dst_height-3));
    }
    else if (arg_->kernel_size == 5) // -3 because -2 + -1; -2 on first downscale then -1 on second
    {
        ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), (dst_width-3));
        ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), (dst_height-3));
    }

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    ASSERT_NO_FAILURE(int_ctimage = ct_image_from_vx_image(int_image));
    ASSERT_NO_FAILURE(dst0 = ct_image_from_vx_image(dst0_image));
    ASSERT_NO_FAILURE(dst1 = ct_image_from_vx_image(dst1_image));

    ASSERT_NO_FAILURE(halfScaleGaussian_check(src, int_ctimage, dst0, dst1, arg_->kernel_size, arg_->border));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryNode(node4, VX_NODE_PERFORMANCE, &perf_node4, sizeof(perf_node4));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst1_image));
    VX_CALL(vxReleaseImage(&dst0_image));
    VX_CALL(vxReleaseImage(&int_image));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst1_image == 0);
    ASSERT(dst0_image == 0);
    ASSERT(int_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3, arg_->width*arg_->height, "N3");
    printPerformance(perf_node4, arg_->width*arg_->height, "N4");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

#define NEGATIVE_ADD_KERNEL_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/k=1", __VA_ARGS__, 1))

#define NEGATIVE_DIMENSIONS_PARAMETERS \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_18x18, NEGATIVE_ADD_KERNEL_SIZE, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ARG, halfScaleGaussian_generate_random, NULL)

TEST_WITH_ARG(tivxHalfScaleGaussian, negativeDimensionsTest, Arg,
    NEGATIVE_DIMENSIONS_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst0_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    CT_Image src = NULL, dst0 = NULL;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_width = arg_->width;
    dst_height = arg_->height;

    ASSERT_VX_OBJECT(dst0_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxHalfScaleGaussianNode(graph, src_image, dst0_image, arg_->kernel_size), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst0_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst0_image == 0);
    ASSERT(src_image == 0);
}

#define NEGATIVE_PARAMETERS \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_18x18, ADD_KERNEL_SIZE, ADD_VX_BORDERS_REQUIRE_CONSTANT_ONLY, ARG, halfScaleGaussian_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_644x258, ADD_KERNEL_SIZE, ADD_VX_BORDERS_REQUIRE_REPLICATE_ONLY, ARG, halfScaleGaussian_generate_random, NULL)

TEST_WITH_ARG(tivxHalfScaleGaussian, negativeTestBorderMode, Arg,
    NEGATIVE_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    int virt_width = 0, virt_height = 0, dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst0_image = 0, dst1_image = 0, virt, int_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;

    CT_Image src = NULL, dst0 = NULL, dst1 = NULL, int_ctimage = NULL;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    virt_width = (src->width + 1) / 2;
    virt_height = (src->height + 1) / 2;
    dst_width = (virt_width + 1) / 2;
    dst_height = (virt_height + 1) / 2;

    ASSERT_VX_OBJECT(int_image = vxCreateImage(context, virt_width, virt_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst0_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst1_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, virt_width, virt_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxHalfScaleGaussianNode(graph, src_image, virt, arg_->kernel_size), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxHalfScaleGaussianNode(graph, virt, dst0_image, arg_->kernel_size), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxHalfScaleGaussianNode(graph, src_image, int_image, arg_->kernel_size), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxHalfScaleGaussianNode(graph, int_image, dst1_image, arg_->kernel_size), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

    ASSERT_EQ_VX_STATUS(vxVerifyGraph(graph), VX_ERROR_NOT_SUPPORTED);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst1_image));
    VX_CALL(vxReleaseImage(&dst0_image));
    VX_CALL(vxReleaseImage(&int_image));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst1_image == 0);
    ASSERT(dst0_image == 0);
    ASSERT(int_image == 0);
    ASSERT(src_image == 0);
}

TESTCASE_TESTS(tivxHalfScaleGaussian,
        testGraphProcessing,
        negativeDimensionsTest,
        negativeTestBorderMode)
