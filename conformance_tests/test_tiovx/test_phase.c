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

static void referenceConvertDepth(CT_Image src, CT_Image dst, int shift, vx_enum policy)
{
    uint32_t i, j;

    ASSERT(src && dst);
    ASSERT(src->width == dst->width);
    ASSERT(src->height == dst->height);
    ASSERT((src->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_S16) || (src->format == VX_DF_IMAGE_S16 && dst->format == VX_DF_IMAGE_U8));
    ASSERT(policy == VX_CONVERT_POLICY_WRAP || policy == VX_CONVERT_POLICY_SATURATE);

    if (shift > 16) shift = 16;
    if (shift < -16) shift = -16;

    if (src->format == VX_DF_IMAGE_U8)
    {
            // up-conversion + wrap
        if (shift < 0)
        {
            for (i = 0; i < dst->height; ++i)
                for (j = 0; j < dst->width; ++j)
                    dst->data.s16[i * dst->stride + j] = ((unsigned)src->data.y[i * src->stride + j]) >> (-shift);
        }
        else
        {
            for (i = 0; i < dst->height; ++i)
                for (j = 0; j < dst->width; ++j)
                    dst->data.s16[i * dst->stride + j] = ((unsigned)src->data.y[i * src->stride + j]) << shift;
        }
    }
    else if (policy == VX_CONVERT_POLICY_WRAP)
    {
        // down-conversion + wrap
        if (shift < 0)
        {
            for (i = 0; i < dst->height; ++i)
                for (j = 0; j < dst->width; ++j)
                    dst->data.y[i * dst->stride + j] = src->data.s16[i * src->stride + j] << (-shift);
        }
        else
        {
            for (i = 0; i < dst->height; ++i)
                for (j = 0; j < dst->width; ++j)
                    dst->data.y[i * dst->stride + j] = src->data.s16[i * src->stride + j] >> shift;
        }
    }
    else if (policy == VX_CONVERT_POLICY_SATURATE)
    {
        // down-conversion + saturate
        if (shift < 0)
        {
            for (i = 0; i < dst->height; ++i)
                for (j = 0; j < dst->width; ++j)
                {
                    int32_t v = src->data.s16[i * src->stride + j] << (-shift);
                    if (v > 255) v = 255;
                    if (v < 0) v = 0;
                    dst->data.y[i * dst->stride + j] = v;
                }
        }
        else
        {
            for (i = 0; i < dst->height; ++i)
                for (j = 0; j < dst->width; ++j)
                {
                    int32_t v = src->data.s16[i * src->stride + j] >> shift;
                    if (v > 255) v = 255;
                    if (v < 0) v = 0;
                    dst->data.y[i * dst->stride + j] = v;
                }
        }
    }
}

static void reference_phase(CT_Image dx, CT_Image dy, CT_Image phase)
{
    uint32_t x, y, width, height, dxstride, dystride, phasestride;

    ASSERT(dx && dy && phase);
    ASSERT(dx->format == VX_DF_IMAGE_S16 && dy->format == VX_DF_IMAGE_S16 && phase->format == VX_DF_IMAGE_U8);
    ASSERT(dx->width > 0 && dx->height > 0 &&
           dx->width == dy->width && dx->height == dy->height &&
           dx->width == phase->width && dx->height == phase->height);
    width = dx->width;
    height = dy->height;
    dxstride = ct_stride_bytes(dx);
    dystride = ct_stride_bytes(dy);
    phasestride = ct_stride_bytes(phase);

    for( y = 0; y < height; y++ )
    {
        const int16_t* dxptr = (const int16_t*)(dx->data.y + y*dxstride);
        const int16_t* dyptr = (const int16_t*)(dy->data.y + y*dystride);
        uint8_t* phaseptr = (uint8_t*)(phase->data.y + y*phasestride);
        for( x = 0; x < width; x++ )
        {
            double val = atan2(dyptr[x], dxptr[x])*256/(3.1415926535897932384626433832795*2);
            int ival;
            if( val < 0 )
                val += 256.;
            ival = (int)floor(val + 0.5);
            if( ival >= 256 )
                ival -= 256;
            phaseptr[x] = CT_CAST_U8(ival);
        }
    }
}

static void reference_sequential_phase(CT_Image dx0, CT_Image dy0, CT_Image dx1, CT_Image dy1,
                            CT_Image virt1, CT_Image virt2, CT_Image virt3, CT_Image virt4, CT_Image phase)
{
   reference_phase(dx0, dy0, virt1);
   reference_phase(dx1, dy1, virt2);
   referenceConvertDepth(virt1, virt3, 0, VX_CONVERT_POLICY_SATURATE);
   referenceConvertDepth(virt2, virt4, 0, VX_CONVERT_POLICY_SATURATE);
   reference_phase(virt3, virt4, phase);
}

TESTCASE(tivxPhase, CT_VXContext, ct_setup_vx_context, 0)

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

#define PHASE_TEST_CASE(imm, sob, tp) \
    {#imm "/" #sob "/" #tp, CT_##imm##_MODE, sob, VX_DF_IMAGE_##tp}

TEST_WITH_ARG(tivxPhase, testOnRandom, format_arg,
              PHASE_TEST_CASE(Graph, Random, S16),
              )
{
    int dxformat = arg_->format;
    int mode = arg_->mode;
    int srcformat = dxformat == VX_DF_IMAGE_S16 ? VX_DF_IMAGE_U8 : -1;
    int phaseformat = dxformat == VX_DF_IMAGE_S16 ? VX_DF_IMAGE_U8 : -1;
    vx_image dx_node1=0, dy_node1=0, dx_node2=0, dy_node2=0, phase=0;
    vx_image dx_node6=0, dy_node6=0, dx_node7=0, dy_node7=0, phase_branch2=0;
    vx_image virt1, virt2, virt3, virt4, intimage1, intimage2, intimage3, intimage4;
    CT_Image dx0, dy0, dx1, dy1, phase0, phase1, phaseref, virt_ctimage1, virt_ctimage2, virt_ctimage3, virt_ctimage4;
    CT_Image int_ref1, int_ref2, int_ref3, int_ref4;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0;
    vx_node node6 = 0, node7 = 0, node8 = 0, node9 = 0, node10 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_node4, perf_node5, perf_graph;
    vx_context context = context_->vx_context_;
    int iter, niters = 20;
    uint64_t rng;
    int dxmin = -32768, dxmax = 32768;
    vx_border_t border;
    vx_scalar shift_convertdepth = 0;
    vx_int32 sh = 0;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    ASSERT( srcformat != -1 && phaseformat != -1 );
    rng = CT()->seed_;
    border.mode = VX_BORDER_REPLICATE;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
                        vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER,
                                              &border, sizeof(border)));

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

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
        int extreme_vals[] = { dxmin, 0, dxmax };
        ASSERT_NO_FAILURE(dx0 = ct_allocate_ct_image_random(width, height, dxformat, &rng, dxmin, dxmax));
        ASSERT_NO_FAILURE(dy0 = ct_allocate_ct_image_random(width, height, dxformat, &rng, dxmin, dxmax));
        ASSERT_NO_FAILURE(dx1 = ct_allocate_ct_image_random(width, height, dxformat, &rng, dxmin, dxmax));
        ASSERT_NO_FAILURE(dy1 = ct_allocate_ct_image_random(width, height, dxformat, &rng, dxmin, dxmax));

        // add some extreme points to the generated Images
        for( k = 0; k < maxk; k++ )
        {
            int x = CT_RNG_NEXT_INT(rng, 0, width);
            int y = CT_RNG_NEXT_INT(rng, 0, height);
            int dxval = extreme_vals[CT_RNG_NEXT_INT(rng, 0, 3)];
            int dyval = extreme_vals[CT_RNG_NEXT_INT(rng, 0, 3)];
            dx0->data.s16[dx0->stride*y + x] = (int16_t)dxval;
            dy0->data.s16[dy0->stride*y + x] = (int16_t)dyval;
            dx1->data.s16[dx1->stride*y + x] = (int16_t)dxval;
            dy1->data.s16[dy1->stride*y + x] = (int16_t)dyval;
        }

        dx_node1 = ct_image_to_vx_image(dx0, context);
        ASSERT_VX_OBJECT(dx_node1, VX_TYPE_IMAGE);
        dx_node6 = ct_image_to_vx_image(dx0, context);
        ASSERT_VX_OBJECT(dx_node6, VX_TYPE_IMAGE);
        dy_node1 = ct_image_to_vx_image(dy0, context);
        ASSERT_VX_OBJECT(dy_node1, VX_TYPE_IMAGE);
        dy_node6 = ct_image_to_vx_image(dy0, context);
        ASSERT_VX_OBJECT(dy_node6, VX_TYPE_IMAGE);

        dx_node2 = ct_image_to_vx_image(dx1, context);
        ASSERT_VX_OBJECT(dx_node2, VX_TYPE_IMAGE);
        dx_node7 = ct_image_to_vx_image(dx1, context);
        ASSERT_VX_OBJECT(dx_node7, VX_TYPE_IMAGE);
        dy_node2 = ct_image_to_vx_image(dy1, context);
        ASSERT_VX_OBJECT(dy_node2, VX_TYPE_IMAGE);
        dy_node7 = ct_image_to_vx_image(dy1, context);
        ASSERT_VX_OBJECT(dy_node7, VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(shift_convertdepth = vxCreateScalar(context, VX_TYPE_INT32, &sh), VX_TYPE_SCALAR);

        phase = vxCreateImage(context, width, height, phaseformat);
        ASSERT_VX_OBJECT(phase, VX_TYPE_IMAGE);

        phase_branch2 = vxCreateImage(context, width, height, phaseformat);
        ASSERT_VX_OBJECT(phase_branch2, VX_TYPE_IMAGE);

        graph = vxCreateGraph(context);
        ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, phaseformat), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, phaseformat), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(virt3   = vxCreateVirtualImage(graph, 0, 0, dxformat), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(virt4   = vxCreateVirtualImage(graph, 0, 0, dxformat), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(intimage1   = vxCreateImage(context, width, height, phaseformat), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(intimage2   = vxCreateImage(context, width, height, phaseformat), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(intimage3   = vxCreateImage(context, width, height, dxformat), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(intimage4   = vxCreateImage(context, width, height, dxformat), VX_TYPE_IMAGE);

        node1 = vxPhaseNode(graph, dx_node1, dy_node1, virt1);
        ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
        node2 = vxPhaseNode(graph, dx_node2, dy_node2, virt2);
        ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);
        node3 = vxConvertDepthNode(graph, virt1, virt3, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
        ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);
        node4 = vxConvertDepthNode(graph, virt2, virt4, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
        ASSERT_VX_OBJECT(node4, VX_TYPE_NODE);
        node5 = vxPhaseNode(graph, virt3, virt4, phase);
        ASSERT_VX_OBJECT(node5, VX_TYPE_NODE);

        node6 = vxPhaseNode(graph, dx_node6, dy_node6, intimage1);
        ASSERT_VX_OBJECT(node6, VX_TYPE_NODE);
        node7 = vxPhaseNode(graph, dx_node7, dy_node7, intimage2);
        ASSERT_VX_OBJECT(node7, VX_TYPE_NODE);
        node8 = vxConvertDepthNode(graph, intimage1, intimage3, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
        ASSERT_VX_OBJECT(node8, VX_TYPE_NODE);
        node9 = vxConvertDepthNode(graph, intimage2, intimage4, VX_CONVERT_POLICY_SATURATE, shift_convertdepth);
        ASSERT_VX_OBJECT(node9, VX_TYPE_NODE);
        node10 = vxPhaseNode(graph, intimage3, intimage4, phase_branch2);
        ASSERT_VX_OBJECT(node10, VX_TYPE_NODE);
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
        ASSERT_EQ_INT(valid_rect, vx_false_e);

        vxGetValidRegionImage(dx_node1, &src_rect);
        vxGetValidRegionImage(phase_branch2, &dst_rect);

        ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), width);
        ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), height);

        ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), width);
        ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), height);

        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
        vxQueryNode(node4, VX_NODE_PERFORMANCE, &perf_node4, sizeof(perf_node4));
        vxQueryNode(node5, VX_NODE_PERFORMANCE, &perf_node5, sizeof(perf_node5));
        vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

        ASSERT_NO_FAILURE(phase0 = ct_allocate_image(width, height, phaseformat));
        ASSERT_NO_FAILURE(virt_ctimage1 = ct_allocate_image(width, height, phaseformat));
        ASSERT_NO_FAILURE(virt_ctimage2 = ct_allocate_image(width, height, phaseformat));
        ASSERT_NO_FAILURE(virt_ctimage3 = ct_allocate_image(width, height, dxformat));
        ASSERT_NO_FAILURE(virt_ctimage4 = ct_allocate_image(width, height, dxformat));

        reference_phase(dx0, dy0, virt_ctimage1);
        reference_phase(dx1, dy1, virt_ctimage2);

        int_ref1 = ct_image_from_vx_image(intimage1);
        int_ref2 = ct_image_from_vx_image(intimage2);

        // Verifying all intermediate images
        ASSERT_CTIMAGE_NEARWRAP(int_ref1, virt_ctimage1, 1, 0);
        ASSERT_CTIMAGE_NEARWRAP(int_ref2, virt_ctimage2, 1, 0);

        referenceConvertDepth(int_ref1, virt_ctimage3, 0, VX_CONVERT_POLICY_SATURATE);
        referenceConvertDepth(int_ref2, virt_ctimage4, 0, VX_CONVERT_POLICY_SATURATE);

        int_ref3 = ct_image_from_vx_image(intimage3);
        int_ref4 = ct_image_from_vx_image(intimage4);

        // Verifying all intermediate images
        ASSERT_CTIMAGE_NEARWRAP(int_ref3, virt_ctimage3, 1, 0);
        ASSERT_CTIMAGE_NEARWRAP(int_ref4, virt_ctimage4, 1, 0);

        reference_phase(int_ref3, int_ref4, phase0);

        phase1 = ct_image_from_vx_image(phase);
        phaseref = ct_image_from_vx_image(phase_branch2);

        ASSERT_CTIMAGE_NEARWRAP(phase0, phase1, 1, 0);
        ASSERT_CTIMAGE_NEARWRAP(phaseref, phase1, 1, 0);

        VX_CALL(vxReleaseImage(&dx_node1));
        VX_CALL(vxReleaseImage(&dx_node2));
        VX_CALL(vxReleaseImage(&dy_node1));
        VX_CALL(vxReleaseImage(&dy_node2));
        VX_CALL(vxReleaseImage(&dx_node6));
        VX_CALL(vxReleaseImage(&dx_node7));
        VX_CALL(vxReleaseImage(&dy_node6));
        VX_CALL(vxReleaseImage(&dy_node7));
        VX_CALL(vxReleaseImage(&phase));
        VX_CALL(vxReleaseImage(&phase_branch2));
        VX_CALL(vxReleaseImage(&virt1));
        VX_CALL(vxReleaseImage(&virt2));
        VX_CALL(vxReleaseImage(&virt3));
        VX_CALL(vxReleaseImage(&virt4));
        VX_CALL(vxReleaseImage(&intimage1));
        VX_CALL(vxReleaseImage(&intimage2));
        VX_CALL(vxReleaseImage(&intimage3));
        VX_CALL(vxReleaseImage(&intimage4));
        VX_CALL(vxReleaseNode(&node1));
        VX_CALL(vxReleaseNode(&node2));
        VX_CALL(vxReleaseNode(&node3));
        VX_CALL(vxReleaseNode(&node4));
        VX_CALL(vxReleaseNode(&node5));
        VX_CALL(vxReleaseNode(&node6));
        VX_CALL(vxReleaseNode(&node7));
        VX_CALL(vxReleaseNode(&node8));
        VX_CALL(vxReleaseNode(&node9));
        VX_CALL(vxReleaseNode(&node10));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseScalar(&shift_convertdepth));
        ASSERT(node1 == 0 && node2 == 0 && node3 == 0 && node4 == 0 && node5 == 0 && graph == 0);
        ASSERT(node6 == 0 && node7 == 0 && node8 == 0 && node9 == 0 && node10 == 0);
        CT_CollectGarbage(CT_GC_IMAGE);

        printPerformance(perf_node1, width*height, "N1");
        printPerformance(perf_node2, width*height, "N2");
        printPerformance(perf_node3, width*height, "N3");
        printPerformance(perf_node4, width*height, "N4");
        printPerformance(perf_node5, width*height, "N5");
        printPerformance(perf_graph, width*height, "G1");
    }
}

TESTCASE_TESTS(tivxPhase, testOnRandom)
