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

#include <VX/vx.h>
#include <VX/vxu.h>

#include "test_engine/test.h"
#include "shared_functions.h"

TESTCASE(NonLinearFilter, CT_VXContext, ct_setup_vx_context, 0)


#define MASK_SIZE_MAX (5)

#ifndef MIN
#define MIN(_a,_b) (((_a) < (_b)) ? (_a) : (_b))
#endif
#ifndef MAX
#define MAX(_a,_b) (((_a) > (_b)) ? (_a) : (_b))
#endif

TEST(NonLinearFilter, testNodeCreation)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_matrix matrix = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    matrix = vxCreateMatrixFromPattern(context, VX_PATTERN_CROSS, 5, 5);
    ASSERT_VX_OBJECT(matrix, VX_TYPE_MATRIX);

    ASSERT_VX_OBJECT(node = vxNonLinearFilterNode(graph, VX_NONLINEAR_FILTER_MEDIAN, src_image, matrix, dst_image), VX_TYPE_NODE);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseMatrix(&matrix));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(dst_image == 0);
    ASSERT(matrix == 0);
    ASSERT(src_image == 0);
}

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

void filter_create_reference_image(vx_enum function, CT_Image src, vx_coordinates2d_t* origin, vx_size cols, vx_size rows, vx_uint8* mask, CT_Image* pdst, vx_border_t* border)
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

    ASSERT_NO_FAILURE(filter_create_reference_image(function, src, &origin, cols, rows, m, &dst_ref, border));

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

#if 0
    if (CT_HasFailure())
    {
        printf("=== SRC ===\n");
        ct_dump_image_info(src);
        printf("=== DST ===\n");
        ct_dump_image_info(dst);
        printf("=== EXPECTED ===\n");
        ct_dump_image_info(dst_ref);
    }
#endif
}


typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char* fileName;
    vx_size mask_size;
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
    CT_GENERATE_PARAMETERS("randomInput/mask=3x3", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_SMALL_SET, ARG, generate_random, NULL, 3), \
    CT_GENERATE_PARAMETERS("randomInput/mask=5x5", ADD_FUNCTIONS, ADD_PATTERNS_BOX_CROSS_DISK, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_SMALL_SET, ARG, generate_random, NULL, 5)


TEST_WITH_ARG(NonLinearFilter, testGraphProcessing, Filter_Arg,
    FILTER_PARAMETERS
    )
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_matrix mask = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_enum pattern = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    mask = vxCreateMatrixFromPattern(context, arg_->pattern, arg_->mask_size, arg_->mask_size);
    ASSERT_VX_OBJECT(mask, VX_TYPE_MATRIX);
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_PATTERN, &pattern, sizeof(pattern)));
    ASSERT_EQ_INT(arg_->pattern, pattern);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxNonLinearFilterNode(graph, arg_->function, src_image, mask, dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(filter_check(arg_->function, src, mask, dst, &border));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseMatrix(&mask));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(mask == 0);
    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

TEST_WITH_ARG(NonLinearFilter, testImmediateProcessing, Filter_Arg,
    FILTER_PARAMETERS
    )
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_matrix mask = 0;
    vx_enum pattern = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    mask = vxCreateMatrixFromPattern(context, arg_->pattern, arg_->mask_size, arg_->mask_size);
    ASSERT_VX_OBJECT(mask, VX_TYPE_MATRIX);
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_PATTERN, &pattern, sizeof(pattern)));
    ASSERT_EQ_INT(arg_->pattern, pattern);

    VX_CALL(vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border)));

    VX_CALL(vxuNonLinearFilter(context, arg_->function, src_image, mask, dst_image));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(filter_check(arg_->function, src, mask, dst, &border));

    VX_CALL(vxReleaseMatrix(&mask));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(mask == 0);
    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

TESTCASE_TESTS(NonLinearFilter, testNodeCreation, testGraphProcessing, testImmediateProcessing)
