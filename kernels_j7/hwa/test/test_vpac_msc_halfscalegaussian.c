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

#include <math.h> // floor
#include "test_hwa_common.h"

#define HALFSCALEGAUSSIAN_TOLERANCE 1
#define CHECK_OUTPUT

TESTCASE(tivxHwaVpacMscHalfScaleGaussian, CT_VXContext, ct_setup_vx_context, 0)

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


TEST_WITH_ARG(tivxHwaVpacMscHalfScaleGaussian, testNodeCreation, ArgCreate, PARAMETERS_CREATE)
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
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 64, 64, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = vxHalfScaleGaussianNode(graph, src_image, dst_image, 3), VX_TYPE_NODE);
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

static CT_Image halfScaleGaussian_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static CT_Image halfScaleGaussian_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
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

static void halfScaleGaussian_check(CT_Image src, CT_Image dst, vx_int32 kernel_size, vx_border_t border)
{
    ASSERT(src && dst);
    EXPECT_EQ_INT((int)ceil(src->width * 0.5), dst->width);
    EXPECT_EQ_INT((int)ceil(src->height * 0.5), dst->height);
    halfScaleGaussian_validate(src, dst, kernel_size, border);
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
    char* target_string;
    vx_int32 kernel_size;
    vx_border_t border;
} Arg;


#define ADD_KERNEL_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/k=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/k=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/k=5", __VA_ARGS__, 5))

#define ADD_MSC(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/MSC_1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/MSC_2", __VA_ARGS__, 2))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_KERNEL_SIZE, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ARG, halfScaleGaussian_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("lena", ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_KERNEL_SIZE, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ARG, halfScaleGaussian_read_image, "lena.bmp"), \

TEST_WITH_ARG(tivxHwaVpacMscHalfScaleGaussian, testGraphProcessing, Arg,
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

        dst_width = (src->width + 1) / 2;
        dst_height = (src->height + 1) / 2;

        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = vxHalfScaleGaussianNode(graph, src_image, dst_image, arg_->kernel_size), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

        #ifdef CHECK_OUTPUT
        ASSERT_NO_FAILURE(halfScaleGaussian_check(src, dst, arg_->kernel_size, arg_->border));
        #endif

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

TEST_WITH_ARG(tivxHwaVpacMscHalfScaleGaussian, testImmediateProcessing, Arg,
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

        dst_width = (src->width + 1) / 2;
        dst_height = (src->height + 1) / 2;

        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        VX_CALL(vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER, &arg_->border, sizeof(arg_->border)));

        ASSERT_NO_FAILURE(vxSetImmediateModeTarget(context, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxuHalfScaleGaussian(context, src_image, dst_image, arg_->kernel_size));

        ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

        #ifdef CHECK_OUTPUT
        ASSERT_NO_FAILURE(halfScaleGaussian_check(src, dst, arg_->kernel_size, arg_->border));
        #endif

        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));

        tivxHwaUnLoadKernels(context);
    }

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

TESTCASE_TESTS(tivxHwaVpacMscHalfScaleGaussian,
        testNodeCreation,
        testGraphProcessing,
        testImmediateProcessing)

#endif /* BUILD_VPAC_MSC */