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
#include <limits.h>
#include <math.h>

#include "shared_functions.h"

static void reference_mag(CT_Image dx, CT_Image dy, CT_Image mag)
{
    uint32_t x, y, width, height, dxstride, dystride, magstride;

    ASSERT(dx && dy && mag);
    ASSERT(dx->format == VX_DF_IMAGE_S16 && dy->format == VX_DF_IMAGE_S16 && mag->format == VX_DF_IMAGE_S16);
    ASSERT(dx->width > 0 && dx->height > 0 &&
           dx->width == dy->width && dx->height == dy->height &&
           dx->width == mag->width && dx->height == mag->height);
    width = dx->width;
    height = dy->height;
    dxstride = ct_stride_bytes(dx);
    dystride = ct_stride_bytes(dy);
    magstride = ct_stride_bytes(mag);

    for( y = 0; y < height; y++ )
    {
        const int16_t* dxptr = (const int16_t*)(dx->data.y + y*dxstride);
        const int16_t* dyptr = (const int16_t*)(dy->data.y + y*dystride);
        int16_t* magptr = (int16_t*)(mag->data.y + y*magstride);
        for( x = 0; x < width; x++ )
        {
            // the specification says - use double in the test implementation
            double val = sqrt((double)dxptr[x]*dxptr[x] + (double)dyptr[x]*dyptr[x]);
            int ival = (int)floor(val + 0.5);
            magptr[x] = CT_CAST_S16(ival);
        }
    }
}

static void reference_sequential_mag(CT_Image dx0, CT_Image dy0, CT_Image dx1, CT_Image dy1, CT_Image virt0, CT_Image virt1, CT_Image mag)
{
    reference_mag(dx0, dy0, virt0);
    reference_mag(dx1, dy1, virt1);
    reference_mag(virt0, virt1, mag);
}


TESTCASE(tivxMagnitude, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    int mode;
    int use_sobel;
    vx_df_image format;
} format_arg;

#undef UseSobel
#undef Random
#define UseSobel 1
#define Random 0

#define MAG_TEST_CASE(imm, sob, tp) \
    {#imm "/" #sob "/" #tp, CT_##imm##_MODE, sob, VX_DF_IMAGE_##tp}

TEST_WITH_ARG(tivxMagnitude, testOnRandom, format_arg,
              MAG_TEST_CASE(Graph, Random, S16),
              )
{
    int dxformat = arg_->format;
    int mode = arg_->mode;
    int srcformat = dxformat == VX_DF_IMAGE_S16 ? VX_DF_IMAGE_U8 : -1;
    int magformat = dxformat == VX_DF_IMAGE_S16 ? VX_DF_IMAGE_S16 : -1;
    vx_image dx_node1=0, dy_node1=0, dx_node2=0, dy_node2=0, mag=0, virt1, virt2;
    CT_Image src0, dx0, dy0, dx1, dy1, mag0, mag1, virt_ctimage1, virt_ctimage2;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_context context = context_->vx_context_;
    int iter, niters = 3;
    uint64_t rng;
    int dxmin = -32768, dxmax = 32768;
    vx_border_t border;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_graph;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    ASSERT( srcformat != -1 && magformat != -1 );
    rng = CT()->seed_;
    border.mode = VX_BORDER_REPLICATE;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
                        vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER,
                                              &border, sizeof(border)));

    for( iter = 0; iter < niters; iter++ )
    {
        int width = ct_roundf(ct_log_rng(&rng, 0, 10));
        int height = ct_roundf(ct_log_rng(&rng, 0, 10));

        width = CT_MAX(width, 1);
        height = CT_MAX(height, 1);

        if( !ct_check_any_size() )
        {
            width = CT_MIN((width + 7) & -8, 640);
            height = CT_MIN((height + 7) & -8, 480);
        }

        int k, maxk = CT_RNG_NEXT_INT(rng, 0, 20);
        ASSERT_NO_FAILURE(dx0 = ct_allocate_ct_image_random(width, height, dxformat, &rng, dxmin, dxmax));
        ASSERT_NO_FAILURE(dy0 = ct_allocate_ct_image_random(width, height, dxformat, &rng, dxmin, dxmax));
        ASSERT_NO_FAILURE(dx1 = ct_allocate_ct_image_random(width, height, dxformat, &rng, dxmin, dxmax));
        ASSERT_NO_FAILURE(dy1 = ct_allocate_ct_image_random(width, height, dxformat, &rng, dxmin, dxmax));

        // add some extreme points to the generated images
        for( k = 0; k < maxk; k++ )
        {
            int x = CT_RNG_NEXT_INT(rng, 0, width);
            int y = CT_RNG_NEXT_INT(rng, 0, height);
            int dxval = CT_RNG_NEXT_BOOL(rng) ? dxmin : dxmax;
            int dyval = CT_RNG_NEXT_BOOL(rng) ? dxmin : dxmax;
            dx0->data.s16[dx0->stride*y + x] = (short)dxval;
            dy0->data.s16[dy0->stride*y + x] = (short)dyval;
            dx1->data.s16[dx1->stride*y + x] = (short)dxval;
            dy1->data.s16[dy1->stride*y + x] = (short)dyval;
        }

        dx_node1 = ct_image_to_vx_image(dx0, context);
        ASSERT_VX_OBJECT(dx_node1, VX_TYPE_IMAGE);
        dx_node2 = ct_image_to_vx_image(dx1, context);
        ASSERT_VX_OBJECT(dx_node2, VX_TYPE_IMAGE);
        dy_node1 = ct_image_to_vx_image(dy0, context);
        ASSERT_VX_OBJECT(dy_node1, VX_TYPE_IMAGE);
        dy_node2 = ct_image_to_vx_image(dy1, context);
        ASSERT_VX_OBJECT(dy_node2, VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(mag0 = ct_allocate_image(width, height, magformat));
        ASSERT_NO_FAILURE(virt_ctimage1 = ct_allocate_image(width, height, magformat));
        ASSERT_NO_FAILURE(virt_ctimage2 = ct_allocate_image(width, height, magformat));
        ASSERT_NO_FAILURE(reference_sequential_mag(dx0, dy0, dx1, dy1, virt_ctimage1, virt_ctimage2, mag0));
        mag = vxCreateImage(context, width, height, magformat);
        ASSERT_VX_OBJECT(mag, VX_TYPE_IMAGE);

        graph = vxCreateGraph(context);
        ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, dxformat), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, dxformat), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
        node1 = vxMagnitudeNode(graph, dx_node1, dy_node1, virt1);
        ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
        node2 = vxMagnitudeNode(graph, dx_node2, dy_node2, virt2);
        ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);
        node3 = vxMagnitudeNode(graph, virt1, virt2, mag);
        ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
        ASSERT_EQ_INT(valid_rect, vx_false_e);

        vxGetValidRegionImage(dx_node1, &src_rect);
        vxGetValidRegionImage(mag, &dst_rect);

        ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), width);
        ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), height);

        ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), width);
        ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), height);

        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
        vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

        mag1 = ct_image_from_vx_image(mag);

        ASSERT_CTIMAGE_NEAR(mag0, mag1, 1);
        VX_CALL(vxReleaseImage(&dx_node1));
        VX_CALL(vxReleaseImage(&dx_node2));
        VX_CALL(vxReleaseImage(&dy_node1));
        VX_CALL(vxReleaseImage(&dy_node2));
        VX_CALL(vxReleaseImage(&virt1));
        VX_CALL(vxReleaseImage(&virt2));
        VX_CALL(vxReleaseImage(&mag));
        VX_CALL(vxReleaseNode(&node1));
        VX_CALL(vxReleaseNode(&node2));
        VX_CALL(vxReleaseNode(&node3));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node1 == 0 && node2 == 0 && node3 == 0 && graph == 0);
        CT_CollectGarbage(CT_GC_IMAGE);

        printPerformance(perf_node1, width*height, "N1");
        printPerformance(perf_node2, width*height, "N2");
        printPerformance(perf_node3, width*height, "N3");
        printPerformance(perf_graph, width*height, "G1");
    }
}

TESTCASE_TESTS(tivxMagnitude, testOnRandom)
