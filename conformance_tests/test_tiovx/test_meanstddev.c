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
#include <stdlib.h>
#include <limits.h>

#include <VX/vx.h>

#include "test_tiovx.h"

#define MAX_NODES 10

static void reference_mean_stddev(CT_Image src, vx_float32* _mean, vx_float32* _stddev)
{
    uint32_t x, y, width = src ? src->width : 0, height = src ? src->height : 0;
    uint32_t npix, stride;
    double sum = 0, sqsum = 0;
    int format = src ? src->format : VX_DF_IMAGE_U8;

    ASSERT(src);
    ASSERT(src->width > 0 && src->height > 0);
    npix = width*height;
    stride = ct_stride_bytes(src);

#define CASE_MEANSTDDEV(format, type, acctype) \
    case format: \
    { \
        acctype s = 0, s2 = 0; \
        for( y = 0; y < src->height; y++ ) \
        { \
            const type* ptr = (const type*)(src->data.y + stride*y); \
            for( x = 0; x < src->width; x++ ) \
            { \
                type val = ptr[x]; \
                s += val; \
                s2 += (acctype)val*val; \
            } \
        } \
        sum = (double)s; sqsum = (double)s2; \
    } \
    break

    switch(format)
    {
    CASE_MEANSTDDEV(VX_DF_IMAGE_U8, uint8_t, uint64_t);
    default:
        FAIL("Unsupported image format: (%d)", &src->format);
    }

    *_mean = (vx_float32)(sum/npix);
    sqsum = sqsum/npix - (sum/npix)*(sum/npix);
    *_stddev = (vx_float32)sqrt(CT_MAX(sqsum, 0.));
}

static void referenceNot(CT_Image src, CT_Image dst)
{
    uint32_t i, j;

    ASSERT(src && dst);
    ASSERT(src->width == dst->width);
    ASSERT(src->height == dst->height);
    ASSERT(src->format == dst->format && src->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = ~src->data.y[i * src->stride + j];
}


TESTCASE(tivxMeanStdDev, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    int mode;
    vx_df_image format;
} format_arg;


#define MEANSTDDEV_TEST_CASE(imm, tp) \
    {#imm "/" #tp, CT_##imm##_MODE, VX_DF_IMAGE_##tp}

TEST_WITH_ARG(tivxMeanStdDev, testOnRandom, format_arg,
              MEANSTDDEV_TEST_CASE(Graph, U8),
              )
{
    double mean_tolerance = 1e-4;
    double stddev_tolerance = 1e-4;
    int format = arg_->format;
    int mode = arg_->mode;
    vx_image src0_image, src1_image;
    CT_Image src0, src1;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;
    vx_graph graph = 0;
    vx_scalar mean0_s, stddev0_s, mean1_s, stddev1_s;
    vx_context context = context_->vx_context_;
    int iter, niters = 3;
    uint64_t rng;
    vx_float32 mean0 = 0.f, stddev0 = 0.f, mean1 = 0.f, stddev1 = 0.f, mean0_orig = 0.f, stddev0_orig = 0.f, mean1_orig = 0.f, stddev1_orig = 0.f;
    int a = 0, b = 256;
    vx_rectangle_t src_rect;
    vx_bool valid_rect;

    rng = CT()->seed_;
    mean_tolerance *= b;
    stddev_tolerance *= b;

    mean0_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean0_orig);
    ASSERT_VX_OBJECT(mean0_s, VX_TYPE_SCALAR);
    mean1_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean1_orig);
    ASSERT_VX_OBJECT(mean1_s, VX_TYPE_SCALAR);
    stddev0_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev0_orig);
    ASSERT_VX_OBJECT(stddev0_s, VX_TYPE_SCALAR);
    stddev1_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev1_orig);
    ASSERT_VX_OBJECT(stddev1_s, VX_TYPE_SCALAR);

    for( iter = 0; iter < niters; iter++ )
    {
        int width = ct_roundf(ct_log_rng(&rng, 0, 10));
        int height = ct_roundf(ct_log_rng(&rng, 0, 10));
        double mean0_diff, stddev0_diff, mean1_diff, stddev1_diff;
        width = CT_MAX(width, 1);
        height = CT_MAX(height, 1);

        if( !ct_check_any_size() )
        {
            width = CT_MIN((width + 7) & -8, 640);
            height = CT_MIN((height + 7) & -8, 480);
        }

        src0 = ct_allocate_ct_image_random(width, height, format, &rng, a, b);
        reference_mean_stddev(src0, &mean0, &stddev0);
        src0_image = ct_image_to_vx_image(src0, context);

        src1 = ct_allocate_ct_image_random(width, height, format, &rng, a, b);
        reference_mean_stddev(src1, &mean1, &stddev1);
        src1_image = ct_image_to_vx_image(src1, context);

        graph = vxCreateGraph(context);
        ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
        node1 = vxMeanStdDevNode(graph, src0_image, mean0_s, stddev0_s);
        ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
        node2 = vxMeanStdDevNode(graph, src1_image, mean1_s, stddev1_s);
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

        VX_CALL(vxCopyScalar(mean0_s, &mean0_orig, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyScalar(stddev0_s, &stddev0_orig, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        VX_CALL(vxCopyScalar(mean1_s, &mean1_orig, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyScalar(stddev1_s, &stddev1_orig, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        mean0_diff = fabs(mean0_orig - mean0);
        stddev0_diff = fabs(stddev0_orig - stddev0);

        mean1_diff = fabs(mean1_orig - mean1);
        stddev1_diff = fabs(stddev1_orig - stddev1);

        if( mean0_diff > mean_tolerance ||
            stddev0_diff > stddev_tolerance )
        {
            CT_RecordFailureAtFormat("Test case %d, image 0. width=%d, height=%d,\n"
                                     "\tExpected: mean=%.5g, stddev=%.5g\n"
                                     "\tActual:   mean=%.5g (diff=%.5g %s %.5g), stddev=%.5f (diff=%.5g %s %.5g)\n",
                                     __FUNCTION__, __FILE__, __LINE__,
                                     iter, width, height,
                                     mean0, stddev0,
                                     mean0_orig, mean0_diff, mean0_diff > mean_tolerance ? ">" : "<=", mean_tolerance,
                                     stddev0_orig, stddev0_diff, stddev0_diff > stddev_tolerance ? ">" : "<=", stddev_tolerance);
            break;
        }

        if( mean1_diff > mean_tolerance ||
            stddev1_diff > stddev_tolerance )
        {
            CT_RecordFailureAtFormat("Test case %d, image 1. width=%d, height=%d,\n"
                                     "\tExpected: mean=%.5g, stddev=%.5g\n"
                                     "\tActual:   mean=%.5g (diff=%.5g %s %.5g), stddev=%.5f (diff=%.5g %s %.5g)\n",
                                     __FUNCTION__, __FILE__, __LINE__,
                                     iter, width, height,
                                     mean1, stddev0,
                                     mean1_orig, mean1_diff, mean1_diff > mean_tolerance ? ">" : "<=", mean_tolerance,
                                     stddev1_orig, stddev1_diff, stddev1_diff > stddev_tolerance ? ">" : "<=", stddev_tolerance);
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

    VX_CALL(vxReleaseScalar(&mean0_s));
    VX_CALL(vxReleaseScalar(&stddev0_s));
    VX_CALL(vxReleaseScalar(&mean1_s));
    VX_CALL(vxReleaseScalar(&stddev1_s));
}

TEST_WITH_ARG(tivxMeanStdDev, testMeanStdDevSupernode, format_arg,
              MEANSTDDEV_TEST_CASE(Graph, U8),
              )
{
    int node_count = 2;
    double mean_tolerance = 1e-4;
    double stddev_tolerance = 1e-4;
    int format = arg_->format;
    int mode = arg_->mode;
    vx_image src0_image, virt_image;
    CT_Image src0, ref_virt;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_graph graph = 0;
    vx_scalar mean0_s, stddev0_s;
    vx_context context = context_->vx_context_;
    uint64_t rng;
    vx_float32 mean0 = 0.f, stddev0 = 0.f, mean1 = 0.f, stddev1 = 0.f, mean0_orig = 0.f, stddev0_orig = 0.f, mean1_orig = 0.f, stddev1_orig = 0.f;
    int a = 0, b = 256;
    vx_rectangle_t src_rect;
    vx_bool valid_rect;

    rng = CT()->seed_;
    mean_tolerance *= b;
    stddev_tolerance *= b;

    mean0_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &mean0_orig);
    ASSERT_VX_OBJECT(mean0_s, VX_TYPE_SCALAR);
    stddev0_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &stddev0_orig);
    ASSERT_VX_OBJECT(stddev0_s, VX_TYPE_SCALAR);

    int width = ct_roundf(ct_log_rng(&rng, 0, 10));
    int height = ct_roundf(ct_log_rng(&rng, 0, 10));
    double mean0_diff, stddev0_diff;
    width = CT_MAX(width, 1);
    height = CT_MAX(height, 1);

    if( !ct_check_any_size() )
    {
        width = CT_MIN((width + 7) & -8, 640);
        height = CT_MIN((height + 7) & -8, 480);
    }

    src0 = ct_allocate_ct_image_random(width, height, format, &rng, a, b);
    ref_virt = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
    referenceNot(src0, ref_virt);
    reference_mean_stddev(ref_virt, &mean0, &stddev0);
    src0_image = ct_image_to_vx_image(src0, context);


    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt_image = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    node1 = vxNotNode(graph, src0_image, virt_image);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
    node2 = vxMeanStdDevNode(graph, virt_image, mean0_s, stddev0_s);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    ASSERT_NO_FAILURE(node_list[0] = node1); 
    ASSERT_NO_FAILURE(node_list[1] = node2);
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src0_image, &src_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), height);

    VX_CALL(vxCopyScalar(mean0_s, &mean0_orig, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyScalar(stddev0_s, &stddev0_orig, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    mean0_diff = fabs(mean0_orig - mean0);
    stddev0_diff = fabs(stddev0_orig - stddev0);

    if( mean0_diff > mean_tolerance ||
        stddev0_diff > stddev_tolerance )
    {
        CT_RecordFailureAtFormat("Test case %d, image 0. width=%d, height=%d,\n"
                                 "\tExpected: mean=%.5g, stddev=%.5g\n"
                                 "\tActual:   mean=%.5g (diff=%.5g %s %.5g), stddev=%.5f (diff=%.5g %s %.5g)\n",
                                 __FUNCTION__, __FILE__, __LINE__,
                                 0, width, height,
                                 mean0, stddev0,
                                 mean0_orig, mean0_diff, mean0_diff > mean_tolerance ? ">" : "<=", mean_tolerance,
                                 stddev0_orig, stddev0_diff, stddev0_diff > stddev_tolerance ? ">" : "<=", stddev_tolerance);
    }

    VX_CALL(vxReleaseScalar(&mean0_s));
    VX_CALL(vxReleaseScalar(&stddev0_s));   
    VX_CALL(vxReleaseImage(&src0_image));
    VX_CALL(vxReleaseImage(&virt_image));
    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(super_node == 0 && node1 == 0 && node2 == 0 && graph == 0);
    CT_CollectGarbage(CT_GC_IMAGE);

    printPerformance(perf_super_node, width * height, "SN");
    printPerformance(perf_graph, width * height, "G");
}

#ifdef BUILD_BAM
#define testMeanStdDevSupernode testMeanStdDevSupernode
#else
#define testMeanStdDevSupernode DISABLED_testMeanStdDevSupernode
#endif

TESTCASE_TESTS(tivxMeanStdDev, 
               testOnRandom,
               testMeanStdDevSupernode)
