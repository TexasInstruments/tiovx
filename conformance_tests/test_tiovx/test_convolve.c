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
#include <string.h>
#include "test_utils_mem_operations.h"

#define MAX_CONV_SIZE 15
#define MAX_NODES     10

TESTCASE(tivxConvolve, CT_VXContext, ct_setup_vx_context, 0)

static vx_convolution convolution_create(vx_context context, int cols, int rows, vx_int16* data, vx_uint32 scale)
{
    vx_convolution convolution = vxCreateConvolution(context, cols, rows);
    vx_size size = 0;

    ASSERT_VX_OBJECT_(return 0, convolution, VX_TYPE_CONVOLUTION);

    VX_CALL_(return 0, vxQueryConvolution(convolution, VX_CONVOLUTION_SIZE, &size, sizeof(size)));

    VX_CALL_(return 0, vxCopyConvolutionCoefficients(convolution, data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL_(return 0, vxSetConvolutionAttribute(convolution, VX_CONVOLUTION_SCALE, &scale, sizeof(scale)));

    return convolution;
}

static void convolution_data_fill_random_128(int cols, int rows, vx_int16* data)
{
    uint64_t* seed = &CT()->seed_;
    int i;

    for (i = 0; i < cols * rows; i++)
        data[i] = (vx_uint8)CT_RNG_NEXT_INT(*seed, (uint32_t)-128, 128);
}

static CT_Image convolve_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static CT_Image convolve_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

static int32_t convolve_get(CT_Image src, int32_t x, int32_t y, vx_border_t border,
        int cols, int rows, vx_int16* data, vx_uint32 scale, vx_df_image dst_format)
{
    int i;
    int32_t sum = 0, value = 0;
    int32_t src_data[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 };

    ASSERT_(return 0, cols <= MAX_CONV_SIZE);
    ASSERT_(return 0, rows <= MAX_CONV_SIZE);

    ASSERT_NO_FAILURE_(return 0,
            ct_image_read_rect_S32(src, src_data, x - cols / 2, y - rows / 2, x + cols / 2, y + rows / 2, border));

    for (i = 0; i < cols * rows; ++i)
        sum += src_data[i] * data[cols * rows - 1 - i];

    value = sum / scale;

    if (dst_format == VX_DF_IMAGE_U8)
    {
        if (value < 0) value = 0;
        else if (value > UINT8_MAX) value = UINT8_MAX;
    }
    else if (dst_format == VX_DF_IMAGE_S16)
    {
        if (value < INT16_MIN) value = INT16_MIN;
        else if (value > INT16_MAX) value = INT16_MAX;
    }

    return value;
}


static CT_Image convolve_create_reference_image(CT_Image src, vx_border_t border,
        int cols, int rows, vx_int16* data, vx_uint32 scale, vx_df_image dst_format)
{
    CT_Image dst;

    CT_ASSERT_(return NULL, src->format == VX_DF_IMAGE_U8);

    dst = ct_allocate_image(src->width, src->height, dst_format);

    if (dst_format == VX_DF_IMAGE_U8)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    int32_t res = convolve_get(src, x, y, border, cols, rows, data, scale, dst_format);
                    *dst_data = (vx_uint8)res;
                });
    }
    else if (dst_format == VX_DF_IMAGE_S16)
    {
        CT_FILL_IMAGE_16S(return 0, dst,
                {
                    int32_t res = convolve_get(src, x, y, border, cols, rows, data, scale, dst_format);
                    *dst_data = (vx_int16)res;
                });
    }
    else
    {
        CT_FAIL_(return 0, "NOT IMPLEMENTED");
    }
    return dst;
}


static void convolve_check(CT_Image src, CT_Image dst, vx_border_t border,
        int cols, int rows, vx_int16* data, vx_uint32 scale, vx_df_image dst_format)
{
    CT_Image dst_ref = NULL;

    ASSERT(src && dst);

    ASSERT_NO_FAILURE(dst_ref = convolve_create_reference_image(src, border, cols, rows, data, scale, dst_format));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst, cols / 2, rows / 2, cols / 2, rows / 2);
            ct_adjust_roi(dst_ref, cols / 2, rows / 2, cols / 2, rows / 2);
        }
    );

    EXPECT_CTIMAGE_NEAR(dst_ref, dst, 1);
}

static void convolve_sequential_check(CT_Image src, CT_Image dst, vx_border_t border,
        int cols, int rows, vx_int16* data1, vx_int16* data2, vx_uint32 scale, vx_df_image dst_format)
{
    CT_Image virt_ref = NULL, dst_ref = NULL;

    ASSERT(src && dst);

    ASSERT_NO_FAILURE(virt_ref = convolve_create_reference_image(src, border, cols, rows, data1, scale, dst_format));
    if (NULL != virt_ref)
    {
        ASSERT_NO_FAILURE(dst_ref = convolve_create_reference_image(virt_ref, border, cols, rows, data2, scale, dst_format));

        ASSERT_NO_FAILURE(
            if (border.mode == VX_BORDER_UNDEFINED)
            {
                ct_adjust_roi(dst, cols - 1, rows - 1, cols - 1, rows - 1);
                ct_adjust_roi(dst_ref, cols - 1, rows - 1, cols - 1, rows - 1);
            }
        );

        EXPECT_CTIMAGE_NEAR(dst_ref, dst, 1);
    }
}


typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int cols, rows;
    vx_uint32 scale;
    void (*convolution_data_generator)(int cols, int rows, vx_int16* data);
    vx_df_image dst_format;
    vx_border_t border;
    int width, height;
} Arg;


#define ADD_CONV_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv=3x3", __VA_ARGS__, 3, 3))

#define ADD_CONV_SCALE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv_scale=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/conv_scale=8", __VA_ARGS__, 8)), \
    CT_EXPAND(nextmacro(testArgName "/conv_scale=256", __VA_ARGS__, 256))

#define ADD_CONV_GENERATORS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv_fill=random128", __VA_ARGS__, convolution_data_fill_random_128))

#define ADD_CONV_DST_FORMAT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dst8U", __VA_ARGS__, VX_DF_IMAGE_U8))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_CONV_SIZE, ADD_CONV_SCALE, ADD_CONV_GENERATORS, ADD_CONV_DST_FORMAT, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_18x18, ARG, convolve_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_CONV_SIZE, ADD_CONV_SCALE, ADD_CONV_GENERATORS, ADD_CONV_DST_FORMAT, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_644x258, ARG, convolve_generate_random, NULL)

TEST_WITH_ARG(tivxConvolve, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt;
    vx_convolution convolution1 = 0, convolution2 = 0;
    vx_int16 data1[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 }, data2[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 };
    vx_size conv_max_dim = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, src->width, src->height, arg_->dst_format), VX_TYPE_IMAGE);

    VX_CALL(vxQueryContext(context, VX_CONTEXT_CONVOLUTION_MAX_DIMENSION, &conv_max_dim, sizeof(conv_max_dim)));

    if ((vx_size)arg_->cols > conv_max_dim || (vx_size)arg_->rows > conv_max_dim)
    {
        printf("%dx%d convolution is not supported. Skip test\n", (int)arg_->cols, (int)arg_->rows);
        return;
    }

    ASSERT_NO_FAILURE(arg_->convolution_data_generator(arg_->cols, arg_->rows, data1));
    ASSERT_NO_FAILURE(convolution1 = convolution_create(context, arg_->cols, arg_->rows, data1, arg_->scale));

    ASSERT_NO_FAILURE(arg_->convolution_data_generator(arg_->cols, arg_->rows, data2));
    ASSERT_NO_FAILURE(convolution2 = convolution_create(context, arg_->cols, arg_->rows, data2, arg_->scale));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, 0, 0, arg_->dst_format), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxConvolveNode(graph, src_image, convolution1, virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxConvolveNode(graph, virt, convolution2, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src_image, &src_rect);
    vxGetValidRegionImage(dst_image, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), arg_->width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), arg_->height);

    ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), (arg_->width - 2*(arg_->cols-1)));
    ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), (arg_->height - 2*(arg_->rows-1)));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(convolve_sequential_check(src, dst, border, arg_->cols, arg_->rows, data1, data2, arg_->scale, arg_->dst_format));

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

    VX_CALL(vxReleaseConvolution(&convolution1));
    VX_CALL(vxReleaseConvolution(&convolution2));
    ASSERT(convolution1 == NULL);
    ASSERT(convolution2 == NULL);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

#define NEGATIVE_ADD_CONV_SCALE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv_scale=1", __VA_ARGS__, 1))

#define  NEGATIVE_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_CONV_SIZE, NEGATIVE_ADD_CONV_SCALE, ADD_CONV_GENERATORS, ADD_CONV_DST_FORMAT, ADD_VX_BORDERS_REQUIRE_CONSTANT_ONLY, ADD_SIZE_18x18, ARG, convolve_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_CONV_SIZE, NEGATIVE_ADD_CONV_SCALE, ADD_CONV_GENERATORS, ADD_CONV_DST_FORMAT, ADD_VX_BORDERS_REQUIRE_REPLICATE_ONLY, ADD_SIZE_644x258, ARG, convolve_generate_random, NULL), \

TEST_WITH_ARG(tivxConvolve, negativeTestBorderMode, Arg,
    NEGATIVE_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt;
    vx_convolution convolution1 = 0, convolution2 = 0;
    vx_int16 data1[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 }, data2[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 };
    vx_size conv_max_dim = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, src->width, src->height, arg_->dst_format), VX_TYPE_IMAGE);

    VX_CALL(vxQueryContext(context, VX_CONTEXT_CONVOLUTION_MAX_DIMENSION, &conv_max_dim, sizeof(conv_max_dim)));

    if ((vx_size)arg_->cols > conv_max_dim || (vx_size)arg_->rows > conv_max_dim)
    {
        printf("%dx%d convolution is not supported. Skip test\n", (int)arg_->cols, (int)arg_->rows);
        return;
    }

    ASSERT_NO_FAILURE(arg_->convolution_data_generator(arg_->cols, arg_->rows, data1));
    ASSERT_NO_FAILURE(convolution1 = convolution_create(context, arg_->cols, arg_->rows, data1, arg_->scale));

    ASSERT_NO_FAILURE(arg_->convolution_data_generator(arg_->cols, arg_->rows, data2));
    ASSERT_NO_FAILURE(convolution2 = convolution_create(context, arg_->cols, arg_->rows, data2, arg_->scale));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, 0, 0, arg_->dst_format), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxConvolveNode(graph, src_image, convolution1, virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxConvolveNode(graph, virt, convolution2, dst_image), VX_TYPE_NODE);

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

    VX_CALL(vxReleaseConvolution(&convolution1));
    VX_CALL(vxReleaseConvolution(&convolution2));
    ASSERT(convolution1 == NULL);
    ASSERT(convolution2 == NULL);
}

#define SUPERNODE_ADD_CONV_SCALE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv_scale=1", __VA_ARGS__, 1))

#define SUPERNODE_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_CONV_SIZE, SUPERNODE_ADD_CONV_SCALE, ADD_CONV_GENERATORS, ADD_CONV_DST_FORMAT, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_644x258, ARG, convolve_generate_random, NULL), \

TEST_WITH_ARG(tivxConvolve, testConvolveSupernode, Arg,
    SUPERNODE_PARAMETERS
)
{
    #ifdef BUILD_BAM
    int node_count = 2;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt;
    vx_convolution convolution1 = 0, convolution2 = 0;
    vx_int16 data1[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 }, data2[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 };
    vx_size conv_max_dim = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_super_node, perf_graph;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, src->width, src->height, arg_->dst_format), VX_TYPE_IMAGE);

    VX_CALL(vxQueryContext(context, VX_CONTEXT_CONVOLUTION_MAX_DIMENSION, &conv_max_dim, sizeof(conv_max_dim)));

    if ((vx_size)arg_->cols > conv_max_dim || (vx_size)arg_->rows > conv_max_dim)
    {
        printf("%dx%d convolution is not supported. Skip test\n", (int)arg_->cols, (int)arg_->rows);
        return;
    }

    ASSERT_NO_FAILURE(arg_->convolution_data_generator(arg_->cols, arg_->rows, data1));
    ASSERT_NO_FAILURE(convolution1 = convolution_create(context, arg_->cols, arg_->rows, data1, arg_->scale));

    ASSERT_NO_FAILURE(arg_->convolution_data_generator(arg_->cols, arg_->rows, data2));
    ASSERT_NO_FAILURE(convolution2 = convolution_create(context, arg_->cols, arg_->rows, data2, arg_->scale));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, 0, 0, arg_->dst_format), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxConvolveNode(graph, src_image, convolution1, virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxConvolveNode(graph, virt, convolution2, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

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

    vxGetValidRegionImage(src_image, &src_rect);
    vxGetValidRegionImage(dst_image, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), arg_->width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), arg_->height);

    ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), (arg_->width - 2*(arg_->cols-1)));
    ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), (arg_->height - 2*(arg_->rows-1)));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(convolve_sequential_check(src, dst, border, arg_->cols, arg_->rows, data1, data2, arg_->scale, arg_->dst_format));

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

    VX_CALL(vxReleaseConvolution(&convolution1));
    VX_CALL(vxReleaseConvolution(&convolution2));
    ASSERT(convolution1 == NULL);
    ASSERT(convolution2 == NULL);

    printPerformance(perf_super_node, arg_->width * arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
    #endif
}

#ifdef BUILD_BAM
#define testConvolveSupernode testConvolveSupernode
#else
#define testConvolveSupernode DISABLED_testConvolveSupernode
#endif

TEST(tivxConvolve, negativeTestQueryConvolution)
{
    #define VX_CONVOLUTION_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_convolution cnvl = NULL;
    vx_enum attribute = VX_CONVOLUTION_DEFAULT;
    vx_size size = 0, columns = 5, rows = 5;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryConvolution(NULL, attribute, &size, sizeof(size)));
    ASSERT_VX_OBJECT(cnvl = vxCreateConvolution(context, columns, rows), VX_TYPE_CONVOLUTION);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryConvolution(cnvl, VX_CONVOLUTION_SCALE, &size, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryConvolution(cnvl, VX_CONVOLUTION_COLUMNS, &size, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryConvolution(cnvl, VX_CONVOLUTION_ROWS, &size, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryConvolution(cnvl, VX_CONVOLUTION_SIZE, &size, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryConvolution(cnvl, attribute, &size, size));
    VX_CALL(vxReleaseConvolution(&cnvl));
}

TEST(tivxConvolve, negativeTestSetConvolutionAttribute)
{
    #define VX_CONVOLUTION_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_convolution cnvl = NULL;
    vx_enum attribute = VX_CONVOLUTION_DEFAULT;
    vx_size size = 0, scale = 3, columns = 5, rows = 5;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetConvolutionAttribute(NULL, attribute, &size, sizeof(size)));
    ASSERT_VX_OBJECT(cnvl = vxCreateConvolution(context, columns, rows), VX_TYPE_CONVOLUTION);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_VALUE, vxSetConvolutionAttribute(cnvl, VX_CONVOLUTION_SCALE, &size, sizeof(vx_uint32)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_VALUE, vxSetConvolutionAttribute(cnvl, VX_CONVOLUTION_SCALE, &scale, sizeof(vx_uint32)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetConvolutionAttribute(cnvl, VX_CONVOLUTION_SCALE, &size, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetConvolutionAttribute(cnvl, attribute, &size, size));
    VX_CALL(vxReleaseConvolution(&cnvl));
}

TEST(tivxConvolve, negativeTestCopyConvolutionCoefficients)
{
    #define VX_CONVOLUTION_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_convolution cnvl = NULL;
    vx_enum usage = VX_READ_ONLY, user_mem_type = VX_MEMORY_TYPE_NONE;
    vx_size size = 0, columns = 5, rows = 5;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxCopyConvolutionCoefficients(NULL, NULL, usage, user_mem_type));
    ASSERT_VX_OBJECT(cnvl = vxCreateConvolution(context, columns, rows), VX_TYPE_CONVOLUTION);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxCopyConvolutionCoefficients(cnvl, NULL, usage, user_mem_type));
    VX_CALL(vxReleaseConvolution(&cnvl));
}

TEST(tivxConvolve, negativeTestCreateConvolution)
{
    vx_context context = context_->vx_context_;

    vx_size size = 0, columns = 5, rows = 5;

    ASSERT(NULL == vxCreateConvolution(NULL, columns, rows));
    ASSERT(NULL == vxCreateConvolution(context, VX_CONTEXT_CONVOLUTION_MAX_DIMENSION + 1, VX_CONTEXT_CONVOLUTION_MAX_DIMENSION + 1));
    ASSERT(NULL == vxCreateConvolution(context, 2, 2));
    ASSERT(NULL == vxCreateConvolution(context, 1, 2));
    ASSERT(NULL == vxCreateConvolution(context, 3, 2));
    ASSERT(NULL == vxCreateConvolution(context, 3, 1));
}

/* Sample application to show the usage of test_utils_mem_operations APIs */
TEST(tivxConvolve, negativeTestMemBufferAlloc)
{
    vx_context context = context_->vx_context_;
    vx_enum mheap_region = TIVX_MEM_EXTERNAL;
    vx_status status = VX_SUCCESS;

    tivx_shared_mem_info_t *tivx_shared_mem_info_array;
    uint32_t num_chunks;

    /* Allocating memory until exhausted */
    VX_CALL(test_utils_max_out_heap_mem(&tivx_shared_mem_info_array, &num_chunks, mheap_region));

    /* Releasing allocated memory */
    VX_CALL(test_utils_release_maxed_out_heap_mem(tivx_shared_mem_info_array, num_chunks));
}

/* To hit negative portions of vxQueryConvolution() */
TEST(tivxConvolve, negativeTestQueryConvolution1)
{
    vx_context context = context_->vx_context_;
    vx_convolution conv;
    vx_size rows = 3, cols = 3;
    vx_size actual_n = 1;

    vx_enum mheap_region = TIVX_MEM_EXTERNAL;
    vx_status status = VX_SUCCESS;

    tivx_shared_mem_info_t *tivx_shared_mem_info_array;
    uint32_t num_chunks;


    ASSERT_VX_OBJECT(conv = vxCreateConvolution(context, cols, rows), VX_TYPE_CONVOLUTION);

    /* Allocating all the memory under heap region TIVX_MEM_EXTERNAL using test-utils mem api*/
    VX_CALL(test_utils_max_out_heap_mem(&tivx_shared_mem_info_array, &num_chunks, mheap_region));

    /* vxQueryConvolution should fail due to tivxMemBufferAlloc failure */
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_MEMORY, vxQueryConvolution(conv, VX_CONVOLUTION_ROWS, &actual_n, sizeof(actual_n)));

    /* Freeing all the previously allocated memory */
    VX_CALL(test_utils_release_maxed_out_heap_mem(tivx_shared_mem_info_array, num_chunks));

    /* Cleanup */
    VX_CALL(vxReleaseConvolution(&conv));

    ASSERT(conv == 0);
}

/* To hit negative portions of vxCopyConvolutionCoefficients() */
TEST(tivxConvolve, negativeTestCopyConvolutionCoefficients1)
{
    vx_context context = context_->vx_context_;
    vx_convolution conv;
    vx_size rows = 3, cols = 3;

    vx_int16 gx[3][3] = {
        { 3, 0, -3},
        { 10, 0,-10},
        { 3, 0, -3},
    };

    tivx_shared_mem_info_t *tivx_shared_mem_info_array;
    uint32_t num_chunks;

    vx_enum mheap_region = TIVX_MEM_EXTERNAL;
    vx_status status = VX_SUCCESS;

    ASSERT_VX_OBJECT(conv = vxCreateConvolution(context, cols, rows), VX_TYPE_CONVOLUTION);

    /* Allocating all the memory under heap region TIVX_MEM_EXTERNAL using test-utils mem api*/
    VX_CALL(test_utils_max_out_heap_mem(&tivx_shared_mem_info_array, &num_chunks, mheap_region));

    /* vxCopyConvolutionCoefficients should fail due to tivxMemBufferAlloc failure */
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_MEMORY, vxCopyConvolutionCoefficients(conv, gx, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    /* Freeing all the previously allocated memory */
    VX_CALL(test_utils_release_maxed_out_heap_mem(tivx_shared_mem_info_array, num_chunks));

    /* Cleanup */
    VX_CALL(vxReleaseConvolution(&conv));

    ASSERT(conv == 0);
}

TESTCASE_TESTS(
    tivxConvolve,
    testGraphProcessing,
    negativeTestBorderMode,
    testConvolveSupernode,
    negativeTestQueryConvolution,
    negativeTestSetConvolutionAttribute,
    negativeTestCopyConvolutionCoefficients,
    negativeTestCreateConvolution,
    negativeTestMemBufferAlloc,
    negativeTestQueryConvolution1,
    negativeTestCopyConvolutionCoefficients1
)

