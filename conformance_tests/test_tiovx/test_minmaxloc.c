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
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "shared_functions.h"

#define MAX_NODES 10

typedef vx_coordinates2d_t Point;

static void reference_minmaxloc(CT_Image src, int* _minval, int* _maxval,
                                uint32_t* _mincount, uint32_t* _maxcount)
{
    Point pt={0, 0};
    int minval = INT_MAX, maxval = INT_MIN;
    int format = src ? src->format : VX_DF_IMAGE_U8;
    uint32_t mincount = 0, maxcount = 0, stride;

    ASSERT(src);
    ASSERT(src->width > 0 && src->height > 0);
    stride = ct_stride_bytes(src);

#define CASE_MINMAXLOC(format, type) \
    case format: \
    for( pt.y = 0; pt.y < src->height; pt.y++ ) \
    { \
        const type* ptr = (const type*)(src->data.y + stride*pt.y); \
        for( pt.x = 0; pt.x < src->width; pt.x++ ) \
        { \
            int val = ptr[pt.x]; \
            if( val <= minval ) \
            { \
                if(val < minval) \
                { \
                    minval = val; \
                    mincount = 0; \
                } \
                mincount++; \
            } \
            if( val >= maxval ) \
            { \
                if(val > maxval) \
                { \
                    maxval = val; \
                    maxcount = 0; \
                } \
                maxcount++; \
            } \
        } \
    } \
    break

    switch(format)
    {
    CASE_MINMAXLOC(VX_DF_IMAGE_U8, uint8_t);
    CASE_MINMAXLOC(VX_DF_IMAGE_S16, int16_t);
    CASE_MINMAXLOC(VX_DF_IMAGE_S32, int32_t);
    default:
        FAIL("Unsupported image format: (%d)", &src->format);
    }

    *_minval = minval;
    *_maxval = maxval;
    if(_mincount)
        *_mincount = mincount;
    if(_maxcount)
        *_maxcount = maxcount;
}

static void reference_minmax(CT_Image src, int* _minval, int* _maxval)
{
    reference_minmaxloc(src, _minval, _maxval, 0, 0);
}

static int cmp_pt(const void* a, const void* b)
{
    const Point* pa = (const Point*)a;
    const Point* pb = (const Point*)b;
    int d = pa->y - pb->y;
    return d ? d : (int)(pa->x - pb->x);
}

static void ct_sort_points(Point* ptbuf, vx_size npoints)
{
    qsort(ptbuf, npoints, sizeof(ptbuf[0]), cmp_pt);
}

static void ct_set_random_pixels(CT_Image image, uint64_t* rng, int where_count, int what_count, const int* valarr)
{
    int format = image->format, i;
    uint32_t stride = ct_stride_bytes(image);

    #define CASE_SET_RANDOM(format, type, cast_macro) \
    case format: \
        for( i = 0; i < where_count; i++) \
        { \
            int y = CT_RNG_NEXT_INT(*rng, 0, image->height); \
            int x = CT_RNG_NEXT_INT(*rng, 0, image->width); \
            int k = CT_RNG_NEXT_INT(*rng, 0, what_count); \
            int val = valarr[k]; \
            ((type*)(image->data.y + stride*y))[x] = cast_macro(val); \
        } \
        break

    switch(format)
    {
    CASE_SET_RANDOM(VX_DF_IMAGE_U8, uint8_t, CT_CAST_U8);
    CASE_SET_RANDOM(VX_DF_IMAGE_U16, uint16_t, CT_CAST_U16);
    CASE_SET_RANDOM(VX_DF_IMAGE_S16, int16_t, CT_CAST_S16);
    CASE_SET_RANDOM(VX_DF_IMAGE_S32, int32_t, CT_CAST_S32);
    default:
        CT_ADD_FAILURE("unsupported image format %d", format);
    }
}

TESTCASE(tivxMinMaxLoc, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    int mode;
    vx_df_image format;
} format_arg;

#define MINMAXLOC_TEST_CASE(imm, tp) \
    {#imm "/" #tp, CT_##imm##_MODE, VX_DF_IMAGE_##tp}

TEST_WITH_ARG(tivxMinMaxLoc, testOnRandom, format_arg,
              MINMAXLOC_TEST_CASE(Graph, U8),
              MINMAXLOC_TEST_CASE(Graph, S16),
              )
{
    const int MAX_CAP = 300;
    int format = arg_->format;
    int mode = arg_->mode;
    vx_image src0_image, src1_image;
    CT_Image src0, src1;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;
    vx_context context = context_->vx_context_;
    int iter, k, niters = 3;
    uint64_t rng;
    int a, b;
    int minval0 = 0, maxval0 = 0, minval0_test = 0, maxval0_test = 0;
    uint32_t mincount0 = 0, maxcount0 = 0, mincount0_test = 0, maxcount0_test = 0;
    int minval1 = 0, maxval1 = 0, minval1_test = 0, maxval1_test = 0;
    uint32_t mincount1 = 0, maxcount1 = 0, mincount1_test = 0, maxcount1_test = 0;
    vx_scalar minval0_, maxval0_, mincount0_, maxcount0_;
    vx_array minloc0_ = 0, maxloc0_ = 0;
    vx_scalar minval1_, maxval1_, mincount1_, maxcount1_;
    vx_array minloc1_ = 0, maxloc1_ = 0;
    vx_enum sctype = format == VX_DF_IMAGE_U8 ? VX_TYPE_UINT8 :
                     format == VX_DF_IMAGE_S16 ? VX_TYPE_INT16 :
                     VX_TYPE_INT32;
    uint32_t pixsize = ct_image_bits_per_pixel(format)/8;
    Point* ptbuf = 0;
    vx_size bufbytes = 0, npoints = 0, bufcap = 0;
    vx_rectangle_t src_rect;
    vx_bool valid_rect;

    if( format == VX_DF_IMAGE_U8 )
        a = 0, b = 256;
    else if( format == VX_DF_IMAGE_S16 )
        a = -32768, b = 32768;
    else
        a = INT_MIN/3, b = INT_MAX/3;

    minval0_ = ct_scalar_from_int(context, sctype, 0);
    maxval0_ = ct_scalar_from_int(context, sctype, 0);
    mincount0_ = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    maxcount0_ = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    minloc0_ = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    maxloc0_ = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    ASSERT(vxGetStatus((vx_reference)minloc0_) == VX_SUCCESS && vxGetStatus((vx_reference)maxloc0_) == VX_SUCCESS);

    minval1_ = ct_scalar_from_int(context, sctype, 0);
    maxval1_ = ct_scalar_from_int(context, sctype, 0);
    mincount1_ = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    maxcount1_ = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    minloc1_ = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    maxloc1_ = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    ASSERT(vxGetStatus((vx_reference)minloc1_) == VX_SUCCESS && vxGetStatus((vx_reference)maxloc1_) == VX_SUCCESS);

    rng = CT()->seed_;

    for( iter = 0; iter < niters; iter++ )
    {
        int return_loc = CT_RNG_NEXT_INT(rng, 0, 2);
        int return_count = CT_RNG_NEXT_INT(rng, 0, 2);
        uint32_t stride0, stride1;
        int width, height;

        width = ct_roundf(ct_log_rng(&rng, 0, 10));
        height = ct_roundf(ct_log_rng(&rng, 0, 10));

        width = CT_MAX(width, 1);
        height = CT_MAX(height, 1);

        src0 = ct_allocate_ct_image_random(width, height, format, &rng, a, b);
        stride0 = ct_stride_bytes(src0);
        src1 = ct_allocate_ct_image_random(width, height, format, &rng, a, b);
        stride1 = ct_stride_bytes(src1);
        if( iter % 3 == 0 )
        {
            int mm[2], maxk;
            reference_minmax(src0, &mm[0], &mm[1]);
            maxk = CT_RNG_NEXT_INT(rng, 0, 100);
            // make sure that there are several pixels with minimum/maximum value
            ct_set_random_pixels(src0, &rng, maxk, 2, mm);
        }
        reference_minmaxloc(src0, &minval0, &maxval0, &mincount0, &maxcount0);
        src0_image = ct_image_to_vx_image(src0, context);
        reference_minmaxloc(src1, &minval1, &maxval1, &mincount1, &maxcount1);
        src1_image = ct_image_to_vx_image(src1, context);

        graph = vxCreateGraph(context);
        ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
        node1 = vxMinMaxLocNode(graph, src0_image, minval0_, maxval0_,
                                   return_loc ? minloc0_ : 0,
                                   return_loc ? maxloc0_ : 0,
                                   return_count ? mincount0_ : 0,
                                   return_count ? maxcount0_ : 0);
        ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
        node2 = vxMinMaxLocNode(graph, src1_image, minval1_, maxval1_,
                                   return_loc ? minloc1_ : 0,
                                   return_loc ? maxloc1_ : 0,
                                   return_count ? mincount1_ : 0,
                                   return_count ? maxcount1_ : 0);
        ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
        ASSERT_EQ_INT(valid_rect, vx_false_e);

        vxGetValidRegionImage(src0_image, &src_rect);

        ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), width);
        ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), height);

        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

        minval0_test = ct_scalar_as_int(minval0_);
        minval1_test = ct_scalar_as_int(minval1_);
        maxval0_test = ct_scalar_as_int(maxval0_);
        maxval1_test = ct_scalar_as_int(maxval1_);
        if( return_count )
        {
            mincount0_test = ct_scalar_as_int(mincount0_);
            mincount1_test = ct_scalar_as_int(mincount1_);
            maxcount0_test = ct_scalar_as_int(maxcount0_);
            maxcount1_test = ct_scalar_as_int(maxcount1_);
        }
        else
        {
            mincount0_test = mincount0;
            mincount1_test = mincount1;
            maxcount0_test = maxcount0;
            maxcount1_test = maxcount1;
        }

        if( minval0_test != minval0 || maxval0_test != maxval0 || mincount0_test != mincount0 || maxcount0_test != maxcount0 )
        {
            CT_RecordFailureAtFormat("Test case %d, first image. width=%d, height=%d,\n"
                                     "\tExpected: minval=%d, maxval=%d, mincount=%d, maxcount=%d\n"
                                     "\tActual:   minval=%d, maxval=%d, mincount=%d, maxcount=%d\n",
                                     __FUNCTION__, __FILE__, __LINE__,
                                     iter, width, height,
                                     minval0, maxval0, mincount0, maxcount0,
                                     minval0_test, maxval0_test, mincount0_test, maxcount0_test);
            break;
        }

        if( minval1_test != minval1 || maxval1_test != maxval1 || mincount1_test != mincount1 || maxcount1_test != maxcount1 )
        {
            CT_RecordFailureAtFormat("Test case %d, second image. width=%d, height=%d,\n"
                                     "\tExpected: minval=%d, maxval=%d, mincount=%d, maxcount=%d\n"
                                     "\tActual:   minval=%d, maxval=%d, mincount=%d, maxcount=%d\n",
                                     __FUNCTION__, __FILE__, __LINE__,
                                     iter, width, height,
                                     minval1, maxval1, mincount1, maxcount1,
                                     minval1_test, maxval1_test, mincount1_test, maxcount1_test);
            break;
        }

        if( return_loc )
        {
            uint8_t* roi_ptr = src0->data.y;
            for( k = 0; k < 2; k++ )
            {
                int val0 = k == 0 ? minval0_test : maxval0_test;
                uint32_t i, count = k == 0 ? mincount0_test : maxcount0_test;
                vx_array loc = k == 0 ? minloc0_ : maxloc0_;
                vx_enum tp;
                union
                {
                    uint8_t u8;
                    int16_t s16;
                    int32_t s32;
                }
                uval;
                if( format == VX_DF_IMAGE_U8 )
                    uval.u8 = (uint8_t)val0;
                else if( format == VX_DF_IMAGE_S16 )
                    uval.s16 = (int16_t)val0;
                else
                    uval.s32 = (int32_t)val0;

                tp = ct_read_array(loc, (void**)&ptbuf, &bufbytes, &npoints, &bufcap);
                ASSERT(tp == VX_TYPE_COORDINATES2D);
                ASSERT(npoints == CT_MIN(bufcap, (vx_size)count));

                ct_sort_points(ptbuf, npoints);
                for( i = 0; i < npoints; i++ )
                {
                    Point p = ptbuf[i];
                    if( i > 0 )
                    {
                        // all the extrema locations should be different
                        ASSERT(p.x != ptbuf[i-1].x || p.y != ptbuf[i-1].y);
                    }
                    // value at each extrema location should match the extremum value
                    ASSERT(memcmp(roi_ptr + p.y*stride0 + p.x*pixsize, &uval, pixsize) == 0);
                }
            }
        }

        if( return_loc )
        {
            uint8_t* roi_ptr = src1->data.y;
            for( k = 0; k < 2; k++ )
            {
                int val0 = k == 0 ? minval1_test : maxval1_test;
                uint32_t i, count = k == 0 ? mincount1_test : maxcount1_test;
                vx_array loc = k == 0 ? minloc1_ : maxloc1_;
                vx_enum tp;
                union
                {
                    uint8_t u8;
                    int16_t s16;
                    int32_t s32;
                }
                uval;
                if( format == VX_DF_IMAGE_U8 )
                    uval.u8 = (uint8_t)val0;
                else if( format == VX_DF_IMAGE_S16 )
                    uval.s16 = (int16_t)val0;
                else
                    uval.s32 = (int32_t)val0;

                tp = ct_read_array(loc, (void**)&ptbuf, &bufbytes, &npoints, &bufcap);
                ASSERT(tp == VX_TYPE_COORDINATES2D);
                ASSERT(npoints == CT_MIN(bufcap, (vx_size)count));

                ct_sort_points(ptbuf, npoints);
                for( i = 0; i < npoints; i++ )
                {
                    Point p = ptbuf[i];
                    if( i > 0 )
                    {
                        // all the extrema locations should be different
                        ASSERT(p.x != ptbuf[i-1].x || p.y != ptbuf[i-1].y);
                    }
                    // value at each extrema location should match the extremum value
                    ASSERT(memcmp(roi_ptr + p.y*stride1 + p.x*pixsize, &uval, pixsize) == 0);
                }
            }
        }

        VX_CALL(vxReleaseImage(&src0_image));
        VX_CALL(vxReleaseImage(&src1_image));
        VX_CALL(vxReleaseNode(&node1));
        VX_CALL(vxReleaseNode(&node2));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node1 == 0 && node2 == 0 && graph == 0);
        CT_CollectGarbage(CT_GC_IMAGE);

        printPerformance(perf_node1, width*height, "N1");
        printPerformance(perf_node2, width*height, "N2");
        printPerformance(perf_graph, width*height, "G1");
    }

    VX_CALL(vxReleaseScalar(&minval0_));
    VX_CALL(vxReleaseScalar(&maxval0_));
    VX_CALL(vxReleaseScalar(&mincount0_));
    VX_CALL(vxReleaseScalar(&maxcount0_));
    VX_CALL(vxReleaseArray(&minloc0_));
    VX_CALL(vxReleaseArray(&maxloc0_));

    VX_CALL(vxReleaseScalar(&minval1_));
    VX_CALL(vxReleaseScalar(&maxval1_));
    VX_CALL(vxReleaseScalar(&mincount1_));
    VX_CALL(vxReleaseScalar(&maxcount1_));
    VX_CALL(vxReleaseArray(&minloc1_));
    VX_CALL(vxReleaseArray(&maxloc1_));

    if(ptbuf)
        ct_free_mem(ptbuf);
}


TEST_WITH_ARG(tivxMinMaxLoc, testOptionalParams, format_arg,
              MINMAXLOC_TEST_CASE(Graph, U8),
              MINMAXLOC_TEST_CASE(Graph, S16),
              )
{
    const int MAX_CAP = 300;
    int format = arg_->format;
    int mode = arg_->mode;
    vx_image src0_image, src1_image;
    CT_Image src0, src1;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;
    vx_context context = context_->vx_context_;
    int iter, k, niters = 3;
    uint64_t rng;
    int a, b;
    int minval0 = 0, maxval0 = 0, minval0_test = 0, maxval0_test = 0;
    uint32_t mincount0 = 0, maxcount0 = 0, mincount0_test = 0, maxcount0_test = 0;
    int minval1 = 0, maxval1 = 0, minval1_test = 0, maxval1_test = 0;
    uint32_t mincount1 = 0, maxcount1 = 0, mincount1_test = 0, maxcount1_test = 0;
    vx_scalar minval0_, maxval0_, mincount0_, maxcount0_;
    vx_array minloc0_ = 0, maxloc0_ = 0;
    vx_scalar minval1_, maxval1_, mincount1_, maxcount1_;
    vx_array minloc1_ = 0, maxloc1_ = 0;
    vx_enum sctype = format == VX_DF_IMAGE_U8 ? VX_TYPE_UINT8 :
                     format == VX_DF_IMAGE_S16 ? VX_TYPE_INT16 :
                     VX_TYPE_INT32;
    uint32_t pixsize = ct_image_bits_per_pixel(format)/8;
    Point* ptbuf = 0;
    vx_size bufbytes = 0, npoints = 0, bufcap = 0;

    if( format == VX_DF_IMAGE_U8 )
        a = 0, b = 256;
    else if( format == VX_DF_IMAGE_S16 )
        a = -32768, b = 32768;
    else
        a = INT_MIN/3, b = INT_MAX/3;

    minval0_ = ct_scalar_from_int(context, sctype, 0);
    maxval0_ = ct_scalar_from_int(context, sctype, 0);
    mincount0_ = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    maxcount0_ = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    minloc0_ = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    maxloc0_ = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    ASSERT(vxGetStatus((vx_reference)minloc0_) == VX_SUCCESS && vxGetStatus((vx_reference)maxloc0_) == VX_SUCCESS);

    minval1_ = ct_scalar_from_int(context, sctype, 0);
    maxval1_ = ct_scalar_from_int(context, sctype, 0);
    mincount1_ = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    maxcount1_ = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    minloc1_ = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    maxloc1_ = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    ASSERT(vxGetStatus((vx_reference)minloc1_) == VX_SUCCESS && vxGetStatus((vx_reference)maxloc1_) == VX_SUCCESS);

    rng = CT()->seed_;

    for( iter = 0; iter < niters; iter++ )
    {
        int return_loc = CT_RNG_NEXT_INT(rng, 0, 2);
        int return_count = CT_RNG_NEXT_INT(rng, 0, 2);
        int width, height;

        width = ct_roundf(ct_log_rng(&rng, 0, 10));
        height = ct_roundf(ct_log_rng(&rng, 0, 10));

        width = CT_MAX(width, 1);
        height = CT_MAX(height, 1);

        src0 = ct_allocate_ct_image_random(width, height, format, &rng, a, b);
        src1 = ct_allocate_ct_image_random(width, height, format, &rng, a, b);
        if( iter % 3 == 0 )
        {
            int mm[2], maxk;
            reference_minmax(src0, &mm[0], &mm[1]);
            maxk = CT_RNG_NEXT_INT(rng, 0, 100);
            // make sure that there are several pixels with minimum/maximum value
            ct_set_random_pixels(src0, &rng, maxk, 2, mm);
        }
        reference_minmaxloc(src0, &minval0, &maxval0, &mincount0, &maxcount0);
        src0_image = ct_image_to_vx_image(src0, context);
        reference_minmaxloc(src1, &minval1, &maxval1, &mincount1, &maxcount1);
        src1_image = ct_image_to_vx_image(src1, context);

        graph = vxCreateGraph(context);
        ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
        node1 = vxMinMaxLocNode(graph, src0_image, minval0_, maxval0_,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL);
        ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
        node2 = vxMinMaxLocNode(graph, src1_image, minval1_, maxval1_,
                                   NULL,
                                   NULL,
                                   mincount1_,
                                   maxcount1_);
        ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

        minval0_test = ct_scalar_as_int(minval0_);
        minval1_test = ct_scalar_as_int(minval1_);
        maxval0_test = ct_scalar_as_int(maxval0_);
        maxval1_test = ct_scalar_as_int(maxval1_);

        if( return_count )
        {
            mincount1_test = ct_scalar_as_int(mincount1_);
            maxcount1_test = ct_scalar_as_int(maxcount1_);
        }
        else
        {
            mincount1_test = mincount1;
            maxcount1_test = maxcount1;
        }

        if( minval0_test != minval0 || maxval0_test != maxval0 )
        {
            CT_RecordFailureAtFormat("Test case %d, first image. width=%d, height=%d,\n"
                                     "\tExpected: minval=%d, maxval=%d"
                                     "\tActual:   minval=%d, maxval=%d",
                                     __FUNCTION__, __FILE__, __LINE__,
                                     iter, width, height,
                                     minval0, maxval0,
                                     minval0_test, maxval0_test);
            break;
        }

        if( minval1_test != minval1 || maxval1_test != maxval1 || mincount1_test != mincount1 || maxcount1_test != maxcount1 )
        {
            CT_RecordFailureAtFormat("Test case %d, second image. width=%d, height=%d,\n"
                                     "\tExpected: minval=%d, maxval=%d, mincount=%d, maxcount=%d\n"
                                     "\tActual:   minval=%d, maxval=%d, mincount=%d, maxcount=%d\n",
                                     __FUNCTION__, __FILE__, __LINE__,
                                     iter, width, height,
                                     minval1, maxval1, mincount1, maxcount1,
                                     minval1_test, maxval1_test, mincount1_test, maxcount1_test);
            break;
        }

        VX_CALL(vxReleaseImage(&src0_image));
        VX_CALL(vxReleaseImage(&src1_image));
        VX_CALL(vxReleaseNode(&node1));
        VX_CALL(vxReleaseNode(&node2));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node1 == 0 && node2 == 0 && graph == 0);
        CT_CollectGarbage(CT_GC_IMAGE);

        printPerformance(perf_node1, width*height, "N1");
        printPerformance(perf_node2, width*height, "N2");
        printPerformance(perf_graph, width*height, "G1");
    }

    VX_CALL(vxReleaseScalar(&minval0_));
    VX_CALL(vxReleaseScalar(&maxval0_));
    VX_CALL(vxReleaseScalar(&mincount0_));
    VX_CALL(vxReleaseScalar(&maxcount0_));
    VX_CALL(vxReleaseArray(&minloc0_));
    VX_CALL(vxReleaseArray(&maxloc0_));

    VX_CALL(vxReleaseScalar(&minval1_));
    VX_CALL(vxReleaseScalar(&maxval1_));
    VX_CALL(vxReleaseScalar(&mincount1_));
    VX_CALL(vxReleaseScalar(&maxcount1_));
    VX_CALL(vxReleaseArray(&minloc1_));
    VX_CALL(vxReleaseArray(&maxloc1_));

    if(ptbuf)
        ct_free_mem(ptbuf);
}


TEST_WITH_ARG(tivxMinMaxLoc, testMinMaxLocSupernode, format_arg,
              MINMAXLOC_TEST_CASE(Graph, U8),
              MINMAXLOC_TEST_CASE(Graph, S16),
              )
{
    int node_count = 3;
    const int MAX_CAP = 300;
    int format = arg_->format;
    int mode = arg_->mode;
    vx_image src0_image, virt_image1, virt_image2, virt_image3;
    CT_Image src0, virt1, virt2, virt3;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_context context = context_->vx_context_;
    uint64_t rng;
    int a, b;
    int minval0 = 0, maxval0 = 0, minval0_test = 0, maxval0_test = 0;
    uint32_t mincount0 = 0, maxcount0 = 0, mincount0_test = 0, maxcount0_test = 0;
    vx_scalar minval0_, maxval0_, mincount0_, maxcount0_;
    vx_array minloc0_ = 0, maxloc0_ = 0;
    vx_enum sctype = format == VX_DF_IMAGE_U8 ? VX_TYPE_UINT8 :
                     format == VX_DF_IMAGE_S16 ? VX_TYPE_INT16 :
                     VX_TYPE_INT32;
    uint32_t pixsize = ct_image_bits_per_pixel(format)/8;
    Point* ptbuf = 0;
    vx_size bufbytes = 0, npoints = 0, bufcap = 0;
    vx_rectangle_t src_rect;
    vx_bool valid_rect = vx_false_e;
    vx_scalar shift_convertdepth = 0;
    vx_int32 sh = 0;
    int iter, k, niters = 3;

    if( format == VX_DF_IMAGE_U8 )
        a = 0, b = 256;
    else if( format == VX_DF_IMAGE_S16 )
        a = -32768, b = 32768;
    else
        a = INT_MIN/3, b = INT_MAX/3;

    minval0_ = ct_scalar_from_int(context, sctype, 0);
    maxval0_ = ct_scalar_from_int(context, sctype, 0);
    mincount0_ = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    maxcount0_ = ct_scalar_from_int(context, VX_TYPE_UINT32, 0);
    minloc0_ = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    maxloc0_ = vxCreateArray(context, VX_TYPE_COORDINATES2D, MAX_CAP);
    ASSERT(vxGetStatus((vx_reference)minloc0_) == VX_SUCCESS && vxGetStatus((vx_reference)maxloc0_) == VX_SUCCESS);

    rng = CT()->seed_;

    for( iter = 0; iter < niters; iter++ )
    {

        int return_loc = CT_RNG_NEXT_INT(rng, 0, 2);
        int return_count = CT_RNG_NEXT_INT(rng, 0, 2);
        uint32_t stride0, stride1;
        int width, height;

        width = ct_roundf(ct_log_rng(&rng, 0, 10));
        height = ct_roundf(ct_log_rng(&rng, 0, 10));

        width = CT_MAX(width, 1);
        height = CT_MAX(height, 1);

        src0 = ct_allocate_ct_image_random(width, height, format, &rng, a, b);
        stride0 = ct_stride_bytes(src0);
        virt1 = ct_allocate_image(width, height, format);
        virt2 = ct_allocate_image(width, height, format);
        
        if( iter % 3 == 0 )
        {
            int mm[2], maxk;
            reference_minmax(src0, &mm[0], &mm[1]);
            maxk = CT_RNG_NEXT_INT(rng, 0, 100);
            // make sure that there are several pixels with minimum/maximum value
            ct_set_random_pixels(src0, &rng, maxk, 2, mm);
        }


        ASSERT_VX_OBJECT(src0_image = ct_image_to_vx_image(src0, context), VX_TYPE_IMAGE);
        graph = vxCreateGraph(context);
        ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(virt_image1 = vxCreateVirtualImage(graph, 0, 0, format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(virt_image2 = vxCreateVirtualImage(graph, 0, 0, format), VX_TYPE_IMAGE);

        if( format == VX_DF_IMAGE_U8 )
        {
            referenceNot(src0, virt1);
            referenceNot(virt1, virt2);
            reference_minmaxloc(virt2, &minval0, &maxval0, &mincount0, &maxcount0);

            node1 = vxNotNode(graph, src0_image, virt_image1);
            node2 = vxNotNode(graph, virt_image1, virt_image2);
        }
        else if ( format == VX_DF_IMAGE_S16 ) {
            referenceSubtractSingle(src0, src0, virt1, VX_CONVERT_POLICY_SATURATE);
            referenceAddSingle(src0, virt1, virt2, VX_CONVERT_POLICY_SATURATE);
            reference_minmaxloc(virt2, &minval0, &maxval0, &mincount0, &maxcount0);

            node1 = vxSubtractNode(graph, src0_image, src0_image, VX_CONVERT_POLICY_SATURATE, virt_image1);
            node2 = vxAddNode(graph, src0_image, virt_image1, VX_CONVERT_POLICY_SATURATE, virt_image2);

        }
        node3 = vxMinMaxLocNode(graph, virt_image2, minval0_, maxval0_,
                                   return_loc ? minloc0_ : 0,
                                   return_loc ? maxloc0_ : 0,
                                   return_count ? mincount0_ : 0,
                                   return_count ? maxcount0_ : 0);

        ASSERT_NO_FAILURE(node_list[0] = node1); 
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_NO_FAILURE(node_list[2] = node3);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
        
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        if (NULL != node1)
        {
            vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
        }

        ASSERT_EQ_INT(valid_rect, vx_false_e);

        vxGetValidRegionImage(src0_image, &src_rect);

        ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), width);
        ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), height);

        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
        VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

        minval0_test = ct_scalar_as_int(minval0_);
        maxval0_test = ct_scalar_as_int(maxval0_);

        if( return_count )
        {
            mincount0_test = ct_scalar_as_int(mincount0_);
            maxcount0_test = ct_scalar_as_int(maxcount0_);
        }
        else
        {
            mincount0_test = mincount0;
            maxcount0_test = maxcount0;
        }

        if( minval0_test != minval0 || maxval0_test != maxval0 || mincount0_test != mincount0 || maxcount0_test != maxcount0 )
        {
            CT_RecordFailureAtFormat("Test case %d, first image. width=%d, height=%d,\n"
                                     "\tExpected: minval=%d, maxval=%d, mincount=%d, maxcount=%d\n"
                                     "\tActual:   minval=%d, maxval=%d, mincount=%d, maxcount=%d\n",
                                     __FUNCTION__, __FILE__, __LINE__,
                                     iter, width, height,
                                     minval0, maxval0, mincount0, maxcount0,
                                     minval0_test, maxval0_test, mincount0_test, maxcount0_test);
        }

        if( return_loc )
        {
            uint8_t* roi_ptr = src0->data.y;
            for(k = 0; k < 2; k++ )
            {
                int val0 = k == 0 ? minval0_test : maxval0_test;
                uint32_t i, count = k == 0 ? mincount0_test : maxcount0_test;
                vx_array loc = k == 0 ? minloc0_ : maxloc0_;
                vx_enum tp;
                union
                {
                    uint8_t u8;
                    int16_t s16;
                    int32_t s32;
                }
                uval;
                if( format == VX_DF_IMAGE_U8 )
                    uval.u8 = (uint8_t)val0;
                else if( format == VX_DF_IMAGE_S16 )
                    uval.s16 = (int16_t)val0;
                else
                    uval.s32 = (int32_t)val0;

                tp = ct_read_array(loc, (void**)&ptbuf, &bufbytes, &npoints, &bufcap);
                ASSERT(tp == VX_TYPE_COORDINATES2D);
                ASSERT(npoints == CT_MIN(bufcap, (vx_size)count));

                ct_sort_points(ptbuf, npoints);
                for( i = 0; i < npoints; i++ )
                {
                    Point p = ptbuf[i];
                    if( i > 0 )
                    {
                        // all the extrema locations should be different
                        ASSERT(p.x != ptbuf[i-1].x || p.y != ptbuf[i-1].y);
                    }
                    // value at each extrema location should match the extremum value
                    ASSERT(memcmp(roi_ptr + p.y*stride0 + p.x*pixsize, &uval, pixsize) == 0);
                }
            }
        }

        VX_CALL(vxReleaseImage(&src0_image));
        VX_CALL(vxReleaseImage(&virt_image1));
        VX_CALL(vxReleaseImage(&virt_image2));
        VX_CALL(tivxReleaseSuperNode(&super_node));
        VX_CALL(vxReleaseNode(&node1));
        VX_CALL(vxReleaseNode(&node2));
        VX_CALL(vxReleaseNode(&node3));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(super_node == 0 && node1 == 0 && node2 == 0 && node3 == 0 && graph == 0);
        CT_CollectGarbage(CT_GC_IMAGE);

        printPerformance(perf_super_node, width * height, "SN");
        printPerformance(perf_graph, width * height, "G");
    }

    VX_CALL(vxReleaseScalar(&minval0_));
    VX_CALL(vxReleaseScalar(&maxval0_));
    VX_CALL(vxReleaseScalar(&mincount0_));
    VX_CALL(vxReleaseScalar(&maxcount0_));
    VX_CALL(vxReleaseArray(&minloc0_));
    VX_CALL(vxReleaseArray(&maxloc0_));

    if(ptbuf)
        ct_free_mem(ptbuf);
}

#ifdef BUILD_BAM
#define testMinMaxLocSupernode testMinMaxLocSupernode
#else
#define testMinMaxLocSupernode DISABLED_testMinMaxLocSupernode
#endif

TESTCASE_TESTS(tivxMinMaxLoc, 
               testOnRandom, 
               testOptionalParams,
               testMinMaxLocSupernode)
