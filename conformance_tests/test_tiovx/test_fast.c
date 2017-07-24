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
#include <VX/vxu.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "shared_functions.h"

#define MAX_POINTS 100
#define LEVELS_COUNT_MAX    7

static vx_size gaussian_pyramid_calc_max_levels_count(int width, int height, vx_float32 scale)
{
    vx_size level = 1;
    while ((16 <= width) && (16 <= height) && level < LEVELS_COUNT_MAX)
    {
        level++;
        width = (int)ceil((vx_float64)width * scale);
        height = (int)ceil((vx_float64)height * scale);
    }
    return level;
}

static vx_array own_create_keypoint_array(vx_context context, vx_size count, vx_keypoint_t* keypoints)
{
    vx_array arr = 0;

    ASSERT_VX_OBJECT_(return 0, arr = vxCreateArray(context, VX_TYPE_KEYPOINT, count), VX_TYPE_ARRAY);

    VX_CALL_(return 0, vxAddArrayItems(arr, count, keypoints, sizeof(vx_keypoint_t)));

    return arr;
}

static CT_Image optflow_pyrlk_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

static vx_size own_read_keypoints(const char* fileName, vx_keypoint_t** p_old_points, vx_keypoint_t** p_new_points)
{
    size_t sz = 0;
    void* buf = 0;
    char file[MAXPATHLENGTH];

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return 0, (sz < MAXPATHLENGTH));
#if 1
    FILE* f = fopen(file, "rb");
    ASSERT_(return 0, f);
    fseek(f, 0, SEEK_END);

    sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    ASSERT_(return 0, buf = ct_alloc_mem(sz + 1));
    ASSERT_(return 0, sz == fread(buf, 1, sz, f));
    fclose(f); f = NULL;
    ((char*)buf)[sz] = 0;
#else
    sz = ...
    buf = ...
#endif

    ASSERT_(return 0, *p_old_points = ct_alloc_mem(sizeof(vx_keypoint_t) * MAX_POINTS));
    ASSERT_(return 0, *p_new_points = ct_alloc_mem(sizeof(vx_keypoint_t) * MAX_POINTS));

    {
        int num = 0;
        char* pos = buf;
        char* next = 0;
        while(pos && (next = strchr(pos, '\n')))
        {
            int id = 0, status = 0;
            float x1, y1, x2, y2;

            int res;

            *next = 0;
            res = sscanf(pos, "%d %d %g %g %g %g", &id, &status, &x1, &y1, &x2, &y2);
            pos = next + 1;
            if (res == 6)
            {
                (*p_old_points)[num].x = (vx_int32)x1;
                (*p_old_points)[num].y = (vx_int32)y1;
                (*p_old_points)[num].strength = 1;
                (*p_old_points)[num].scale = 0;
                (*p_old_points)[num].orientation = 0;
                (*p_old_points)[num].tracking_status = 1;
                (*p_old_points)[num].error = 0;

                (*p_new_points)[num].x = (vx_int32)x2;
                (*p_new_points)[num].y = (vx_int32)y2;
                (*p_new_points)[num].strength = 1;
                (*p_new_points)[num].scale = 0;
                (*p_new_points)[num].orientation = 0;
                (*p_new_points)[num].tracking_status = status;
                (*p_new_points)[num].error = 0;

                num++;
            }
            else
                break;
        }

        ct_free_mem(buf);

        return num;
    }
}

static void own_keypoints_check(vx_size num_points,
        vx_keypoint_t* old_points, vx_keypoint_t* new_points_ref, vx_keypoint_t* new_points)
{
    vx_size i;
    int num_valid_points = 0;
    int num_lost = 0;
    int num_errors = 0;
    int num_tracked_points = 0;

    for (i = 0; i < num_points; i++)
    {
        vx_int32 dx, dy;
        if (new_points_ref[i].tracking_status == 0)
            continue;
        num_valid_points++;
        if (new_points[i].tracking_status == 0)
        {
            num_lost++;
            continue;
        }
        num_tracked_points++;
        dx = new_points_ref[i].x - new_points[i].x;
        dy = new_points_ref[i].y - new_points[i].y;
        if ((dx * dx + dy * dy) > 2 * 2)
        {
            num_errors++;
        }
    }

    if (num_lost > (int)(num_valid_points * 0.05f))
        CT_ADD_FAILURE("Too many lost points: %d (threshold %d)\n",
                num_lost, (int)(num_valid_points * 0.05f));
    if (num_errors > (int)(num_tracked_points * 0.10f))
        CT_ADD_FAILURE("Too many bad points: %d (threshold %d, both tracked points %d)\n",
                num_errors, (int)(num_tracked_points * 0.10f), num_tracked_points);
}

static const int circle[][2] =
{
    {3, 0}, {3, -1}, {2, -2}, {1, -3}, {0, -3}, {-1, -3}, {-2, -2}, {-3, -1},
    {-3, 0}, {-3, 1}, {-2, 2}, {-1, 3}, {0, 3}, {1, 3}, {2, 2}, {3, 1},
    {3, 0}, {3, -1}, {2, -2}, {1, -3}, {0, -3}, {-1, -3}, {-2, -2}, {-3, -1},
    {-3, 0}, {-3, 1}, {-2, 2}, {-1, 3}, {0, 3}, {1, 3}, {2, 2}, {3, 1},
};

static int check_pt(const uint8_t* ptr, int32_t stride, int t)
{
    int cval = ptr[0];
    int max_up_count = 0, max_lo_count = 0;
    int i, up_count = 0, lo_count = 0;

    for( i = 0; i < 16+9; i++ )
    {
        int val = ptr[circle[i][0] + circle[i][1]*stride];
        if( val > cval + t )
            up_count++;
        else
        {
            max_up_count = CT_MAX(max_up_count, up_count);
            up_count = 0;
        }
        if( val < cval - t )
            lo_count++;
        else
        {
            max_lo_count = CT_MAX(max_lo_count, lo_count);
            lo_count = 0;
        }
    }
    max_up_count = CT_MAX(max_up_count, up_count);
    max_lo_count = CT_MAX(max_lo_count, lo_count);
    return max_up_count >= 9 || max_lo_count >= 9;
}

static uint32_t reference_fast(CT_Image src, CT_Image dst, CT_Image mask, int threshold, int nonmax_suppression)
{
    const int r = 3;
    int x, y, width, height;
    int32_t srcstride, dststride;
    uint32_t ncorners = 0;

    ASSERT_(return 0, src && dst);
    ASSERT_(return 0, src->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_U8);
    ASSERT_(return 0, src->width > 0 && src->height > 0 &&
           src->width == dst->width && src->height == dst->height);
    width = src->width;
    height = src->height;
    srcstride = (int32_t)ct_stride_bytes(src);
    dststride = (int32_t)ct_stride_bytes(dst);
    ct_memset( dst->data.y, 0, (vx_size)dststride*height );

    for( y = r; y < height - r; y++ )
    {
        const uint8_t* srcptr = src->data.y + y*srcstride;
        uint8_t* dstptr = dst->data.y + y*dststride;
        for( x = r; x < width - r; x++ )
        {
            int is_corner = check_pt(srcptr + x, srcstride, threshold);
            int strength = 0;

            if( is_corner )
            {
                // determine the corner strength using binary search
                int a = threshold;
                int b = 255;
                // loop invariant:
                //    1. point is corner with threshold=a
                //    2. point is not a corner with threshold=b
                while( b - a > 1 )
                {
                    int c = (b + a)/2;
                    is_corner = check_pt(srcptr + x, srcstride, c);
                    if( is_corner )
                        a = c;
                    else
                        b = c;
                }
                strength = a;
                ncorners++;
            }
            dstptr[x] = CT_CAST_U8(strength);
        }
    }

    if( nonmax_suppression )
    {
        int32_t maskstride = (int32_t)ct_stride_bytes(mask);

        for( y = r; y < height - r; y++ )
        {
            const uint8_t* dstptr = dst->data.y + y*dststride;
            uint8_t* mptr = mask->data.y + y*maskstride;
            for( x = r; x < width - r; x++ )
            {
                const uint8_t* ptr = dstptr + x;
                int cval = ptr[0];
                mptr[x] = cval >= ptr[-1-dststride] && cval >= ptr[-dststride] && cval >= ptr[-dststride+1] && cval >= ptr[-1] &&
                          cval >  ptr[-1+dststride] && cval >  ptr[ dststride] && cval >  ptr[ dststride+1] && cval >  ptr[ 1];
            }
        }

        ncorners = 0;
        for( y = r; y < height - r; y++ )
        {
            uint8_t* dstptr = dst->data.y + y*dststride;
            const uint8_t* mptr = mask->data.y + y*maskstride;
            for( x = r; x < width - r; x++ )
            {
                if( mptr[x] )
                    ncorners++;
                else
                    dstptr[x] = 0;
            }
        }
    }
    return ncorners;
}

TESTCASE(tivxFastCorners, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    const char* imgname;
    int threshold;
    int nonmax;
    int mode;
} format_arg;

#define MAX_BINS 256

#define FAST_TEST_CASE(imm, imgname, t, nm) \
    {#imm "/" "image=" #imgname "/" "threshold=" #t "/" "nonmax_suppression=" #nm, #imgname ".bmp", t, nm, CT_##imm##_MODE}

TEST_WITH_ARG(tivxFastCorners, testVirtualImages, format_arg,
              FAST_TEST_CASE(Graph, baboon, 10, 0),
              FAST_TEST_CASE(Graph, baboon, 10, 1),
              FAST_TEST_CASE(Graph, baboon, 80, 0),
              FAST_TEST_CASE(Graph, baboon, 80, 1),
              FAST_TEST_CASE(Graph, optflow_00, 10, 0),
              FAST_TEST_CASE(Graph, optflow_00, 10, 1),
              FAST_TEST_CASE(Graph, optflow_00, 80, 0),
              FAST_TEST_CASE(Graph, optflow_00, 80, 1),
              )
{
    int mode = arg_->mode;
    const char* imgname = arg_->imgname;
    int threshold = arg_->threshold;
    int nonmax = arg_->nonmax;
    vx_image src0_image = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    vx_graph graph = 0;
    vx_context context = context_->vx_context_;
    vx_scalar sthresh;
    vx_array corners0, new_points_arr0 = 0, corners1, new_points_arr1 = 0;
    uint32_t width, height;
    vx_float32 threshold_f = (vx_float32)threshold;
    uint32_t ncorners0, ncorners;
    vx_size corners_data_size = 0;
    vx_keypoint_t* corners_data = 0;
    uint32_t i, dst1stride;
    vx_perf_t perf_node1, perf_node2, perf_graph;

    vx_pyramid pyr = 0, src_pyr   = 0;
    vx_float32 eps_val      = 0.001f;
    vx_uint32  num_iter_val = 100;
    vx_bool   use_estimations_val = vx_true_e;
    vx_scalar eps                 = 0;
    vx_scalar num_iter            = 0;
    vx_scalar use_estimations     = 0;
    vx_size   winSize             = 5; // hardcoded from optflow test case
    vx_border_t border = { VX_BORDER_REPLICATE };
    vx_size levels;
    vx_keypoint_t* old_points = 0;
    vx_keypoint_t* new_points_ref = 0;
    vx_keypoint_t* new_points0 = 0;
    vx_keypoint_t* new_points1 = 0;
    vx_size new_points_size0 = 0, new_points_size1 = 0;

    CT_Image src_ct_image = NULL;
    CT_Image src0, dst0, mask0, dst1;

    ASSERT_NO_FAILURE(src0 = ct_read_image(imgname, 1));
    ASSERT(src0->format == VX_DF_IMAGE_U8);

    width = src0->width;
    height = src0->height;

    levels = gaussian_pyramid_calc_max_levels_count(width, height, VX_SCALE_PYRAMID_HALF);

    ASSERT_NO_FAILURE(src_ct_image = optflow_pyrlk_read_image( "optflow_01.bmp", 0, 0));

    ASSERT_VX_OBJECT(src_pyr = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_VX_OBJECT(new_points_arr0 = vxCreateArray(context, VX_TYPE_KEYPOINT, 80000), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(new_points_arr1 = vxCreateArray(context, VX_TYPE_KEYPOINT, 80000), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(eps             = vxCreateScalar(context, VX_TYPE_FLOAT32, &eps_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_iter        = vxCreateScalar(context, VX_TYPE_UINT32, &num_iter_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(use_estimations = vxCreateScalar(context, VX_TYPE_BOOL, &use_estimations_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_NO_FAILURE(tivx_gaussian_pyramid_fill_reference(src_ct_image, src_pyr, levels, VX_SCALE_PYRAMID_HALF, border));

    ASSERT_NO_FAILURE(dst0 = ct_allocate_image(width, height, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(mask0 = ct_allocate_image(width, height, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(dst1 = ct_allocate_image(width, height, VX_DF_IMAGE_U8));
    dst1stride = ct_stride_bytes(dst1);
    ct_memset(dst1->data.y, 0, (vx_size)dst1stride*height);

    ncorners0 = reference_fast(src0, dst0, mask0, threshold, nonmax);

    src0_image = ct_image_to_vx_image(src0, context);
    sthresh = vxCreateScalar(context, VX_TYPE_FLOAT32, &threshold_f);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(corners0 = vxCreateVirtualArray(graph, VX_TYPE_KEYPOINT, 80000), VX_TYPE_ARRAY);
    corners1 = vxCreateArray(context, VX_TYPE_KEYPOINT, 80000);
    node1 = vxFastCornersNode(graph, src0_image, sthresh, nonmax ? vx_true_e : vx_false_e, corners0, 0);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxOpticalFlowPyrLKNode(
        graph,
        pyr, src_pyr,
        corners0, corners0, new_points_arr0,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    node3 = vxFastCornersNode(graph, src0_image, sthresh, nonmax ? vx_true_e : vx_false_e, corners1, 0);
    ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node4 = vxOpticalFlowPyrLKNode(
        graph,
        pyr, src_pyr,
        corners1, corners1, new_points_arr1,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4, VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node4));
    ASSERT(node4 == 0);
    VX_CALL(vxReleaseNode(&node3));
    ASSERT(node3 == 0);
    VX_CALL(vxReleaseNode(&node2));
    ASSERT(node2 == 0);
    VX_CALL(vxReleaseScalar(&eps));
    ASSERT(eps == 0);
    VX_CALL(vxReleaseScalar(&num_iter));
    ASSERT(num_iter == 0);
    VX_CALL(vxReleaseScalar(&use_estimations));
    ASSERT(use_estimations == 0);
    VX_CALL(vxReleasePyramid(&src_pyr));
    ASSERT(src_pyr == 0);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleasePyramid(&pyr));
    ASSERT(pyr == 0);
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node1 == 0);
    ASSERT(graph == 0);
    VX_CALL(vxReleaseImage(&src0_image));
    ASSERT(src0_image == 0);
    VX_CALL(vxReleaseScalar(&sthresh));
    ASSERT(sthresh == 0);
    ct_read_array(corners1, (void**)&corners_data, 0, &corners_data_size, 0);
    VX_CALL(vxReleaseArray(&corners1));
    VX_CALL(vxReleaseArray(&corners0));
    ASSERT(corners0 == 0 && corners1 == 0);
    ncorners = (uint32_t)corners_data_size;

    for( i = 0; i < ncorners; i++ )
    {
        vx_keypoint_t* pt = &corners_data[i];
        int ix, iy;
        ASSERT( 0.f <= pt->x && pt->x < (float)width &&
                0.f <= pt->y && pt->y < (float)height );
        ASSERT(pt->tracking_status == 1);
        ix = (int)(pt->x + 0.5f);
        iy = (int)(pt->y + 0.5f);
        ix = CT_MIN(ix, (int)width-1);
        iy = CT_MIN(iy, (int)height-1);
        ASSERT( !nonmax || (0 < pt->strength && pt->strength <= 255) );
        dst1->data.y[dst1stride*iy + ix] = nonmax ? (uint8_t)(pt->strength + 0.5f) : 1;
    }

    {
    const uint32_t border = 3;
    int32_t stride0 = (int32_t)ct_stride_bytes(dst0), stride1 = (int32_t)ct_stride_bytes(dst1);
    uint32_t x, y;
    uint32_t missing0 = 0, missing1 = 0;

    for( y = border; y < height - border; y++ )
    {
        const uint8_t* ptr0 = dst0->data.y + stride0*y;
        const uint8_t* ptr1 = dst1->data.y + stride1*y;

        for( x = border; x < width - border; x++ )
        {
            if( ptr0[x] > 0 && ptr1[x] == 0 )
                missing0++;
            else if( ptr0[x] == 0 && ptr1[x] > 0 )
                missing1++;
            else if( nonmax && ptr0[x] > 0 && ptr1[x] > 0 && fabs(log10((double)ptr0[x]/ptr1[x])) >= 1 )
            {
                missing0++;
                missing1++;
            }
        }
    }

    ASSERT( missing0 <= 0.02*ncorners0 );
    ASSERT( missing1 <= 0.02*ncorners );
    }

    ASSERT(VX_TYPE_KEYPOINT == ct_read_array(new_points_arr1, (void**)&new_points1, 0, &new_points_size1, 0));
    ASSERT(new_points_size1 == corners_data_size);

    ASSERT(VX_TYPE_KEYPOINT == ct_read_array(new_points_arr0, (void**)&new_points0, 0, &new_points_size0, 0));
    ASSERT(new_points_size0 == corners_data_size);

    ASSERT_NO_FAILURE(own_keypoints_check(new_points_size0, NULL, new_points0, new_points1));

    ct_free_mem(corners_data);
    ct_free_mem(new_points1);
    ct_free_mem(new_points0);
    ct_free_mem(new_points_ref);
    ct_free_mem(old_points);

    VX_CALL(vxReleaseArray(&new_points_arr0));
    ASSERT(new_points_arr0 == 0);
    VX_CALL(vxReleaseArray(&new_points_arr1));
    ASSERT(new_points_arr1 == 0);

    printPerformance(perf_node1, width*height, "N1");
    printPerformance(perf_node2, width*height, "N2");
    printPerformance(perf_graph, width*height, "G1");
}

TEST_WITH_ARG(tivxFastCorners, testMultipleNodes, format_arg,
              FAST_TEST_CASE(Graph, lena, 10, 0),
              FAST_TEST_CASE(Graph, lena, 10, 1),
              FAST_TEST_CASE(Graph, lena, 80, 0),
              FAST_TEST_CASE(Graph, lena, 80, 1),
              )
{
    int mode = arg_->mode;
    const char* imgname = arg_->imgname;
    int threshold = arg_->threshold;
    int nonmax = arg_->nonmax;
    vx_image src0_image, src1_image;
    vx_node node1 = 0, node2 = 0;
    vx_graph graph = 0;
    CT_Image src0, src1, dst0, dst0_ref, mask0, dst1, dst1_ref, mask1;
    vx_context context = context_->vx_context_;
    vx_scalar sthresh, num_corners_scalar0, num_corners_scalar1;
    vx_array corners0, corners1;
    uint32_t width0, height0, width1, height1;
    vx_float32 threshold_f = (vx_float32)threshold;
    uint32_t ncorners0, ncorners1, ncorners0_ref, ncorners1_ref;
    vx_size corners0_data_size = 0, corners1_data_size = 0;
    vx_keypoint_t* corners0_data = 0;
    vx_keypoint_t* corners1_data = 0;
    uint32_t i, dst0stride, dst1stride;
    vx_size num_corners = 0;
    int scalar_corners0, scalar_corners1;
    vx_perf_t perf_node1, perf_node2, perf_graph;

    ASSERT_NO_FAILURE(src0 = ct_read_image("lena.bmp", 1));
    ASSERT(src0->format == VX_DF_IMAGE_U8);

    width0 = src0->width;
    height0 = src0->height;

    ASSERT_NO_FAILURE(src1 = ct_read_image("baboon.bmp", 1));
    ASSERT(src1->format == VX_DF_IMAGE_U8);

    width1 = src1->width;
    height1 = src1->height;

    ASSERT_NO_FAILURE(dst0 = ct_allocate_image(width0, height0, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(mask0 = ct_allocate_image(width0, height0, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(dst0_ref = ct_allocate_image(width0, height0, VX_DF_IMAGE_U8));
    dst0stride = ct_stride_bytes(dst0_ref);
    ct_memset(dst0_ref->data.y, 0, (vx_size)dst0stride*height0);

    ASSERT_NO_FAILURE(dst1 = ct_allocate_image(width1, height1, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(mask1 = ct_allocate_image(width1, height1, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(dst1_ref = ct_allocate_image(width1, height1, VX_DF_IMAGE_U8));
    dst1stride = ct_stride_bytes(dst1_ref);
    ct_memset(dst1_ref->data.y, 0, (vx_size)dst1stride*height1);

    ncorners0 = reference_fast(src0, dst0, mask0, threshold, nonmax);

    ncorners1 = reference_fast(src1, dst1, mask1, threshold, nonmax);

    src0_image = ct_image_to_vx_image(src0, context);
    src1_image = ct_image_to_vx_image(src1, context);
    sthresh = vxCreateScalar(context, VX_TYPE_FLOAT32, &threshold_f);
    corners0 = vxCreateArray(context, VX_TYPE_KEYPOINT, 80000);
    corners1 = vxCreateArray(context, VX_TYPE_KEYPOINT, 80000);
    ASSERT_VX_OBJECT(num_corners_scalar0 = vxCreateScalar(context, VX_TYPE_SIZE, &num_corners), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_corners_scalar1 = vxCreateScalar(context, VX_TYPE_SIZE, &num_corners), VX_TYPE_SCALAR);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    node1 = vxFastCornersNode(graph, src0_image, sthresh, nonmax ? vx_true_e : vx_false_e, corners0, num_corners_scalar0);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
    node2 = vxFastCornersNode(graph, src1_image, sthresh, nonmax ? vx_true_e : vx_false_e, corners1, num_corners_scalar1);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);
    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node1 == 0 && node2 == 0 && graph == 0);

    VX_CALL(vxReleaseImage(&src0_image));
    VX_CALL(vxReleaseImage(&src1_image));
    VX_CALL(vxReleaseScalar(&sthresh));

    ct_read_array(corners0, (void**)&corners0_data, 0, &corners0_data_size, 0);
    ct_read_array(corners1, (void**)&corners1_data, 0, &corners1_data_size, 0);
    VX_CALL(vxReleaseArray(&corners0));
    VX_CALL(vxReleaseArray(&corners1));
    ncorners0_ref = (uint32_t)corners0_data_size;
    ncorners1_ref = (uint32_t)corners1_data_size;

    scalar_corners0 = ct_scalar_as_int(num_corners_scalar0);
    scalar_corners1 = ct_scalar_as_int(num_corners_scalar1);
    ASSERT( scalar_corners0 == ncorners0_ref );
    ASSERT( scalar_corners1 == ncorners1_ref );
    VX_CALL(vxReleaseScalar(&num_corners_scalar1));
    VX_CALL(vxReleaseScalar(&num_corners_scalar0));

    for( i = 0; i < ncorners0_ref; i++ )
    {
        vx_keypoint_t* pt = &corners0_data[i];
        int ix, iy;
        ASSERT( 0.f <= pt->x && pt->x < (float)width0 &&
                0.f <= pt->y && pt->y < (float)height0 );
        ASSERT(pt->tracking_status == 1);
        ix = (int)(pt->x + 0.5f);
        iy = (int)(pt->y + 0.5f);
        ix = CT_MIN(ix, (int)width0-1);
        iy = CT_MIN(iy, (int)height0-1);
        ASSERT( !nonmax || (0 < pt->strength && pt->strength <= 255) );
        dst0_ref->data.y[dst0stride*iy + ix] = nonmax ? (uint8_t)(pt->strength + 0.5f) : 1;
    }

    ct_free_mem(corners0_data);

    {
    const uint32_t border = 3;
    int32_t stride0 = (int32_t)ct_stride_bytes(dst0), stride1 = (int32_t)ct_stride_bytes(dst0_ref);
    uint32_t x, y;
    uint32_t missing0 = 0, missing1 = 0;

    for( y = border; y < height0 - border; y++ )
    {
        const uint8_t* ptr0 = dst0->data.y + stride0*y;
        const uint8_t* ptr1 = dst0_ref->data.y + stride1*y;

        for( x = border; x < width0 - border; x++ )
        {
            if( ptr0[x] > 0 && ptr1[x] == 0 )
                missing0++;
            else if( ptr0[x] == 0 && ptr1[x] > 0 )
                missing1++;
            else if( nonmax && ptr0[x] > 0 && ptr1[x] > 0 && fabs(log10((double)ptr0[x]/ptr1[x])) >= 1 )
            {
                missing0++;
                missing1++;
            }
        }
    }

    ASSERT( missing0 <= 0.02*ncorners0 );
    ASSERT( missing1 <= 0.02*ncorners0_ref );
    }

    for( i = 0; i < ncorners1_ref; i++ )
    {
        vx_keypoint_t* pt = &corners1_data[i];
        int ix, iy;
        ASSERT( 0.f <= pt->x && pt->x < (float)width1 &&
                0.f <= pt->y && pt->y < (float)height1 );
        ASSERT(pt->tracking_status == 1);
        ix = (int)(pt->x + 0.5f);
        iy = (int)(pt->y + 0.5f);
        ix = CT_MIN(ix, (int)width1-1);
        iy = CT_MIN(iy, (int)height1-1);
        ASSERT( !nonmax || (0 < pt->strength && pt->strength <= 255) );
        dst1_ref->data.y[dst1stride*iy + ix] = nonmax ? (uint8_t)(pt->strength + 0.5f) : 1;
    }

    ct_free_mem(corners1_data);

    {
    const uint32_t border = 3;
    int32_t stride0 = (int32_t)ct_stride_bytes(dst1), stride1 = (int32_t)ct_stride_bytes(dst1_ref);
    uint32_t x, y;
    uint32_t missing0 = 0, missing1 = 0;

    for( y = border; y < height1 - border; y++ )
    {
        const uint8_t* ptr0 = dst1->data.y + stride0*y;
        const uint8_t* ptr1 = dst1_ref->data.y + stride1*y;

        for( x = border; x < width1 - border; x++ )
        {
            if( ptr0[x] > 0 && ptr1[x] == 0 )
                missing0++;
            else if( ptr0[x] == 0 && ptr1[x] > 0 )
                missing1++;
            else if( nonmax && ptr0[x] > 0 && ptr1[x] > 0 && fabs(log10((double)ptr0[x]/ptr1[x])) >= 1 )
            {
                missing0++;
                missing1++;
            }
        }
    }

    ASSERT( missing0 <= 0.02*ncorners1 );
    ASSERT( missing1 <= 0.02*ncorners1_ref );
    }

    printPerformance(perf_node1, width0*height0, "N1");
    printPerformance(perf_node2, width1*height1, "N2");
    printPerformance(perf_graph, width0*height0, "G1");
}

TEST_WITH_ARG(tivxFastCorners, testOptionalParameters, format_arg,
              FAST_TEST_CASE(Graph, lena, 10, 0),
              FAST_TEST_CASE(Graph, lena, 10, 1),
              FAST_TEST_CASE(Graph, lena, 80, 0),
              FAST_TEST_CASE(Graph, lena, 80, 1),
              FAST_TEST_CASE(Graph, baboon, 10, 0),
              FAST_TEST_CASE(Graph, baboon, 10, 1),
              FAST_TEST_CASE(Graph, baboon, 80, 0),
              FAST_TEST_CASE(Graph, baboon, 80, 1),
              FAST_TEST_CASE(Graph, optflow_00, 10, 0),
              FAST_TEST_CASE(Graph, optflow_00, 10, 1),
              FAST_TEST_CASE(Graph, optflow_00, 80, 0),
              FAST_TEST_CASE(Graph, optflow_00, 80, 1),
              )
{
    int mode = arg_->mode;
    const char* imgname = arg_->imgname;
    int threshold = arg_->threshold;
    int nonmax = arg_->nonmax;
    vx_image src;
    vx_size num_corners = 0;
    vx_node node = 0;
    vx_graph graph = 0;
    CT_Image src0, dst0, mask0, dst1;
    vx_context context = context_->vx_context_;
    vx_scalar sthresh;
    vx_array corners;
    uint32_t width, height;
    vx_float32 threshold_f = (vx_float32)threshold;
    uint32_t ncorners0, ncorners;
    vx_size corners_data_size = 0;
    vx_keypoint_t* corners_data = 0;
    uint32_t i, dst1stride;
    vx_perf_t perf_node1, perf_graph;
    vx_rectangle_t src_rect;
    vx_bool valid_rect;

    ASSERT_NO_FAILURE(src0 = ct_read_image(imgname, 1));
    ASSERT(src0->format == VX_DF_IMAGE_U8);

    width = src0->width;
    height = src0->height;

    ASSERT_NO_FAILURE(dst0 = ct_allocate_image(width, height, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(mask0 = ct_allocate_image(width, height, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(dst1 = ct_allocate_image(width, height, VX_DF_IMAGE_U8));
    dst1stride = ct_stride_bytes(dst1);
    ct_memset(dst1->data.y, 0, (vx_size)dst1stride*height);

    ncorners0 = reference_fast(src0, dst0, mask0, threshold, nonmax);

    src = ct_image_to_vx_image(src0, context);
    sthresh = vxCreateScalar(context, VX_TYPE_FLOAT32, &threshold_f);
    corners = vxCreateArray(context, VX_TYPE_KEYPOINT, 80000);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    node = vxFastCornersNode(graph, src, sthresh, nonmax ? vx_true_e : vx_false_e, corners, NULL);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);
    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src, &src_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), height);

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseScalar(&sthresh));
    ct_read_array(corners, (void**)&corners_data, 0, &corners_data_size, 0);
    VX_CALL(vxReleaseArray(&corners));
    ncorners = (uint32_t)corners_data_size;

    for( i = 0; i < ncorners; i++ )
    {
        vx_keypoint_t* pt = &corners_data[i];
        int ix, iy;
        ASSERT( 0.f <= pt->x && pt->x < (float)width &&
                0.f <= pt->y && pt->y < (float)height );
        ASSERT(pt->tracking_status == 1);
        ix = (int)(pt->x + 0.5f);
        iy = (int)(pt->y + 0.5f);
        ix = CT_MIN(ix, (int)width-1);
        iy = CT_MIN(iy, (int)height-1);
        ASSERT( !nonmax || (0 < pt->strength && pt->strength <= 255) );
        dst1->data.y[dst1stride*iy + ix] = nonmax ? (uint8_t)(pt->strength + 0.5f) : 1;
    }

    ct_free_mem(corners_data);

    {
    const uint32_t border = 3;
    int32_t stride0 = (int32_t)ct_stride_bytes(dst0), stride1 = (int32_t)ct_stride_bytes(dst1);
    uint32_t x, y;
    uint32_t missing0 = 0, missing1 = 0;

    for( y = border; y < height - border; y++ )
    {
        const uint8_t* ptr0 = dst0->data.y + stride0*y;
        const uint8_t* ptr1 = dst1->data.y + stride1*y;

        for( x = border; x < width - border; x++ )
        {
            if( ptr0[x] > 0 && ptr1[x] == 0 )
                missing0++;
            else if( ptr0[x] == 0 && ptr1[x] > 0 )
                missing1++;
            else if( nonmax && ptr0[x] > 0 && ptr1[x] > 0 && fabs(log10((double)ptr0[x]/ptr1[x])) >= 1 )
            {
                missing0++;
                missing1++;
            }
        }
    }

    ASSERT( missing0 <= 0.02*ncorners0 );
    ASSERT( missing1 <= 0.02*ncorners );
    }

    printPerformance(perf_node1, width*height, "N1");
    printPerformance(perf_graph, width*height, "G1");
}

TEST_WITH_ARG(tivxFastCorners, negativeTestBorderMode, format_arg,
              FAST_TEST_CASE(Graph, lena, 10, 0),
              FAST_TEST_CASE(Graph, lena, 10, 1),
              )
{
    int mode = arg_->mode;
    const char* imgname = arg_->imgname;
    int threshold = arg_->threshold;
    int nonmax = arg_->nonmax;
    vx_image src;
    vx_size num_corners = 0;
    vx_node node = 0;
    vx_graph graph = 0;
    CT_Image src0, dst0, mask0, dst1;
    vx_context context = context_->vx_context_;
    vx_scalar sthresh;
    vx_array corners;
    uint32_t width, height;
    vx_float32 threshold_f = (vx_float32)threshold;
    uint32_t ncorners0, ncorners;
    vx_size corners_data_size = 0;
    vx_keypoint_t* corners_data = 0;
    uint32_t i, dst1stride;
    vx_border_t border = { VX_BORDER_REPLICATE };

    if (nonmax == 1)
        border.mode = VX_BORDER_CONSTANT;

    ASSERT_NO_FAILURE(src0 = ct_read_image(imgname, 1));
    ASSERT(src0->format == VX_DF_IMAGE_U8);

    width = src0->width;
    height = src0->height;

    ASSERT_NO_FAILURE(dst0 = ct_allocate_image(width, height, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(mask0 = ct_allocate_image(width, height, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(dst1 = ct_allocate_image(width, height, VX_DF_IMAGE_U8));
    dst1stride = ct_stride_bytes(dst1);
    ct_memset(dst1->data.y, 0, (vx_size)dst1stride*height);

    ncorners0 = reference_fast(src0, dst0, mask0, threshold, nonmax);

    src = ct_image_to_vx_image(src0, context);
    sthresh = vxCreateScalar(context, VX_TYPE_FLOAT32, &threshold_f);
    corners = vxCreateArray(context, VX_TYPE_KEYPOINT, 80000);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    node = vxFastCornersNode(graph, src, sthresh, nonmax ? vx_true_e : vx_false_e, corners, NULL);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
    ASSERT_EQ_VX_STATUS(vxVerifyGraph(graph), VX_ERROR_NOT_SUPPORTED);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseScalar(&sthresh));
    VX_CALL(vxReleaseArray(&corners));
}

TESTCASE_TESTS(tivxFastCorners, testVirtualImages, testMultipleNodes, testOptionalParameters, negativeTestBorderMode)
