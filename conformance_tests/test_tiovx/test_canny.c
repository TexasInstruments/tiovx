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
#include <stdint.h>
#include <VX/vx.h>
#include <VX/vx_compatibility.h>
#include <stdio.h>
#include "shared_functions.h"

#define USE_OPENCV_GENERATED_REFERENCE
#define CANNY_ACCEPTANCE_THRESHOLD 0.95
//#define EXECUTE_ASYNC

#define MAX_NODES 10

#define CREF_EDGE 2
#define CREF_LINK 1
#define CREF_NONE 0

#define MAXLENGTH   (512u)


static uint8_t box3x3_calculate(CT_Image src, uint32_t x, uint32_t y)
{
    uint8_t res = (uint8_t)(ct_floor_u32_no_overflow( ((float)(
            (int16_t)(*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 0)) +
                      *CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 0) +
                      *CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 0) +
                      *CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 1) +
                      *CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 1) +
                      *CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 1) +
                      *CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 1) +
                      *CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 1) +
                      *CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 1)) )/9 ) );
    return res;
}

static uint8_t box3x3_calculate_replicate(CT_Image src, uint32_t x_, uint32_t y_)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    uint8_t res = (uint8_t)(ct_floor_u32_no_overflow( ((float)(
            (int16_t)(CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 0)) +
                      CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 0) +
                      CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 0) +
                      CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 1) +
                      CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 1) +
                      CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 1) +
                      CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 1) +
                      CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 1) +
                      CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 1)) )/9 ) );
    return res;
}

static uint8_t box3x3_calculate_constant(CT_Image src, uint32_t x_, uint32_t y_, vx_uint32 constant_value)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    uint8_t res = (uint8_t)(ct_floor_u32_no_overflow( ((float)(
            (int16_t)(CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 0, constant_value)) +
                      CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 0, constant_value) +
                      CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 0, constant_value) +
                      CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 1, constant_value) +
                      CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 1, constant_value) +
                      CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 1, constant_value) +
                      CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 1, constant_value) +
                      CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 1, constant_value) +
                      CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 1, constant_value)) )/9 ) );
    return res;
}

static int32_t offsets[][2] =
{
    { -1, -1}, {  0, -1}, {  1, -1},
    { -1,  0},            {  1,  0},
    { -1,  1}, {  0,  1}, {  1,  1}
};

//#ifndef USE_OPENCV_GENERATED_REFERENCE
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
//#endif

// computes count(disttransform(src) >= 2, where dst != 0)
static uint32_t disttransform2_metric(CT_Image src, CT_Image dst, CT_Image dist, uint32_t* total_edge_pixels, uint8_t value)
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
            if (dst->data.y[j * dst->stride + i] != value) // Must be 255 when using vxNot
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

static CT_Image get_reference_result_virtual(const char* src_name, CT_Image src, CT_Image virt_ctimage, int32_t low_thresh, int32_t high_thresh, uint32_t gsz, vx_enum norm)
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
        reference_canny(src, virt_ctimage, low_thresh, high_thresh, gsz, norm);
        referenceNot(virt_ctimage, dst);
    return dst;
#endif
}

static CT_Image get_supernode_result(CT_Image src, int32_t low_thresh, int32_t high_thresh, uint32_t gsz, vx_enum norm)
{
    CT_Image dst;
    ASSERT_(return 0, src);
    dst = ct_allocate_image(src->width, src->height, VX_DF_IMAGE_U8);
    reference_canny(src, dst, low_thresh, high_thresh, gsz, norm);
    return dst;
}

TESTCASE(tivxCanny,  CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    const char* filename;
    int32_t grad_size;
    vx_enum norm_type;
    int32_t low_thresh;
    int32_t high_thresh;
} canny_arg;

#define CANNY_ARG(grad, norm, lo, hi, file) ARG(#file "/" #norm " " #grad "x" #grad " thresh=(" #lo ", " #hi ")", #file ".bmp", grad, VX_NORM_##norm, lo, hi)
#define DISABLED_CANNY_ARG(grad, norm, lo, hi, file) ARG("DISABLED_" #file "/" #norm " " #grad "x" #grad " thresh=(" #lo ", " #hi ")", #file ".bmp", grad, VX_NORM_##norm, lo, hi)

TEST_WITH_ARG(tivxCanny, virtImage, canny_arg,
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
    vx_image src, dst, virt;
    vx_threshold hyst;
    vx_graph graph;
    vx_node node1, node2;
    CT_Image lena, vxdst, refdst, dist, virt_ctimage;
    vx_int32 low_thresh  = arg_->low_thresh;
    vx_int32 high_thresh = arg_->high_thresh;
    vx_border_t border = { VX_BORDER_UNDEFINED, {{ 0 }} };
    vx_int32 border_width = arg_->grad_size/2 + 1;
    vx_context context = context_->vx_context_;
    vx_enum thresh_data_type = VX_TYPE_UINT8;
    vx_perf_t perf_node1, perf_node2, perf_graph;
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
    ASSERT_VX_OBJECT(virt = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node1 = vxCannyEdgeDetectorNode(graph, src, hyst, arg_->grad_size, arg_->norm_type, virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, virt, dst), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    // run graph
#ifdef EXECUTE_ASYNC
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseNode(&node1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseNode(&node2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseGraph(&graph));

    ASSERT_NO_FAILURE(vxdst = ct_image_from_vx_image(dst));
    virt_ctimage = ct_allocate_image(lena->width, lena->height, VX_DF_IMAGE_U8);
    ASSERT_NO_FAILURE(refdst = get_reference_result_virtual(arg_->filename, lena, virt_ctimage, low_thresh, high_thresh, arg_->grad_size, arg_->norm_type));

    ASSERT_NO_FAILURE(ct_adjust_roi(vxdst,  border_width, border_width, border_width, border_width));
    ASSERT_NO_FAILURE(ct_adjust_roi(refdst, border_width, border_width, border_width, border_width));

    ASSERT_NO_FAILURE(dist = ct_allocate_image(refdst->width, refdst->height, VX_DF_IMAGE_U8));

    // disttransform(x,y) < tolerance for all (x,y) such that output(x,y) = 255,
    // where disttransform is the distance transform image with Euclidean distance
    // of the reference(x,y) (canny edge ground truth). This condition should be
    // satisfied by 98% of output edge pixels, tolerance = 2.
    ASSERT_NO_FAILURE(count = disttransform2_metric(refdst, vxdst, dist, &total, 255));
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
    ASSERT_NO_FAILURE(count = disttransform2_metric(vxdst, refdst, dist, &total, 255));
    if (count < CANNY_ACCEPTANCE_THRESHOLD * total)
    {
        CT_RecordFailureAtFormat("disttransform(output) < 2 only for %u of %u pixels of reference edges which is %.2f%% < %.2f%%", __FUNCTION__, __FILE__, __LINE__,
            count, total, count/(double)total*100, CANNY_ACCEPTANCE_THRESHOLD*100);

        // ct_write_image("canny_vx.bmp", vxdst);
        // ct_write_image("canny_ref.bmp", refdst);
    }

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseThreshold(&hyst));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&src));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&virt));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&dst));

    printPerformance(perf_node1, lena->width*lena->height, "N1");
    printPerformance(perf_node2, lena->width*lena->height, "N2");
    printPerformance(perf_graph, lena->width*lena->height, "G1");
}

TEST_WITH_ARG(tivxCanny, multipleNode, canny_arg,
    CANNY_ARG(3, L1, 100, 120, lena_gray),
    CANNY_ARG(3, L2, 100, 120, lena_gray),
    CANNY_ARG(3, L2, 90,  130, lena_gray),
    CANNY_ARG(3, L1, 70,  71 , lena_gray),
    CANNY_ARG(3, L2, 70,  71 , lena_gray),
    CANNY_ARG(3, L1, 150, 220, lena_gray),
    CANNY_ARG(3, L2, 150, 220, lena_gray),
    CANNY_ARG(5, L1, 100, 120, lena_gray),
    CANNY_ARG(5, L2, 100, 120, lena_gray),
    CANNY_ARG(7, L1, 100, 120, lena_gray),
    CANNY_ARG(7, L2, 100, 120, lena_gray),
)
{
    uint32_t total, count;
    vx_image src0, src1, dst0, dst1;
    vx_threshold hyst;
    vx_graph graph;
    vx_node node1, node2;
    CT_Image lena, blurred_lena, vxdst0, vxdst1, refdst0, refdst1, dist0, dist1;
    vx_int32 low_thresh  = arg_->low_thresh;
    vx_int32 high_thresh = arg_->high_thresh;
    vx_border_t border = { VX_BORDER_UNDEFINED, {{ 0 }} };
    vx_int32 border_width = arg_->grad_size/2 + 1;
    vx_context context = context_->vx_context_;
    vx_enum thresh_data_type = VX_TYPE_UINT8;
    vx_perf_t perf_node1, perf_node2, perf_graph;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;
    if (low_thresh > 255)
        thresh_data_type = VX_TYPE_INT16;

    ASSERT_NO_FAILURE(lena = get_source_image("lena_gray.bmp"));
    ASSERT_NO_FAILURE(blurred_lena = get_source_image("blurred_lena_gray.bmp"));
    ASSERT_NO_FAILURE(src0 = ct_image_to_vx_image(lena, context));
    ASSERT_NO_FAILURE(src1 = ct_image_to_vx_image(blurred_lena, context));
    ASSERT_VX_OBJECT(dst0 = vxCreateImage(context, lena->width, lena->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst1 = vxCreateImage(context, blurred_lena->width, blurred_lena->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(hyst = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, thresh_data_type), VX_TYPE_THRESHOLD);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_LOWER, &low_thresh,  sizeof(low_thresh)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_UPPER, &high_thresh, sizeof(high_thresh)));
    /* FALSE_VALUE and TRUE_VALUE of hyst parameter are set to their default values (0, 255) by vxCreateThreshold */
    /* test reference data are computed with assumption that FALSE_VALUE and TRUE_VALUE set to 0 and 255 */

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node1 = vxCannyEdgeDetectorNode(graph, src0, hyst, arg_->grad_size, arg_->norm_type, dst0), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxCannyEdgeDetectorNode(graph, src1, hyst, arg_->grad_size, arg_->norm_type, dst1), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));

    // run graph
#ifdef EXECUTE_ASYNC
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

    vxGetValidRegionImage(src0, &src_rect);
    vxGetValidRegionImage(dst0, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), lena->width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), lena->height);

    ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), lena->width - 2*border_width);
    ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), lena->height - 2*border_width);

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseNode(&node1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseNode(&node2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseGraph(&graph));

    ASSERT_NO_FAILURE(vxdst0 = ct_image_from_vx_image(dst0));
    ASSERT_NO_FAILURE(refdst0 = get_reference_result("lena_gray.bmp", lena, low_thresh, high_thresh, arg_->grad_size, arg_->norm_type));

    ASSERT_NO_FAILURE(vxdst1 = ct_image_from_vx_image(dst1));
    ASSERT_NO_FAILURE(refdst1 = get_reference_result("blurred_lena_gray.bmp", blurred_lena, low_thresh, high_thresh, arg_->grad_size, arg_->norm_type));

    ASSERT_NO_FAILURE(ct_adjust_roi(vxdst0,  border_width, border_width, border_width, border_width));
    ASSERT_NO_FAILURE(ct_adjust_roi(refdst0, border_width, border_width, border_width, border_width));

    ASSERT_NO_FAILURE(ct_adjust_roi(vxdst1,  border_width, border_width, border_width, border_width));
    ASSERT_NO_FAILURE(ct_adjust_roi(refdst1, border_width, border_width, border_width, border_width));

    ASSERT_NO_FAILURE(dist0 = ct_allocate_image(refdst0->width, refdst0->height, VX_DF_IMAGE_U8));

    ASSERT_NO_FAILURE(dist1 = ct_allocate_image(refdst1->width, refdst1->height, VX_DF_IMAGE_U8));

    // disttransform(x,y) < tolerance for all (x,y) such that output(x,y) = 255,
    // where disttransform is the distance transform image with Euclidean distance
    // of the reference(x,y) (canny edge ground truth). This condition should be
    // satisfied by 98% of output edge pixels, tolerance = 2.
    ASSERT_NO_FAILURE(count = disttransform2_metric(refdst0, vxdst0, dist0, &total, 0));
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
    ASSERT_NO_FAILURE(count = disttransform2_metric(vxdst0, refdst0, dist0, &total, 0));
    if (count < CANNY_ACCEPTANCE_THRESHOLD * total)
    {
        CT_RecordFailureAtFormat("disttransform(output) < 2 only for %u of %u pixels of reference edges which is %.2f%% < %.2f%%", __FUNCTION__, __FILE__, __LINE__,
            count, total, count/(double)total*100, CANNY_ACCEPTANCE_THRESHOLD*100);

        // ct_write_image("canny_vx.bmp", vxdst);
        // ct_write_image("canny_ref.bmp", refdst);
    }

    // disttransform(x,y) < tolerance for all (x,y) such that output(x,y) = 255,
    // where disttransform is the distance transform image with Euclidean distance
    // of the reference(x,y) (canny edge ground truth). This condition should be
    // satisfied by 98% of output edge pixels, tolerance = 2.
    ASSERT_NO_FAILURE(count = disttransform2_metric(refdst1, vxdst1, dist1, &total, 0));
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
    ASSERT_NO_FAILURE(count = disttransform2_metric(vxdst1, refdst1, dist1, &total, 0));
    if (count < CANNY_ACCEPTANCE_THRESHOLD * total)
    {
        CT_RecordFailureAtFormat("disttransform(output) < 2 only for %u of %u pixels of reference edges which is %.2f%% < %.2f%%", __FUNCTION__, __FILE__, __LINE__,
            count, total, count/(double)total*100, CANNY_ACCEPTANCE_THRESHOLD*100);

        // ct_write_image("canny_vx.bmp", vxdst);
        // ct_write_image("canny_ref.bmp", refdst);
    }

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseThreshold(&hyst));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&src0));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&src1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&dst0));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&dst1));

    printPerformance(perf_node1, lena->width*lena->height, "N1");
    printPerformance(perf_node2, lena->width*lena->height, "N2");
    printPerformance(perf_graph, lena->width*lena->height, "G1");
}

TEST_WITH_ARG(tivxCanny, negativeTestBorderMode, canny_arg,
    CANNY_ARG(3, L1, 100, 120, lena_gray),
    CANNY_ARG(3, L2, 90,  130, lena_gray)
)
{
    uint32_t total, count;
    vx_image src, dst, virt;
    vx_threshold hyst;
    vx_graph graph;
    vx_node node1, node2;
    CT_Image lena, vxdst, refdst, dist, virt_ctimage;
    vx_int32 low_thresh  = arg_->low_thresh;
    vx_int32 high_thresh = arg_->high_thresh;
    vx_border_t border = { VX_BORDER_REPLICATE, {{ 0 }} };
    vx_int32 border_width = arg_->grad_size/2 + 1;
    vx_context context = context_->vx_context_;
    vx_enum thresh_data_type = VX_TYPE_UINT8;
    if (low_thresh > 255)
        thresh_data_type = VX_TYPE_INT16;

    if (90 == low_thresh)
        border.mode = VX_BORDER_CONSTANT;

    ASSERT_NO_FAILURE(lena = get_source_image(arg_->filename));
    ASSERT_NO_FAILURE(src = ct_image_to_vx_image(lena, context));
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, lena->width, lena->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(hyst = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, thresh_data_type), VX_TYPE_THRESHOLD);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_LOWER, &low_thresh,  sizeof(low_thresh)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_UPPER, &high_thresh, sizeof(high_thresh)));
    /* FALSE_VALUE and TRUE_VALUE of hyst parameter are set to their default values (0, 255) by vxCreateThreshold */
    /* test reference data are computed with assumption that FALSE_VALUE and TRUE_VALUE set to 0 and 255 */

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node1 = vxCannyEdgeDetectorNode(graph, src, hyst, arg_->grad_size, arg_->norm_type, virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, virt, dst), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_EQ_VX_STATUS(vxVerifyGraph(graph), VX_ERROR_NOT_SUPPORTED);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseNode(&node1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseNode(&node2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseGraph(&graph));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseThreshold(&hyst));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&src));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&virt));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&dst));
}

TEST_WITH_ARG(tivxCanny, testCannySupernode, canny_arg,
    CANNY_ARG(3, L1, 100, 120, lena_gray),
    CANNY_ARG(3, L2, 90,  130, lena_gray),
    CANNY_ARG(5, L1, 100, 120, lena_gray),
    CANNY_ARG(5, L2, 100, 120, lena_gray),
    CANNY_ARG(7, L1, 100, 120, lena_gray),
    CANNY_ARG(7, L2, 100, 120, lena_gray)
)
{
    int node_count = 3;
    uint32_t total, count;
    vx_image src, dst, virt1, virt2;
    vx_threshold hyst;
    vx_graph graph;
    vx_node node1, node2, node3;
    CT_Image lena, vxdst, refdst, dist, virt_ctimage;
    vx_int32 low_thresh  = arg_->low_thresh;
    vx_int32 high_thresh = arg_->high_thresh;
    vx_border_t border = { VX_BORDER_UNDEFINED, {{ 0 }} };
    vx_int32 border_width = arg_->grad_size/2 + 1;
    vx_context context = context_->vx_context_;
    vx_enum thresh_data_type = VX_TYPE_UINT8;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
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
    ASSERT_VX_OBJECT(virt1 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node1 = vxNotNode(graph, src, virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, virt1, virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxCannyEdgeDetectorNode(graph, virt2, hyst, arg_->grad_size, arg_->norm_type, dst), VX_TYPE_NODE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetNodeAttribute(node3, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_NO_FAILURE(node_list[0] = node1);
    ASSERT_NO_FAILURE(node_list[1] = node2);
    ASSERT_NO_FAILURE(node_list[2] = node3);
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    // run graph
#ifdef EXECUTE_ASYNC
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxReleaseSuperNode(&super_node));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseNode(&node1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseNode(&node2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseNode(&node3));
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
    ASSERT_NO_FAILURE(count = disttransform2_metric(refdst, vxdst, dist, &total, 0));
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
    ASSERT_NO_FAILURE(count = disttransform2_metric(vxdst, refdst, dist, &total, 0));
    if (count < CANNY_ACCEPTANCE_THRESHOLD * total)
    {
        CT_RecordFailureAtFormat("disttransform(output) < 2 only for %u of %u pixels of reference edges which is %.2f%% < %.2f%%", __FUNCTION__, __FILE__, __LINE__,
            count, total, count/(double)total*100, CANNY_ACCEPTANCE_THRESHOLD*100);

        // ct_write_image("canny_vx.bmp", vxdst);
        // ct_write_image("canny_ref.bmp", refdst);
    }

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseThreshold(&hyst));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&src));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&virt1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&virt2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&dst));

    printPerformance(perf_super_node, lena->width * lena->height, "SN");
    printPerformance(perf_graph, lena->width*lena->height, "G");
}

TEST(tivxCanny, testRobustness)
{
    vx_context context = context_->vx_context_;

    const vx_uint32 imageWidth = 300;
    const vx_uint32 imageHeight = 300;
    const vx_uint32 imageChannel = 3;
    vx_image inputImage = vxCreateImage(context, imageWidth, imageHeight, VX_DF_IMAGE_RGB);
    VX_CALL(vxSetReferenceName( (vx_reference)inputImage, "inputImage"));
    vx_image outputImage = vxCreateImage(context, imageWidth, imageHeight, VX_DF_IMAGE_RGB);
    VX_CALL(vxSetReferenceName( (vx_reference)outputImage, "outputImage"));

    /// Create the graph
    // It creates a graph with one input image and one output image (both RGB) which can be provided through the mechanism of graph parameters.
    // Graph converts first input RGB image into greyscale image to find edges using Canny Edge Detector after smoothing with Gaussian filter.
    // Then, it makes the edges black and AND the edges back with the Y value so as to super-impose a black background.
    // Finally, it converts greyscale image into RGB image and stores it in output image.
    vx_graph graph = vxCreateGraph(context);

    const vx_uint8 numyuv = 2; // Number of YUV images needed
    const vx_uint8 numu8 = 15; // Number of U8 images needed
    const vx_uint8 numnodes = numyuv+numu8+1;
    vx_image imgsu8[numu8], imgsyuv[numyuv];
    vx_node nodes[numnodes];

    vx_threshold hyst = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_INT16);
    VX_CALL(vxSetReferenceName( (vx_reference)hyst, "hyst"));
    vx_pixel_value_t lower_value = {{200}};
    vx_pixel_value_t upper_value = {{230}};
    vxSetThresholdAttribute(hyst, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_LOWER, &lower_value.S32, sizeof(lower_value.S32));
    vxSetThresholdAttribute(hyst, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_UPPER, &upper_value.S32, sizeof(upper_value.S32));
    char     refName[MAXLENGTH];

    for (int i = 0; i < numyuv; i++) {
        imgsyuv[i] = vxCreateImage(context, imageWidth, imageHeight, VX_DF_IMAGE_IYUV);
        snprintf(refName, MAXLENGTH,
             "imgsyuv_%d", i);
        VX_CALL(vxSetReferenceName( (vx_reference)imgsyuv[i], refName));
    }
    for (int i = 0; i < numu8 - 2; i++) {
        imgsu8[i] = vxCreateImage(context, imageWidth, imageHeight, VX_DF_IMAGE_U8);
        snprintf(refName, MAXLENGTH,
             "imgsu8_%d", i);
        VX_CALL(vxSetReferenceName( (vx_reference)imgsu8[i], refName));
    }
    imgsu8[13] = vxCreateImage(context, imageWidth / 2, imageHeight / 2, VX_DF_IMAGE_U8);
    VX_CALL(vxSetReferenceName( (vx_reference)imgsu8[13], "imgsu8_13"));
    imgsu8[14] = vxCreateImage(context, imageWidth / 2, imageHeight / 2, VX_DF_IMAGE_U8);
    VX_CALL(vxSetReferenceName( (vx_reference)imgsu8[14], "imgsu8_14"));

    // First, make a greyscale image by converting RGB to YUV and extracting the Y
    nodes[0] = vxColorConvertNode(graph, inputImage, imgsyuv[0]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[0], "n0"));

    // Get the parameter that will be the input and add it to the graph
    vx_parameter parameter = vxGetParameterByIndex(nodes[0], 0);
    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);

    nodes[1] = vxChannelExtractNode(graph, imgsyuv[0], VX_CHANNEL_Y, imgsu8[0]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[1], "n1"));

    // Apply Gaussian filter to reduce noise in image
    nodes[2] = vxGaussian3x3Node(graph, imgsu8[0], imgsu8[1]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[2], "n2"));
    nodes[3] = vxGaussian3x3Node(graph, imgsu8[1], imgsu8[2]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[3], "n3"));
    nodes[4] = vxGaussian3x3Node(graph, imgsu8[2], imgsu8[3]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[4], "n4"));
    nodes[5] = vxGaussian3x3Node(graph, imgsu8[3], imgsu8[4]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[5], "n5"));
    nodes[6] = vxGaussian3x3Node(graph, imgsu8[4], imgsu8[5]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[6], "n6"));
    nodes[7] = vxGaussian3x3Node(graph, imgsu8[5], imgsu8[6]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[7], "n7"));
    nodes[8] = vxGaussian3x3Node(graph, imgsu8[6], imgsu8[7]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[8], "n8"));
    nodes[9] = vxGaussian3x3Node(graph, imgsu8[7], imgsu8[8]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[9], "n9"));
    nodes[10] = vxGaussian3x3Node(graph, imgsu8[8], imgsu8[9]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[10], "n10"));

    // Use a Canny detector on the greyscale image to find edges
    nodes[11] = vxCannyEdgeDetectorNode(graph, imgsu8[9], hyst, 3, VX_NORM_L2, imgsu8[10]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[11], "n11"));

    // Make the edges black and AND the edges back with the Y value so as to super-impose a black background
    nodes[12] = vxNotNode(graph, imgsu8[10], imgsu8[11]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[12], "n12"));
    nodes[13] = vxAndNode(graph, imgsu8[0], imgsu8[11], imgsu8[12]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[13], "n13"));

    // Get the U and V channels and combine the color channels to give a YUV output image
    nodes[14] = vxChannelExtractNode(graph, imgsyuv[0], VX_CHANNEL_U, imgsu8[13]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[14], "n14"));
    nodes[15] = vxChannelExtractNode(graph, imgsyuv[0], VX_CHANNEL_V, imgsu8[14]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[15], "n15"));
    nodes[16] = vxChannelCombineNode(graph, imgsu8[12], imgsu8[13], imgsu8[14], NULL, imgsyuv[1]);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[16], "n16"));

    // Convert the YUV to RGB output
    nodes[17] = vxColorConvertNode(graph, imgsyuv[1], outputImage);
    VX_CALL(vxSetReferenceName( (vx_reference)nodes[17], "n17"));

    // Get the parameter that will be the output and add it to the graph
    parameter = vxGetParameterByIndex(nodes[17], 1);
    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);

    // Give the graph a name
    vxSetReferenceName((vx_reference)graph, "TestGraphWithGraphParameter");

    // Execute the graph
    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    // Release resources
    for (int i = 0; i < numyuv; i++) {
        VX_CALL(vxReleaseImage(&imgsyuv[i]));
    }
    for (int i = 0; i < numu8; i++) {
         VX_CALL(vxReleaseImage(&imgsu8[i]));
    }
    VX_CALL(vxReleaseImage(&inputImage));
    VX_CALL(vxReleaseImage(&outputImage));
    VX_CALL(vxReleaseThreshold(&hyst));
    for (int i = 0; i < numnodes; i++) {
        VX_CALL(vxReleaseNode(&nodes[i]));
    }
    VX_CALL(vxReleaseGraph(&graph));
}

#ifdef BUILD_BAM
#define testCannySupernode testCannySupernode
#else
#define testCannySupernode DISABLED_testCannySupernode
#endif

TESTCASE_TESTS(tivxCanny,
               virtImage,
               multipleNode,
               negativeTestBorderMode,
               #if defined(x86_64)
               DISABLED_testRobustness,
               #else
               testRobustness,
               #endif
               testCannySupernode
               )
