/*

 * Copyright (c) 2016-2017 The Khronos Group Inc.
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
#include <VX/vxu.h>
#include "shared_functions.h"

TESTCASE(tivxNonLinearFilter, CT_VXContext, ct_setup_vx_context, 0)


#define MASK_SIZE_MAX (9)

#ifndef MIN
#define MIN(_a,_b) (((_a) < (_b)) ? (_a) : (_b))
#endif
#ifndef MAX
#define MAX(_a,_b) (((_a) > (_b)) ? (_a) : (_b))
#endif

static CT_Image generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
        image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static int vx_uint8_compare(const void *p1, const void *p2)
{
    vx_uint8 a = *(vx_uint8 *)p1;
    vx_uint8 b = *(vx_uint8 *)p2;
    if (a > b)
        return 1;
    else if (a == b)
        return 0;
    else
        return -1;
}

static void filter_calculate(vx_enum function, CT_Image src, vx_coordinates2d_t* origin, vx_int32 cols, vx_int32 rows, vx_uint8* mask, vx_border_t* border, int32_t x, int32_t y, uint8_t *data)
{
    vx_uint8 values[MASK_SIZE_MAX * MASK_SIZE_MAX];

    vx_int32 i, j, ci, cj, m = 0, v = 0;
    vx_int32 cx = origin->x;
    vx_int32 cy = origin->y;

    for (j = y - cy; j < y - cy + rows; ++j)
    {
        for (i = x - cx; i < x - cx + cols; ++i, ++m)
        {
            if (mask[m])
            {
                ci = MAX(0, MIN(i, (vx_int32)src->width - 1));
                cj = MAX(0, MIN(j, (vx_int32)src->height - 1));

                values[v++] = (border->mode == VX_BORDER_CONSTANT && (i != ci || j != cj)) ? border->constant_value.U8 : *CT_IMAGE_DATA_PTR_8U(src, ci, cj);
            }
        }
    }

    qsort(values, v, sizeof(vx_uint8), vx_uint8_compare);

    switch (function)
    {
    case VX_NONLINEAR_FILTER_MIN: *data = values[0]; break; /* minimal value */
    case VX_NONLINEAR_FILTER_MAX: *data = values[v - 1]; break; /* maximum value */
    case VX_NONLINEAR_FILTER_MEDIAN: *data = values[v / 2]; break; /* pick the middle value */
    }
}

void tivx_filter_create_reference_image(vx_enum function, CT_Image src, vx_coordinates2d_t* origin, vx_size cols, vx_size rows, vx_uint8* mask, CT_Image* pdst, vx_border_t* border)
{
    CT_Image dst = NULL;

    CT_ASSERT(src->format == VX_DF_IMAGE_U8);

    dst = ct_allocate_image(src->width, src->height, VX_DF_IMAGE_U8);

    if (border->mode == VX_BORDER_UNDEFINED)
    {
        vx_uint32 left = origin->x;
        vx_uint32 top = origin->y;
        vx_uint32 right = (vx_uint32)(cols - origin->x - 1);
        vx_uint32 bottom = (vx_uint32)(rows - origin->y - 1);

        CT_FILL_IMAGE_8U(return, dst,
            if (x >= left && y >= top && x < src->width - right && y < src->height - bottom)
                filter_calculate(function, src, origin, (vx_int32)cols, (vx_int32)rows, mask, border, x, y, dst_data);
        );
    }
    else
    {
        CT_FILL_IMAGE_8U(return, dst,
            filter_calculate(function, src, origin, (vx_int32)cols, (vx_int32)rows, mask, border, x, y, dst_data);
        );
    }

    *pdst = dst;
}

static void pattern_check(vx_uint8* mask, vx_size cols, vx_size rows, vx_enum pattern)
{
    vx_size x, y;

    ASSERT(pattern != VX_PATTERN_OTHER);

    for (y = 0; y < rows; ++y)
    {
        for (x = 0; x < cols; ++x)
        {
            vx_uint8 value = mask[x + y * cols];
            vx_uint8 ref = 0;
            switch (pattern)
            {
            case VX_PATTERN_BOX: ref = 255; break;
            case VX_PATTERN_CROSS: ref = ((y == rows / 2) || (x == cols / 2)) ? 255 : 0; break;
            case VX_PATTERN_DISK:
                ref = (((y - rows / 2.0 + 0.5) * (y - rows / 2.0 + 0.5)) / ((rows / 2.0) * (rows / 2.0)) +
                    ((x - cols / 2.0 + 0.5) * (x - cols / 2.0 + 0.5)) / ((cols / 2.0) * (cols / 2.0)))
                    <= 1 ? 255 : 0;
                break;
            }

            ASSERT(value == ref);
        }
    }
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


static void filter_check(vx_enum function, CT_Image src, vx_matrix mask, CT_Image dst, vx_border_t* border)
{
    CT_Image dst_ref = NULL;
    ASSERT(src && dst && mask && border);

    vx_size rows, cols;
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_ROWS, &rows, sizeof(rows)));
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_COLUMNS, &cols, sizeof(cols)));

    vx_coordinates2d_t origin;
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_ORIGIN, &origin, sizeof(origin)));

    vx_enum pattern = 0;
    vx_uint8 m[MASK_SIZE_MAX * MASK_SIZE_MAX];
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_PATTERN, &pattern, sizeof(pattern)));
    VX_CALL(vxCopyMatrix(mask, m, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    ASSERT_NO_FAILURE(pattern_check(m, cols, rows, pattern));

    ASSERT_NO_FAILURE(tivx_filter_create_reference_image(function, src, &origin, cols, rows, m, &dst_ref, border));

    ASSERT_NO_FAILURE(
    if (border->mode == VX_BORDER_UNDEFINED)
    {
        vx_int32 left = origin.x;
        vx_int32 top = origin.y;
        vx_int32 right = (vx_int32)(cols - origin.x - 1);
        vx_int32 bottom = (vx_int32)(rows - origin.y - 1);

        ct_adjust_roi(dst, left, top, right, bottom);
        ct_adjust_roi(dst_ref, left, top, right, bottom);
    }
    );

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
}

static void filter_not_check(vx_enum function, CT_Image src, vx_matrix mask, CT_Image dst, vx_border_t* border)
{
    CT_Image virt_ref = NULL, dst_ref = NULL;
    ASSERT(src && dst && mask && border);

    vx_size rows, cols;
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_ROWS, &rows, sizeof(rows)));
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_COLUMNS, &cols, sizeof(cols)));

    vx_coordinates2d_t origin;
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_ORIGIN, &origin, sizeof(origin)));

    vx_enum pattern = 0;
    vx_uint8 m[MASK_SIZE_MAX * MASK_SIZE_MAX];
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_PATTERN, &pattern, sizeof(pattern)));
    VX_CALL(vxCopyMatrix(mask, m, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    ASSERT_NO_FAILURE(pattern_check(m, cols, rows, pattern));

    dst_ref = ct_allocate_image(src->width, src->height, VX_DF_IMAGE_U8);

    ASSERT_NO_FAILURE(tivx_filter_create_reference_image(function, src, &origin, cols, rows, m, &virt_ref, border));

    ASSERT_NO_FAILURE(referenceNot(virt_ref, dst_ref));

    ASSERT_NO_FAILURE(
    if (border->mode == VX_BORDER_UNDEFINED)
    {
        vx_int32 left = origin.x;
        vx_int32 top = origin.y;
        vx_int32 right = (vx_int32)(cols - origin.x - 1);
        vx_int32 bottom = (vx_int32)(rows - origin.y - 1);

        ct_adjust_roi(virt_ref, left, top, right, bottom);
        ct_adjust_roi(dst, left, top, right, bottom);
        ct_adjust_roi(dst_ref, left, top, right, bottom);
    }
    );

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
}


typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char* fileName;
    vx_size mask_width;
    vx_size mask_height;
    vx_enum function;
    vx_enum pattern;
    vx_border_t border;
    int width, height;
} Filter_Arg;


#define ADD_FUNCTIONS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_NONLINEAR_FILTER_MIN", __VA_ARGS__, VX_NONLINEAR_FILTER_MIN)), \
    CT_EXPAND(nextmacro(testArgName "/VX_NONLINEAR_FILTER_MAX", __VA_ARGS__, VX_NONLINEAR_FILTER_MAX)), \
    CT_EXPAND(nextmacro(testArgName "/VX_NONLINEAR_FILTER_MEDIAN", __VA_ARGS__, VX_NONLINEAR_FILTER_MEDIAN))

#define ADD_PATTERNS_BOX_CROSS_DISK(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_BOX", __VA_ARGS__, VX_PATTERN_BOX)), \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_CROSS", __VA_ARGS__, VX_PATTERN_CROSS)), \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_DISK", __VA_ARGS__, VX_PATTERN_DISK))

#define ADD_PATTERNS_BOX_CROSS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_BOX", __VA_ARGS__, VX_PATTERN_BOX)), \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_CROSS", __VA_ARGS__, VX_PATTERN_CROSS))

#define FILTER_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput/mask=3x5", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, generate_random, NULL, 3, 5), \
    CT_GENERATE_PARAMETERS("randomInput/mask=3x7", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, generate_random, NULL, 3, 7), \
    CT_GENERATE_PARAMETERS("randomInput/mask=3x9", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, generate_random, NULL, 3, 9), \
    CT_GENERATE_PARAMETERS("randomInput/mask=5x3", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, generate_random, NULL, 5, 3), \
    CT_GENERATE_PARAMETERS("randomInput/mask=5x7", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, generate_random, NULL, 5, 7), \
    CT_GENERATE_PARAMETERS("randomInput/mask=5x9", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, generate_random, NULL, 5, 9), \
    CT_GENERATE_PARAMETERS("randomInput/mask=7x3", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, generate_random, NULL, 7, 3), \
    CT_GENERATE_PARAMETERS("randomInput/mask=7x5", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, generate_random, NULL, 7, 5), \
    CT_GENERATE_PARAMETERS("randomInput/mask=7x7", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, generate_random, NULL, 7, 7), \
    CT_GENERATE_PARAMETERS("randomInput/mask=7x9", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, generate_random, NULL, 7, 9), \
    CT_GENERATE_PARAMETERS("randomInput/mask=9x3", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, generate_random, NULL, 9, 3), \
    CT_GENERATE_PARAMETERS("randomInput/mask=9x5", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, generate_random, NULL, 9, 5), \
    CT_GENERATE_PARAMETERS("randomInput/mask=9x7", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, generate_random, NULL, 9, 7), \
    CT_GENERATE_PARAMETERS("randomInput/mask=9x9", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ARG, generate_random, NULL, 9, 9), \
    CT_GENERATE_PARAMETERS("randomInput/mask=5x5", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS_DISK, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_18x18, ARG, generate_random, NULL, 5, 5), \
    CT_GENERATE_PARAMETERS("randomInput/mask=5x5", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS_DISK, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_1600x1200, ARG, generate_random, NULL, 5, 5)


TEST_WITH_ARG(tivxNonLinearFilter, testVirtualImage, Filter_Arg,
    FILTER_PARAMETERS
    )
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt = 0;
    vx_matrix mask = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_enum pattern = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src = arg_->generator(NULL, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    mask = vxCreateMatrixFromPattern(context, arg_->pattern, arg_->mask_width, arg_->mask_height);
    ASSERT_VX_OBJECT(mask, VX_TYPE_MATRIX);
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_PATTERN, &pattern, sizeof(pattern)));
    ASSERT_EQ_INT(arg_->pattern, pattern);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    node1 = vxNonLinearFilterNode(graph, arg_->function, src_image, mask, virt);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, virt, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(filter_not_check(arg_->function, src, mask, dst, &border));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseMatrix(&mask));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(mask == 0);
    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TEST_WITH_ARG(tivxNonLinearFilter, testGraphProcessing, Filter_Arg,
    FILTER_PARAMETERS
    )
{
    vx_context context = context_->vx_context_;
    vx_image src0_image = 0, dst0_image = 0, src1_image = 0, dst1_image = 0;
    vx_matrix mask = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_enum pattern = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    CT_Image src0 = NULL, dst0 = NULL, src1 = NULL, dst1 = NULL;
    vx_border_t border = arg_->border;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src0 = arg_->generator(NULL, arg_->width, arg_->height));

    ASSERT_NO_FAILURE(src1 = arg_->generator(NULL, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src0_image = ct_image_to_vx_image(src0, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1_image = ct_image_to_vx_image(src1, context), VX_TYPE_IMAGE);

    dst0_image = ct_create_similar_image(src0_image);
    ASSERT_VX_OBJECT(dst0_image, VX_TYPE_IMAGE);

    dst1_image = ct_create_similar_image(src1_image);
    ASSERT_VX_OBJECT(dst1_image, VX_TYPE_IMAGE);

    mask = vxCreateMatrixFromPattern(context, arg_->pattern, arg_->mask_width, arg_->mask_height);
    ASSERT_VX_OBJECT(mask, VX_TYPE_MATRIX);
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_PATTERN, &pattern, sizeof(pattern)));
    ASSERT_EQ_INT(arg_->pattern, pattern);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node1 = vxNonLinearFilterNode(graph, arg_->function, src0_image, mask, dst0_image);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
    node2 = vxNonLinearFilterNode(graph, arg_->function, src1_image, mask, dst1_image);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src0_image, &src_rect);
    vxGetValidRegionImage(dst0_image, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), arg_->width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), arg_->height);

    ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), (arg_->width - (arg_->mask_width-1) ));
    ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), (arg_->height - (arg_->mask_height-1) ));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(dst0 = ct_image_from_vx_image(dst0_image));
    ASSERT_NO_FAILURE(dst1 = ct_image_from_vx_image(dst1_image));

    ASSERT_NO_FAILURE(filter_check(arg_->function, src0, mask, dst0, &border));
    ASSERT_NO_FAILURE(filter_check(arg_->function, src1, mask, dst1, &border));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseMatrix(&mask));
    VX_CALL(vxReleaseImage(&dst1_image));
    VX_CALL(vxReleaseImage(&dst0_image));
    VX_CALL(vxReleaseImage(&src1_image));
    VX_CALL(vxReleaseImage(&src0_image));

    ASSERT(mask == 0);
    ASSERT(dst1_image == 0);
    ASSERT(dst0_image == 0);
    ASSERT(src1_image == 0);
    ASSERT(src0_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

#define NEGATIVE_ADD_FUNCTIONS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_NONLINEAR_FILTER_MEDIAN", __VA_ARGS__, VX_NONLINEAR_FILTER_MEDIAN))

#define NEGATIVE_ADD_PATTERNS_BOX_CROSS_DISK(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_PATTERN_BOX", __VA_ARGS__, VX_PATTERN_BOX))

#define NEGATIVE_FILTER_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput/mask=5x5", NEGATIVE_ADD_FUNCTIONS, NEGATIVE_ADD_PATTERNS_BOX_CROSS_DISK, ADD_VX_BORDERS_REQUIRE_CONSTANT_ONLY, ADD_SIZE_18x18, ARG, generate_random, NULL, 5, 5), \
    CT_GENERATE_PARAMETERS("randomInput/mask=5x5", NEGATIVE_ADD_FUNCTIONS, NEGATIVE_ADD_PATTERNS_BOX_CROSS_DISK, ADD_VX_BORDERS_REQUIRE_REPLICATE_ONLY, ADD_SIZE_644x258, ARG, generate_random, NULL, 5, 5)

TEST_WITH_ARG(tivxNonLinearFilter, negativeTestBorderMode, Filter_Arg,
    NEGATIVE_FILTER_PARAMETERS
    )
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt = 0;
    vx_matrix mask = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_enum pattern = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src = arg_->generator(NULL, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    mask = vxCreateMatrixFromPattern(context, arg_->pattern, arg_->mask_width, arg_->mask_height);
    ASSERT_VX_OBJECT(mask, VX_TYPE_MATRIX);
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_PATTERN, &pattern, sizeof(pattern)));
    ASSERT_EQ_INT(arg_->pattern, pattern);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    node1 = vxNonLinearFilterNode(graph, arg_->function, src_image, mask, virt);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, virt, dst_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_EQ_VX_STATUS(vxVerifyGraph(graph), VX_ERROR_NOT_SUPPORTED);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseMatrix(&mask));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(mask == 0);
    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

TESTCASE_TESTS(tivxNonLinearFilter, testVirtualImage, testGraphProcessing, negativeTestBorderMode)
