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
#include "tivx_utils_checksum.h"
#include "tivx_utils_file_rd_wr.h"


TESTCASE(tivxHwaVpacMscScaleMultiOutput, CT_VXContext, ct_setup_vx_context, 0)

static vx_status save_image_from_msc(vx_image y8, char *filename_prefix)
{
    char filename[MAXPATHLENGTH];
    vx_status status;

    snprintf(filename, MAXPATHLENGTH, "%s/%s.bmp",
        ct_get_test_file_path(), filename_prefix);

    status = tivx_utils_save_vximage_to_bmpfile(filename, y8);

    return status;
}

typedef struct {
    const char* testName;
    char* target_string;
    int dummy;
} ArgFixed;

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

#define PARAMETERS_FIX \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS, ADD_DUMMY, ARG)

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testNodeCreation, ArgFixed, PARAMETERS_FIX)
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

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);
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

static void scale_set_output_params(tivx_vpac_msc_output_params_t *params,
    uint32_t interpolation, uint32_t iw, uint32_t ih, uint32_t ow, uint32_t oh)
{
    uint32_t i;
    uint32_t idx;
    uint32_t weight;

    params->signed_data = 0;
    params->filter_mode = 1; // Multi-phase scaling
    params->coef_shift = 8;
    params->saturation_mode = 0;
    params->offset_x = 0;
    params->offset_y = 0;
    //params->output_align_12bit =
    params->multi_phase.phase_mode = 0;
    params->multi_phase.horz_coef_sel = 0;
    params->multi_phase.vert_coef_sel = 0;

    if (VX_INTERPOLATION_BILINEAR == interpolation)
    {
        params->multi_phase.init_phase_x =
                    (((((float)iw/(float)ow) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;

        params->multi_phase.init_phase_y =
                    (((((float)ih/(float)oh) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
    }
    else
    {
        params->multi_phase.init_phase_x = 0;

        params->multi_phase.init_phase_y = 0;
    }
}

static void scale_set_coeff(tivx_vpac_msc_coefficients_t *coeff,
    uint32_t interpolation)
{
    uint32_t i;
    uint32_t idx;
    uint32_t weight;

    idx = 0;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 256;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 0;
    idx = 0;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 256;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 0;

    if (VX_INTERPOLATION_BILINEAR == interpolation)
    {
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = i<<2;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 256-weight;
            coeff->multi_phase[0][idx ++] = weight;
            coeff->multi_phase[0][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = (i+32)<<2;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 256-weight;
            coeff->multi_phase[1][idx ++] = weight;
            coeff->multi_phase[1][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = i<<2;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 256-weight;
            coeff->multi_phase[2][idx ++] = weight;
            coeff->multi_phase[2][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = (i+32)<<2;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 256-weight;
            coeff->multi_phase[3][idx ++] = weight;
            coeff->multi_phase[3][idx ++] = 0;
        }
    }
    else /* STR_VX_INTERPOLATION_NEAREST_NEIGHBOR */
    {
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 256;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 256;
            coeff->multi_phase[1][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 256;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 256;
            coeff->multi_phase[3][idx ++] = 0;
        }
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
        else
        {
            //printf("res = %d\n", res);
            //printf("ref = %d\n", ref);
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
        vx_int32 ref = ct_image_get_pixel_8u(src, x_ref, y_ref, border); // returning an incorrect value
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
    //if (interpolation == VX_INTERPOLATION_BILINEAR)
   // {
        int total = dst->width * dst->height;
        if (num_failed ) // 98% should be valid
        {
            printf("exact = %d\n", exact);
            printf("Check failed: %g (%d) pixels are wrong", (float)num_failed / total, num_failed);
            CT_FAIL("Check failed: %g (%d) pixels are wrong", (float)num_failed / total, num_failed);
        }
    //}
}

static void scale_check(CT_Image src, CT_Image dst, vx_enum interpolation, vx_border_t border, int exact)
{
    ASSERT(src && dst);
    scale_validate(src, dst, interpolation, border, exact);
    /*int i;
    for (i = 0; i < 16*4; i++)
    {
        printf("src->data.y[%d] = %d\n", i, src->data.y[i]);
        printf("dst->data.y[%d] = %d\n", i, dst->data.y[i]);
    }*/
#if 1
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
    int crop_mode;
    uint32_t checksum;
    int width, height;
    char* target_string;
} Arg_OneOutput;

typedef struct {
    const char* testName;
    int dummy;
    vx_enum interpolation;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    void (*dst_size_generator0)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator1)(int width, int height, int* dst_width, int* dst_height);
    int exact_result;
    int width, height;
    char* target_string;
    vx_border_t border;
} Arg_TwoOutput;

typedef struct {
    const char* testName;
    int dummy;
    vx_enum interpolation;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    void (*dst_size_generator0)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator1)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator2)(int width, int height, int* dst_width, int* dst_height);
    int exact_result;
    int width, height;
    char* target_string;
    vx_border_t border;
} Arg_ThreeOutput;

typedef struct {
    const char* testName;
    int dummy;
    vx_enum interpolation;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    void (*dst_size_generator0)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator1)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator2)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator3)(int width, int height, int* dst_width, int* dst_height);
    int exact_result;
    int width, height;
    char* target_string;
    vx_border_t border;
} Arg_FourOutput;

typedef struct {
    const char* testName;
    int dummy;
    vx_enum interpolation;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    void (*dst_size_generator0)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator1)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator2)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator3)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator4)(int width, int height, int* dst_width, int* dst_height);
    int exact_result;
    int width, height;
    char* target_string;
    vx_border_t border;
} Arg_FiveOutput;

static void dst_size_generator_1_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width;
    *dst_height = height;
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

#define ADD_CROP_0(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/crop=none", __VA_ARGS__, 0))

#define ADD_CROP_1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/crop=1", __VA_ARGS__, 1))


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_FixedPattern, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 16, h = 16, i, j, crop_mode = 0;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

/* Note: for debug--load in a file by putting a breakpoint at unmap */
#if 1
        vx_imagepatch_addressing_t image_addr;
        vx_map_id map_id;
        vx_df_image df;
        void *data_ptr, *data_ptr2;
        uint8_t *data_ptr_u8;

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = w;
        rect.end_y = h;

        vxMapImagePatch(src_image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        data_ptr_u8 = data_ptr;

        for(j=0; j < h; j++)
        {
            for(i=0; i<w; i++)
            {
                data_ptr_u8[j*image_addr.stride_y+i] = j*16;
                //printf("%03d,", data_ptr_u8[j*image_addr.stride_y+i]);
            }
            //printf("\n");
            //printf("\n");
        }

        vxUnmapImagePatch(src_image, map_id);
#endif
        dst_width = w-4;
        dst_height = h-4;

        if(crop_mode == 1)
        {
            dst_width /= 2;
            dst_height /= 2;
        }

        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        scale_set_coeff(&coeffs, interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        if(crop_mode == 1)
        {
            /* Set Input Crop */
            ASSERT_VX_OBJECT(crop_obj = vxCreateUserDataObject(context,
                "tivx_vpac_msc_crop_params_t",
                sizeof(tivx_vpac_msc_crop_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            /* Center crop of input */
            crop.crop_start_x = w / 4;
            crop.crop_start_y =h / 4;
            crop.crop_width   = w / 2;
            crop.crop_height  = h / 2;

            VX_CALL(vxCopyUserDataObject(crop_obj, 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop, VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            refs[0] = (vx_reference)crop_obj;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS,
                tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
                refs, 5u));

            VX_CALL(vxReleaseUserDataObject(&crop_obj));
        }

        VX_CALL(vxProcessGraph(graph));

        //ASSERT_NO_FAILURE(src = ct_image_from_vx_image(src_image));
        //ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

#if 0
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        vxMapImagePatch(dst_image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr2,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        data_ptr_u8 = data_ptr2;

        for(j=0; j < dst_height; j++)
        {
            for(i=0; i<dst_width; i++)
            {
                printf("%03d,", data_ptr_u8[j*image_addr.stride_y+i]);
            }
            printf("\n");
        }

        vxUnmapImagePatch(dst_image, map_id);
#endif

        //ASSERT_NO_FAILURE(scale_check(src, dst, arg_->interpolation, arg_->border, arg_->exact_result));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
        //printf("0x%08x\n", checksum_actual);
        //printf("end_x = %d\n", dst_width);
        //printf("end_y = %d\n", dst_height);

        ASSERT((uint32_t)0xdbdbdbcb == checksum_actual);

        //save_image_from_msc(dst_image, "output/lena_msc");

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

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_FixedPattern_tiovx_1129, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 3840, h = 2160, i, j, crop_mode = 0;
    int dst_width = 1280, dst_height = 720;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

/* Note: for debug--load in a file by putting a breakpoint at unmap */
#if 1
        vx_imagepatch_addressing_t image_addr;
        vx_map_id map_id;
        vx_df_image df;
        void *data_ptr, *data_ptr2;
        uint8_t *data_ptr_u8;

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = w;
        rect.end_y = h;

        vxMapImagePatch(src_image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        data_ptr_u8 = data_ptr;

        for(j=0; j < h; j++)
        {
            for(i=0; i<w; i++)
            {
                data_ptr_u8[j*image_addr.stride_y+i] = 128;
                //printf("%03d,", data_ptr_u8[j*image_addr.stride_y+i]);
            }
            //printf("\n");
            //printf("\n");
        }

        vxUnmapImagePatch(src_image, map_id);

        rect.end_y = h/2;
        vxMapImagePatch(src_image,
            &rect,
            1,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        data_ptr_u8 = data_ptr;

        for(j=0; j < h/2; j++)
        {
            for(i=0; i<w; i+=2)
            {
                data_ptr_u8[j*image_addr.stride_y+i] = 128-64;
                data_ptr_u8[j*image_addr.stride_y+i+1] = 128+64;
                //printf("%03d,", data_ptr_u8[j*image_addr.stride_y+i]);
            }
            //printf("\n");
            //printf("\n");
        }

        //printf("%03d,", data_ptr_u8[0]);
        //printf("%03d,", data_ptr_u8[1]);
        //printf("\n");

        vxUnmapImagePatch(src_image, map_id);

#endif
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        scale_set_coeff(&coeffs, interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        VX_CALL(vxProcessGraph(graph));

        //ASSERT_NO_FAILURE(src = ct_image_from_vx_image(src_image));
        //ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

#if 1
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height/2;

        vxMapImagePatch(dst_image,
            &rect,
            1,
            &map_id,
            &image_addr,
            &data_ptr2,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        data_ptr_u8 = data_ptr2;

        //printf("%03d,", data_ptr_u8[0]);
        //printf("%03d,", data_ptr_u8[1]);
        //printf("\n");

        //ASSERT(128+64 == data_ptr_u8[0]);
        //ASSERT(128-64 == data_ptr_u8[1]);


        vxUnmapImagePatch(dst_image, map_id);
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

static vx_status readNV12Input(char* file_name, vx_image in_img)
{
    vx_status status;

    status = vxGetStatus((vx_reference)in_img);

    if(status == VX_SUCCESS)
    {
        FILE * fp = fopen(file_name,"rb");
        vx_size arr_len;
        vx_int32 i, j;

        if(fp == NULL)
        {
            printf("Unable to open file %s \n", file_name);
            return (VX_FAILURE);
        }

        {
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t image_addr;
            vx_map_id map_id;
            void * data_ptr;
            uint8_t *data_ptr_8;
            vx_uint32  img_width;
            vx_uint32  img_height;
            vx_uint32 img_format;
            vx_uint32  num_bytes = 0;

            vxQueryImage(in_img, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
            vxQueryImage(in_img, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));
            vxQueryImage(in_img, VX_IMAGE_FORMAT, &img_format, sizeof(vx_uint32));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = img_width;
            rect.end_y = img_height;
            status = vxMapImagePatch(in_img,
                                    &rect,
                                    0,
                                    &map_id,
                                    &image_addr,
                                    &data_ptr,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X);

            /* Copy Luma */
            data_ptr_8 = (uint8_t *)data_ptr;
            for (j = 0; j < img_height; j++)
            {
                num_bytes += fread(data_ptr_8, 1, img_width, fp);
                data_ptr_8 += image_addr.stride_y;
            }

            if(num_bytes != (img_width*img_height)) {
                printf("Luma bytes read = %d, expected = %d\n", num_bytes, img_width*img_height);
                return (VX_FAILURE);
            }

            vxUnmapImagePatch(in_img, map_id);

            if(img_format == VX_DF_IMAGE_NV12)
            {
                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = img_width;
                rect.end_y = img_height / 2;
                status = vxMapImagePatch(in_img,
                                        &rect,
                                        1,
                                        &map_id,
                                        &image_addr,
                                        &data_ptr,
                                        VX_WRITE_ONLY,
                                        VX_MEMORY_TYPE_HOST,
                                        VX_NOGAP_X);


                /* Copy CbCr */
                num_bytes = 0;
                data_ptr_8 = (uint8_t *)data_ptr;
                for (j = 0; j < img_height/2; j++)
                {
                    num_bytes += fread(data_ptr_8, 1, img_width, fp);
                    data_ptr_8 += image_addr.stride_y;
                }

                if(num_bytes != (img_width*img_height/2)) {
                    printf("CbCr bytes read = %d, expected = %d\n", num_bytes, img_width*img_height/2);
                    return (VX_FAILURE);
                }

                vxUnmapImagePatch(in_img, map_id);
            }
        }

        fclose(fp);
    }

    return(status);
}

/* Write NV12 output image */
static vx_int32 write_output_image_fp(FILE * fp, vx_image out_image)
{
    vx_uint32 width, height;
    vx_df_image df;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id1, map_id2;
    void *data_ptr1, *data_ptr2;
    uint8_t *temp_ptr = NULL;
    vx_uint32 num_bytes_per_pixel = 1;
    vx_uint32 num_luma_bytes_written_to_file = 0, num_chroma_bytes_written_to_file = 0, num_bytes_written_to_file = 0;
    vx_int32 i;

    vxQueryImage(out_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(out_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(out_image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

    printf("out width =  %d\n", width);
    printf("out height =  %d\n", height);
    printf("out format =  %d\n", df);

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;

    vxMapImagePatch(out_image,
        &rect,
        0,
        &map_id1,
        &image_addr,
        &data_ptr1,
        VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
        );

    if(!data_ptr1)
    {
        printf("data_ptr1 is NULL \n");
        fclose(fp);
        return -1;
    }

    temp_ptr = (uint8_t *)data_ptr1;
    for(i=0; i<height; i++)
    {
        num_luma_bytes_written_to_file += fwrite(temp_ptr, 1, width*num_bytes_per_pixel, fp);
        temp_ptr += image_addr.stride_y;
    }

    vxMapImagePatch(out_image,
        &rect,
        1,
        &map_id2,
        &image_addr,
        &data_ptr2,
        VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
        );

    if(!data_ptr2)
    {
        printf("data_ptr2 is NULL \n");
        fclose(fp);
        return -1;
    }

    temp_ptr = (uint8_t *)data_ptr2;
    for(i=0; i<height/2; i++)
    {
        num_chroma_bytes_written_to_file += fwrite(temp_ptr, 1, width*num_bytes_per_pixel, fp);
        temp_ptr += image_addr.stride_y;
    }

    num_bytes_written_to_file = num_luma_bytes_written_to_file + num_chroma_bytes_written_to_file;

    vxUnmapImagePatch(out_image, map_id1);
    vxUnmapImagePatch(out_image, map_id2);

    return num_bytes_written_to_file;
}

/* Open and write NV12 output image */
static vx_int32 write_output_image_nv12_8bit(char * file_name, vx_image out)
{
    FILE * fp;
    printf("Opening file %s \n", file_name);

    fp = fopen(file_name, "wb");
    if(!fp)
    {
        printf("Unable to open file %s\n", file_name);
        return -1;
    }
    vx_uint32 len1 = write_output_image_fp(fp, out);
    printf("Written image \n");
    fclose(fp);
    printf("%d bytes written to %s\n", len1, file_name);
    return len1 ;
}


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_Nv12, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i, j, crop_mode = 0;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char *output_file_name = "output/msc_out.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    int run_cnt;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&coeff_obj));

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    rect.start_x = 0;
    rect.start_y = 0;

    for(run_cnt=0; run_cnt<4; run_cnt++)
    {
        VX_CALL(vxProcessGraph(graph));

        //write_output_image_nv12_8bit(file, dst_image);

        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
        printf("luma  =0x%08x\n", checksum_actual);
        ASSERT((uint32_t)0x59ab963a == checksum_actual);

        rect.end_x = dst_width/2;
        rect.end_y = dst_height/2;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 1, rect);
        printf("chroma=0x%08x\n", checksum_actual);
        ASSERT((uint32_t)0x08661321 == checksum_actual);
    }

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}



#define SCALE_TEST_ONE_OUTPUT(interpolation, inputDataGenerator, inputDataFile, scale, crop_mode, checksum, nextmacro, ...) \
    CT_EXPAND(nextmacro(STR_##interpolation "/" inputDataFile "/" #scale "/crop=" #crop_mode, __VA_ARGS__, \
            interpolation, inputDataGenerator, inputDataFile, dst_size_generator_ ## scale, crop_mode, checksum))

#define ADD_VX_BORDERS_REQUIRE_REPLICATE_ONLY(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_REPLICATE", __VA_ARGS__, { VX_BORDER_REPLICATE, {{ 0 }} }))

#define ADD_DST_SIZE_NN(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/1_1", __VA_ARGS__, dst_size_generator_1_1)), \
    CT_EXPAND(nextmacro(testArgName "/2_1", __VA_ARGS__, dst_size_generator_2_1)), \
    CT_EXPAND(nextmacro(testArgName "/3_1", __VA_ARGS__, dst_size_generator_3_1)), \
    CT_EXPAND(nextmacro(testArgName "/4_1", __VA_ARGS__, dst_size_generator_4_1))

#define ADD_DST_SIZE_BILINEAR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/1:1", __VA_ARGS__, dst_size_generator_1_1)), \
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


#define PARAMETERS_ONE_OUTPUT \
    /* Crop off */ \
    /* NN downscale */ \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 1_1, 0, 0x41dd742d, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 2_1, 0, 0xaad0c491, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 3_1, 0, 0x08c84775, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 4_1, 0, 0x39393c76, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", SCALE_PYRAMID_ORB, 0, 0xf4ca5ddd, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    /* BILINEAR downscales */ \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 1_1, 0, 0x41dd742d, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 2_1, 0, 0x2891bf11, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 3_1, 0, 0xe5404e1a, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 4_1, 0, 0x21264e4f, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", SCALE_PYRAMID_ORB, 0, 0xb4697795, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    /* Crop on */ \
    /* NN downscale */ \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 1_1, 1, 0x8b8c0ce1, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 2_1, 1, 0x14dc69f0, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 3_1, 1, 0xe22db6f5, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 4_1, 1, 0x5d69ead7, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", SCALE_PYRAMID_ORB, 1, 0x87d625f5, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    /* BILINEAR downscales */ \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 1_1, 1, 0x8b8c0ce1, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 2_1, 1, 0xded3fe1d, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 3_1, 1, 0x8f75b64d, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 4_1, 1, 0x1f8aa69c, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", SCALE_PYRAMID_ORB, 1, 0x2d376dc1, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_OneOutput, Arg_OneOutput,
    PARAMETERS_ONE_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[1] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;

    CT_Image src = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

/* Note: for debug--load in a file by putting a breakpoint at unmap */
#if 0
        vx_imagepatch_addressing_t image_addr;
        vx_map_id map_id;
        vx_df_image df;
        void *data_ptr, *data_ptr2;

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = arg_->width;
        rect.end_y = arg_->height;

        vxMapImagePatch(src_image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );


        vxUnmapImagePatch(src_image, map_id);
#endif
        ASSERT_NO_FAILURE(arg_->dst_size_generator(src->width, src->height, &dst_width, &dst_height));

        if(arg_->crop_mode == 1)
        {
            dst_width /= 2;
            dst_height /= 2;
        }

        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        tivx_vpac_msc_coefficients_params_init(&coeffs, arg_->interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, sizeof(refs)/sizeof(refs[0])));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        if(arg_->crop_mode == 1)
        {
            /* Set Input Crop */
            ASSERT_VX_OBJECT(crop_obj = vxCreateUserDataObject(context,
                "tivx_vpac_msc_crop_params_t",
                sizeof(tivx_vpac_msc_crop_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            /* Center crop of input */
            crop.crop_start_x = src->width / 4;
            crop.crop_start_y = src->height / 4;
            crop.crop_width   = src->width / 2;
            crop.crop_height  = src->height / 2;

            VX_CALL(vxCopyUserDataObject(crop_obj, 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop, VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            refs[0] = (vx_reference)crop_obj;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS,
                tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
                refs, 1u));

            VX_CALL(vxReleaseUserDataObject(&crop_obj));
        }

        VX_CALL(vxProcessGraph(graph));

        //ASSERT_NO_FAILURE(src = ct_image_from_vx_image(src_image));
        //ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

#if 0
        /*rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        vxMapImagePatch(dst_image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr2,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        vxUnmapImagePatch(dst_image, map_id);*/
#endif

        //ASSERT_NO_FAILURE(scale_check(src, dst, arg_->interpolation, arg_->border, arg_->exact_result));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
        //printf("0x%08x\n", checksum_actual);
        //printf("end_x = %d\n", dst_width);
        //printf("end_y = %d\n", dst_height);

        ASSERT(arg_->checksum == checksum_actual);

        //save_image_from_msc(dst_image, "output/lena_msc");

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

#define SCALE_TEST_TWO_OUTPUT(interpolation, inputDataGenerator, inputDataFile, scale1, scale2, exact, nextmacro, ...) \
    CT_EXPAND(nextmacro(STR_##interpolation "/" inputDataFile "/" #scale1 "/" #scale2, __VA_ARGS__, \
            interpolation, inputDataGenerator, inputDataFile, dst_size_generator_ ## scale1, dst_size_generator_ ## scale2, exact))

#define PARAMETERS_TWO_OUTPUT \
    /* 1:1 scale */ \
    SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_1, 2_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /*SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 1_1, 2_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0),*/ \
    SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_pattern3x3, "pattern3x3", 3_1, 1_1, 0, ADD_SIZE_96x96, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /* SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 3_1, 0, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */\
    /* other NN downscales */ \
    SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 2_1, 1_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 4_1, 2_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /* SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", SCALE_PYRAMID_ORB, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* BILINEAR upscale with integer factor */ \
    /* SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 1_2, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 1_3, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* BILINEAR downscales */ \
    SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 2_1, 2_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /* SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 3_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0),*/ \
    /*SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 4_1, 2_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */\
    /* SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 5_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", SCALE_PYRAMID_ORB, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* AREA tests */ \
    SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_AREA,             scale_generate_gradient_16x16, "gradient16x16", 4_1, 2_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /*SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_AREA,             scale_read_image, "lena.bmp", 4_1, 1_1, 0, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0),*/ \


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_TwoOutput, Arg_TwoOutput,
    PARAMETERS_TWO_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image0 = 0, dst_image1 = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[1];
    uint32_t cnt;

    CT_Image src = NULL, dst = NULL, dst2 = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image0 = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator1(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image1 = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image0, dst_image1, NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        scale_set_coeff(&coeffs, arg_->interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxProcessGraph(graph));

        ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image0));
        ASSERT_NO_FAILURE(scale_check(src, dst, arg_->interpolation, arg_->border, arg_->exact_result));
        ASSERT_NO_FAILURE(dst2 = ct_image_from_vx_image(dst_image1));
        ASSERT_NO_FAILURE(scale_check(src, dst2, arg_->interpolation, arg_->border, arg_->exact_result));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image0));
        VX_CALL(vxReleaseImage(&dst_image1));
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        tivxHwaUnLoadKernels(context);
    }

    ASSERT(dst_image0 == 0);
    ASSERT(dst_image1 == 0);
    ASSERT(src_image == 0);
}

#define SCALE_TEST_THREE_OUTPUT(interpolation, inputDataGenerator, inputDataFile, scale1, scale2, scale3, exact, nextmacro, ...) \
    CT_EXPAND(nextmacro(STR_##interpolation "/" inputDataFile "/" #scale1 "/" #scale2 "/" #scale3, __VA_ARGS__, \
            interpolation, inputDataGenerator, inputDataFile, dst_size_generator_ ## scale1, dst_size_generator_ ## scale2, dst_size_generator_ ## scale3, exact))

#define PARAMETERS_THREE_OUTPUT \
    /* 1:1 scale */ \
    SCALE_TEST_THREE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_1, 2_1, 2_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    SCALE_TEST_THREE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 2_1, 4_1, 1_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /*SCALE_TEST_THREE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_generate_random, "random", 1_1, 2_1, 4_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0),*/ \
    SCALE_TEST_THREE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_generate_random, "random", 2_1, 2_1, 1_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /*SCALE_TEST_THREE_OUTPUT(VX_INTERPOLATION_AREA, scale_read_image, "lena.bmp", 4_1, 3_1, 2_1, 0, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0)*/

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_ThreeOutput, Arg_ThreeOutput,
    PARAMETERS_THREE_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image[3] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[3];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[3];
    vx_reference refs[1], output_refs[5];

    CT_Image src = NULL, dst = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);
        scale_set_output_params(&output_params[0], arg_->interpolation, src->width, src->height, dst_width, dst_height);

        ASSERT_NO_FAILURE(arg_->dst_size_generator1(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);
        scale_set_output_params(&output_params[1], arg_->interpolation, src->width, src->height, dst_width, dst_height);

        ASSERT_NO_FAILURE(arg_->dst_size_generator2(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[2]);
        scale_set_output_params(&output_params[2], arg_->interpolation, src->width, src->height, dst_width, dst_height);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image[0], dst_image[1], dst_image[2], NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        scale_set_coeff(&coeffs, arg_->interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        for (cnt = 0; cnt < 5; cnt++)
        {
            output_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 3; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 5u));

        VX_CALL(vxProcessGraph(graph));

        for (cnt = 0; cnt < 3; cnt ++)
        {
            ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image[cnt]));
            ASSERT_NO_FAILURE(scale_check(src, dst, arg_->interpolation, arg_->border, arg_->exact_result));
        }


        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 3; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        tivxHwaUnLoadKernels(context);
    }

    for (cnt = 0; cnt < 3; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(src_image == 0);
}

#define SCALE_TEST_FOUR_OUTPUT(interpolation, inputDataGenerator, inputDataFile, scale1, scale2, scale3, scale4, exact, nextmacro, ...) \
    CT_EXPAND(nextmacro(STR_##interpolation "/" inputDataFile "/" #scale1 "/" #scale2 "/" #scale3 "/" #scale4, __VA_ARGS__, \
            interpolation, inputDataGenerator, inputDataFile, dst_size_generator_ ## scale1, dst_size_generator_ ## scale2, dst_size_generator_ ## scale3, dst_size_generator_ ## scale4, exact))

#define PARAMETERS_FOUR_OUTPUT \
    /* 1:1 scale */ \
    SCALE_TEST_FOUR_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_1, 2_1, 2_1, 4_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    SCALE_TEST_FOUR_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_generate_random, "random", 2_1, 2_1, 2_1, 1_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /*SCALE_TEST_FOUR_OUTPUT(VX_INTERPOLATION_AREA, scale_read_image, "lena.bmp", 1_1, 2_1, 3_1, 4_1, 0, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0)*/

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_FourOutput, Arg_FourOutput,
    PARAMETERS_FOUR_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image[4] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[1];

    CT_Image src = NULL, dst = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator1(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator2(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator3(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[3] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image[0], dst_image[1], dst_image[2], dst_image[3], NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        scale_set_coeff(&coeffs, arg_->interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxProcessGraph(graph));

        for (cnt = 0; cnt < 4; cnt ++)
        {
            ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image[cnt]));
            ASSERT_NO_FAILURE(scale_check(src, dst, arg_->interpolation, arg_->border, arg_->exact_result));
        }


        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 4; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
        }
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        tivxHwaUnLoadKernels(context);
    }

    for (cnt = 0; cnt < 4; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(src_image == 0);
}

#define SCALE_TEST_FIVE_OUTPUT(interpolation, inputDataGenerator, inputDataFile, scale1, scale2, scale3, scale4, scale5, exact, nextmacro, ...) \
    CT_EXPAND(nextmacro(STR_##interpolation "/" inputDataFile "/" #scale1 "/" #scale2 "/" #scale3 "/" #scale4 "/" #scale5, __VA_ARGS__, \
            interpolation, inputDataGenerator, inputDataFile, dst_size_generator_ ## scale1, dst_size_generator_ ## scale2, dst_size_generator_ ## scale3, dst_size_generator_ ## scale4, dst_size_generator_ ## scale5, exact))

#define PARAMETERS_FIVE_OUTPUT \
    /* 1:1 scale */ \
    SCALE_TEST_FIVE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_1, 2_1, 2_1, 4_1, 2_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    SCALE_TEST_FIVE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_generate_random, "random", 2_1, 2_1, 2_1, 1_1, 1_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /*SCALE_TEST_FIVE_OUTPUT(VX_INTERPOLATION_AREA, scale_read_image, "lena.bmp", 1_1, 2_1, 3_1, 4_1, 2_1, 0, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0)*/

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_FiveOutput, Arg_FiveOutput,
    PARAMETERS_FIVE_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image[5] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[1];

    CT_Image src = NULL, dst = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator1(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator2(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator3(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[3] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator4(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[4] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        scale_set_coeff(&coeffs, arg_->interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxProcessGraph(graph));

        for (cnt = 0; cnt < 5; cnt ++)
        {
            ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image[cnt]));
            ASSERT_NO_FAILURE(scale_check(src, dst, arg_->interpolation, arg_->border, arg_->exact_result));
        }


        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 5; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
        }
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));


        tivxHwaUnLoadKernels(context);
    }

    for (cnt = 0; cnt < 5; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(src_image == 0);
}

TESTCASE_TESTS(tivxHwaVpacMscScaleMultiOutput,
    testNodeCreation,
    testGraphProcessing_FixedPattern,
    testGraphProcessing_FixedPattern_tiovx_1129,
    testGraphProcessing_Nv12,
    testGraphProcessing_OneOutput,
    testGraphProcessing_TwoOutput,
    testGraphProcessing_ThreeOutput,
    testGraphProcessing_FourOutput,
    testGraphProcessing_FiveOutput)

#endif /* BUILD_VPAC_MSC */