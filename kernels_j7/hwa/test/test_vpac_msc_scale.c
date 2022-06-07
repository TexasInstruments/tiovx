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
#ifdef BUILD_VPAC_MSC

#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx.h>
#include "test_engine/test.h"
#include <string.h>

#include <math.h> // floorf
#include "test_hwa_common.h"

TESTCASE(tivxHwaVpacMscScale, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* testName;
    char* target_string;
    int dummy;
} ArgCreate;

#if defined(SOC_J784S4)
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC_MSC2)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC2_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC2_MSC2))
#else
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC_MSC2))
#endif

#define ADD_DUMMY(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "", __VA_ARGS__, 0))

#define PARAMETERS_CREATE \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS, ADD_DUMMY, ARG)


TEST_WITH_ARG(tivxHwaVpacMscScale, testNodeCreation, ArgCreate, PARAMETERS_CREATE)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = vxScaleImageNode(graph, src_image, dst_image, VX_INTERPOLATION_NEAREST_NEIGHBOR), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(dst_image == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}

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

        // If the computed coordinate is very close to the boundary (1e-7), we don't
        // consider it out-of-bound, in order to handle potential float accuracy errors
        vx_bool defined = (vx_bool)((p00 != -1) && (p10 != -1) && (p01 != -1) && (p11 != -1));
        if (defined == vx_false_e)
        {
            vx_bool defined_any = (vx_bool)((p00 != -1) || (p10 != -1) || (p01 != -1) || (p11 != -1));
            if (defined_any)
            {
                if ((p00 == -1 || p10 == -1) && fabs(t - 1.0) <= 1e-7)
                    p00 = p10 = 0;
                else if ((p01 == -1 || p11 == -1) && fabs(t - 0.0) <= 1e-7)
                    p01 = p11 = 0;
                if ((p00 == -1 || p01 == -1) && fabs(s - 1.0) <= 1e-7)
                    p00 = p01 = 0;
                else if ((p10 == -1 || p11 == -1) && fabs(s - 0.0) <= 1e-7)
                    p10 = p11 = 0;
                defined = (vx_bool)((p00 != -1) && (p10 != -1) && (p01 != -1) && (p11 != -1));
            }
        }
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

        // If the computed coordinate is very close to the boundary (1e-7), we don't
        // consider it out-of-bound, in order to handle potential float accuracy errors
        vx_bool defined = (vx_bool)((p00 != -1) && (p10 != -1) && (p01 != -1) && (p11 != -1));
        if (defined == vx_false_e)
        {
            vx_bool defined_any = (vx_bool)((p00 != -1) || (p10 != -1) || (p01 != -1) || (p11 != -1));
            if (defined_any)
            {
                if ((p00 == -1 || p10 == -1) && fabs(t - 1.0) <= 1e-7)
                    p00 = p10 = 0;
                else if ((p01 == -1 || p11 == -1) && fabs(t - 0.0) <= 1e-7)
                    p01 = p11 = 0;
                if ((p00 == -1 || p01 == -1) && fabs(s - 1.0) <= 1e-7)
                    p00 = p01 = 0;
                else if ((p10 == -1 || p11 == -1) && fabs(s - 0.0) <= 1e-7)
                    p10 = p11 = 0;
                defined = (vx_bool)((p00 != -1) && (p10 != -1) && (p01 != -1) && (p11 != -1));
            }
        }
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
    int dummy;
    vx_enum interpolation;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    void (*dst_size_generator)(int width, int height, int* dst_width, int* dst_height);
    int exact_result;
    int width, height;
    char* target_string;
    vx_border_t border;
} Arg;


static void dst_size_generator_1_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width;
    *dst_height = height;
}

static void dst_size_generator_1_2(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width * 2;
    *dst_height = height * 2;
}

static void dst_size_generator_1_3(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width * 3;
    *dst_height = height * 3;
}

static void dst_size_generator_2_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (width + 1) / 2;
    *dst_height = (height + 1) / 2;
}

static void dst_size_generator_3_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (width + 2) / 3;
    *dst_height = (height + 2) / 3;
}

static void dst_size_generator_4_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (width + 3) / 4;
    *dst_height = (height + 3) / 4;
}

static void dst_size_generator_5_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (width + 4) / 5;
    *dst_height = (height + 4) / 5;
}

static void dst_size_generator_SCALE_PYRAMID_ORB(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (int)(width * VX_SCALE_PYRAMID_ORB);
    *dst_height = (int)(height * VX_SCALE_PYRAMID_ORB);
}

static void dst_size_generator_SCALE_NEAR_UP(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width + 1;
    *dst_height = height + 1;
}

static void dst_size_generator_SCALE_NEAR_DOWN(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width - 1;
    *dst_height = height - 1;
}

#define STR_VX_INTERPOLATION_NEAREST_NEIGHBOR "NN"
#define STR_VX_INTERPOLATION_BILINEAR "BILINEAR"
#define STR_VX_INTERPOLATION_AREA "AREA"

#define SCALE_TEST(interpolation, inputDataGenerator, inputDataFile, scale, exact, nextmacro, ...) \
    CT_EXPAND(nextmacro(STR_##interpolation "/" inputDataFile "/" #scale, __VA_ARGS__, \
            interpolation, inputDataGenerator, inputDataFile, dst_size_generator_ ## scale, exact))

#define ADD_DST_SIZE_NN(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/1_1", __VA_ARGS__, dst_size_generator_1_1)), \
    CT_EXPAND(nextmacro(testArgName "/1_2", __VA_ARGS__, dst_size_generator_1_2)), \
    CT_EXPAND(nextmacro(testArgName "/2_1", __VA_ARGS__, dst_size_generator_2_1)), \
    CT_EXPAND(nextmacro(testArgName "/3_1", __VA_ARGS__, dst_size_generator_3_1)), \
    CT_EXPAND(nextmacro(testArgName "/4_1", __VA_ARGS__, dst_size_generator_4_1))

#define ADD_DST_SIZE_BILINEAR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/1:1", __VA_ARGS__, dst_size_generator_1_1)), \
    CT_EXPAND(nextmacro(testArgName "/1:2", __VA_ARGS__, dst_size_generator_1_2)), \
    CT_EXPAND(nextmacro(testArgName "/2:1", __VA_ARGS__, dst_size_generator_2_1)), \
    CT_EXPAND(nextmacro(testArgName "/3:1", __VA_ARGS__, dst_size_generator_3_1)), \
    CT_EXPAND(nextmacro(testArgName "/4:1", __VA_ARGS__, dst_size_generator_4_1))

#define ADD_DST_SIZE_AREA(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/1:1", __VA_ARGS__, dst_size_generator_1_1)), \
    CT_EXPAND(nextmacro(testArgName "/87:100", __VA_ARGS__, dst_size_generator_87_100)), \
    CT_EXPAND(nextmacro(testArgName "/4:1", __VA_ARGS__, dst_size_generator_4_1))

#define ADD_SIZE_96x96(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=96x96", __VA_ARGS__, 96, 96))

#define ADD_SIZE_100x100(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=100x100", __VA_ARGS__, 100, 100))

#define PARAMETERS \
    /* 1:1 scale */ \
    SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_1, 1, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    SCALE_TEST(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 1_1, 1, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    SCALE_TEST(VX_INTERPOLATION_AREA,             scale_generate_random, "random", 1_1, 1, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /* NN upscale with integer factor */ \
    /* SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_2, 1, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_3, 1, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 1_2, 1, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* NN downscale with odd integer factor */\
    /* SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 3_1, 1, ADD_SIZE_96x96, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */\
    /* SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 5_1, 1, ADD_SIZE_100x100, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_pattern3x3, "pattern3x3", 3_1, 1, ADD_SIZE_96x96, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 3_1, 0, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */\
    /* other NN downscales */ \
    SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 2_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 4_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /* SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", SCALE_PYRAMID_ORB, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* BILINEAR upscale with integer factor */ \
    /* SCALE_TEST(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 1_2, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 1_3, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* BILINEAR downscales */ \
    /*SCALE_TEST(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 2_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 3_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0),*/ \
    /* SCALE_TEST(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 4_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 5_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", SCALE_PYRAMID_ORB, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* AREA tests */ \
    SCALE_TEST(VX_INTERPOLATION_AREA,             scale_generate_gradient_16x16, "gradient16x16", 4_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    SCALE_TEST(VX_INTERPOLATION_AREA,             scale_read_image, "lena.bmp", 4_1, 0, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /* AREA upscale */ \
    /* SCALE_TEST(VX_INTERPOLATION_AREA,             scale_generate_random, "random", 1_2, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST(VX_INTERPOLATION_AREA,             scale_generate_random, "random", 1_3, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* other */ \
    /* SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", SCALE_NEAR_UP, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", SCALE_NEAR_UP, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST(VX_INTERPOLATION_AREA,             scale_generate_random, "random", SCALE_NEAR_UP, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", SCALE_NEAR_DOWN, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */\
    /* SCALE_TEST(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", SCALE_NEAR_DOWN, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */\
    /* SCALE_TEST(VX_INTERPOLATION_AREA,             scale_generate_random, "random", SCALE_NEAR_DOWN, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */\

TEST_WITH_ARG(tivxHwaVpacMscScale, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    CT_Image src = NULL, dst = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator(src->width, src->height, &dst_width, &dst_height));

        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = vxScaleImageNode(graph, src_image, dst_image, arg_->interpolation), VX_TYPE_NODE);

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

        ASSERT_NO_FAILURE(scale_check(src, dst, arg_->interpolation, arg_->border, arg_->exact_result));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));

        tivxHwaUnLoadKernels(context);
    }

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

TEST_WITH_ARG(tivxHwaVpacMscScale, testImmediateProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image = 0;

    CT_Image src = NULL, dst = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator(src->width, src->height, &dst_width, &dst_height));

        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        VX_CALL(vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER, &arg_->border, sizeof(arg_->border)));

        ASSERT_NO_FAILURE(vxSetImmediateModeTarget(context, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxuScaleImage(context, src_image, dst_image, arg_->interpolation));

        ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

        ASSERT_NO_FAILURE(scale_check(src, dst, arg_->interpolation, arg_->border, arg_->exact_result));

        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));

        tivxHwaUnLoadKernels(context);
    }

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

TESTCASE_TESTS(tivxHwaVpacMscScale, testNodeCreation, testGraphProcessing, testImmediateProcessing)

#endif /* BUILD_VPAC_MSC */