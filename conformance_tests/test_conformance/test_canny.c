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

#include "test_engine/test.h"

#include <stdint.h>
#include <VX/vx.h>
#include <VX/vxu.h>

#define USE_OPENCV_GENERATED_REFERENCE
#define CANNY_ACCEPTANCE_THRESHOLD 0.95
//#define EXECUTE_ASYNC


#define CREF_EDGE 2
#define CREF_LINK 1
#define CREF_NONE 0

static int32_t offsets[][2] =
{
    { -1, -1}, {  0, -1}, {  1, -1},
    { -1,  0},            {  1,  0},
    { -1,  1}, {  0,  1}, {  1,  1}
};

#ifndef USE_OPENCV_GENERATED_REFERENCE
static uint64_t magnitude(CT_Image img, uint32_t x, uint32_t y, int32_t k, vx_enum type, int32_t* dx_out, int32_t* dy_out)
{
    static int32_t dim1[][7] = { { 1, 2, 1}, { 1,  4, 6, 4, 1}, { 1,  6, 15, 20, 15, 6, 1}};
    static int32_t dim2[][7] = { {-1, 0, 1}, {-1, -2, 0, 2, 1}, {-1, -4, -5,  0,  5, 4, 1}};
    int32_t dx = 0, dy = 0;
    int32_t i,j;

    int32_t* w1 = dim1[k/2-1];
    int32_t* w2 = dim2[k/2-1];

    x -= k/2;
    y -= k/2;

    for (i = 0; i < k; ++i)
    {
        int32_t xx = 0, yy = 0;
        for (j = 0; j < k; ++j)
        {
            vx_int32 v = img->data.y[(y + i) * img->stride + (x + j)];
            xx +=  v * w2[j];
            yy +=  v * w1[j];
        }

        dx += xx * w1[i];
        dy += yy * w2[i];
    }

    if (dx_out) *dx_out = dx;
    if (dy_out) *dy_out = dy;

    if (type == VX_NORM_L2)
        return dx * (int64_t)dx + dy * (int64_t)dy;
    else
        return (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
}

static void follow_edge(CT_Image img, uint32_t x, uint32_t y)
{
    uint32_t i;
    img->data.y[y * img->stride + x] = 255;
    for (i = 0; i < sizeof(offsets)/sizeof(offsets[0]); ++i)
        if (img->data.y[(y + offsets[i][0]) * img->stride + x + offsets[i][1]] == CREF_LINK)
            follow_edge(img, x + offsets[i][1], y + offsets[i][0]);
}

static void reference_canny(CT_Image src, CT_Image dst, int32_t low_thresh, int32_t high_thresh, uint32_t gsz, vx_enum norm)
{
    uint64_t lo = norm == VX_NORM_L1 ? low_thresh  : low_thresh*low_thresh;
    uint64_t hi = norm == VX_NORM_L1 ? high_thresh : high_thresh*high_thresh;
    uint32_t i, j;
    uint32_t bsz = gsz/2 + 1;

    ASSERT(src && dst);
    ASSERT(src->width == dst->width);
    ASSERT(src->height == dst->height);
    ASSERT(src->format == dst->format && src->format == VX_DF_IMAGE_U8);

    ASSERT(low_thresh <= high_thresh);
    ASSERT(low_thresh >= 0);
    ASSERT(gsz == 3 || gsz == 5 || gsz == 7);
    ASSERT(norm == VX_NORM_L2 || norm == VX_NORM_L1);
    ASSERT(src->width >= gsz && src->height >= gsz);

    // zero border pixels
    for (j = 0; j < bsz; ++j)
        for (i = 0; i < dst->width; ++i)
            dst->data.y[j * dst->stride + i] = dst->data.y[(dst->height - 1 - j) * dst->stride + i] = 255;
    for (j = bsz; j < dst->height - bsz; ++j)
        for (i = 0; i < bsz; ++i)
            dst->data.y[j * dst->stride + i] = dst->data.y[j * dst->stride + dst->width - 1 - i] = 255;

    // threshold + nms
    for (j = bsz; j < dst->height - bsz; ++j)
    {
        for (i = bsz; i < dst->width - bsz; ++i)
        {
            int32_t dx, dy, e = CREF_NONE;
            uint64_t m1, m2;

            uint64_t m = magnitude(src, i, j, gsz, norm, &dx, &dy);

            if (m > lo)
            {
                uint64_t l1 = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);

                if (l1 * l1 < (uint64_t)(2 * dx * (int64_t)dx)) // |y| < |x| * tan(pi/8)
                {
                    m1 = magnitude(src, i-1, j, gsz, norm, 0, 0);
                    m2 = magnitude(src, i+1, j, gsz, norm, 0, 0);
                }
                else if (l1 * l1 < (uint64_t)(2 * dy * (int64_t)dy)) // |x| < |y| * tan(pi/8)
                {
                    m1 = magnitude(src, i, j-1, gsz, norm, 0, 0);
                    m2 = magnitude(src, i, j+1, gsz, norm, 0, 0);
                }
                else
                {
                    int32_t s = (dx ^ dy) < 0 ? -1 : 1;
                    m1 = magnitude(src, i-s, j-1, gsz, norm, 0, 0);
                    m2 = magnitude(src, i+s, j+1, gsz, norm, 0, 0) + 1; // (+1) is OpenCV's gotcha
                }

                if (m > m1 && m >= m2)
                    e = (m > hi ? CREF_EDGE : CREF_LINK);
            }

            dst->data.y[j * src->stride + i] = e;
        }
    }

    // trace edges
    for (j = bsz; j < dst->height - bsz; ++j)
        for (i = bsz; i < dst->width - bsz; ++i)
            if(dst->data.y[j * dst->stride + i] == CREF_EDGE)
                follow_edge(dst, i, j);

    // clear non-edges
    for (j = bsz; j < dst->height - bsz; ++j)
        for (i = bsz; i < dst->width - bsz; ++i)
            if(dst->data.y[j * dst->stride + i] < 255)
                dst->data.y[j * dst->stride + i] = 0;
}
#endif

// computes count(disttransform(src) >= 2, where dst != 0)
static uint32_t disttransform2_metric(CT_Image src, CT_Image dst, CT_Image dist, uint32_t* total_edge_pixels)
{
    uint32_t i, j, k, count, total;

    ASSERT_(return 0, src && dst && dist && total_edge_pixels);
    ASSERT_(return 0, src->width == dst->width && src->width == dist->width);
    ASSERT_(return 0, src->height == dst->height && src->height == dist->height);
    ASSERT_(return 0, src->format == dst->format && src->format == dist->format && src->format == VX_DF_IMAGE_U8);

    // fill borders with 1 (or 0 for edges)
    for (i = 0; i < dst->width; ++i)
    {
        dist->data.y[i] = src->data.y[i] == 0 ? 1 : 0;
        dist->data.y[(dist->height - 1) * dist->stride + i] = src->data.y[(dist->height - 1) * src->stride + i] == 0 ? 1 : 0;
    }
    for (j = 1; j < dst->height - 1; ++j)
    {
        dist->data.y[j * dist->stride] = src->data.y[j * src->stride] == 0 ? 1 : 0;
        dist->data.y[j * dist->stride + dist->width - 1] = src->data.y[j * src->stride + dist->width - 1] == 0 ? 1 : 0;
    }

    // minimalistic variant of disttransform:
    // 0   ==>      disttransform(src) == 0
    // 1   ==> 1 <= disttransform(src) < 2
    // 255 ==>      disttransform(src) >= 2
    for (j = 1; j < src->height-1; ++j)
    {
        for (i = 1; i < src->width-1; ++i)
        {
            if (src->data.y[j * src->stride + i] != 0)
                dist->data.y[j * dist->stride + i] = 0;
            else
            {
                int has_edge = 0;
                for (k = 0; k < sizeof(offsets)/sizeof(offsets[0]); ++k)
                {
                    if (src->data.y[(j + offsets[k][1]) * src->stride + i + offsets[k][0]] != 0)
                    {
                        has_edge = 1;
                        break;
                    }
                }

                dist->data.y[j * dist->stride + i] = (has_edge ? 1 : 255);
            }
        }
    }

    // count pixels where disttransform(src) < 2 and dst != 0
    total = count = 0;
    for (j = 0; j < dst->height; ++j)
    {
        for (i = 0; i < dst->width; ++i)
        {
            if (dst->data.y[j * dst->stride + i] != 0)
            {
                total += 1;
                count += (dist->data.y[j * dist->stride + i] < 2) ? 1 : 0;
            }
        }
    }

    *total_edge_pixels = total;

    return count;
}

#ifndef USE_OPENCV_GENERATED_REFERENCE
// own blur to not depend on OpenVX borders handling
static CT_Image gaussian5x5(CT_Image img)
{
    CT_Image res;
    uint32_t i, j, k, n;
    uint32_t ww[] = {1, 4, 6, 4, 1};

    ASSERT_(return 0, img);
    ASSERT_(return 0, img->format == VX_DF_IMAGE_U8);

    res = ct_allocate_image(img->width, img->height, img->format);
    ASSERT_(return 0, res);

    for (j = 0; j < img->height; ++j)
    {
        for (i = 0; i < img->width; ++i)
        {
            uint32_t r = 0;
            for (k = 0; k < 5; ++k)
            {
                uint32_t rr = 0;
                uint32_t kj = k + j < 2 ? 0 : (k + j - 2 >= img->width ? img->width -1 :  k + j - 2);
                for (n = 0; n < 5; ++n)
                {
                    uint32_t ni = n + i < 2 ? 0 : (n + i - 2 >= img->width ? img->width -1 :  n + i - 2);
                    rr += ww[n] * img->data.y[kj * img->stride + ni];
                }

                r += rr * ww[k];
            }
            res->data.y[j * res->stride + i] = (r + (1<<7)) >> 8;
        }
    }

    return res;
}
#endif

static CT_Image get_source_image(const char* filename)
{
#ifndef USE_OPENCV_GENERATED_REFERENCE
    if (strncmp(filename, "blurred_", 8) == 0)
        return gaussian5x5(ct_read_image(filename + 8, 1));
    else
#endif
        return ct_read_image(filename, 1);
}

/*
#define vxuCannyEdgeDetector vxuCannyEdgeDetector_ref
vx_status vxuCannyEdgeDetector_ref(vx_context context, vx_image input, vx_threshold hyst,
                               vx_int32 gradient_size, vx_enum norm_type,
                               vx_image output)
{
    CT_Image src, dst;
    int32_t hi,lo;
    vx_image tmp;
    vx_status s = VX_SUCCESS;

    src = ct_image_from_vx_image(input);
    dst = ct_allocate_image(src->width, src->height, VX_DF_IMAGE_U8);

    s |= vxQueryThreshold(hyst, VX_THRESHOLD_THRESHOLD_LOWER, &lo, sizeof(lo));
    s |= vxQueryThreshold(hyst, VX_THRESHOLD_THRESHOLD_UPPER, &hi, sizeof(hi));

    reference_canny(src, dst, lo, hi, gradient_size, norm_type);
    tmp = vxCreateImage(context, src->width, src->height, VX_DF_IMAGE_U8);
    s |= vxuNot(context, ct_image_to_vx_image(dst, context), tmp);
    s |= vxuNot(context, tmp, output);
    return s;
}
*/

static CT_Image get_reference_result(const char* src_name, CT_Image src, int32_t low_thresh, int32_t high_thresh, uint32_t gsz, vx_enum norm)
{
#ifdef USE_OPENCV_GENERATED_REFERENCE
    char buff[1024];
    sprintf(buff, "canny_%ux%u_%d_%d_%s_%s", gsz, gsz, low_thresh, high_thresh, norm == VX_NORM_L1 ? "L1" : "L2", src_name);
    // printf("reading: %s\n", buff);
    return ct_read_image(buff, 1);
#else
    CT_Image dst;
    ASSERT_(return 0, src);
    if (dst = ct_allocate_image(src->width, src->height, VX_DF_IMAGE_U8))
        reference_canny(src, dst, low_thresh, high_thresh, gsz, norm);
    return dst;
#endif
}

TESTCASE(vxuCanny, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(vxCanny,  CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    const char* filename;
    int32_t grad_size;
    vx_enum norm_type;
    int32_t low_thresh;
    int32_t high_thresh;
} canny_arg;

#define BIT_EXACT_ARG(grad, thresh) ARG(#grad "x" #grad " thresh=" #thresh, "lena_gray.bmp", grad, VX_NORM_L1, thresh, thresh)
#define DIS_BIT_EXACT_ARG(grad, thresh) ARG("DISABLED_" #grad "x" #grad " thresh=" #thresh, "lena_gray.bmp", grad, VX_NORM_L1, thresh, thresh)

TEST_WITH_ARG(vxuCanny, BitExactL1, canny_arg, BIT_EXACT_ARG(3, 120), BIT_EXACT_ARG(5, 100), /* DIS_BIT_EXACT_ARG(7, 80 do not enable this argument) */)
{
    vx_image src, dst;
    vx_threshold hyst;
    CT_Image lena, vxdst, refdst;
    vx_int32 low_thresh  = arg_->low_thresh;
    vx_int32 high_thresh = arg_->high_thresh;
    vx_int32 false_val = 0;
    vx_int32 true_val = 255;
    vx_border_t border = { VX_BORDER_UNDEFINED, {{ 0 }} };
    vx_int32 border_width = arg_->grad_size/2 + 1;
    vx_context context = context_->vx_context_;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border)));

    ASSERT_NO_FAILURE(lena = get_source_image(arg_->filename));
    ASSERT_NO_FAILURE(src = ct_image_to_vx_image(lena, context));
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, lena->width, lena->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(hyst = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8), VX_TYPE_THRESHOLD);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_LOWER, &low_thresh,  sizeof(low_thresh)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_UPPER, &high_thresh, sizeof(high_thresh)));
    /* explicitly set FALSE_VALUE and TRUE_VALUE for hyst parameter */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_FALSE_VALUE, &false_val, sizeof(false_val)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_TRUE_VALUE, &true_val, sizeof(true_val)));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxuCannyEdgeDetector(context, src, hyst, arg_->grad_size, arg_->norm_type, dst));

    ASSERT_NO_FAILURE(vxdst = ct_image_from_vx_image(dst));
    ASSERT_NO_FAILURE(refdst = get_reference_result(arg_->filename, lena, low_thresh, high_thresh, arg_->grad_size, arg_->norm_type));

    ASSERT_NO_FAILURE(ct_adjust_roi(vxdst,  border_width, border_width, border_width, border_width));
    ASSERT_NO_FAILURE(ct_adjust_roi(refdst, border_width, border_width, border_width, border_width));

    ASSERT_EQ_CTIMAGE(refdst, vxdst);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseThreshold(&hyst));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&src));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&dst));
}

TEST_WITH_ARG(vxCanny, BitExactL1, canny_arg, BIT_EXACT_ARG(3, 120), BIT_EXACT_ARG(5, 100), /* DIS_BIT_EXACT_ARG(7, 80 do not enable this argument) */)
{
    vx_image src, dst;
    vx_graph graph;
    vx_node node;
    vx_threshold hyst;
    CT_Image lena, vxdst, refdst;
    vx_int32 low_thresh  = arg_->low_thresh;
    vx_int32 high_thresh = arg_->high_thresh;
    vx_int32 false_val = 0;
    vx_int32 true_val = 255;
    vx_border_t border = { VX_BORDER_UNDEFINED, {{ 0 }} };
    vx_int32 border_width = arg_->grad_size/2 + 1;
    vx_context context = context_->vx_context_;

    ASSERT_NO_FAILURE(lena = get_source_image(arg_->filename));
    ASSERT_NO_FAILURE(src = ct_image_to_vx_image(lena, context));
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, lena->width, lena->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(hyst = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8), VX_TYPE_THRESHOLD);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_LOWER, &low_thresh,  sizeof(low_thresh)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_UPPER, &high_thresh, sizeof(high_thresh)));
    /* explicitly set FALSE_VALUE and TRUE_VALUE for hyst parameter */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_FALSE_VALUE, &false_val, sizeof(false_val)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_TRUE_VALUE, &true_val, sizeof(true_val)));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = vxCannyEdgeDetectorNode(graph, src, hyst, arg_->grad_size, arg_->norm_type, dst), VX_TYPE_NODE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    // run graph
#ifdef EXECUTE_ASYNC
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

    ASSERT_NO_FAILURE(vxdst = ct_image_from_vx_image(dst));
    ASSERT_NO_FAILURE(refdst = get_reference_result(arg_->filename, lena, low_thresh, high_thresh, arg_->grad_size, arg_->norm_type));

    ASSERT_NO_FAILURE(ct_adjust_roi(vxdst,  border_width, border_width, border_width, border_width));
    ASSERT_NO_FAILURE(ct_adjust_roi(refdst, border_width, border_width, border_width, border_width));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseNode(&node));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseGraph(&graph));

    ASSERT_EQ_CTIMAGE(refdst, vxdst);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseThreshold(&hyst));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&src));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&dst));
}

#define CANNY_ARG(grad, norm, lo, hi, file) ARG(#file "/" #norm " " #grad "x" #grad " thresh=(" #lo ", " #hi ")", #file ".bmp", grad, VX_NORM_##norm, lo, hi)
#define DISABLED_CANNY_ARG(grad, norm, lo, hi, file) ARG("DISABLED_" #file "/" #norm " " #grad "x" #grad " thresh=(" #lo ", " #hi ")", #file ".bmp", grad, VX_NORM_##norm, lo, hi)

TEST_WITH_ARG(vxuCanny, Lena, canny_arg,
    CANNY_ARG(3, L1, 100, 120, lena_gray),
    CANNY_ARG(3, L2, 100, 120, lena_gray),
    CANNY_ARG(3, L1, 90,  130, lena_gray),
    CANNY_ARG(3, L2, 90,  130, lena_gray),
    CANNY_ARG(3, L1, 70,  71 , lena_gray),
    CANNY_ARG(3, L2, 70,  71 , lena_gray),
    CANNY_ARG(3, L1, 150, 220, lena_gray),
    CANNY_ARG(3, L2, 150, 220, lena_gray),
    CANNY_ARG(5, L1, 100, 120, lena_gray),
    CANNY_ARG(5, L2, 100, 120, lena_gray),
    CANNY_ARG(7, L1, 100, 120, lena_gray),
    CANNY_ARG(7, L2, 100, 120, lena_gray),

    CANNY_ARG(5, L1, 1200, 1440, lena_gray),
    CANNY_ARG(5, L2, 1200, 1440, lena_gray),
    CANNY_ARG(7, L1, 16000, 19200, lena_gray),
    CANNY_ARG(7, L2, 16000, 19200, lena_gray),

    CANNY_ARG(3, L1, 100, 120, blurred_lena_gray),
    CANNY_ARG(3, L2, 100, 120, blurred_lena_gray),
    CANNY_ARG(3, L1, 90,  125, blurred_lena_gray),
    CANNY_ARG(3, L2, 90,  130, blurred_lena_gray),
    CANNY_ARG(3, L1, 70,  71 , blurred_lena_gray),
    CANNY_ARG(3, L2, 70,  71 , blurred_lena_gray),
    CANNY_ARG(3, L1, 150, 220, blurred_lena_gray),
    CANNY_ARG(3, L2, 150, 220, blurred_lena_gray),
    CANNY_ARG(5, L1, 100, 120, blurred_lena_gray),
    CANNY_ARG(5, L2, 100, 120, blurred_lena_gray),
    CANNY_ARG(7, L1, 100, 120, blurred_lena_gray),
    CANNY_ARG(7, L2, 100, 120, blurred_lena_gray),

)
{
    uint32_t total, count;
    vx_image src, dst;
    vx_threshold hyst;
    CT_Image lena, vxdst, refdst, dist;
    vx_int32 low_thresh  = arg_->low_thresh;
    vx_int32 high_thresh = arg_->high_thresh;
    vx_border_t border = { VX_BORDER_UNDEFINED, {{ 0 }} };
    vx_int32 border_width = arg_->grad_size/2 + 1;
    vx_context context = context_->vx_context_;
    vx_enum thresh_data_type = VX_TYPE_UINT8;
    if (low_thresh > 255)
        thresh_data_type = VX_TYPE_INT16;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border)));

    ASSERT_NO_FAILURE(lena = get_source_image(arg_->filename));
    ASSERT_NO_FAILURE(src = ct_image_to_vx_image(lena, context));
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, lena->width, lena->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(hyst = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, thresh_data_type), VX_TYPE_THRESHOLD);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_LOWER, &low_thresh,  sizeof(low_thresh)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_UPPER, &high_thresh, sizeof(high_thresh)));
    /* FALSE_VALUE and TRUE_VALUE of hyst parameter are set to their default values (0, 255) by vxCreateThreshold */
    /* test reference data are computed with assumption that FALSE_VALUE and TRUE_VALUE set to 0 and 255 */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxuCannyEdgeDetector(context, src, hyst, arg_->grad_size, arg_->norm_type, dst));

    ASSERT_NO_FAILURE(vxdst = ct_image_from_vx_image(dst));

    ASSERT_NO_FAILURE(refdst = get_reference_result(arg_->filename, lena, low_thresh, high_thresh, arg_->grad_size, arg_->norm_type));
    ASSERT_NO_FAILURE(ct_adjust_roi(vxdst,  border_width, border_width, border_width, border_width));
    ASSERT_NO_FAILURE(ct_adjust_roi(refdst, border_width, border_width, border_width, border_width));

    ASSERT_NO_FAILURE(dist = ct_allocate_image(refdst->width, refdst->height, VX_DF_IMAGE_U8));

    // disttransform(x,y) < tolerance for all (x,y) such that output(x,y) = 255,
    // where disttransform is the distance transform image with Euclidean distance
    // of the reference(x,y) (canny edge ground truth). This condition should be
    // satisfied by 98% of output edge pixels, tolerance = 2.
    ASSERT_NO_FAILURE(count = disttransform2_metric(refdst, vxdst, dist, &total));

    if (count < CANNY_ACCEPTANCE_THRESHOLD * total)
    {
        CT_RecordFailureAtFormat("disttransform(reference) < 2 only for %u of %u pixels of output edges which is %.2f%% < %.2f%%", __FUNCTION__, __FILE__, __LINE__,
            count, total, count/(double)total*100, CANNY_ACCEPTANCE_THRESHOLD*100);

        // ct_write_image("canny_vx.bmp", vxdst);
        // ct_write_image("canny_ref.bmp", refdst);
    }

    // And the inverse: disttransform(x,y) < tolerance for all (x,y) such that
    // reference(x,y) = 255, where disttransform is the distance transform image
    // with Euclidean distance of the output(x,y) (canny edge ground truth). This
    // condition should be satisfied by 98% of reference edge pixels, tolerance = 2.
    ASSERT_NO_FAILURE(count = disttransform2_metric(vxdst, refdst, dist, &total));

    if (count < CANNY_ACCEPTANCE_THRESHOLD * total)
    {
        CT_RecordFailureAtFormat("disttransform(output) < 2 only for %u of %u pixels of reference edges which is %.2f%% < %.2f%%", __FUNCTION__, __FILE__, __LINE__,
            count, total, count/(double)total*100, CANNY_ACCEPTANCE_THRESHOLD*100);

        // ct_write_image("canny_vx.bmp", vxdst);
        // ct_write_image("canny_ref.bmp", refdst);
    }

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseThreshold(&hyst));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&src));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&dst));
}

TEST_WITH_ARG(vxCanny, Lena, canny_arg,
    CANNY_ARG(3, L1, 100, 120, lena_gray),
    CANNY_ARG(3, L2, 100, 120, lena_gray),
    CANNY_ARG(3, L1, 90,  130, lena_gray),
    CANNY_ARG(3, L2, 90,  130, lena_gray),
    CANNY_ARG(3, L1, 70,  71 , lena_gray),
    CANNY_ARG(3, L2, 70,  71 , lena_gray),
    CANNY_ARG(3, L1, 150, 220, lena_gray),
    CANNY_ARG(3, L2, 150, 220, lena_gray),
    CANNY_ARG(5, L1, 100, 120, lena_gray),
    CANNY_ARG(5, L2, 100, 120, lena_gray),
    CANNY_ARG(7, L1, 100, 120, lena_gray),
    CANNY_ARG(7, L2, 100, 120, lena_gray),

    CANNY_ARG(5, L1, 1200, 1440, lena_gray),
    CANNY_ARG(5, L2, 1200, 1440, lena_gray),
    CANNY_ARG(7, L1, 16000, 19200, lena_gray),
    CANNY_ARG(7, L2, 16000, 19200, lena_gray),

    CANNY_ARG(3, L1, 100, 120, blurred_lena_gray),
    CANNY_ARG(3, L2, 100, 120, blurred_lena_gray),
    CANNY_ARG(3, L1, 90,  125, blurred_lena_gray),
    CANNY_ARG(3, L2, 90,  130, blurred_lena_gray),
    CANNY_ARG(3, L1, 70,  71 , blurred_lena_gray),
    CANNY_ARG(3, L2, 70,  71 , blurred_lena_gray),
    CANNY_ARG(3, L1, 150, 220, blurred_lena_gray),
    CANNY_ARG(3, L2, 150, 220, blurred_lena_gray),
    CANNY_ARG(5, L1, 100, 120, blurred_lena_gray),
    CANNY_ARG(5, L2, 100, 120, blurred_lena_gray),
    CANNY_ARG(7, L1, 100, 120, blurred_lena_gray),
    CANNY_ARG(7, L2, 100, 120, blurred_lena_gray),

)
{
    uint32_t total, count;
    vx_image src, dst;
    vx_threshold hyst;
    vx_graph graph;
    vx_node node;
    CT_Image lena, vxdst, refdst, dist;
    vx_int32 low_thresh  = arg_->low_thresh;
    vx_int32 high_thresh = arg_->high_thresh;
    vx_border_t border = { VX_BORDER_UNDEFINED, {{ 0 }} };
    vx_int32 border_width = arg_->grad_size/2 + 1;
    vx_context context = context_->vx_context_;
    vx_enum thresh_data_type = VX_TYPE_UINT8;
    if (low_thresh > 255)
        thresh_data_type = VX_TYPE_INT16;

    ASSERT_NO_FAILURE(lena = get_source_image(arg_->filename));
    ASSERT_NO_FAILURE(src = ct_image_to_vx_image(lena, context));
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, lena->width, lena->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(hyst = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, thresh_data_type), VX_TYPE_THRESHOLD);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_LOWER, &low_thresh,  sizeof(low_thresh)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_UPPER, &high_thresh, sizeof(high_thresh)));
    /* FALSE_VALUE and TRUE_VALUE of hyst parameter are set to their default values (0, 255) by vxCreateThreshold */
    /* test reference data are computed with assumption that FALSE_VALUE and TRUE_VALUE set to 0 and 255 */

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = vxCannyEdgeDetectorNode(graph, src, hyst, arg_->grad_size, arg_->norm_type, dst), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    // run graph
#ifdef EXECUTE_ASYNC
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseNode(&node));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseGraph(&graph));

    ASSERT_NO_FAILURE(vxdst = ct_image_from_vx_image(dst));
    ASSERT_NO_FAILURE(refdst = get_reference_result(arg_->filename, lena, low_thresh, high_thresh, arg_->grad_size, arg_->norm_type));

    ASSERT_NO_FAILURE(ct_adjust_roi(vxdst,  border_width, border_width, border_width, border_width));
    ASSERT_NO_FAILURE(ct_adjust_roi(refdst, border_width, border_width, border_width, border_width));

    ASSERT_NO_FAILURE(dist = ct_allocate_image(refdst->width, refdst->height, VX_DF_IMAGE_U8));

    // disttransform(x,y) < tolerance for all (x,y) such that output(x,y) = 255,
    // where disttransform is the distance transform image with Euclidean distance
    // of the reference(x,y) (canny edge ground truth). This condition should be
    // satisfied by 98% of output edge pixels, tolerance = 2.
    ASSERT_NO_FAILURE(count = disttransform2_metric(refdst, vxdst, dist, &total));
    if (count < CANNY_ACCEPTANCE_THRESHOLD * total)
    {
        CT_RecordFailureAtFormat("disttransform(reference) < 2 only for %u of %u pixels of output edges which is %.2f%% < %.2f%%", __FUNCTION__, __FILE__, __LINE__,
            count, total, count/(double)total*100, CANNY_ACCEPTANCE_THRESHOLD*100);

        // ct_write_image("canny_vx.bmp", vxdst);
        // ct_write_image("canny_ref.bmp", refdst);
    }

    // And the inverse: disttransform(x,y) < tolerance for all (x,y) such that
    // reference(x,y) = 255, where disttransform is the distance transform image
    // with Euclidean distance of the output(x,y) (canny edge ground truth). This
    // condition should be satisfied by 98% of reference edge pixels, tolerance = 2.
    ASSERT_NO_FAILURE(count = disttransform2_metric(vxdst, refdst, dist, &total));
    if (count < CANNY_ACCEPTANCE_THRESHOLD * total)
    {
        CT_RecordFailureAtFormat("disttransform(output) < 2 only for %u of %u pixels of reference edges which is %.2f%% < %.2f%%", __FUNCTION__, __FILE__, __LINE__,
            count, total, count/(double)total*100, CANNY_ACCEPTANCE_THRESHOLD*100);

        // ct_write_image("canny_vx.bmp", vxdst);
        // ct_write_image("canny_ref.bmp", refdst);
    }

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseThreshold(&hyst));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&src));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&dst));
}

TESTCASE_TESTS(vxuCanny, DISABLED_BitExactL1, Lena)
TESTCASE_TESTS(vxCanny,  DISABLED_BitExactL1, Lena)
