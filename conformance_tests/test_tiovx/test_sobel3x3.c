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
#include "shared_functions.h"

#define MAX_NODES 10

TESTCASE(tivxSobel3x3, CT_VXContext, ct_setup_vx_context, 0)

static void ct_print_image(CT_Image img)
{
    printf("CT_Image 0x%p:\n", img);
    if (img)
    {
        printf("\t width       = %d\n", img->width);
        printf("\t height      = %d\n", img->height);
        printf("\t stride      = %d\n", img->stride);
        printf("\t roi.x       = %d\n", img->roi.x);
        printf("\t roi.y       = %d\n", img->roi.y);
        printf("\t roi.width   = %d\n", img->roi.width);
        printf("\t roi.height  = %d\n", img->roi.height);
        printf("\t data        = 0x%p\n", img->data.y);
        printf("\t data_begin_ = 0x%p\n", img->data_begin_);
        printf("\t refcount_   = 0x%p (%u)\n", img->refcount_, img->refcount_ ? *img->refcount_ : 0);
    }
    printf("\n");
    fflush(stdout);
}

static void ct_print_image_numeric(CT_Image img, int row_count, int column_count)
{
    int32_t i, j;
    for (i = 0; i < row_count; i++)
    {
        for (j = 0; j < column_count; j++)
        {
            printf("\t%d ", img->data.y[i * img->stride + j]);
        }
        printf("\n");
    }
    printf("\n");
    fflush(stdout);
}

// Generate input to cover these requirements:
// There should be a image with randomly generated pixel intensities.
static CT_Image sobel3x3_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static int16_t sobel_x_get(int32_t *values)
{
    int32_t res = (-values[0])     + (values[2]) +
                  (-values[3] * 2) + (values[5] * 2) +
                  (-values[6])     + (values[8]);
    return (int16_t)res;
}

static int16_t sobel_y_get(int32_t *values)
{
    int32_t res = (-values[0])     + (values[6]) +
                  (-values[1] * 2) + (values[7] * 2) +
                  (-values[2])     + (values[8]);
    return (int16_t)res;
}

static void sobel3x3_calculate(CT_Image src, uint32_t x, uint32_t y, int16_t *sobel_x, int16_t *sobel_y)
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
    *sobel_x = sobel_x_get(values);
    *sobel_y = sobel_y_get(values);
}

static void sobel3x3_calculate_replicate(CT_Image src, uint32_t x_, uint32_t y_, int16_t *sobel_x, int16_t *sobel_y)
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
    *sobel_x = sobel_x_get(values);
    *sobel_y = sobel_y_get(values);
}

static void sobel3x3_calculate_constant(CT_Image src, uint32_t x_, uint32_t y_, vx_uint32 constant_value, int16_t *sobel_x, int16_t *sobel_y)
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
    *sobel_x = sobel_x_get(values);
    *sobel_y = sobel_y_get(values);
}


void tivx_sobel3x3_create_reference_image(CT_Image src, vx_border_t border, CT_Image *p_dst_x, CT_Image *p_dst_y)
{
    CT_Image dst_x = NULL, dst_y = NULL;

    CT_ASSERT(src->format == VX_DF_IMAGE_U8);

    dst_x = ct_allocate_image(src->width, src->height, VX_DF_IMAGE_S16);
    dst_y = ct_allocate_image(src->width, src->height, VX_DF_IMAGE_S16);

    if (border.mode == VX_BORDER_UNDEFINED)
    {
        CT_FILL_IMAGE_16S(return, dst_x,
                if (x >= 1 && y >= 1 && x < src->width - 1 && y < src->height - 1)
                {
                    int16_t* dst_y_data = CT_IMAGE_DATA_PTR_16S(dst_y, x, y);
                    sobel3x3_calculate(src, x, y, dst_data, dst_y_data);
                });
    }
    else if (border.mode == VX_BORDER_REPLICATE)
    {
        CT_FILL_IMAGE_16S(return, dst_x,
                {
                    int16_t* dst_y_data = CT_IMAGE_DATA_PTR_16S(dst_y, x, y);
                    sobel3x3_calculate_replicate(src, x, y, dst_data, dst_y_data);
                });
    }
    else if (border.mode == VX_BORDER_CONSTANT)
    {
        vx_uint32 constant_value = border.constant_value.U32;
        CT_FILL_IMAGE_16S(return, dst_x,
                {
                    int16_t* dst_y_data = CT_IMAGE_DATA_PTR_16S(dst_y, x, y);
                    sobel3x3_calculate_constant(src, x, y, constant_value, dst_data, dst_y_data);
                });
    }
    else
    {
        ASSERT_(return, 0);
    }

    *p_dst_x = dst_x;
    *p_dst_y = dst_y;
}


static void sobel3x3_check(CT_Image src, CT_Image dst_x, CT_Image dst_y, vx_border_t border)
{
    CT_Image dst_x_ref = NULL, dst_y_ref = NULL;

    ASSERT(src && dst_x && dst_y);

    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(src, border, &dst_x_ref, &dst_y_ref));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst_x,  1, 1, 1, 1);
            ct_adjust_roi(dst_x_ref, 1, 1, 1, 1);
            ct_adjust_roi(dst_y,  1, 1, 1, 1);
            ct_adjust_roi(dst_y_ref, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst_x_ref, dst_x);
    EXPECT_EQ_CTIMAGE(dst_y_ref, dst_y);
}

static void sobel3x3_sequential_check(CT_Image src, CT_Image virt1, CT_Image virt2, CT_Image dst1_x, CT_Image dst1_y, CT_Image dst2_x, CT_Image dst2_y, vx_border_t border)
{
    CT_Image dst0_x_ref = NULL, dst0_y_ref = NULL, dst1_x_ref = NULL, dst1_y_ref = NULL, dst2_x_ref = NULL, dst2_y_ref = NULL;

    ASSERT(src && virt1 && virt2 && dst1_x && dst1_y && dst2_x && dst2_y);

    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(src, border, &dst0_x_ref, &dst0_y_ref));
    ASSERT_NO_FAILURE(referenceConvertDepth(dst0_x_ref, virt1, 0, VX_CONVERT_POLICY_SATURATE));
    ASSERT_NO_FAILURE(referenceConvertDepth(dst0_y_ref, virt2, 0, VX_CONVERT_POLICY_SATURATE));
    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(virt1, border, &dst1_x_ref, &dst1_y_ref));
    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(virt2, border, &dst2_x_ref, &dst2_y_ref));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst1_x,  2, 2, 2, 2);
            ct_adjust_roi(dst1_x_ref, 2, 2, 2, 2);
            ct_adjust_roi(dst1_y,  2, 2, 2, 2);
            ct_adjust_roi(dst1_y_ref, 2, 2, 2, 2);
            ct_adjust_roi(dst2_x,  2, 2, 2, 2);
            ct_adjust_roi(dst2_x_ref, 2, 2, 2, 2);
            ct_adjust_roi(dst2_y,  2, 2, 2, 2);
            ct_adjust_roi(dst2_y_ref, 2, 2, 2, 2);
        }
    );

    EXPECT_EQ_CTIMAGE(dst1_x_ref, dst1_x);
    EXPECT_EQ_CTIMAGE(dst1_y_ref, dst1_y);
    EXPECT_EQ_CTIMAGE(dst2_x_ref, dst2_x);
    EXPECT_EQ_CTIMAGE(dst2_y_ref, dst2_y);
    /*
    int row_count = 20, column_count = 20;

    printf("%s\n", "dst1_x_ref");
    ct_print_image(dst1_x_ref);
    ct_print_image_numeric(dst1_x_ref, row_count, column_count);

    printf("%s\n", "dst1_x");
    ct_print_image(dst1_x);
    ct_print_image_numeric(dst1_x, row_count, column_count);

    printf("%s\n", "dst1_y_ref");
    ct_print_image(dst1_y_ref);
    ct_print_image_numeric(dst1_y_ref, row_count, column_count);

    printf("%s\n", "dst1_y");
    ct_print_image(dst1_y);
    ct_print_image_numeric(dst1_y, row_count, column_count);

    printf("%s\n", "dst2_x_ref");
    ct_print_image(dst2_x_ref);
    ct_print_image_numeric(dst2_x_ref, row_count, column_count);

    printf("%s\n", "dst2_x");
    ct_print_image(dst2_x);
    ct_print_image_numeric(dst2_x, row_count, column_count);

    printf("%s\n", "dst2_y_ref");
    ct_print_image(dst2_y_ref);
    ct_print_image_numeric(dst2_y_ref, row_count, column_count);

    printf("%s\n", "dst2_y");
    ct_print_image(dst2_y);
    ct_print_image_numeric(dst2_y, row_count, column_count);
    */
}

static void sobel3x3_sequential_check2(CT_Image src, CT_Image virt1, CT_Image virt2, CT_Image dst1_x, CT_Image dst1_y, CT_Image dst2_x, CT_Image dst2_y, vx_border_t border)
{
    CT_Image dst0_x_ref = NULL, dst0_y_ref = NULL, dst1_x_ref = NULL, dst1_y_ref = NULL, dst2_x_ref = NULL;

    ASSERT(src && virt1 && virt2 && dst1_x && dst1_y && dst2_x);

    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(src, border, &dst0_x_ref, &dst0_y_ref));
    ASSERT_NO_FAILURE(referenceConvertDepth(dst0_x_ref, virt1, 0, VX_CONVERT_POLICY_SATURATE));
    ASSERT_NO_FAILURE(referenceConvertDepth(dst0_y_ref, virt2, 0, VX_CONVERT_POLICY_SATURATE));
    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(virt1, border, &dst1_x_ref, &dst1_y_ref));
    ASSERT_NO_FAILURE(referenceNot(virt2, dst2_y));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst1_x,  2, 2, 2, 2);
            ct_adjust_roi(dst1_x_ref, 2, 2, 2, 2);
            ct_adjust_roi(dst1_y,  2, 2, 2, 2);
            ct_adjust_roi(dst1_y_ref, 2, 2, 2, 2);
            ct_adjust_roi(dst2_x,  1, 1, 1, 1);
            ct_adjust_roi(dst2_y, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst1_x_ref, dst1_x);
    EXPECT_EQ_CTIMAGE(dst1_y_ref, dst1_y);
    EXPECT_EQ_CTIMAGE(dst2_y, dst2_x);
    /*
    int row_count = 20, column_count = 20;

    printf("%s\n", "dst1_x_ref");
    ct_print_image(dst1_x_ref);
    ct_print_image_numeric(dst1_x_ref, row_count, column_count);

    printf("%s\n", "dst1_x");
    ct_print_image(dst1_x);
    ct_print_image_numeric(dst1_x, row_count, column_count);

    printf("%s\n", "dst1_y_ref");
    ct_print_image(dst1_y_ref);
    ct_print_image_numeric(dst1_y_ref, row_count, column_count);

    printf("%s\n", "dst1_y");
    ct_print_image(dst1_y);
    ct_print_image_numeric(dst1_y, row_count, column_count);

    printf("%s\n", "dst2_y");
    ct_print_image(dst2_y);
    ct_print_image_numeric(dst2_y, row_count, column_count);

    printf("%s\n", "dst2_x");
    ct_print_image(dst2_x);
    ct_print_image_numeric(dst2_x, row_count, column_count);
    */
}

static void sobel3x3_sequential_check3(CT_Image src, CT_Image dst1, CT_Image dst2, CT_Image dst1_ref, CT_Image dst2_ref, vx_border_t border)
{
    CT_Image dst_x_ref = NULL, dst_y_ref = NULL, dst2_y_ref = NULL;

    ASSERT(src && dst1 && dst2);

    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(src, border, &dst_x_ref, &dst_y_ref));
    ASSERT_NO_FAILURE(referenceConvertDepth(dst_x_ref, dst1_ref, 0, VX_CONVERT_POLICY_SATURATE));
    ASSERT_NO_FAILURE(referenceConvertDepth(dst_y_ref, dst2_ref, 0, VX_CONVERT_POLICY_SATURATE));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst1_ref,  1, 1, 1, 1);
            ct_adjust_roi(dst2_ref, 1, 1, 1, 1);
            ct_adjust_roi(dst1,  1, 1, 1, 1);
            ct_adjust_roi(dst2, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst1_ref, dst1);
    EXPECT_EQ_CTIMAGE(dst2_ref, dst2);
}

static void sobel3x3_sequential_check4(CT_Image src, CT_Image virt1, CT_Image virt2, CT_Image virt3, CT_Image dst0_x, CT_Image dst0_y, \
                                       CT_Image dst1_y, CT_Image dst2_x, CT_Image dst2_y, vx_border_t border)
{
    CT_Image dst0_x_ref = NULL, dst0_y_ref = NULL, dst1_x_ref = NULL, dst1_y_ref = NULL, dst2_x_ref = NULL, dst2_y_ref = NULL;

    ASSERT(src && virt1 && virt2 && virt3 && dst0_x && dst0_y && dst1_y && dst2_x && dst2_y);

    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(src, border, &dst0_x_ref, &dst0_y_ref));
    ASSERT_NO_FAILURE(referenceConvertDepth(dst0_x_ref, virt1, 0, VX_CONVERT_POLICY_SATURATE));
    ASSERT_NO_FAILURE(referenceConvertDepth(dst0_y_ref, virt2, 0, VX_CONVERT_POLICY_SATURATE));
    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(virt1, border, &dst1_x_ref, &dst1_y_ref));
    ASSERT_NO_FAILURE(referenceConvertDepth(dst1_x_ref, virt3, 0, VX_CONVERT_POLICY_SATURATE));
    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(virt3, border, &dst0_x_ref, &dst0_y_ref));
    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(virt2, border, &dst2_x_ref, &dst2_y_ref));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst0_x,  3, 3, 3, 3);
            ct_adjust_roi(dst0_x_ref, 3, 3, 3, 3);
            ct_adjust_roi(dst0_y,  3, 3, 3, 3);
            ct_adjust_roi(dst0_y_ref, 3, 3, 3, 3);
            ct_adjust_roi(dst1_y,  2, 2, 2, 2);
            ct_adjust_roi(dst1_y_ref, 2, 2, 2, 2);
            ct_adjust_roi(dst2_x,  2, 2, 2, 2);
            ct_adjust_roi(dst2_x_ref, 2, 2, 2, 2);
            ct_adjust_roi(dst2_y,  2, 2, 2, 2);
            ct_adjust_roi(dst2_y_ref, 2, 2, 2, 2);
        }
    );

    EXPECT_EQ_CTIMAGE(dst0_x_ref, dst0_x);
    EXPECT_EQ_CTIMAGE(dst0_y_ref, dst0_y);
    EXPECT_EQ_CTIMAGE(dst1_y_ref, dst1_y);
    EXPECT_EQ_CTIMAGE(dst2_x_ref, dst2_x);
    EXPECT_EQ_CTIMAGE(dst2_y_ref, dst2_y);
}

static void sobel3x3_check_y(CT_Image src, CT_Image dst_y, vx_border_t border)
{
    CT_Image dst_x_ref = NULL, dst_y_ref = NULL;

    ASSERT(src && dst_y);

    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(src, border, &dst_x_ref, &dst_y_ref));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst_y,  1, 1, 1, 1);
            ct_adjust_roi(dst_y_ref, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst_y_ref, dst_y);
}

static void sobel3x3_check_x(CT_Image src, CT_Image dst_x, vx_border_t border)
{
    CT_Image dst_x_ref = NULL, dst_y_ref = NULL;

    ASSERT(src && dst_x);

    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(src, border, &dst_x_ref, &dst_y_ref));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst_x,  1, 1, 1, 1);
            ct_adjust_roi(dst_x_ref, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst_x_ref, dst_x);
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    vx_border_t border;
    int width, height;
} Filter_Arg;

#define SOBEL_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_18x18, ARG, sobel3x3_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_644x258, ARG, sobel3x3_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_1600x1200, ARG, sobel3x3_generate_random, NULL)

TEST_WITH_ARG(tivxSobel3x3, testGraphProcessing, Filter_Arg,
    SOBEL_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst1_x_image = 0, dst1_y_image = 0;
    vx_image dst2_x_image = 0, dst2_y_image = 0, virt1, virt2, virt3, virt4;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_node4, perf_node5, perf_graph;
    vx_scalar shift_convertdepth = 0;
    vx_int32 sh = 0;

    CT_Image src = NULL, dst1_x = NULL, dst1_y = NULL, dst2_x = NULL, dst2_y = NULL, virt_ctimage1 = NULL, virt_ctimage2 = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst1_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst1_x_image, VX_TYPE_IMAGE);

    dst2_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst2_x_image, VX_TYPE_IMAGE);

    dst1_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst1_y_image, VX_TYPE_IMAGE);

    dst2_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst2_y_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt3   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt4   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(shift_convertdepth = vxCreateScalar(context, VX_TYPE_INT32, &sh), VX_TYPE_SCALAR);

    node1 = vxSobel3x3Node(graph, src_image, virt1, virt2);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    node2 = vxConvertDepthNode(graph, virt1, virt3, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    node3 = vxConvertDepthNode(graph, virt2, virt4, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
    ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);

    node4 = vxSobel3x3Node(graph, virt3, dst1_x_image, dst1_y_image);
    ASSERT_VX_OBJECT(node4, VX_TYPE_NODE);

    node5 = vxSobel3x3Node(graph, virt4, dst2_x_image, dst2_y_image);
    ASSERT_VX_OBJECT(node5, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node4, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node5, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryNode(node4, VX_NODE_PERFORMANCE, &perf_node4, sizeof(perf_node4));
    vxQueryNode(node5, VX_NODE_PERFORMANCE, &perf_node5, sizeof(perf_node5));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(dst1_x = ct_image_from_vx_image(dst1_x_image));
    ASSERT_NO_FAILURE(dst1_y = ct_image_from_vx_image(dst1_y_image));

    ASSERT_NO_FAILURE(dst2_x = ct_image_from_vx_image(dst2_x_image));
    ASSERT_NO_FAILURE(dst2_y = ct_image_from_vx_image(dst2_y_image));

    virt_ctimage1 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
    virt_ctimage2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    ASSERT_NO_FAILURE(sobel3x3_sequential_check(src, virt_ctimage1, virt_ctimage2, dst1_x, dst1_y, dst2_x, dst2_y, border));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(node5 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&virt3));
    VX_CALL(vxReleaseImage(&virt4));
    VX_CALL(vxReleaseImage(&dst1_x_image));
    VX_CALL(vxReleaseImage(&dst1_y_image));
    VX_CALL(vxReleaseImage(&dst2_x_image));
    VX_CALL(vxReleaseImage(&dst2_y_image));
    VX_CALL(vxReleaseImage(&src_image));
    VX_CALL(vxReleaseScalar(&shift_convertdepth));

    ASSERT(dst1_x_image == 0);
    ASSERT(dst1_y_image == 0);
    ASSERT(dst2_x_image == 0);
    ASSERT(dst2_y_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3, arg_->width*arg_->height, "N3");
    printPerformance(perf_node4, arg_->width*arg_->height, "N4");
    printPerformance(perf_node5, arg_->width*arg_->height, "N5");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}


TEST_WITH_ARG(tivxSobel3x3, testValidRegion, Filter_Arg,
    SOBEL_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst1_x_image = 0, dst1_y_image = 0;
    vx_image dst2_x_image = 0, dst2_y_image = 0, virt1, virt2, virt3, virt4;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_node4, perf_node5, perf_graph;
    vx_scalar shift_convertdepth = 0;
    vx_int32 sh = 0;
    vx_rectangle_t rect;
    vx_bool valid_rect;

    CT_Image src = NULL, dst1_x = NULL, dst1_y = NULL, dst2_x = NULL, dst2_y = NULL, virt_ctimage1 = NULL, virt_ctimage2 = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst1_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst1_x_image, VX_TYPE_IMAGE);

    dst2_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst2_x_image, VX_TYPE_IMAGE);

    dst1_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst1_y_image, VX_TYPE_IMAGE);

    dst2_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst2_y_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt3   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt4   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(shift_convertdepth = vxCreateScalar(context, VX_TYPE_INT32, &sh), VX_TYPE_SCALAR);

    node1 = vxSobel3x3Node(graph, src_image, virt1, virt2);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    node2 = vxConvertDepthNode(graph, virt1, virt3, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    node3 = vxConvertDepthNode(graph, virt2, virt4, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
    ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);

    node4 = vxSobel3x3Node(graph, virt3, dst1_x_image, dst1_y_image);
    ASSERT_VX_OBJECT(node4, VX_TYPE_NODE);

    node5 = vxSobel3x3Node(graph, virt4, dst2_x_image, dst2_y_image);
    ASSERT_VX_OBJECT(node5, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node4, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node5, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryNode(node4, VX_NODE_PERFORMANCE, &perf_node4, sizeof(perf_node4));
    vxQueryNode(node5, VX_NODE_PERFORMANCE, &perf_node5, sizeof(perf_node5));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    vxGetValidRegionImage(dst1_x_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    vxGetValidRegionImage(dst1_y_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    vxGetValidRegionImage(dst2_x_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    vxGetValidRegionImage(dst2_y_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    ASSERT_NO_FAILURE(dst1_x = ct_image_from_vx_image(dst1_x_image));
    ASSERT_NO_FAILURE(dst1_y = ct_image_from_vx_image(dst1_y_image));

    ASSERT_NO_FAILURE(dst2_x = ct_image_from_vx_image(dst2_x_image));
    ASSERT_NO_FAILURE(dst2_y = ct_image_from_vx_image(dst2_y_image));

    virt_ctimage1 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
    virt_ctimage2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    ASSERT_NO_FAILURE(sobel3x3_sequential_check(src, virt_ctimage1, virt_ctimage2, dst1_x, dst1_y, dst2_x, dst2_y, border));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(node5 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&virt3));
    VX_CALL(vxReleaseImage(&virt4));
    VX_CALL(vxReleaseImage(&dst1_x_image));
    VX_CALL(vxReleaseImage(&dst1_y_image));
    VX_CALL(vxReleaseImage(&dst2_x_image));
    VX_CALL(vxReleaseImage(&dst2_y_image));
    VX_CALL(vxReleaseImage(&src_image));
    VX_CALL(vxReleaseScalar(&shift_convertdepth));

    ASSERT(dst1_x_image == 0);
    ASSERT(dst1_y_image == 0);
    ASSERT(dst2_x_image == 0);
    ASSERT(dst2_y_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3, arg_->width*arg_->height, "N3");
    printPerformance(perf_node4, arg_->width*arg_->height, "N4");
    printPerformance(perf_node5, arg_->width*arg_->height, "N5");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TEST_WITH_ARG(tivxSobel3x3, testOptionalParametersX, Filter_Arg,
    SOBEL_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_x_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node1, perf_graph;

    CT_Image src = NULL, dst_x = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst_x_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxSobel3x3Node(graph, src_image, dst_x_image, NULL);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(dst_x = ct_image_from_vx_image(dst_x_image));

    ASSERT_NO_FAILURE(sobel3x3_check_x(src, dst_x, border));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_x_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_x_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TEST_WITH_ARG(tivxSobel3x3, testOptionalParametersY, Filter_Arg,
    SOBEL_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_y_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node1, perf_graph;

    CT_Image src = NULL, dst_y = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst_y_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxSobel3x3Node(graph, src_image, NULL, dst_y_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(dst_y = ct_image_from_vx_image(dst_y_image));

    ASSERT_NO_FAILURE(sobel3x3_check_y(src, dst_y, border));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_y_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_y_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

#define NEGATIVE_SOBEL_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_REPLICATE_ONLY, ADD_SIZE_18x18, ARG, sobel3x3_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_CONSTANT_ONLY, ADD_SIZE_18x18, ARG, sobel3x3_generate_random, NULL)

TEST_WITH_ARG(tivxSobel3x3, negativeTestBorderMode, Filter_Arg,
    NEGATIVE_SOBEL_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_y_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    CT_Image src = NULL, dst_y = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst_y_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxSobel3x3Node(graph, src_image, NULL, dst_y_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_EQ_VX_STATUS(vxVerifyGraph(graph), VX_ERROR_NOT_SUPPORTED);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_y_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_y_image == 0);
    ASSERT(src_image == 0);
}

#define SUPERNODE_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_644x258, ARG, sobel3x3_generate_random, NULL)

TEST_WITH_ARG(tivxSobel3x3, testSobel3x3Supernode, Filter_Arg,
    SUPERNODE_PARAMETERS
)
{
    int node_count = 1;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_x_image = 0, dst_y_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    CT_Image src = NULL, dst_x = NULL, dst_y = NULL;;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst_x_image, VX_TYPE_IMAGE);
    dst_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst_y_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxSobel3x3Node(graph, src_image, dst_x_image, dst_y_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_NO_FAILURE(node_list[0] = node); 
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    ASSERT_NO_FAILURE(dst_x = ct_image_from_vx_image(dst_x_image));
    ASSERT_NO_FAILURE(dst_y = ct_image_from_vx_image(dst_y_image));

    ASSERT_NO_FAILURE(sobel3x3_check(src, dst_x, dst_y, border));

    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_x_image));
    VX_CALL(vxReleaseImage(&dst_y_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_x_image == 0);
    ASSERT(dst_y_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_super_node, arg_->width * arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
}

TEST_WITH_ARG(tivxSobel3x3, testSobel3x3SupernodeMultiNode, Filter_Arg,
    SUPERNODE_PARAMETERS
)
{
    int node_count = 5;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst1_x_image = 0, dst1_y_image = 0;
    vx_image dst2_x_image = 0, dst2_y_image = 0, virt1, virt2, virt3, virt4;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_scalar shift_convertdepth = 0;
    vx_int32 sh = 0;
    vx_rectangle_t rect;
    vx_bool valid_rect;
    vx_rectangle_t img_rect = { 0, 0, arg_->width-4, arg_->height-4 };
    vx_map_id map_id;
    vx_imagepatch_addressing_t addr = { 0 };
    vx_int16* base_ptr = NULL;
    vx_pixel_value_t d2_val = {{ 1 }};
    vx_uint32 x, y;

    CT_Image src = NULL, dst1_x = NULL, dst1_y = NULL, dst2_x = NULL, dst2_y = NULL, virt_ctimage1 = NULL, virt_ctimage2 = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst1_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst1_x_image, VX_TYPE_IMAGE);

    dst2_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst2_x_image, VX_TYPE_IMAGE);

    dst1_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst1_y_image, VX_TYPE_IMAGE);

    dst2_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst2_y_image, VX_TYPE_IMAGE);

    /*
    VX_CALL(vxMapImagePatch(dst1_x_image, &img_rect, 0, &map_id, &addr, (void**)&base_ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    for ( y = 0; y < addr.dim_y; y++)
    {
        for ( x = 0; x < addr.dim_x; x++)
        {
            base_ptr[y*addr.dim_x + x] = 0xFFFF;
        }
    }
    VX_CALL(vxUnmapImagePatch(dst1_x_image, map_id));
    
    VX_CALL(vxMapImagePatch(dst2_x_image, &img_rect, 0, &map_id, &addr, (void**)&base_ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    for ( y = 0; y < addr.dim_y; y++)
    {
        for ( x = 0; x < addr.dim_x; x++)
        {
            base_ptr[y*addr.dim_x + x] = 0xFFFF;
        }
    }
    VX_CALL(vxUnmapImagePatch(dst2_x_image, map_id));
    
    VX_CALL(vxMapImagePatch(dst1_y_image, &img_rect, 0, &map_id, &addr, (void**)&base_ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    for ( y = 0; y < addr.dim_y; y++)
    {
        for ( x = 0; x < addr.dim_x; x++)
        {
            base_ptr[y*addr.dim_x + x] = 0xFFFF;
        }
    }
    VX_CALL(vxUnmapImagePatch(dst1_y_image, map_id));
    
    VX_CALL(vxMapImagePatch(dst2_y_image, &img_rect, 0, &map_id, &addr, (void**)&base_ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    for ( y = 0; y < addr.dim_y; y++)
    {
        for ( x = 0; x < addr.dim_x; x++)
        {
            base_ptr[y*addr.dim_x + x] = 0xFFFF;
        }
    }
    VX_CALL(vxUnmapImagePatch(dst2_y_image, map_id));
    */


    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt3   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt4   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(shift_convertdepth = vxCreateScalar(context, VX_TYPE_INT32, &sh), VX_TYPE_SCALAR);

    node1 = vxSobel3x3Node(graph, src_image, virt1, virt2);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    node2 = vxConvertDepthNode(graph, virt1, virt3, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    node3 = vxConvertDepthNode(graph, virt2, virt4, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
    ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);

    node4 = vxSobel3x3Node(graph, virt3, dst1_x_image, dst1_y_image);
    ASSERT_VX_OBJECT(node4, VX_TYPE_NODE);

    node5 = vxSobel3x3Node(graph, virt4, dst2_x_image, dst2_y_image);
    ASSERT_VX_OBJECT(node5, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node4, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node5, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_NO_FAILURE(node_list[0] = node1); 
    ASSERT_NO_FAILURE(node_list[1] = node2);
    ASSERT_NO_FAILURE(node_list[2] = node3); 
    ASSERT_NO_FAILURE(node_list[3] = node4);
    ASSERT_NO_FAILURE(node_list[4] = node5); 
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    vxGetValidRegionImage(dst1_x_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    vxGetValidRegionImage(dst1_y_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    vxGetValidRegionImage(dst2_x_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    vxGetValidRegionImage(dst2_y_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    ASSERT_NO_FAILURE(dst1_x = ct_image_from_vx_image(dst1_x_image));
    ASSERT_NO_FAILURE(dst1_y = ct_image_from_vx_image(dst1_y_image));

    ASSERT_NO_FAILURE(dst2_x = ct_image_from_vx_image(dst2_x_image));
    ASSERT_NO_FAILURE(dst2_y = ct_image_from_vx_image(dst2_y_image));

    virt_ctimage1 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
    virt_ctimage2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    ASSERT_NO_FAILURE(sobel3x3_sequential_check(src, virt_ctimage1, virt_ctimage2, dst1_x, dst1_y, dst2_x, dst2_y, border));

    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(node5 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&virt3));
    VX_CALL(vxReleaseImage(&virt4));
    VX_CALL(vxReleaseImage(&dst1_x_image));
    VX_CALL(vxReleaseImage(&dst1_y_image));
    VX_CALL(vxReleaseImage(&dst2_x_image));
    VX_CALL(vxReleaseImage(&dst2_y_image));
    VX_CALL(vxReleaseImage(&src_image));
    VX_CALL(vxReleaseScalar(&shift_convertdepth));

    ASSERT(dst1_x_image == 0);
    ASSERT(dst1_y_image == 0);
    ASSERT(dst2_x_image == 0);
    ASSERT(dst2_y_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_super_node, arg_->width * arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
}

TEST_WITH_ARG(tivxSobel3x3, testSobel3x3SupernodeMultiNode2, Filter_Arg,
    SUPERNODE_PARAMETERS
)
{
    int node_count = 5;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst1_x_image = 0, dst1_y_image = 0;
    vx_image dst2_x_image = 0, virt1, virt2, virt3, virt4;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_scalar shift_convertdepth = 0;
    vx_int32 sh = 0;
    vx_rectangle_t rect;
    vx_bool valid_rect;
    vx_rectangle_t img_rect = { 0, 0, arg_->width-4, arg_->width-4 };
    vx_map_id map_id;
    vx_imagepatch_addressing_t addr = { 0 };
    vx_uint8* base_ptr = NULL;
    vx_pixel_value_t d2_val = {{ 1 }};
    vx_uint32 x, y;

    CT_Image src = NULL, dst1_x = NULL, dst1_y = NULL, dst2_x = NULL, virt_ctimage1 = NULL, virt_ctimage2 = NULL, dst2_ref = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst1_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst1_x_image, VX_TYPE_IMAGE);

    dst2_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_U8);
    ASSERT_VX_OBJECT(dst2_x_image, VX_TYPE_IMAGE);

    dst1_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst1_y_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt3   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt4   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(shift_convertdepth = vxCreateScalar(context, VX_TYPE_INT32, &sh), VX_TYPE_SCALAR);

    node1 = vxSobel3x3Node(graph, src_image, virt1, virt2);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    node2 = vxConvertDepthNode(graph, virt1, virt3, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    node3 = vxConvertDepthNode(graph, virt2, virt4, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
    ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);

    node4 = vxSobel3x3Node(graph, virt3, dst1_x_image, dst1_y_image);
    ASSERT_VX_OBJECT(node4, VX_TYPE_NODE);

    node5 = vxNotNode(graph, virt4, dst2_x_image);
    ASSERT_VX_OBJECT(node5, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node4, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node5, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_NO_FAILURE(node_list[0] = node1); 
    ASSERT_NO_FAILURE(node_list[1] = node2);
    ASSERT_NO_FAILURE(node_list[2] = node3); 
    ASSERT_NO_FAILURE(node_list[3] = node4);
    ASSERT_NO_FAILURE(node_list[4] = node5); 
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    vxGetValidRegionImage(dst1_x_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    vxGetValidRegionImage(dst1_y_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    vxGetValidRegionImage(dst2_x_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 2));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 2));

    ASSERT_NO_FAILURE(dst1_x = ct_image_from_vx_image(dst1_x_image));
    ASSERT_NO_FAILURE(dst1_y = ct_image_from_vx_image(dst1_y_image));

    ASSERT_NO_FAILURE(dst2_x = ct_image_from_vx_image(dst2_x_image));
    ASSERT_NO_FAILURE(dst2_ref = ct_image_from_vx_image(dst2_x_image));

    virt_ctimage1 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
    virt_ctimage2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    ASSERT_NO_FAILURE(sobel3x3_sequential_check2(src, virt_ctimage1, virt_ctimage2, dst1_x, dst1_y, dst2_x, dst2_ref, border));

    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(node5 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&virt3));
    VX_CALL(vxReleaseImage(&virt4));
    VX_CALL(vxReleaseImage(&dst1_x_image));
    VX_CALL(vxReleaseImage(&dst1_y_image));
    VX_CALL(vxReleaseImage(&dst2_x_image));
    VX_CALL(vxReleaseImage(&src_image));
    VX_CALL(vxReleaseScalar(&shift_convertdepth));

    ASSERT(dst1_x_image == 0);
    ASSERT(dst1_y_image == 0);
    ASSERT(dst2_x_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_super_node, arg_->width * arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
}

TEST_WITH_ARG(tivxSobel3x3, testSobel3x3SupernodeMultiNode3, Filter_Arg,
    SUPERNODE_PARAMETERS
)
{
    int node_count = 3;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst1 = 0, dst2 = 0;
    vx_image virt1, virt2;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_scalar shift_convertdepth = 0;
    vx_int32 sh = 0;
    vx_rectangle_t rect;
    vx_bool valid_rect;

    CT_Image src = NULL, vxdst1 = NULL, vxdst2 = NULL, dst1_ref = NULL, dst2_ref = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst1 = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_U8);
    ASSERT_VX_OBJECT(dst1, VX_TYPE_IMAGE);

    dst2 = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_U8);
    ASSERT_VX_OBJECT(dst2, VX_TYPE_IMAGE);

    dst1_ref = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
    dst2_ref = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
    //vxdst = ct_image_from_vx_image(dst);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(shift_convertdepth = vxCreateScalar(context, VX_TYPE_INT32, &sh), VX_TYPE_SCALAR);

    node1 = vxSobel3x3Node(graph, src_image, virt1, virt2);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    node2 = vxConvertDepthNode(graph, virt1, dst1, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    node3 = vxConvertDepthNode(graph, virt2, dst2, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
    ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_NO_FAILURE(node_list[0] = node1); 
    ASSERT_NO_FAILURE(node_list[1] = node2);
    ASSERT_NO_FAILURE(node_list[2] = node3);
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    vxGetValidRegionImage(dst1, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 2));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 2));

    vxGetValidRegionImage(dst2, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 2));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 2));

    ASSERT_NO_FAILURE(vxdst1 = ct_image_from_vx_image(dst1));
    ASSERT_NO_FAILURE(vxdst2 = ct_image_from_vx_image(dst2));

    ASSERT_NO_FAILURE(sobel3x3_sequential_check3(src, vxdst1, vxdst2, dst1_ref, dst2_ref, border));

    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst1));
    VX_CALL(vxReleaseImage(&dst2));
    VX_CALL(vxReleaseImage(&src_image));
    VX_CALL(vxReleaseScalar(&shift_convertdepth));

    ASSERT(dst1 == 0);
    ASSERT(dst2 == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_super_node, arg_->width * arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
}

TEST_WITH_ARG(tivxSobel3x3, testSobel3x3SupernodeOptionalParametersX, Filter_Arg,
    SUPERNODE_PARAMETERS
)
{
    int node_count = 1;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_x_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    CT_Image src = NULL, dst_x = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst_x_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxSobel3x3Node(graph, src_image, dst_x_image, NULL);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_NO_FAILURE(node_list[0] = node); 
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    ASSERT_NO_FAILURE(dst_x = ct_image_from_vx_image(dst_x_image));

    ASSERT_NO_FAILURE(sobel3x3_check_x(src, dst_x, border));

    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_x_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_x_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_super_node, arg_->width * arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
}

TEST_WITH_ARG(tivxSobel3x3, testSobel3x3SupernodeOptionalParametersY, Filter_Arg,
    SUPERNODE_PARAMETERS
)
{
    int node_count = 1;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_y_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    CT_Image src = NULL, dst_y = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst_y_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxSobel3x3Node(graph, src_image, NULL, dst_y_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_NO_FAILURE(node_list[0] = node); 
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    
    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    ASSERT_NO_FAILURE(dst_y = ct_image_from_vx_image(dst_y_image));

    ASSERT_NO_FAILURE(sobel3x3_check_y(src, dst_y, border));

    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_y_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_y_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_super_node, arg_->width * arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
}

TEST_WITH_ARG(tivxSobel3x3, testSobel3x3SupernodeBranches, Filter_Arg,
    SUPERNODE_PARAMETERS
)
{
    /*this test case is to test the variying block width, heights in different branches of the graph*/
    int node_count = 7;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst1_x_image = 0, dst1_y_image = 0, dst0_x_image = 0, dst0_y_image = 0;
    vx_image dst2_x_image = 0, dst2_y_image = 0, virt1, virt2, virt3, virt4, virt5, virt6;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0, node6 = 0, node7 = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_scalar shift_convertdepth = 0;
    vx_int32 sh = 0;
    vx_rectangle_t rect;
    vx_bool valid_rect;
    vx_map_id map_id;
    vx_imagepatch_addressing_t addr = { 0 };
    vx_int16* base_ptr = NULL;
    vx_pixel_value_t d2_val = {{ 1 }};
    vx_uint32 x, y;

    CT_Image src = NULL, dst0_x = NULL, dst0_y = NULL, dst1_x = NULL, dst1_y = NULL, dst2_x = NULL, dst2_y = NULL;
    CT_Image virt_ctimage1 = NULL, virt_ctimage2 = NULL, virt_ctimage3 = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst0_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst0_x_image, VX_TYPE_IMAGE);

    dst0_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst0_x_image, VX_TYPE_IMAGE);

    dst1_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst1_y_image, VX_TYPE_IMAGE);

    dst2_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst2_x_image, VX_TYPE_IMAGE);

    dst2_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst2_y_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt3   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt4   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt5   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt6   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(shift_convertdepth = vxCreateScalar(context, VX_TYPE_INT32, &sh), VX_TYPE_SCALAR);

    node1 = vxSobel3x3Node(graph, src_image, virt1, virt2);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    node2 = vxConvertDepthNode(graph, virt1, virt3, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    node3 = vxConvertDepthNode(graph, virt2, virt4, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
    ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);

    node4 = vxSobel3x3Node(graph, virt3, virt5, dst1_y_image);
    ASSERT_VX_OBJECT(node4, VX_TYPE_NODE);

    node5 = vxConvertDepthNode(graph, virt5, virt6, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
    ASSERT_VX_OBJECT(node5, VX_TYPE_NODE);

    node6 = vxSobel3x3Node(graph, virt6, dst0_x_image, dst0_y_image);
    ASSERT_VX_OBJECT(node6, VX_TYPE_NODE);

    node7 = vxSobel3x3Node(graph, virt4, dst2_x_image, dst2_y_image);
    ASSERT_VX_OBJECT(node7, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node4, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node6, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node7, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_NO_FAILURE(node_list[0] = node1); 
    ASSERT_NO_FAILURE(node_list[1] = node2);
    ASSERT_NO_FAILURE(node_list[2] = node3); 
    ASSERT_NO_FAILURE(node_list[3] = node4);
    ASSERT_NO_FAILURE(node_list[4] = node5); 
    ASSERT_NO_FAILURE(node_list[5] = node6); 
    ASSERT_NO_FAILURE(node_list[6] = node7); 
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    vxGetValidRegionImage(dst0_x_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 6));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 6));

    vxGetValidRegionImage(dst0_y_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 6));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 6));

    vxGetValidRegionImage(dst1_y_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    vxGetValidRegionImage(dst2_x_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    vxGetValidRegionImage(dst2_y_image, &rect);

    ASSERT_EQ_INT((rect.end_x - rect.start_x), (arg_->width - 4));
    ASSERT_EQ_INT((rect.end_y - rect.start_y), (arg_->height - 4));

    ASSERT_NO_FAILURE(dst0_x = ct_image_from_vx_image(dst0_x_image));
    ASSERT_NO_FAILURE(dst0_y = ct_image_from_vx_image(dst0_y_image));

    ASSERT_NO_FAILURE(dst1_y = ct_image_from_vx_image(dst1_y_image));

    ASSERT_NO_FAILURE(dst2_x = ct_image_from_vx_image(dst2_x_image));
    ASSERT_NO_FAILURE(dst2_y = ct_image_from_vx_image(dst2_y_image));

    virt_ctimage1 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
    virt_ctimage2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
    virt_ctimage3 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    ASSERT_NO_FAILURE(sobel3x3_sequential_check4(src, virt_ctimage1, virt_ctimage2, virt_ctimage3, \
                                                 dst0_x, dst0_y, dst1_y, dst2_x, dst2_y, border));

    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseNode(&node6));
    VX_CALL(vxReleaseNode(&node7));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(node5 == 0);
    ASSERT(node6 == 0);
    ASSERT(node7 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&virt3));
    VX_CALL(vxReleaseImage(&virt4));
    VX_CALL(vxReleaseImage(&virt5));
    VX_CALL(vxReleaseImage(&virt6));
    VX_CALL(vxReleaseImage(&dst0_x_image));
    VX_CALL(vxReleaseImage(&dst0_y_image));
    VX_CALL(vxReleaseImage(&dst1_y_image));
    VX_CALL(vxReleaseImage(&dst2_x_image));
    VX_CALL(vxReleaseImage(&dst2_y_image));
    VX_CALL(vxReleaseImage(&src_image));
    VX_CALL(vxReleaseScalar(&shift_convertdepth));

    ASSERT(dst0_x_image == 0);
    ASSERT(dst0_y_image == 0);
    ASSERT(dst1_y_image == 0);
    ASSERT(dst2_x_image == 0);
    ASSERT(dst2_y_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_super_node, arg_->width * arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
}

#ifdef BUILD_BAM
#define testSobel3x3Supernode testSobel3x3Supernode
#define testSobel3x3SupernodeMultiNode testSobel3x3SupernodeMultiNode
#define testSobel3x3SupernodeMultiNode2 testSobel3x3SupernodeMultiNode2
#define testSobel3x3SupernodeMultiNode3 testSobel3x3SupernodeMultiNode3
#define testSobel3x3SupernodeOptionalParametersX testSobel3x3SupernodeOptionalParametersX
#define testSobel3x3SupernodeOptionalParametersY testSobel3x3SupernodeOptionalParametersY
#else
#define testSobel3x3Supernode DISABLED_testSobel3x3Supernode
#define testSobel3x3SupernodeMultiNode DISABLED_testSobel3x3SupernodeMultiNode
#define testSobel3x3SupernodeMultiNode2 DISABLED_testSobel3x3SupernodeMultiNode2
#define testSobel3x3SupernodeMultiNode3 DISABLED_testSobel3x3SupernodeMultiNode3
#define testSobel3x3SupernodeOptionalParametersX DISABLED_testSobel3x3SupernodeOptionalParametersX
#define testSobel3x3SupernodeOptionalParametersY DISABLED_testSobel3x3SupernodeOptionalParametersY
#endif

TESTCASE_TESTS(tivxSobel3x3, 
               testGraphProcessing, 
               testValidRegion, 
               testOptionalParametersX, 
               testOptionalParametersY, 
               negativeTestBorderMode,
               testSobel3x3Supernode,
               testSobel3x3SupernodeMultiNode,
               testSobel3x3SupernodeMultiNode2,
               testSobel3x3SupernodeMultiNode3,
               testSobel3x3SupernodeOptionalParametersX,
               testSobel3x3SupernodeOptionalParametersY/*,
               testSobel3x3SupernodeBranches*/)
