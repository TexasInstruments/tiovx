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

#include <math.h> // floorf

TESTCASE(tivxScale, CT_VXContext, ct_setup_vx_context, 0)

static CT_Image scale_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static CT_Image _scale_generate_simple_gradient(int width, int height, int step_x, int step_y, int offset)
{
    CT_Image image = NULL;
    uint32_t x, y;

    ASSERT_(return 0, step_x > 0);
    ASSERT_(return 0, step_y > 0);

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_image(width, height, VX_DF_IMAGE_U8));

    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x++)
        {
            uint8_t* ptr = CT_IMAGE_DATA_PTR_8U(image, x, y);
            int v = offset + (y / step_y) + (x / step_x);
            *ptr = (uint8_t)v;
        }
    }

    return image;
}

static CT_Image scale_generate_gradient_2x2(const char* fileName, int width, int height)
{
    return _scale_generate_simple_gradient(width, height, 2, 2, 0);
}

static CT_Image scale_generate_gradient_16x16(const char* fileName, int width, int height)
{
    return _scale_generate_simple_gradient(width, height, 16, 16, 32);
}

static CT_Image scale_generate_pattern3x3(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    uint32_t x, y;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_image(width, height, VX_DF_IMAGE_U8));

    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x++)
        {
            uint8_t* ptr = CT_IMAGE_DATA_PTR_8U(image, x, y);
            int v = ((y % 3) == 1 && (x % 3) == 1) ? 0 : 255;
            *ptr = (uint8_t)v;
        }
    }

    return image;
}

static CT_Image scale_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

static vx_int32 ct_image_get_pixel_8u(CT_Image img, int x, int y, vx_border_t border)
{
    if (border.mode == VX_BORDER_UNDEFINED)
    {
        if (x < 0 || x >= (int)img->width || y < 0 || y >= (int)img->height)
            return -1; //border
        return *CT_IMAGE_DATA_PTR_8U(img, x, y);
    }
    else if (border.mode == VX_BORDER_REPLICATE)
    {
        return CT_IMAGE_DATA_REPLICATE_8U(img, x, y);
    }
    else if (border.mode == VX_BORDER_CONSTANT)
    {
        return CT_IMAGE_DATA_CONSTANT_8U(img, x, y, border.constant_value.U8);
    }
    else
    {
        CT_FAIL_(return -1, "Invalid border type");
    }
}

static int scale_check_pixel(CT_Image src, CT_Image dst, int x, int y, vx_enum interpolation, vx_border_t border)
{
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(dst, x, y);
    vx_float32 x_src = (((vx_float32)x + 0.5f) * (vx_float32)src->width / (vx_float32)dst->width) - 0.5f;
    vx_float32 y_src = (((vx_float32)y + 0.5f) * (vx_float32)src->height / (vx_float32)dst->height) - 0.5f;
    int x_min = (int)floorf(x_src), y_min = (int)floorf(y_src);
    if (interpolation == VX_INTERPOLATION_NEAREST_NEIGHBOR)
    {
        int sx, sy;
        for (sy = -1; sy <= 1; sy++)
        {
            for (sx = -1; sx <= 1; sx++)
            {
                vx_int32 candidate = 0;
                ASSERT_NO_FAILURE_(return 0, candidate = ct_image_get_pixel_8u(src, x_min + sx, y_min + sy, border));
                if (candidate == -1 || candidate == res)
                    return 1;
            }
        }
        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
    }
    if (interpolation == VX_INTERPOLATION_BILINEAR)
    {
        vx_float32 s = x_src - x_min;
        vx_float32 t = y_src - y_min;
        vx_int32 p00 = ct_image_get_pixel_8u(src, x_min + 0, y_min + 0, border);
        vx_int32 p01 = ct_image_get_pixel_8u(src, x_min + 0, y_min + 1, border);
        vx_int32 p10 = ct_image_get_pixel_8u(src, x_min + 1, y_min + 0, border);
        vx_int32 p11 = ct_image_get_pixel_8u(src, x_min + 1, y_min + 1, border);
        vx_float32 ref_float;
        vx_int32 ref;

        // We don't compare the output when an input pixel is undefined (UNDEFINED border mode)
        vx_bool defined = (vx_bool)((p00 != -1) && (p10 != -1) && (p01 != -1) && (p11 != -1));
        if (defined == vx_false_e) {
            return 1;
        }

        // Compute the expected result (float)
        ref_float = (1 - s) * (1 - t) * p00 +
                    (    s) * (1 - t) * p10 +
                    (1 - s) * (    t) * p01 +
                    (    s) * (    t) * p11;

        // Take the nearest integer to avoid problems with casts in case of float rounding errors
        // (e.g: 30.999999 should give 31, not 30)
        ref = (vx_int32)(ref_float + 0.5f);

        // A difference of 1 is allowed
        if (abs(res - ref) <= 1) {
            return 1;
        }

        return 0; // don't generate failure, we will check num failed pixels later
    }
    if (interpolation == VX_INTERPOLATION_AREA)
    {
        vx_int32 v_min = 256, v_max = -1;
        int sx, sy;
        // check values at 5x5 area
        for (sy = -2; sy <= 2; sy++)
        {
            for (sx = -2; sx <= 2; sx++)
            {
                vx_int32 candidate = 0;
                ASSERT_NO_FAILURE_(return 0, candidate = ct_image_get_pixel_8u(src, x_min + sx, y_min + sy, border));
                if (candidate == -1)
                    return 1;
                if (v_min > candidate)
                    v_min = candidate;
                if (v_max < candidate)
                    v_max = candidate;
            }
            if (v_min <= res && v_max >= res)
                return 1;
        }
        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
    }
    CT_FAIL_(return 0, "NOT IMPLEMENTED");
}

static int scale_check_pixel_exact(CT_Image src, CT_Image dst, int x, int y, vx_enum interpolation, vx_border_t border)
{
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(dst, x, y);
    vx_float32 x_src = (((vx_float32)x + 0.5f) * (vx_float32)src->width / (vx_float32)dst->width) - 0.5f;
    vx_float32 y_src = (((vx_float32)y + 0.5f) * (vx_float32)src->height / (vx_float32)dst->height) - 0.5f;
    vx_float32 x_minf = floorf(x_src);
    vx_float32 y_minf = floorf(y_src);
    int x_min = (vx_int32)x_minf;
    int y_min = (vx_int32)y_minf;
    int x_ref = x_min;
    int y_ref = y_min;
    if (x_src - x_minf >= 0.5f)
        x_ref++;
    if (y_src - y_minf >= 0.5f)
        y_ref++;
    if (interpolation == VX_INTERPOLATION_NEAREST_NEIGHBOR)
    {
        vx_int32 ref = ct_image_get_pixel_8u(src, x_ref, y_ref, border);
        if (ref == -1 || ref == res)
            return 1;
        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d (expected %d)", x, y, (int)res, (int)ref);
    }
    if (interpolation == VX_INTERPOLATION_BILINEAR)
    {
        vx_float32 s = x_src - x_minf;
        vx_float32 t = y_src - y_minf;
        vx_int32 p00 = ct_image_get_pixel_8u(src, x_min + 0, y_min + 0, border);
        vx_int32 p01 = ct_image_get_pixel_8u(src, x_min + 0, y_min + 1, border);
        vx_int32 p10 = ct_image_get_pixel_8u(src, x_min + 1, y_min + 0, border);
        vx_int32 p11 = ct_image_get_pixel_8u(src, x_min + 1, y_min + 1, border);
        vx_float32 ref_float;
        vx_int32 ref;

        // We don't compare the output when an input pixel is undefined (UNDEFINED border mode)
        vx_bool defined = (vx_bool)((p00 != -1) && (p10 != -1) && (p01 != -1) && (p11 != -1));
        if (defined == vx_false_e) {
            return 1;
        }

        // Compute the expected result (float)
        ref_float = (1 - s) * (1 - t) * p00 +
                    (    s) * (1 - t) * p10 +
                    (1 - s) * (    t) * p01 +
                    (    s) * (    t) * p11;

        // Take the nearest integer to avoid problems with casts in case of float rounding errors
        // (e.g: 30.999999 should give 31, not 30)
        ref = (vx_int32)(ref_float + 0.5f);

        // The result must be exact
        if (ref == res) {
            return 1;
        }

        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d (expected %d)", x, y, (int)res, (int)ref);
    }
    if (interpolation == VX_INTERPOLATION_AREA)
    {
        vx_int32 ref;
        ASSERT_(return 0, dst->width % src->width == 0 && dst->height % src->height == 0);
        ref = ct_image_get_pixel_8u(src, x_ref, y_ref, border);
        if (ref == -1)
            return 1;
        if (ref == res)
            return 1;
        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d (expected %d)", x, y, (int)res, (int)ref);
    }
    CT_FAIL_(return 0, "NOT IMPLEMENTED");
}

static void scale_validate(CT_Image src, CT_Image dst, vx_enum interpolation, vx_border_t border, int exact)
{
    int num_failed = 0;
    if (src->width == dst->width && src->height == dst->height) // special case for scale=1.0
    {
        ASSERT_EQ_CTIMAGE(src, dst);
        return;
    }
    CT_FILL_IMAGE_8U(, dst,
            {
                int check;
                if (exact == 0)
                    ASSERT_NO_FAILURE(check = scale_check_pixel(src, dst, x, y, interpolation, border));
                else
                    ASSERT_NO_FAILURE(check = scale_check_pixel_exact(src, dst, x, y, interpolation, border));
                if (check == 0) {
                    num_failed++;
                }
            });
    if (interpolation == VX_INTERPOLATION_BILINEAR)
    {
        int total = dst->width * dst->height;
        if (num_failed * 100 > total * 2) // 98% should be valid
        {
            CT_FAIL("Check failed: %g (%d) pixels are wrong", (float)num_failed / total, num_failed);
        }
    }
}

static void scale_check(CT_Image src, CT_Image dst, vx_enum interpolation, vx_border_t border, int exact)
{
    ASSERT(src && dst);
    scale_validate(src, dst, interpolation, border, exact);
}

typedef struct {
    const char* testName;
    int dummy;
    vx_enum interpolation;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    void (*dst_size_generator)(int width, int height, int* dst_width, int* dst_height);
    int exact_result;
    int width, height;
    vx_border_t border;
} Arg;


void tivx_dst_size_generator_1_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width;
    *dst_height = height;
}

void tivx_dst_size_generator_1_2(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width * 2;
    *dst_height = height * 2;
}

void tivx_dst_size_generator_1_3(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width * 3;
    *dst_height = height * 3;
}

void tivx_dst_size_generator_2_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (width + 1) / 2;
    *dst_height = (height + 1) / 2;
}

void tivx_dst_size_generator_3_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (width + 2) / 3;
    *dst_height = (height + 2) / 3;
}

void tivx_dst_size_generator_4_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (width + 3) / 4;
    *dst_height = (height + 3) / 4;
}

void tivx_dst_size_generator_5_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (width + 4) / 5;
    *dst_height = (height + 4) / 5;
}

void tivx_dst_size_generator_SCALE_PYRAMID_ORB(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (int)(width * VX_SCALE_PYRAMID_ORB);
    *dst_height = (int)(height * VX_SCALE_PYRAMID_ORB);
}

void tivx_dst_size_generator_SCALE_NEAR_UP(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width + 1;
    *dst_height = height + 1;
}

void tivx_dst_size_generator_SCALE_NEAR_DOWN(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width - 1;
    *dst_height = height - 1;
}

#define STR_VX_INTERPOLATION_NEAREST_NEIGHBOR "NN"
#define STR_VX_INTERPOLATION_BILINEAR "BILINEAR"
#define STR_VX_INTERPOLATION_AREA "AREA"

#define SCALE_TEST(interpolation, inputDataGenerator, inputDataFile, scale, exact, nextmacro, ...) \
    CT_EXPAND(nextmacro(STR_##interpolation "/" inputDataFile "/" #scale, __VA_ARGS__, \
            interpolation, inputDataGenerator, inputDataFile, tivx_dst_size_generator_ ## scale, exact))

#define PARAMETERS \
    SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_2, 2, ADD_SIZE_18x18, ADD_VX_BORDERS, ARG, 0), \
    SCALE_TEST(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 1_2, 2, ADD_SIZE_644x258, ADD_VX_BORDERS, ARG, 0), \
    SCALE_TEST(VX_INTERPOLATION_AREA,             scale_generate_random, "random", 1_2, 2, ADD_SIZE_1600x1200, ADD_VX_BORDERS, ARG, 0), \

TEST_WITH_ARG(tivxScale, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    int dst_width = 0, dst_height = 0;
    vx_image src0_image = 0, dst0_image = 0, src1_image = 0, dst1_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;

    CT_Image src0 = NULL, dst0 = NULL, src1 = NULL, dst1 = NULL;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(src0 = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src0_image = ct_image_to_vx_image(src0, context), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(src1 = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src1_image = ct_image_to_vx_image(src1, context), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(arg_->dst_size_generator(src0->width, src0->height, &dst_width, &dst_height));
    ASSERT_NO_FAILURE(arg_->dst_size_generator(src1->width, src1->height, &dst_width, &dst_height));

    ASSERT_VX_OBJECT(dst0_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst1_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxScaleImageNode(graph, src0_image, dst0_image, arg_->interpolation), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxScaleImageNode(graph, src1_image, dst1_image, arg_->interpolation), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(dst0 = ct_image_from_vx_image(dst0_image));
    ASSERT_NO_FAILURE(dst1 = ct_image_from_vx_image(dst1_image));

    ASSERT_NO_FAILURE(scale_check(src0, dst0, arg_->interpolation, arg_->border, arg_->exact_result));
    ASSERT_NO_FAILURE(scale_check(src1, dst1, arg_->interpolation, arg_->border, arg_->exact_result));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst0_image));
    VX_CALL(vxReleaseImage(&dst1_image));
    VX_CALL(vxReleaseImage(&src0_image));
    VX_CALL(vxReleaseImage(&src1_image));

    ASSERT(dst0_image == 0);
    ASSERT(dst1_image == 0);
    ASSERT(src0_image == 0);
    ASSERT(src1_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TEST_WITH_ARG(tivxScale, testSequentialNodes, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    int virt_width = 0, virt_height = 0, dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst0_image = 0, dst1_image = 0, virt = 0, int_image;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_node4, perf_graph;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    CT_Image src = NULL, dst0 = NULL, dst1 = NULL, virt_ctimage = NULL, int_ctimage = NULL;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(arg_->dst_size_generator(src->width, src->height, &virt_width, &virt_height));

    ASSERT_NO_FAILURE(arg_->dst_size_generator(virt_width, virt_height, &dst_width, &dst_height));

    ASSERT_VX_OBJECT(dst0_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst1_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, virt_width, virt_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(int_image   = vxCreateImage(context, virt_width, virt_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxScaleImageNode(graph, src_image, virt, arg_->interpolation), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxScaleImageNode(graph, virt, dst0_image, arg_->interpolation), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxScaleImageNode(graph, src_image, int_image, arg_->interpolation), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxScaleImageNode(graph, int_image, dst1_image, arg_->interpolation), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));
    VX_CALL(vxSetNodeAttribute(node3, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));
    VX_CALL(vxSetNodeAttribute(node4, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src_image, &src_rect);
    vxGetValidRegionImage(dst1_image, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), arg_->width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), arg_->height);

    ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), dst_width);
    ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), dst_height);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryNode(node4, VX_NODE_PERFORMANCE, &perf_node4, sizeof(perf_node4));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(dst0 = ct_image_from_vx_image(dst0_image));
    ASSERT_NO_FAILURE(dst1 = ct_image_from_vx_image(dst1_image));

    ASSERT_NO_FAILURE(int_ctimage = ct_image_from_vx_image(int_image));
    ASSERT_NO_FAILURE(scale_check(src, int_ctimage, arg_->interpolation, arg_->border, arg_->exact_result));
    ASSERT_EQ_CTIMAGE(dst0, dst1);
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

    VX_CALL(vxReleaseImage(&dst0_image));
    VX_CALL(vxReleaseImage(&dst1_image));
    VX_CALL(vxReleaseImage(&int_image));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst0_image == 0);
    ASSERT(dst1_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3, arg_->width*arg_->height, "N3");
    printPerformance(perf_node4, arg_->width*arg_->height, "N4");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TESTCASE_TESTS(tivxScale, testGraphProcessing, testSequentialNodes)
