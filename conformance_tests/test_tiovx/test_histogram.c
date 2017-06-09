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

static void reference_histogram(CT_Image src, int32_t* hist, int nbins, int offset, int range)
{
    int i, hist0[256];
    uint32_t x, y, width = src->width, height = src->height, stride;

    ASSERT(src);
    ASSERT(src->format == VX_DF_IMAGE_U8);
    ASSERT(src->width > 0 && src->height > 0);
    stride = ct_stride_bytes(src);

    for( i = 0; i < 256; i++ )
        hist0[i] = 0;

    for( y = 0; y < height; y++ )
    {
        const uint8_t* ptr = src->data.y + y*stride;
        for( x = 0; x < width; x++ )
            hist0[ptr[x]]++;
    }

    for( i = 0; i < nbins; i++ )
        hist[i] = 0;

    for( i = offset; i < offset + range; i++ )
    {
        int j = (i - offset)*nbins/range;
        hist[j] = (int32_t)(hist[j] + hist0[i]);
    }
}

static vx_uint32 reference_window(vx_uint32 range, vx_size nbins)
{
    vx_uint32 test_window = (vx_uint32)(range / nbins);
    if (test_window*nbins == range)
        return test_window;
    else
        return 0;
}

static void reverse_histogram(int nbins, int32_t hist[])
{
    int i, j;
    for (i = 0, j = nbins-1; i < j; ++i, --j)
    {
        int32_t a = hist[i];
        int32_t b = hist[j];
        hist[i] = b;
        hist[j] = a;
    }
}

TESTCASE(tivxHistogram, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    int mode;
    vx_df_image format;
} format_arg;

#define MAX_BINS 256

#define HIST_TEST_CASE(imm, tp) \
    {#imm "/" #tp, CT_##imm##_MODE, VX_DF_IMAGE_##tp}

#define COMPARE_HISTOGRAMS_SRC0(_equal, _hist0, _hist1) \
    do { \
        _equal = 1; \
        int _i; \
        for( _i = 0; _i < nbins; _i++ ) \
        { \
            if( _hist0[_i] != _hist1[_i] ) \
            { \
                _equal = 0; \
                CT_RecordFailureAtFormat("Test case %d. width=%d, height=%d, nbins=%d, offset=%d, range=%d\n" \
                                         "\tExpected: %s[%d]=%d\n" \
                                         "\tActual:   %s[%d]=%d\n", \
                                         __FUNCTION__, __FILE__, __LINE__, \
                                         iter, width_src0, height_src0, nbins, offset, range, \
                                         #_hist1, _i, _hist0[_i], #_hist1, _i, _hist1[_i]); \
                break; \
            } \
        } \
    } while(0)

#define COMPARE_HISTOGRAMS_SRC1(_equal, _hist0, _hist1) \
    do { \
        _equal = 1; \
        int _i; \
        for( _i = 0; _i < nbins; _i++ ) \
        { \
            if( _hist0[_i] != _hist1[_i] ) \
            { \
                _equal = 0; \
                CT_RecordFailureAtFormat("Test case %d. width=%d, height=%d, nbins=%d, offset=%d, range=%d\n" \
                                         "\tExpected: %s[%d]=%d\n" \
                                         "\tActual:   %s[%d]=%d\n", \
                                         __FUNCTION__, __FILE__, __LINE__, \
                                         iter, width_src1, height_src1, nbins, offset, range, \
                                         #_hist1, _i, _hist0[_i], #_hist1, _i, _hist1[_i]); \
                break; \
            } \
        } \
    } while(0)

TEST_WITH_ARG(tivxHistogram, testOnRandom, format_arg,
              HIST_TEST_CASE(Graph, U8),
              )
{
    int format = arg_->format;
    int mode = arg_->mode;
    vx_image src0_image, src1_image;
    CT_Image src0, src1;
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_distribution dist_src0, dist_src1;
    int iter, niters = 3;
    uint64_t rng;
    int a = 0, b = 256;
    int32_t hist0_src0[MAX_BINS];
    int32_t hist1_src0[MAX_BINS];
    int32_t hist0_src1[MAX_BINS];
    int32_t hist1_src1[MAX_BINS];
    vx_perf_t perf_node1, perf_node2, perf_graph;

    const vx_enum mem_type = VX_MEMORY_TYPE_HOST;
    const vx_bitfield flags = 0;

    rng = CT()->seed_;

    for( iter = 0; iter < niters; iter++ )
    {
        int width_src0, height_src0, width_src1, height_src1;
        int val0 = CT_RNG_NEXT_INT(rng, 0, (MAX_BINS-1)), val1 = CT_RNG_NEXT_INT(rng, 0, (MAX_BINS-1));
        int offset = CT_MIN(val0, val1), range = CT_MAX(val0, val1) - offset + 1;
        int nbins = CT_RNG_NEXT_INT(rng, 1, range+1);

        width_src0 = 640;
        height_src0 = 480;
        width_src1 = 644;
        height_src1 = 248;

        ASSERT_NO_FAILURE(src0 = ct_allocate_ct_image_random(width_src0, height_src0, format, &rng, a, b));
        ASSERT_NO_FAILURE(reference_histogram(src0, hist0_src0, nbins, offset, range));

        ASSERT_NO_FAILURE(src1 = ct_allocate_ct_image_random(width_src1, height_src1, format, &rng, a, b));
        ASSERT_NO_FAILURE(reference_histogram(src1, hist0_src1, nbins, offset, range));

        src0_image = ct_image_to_vx_image(src0, context);
        ASSERT_VX_OBJECT(src0_image, VX_TYPE_IMAGE);

        src1_image = ct_image_to_vx_image(src1, context);
        ASSERT_VX_OBJECT(src1_image, VX_TYPE_IMAGE);

        dist_src0 = vxCreateDistribution(context, nbins, offset, range);
        ASSERT_VX_OBJECT(dist_src0, VX_TYPE_DISTRIBUTION);

        dist_src1 = vxCreateDistribution(context, nbins, offset, range);
        ASSERT_VX_OBJECT(dist_src1, VX_TYPE_DISTRIBUTION);

        graph = vxCreateGraph(context);
        ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
        node1 = vxHistogramNode(graph, src0_image, dist_src0);
        ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
        node2 = vxHistogramNode(graph, src1_image, dist_src1);
        ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

        /* test for copy - read distribution */
        {
            int equalHist0 = 0;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyDistribution(dist_src0, hist1_src0, VX_READ_ONLY, mem_type));
            COMPARE_HISTOGRAMS_SRC0(equalHist0, hist0_src0, hist1_src0);
            if( !equalHist0 )
                CT_FAIL("check for vxCopyDistribution(read dist_src0) failed\n");
        }

        /* test for copy - read distribution */
        {
            int equalHist1 = 0;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyDistribution(dist_src1, hist1_src1, VX_READ_ONLY, mem_type));
            COMPARE_HISTOGRAMS_SRC1(equalHist1, hist0_src1, hist1_src1);
            if( !equalHist1 )
                CT_FAIL("check for vxCopyDistribution(read dist_src1) failed\n");
        }

        {
            /* smoke tests for query distribution attributes */
            vx_size   attr_dims = 0;
            vx_int32  attr_offset = 0;
            vx_uint32 attr_range = 0;
            vx_size   attr_bins = 0;
            vx_uint32 attr_window = 0;
            vx_size   attr_size = 0;
            VX_CALL(vxQueryDistribution(dist_src0, VX_DISTRIBUTION_DIMENSIONS, &attr_dims, sizeof(attr_dims)));
            if (1 != attr_dims)
                CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_DIMENSIONS failed\n");

            VX_CALL(vxQueryDistribution(dist_src0, VX_DISTRIBUTION_OFFSET, &attr_offset, sizeof(attr_offset)));
            if (attr_offset != offset)
                CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_OFFSET failed\n");

            VX_CALL(vxQueryDistribution(dist_src0, VX_DISTRIBUTION_RANGE, &attr_range, sizeof(attr_range)));
            if (attr_range != range)
                CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_RANGE failed\n");

            VX_CALL(vxQueryDistribution(dist_src0, VX_DISTRIBUTION_BINS, &attr_bins, sizeof(attr_bins)));
            if (attr_bins != nbins)
                CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_BINS failed\n");

            VX_CALL(vxQueryDistribution(dist_src0, VX_DISTRIBUTION_WINDOW, &attr_window, sizeof(attr_window)));
	    /*Tthe attribute is specified as valid only when the range is a multiple of nbins,
	     * in other cases, its value shouldn't be checked */
            if (((range % nbins) == 0) && (attr_window != reference_window(range, nbins)))
                CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_WINDOW failed\n");

            VX_CALL(vxQueryDistribution(dist_src0, VX_DISTRIBUTION_SIZE, &attr_size, sizeof(attr_size)));
            if (attr_size < nbins*sizeof(hist1_src0[0]))
                CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_SIZE failed\n");
        }

        {
            /* smoke tests for query distribution attributes */
            vx_size   attr_dims = 0;
            vx_int32  attr_offset = 0;
            vx_uint32 attr_range = 0;
            vx_size   attr_bins = 0;
            vx_uint32 attr_window = 0;
            vx_size   attr_size = 0;
            VX_CALL(vxQueryDistribution(dist_src1, VX_DISTRIBUTION_DIMENSIONS, &attr_dims, sizeof(attr_dims)));
            if (1 != attr_dims)
                CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_DIMENSIONS failed\n");

            VX_CALL(vxQueryDistribution(dist_src1, VX_DISTRIBUTION_OFFSET, &attr_offset, sizeof(attr_offset)));
            if (attr_offset != offset)
                CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_OFFSET failed\n");

            VX_CALL(vxQueryDistribution(dist_src1, VX_DISTRIBUTION_RANGE, &attr_range, sizeof(attr_range)));
            if (attr_range != range)
                CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_RANGE failed\n");

            VX_CALL(vxQueryDistribution(dist_src1, VX_DISTRIBUTION_BINS, &attr_bins, sizeof(attr_bins)));
            if (attr_bins != nbins)
                CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_BINS failed\n");

            VX_CALL(vxQueryDistribution(dist_src1, VX_DISTRIBUTION_WINDOW, &attr_window, sizeof(attr_window)));
	    /*Tthe attribute is specified as valid only when the range is a multiple of nbins,
	     * in other cases, its value shouldn't be checked */
            if (((range % nbins) == 0) && (attr_window != reference_window(range, nbins)))
                CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_WINDOW failed\n");

            VX_CALL(vxQueryDistribution(dist_src1, VX_DISTRIBUTION_SIZE, &attr_size, sizeof(attr_size)));
            if (attr_size < nbins*sizeof(hist1_src1[0]))
                CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_SIZE failed\n");
        }

        /* test for copy - write distribution */
        {
            vx_distribution dist2;
            int32_t hist2[MAX_BINS];
            int equal = 0;

            dist2 = vxCreateDistribution(context, nbins, offset, range);
            ASSERT_VX_OBJECT(dist2, VX_TYPE_DISTRIBUTION);

            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyDistribution(dist2, hist1_src0, VX_WRITE_ONLY, mem_type));
            COMPARE_HISTOGRAMS_SRC0(equal, hist0_src0, hist1_src0);
            if( !equal )
                CT_FAIL("check for vxCopyDistribution(write dist2) failed\n");

            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyDistribution(dist2, hist2, VX_READ_ONLY, mem_type));
            COMPARE_HISTOGRAMS_SRC0(equal, hist0_src0, hist2);
            if( !equal )
                CT_FAIL("check for vxCopyDistribution(read dist2) failed\n");

            VX_CALL(vxReleaseDistribution(&dist2));
        }

        /* test for copy - write distribution */
        {
            vx_distribution dist2;
            int32_t hist2[MAX_BINS];
            int equal = 0;

            dist2 = vxCreateDistribution(context, nbins, offset, range);
            ASSERT_VX_OBJECT(dist2, VX_TYPE_DISTRIBUTION);

            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyDistribution(dist2, hist1_src1, VX_WRITE_ONLY, mem_type));
            COMPARE_HISTOGRAMS_SRC1(equal, hist0_src1, hist1_src1);
            if( !equal )
                CT_FAIL("check for vxCopyDistribution(write dist2) failed\n");

            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyDistribution(dist2, hist2, VX_READ_ONLY, mem_type));
            COMPARE_HISTOGRAMS_SRC1(equal, hist0_src1, hist2);
            if( !equal )
                CT_FAIL("check for vxCopyDistribution(read dist2) failed\n");

            VX_CALL(vxReleaseDistribution(&dist2));
        }

        /* test for map/unmap - read/write distribution */
        {
            vx_map_id map1;
            int32_t* hptr1 = NULL;
            int32_t hist1r[MAX_BINS];
            int equal = 0;

            reverse_histogram(nbins, hist1_src0);
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapDistribution(dist_src0, &map1, (void*)&hptr1, VX_WRITE_ONLY, mem_type, flags));
            memcpy(hptr1, hist1_src0, nbins*sizeof(hist1_src0[0]));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapDistribution(dist_src0, map1));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyDistribution(dist_src0, hist1r, VX_READ_ONLY, mem_type));
            COMPARE_HISTOGRAMS_SRC0(equal, hist1_src0, hist1r);
            if( !equal )
                CT_FAIL("check for vxMapDistribution/vxUnmapDistribution(write dist_src0) failed\n");

            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapDistribution(dist_src0, &map1, (void*)&hptr1, VX_READ_ONLY, mem_type, flags));
            COMPARE_HISTOGRAMS_SRC0(equal, hist1_src0, hptr1);
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapDistribution(dist_src0, map1));
            if( !equal )
                CT_FAIL("check for vxMapDistribution/vxUnmapDistribution(read dist_src0) failed\n");

            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapDistribution(dist_src0, &map1, (void*)&hptr1, VX_READ_AND_WRITE, mem_type, flags));
            COMPARE_HISTOGRAMS_SRC0(equal, hist1_src0, hptr1);
            reverse_histogram(nbins, hist1_src0);
            memcpy(hist1_src0, hptr1, nbins*sizeof(hist1_src0[0]));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapDistribution(dist_src0, map1));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyDistribution(dist_src0, hist1r, VX_READ_ONLY, mem_type));
            COMPARE_HISTOGRAMS_SRC0(equal, hist1_src0, hist1r);
            if( !equal )
                CT_FAIL("check for vxMapDistribution/vxUnmapDistribution(read-write dist_src0) failed\n");
        }

        /* test for map/unmap - read/write distribution */
        {
            vx_map_id map1;
            int32_t* hptr1 = NULL;
            int32_t hist1r[MAX_BINS];
            int equal = 0;

            reverse_histogram(nbins, hist1_src1);
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapDistribution(dist_src1, &map1, (void*)&hptr1, VX_WRITE_ONLY, mem_type, flags));
            memcpy(hptr1, hist1_src1, nbins*sizeof(hist1_src1[0]));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapDistribution(dist_src1, map1));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyDistribution(dist_src1, hist1r, VX_READ_ONLY, mem_type));
            COMPARE_HISTOGRAMS_SRC1(equal, hist1_src1, hist1r);
            if( !equal )
                CT_FAIL("check for vxMapDistribution/vxUnmapDistribution(write dist_src1) failed\n");

            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapDistribution(dist_src1, &map1, (void*)&hptr1, VX_READ_ONLY, mem_type, flags));
            COMPARE_HISTOGRAMS_SRC1(equal, hist1_src1, hptr1);
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapDistribution(dist_src1, map1));
            if( !equal )
                CT_FAIL("check for vxMapDistribution/vxUnmapDistribution(read dist_src1) failed\n");

            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapDistribution(dist_src1, &map1, (void*)&hptr1, VX_READ_AND_WRITE, mem_type, flags));
            COMPARE_HISTOGRAMS_SRC1(equal, hist1_src1, hptr1);
            reverse_histogram(nbins, hist1_src1);
            memcpy(hist1_src1, hptr1, nbins*sizeof(hist1_src1[0]));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapDistribution(dist_src1, map1));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyDistribution(dist_src1, hist1r, VX_READ_ONLY, mem_type));
            COMPARE_HISTOGRAMS_SRC1(equal, hist1_src1, hist1r);
            if( !equal )
                CT_FAIL("check for vxMapDistribution/vxUnmapDistribution(read-write dist_src1) failed\n");
        }

        VX_CALL(vxReleaseImage(&src0_image));
        VX_CALL(vxReleaseImage(&src1_image));
        VX_CALL(vxReleaseDistribution(&dist_src0));
        VX_CALL(vxReleaseDistribution(&dist_src1));
        VX_CALL(vxReleaseNode(&node1));
        VX_CALL(vxReleaseNode(&node2));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node1 == 0 && node2 == 0 && graph == 0);
        CT_CollectGarbage(CT_GC_IMAGE);

        printf("Test case %d\n", iter);
        printPerformance(perf_node1, width_src0*height_src0, "N1");
        printPerformance(perf_node2, width_src0*height_src0, "N2");
        printPerformance(perf_graph, width_src0*height_src0, "G1");
        printf("\n");
    }
}

TESTCASE_TESTS(tivxHistogram, testOnRandom)
