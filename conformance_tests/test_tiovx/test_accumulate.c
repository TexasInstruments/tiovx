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
#include <VX/vxu.h>

TESTCASE(Accumulate, CT_VXContext, ct_setup_vx_context, 0)

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
        // according to spec the policy is ignored
        // if (policy == VX_CONVERT_POLICY_WRAP)
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
        // else if (VX_CONVERT_POLICY_SATURATE)
        // {
        //     // up-conversion + saturate
        //     if (shift < 0)
        //     {
        //         for (i = 0; i < dst->height; ++i)
        //             for (j = 0; j < dst->width; ++j)
        //                 dst->data.s16[i * dst->stride + j] = ((unsigned)src->data.y[i * src->stride + j]) >> (-shift);
        //     }
        //     else
        //     {
        //         for (i = 0; i < dst->height; ++i)
        //             for (j = 0; j < dst->width; ++j)
        //             {
        //                 unsigned v = ((unsigned)src->data.y[i * src->stride + j]) << shift;
        //                 if (v > 32767) v = 32767;
        //                 dst->data.s16[i * dst->stride + j] = v;
        //             }
        //     }
        // }
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

static CT_Image accumulate_generate_random_8u(int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}


static CT_Image accumulate_generate_random_16s(int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_S16, &CT()->seed_, -32768, 32768));

    return image;
}


static void accumulate_reference(CT_Image input, CT_Image accum)
{
    CT_FILL_IMAGE_16S(return, accum,
            {
                uint8_t* input_data = CT_IMAGE_DATA_PTR_8U(input, x, y);
                int32_t res32 = ((int32_t)(*dst_data)) + ((int32_t)(*input_data));
                int16_t res = CT_SATURATE_S16(res32);
                *dst_data = res;
            });
}


static void accumulate_check(CT_Image input, CT_Image accum_src, CT_Image accum_dst)
{
    CT_Image accum_ref = NULL;

    ASSERT(input && accum_src && accum_dst);

    ASSERT_NO_FAILURE(accum_ref = ct_image_create_clone(accum_src));

    ASSERT_NO_FAILURE(accumulate_reference(input, accum_ref));

    EXPECT_EQ_CTIMAGE(accum_ref, accum_dst);
#if 0
    if (CT_HasFailure())
    {
        printf("=== Input ===\n");
        ct_dump_image_info(input);
        printf("=== Accum source ===\n");
        ct_dump_image_info(accum_src);
        printf("=== Accum RESULT ===\n");
        ct_dump_image_info(accum_dst);
        printf("=== EXPECTED RESULT ===\n");
        ct_dump_image_info(accum_ref);
    }
#endif
}

static void accumulate_chain_check(CT_Image input, CT_Image virtual_img, CT_Image accum_input1, 
                                          CT_Image accum_input2, CT_Image accum_dst)
{
    CT_Image accum_ref = NULL, accum_intermediate = NULL;

    ASSERT(input && virtual_img && accum_input1 && accum_input2 && accum_dst);

    ASSERT_NO_FAILURE(accum_intermediate = ct_image_create_clone(accum_input1));

    ASSERT_NO_FAILURE(accum_ref = ct_image_create_clone(accum_input2));

    ASSERT_NO_FAILURE(accumulate_reference(input, accum_intermediate));

    ASSERT_NO_FAILURE(referenceConvertDepth(accum_intermediate, virtual_img, 0, VX_CONVERT_POLICY_SATURATE));

    ASSERT_NO_FAILURE(accumulate_reference(virtual_img, accum_ref));

    EXPECT_EQ_CTIMAGE(accum_ref, accum_dst);
#if 0
    if (CT_HasFailure())
    {
        printf("=== Input ===\n");
        ct_dump_image_info(input);
        printf("=== Accum source ===\n");
        ct_dump_image_info(accum_src);
        printf("=== Accum RESULT ===\n");
        ct_dump_image_info(accum_dst);
        printf("=== EXPECTED RESULT ===\n");
        ct_dump_image_info(accum_ref);
    }
#endif
}

typedef struct {
    const char* testName;
    int dummy_;
    int width, height;
} Arg;


#define PARAMETERS \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_18x18, ARG, 0), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_644x258, ARG, 0), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_1600x1200, ARG, 0)


TEST_WITH_ARG(Accumulate, testParallelNodes, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image1 = 0, input_image2 = 0, accum_image1 = 0, accum_image2 = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;

    CT_Image input1 = NULL, input2 = NULL, accum_src1 = NULL, accum_src2 = NULL, accum_dst1 = NULL, accum_dst2 = NULL;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_NO_FAILURE(input1 = accumulate_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input2 = accumulate_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_src1 = accumulate_generate_random_16s(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_src2 = accumulate_generate_random_16s(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image1 = ct_image_to_vx_image(input1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image2 = ct_image_to_vx_image(input2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image1 = ct_image_to_vx_image(accum_src1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image2 = ct_image_to_vx_image(accum_src2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxAccumulateImageNode(graph, input_image1, accum_image1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxAccumulateImageNode(graph, input_image2, accum_image2), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(accum_dst1 = ct_image_from_vx_image(accum_image1));

    ASSERT_NO_FAILURE(accum_dst2 = ct_image_from_vx_image(accum_image2));

    ASSERT_NO_FAILURE(accumulate_check(input1, accum_src1, accum_dst1));

    ASSERT_NO_FAILURE(accumulate_check(input2, accum_src2, accum_dst2));

    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&accum_image2));
    VX_CALL(vxReleaseImage(&accum_image1));
    VX_CALL(vxReleaseImage(&input_image2));
    VX_CALL(vxReleaseImage(&input_image1));

    ASSERT(accum_image2 == 0);
    ASSERT(accum_image1 == 0);
    ASSERT(input_image2 == 0);
    ASSERT(input_image1 == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");

    printPerformance(perf_node2, arg_->width*arg_->height, "N2");

    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TEST_WITH_ARG(Accumulate, testSequentialNodes, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image1 = 0, accum_image1 = 0, accum_image2 = 0, virtual_image = 0;
    vx_scalar shift_convertdepth = 0;
    vx_int32 sh = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_graph;

    CT_Image input1 = NULL, accum_src1 = NULL, accum_src2 = NULL, accum_dst = NULL, virtual_ctimage = NULL;

    virtual_ctimage = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(shift_convertdepth = vxCreateScalar(context, VX_TYPE_INT32, &sh), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input1 = accumulate_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_src1 = accumulate_generate_random_16s(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_src2 = accumulate_generate_random_16s(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image1 = ct_image_to_vx_image(input1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image1 = ct_image_to_vx_image(accum_src1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image2 = ct_image_to_vx_image(accum_src2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virtual_image = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxAccumulateImageNode(graph, input_image1, accum_image1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxConvertDepthNode(graph, accum_image1, virtual_image, VX_CONVERT_POLICY_SATURATE, shift_convertdepth), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxAccumulateImageNode(graph, virtual_image, accum_image2), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(accum_dst = ct_image_from_vx_image(accum_image2));

    ASSERT_NO_FAILURE(accumulate_chain_check(input1, virtual_ctimage, accum_src1, accum_src2, accum_dst));

    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&virtual_image));
    VX_CALL(vxReleaseImage(&accum_image2));
    VX_CALL(vxReleaseImage(&accum_image1));
    VX_CALL(vxReleaseImage(&input_image1));
    VX_CALL(vxReleaseScalar(&shift_convertdepth));

    ASSERT(accum_image2 == 0);
    ASSERT(accum_image1 == 0);
    ASSERT(input_image1 == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");

    printPerformance(perf_node2, arg_->width*arg_->height, "N2");

    printPerformance(perf_node3, arg_->width*arg_->height, "N3");

    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TESTCASE_TESTS(Accumulate,
        testParallelNodes,
        testSequentialNodes
)
