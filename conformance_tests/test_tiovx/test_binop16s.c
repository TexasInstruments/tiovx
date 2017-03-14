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


#include "test_tiovx.h"

#include <VX/vx.h>

//#define CT_EXECUTE_ASYNC

static void referenceAbsDiffSingle(CT_Image src0, CT_Image src1, CT_Image dst)
{
    uint32_t i, j;

    ASSERT(src0 && src1 && dst);
    ASSERT(src0->width = src1->width && src0->width == dst->width);
    ASSERT(src0->height = src1->height && src0->height == dst->height);
    ASSERT(src0->format == src1->format && src1->format == VX_DF_IMAGE_S16);
    ASSERT(dst->format == VX_DF_IMAGE_S16 || dst->format == VX_DF_IMAGE_U16);

    if (dst->format == VX_DF_IMAGE_S16)
    {
        for (i = 0; i < dst->height; ++i)
        {
            for (j = 0; j < dst->width; ++j)
            {
                int32_t val = src0->data.s16[i * src0->stride + j] - src1->data.s16[i * src1->stride + j];
                val = val < 0 ? -val : val;
                dst->data.s16[i * dst->stride + j] = CT_CAST_S16(val);
            }
        }
    }
    else if (dst->format == VX_DF_IMAGE_U16)
    {
        for (i = 0; i < dst->height; ++i)
        {
            for (j = 0; j < dst->width; ++j)
            {
                int32_t val = src0->data.s16[i * src0->stride + j] - src1->data.s16[i * src1->stride + j];
                val = val < 0 ? -val : val;
                dst->data.u16[i * dst->stride + j] = CT_CAST_U16(val);
            }
        }
    }
}

static void referenceAbsDiff(CT_Image src0, CT_Image src1, CT_Image src2, CT_Image src3, CT_Image virt1, CT_Image virt2, CT_Image dst)
{
    ASSERT_NO_FAILURE(referenceAbsDiffSingle(src0, src1, virt1));
    ASSERT_NO_FAILURE(referenceAbsDiffSingle(src2, src3, virt2));
    ASSERT_NO_FAILURE(referenceAbsDiffSingle(virt1, virt2, dst));
}

typedef vx_node   (VX_API_CALL *vxBinopFunction) (vx_graph, vx_image, vx_image, vx_image);
typedef void      (*referenceFunction)(CT_Image, CT_Image, CT_Image, CT_Image, CT_Image, CT_Image, CT_Image);

TESTCASE(vxBinOp16s,  CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    uint32_t width;
    uint32_t height;
    vxBinopFunction   vxFunc;
    referenceFunction referenceFunc;
} fuzzy_arg;

#define FUZZY_ARG(func,w,h) ARG(#func ": " #w "x" #h, w, h, vx##func##Node, reference##func)

#define BINOP_SIZE_ARGS(func)       \
    FUZZY_ARG(func, 18, 18),        \
    FUZZY_ARG(func, 644, 258),      \
    FUZZY_ARG(func, 1600, 1200),    \
    ARG_EXTENDED_BEGIN(),           \
    FUZZY_ARG(func, 1, 1),          \
    FUZZY_ARG(func, 15, 17),        \
    FUZZY_ARG(func, 32, 32),        \
    FUZZY_ARG(func, 1231, 1234),    \
    FUZZY_ARG(func, 1280, 720),     \
    FUZZY_ARG(func, 1920, 1080),    \
    ARG_EXTENDED_END()

TEST_WITH_ARG(vxBinOp16s, testFuzzy, fuzzy_arg, BINOP_SIZE_ARGS(AbsDiff))
{
    vx_image src1, src2, src3, src4, dst, virt1, virt2;
    vx_graph graph;
    CT_Image ref1, ref2, ref3, ref4, refdst, vxdst, virt_ctimage1, virt_ctimage2;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_graph;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_S16),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));

    virt_ctimage1 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_S16);
    virt_ctimage2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_S16);

    // build one-node graph
    ASSERT_VX_OBJECT(node1 = arg_->vxFunc(graph, src1, src2, virt1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = arg_->vxFunc(graph, src3, src4, virt2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = arg_->vxFunc(graph, virt1, virt2, dst), VX_TYPE_NODE);

    // run graph
#ifdef CT_EXECUTE_ASYNC
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ref1  = ct_image_from_vx_image(src1);
    ref2  = ct_image_from_vx_image(src2);
    ref3  = ct_image_from_vx_image(src3);
    ref4  = ct_image_from_vx_image(src4);
    vxdst = ct_image_from_vx_image(dst);
    refdst = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_S16);

    arg_->referenceFunc(ref1, ref2, ref3, ref4, virt_ctimage1, virt_ctimage2, refdst);

    ASSERT_EQ_CTIMAGE(refdst, vxdst);

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");

    printPerformance(perf_node2, arg_->width*arg_->height, "N2");

    printPerformance(perf_node3, arg_->width*arg_->height, "N3");

    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TESTCASE_TESTS(vxBinOp16s, testFuzzy)
