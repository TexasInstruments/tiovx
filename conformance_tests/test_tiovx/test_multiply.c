/* 
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include "test_tiovx_engine/test.h"

#include <VX/vx.h>

#include <math.h>

#ifdef _MSC_VER
#define ONE_255 (1.0f/255)
#else
#define ONE_255 0x1.010102p-8f
#endif
#define ONE_2_0 1.0f
#define ONE_2_1 (1.0f/(1<<1))
#define ONE_2_2 (1.0f/(1<<2))
#define ONE_2_3 (1.0f/(1<<3))
#define ONE_2_4 (1.0f/(1<<4))
#define ONE_2_5 (1.0f/(1<<5))
#define ONE_2_6 (1.0f/(1<<6))
#define ONE_2_7 (1.0f/(1<<7))
#define ONE_2_8 (1.0f/(1<<8))
#define ONE_2_9 (1.0f/(1<<9))
#define ONE_2_10 (1.0f/(1<<10))
#define ONE_2_11 (1.0f/(1<<11))
#define ONE_2_12 (1.0f/(1<<12))
#define ONE_2_13 (1.0f/(1<<13))
#define ONE_2_14 (1.0f/(1<<14))
#define ONE_2_15 (1.0f/(1<<15))

#define ONE_255_STR "(1/255)"
#define ONE_2_0_STR "(1/2^0)"
#define ONE_2_1_STR "(1/2^1)"
#define ONE_2_2_STR "(1/2^2)"
#define ONE_2_3_STR "(1/2^3)"
#define ONE_2_4_STR "(1/2^4)"
#define ONE_2_5_STR "(1/2^5)"
#define ONE_2_6_STR "(1/2^6)"
#define ONE_2_7_STR "(1/2^7)"
#define ONE_2_8_STR "(1/2^8)"
#define ONE_2_9_STR "(1/2^9)"
#define ONE_2_10_STR "(1/2^10)"
#define ONE_2_11_STR "(1/2^11)"
#define ONE_2_12_STR "(1/2^12)"
#define ONE_2_13_STR "(1/2^13)"
#define ONE_2_14_STR "(1/2^14)"
#define ONE_2_15_STR "(1/2^15)"

//#define CT_EXECUTE_ASYNC

static void referenceMultiply(CT_Image src0, CT_Image src1, CT_Image dst, CT_Image dst_plus_1, vx_float32 scale, enum vx_convert_policy_e policy)
{
    int32_t min_bound, max_bound;
    uint32_t i, j;

    ASSERT(src0 && src1 && dst && dst_plus_1);
    ASSERT(src0->width  == src1->width  && src0->width  == dst->width  && src0->width  == dst_plus_1->width);
    ASSERT(src0->height == src1->height && src0->height == dst->height && src0->height == dst_plus_1->height);
    ASSERT(dst->format == dst_plus_1->format);

    switch (policy)
    {
        case VX_CONVERT_POLICY_SATURATE:
            if (dst->format == VX_DF_IMAGE_U8)
            {
                min_bound = 0;
                max_bound = 255;
            }
            else if (dst->format == VX_DF_IMAGE_S16)
            {
                min_bound = -32768;
                max_bound =  32767;
            }
            else
                FAIL("Unsupported result format: (%.4s)", &dst->format);
            break;
        case VX_CONVERT_POLICY_WRAP:
            min_bound = INT32_MIN;
            max_bound = INT32_MAX;
            break;
        default: FAIL("Unknown owerflow policy"); break;
    };

#define MULTIPLY_LOOP(s0, s1, r)                                                                                \
    do{                                                                                                         \
        for (i = 0; i < dst->height; ++i)                                                                       \
            for (j = 0; j < dst->width; ++j)                                                                    \
            {                                                                                                   \
                int32_t val0 = src0->data.s0[i * src0->stride + j];                                             \
                int32_t val1 = src1->data.s1[i * src1->stride + j];                                             \
                /* use double precision because in S16*S16 case (val0*val1) can be not representable as float */\
                int32_t res0 = (int32_t)floor(((double)(val0 * val1)) * scale);                                 \
                int32_t res1 = res0 + 1;                                                                        \
                dst->data.r[i * dst->stride + j] = (res0 < min_bound ? min_bound :                              \
                                                                        (res0 > max_bound ? max_bound : res0)); \
                dst_plus_1->data.r[i * dst_plus_1->stride + j] = (res1 < min_bound ? min_bound :                \
                                                                        (res1 > max_bound ? max_bound : res1)); \
            }                                                                                                   \
    }while(0)

    if (src0->format == VX_DF_IMAGE_U8 && src1->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_U8)
        MULTIPLY_LOOP(y, y, y);
    else if (src0->format == VX_DF_IMAGE_U8 && src1->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_S16)
        MULTIPLY_LOOP(y, y, s16);
    else if (src0->format == VX_DF_IMAGE_U8 && src1->format == VX_DF_IMAGE_S16 && dst->format == VX_DF_IMAGE_S16)
        MULTIPLY_LOOP(y, s16, s16);
    else if (src0->format == VX_DF_IMAGE_S16 && src1->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_S16)
        MULTIPLY_LOOP(s16, y, s16);
    else if (src0->format == VX_DF_IMAGE_S16 && src1->format == VX_DF_IMAGE_S16 && dst->format == VX_DF_IMAGE_S16)
        MULTIPLY_LOOP(s16, s16, s16);
    else
        FAIL("Unsupported combination of argument formats: %.4s + %.4s = %.4s", &src0->format, &src1->format, &dst->format);

#undef MULTIPLY_LOOP
}

static void referenceSequentialMultiply(CT_Image src0, CT_Image src1, CT_Image src2, 
                CT_Image src3, CT_Image virt1, CT_Image virt2, CT_Image virt1_converted, CT_Image virt2_converted, CT_Image virt1_plus1, CT_Image virt2_plus1,
                CT_Image dst, CT_Image dst_plus_1, vx_float32 scale, enum vx_convert_policy_e policy)
{
    ASSERT_NO_FAILURE(referenceMultiply(src0, src1, virt1, virt1_plus1, scale, policy));
    ASSERT_NO_FAILURE(referenceMultiply(src2, src3, virt2, virt2_plus1, scale, policy));
    ASSERT_NO_FAILURE(referenceMultiply(virt1_converted, virt2_converted, dst, dst_plus_1, scale, policy));

//        referenceMultiply(ref1, ref2, refdst, refdst_plus_1, arg_->scale, arg_->overflow_policy);
}

typedef struct {
    const char* name;
    enum vx_convert_policy_e overflow_policy;
    int width, height;
    vx_df_image arg1_format, arg2_format, result_format;
    enum vx_round_policy_e round_policy;
    vx_float32 scale;
} fuzzy_arg;

#define FUZZY_ARG(owp, w, h, f1, f2, fr, rp, scale)                         \
    ARG(#owp "/" #rp " " #w "x" #h " " #f1 "*" #f2 "*" scale##_STR "=" #fr, \
        VX_CONVERT_POLICY_##owp, w, h, VX_DF_IMAGE_##f1, VX_DF_IMAGE_##f2, VX_DF_IMAGE_##fr, VX_ROUND_POLICY_##rp, scale)

#define APPEND_SCALE(macro, ...)                                \
    CT_EXPAND(macro(__VA_ARGS__, TO_ZERO, ONE_2_0)),            \
    CT_EXPAND(macro(__VA_ARGS__, TO_ZERO, ONE_2_1)),            \
    CT_EXPAND(macro(__VA_ARGS__, TO_ZERO, ONE_2_8)),            \
    CT_EXPAND(macro(__VA_ARGS__, TO_ZERO, ONE_2_15))

#define MUL_FUZZY_ARGS(owp)                                 \
    APPEND_SCALE(FUZZY_ARG, owp, 18, 18, U8, U8, U8),     \
    APPEND_SCALE(FUZZY_ARG, owp, 644, 258, U8, U8, S16),    \
    APPEND_SCALE(FUZZY_ARG, owp, 1600, 1200, S16, S16, S16)

TESTCASE(vxMultiply,  CT_VXContext, ct_setup_vx_context, 0)


TEST_WITH_ARG(vxMultiply, testFuzzy, fuzzy_arg, MUL_FUZZY_ARGS(SATURATE), MUL_FUZZY_ARGS(WRAP))
{
    vx_image src1, src2, src3, src4, dst, virt1, virt2;
    vx_image dst_graph2, virt1_graph2, virt2_graph2;
    vx_graph graph1, graph2;
    vx_scalar scale = 0;
    CT_Image ref1, ref2, ref3, ref4, refdst, refdst_plus_1, vxdst, vxdst_graph2, vxint1, vxint2;
    CT_Image virt_ctimage1, virt_ctimage2, virt_ctimage1_plus_1, virt_ctimage2_plus_1;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_node node1_graph2 = 0, node2_graph2 = 0, node3_graph2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_graph1;
    vx_perf_t perf_node1_graph2, perf_node2_graph2, perf_node3_graph2, perf_graph2;

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateImage(context, arg_->width, arg_->height, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateImage(context, arg_->width, arg_->height, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->scale), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(virt1_graph2   = vxCreateVirtualImage(graph2, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2_graph2   = vxCreateVirtualImage(graph2, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_graph2   = vxCreateImage(context, arg_->width, arg_->height, arg_->result_format), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg2_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg2_format),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &tiovx()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &tiovx()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &tiovx()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &tiovx()->seed_));

    // build one-node graph
    ASSERT_VX_OBJECT(node1 = vxMultiplyNode(graph1, src1, src2, scale, arg_->overflow_policy, arg_->round_policy, virt1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxMultiplyNode(graph1, src3, src4, scale, arg_->overflow_policy, arg_->round_policy, virt2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxMultiplyNode(graph1, virt1, virt2, scale, arg_->overflow_policy, arg_->round_policy, dst), VX_TYPE_NODE);

    // build one-node graph
    ASSERT_VX_OBJECT(node1_graph2 = vxMultiplyNode(graph2, src1, src2, scale, arg_->overflow_policy, arg_->round_policy, virt1_graph2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2_graph2 = vxMultiplyNode(graph2, src3, src4, scale, arg_->overflow_policy, arg_->round_policy, virt2_graph2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3_graph2 = vxMultiplyNode(graph2, virt1_graph2, virt2_graph2, scale, arg_->overflow_policy, arg_->round_policy, dst_graph2), VX_TYPE_NODE);

    // run graph
#ifdef CT_EXECUTE_ASYNC
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph2));
#else
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph2));
#endif

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));

    vxQueryNode(node1_graph2, VX_NODE_PERFORMANCE, &perf_node1_graph2, sizeof(perf_node1_graph2));
    vxQueryNode(node2_graph2, VX_NODE_PERFORMANCE, &perf_node2_graph2, sizeof(perf_node2_graph2));
    vxQueryNode(node3_graph2, VX_NODE_PERFORMANCE, &perf_node3_graph2, sizeof(perf_node3_graph2));
    vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));

    ASSERT_NO_FAILURE({
        ref1  = ct_image_from_vx_image(src1);
        ref2  = ct_image_from_vx_image(src2);
        ref3  = ct_image_from_vx_image(src3);
        ref4  = ct_image_from_vx_image(src4);
        vxint1 = ct_image_from_vx_image(virt1);
        vxint2 = ct_image_from_vx_image(virt2);
        vxdst = ct_image_from_vx_image(dst);
        vxdst_graph2 = ct_image_from_vx_image(dst_graph2);
        refdst        = ct_allocate_image(arg_->width, arg_->height, arg_->result_format);
        refdst_plus_1 = ct_allocate_image(arg_->width, arg_->height, arg_->result_format);
        virt_ctimage1 = ct_allocate_image(arg_->width, arg_->height, arg_->result_format);
        virt_ctimage2 = ct_allocate_image(arg_->width, arg_->height, arg_->result_format);
        virt_ctimage1_plus_1 = ct_allocate_image(arg_->width, arg_->height, arg_->result_format);
        virt_ctimage2_plus_1 = ct_allocate_image(arg_->width, arg_->height, arg_->result_format);

        referenceSequentialMultiply(ref1, ref2, ref3, ref4, virt_ctimage1, virt_ctimage2, vxint1, vxint2,
                               virt_ctimage1_plus_1, virt_ctimage2_plus_1, refdst, refdst_plus_1,
                               arg_->scale, arg_->overflow_policy);
    });

    if (arg_->scale == ONE_2_0)
    {
        ASSERT_EQ_CTIMAGE(virt_ctimage1, vxint1);
        ASSERT_EQ_CTIMAGE(virt_ctimage2, vxint2);
        ASSERT_EQ_CTIMAGE(refdst, vxdst);
        ASSERT_EQ_CTIMAGE(refdst, vxdst_graph2);
    }
    else
    {
        // (|ref-v| <= 1 && |ref+1-v| <= 1)  is equivalent to (v == ref || v == ref + 1)
        if (arg_->overflow_policy == VX_CONVERT_POLICY_WRAP)
        {
            EXPECT_CTIMAGE_NEARWRAP(virt_ctimage1, vxint1, 1, CTIMAGE_ALLOW_WRAP);
            EXPECT_CTIMAGE_NEARWRAP(virt_ctimage1_plus_1, vxint1, 1, CTIMAGE_ALLOW_WRAP);
            EXPECT_CTIMAGE_NEARWRAP(virt_ctimage2, vxint2, 1, CTIMAGE_ALLOW_WRAP);
            EXPECT_CTIMAGE_NEARWRAP(virt_ctimage2_plus_1, vxint2, 1, CTIMAGE_ALLOW_WRAP);
            EXPECT_CTIMAGE_NEARWRAP(refdst, vxdst, 1, CTIMAGE_ALLOW_WRAP);
            EXPECT_CTIMAGE_NEARWRAP(refdst_plus_1, vxdst, 1, CTIMAGE_ALLOW_WRAP);
            EXPECT_CTIMAGE_NEARWRAP(refdst, vxdst_graph2, 1, CTIMAGE_ALLOW_WRAP);
            EXPECT_CTIMAGE_NEARWRAP(refdst_plus_1, vxdst_graph2, 1, CTIMAGE_ALLOW_WRAP);
        }
        else
        {
            EXPECT_CTIMAGE_NEAR(virt_ctimage1, vxint1, 1);
            EXPECT_CTIMAGE_NEAR(virt_ctimage1_plus_1, vxint1, 1);
            EXPECT_CTIMAGE_NEAR(virt_ctimage2, vxint2, 1);
            EXPECT_CTIMAGE_NEAR(virt_ctimage2_plus_1, vxint2, 1);
            EXPECT_CTIMAGE_NEAR(refdst, vxdst, 1);
            EXPECT_CTIMAGE_NEAR(refdst_plus_1, vxdst, 1);
            EXPECT_CTIMAGE_NEAR(refdst, vxdst_graph2, 1);
            EXPECT_CTIMAGE_NEAR(refdst_plus_1, vxdst_graph2, 1);
        }
    }

    VX_CALL(vxReleaseImage(&virt1_graph2));
    VX_CALL(vxReleaseImage(&virt2_graph2));
    VX_CALL(vxReleaseImage(&dst_graph2));
    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseScalar(&scale));
    VX_CALL(vxReleaseNode(&node1_graph2));
    VX_CALL(vxReleaseNode(&node2_graph2));
    VX_CALL(vxReleaseNode(&node3_graph2));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph1));

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3, arg_->width*arg_->height, "N3");
    printPerformance(perf_graph1, arg_->width*arg_->height, "G1");

    printPerformance(perf_node1_graph2, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2_graph2, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3_graph2, arg_->width*arg_->height, "N3");
    printPerformance(perf_graph2, arg_->width*arg_->height, "G1");
}

TESTCASE_TESTS(vxMultiply, testFuzzy)
