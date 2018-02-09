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

#include <math.h>
#include <string.h>
#include <VX/vx.h>
#include <VX/vxu.h>

#include "test_engine/test.h"
#include "shared_functions.h"

#define VX_GAUSSIAN_PYRAMID_TOLERANCE 1

TESTCASE(LaplacianPyramid, CT_VXContext, ct_setup_vx_context, 0)


TEST(LaplacianPyramid, testNodeCreation)
{
    vx_context context = context_->vx_context_;
    vx_image   input = 0;
    vx_pyramid laplacian = 0;
    vx_image   output = 0;
    vx_graph   graph = 0;
    vx_node    node = 0;
    const vx_size levels     = 4;
    const vx_float32 scale   = VX_SCALE_PYRAMID_HALF;
    const vx_uint32 width    = 640;
    const vx_uint32 height   = 480;
    const vx_df_image format = VX_DF_IMAGE_S16;
    vx_uint32 w = width;
    vx_uint32 h = height;
    vx_size L = levels - 1;

    ASSERT_VX_OBJECT(input = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(laplacian = vxCreatePyramid(context, L, scale, width, height, format), VX_TYPE_PYRAMID);

    while (L--)
    {
        w = (vx_uint32)(w * scale);
        h = (vx_uint32)(h * scale);
    }

    ASSERT_VX_OBJECT(output = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxLaplacianPyramidNode(graph, input, laplacian, output), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleasePyramid(&laplacian));
    VX_CALL(vxReleaseImage(&output));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(input == 0);
    ASSERT(laplacian == 0);
    ASSERT(output == 0);
    ASSERT(node == 0);
    ASSERT(graph == 0);
}

#define LEVELS_COUNT_MAX    7

static CT_Image own_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
        image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static CT_Image own_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

static vx_size own_pyramid_calc_max_levels_count(int width, int height, vx_float32 scale)
{
    vx_size level = 1;

    while ((16 <= width) && (16 <= height) && level < LEVELS_COUNT_MAX)
    {
        level++;
        width  = (int)ceil((vx_float64)width * scale);
        height = (int)ceil((vx_float64)height * scale);
    }

    return level;
}

static vx_status ownCopyImage(vx_image input, vx_image output)
{
    vx_status status = VX_SUCCESS; // assume success until an error occurs.
    vx_uint32 p = 0;
    vx_uint32 y = 0;
    vx_uint32 len = 0;
    vx_size planes = 0;
    void* src;
    void* dst;
    vx_imagepatch_addressing_t src_addr;
    vx_imagepatch_addressing_t dst_addr;
    vx_rectangle_t rect;
    vx_map_id map_id1;
    vx_map_id map_id2;

    status |= vxQueryImage(input, VX_IMAGE_PLANES, &planes, sizeof(planes));
    status |= vxGetValidRegionImage(input, &rect);

    for (p = 0; p < planes && status == VX_SUCCESS; p++)
    {
        status = VX_SUCCESS;
        src = NULL;
        dst = NULL;

        status |= vxMapImagePatch(input, &rect, p, &map_id1, &src_addr, &src, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);
        status |= vxMapImagePatch(output, &rect, p, &map_id2, &dst_addr, &dst, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        for (y = 0; y < src_addr.dim_y && status == VX_SUCCESS; y += src_addr.step_y)
        {
            /*
            * in the case where the secondary planes are subsampled, the
            * scale will skip over the lines that are repeated.
            */
            vx_uint8* srcp = vxFormatImagePatchAddress2d(src, 0, y, &src_addr);
            vx_uint8* dstp = vxFormatImagePatchAddress2d(dst, 0, y, &dst_addr);

            len = (src_addr.stride_x * src_addr.dim_x * src_addr.scale_x) / VX_SCALE_UNITY;

            memcpy(dstp, srcp, len);
        }

        if (status == VX_SUCCESS)
        {
            status |= vxUnmapImagePatch(input, map_id1);
            status |= vxUnmapImagePatch(output, map_id2);
        }
    }

    return status;
}

static vx_bool own_read_pixel_16s(void *base, vx_imagepatch_addressing_t *addr,
    vx_int32 x, vx_int32 y, const vx_border_t *borders, vx_int16 *pixel)
{
    vx_uint32 bx;
    vx_uint32 by;
    vx_int16* bpixel;

    vx_bool out_of_bounds = (vx_bool)(x < 0 || y < 0 || x >= (vx_int32)addr->dim_x || y >= (vx_int32)addr->dim_y);

    if (out_of_bounds)
    {
        if (borders->mode == VX_BORDER_UNDEFINED)
            return vx_false_e;
        if (borders->mode == VX_BORDER_CONSTANT)
        {
            *pixel = (vx_int16)borders->constant_value.S16;
            return vx_true_e;
        }
    }

    // bounded x/y
    bx = x < 0 ? 0 : x >= (vx_int32)addr->dim_x ? addr->dim_x - 1 : (vx_uint32)x;
    by = y < 0 ? 0 : y >= (vx_int32)addr->dim_y ? addr->dim_y - 1 : (vx_uint32)y;

    bpixel = (vx_int16*)vxFormatImagePatchAddress2d(base, bx, by, addr);
    *pixel = *bpixel;

    return vx_true_e;
}

static vx_status ownScaleImageNearestS16(vx_image src_image, vx_image dst_image, const vx_border_t *borders)
{
    vx_status status = VX_SUCCESS;
    vx_int32 x1, y1, x2, y2;
    void* src_base = NULL;
    void* dst_base = NULL;
    vx_rectangle_t src_rect;
    vx_rectangle_t dst_rect;
    vx_imagepatch_addressing_t src_addr;
    vx_imagepatch_addressing_t dst_addr;
    vx_uint32 w1 = 0, h1 = 0, w2 = 0, h2 = 0;
    vx_float32 wr, hr;
    vx_map_id map_id1;
    vx_map_id map_id2;

    vxQueryImage(src_image, VX_IMAGE_WIDTH, &w1, sizeof(w1));
    vxQueryImage(src_image, VX_IMAGE_HEIGHT, &h1, sizeof(h1));

    vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w2, sizeof(w2));
    vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h2, sizeof(h2));

    src_rect.start_x = src_rect.start_y = 0;
    src_rect.end_x = w1;
    src_rect.end_y = h1;

    dst_rect.start_x = dst_rect.start_y = 0;
    dst_rect.end_x = w2;
    dst_rect.end_y = h2;

    status = VX_SUCCESS;
    status |= vxMapImagePatch(src_image, &src_rect, 0, &map_id1, &src_addr, &src_base, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);
    status |= vxMapImagePatch(dst_image, &dst_rect, 0, &map_id2, &dst_addr, &dst_base, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

    wr = (vx_float32)w1/(vx_float32)w2;
    hr = (vx_float32)h1/(vx_float32)h2;

    for (y2 = 0; y2 < (vx_int32)dst_addr.dim_y; y2 += dst_addr.step_y)
    {
        for (x2 = 0; x2 < (vx_int32)dst_addr.dim_x; x2 += dst_addr.step_x)
        {
            vx_int16 v = 0;
            vx_int16* dst = vxFormatImagePatchAddress2d(dst_base, x2, y2, &dst_addr);
            vx_float32 x_src = ((vx_float32)x2 + 0.5f)*wr - 0.5f;
            vx_float32 y_src = ((vx_float32)y2 + 0.5f)*hr - 0.5f;
            vx_float32 x_min = floorf(x_src);
            vx_float32 y_min = floorf(y_src);
            x1 = (vx_int32)x_min;
            y1 = (vx_int32)y_min;

            if (x_src - x_min >= 0.5f)
                x1++;
            if (y_src - y_min >= 0.5f)
                y1++;

            if (dst && vx_true_e == own_read_pixel_16s(src_base, &src_addr, x1, y1, borders, &v))
                *dst = v;
        }
    }

    status |= vxUnmapImagePatch(src_image, map_id1);
    status |= vxUnmapImagePatch(dst_image, map_id2);

    return VX_SUCCESS;
}

static const vx_uint32 gaussian5x5scale = 256;
static const vx_int16 gaussian5x5[5][5] =
{
    { 1, 4, 6, 4, 1 },
    { 4, 16, 24, 16, 4 },
    { 6, 24, 36, 24, 6 },
    { 4, 16, 24, 16, 4 },
    { 1, 4, 6, 4, 1 }
};

static void own_laplacian_pyramid_reference(vx_context context, vx_border_t border, vx_image input, vx_pyramid laplacian, vx_image output)
{
    vx_uint32 i;
    vx_size levels = 0;
    vx_uint32 width = 0;
    vx_uint32 height = 0;
    vx_df_image format = 0;
    vx_pyramid gaussian = 0;
    vx_int32 shift_val = 0;
    vx_scalar shift = 0;
    vx_convolution conv = 0;

    VX_CALL(vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border)));

    VX_CALL(vxQueryPyramid(laplacian, VX_PYRAMID_LEVELS, &levels, sizeof(levels)));

    VX_CALL(vxQueryImage(input, VX_IMAGE_WIDTH, &width, sizeof(width)));
    VX_CALL(vxQueryImage(input, VX_IMAGE_HEIGHT, &height, sizeof(height)));
    VX_CALL(vxQueryImage(input, VX_IMAGE_FORMAT, &format, sizeof(format)));

    ASSERT_VX_OBJECT(gaussian = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);

    VX_CALL(vxuGaussianPyramid(context, input, gaussian));

    ASSERT_VX_OBJECT(conv = vxCreateConvolution(context, 5, 5), VX_TYPE_CONVOLUTION);
    ASSERT_VX_OBJECT(shift = vxCreateScalar(context, VX_TYPE_INT32, &shift_val), VX_TYPE_SCALAR);

    VX_CALL(vxCopyConvolutionCoefficients(conv, (vx_int16*)gaussian5x5, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxSetConvolutionAttribute(conv, VX_CONVOLUTION_SCALE, (void*)&gaussian5x5scale, sizeof(gaussian5x5scale)));

    for (i = 0; i < levels; i++)
    {
        vx_uint32 w = 0;
        vx_uint32 h = 0;
        vx_image in1 = 0;
        vx_image in2 = 0;
        vx_image out = 0;

        ASSERT_VX_OBJECT(in1 = vxGetPyramidLevel(gaussian, i), VX_TYPE_IMAGE);

        VX_CALL(vxQueryImage(in1, VX_IMAGE_WIDTH, &w, sizeof(vx_uint32)));
        VX_CALL(vxQueryImage(in1, VX_IMAGE_HEIGHT, &h, sizeof(vx_uint32)));

        ASSERT_VX_OBJECT(in2 = vxCreateImage(context, w, h, format), VX_TYPE_IMAGE);
        VX_CALL(vxuConvolve(context, in1, conv, in2));

        /* laplacian is S16 format */
        ASSERT_VX_OBJECT(out = vxGetPyramidLevel(laplacian, i), VX_TYPE_IMAGE);
        VX_CALL(vxuSubtract(context, in1, in2, VX_CONVERT_POLICY_WRAP, out));

        if (i == levels - 1)
        {
            //VX_CALL(vxuConvertDepth(context, in2, output, VX_CONVERT_POLICY_WRAP, shift_val));
        }

        VX_CALL(vxReleaseImage(&in1));
        VX_CALL(vxReleaseImage(&in2));
        VX_CALL(vxReleaseImage(&out));
    }

    VX_CALL(vxReleasePyramid(&gaussian));
    VX_CALL(vxReleaseConvolution(&conv));
    VX_CALL(vxReleaseScalar(&shift));

    ASSERT(conv == 0);
    ASSERT(gaussian == 0);

    return;
}

static void own_laplacian_pyramid_openvx(vx_context context, vx_border_t border, vx_image input, vx_pyramid laplacian, vx_image output)
{
    vx_graph graph = 0;
    vx_node node = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxLaplacianPyramidNode(graph, input, laplacian, output), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    return;
}

typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char* fileName;
    vx_border_t border;
    int width;
    int height;
} Arg;


#define ADD_SIZE_OWN_SET(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=128x128", __VA_ARGS__, 128, 128)), \
    CT_EXPAND(nextmacro(testArgName "/sz=256x256", __VA_ARGS__, 256, 256)), \
    CT_EXPAND(nextmacro(testArgName "/sz=640x480", __VA_ARGS__, 640, 480))


#define LAPLACIAN_PYRAMID_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_OWN_SET, ARG, own_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("lena", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, own_read_image, "lena.bmp")

TEST_WITH_ARG(LaplacianPyramid, testGraphProcessing, Arg, LAPLACIAN_PYRAMID_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_size levels = 0;
    vx_uint32 i;
    vx_image src = 0;
    vx_image ref_dst = 0;
    vx_image tst_dst = 0;
    vx_pyramid ref_pyr = 0;
    vx_pyramid tst_pyr = 0;
    int undefined_border = 2; // 5x5 kernel border

    CT_Image input = NULL;

    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(input = arg_->generator( arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    levels = own_pyramid_calc_max_levels_count(input->width, input->height, VX_SCALE_PYRAMID_HALF);

    {
        vx_uint32 next_lev_width  = input->width;
        vx_uint32 next_lev_height = input->height;

        ASSERT_VX_OBJECT(ref_pyr = vxCreatePyramid(context, levels-1, VX_SCALE_PYRAMID_HALF, input->width, input->height, VX_DF_IMAGE_S16), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(tst_pyr = vxCreatePyramid(context, levels-1, VX_SCALE_PYRAMID_HALF, input->width, input->height, VX_DF_IMAGE_S16), VX_TYPE_PYRAMID);

        for (i = 1; i < levels; i++)
        {
            next_lev_width  = (vx_uint32)ceilf(next_lev_width * VX_SCALE_PYRAMID_HALF);
            next_lev_height = (vx_uint32)ceilf(next_lev_height * VX_SCALE_PYRAMID_HALF);
        }

        ASSERT_VX_OBJECT(ref_dst = vxCreateImage(context, next_lev_width, next_lev_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(tst_dst = vxCreateImage(context, next_lev_width, next_lev_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    own_laplacian_pyramid_reference(context, border, src, ref_pyr, ref_dst);
    own_laplacian_pyramid_openvx(context, border, src, tst_pyr, tst_dst);

    {
        CT_Image ct_ref_dst = 0;
        CT_Image ct_tst_dst = 0;

        ASSERT_NO_FAILURE(ct_ref_dst = ct_image_from_vx_image(ref_dst));
        ASSERT_NO_FAILURE(ct_tst_dst = ct_image_from_vx_image(tst_dst));
        ASSERT_NO_FAILURE(ct_adjust_roi(ct_ref_dst, 2 * undefined_border, 2 * undefined_border, 2 * undefined_border, 2 * undefined_border));
        ASSERT_NO_FAILURE(ct_adjust_roi(ct_tst_dst, 2 * undefined_border, 2 * undefined_border, 2 * undefined_border, 2 * undefined_border));
        //EXPECT_CTIMAGE_NEAR(ct_ref_dst, ct_tst_dst, 1);

        for (i = 0; i < levels-1; i++)
        {
            //char name[32];
            CT_Image ct_ref_lev = 0;
            CT_Image ct_tst_lev = 0;
            vx_image ref_lev = 0;
            vx_image tst_lev = 0;

            ASSERT_VX_OBJECT(ref_lev = vxGetPyramidLevel(ref_pyr, i), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(tst_lev = vxGetPyramidLevel(tst_pyr, i), VX_TYPE_IMAGE);

            ASSERT_NO_FAILURE(ct_ref_lev = ct_image_from_vx_image(ref_lev));
            ASSERT_NO_FAILURE(ct_tst_lev = ct_image_from_vx_image(tst_lev));
            ASSERT_NO_FAILURE(ct_adjust_roi(ct_ref_lev, 2 * undefined_border, 2 * undefined_border, 2 * undefined_border, 2 * undefined_border));
            ASSERT_NO_FAILURE(ct_adjust_roi(ct_tst_lev, 2 * undefined_border, 2 * undefined_border, 2 * undefined_border, 2 * undefined_border));
            //ct_write_image("laplac_ref.bmp", ct_ref_lev);
            //ct_write_image("laplac_tst.bmp", ct_tst_lev);
            //EXPECT_CTIMAGE_NEAR(ct_ref_lev, ct_tst_lev, 1);

            VX_CALL(vxReleaseImage(&ref_lev));
            VX_CALL(vxReleaseImage(&tst_lev));
        }
    }

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleasePyramid(&ref_pyr));
    VX_CALL(vxReleasePyramid(&tst_pyr));
    VX_CALL(vxReleaseImage(&ref_dst));
    VX_CALL(vxReleaseImage(&tst_dst));

    ASSERT(src == 0);
    ASSERT(ref_pyr == 0);
    ASSERT(tst_pyr == 0);
    ASSERT(ref_dst == 0);
    ASSERT(tst_dst == 0);
}

TEST_WITH_ARG(LaplacianPyramid, testImmediateProcessing, Arg, LAPLACIAN_PYRAMID_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_size levels = 0;
    vx_uint32 i;
    vx_image src = 0;
    vx_image ref_dst = 0;
    vx_image tst_dst = 0;
    vx_pyramid ref_pyr = 0;
    vx_pyramid tst_pyr = 0;
    int undefined_border = 2; // 5x5 kernel border

    CT_Image input = NULL;

    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    levels = own_pyramid_calc_max_levels_count(input->width, input->height, VX_SCALE_PYRAMID_HALF);

    {
        vx_uint32 next_lev_width  = input->width;
        vx_uint32 next_lev_height = input->height;

        ASSERT_VX_OBJECT(ref_pyr = vxCreatePyramid(context, levels-1, VX_SCALE_PYRAMID_HALF, input->width, input->height, VX_DF_IMAGE_S16), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(tst_pyr = vxCreatePyramid(context, levels-1, VX_SCALE_PYRAMID_HALF, input->width, input->height, VX_DF_IMAGE_S16), VX_TYPE_PYRAMID);

        for (i = 1; i < levels; i++)
        {
            next_lev_width  = (vx_uint32)ceilf(next_lev_width * VX_SCALE_PYRAMID_HALF);
            next_lev_height = (vx_uint32)ceilf(next_lev_height * VX_SCALE_PYRAMID_HALF);
        }

        ASSERT_VX_OBJECT(ref_dst = vxCreateImage(context, next_lev_width, next_lev_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(tst_dst = vxCreateImage(context, next_lev_width, next_lev_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    own_laplacian_pyramid_reference(context, border, src, ref_pyr, ref_dst);
    VX_CALL(vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border)));
    VX_CALL(vxuLaplacianPyramid(context, src, tst_pyr, tst_dst));

    {
        CT_Image ct_ref_dst = 0;
        CT_Image ct_tst_dst = 0;

        ASSERT_NO_FAILURE(ct_ref_dst = ct_image_from_vx_image(ref_dst));
        ASSERT_NO_FAILURE(ct_tst_dst = ct_image_from_vx_image(tst_dst));
        ASSERT_NO_FAILURE(ct_adjust_roi(ct_ref_dst, 2*undefined_border, 2*undefined_border, 2*undefined_border, 2*undefined_border));
        ASSERT_NO_FAILURE(ct_adjust_roi(ct_tst_dst, 2*undefined_border, 2*undefined_border, 2*undefined_border, 2*undefined_border));
        //EXPECT_CTIMAGE_NEAR(ct_ref_dst, ct_tst_dst, 1);

        for (i = 0; i < levels-1; i++)
        {
            CT_Image ct_ref_lev = 0;
            CT_Image ct_tst_lev = 0;
            vx_image ref_lev = 0;
            vx_image tst_lev = 0;

            ASSERT_VX_OBJECT(ref_lev = vxGetPyramidLevel(ref_pyr, i), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(tst_lev = vxGetPyramidLevel(tst_pyr, i), VX_TYPE_IMAGE);

            ASSERT_NO_FAILURE(ct_ref_lev = ct_image_from_vx_image(ref_lev));
            ASSERT_NO_FAILURE(ct_tst_lev = ct_image_from_vx_image(tst_lev));
            ASSERT_NO_FAILURE(ct_adjust_roi(ct_ref_lev, 2*undefined_border, 2*undefined_border, 2*undefined_border, 2*undefined_border));
            ASSERT_NO_FAILURE(ct_adjust_roi(ct_tst_lev, 2*undefined_border, 2*undefined_border, 2*undefined_border, 2*undefined_border));
            //EXPECT_CTIMAGE_NEAR(ct_ref_lev, ct_tst_lev, 1);

            VX_CALL(vxReleaseImage(&ref_lev));
            VX_CALL(vxReleaseImage(&tst_lev));
        }
    }

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleasePyramid(&ref_pyr));
    VX_CALL(vxReleasePyramid(&tst_pyr));
    VX_CALL(vxReleaseImage(&ref_dst));
    VX_CALL(vxReleaseImage(&tst_dst));

    ASSERT(src == 0);
    ASSERT(ref_pyr == 0);
    ASSERT(tst_pyr == 0);
    ASSERT(ref_dst == 0);
    ASSERT(tst_dst == 0);
}

TESTCASE_TESTS(LaplacianPyramid,
        testNodeCreation,
        testGraphProcessing,
        testImmediateProcessing
)

/* reconstruct image from laplacian pyramid */

#define VX_SCALE_PYRAMID_DOUBLE (2.0f)

TESTCASE(LaplacianReconstruct, CT_VXContext, ct_setup_vx_context, 0)

TEST(LaplacianReconstruct, testNodeCreation)
{
    vx_context context = context_->vx_context_;
    vx_pyramid laplacian = 0;
    vx_image   input = 0;
    vx_image   output = 0;
    vx_graph   graph = 0;
    vx_node    node = 0;
    const vx_size levels = 4;
    const vx_float32 scale = VX_SCALE_PYRAMID_HALF;
    const vx_uint32 width = 640;
    const vx_uint32 height = 480;
    const vx_df_image format = VX_DF_IMAGE_U8;
    vx_size num_levels = levels - 1;
    vx_uint32 w = width;
    vx_uint32 h = height;

    while (num_levels--)
    {
        w = (vx_uint32)ceilf(w * scale);
        h = (vx_uint32)ceilf(h * scale);
    }

    ASSERT_VX_OBJECT(input = vxCreateImage(context, w, h, format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output = vxCreateImage(context, width, height, format), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(laplacian = vxCreatePyramid(context, levels-1, scale, width, height, VX_DF_IMAGE_S16), VX_TYPE_PYRAMID);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxLaplacianReconstructNode(graph, laplacian, input, output), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleasePyramid(&laplacian));
    VX_CALL(vxReleaseImage(&output));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(laplacian == 0);
    ASSERT(input == 0);
    ASSERT(output == 0);
    ASSERT(node == 0);
    ASSERT(graph == 0);
}

static void own_laplacian_reconstruct_reference(vx_context context, vx_border_t border, vx_pyramid laplacian, vx_image input, vx_image output)
{
    vx_size i;
    vx_size levels = 0;
    vx_uint32 width = 0;
    vx_uint32 height = 0;
    vx_uint32 prev_lev_width = 0;
    vx_uint32 prev_lev_height = 0;
    vx_int32 shift_val = 0;
    vx_scalar shift = 0;
    vx_df_image format = 0;
    vx_image pyr_level = 0;
    vx_image image_prev = 0;
    vx_image image_curr = 0;

    VX_CALL(vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border)));

    VX_CALL(vxQueryPyramid(laplacian, VX_PYRAMID_LEVELS, &levels, sizeof(vx_size)));
    VX_CALL(vxQueryPyramid(laplacian, VX_PYRAMID_FORMAT, &format, sizeof(vx_df_image)));

    VX_CALL(vxQueryImage(input, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32)));
    VX_CALL(vxQueryImage(input, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32)));

    ASSERT_VX_OBJECT(shift = vxCreateScalar(context, VX_TYPE_INT32, &shift_val), VX_TYPE_SCALAR);

    /* intermediate data type is S16 */
    ASSERT_VX_OBJECT(image_prev = vxCreateImage(context, width, height, format), VX_TYPE_IMAGE);
    VX_CALL(ownCopyImage(input, image_prev));

    /* reconstruction starts from the lowest resolution */
    prev_lev_width  = width;
    prev_lev_height = height;

    for (i = 0; i < levels; i++)
    {
        ASSERT_VX_OBJECT(image_curr = vxCreateImage(context, prev_lev_width, prev_lev_height, format), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(pyr_level = vxGetPyramidLevel(laplacian, (vx_uint32)((levels - 1) - i)), VX_TYPE_IMAGE);

        VX_CALL(vxuAdd(context, pyr_level, image_prev, VX_CONVERT_POLICY_SATURATE, image_curr));

        VX_CALL(vxReleaseImage(&pyr_level));
        VX_CALL(vxReleaseImage(&image_prev));

        if ((levels - 1) - i == 0)
        {
            //VX_CALL(vxuConvertDepth(context, image_curr, output, VX_CONVERT_POLICY_SATURATE, shift_val));
        }
        else
        {
            /* compute dimensions for the prev level */
            prev_lev_width  = (vx_uint32)ceilf(prev_lev_width * VX_SCALE_PYRAMID_DOUBLE);
            prev_lev_height = (vx_uint32)ceilf(prev_lev_height * VX_SCALE_PYRAMID_DOUBLE);

            ASSERT_VX_OBJECT(image_prev = vxCreateImage(context, prev_lev_width, prev_lev_height, format), VX_TYPE_IMAGE);

            VX_CALL(ownScaleImageNearestS16(image_curr, image_prev, &border));
        }

        VX_CALL(vxReleaseImage(&image_curr));
    }

    VX_CALL(vxReleaseScalar(&shift));

    return;
}

static void own_laplacian_reconstruct_openvx(vx_context context, vx_border_t border, vx_pyramid laplacian, vx_image input, vx_image output)
{
    vx_graph graph = 0;
    vx_node node = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxLaplacianReconstructNode(graph, laplacian, input, output), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    return;
}

#define LAPLACIAN_RECONSTRUCT_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_OWN_SET, ARG, own_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("lena", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, own_read_image, "lena.bmp")

TEST_WITH_ARG(LaplacianReconstruct, testGraphProcessing, Arg, LAPLACIAN_RECONSTRUCT_PARAMETERS)
{
    vx_uint32 i;
    vx_context context = context_->vx_context_;
    vx_size levels = 0;
    vx_image src = 0;
    vx_image ref_lowest_res = 0;
    vx_image ref_dst = 0;
    vx_image tst_dst = 0;
    vx_pyramid ref_pyr = 0;

    CT_Image input = NULL;

    vx_border_t border = arg_->border;
    vx_border_t build_border = {VX_BORDER_REPLICATE};

    ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    levels = own_pyramid_calc_max_levels_count(input->width, input->height, VX_SCALE_PYRAMID_HALF);

    {
        vx_uint32 lowest_res_width  = input->width;
        vx_uint32 lowest_res_height = input->height;

        for (i = 1; i < levels; i++)
        {
            lowest_res_width  = (vx_uint32)ceilf(lowest_res_width * VX_SCALE_PYRAMID_HALF);
            lowest_res_height = (vx_uint32)ceilf(lowest_res_height * VX_SCALE_PYRAMID_HALF);
        }

        ASSERT_VX_OBJECT(ref_pyr = vxCreatePyramid(context, levels-1, VX_SCALE_PYRAMID_HALF, input->width, input->height, VX_DF_IMAGE_S16), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(ref_lowest_res = vxCreateImage(context, lowest_res_width, lowest_res_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(ref_dst = vxCreateImage(context, input->width, input->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(tst_dst = vxCreateImage(context, input->width, input->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    //own_laplacian_pyramid_reference(context, build_border, src, ref_pyr, ref_lowest_res);
    //own_laplacian_reconstruct_reference(context, border, ref_pyr, ref_lowest_res, ref_dst);
    own_laplacian_pyramid_openvx(context, build_border, src, ref_pyr, ref_lowest_res);
    own_laplacian_reconstruct_openvx(context, border, ref_pyr, ref_lowest_res, tst_dst);

    {
        CT_Image ct_ref_dst = 0;
        CT_Image ct_tst_dst = 0;

        //ASSERT_NO_FAILURE(ct_ref_dst = ct_image_from_vx_image(ref_dst));
        ASSERT_NO_FAILURE(ct_tst_dst = ct_image_from_vx_image(tst_dst));
        //ct_write_image("laplac_tst.bmp", ct_tst_dst);
        EXPECT_CTIMAGE_NEAR(input, ct_tst_dst, 1);
    }

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleasePyramid(&ref_pyr));
    VX_CALL(vxReleaseImage(&ref_lowest_res));
    VX_CALL(vxReleaseImage(&ref_dst));
    VX_CALL(vxReleaseImage(&tst_dst));

    ASSERT(src == 0);
    ASSERT(ref_pyr == 0);
    ASSERT(ref_lowest_res == 0);
    ASSERT(ref_dst == 0);
    ASSERT(tst_dst == 0);
}

TEST_WITH_ARG(LaplacianReconstruct, testImmediateProcessing, Arg, LAPLACIAN_RECONSTRUCT_PARAMETERS)
{
    vx_uint32 i;
    vx_context context = context_->vx_context_;
    vx_size levels = 0;
    vx_image src = 0;
    vx_image ref_lovest_res = 0;
    vx_image ref_dst = 0;
    vx_image tst_dst = 0;
    vx_pyramid ref_pyr = 0;

    CT_Image input = NULL;

    vx_border_t border = arg_->border;
    vx_border_t build_border = {VX_BORDER_REPLICATE};

    ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    levels = own_pyramid_calc_max_levels_count(input->width, input->height, VX_SCALE_PYRAMID_HALF);

    {
        vx_uint32 lowest_res_width  = input->width;
        vx_uint32 lowest_res_height = input->height;

        for (i = 1; i < levels; i++)
        {
            lowest_res_width  = (vx_uint32)ceilf(lowest_res_width * VX_SCALE_PYRAMID_HALF);
            lowest_res_height = (vx_uint32)ceilf(lowest_res_height * VX_SCALE_PYRAMID_HALF);
        }

        ASSERT_VX_OBJECT(ref_pyr = vxCreatePyramid(context, levels-1, VX_SCALE_PYRAMID_HALF, input->width, input->height, VX_DF_IMAGE_S16), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(ref_lovest_res = vxCreateImage(context, lowest_res_width, lowest_res_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(ref_dst = vxCreateImage(context, input->width, input->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(tst_dst = vxCreateImage(context, input->width, input->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    //own_laplacian_pyramid_reference(context, build_border, src, ref_pyr, ref_lovest_res);
    //own_laplacian_reconstruct_reference(context, border, ref_pyr, ref_lovest_res, ref_dst);
    own_laplacian_pyramid_openvx(context, build_border, src, ref_pyr, ref_lovest_res);

    VX_CALL(vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border)));
    VX_CALL(vxuLaplacianReconstruct(context, ref_pyr, ref_lovest_res, tst_dst));

    {
        CT_Image ct_ref_dst = 0;
        CT_Image ct_tst_dst = 0;

        //ASSERT_NO_FAILURE(ct_ref_dst = ct_image_from_vx_image(ref_dst));
        ASSERT_NO_FAILURE(ct_tst_dst = ct_image_from_vx_image(tst_dst));
        //ct_write_image("laplac_tst.bmp", ct_tst_dst);
        EXPECT_CTIMAGE_NEAR(input, ct_tst_dst, 1);
    }

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleasePyramid(&ref_pyr));
    VX_CALL(vxReleaseImage(&ref_lovest_res));
    VX_CALL(vxReleaseImage(&ref_dst));
    VX_CALL(vxReleaseImage(&tst_dst));

    ASSERT(src == 0);
    ASSERT(ref_pyr == 0);
    ASSERT(ref_lovest_res == 0);
    ASSERT(ref_dst == 0);
    ASSERT(tst_dst == 0);
}

TESTCASE_TESTS(LaplacianReconstruct,
    testNodeCreation,
    testGraphProcessing,
    testImmediateProcessing
    )
