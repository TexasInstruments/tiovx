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

#include "test_tiovx_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>

TESTCASE(AccumulateWeighted, CT_VXContext, ct_setup_vx_context, 0)

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

static CT_Image accumulate_weighted_generate_random_8u(int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &tiovx()->seed_, 0, 256));

    return image;
}


static void accumulate_weighted_reference(CT_Image input, vx_float32 alpha, CT_Image accum)
{
    CT_FILL_IMAGE_8U(return, accum,
            {
                uint8_t* input_data = CT_IMAGE_DATA_PTR_8U(input, x, y);
                vx_float32 res = (1 - alpha) * ((vx_float32)(int32_t)(*dst_data)) + (alpha) * ((vx_float32)(int32_t)(*input_data));
                uint8_t res8 = CT_SATURATE_U8(res);
                *dst_data = res8;
            });
}

static void accumulate_weighted_check(CT_Image input, vx_float32 alpha, CT_Image accum_src, CT_Image accum_dst)
{
    CT_Image accum_ref = NULL;

    ASSERT(input && accum_src && accum_dst);

    ASSERT_NO_FAILURE(accum_ref = ct_image_create_clone(accum_src));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(input, alpha, accum_ref));

    EXPECT_CTIMAGE_NEAR(accum_ref, accum_dst, 1);
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

static void accumulate_multiple_weighted_check(CT_Image input, vx_float32 alpha_intermediate, vx_float32 alpha_final, CT_Image accum_src, CT_Image accum_final, CT_Image accum_dst)
{
    CT_Image accum_ref = NULL, accum_intermediate = NULL;

    ASSERT(input && accum_src && accum_final && accum_dst);

    ASSERT_NO_FAILURE(accum_intermediate = ct_image_create_clone(accum_src));

    ASSERT_NO_FAILURE(accum_ref = ct_image_create_clone(accum_final));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(input, alpha_intermediate, accum_intermediate));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(accum_intermediate, alpha_final, accum_ref));

    EXPECT_CTIMAGE_NEAR(accum_ref, accum_dst, 2); // Changed tolerance to 2 for the amount of nodes
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

static void accumulate_not_multiple_weighted_check(CT_Image input_not, CT_Image input_acc, CT_Image virtual_dummy, 
            vx_float32 alpha_intermediate, vx_float32 alpha_final, CT_Image accum_final, CT_Image accum_dst)
{
    CT_Image accum_ref = NULL, accum_intermediate = NULL;

    ASSERT(input_not && input_acc && accum_final && accum_dst);

    ASSERT_NO_FAILURE(accum_ref = ct_image_create_clone(accum_final));

    ASSERT_NO_FAILURE(accum_intermediate = ct_image_create_clone(input_acc));

    ASSERT_NO_FAILURE(referenceNot(input_not, virtual_dummy));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(virtual_dummy, alpha_intermediate, accum_intermediate));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(accum_intermediate, alpha_final, accum_ref));

    EXPECT_CTIMAGE_NEAR(accum_ref, accum_dst, 2); // Changed tolerance to 2
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

static void alternate_node_check(CT_Image input_not, CT_Image input_acc_1, CT_Image input_acc_2, CT_Image virtual_dummy_1,
            CT_Image virtual_dummy_2, CT_Image virtual_dummy_3, vx_float32 alpha_intermediate, vx_float32 alpha_final, 
            CT_Image accum_final, CT_Image accum_dst)
{
    CT_Image accum_ref = NULL, accum_intermediate_1 = NULL, accum_intermediate_2 = NULL;

    ASSERT(input_not && input_acc_1 && input_acc_2 && accum_final && accum_dst);

    ASSERT_NO_FAILURE(accum_ref = ct_image_create_clone(accum_final));

    ASSERT_NO_FAILURE(accum_intermediate_1 = ct_image_create_clone(input_acc_1));

    ASSERT_NO_FAILURE(accum_intermediate_2 = ct_image_create_clone(input_acc_2));

    ASSERT_NO_FAILURE(referenceNot(input_not, virtual_dummy_1));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(virtual_dummy_1, alpha_intermediate, accum_intermediate_1));

    ASSERT_NO_FAILURE(referenceNot(accum_intermediate_1, virtual_dummy_2));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(virtual_dummy_2, alpha_intermediate, accum_intermediate_2));

    ASSERT_NO_FAILURE(referenceNot(accum_intermediate_2, virtual_dummy_3));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(virtual_dummy_3, alpha_final, accum_ref));

    EXPECT_CTIMAGE_NEAR(accum_ref, accum_dst, 2); // Changed tolerance to 2
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
    vx_float32 alpha_intermediate, alpha_final;
    int width, height;
} Arg;


#define PARAMETERS \
    CT_GENERATE_PARAMETERS("random/alphaintermediate0.5f/alphafinal0.25f", ADD_SIZE_18x18, ARG, 0.5f, 0.25f), \
    CT_GENERATE_PARAMETERS("random/alpha0.33f/alphafinal0.67f", ADD_SIZE_644x258, ARG, 0.33f, 0.67f), \
    CT_GENERATE_PARAMETERS("random/alpha0.99f/alphafinal0.8f", ADD_SIZE_1600x1200, ARG, 0.99f, 0.8f)

TEST_WITH_ARG(AccumulateWeighted, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image = 0, accum_image_intermediate = 0, accum_image_final = 0;
    vx_scalar alpha_scalar, alpha_scalar_final = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;

    CT_Image input = NULL, accum_src = NULL, accum_final = NULL, accum_dst = NULL;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(alpha_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar_final = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_src = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_intermediate = ct_image_to_vx_image(accum_src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final = ct_image_to_vx_image(accum_final, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxAccumulateWeightedImageNode(graph, input_image, alpha_scalar, accum_image_intermediate), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxAccumulateWeightedImageNode(graph, accum_image_intermediate, alpha_scalar_final, accum_image_final), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, "DSP-2"));

    VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(accum_dst = ct_image_from_vx_image(accum_image_final));

    ASSERT_NO_FAILURE(accumulate_multiple_weighted_check(input, arg_->alpha_intermediate, arg_->alpha_final, accum_src, accum_final, accum_dst));

    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node2 == 0);
    ASSERT(node1 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&accum_image_final));
    VX_CALL(vxReleaseImage(&accum_image_intermediate));
    VX_CALL(vxReleaseImage(&input_image));
    VX_CALL(vxReleaseScalar(&alpha_scalar_final));
    VX_CALL(vxReleaseScalar(&alpha_scalar));

    ASSERT(accum_image_final == 0);
    ASSERT(accum_image_intermediate == 0);
    ASSERT(input_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");

    printPerformance(perf_node2, arg_->width*arg_->height, "N2");

    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TEST_WITH_ARG(AccumulateWeighted, testParallelNodes, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image1 = 0, input_image2 = 0, accum_image_final1 = 0, accum_image_final2 = 0;
    vx_scalar alpha_scalar1, alpha_scalar2 = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;

    CT_Image input1 = NULL, input2 = NULL, accum_final1 = NULL, accum_final2 = NULL, accum_dst1 = NULL, accum_dst2 = NULL;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(alpha_scalar1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image1 = ct_image_to_vx_image(input1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image2 = ct_image_to_vx_image(input2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final1 = ct_image_to_vx_image(accum_final1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final2 = ct_image_to_vx_image(accum_final2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxAccumulateWeightedImageNode(graph, input_image1, alpha_scalar1, accum_image_final1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxAccumulateWeightedImageNode(graph, input_image2, alpha_scalar2, accum_image_final2), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-2"));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(accum_dst1 = ct_image_from_vx_image(accum_image_final1));

    ASSERT_NO_FAILURE(accum_dst2 = ct_image_from_vx_image(accum_image_final2));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input1, arg_->alpha_intermediate, accum_final1, accum_dst1));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input2, arg_->alpha_final, accum_final2, accum_dst2));

    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node2 == 0);
    ASSERT(node1 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&accum_image_final2));
    VX_CALL(vxReleaseImage(&accum_image_final1));
    VX_CALL(vxReleaseImage(&input_image2));
    VX_CALL(vxReleaseImage(&input_image1));
    VX_CALL(vxReleaseScalar(&alpha_scalar2));
    VX_CALL(vxReleaseScalar(&alpha_scalar1));

    ASSERT(accum_image_final1 == 0);
    ASSERT(accum_image_final2 == 0);
    ASSERT(input_image1 == 0);
    ASSERT(input_image2 == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");

    printPerformance(perf_node2, arg_->width*arg_->height, "N2");

    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TEST_WITH_ARG(AccumulateWeighted, testVirtualDataObject, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image_acc = 0, input_image_not = 0, accum_image_virtual = 0, accum_image_final = 0;
    vx_scalar alpha_scalar, alpha_scalar_final = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_graph;

    CT_Image input_acc = NULL, input_not = NULL, accum_final = NULL, accum_dst = NULL, virtual_dummy = NULL;

    virtual_dummy = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(alpha_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar_final = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input_acc = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input_not = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image_acc = ct_image_to_vx_image(input_acc, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image_not = ct_image_to_vx_image(input_not, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final = ct_image_to_vx_image(accum_final, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    // Must be after vxCreateGraph
    ASSERT_VX_OBJECT(accum_image_virtual = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxNotNode(graph, input_image_not, accum_image_virtual), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxAccumulateWeightedImageNode(graph, accum_image_virtual, alpha_scalar, input_image_acc), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxAccumulateWeightedImageNode(graph, input_image_acc, alpha_scalar_final, accum_image_final), VX_TYPE_NODE);

    // if any or all are on DSP-2, then it fails on PC; w/ or w/out virtual images
    VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node3, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(accum_dst = ct_image_from_vx_image(accum_image_final));

    ASSERT_NO_FAILURE(accumulate_not_multiple_weighted_check(input_not, input_acc, virtual_dummy, arg_->alpha_intermediate, arg_->alpha_final, accum_final, accum_dst));

    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node3 == 0);
    ASSERT(node2 == 0);
    ASSERT(node1 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&accum_image_final));
    VX_CALL(vxReleaseImage(&accum_image_virtual));
    VX_CALL(vxReleaseImage(&input_image_acc));
    VX_CALL(vxReleaseImage(&input_image_not));
    VX_CALL(vxReleaseScalar(&alpha_scalar_final));
    VX_CALL(vxReleaseScalar(&alpha_scalar));

    ASSERT(accum_image_final == 0);
    ASSERT(input_image_acc == 0);
    ASSERT(input_image_not == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");

    printPerformance(perf_node2, arg_->width*arg_->height, "N2");

    printPerformance(perf_node3, arg_->width*arg_->height, "N3");

    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TEST_WITH_ARG(AccumulateWeighted, testParallelGraphs, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image1 = 0, input_image2 = 0, accum_image_final1 = 0, accum_image_final2 = 0;
    vx_scalar alpha_scalar1, alpha_scalar2 = 0;
    vx_graph graph1 = 0, graph2 = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph1, perf_graph2;

    CT_Image input1 = NULL, input2 = NULL, accum_final1 = NULL, accum_final2 = NULL, accum_dst1 = NULL, accum_dst2 = NULL;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(alpha_scalar1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image1 = ct_image_to_vx_image(input1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image2 = ct_image_to_vx_image(input2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final1 = ct_image_to_vx_image(accum_final1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final2 = ct_image_to_vx_image(accum_final2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxAccumulateWeightedImageNode(graph1, input_image1, alpha_scalar1, accum_image_final1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxAccumulateWeightedImageNode(graph2, input_image2, alpha_scalar2, accum_image_final2), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-2"));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph2));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph2));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));

    vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));

    ASSERT_NO_FAILURE(accum_dst1 = ct_image_from_vx_image(accum_image_final1));

    ASSERT_NO_FAILURE(accum_dst2 = ct_image_from_vx_image(accum_image_final2));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input1, arg_->alpha_intermediate, accum_final1, accum_dst1));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input2, arg_->alpha_final, accum_final2, accum_dst2));

    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseGraph(&graph1));

    ASSERT(node2 == 0);
    ASSERT(node1 == 0);
    ASSERT(graph2 == 0);
    ASSERT(graph1 == 0);

    VX_CALL(vxReleaseImage(&accum_image_final2));
    VX_CALL(vxReleaseImage(&accum_image_final1));
    VX_CALL(vxReleaseImage(&input_image2));
    VX_CALL(vxReleaseImage(&input_image1));
    VX_CALL(vxReleaseScalar(&alpha_scalar2));
    VX_CALL(vxReleaseScalar(&alpha_scalar1));

    ASSERT(accum_image_final1 == 0);
    ASSERT(accum_image_final2 == 0);
    ASSERT(input_image1 == 0);
    ASSERT(input_image2 == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");

    printPerformance(perf_node2, arg_->width*arg_->height, "N2");

    printPerformance(perf_graph1, arg_->width*arg_->height, "G1");

    printPerformance(perf_graph2, arg_->width*arg_->height, "G2");
}

TEST_WITH_ARG(AccumulateWeighted, testParallelGraphsMultipleNodes, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image_graph1 = 0, accum_image_intermediate_graph1 = 0, accum_image_final_graph1 = 0;
    vx_image input_image_graph2 = 0, accum_image_intermediate_graph2 = 0, accum_image_final_graph2 = 0;
    vx_scalar alpha_scalar_graph1, alpha_scalar_final_graph1 = 0;
    vx_scalar alpha_scalar_graph2, alpha_scalar_final_graph2 = 0;
    vx_graph graph1 = 0, graph2 = 0;
    vx_node node1_graph1 = 0, node2_graph1 = 0, node1_graph2 = 0, node2_graph2 = 0;
    vx_perf_t perf_node1_graph1, perf_node2_graph1, perf_node1_graph2, perf_node2_graph2, perf_graph1, perf_graph2;

    CT_Image input_graph1 = NULL, accum_src_graph1 = NULL, accum_final_graph1 = NULL, accum_dst_graph1 = NULL;
    CT_Image input_graph2 = NULL, accum_src_graph2 = NULL, accum_final_graph2 = NULL, accum_dst_graph2 = NULL;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(alpha_scalar_graph1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar_graph2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar_final_graph1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar_final_graph2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input_graph1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input_graph2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_src_graph1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_src_graph2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final_graph1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final_graph2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image_graph1 = ct_image_to_vx_image(input_graph1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image_graph2 = ct_image_to_vx_image(input_graph2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_intermediate_graph1 = ct_image_to_vx_image(accum_src_graph1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_intermediate_graph2 = ct_image_to_vx_image(accum_src_graph2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final_graph1 = ct_image_to_vx_image(accum_final_graph1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final_graph2 = ct_image_to_vx_image(accum_final_graph2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1_graph1 = vxAccumulateWeightedImageNode(graph1, input_image_graph1, alpha_scalar_graph1, accum_image_intermediate_graph1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2_graph1 = vxAccumulateWeightedImageNode(graph1, accum_image_intermediate_graph1, alpha_scalar_final_graph1, accum_image_final_graph1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node1_graph2 = vxAccumulateWeightedImageNode(graph2, input_image_graph2, alpha_scalar_graph2, accum_image_intermediate_graph2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2_graph2 = vxAccumulateWeightedImageNode(graph2, accum_image_intermediate_graph2, alpha_scalar_final_graph2, accum_image_final_graph2), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node1_graph1, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node2_graph1, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node1_graph2, VX_TARGET_STRING, "DSP-2"));

    VX_CALL(vxSetNodeTarget(node2_graph2, VX_TARGET_STRING, "DSP-2"));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph2));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph2));

    vxQueryNode(node1_graph1, VX_NODE_PERFORMANCE, &perf_node1_graph1, sizeof(perf_node1_graph1));
    vxQueryNode(node2_graph1, VX_NODE_PERFORMANCE, &perf_node2_graph1, sizeof(perf_node2_graph1));
    vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));

    vxQueryNode(node1_graph2, VX_NODE_PERFORMANCE, &perf_node1_graph2, sizeof(perf_node1_graph2));
    vxQueryNode(node2_graph2, VX_NODE_PERFORMANCE, &perf_node2_graph2, sizeof(perf_node2_graph2));
    vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));

    ASSERT_NO_FAILURE(accum_dst_graph1 = ct_image_from_vx_image(accum_image_final_graph1));

    ASSERT_NO_FAILURE(accum_dst_graph2 = ct_image_from_vx_image(accum_image_final_graph2));

    ASSERT_NO_FAILURE(accumulate_multiple_weighted_check(input_graph1, arg_->alpha_intermediate, arg_->alpha_final, accum_src_graph1, accum_final_graph1, accum_dst_graph1));

    ASSERT_NO_FAILURE(accumulate_multiple_weighted_check(input_graph2, arg_->alpha_intermediate, arg_->alpha_final, accum_src_graph2, accum_final_graph2, accum_dst_graph2));

    VX_CALL(vxReleaseNode(&node2_graph2));
    VX_CALL(vxReleaseNode(&node1_graph2));
    VX_CALL(vxReleaseNode(&node2_graph1));
    VX_CALL(vxReleaseNode(&node1_graph1));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseGraph(&graph1));

    ASSERT(node2_graph2 == 0);
    ASSERT(node1_graph2 == 0);
    ASSERT(node2_graph1 == 0);
    ASSERT(node1_graph1 == 0);
    ASSERT(graph2 == 0);
    ASSERT(graph1 == 0);

    VX_CALL(vxReleaseImage(&accum_image_final_graph1));
    VX_CALL(vxReleaseImage(&accum_image_final_graph2));
    VX_CALL(vxReleaseImage(&accum_image_intermediate_graph1));
    VX_CALL(vxReleaseImage(&accum_image_intermediate_graph2));
    VX_CALL(vxReleaseImage(&input_image_graph1));
    VX_CALL(vxReleaseImage(&input_image_graph2));
    VX_CALL(vxReleaseScalar(&alpha_scalar_final_graph1));
    VX_CALL(vxReleaseScalar(&alpha_scalar_final_graph2));
    VX_CALL(vxReleaseScalar(&alpha_scalar_graph1));
    VX_CALL(vxReleaseScalar(&alpha_scalar_graph2));

    ASSERT(accum_image_final_graph1 == 0);
    ASSERT(accum_image_final_graph2 == 0);
    ASSERT(accum_image_intermediate_graph1 == 0);
    ASSERT(accum_image_intermediate_graph2 == 0);
    ASSERT(input_image_graph1 == 0);
    ASSERT(input_image_graph2 == 0);

    printPerformance(perf_node1_graph1, arg_->width*arg_->height, "N1");

    printPerformance(perf_node2_graph1, arg_->width*arg_->height, "N2");

    printPerformance(perf_graph1, arg_->width*arg_->height, "G1");

    printPerformance(perf_node1_graph2, arg_->width*arg_->height, "N1");

    printPerformance(perf_node1_graph1, arg_->width*arg_->height, "N2");

    printPerformance(perf_node2_graph2, arg_->width*arg_->height, "G2");
}

TEST_WITH_ARG(AccumulateWeighted, testThreeParallelGraphs, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image1 = 0, input_image2 = 0, input_image3 = 0, accum_image_final1 = 0, accum_image_final2 = 0, accum_image_final3 = 0;
    vx_scalar alpha_scalar1, alpha_scalar2 = 0, alpha_scalar3 = 0;
    vx_graph graph1 = 0, graph2 = 0, graph3 = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_graph1, perf_graph2, perf_graph3;

    CT_Image input1 = NULL, input2 = NULL, input3 = NULL, accum_final1 = NULL, accum_final2 = NULL, accum_final3 = NULL;
    CT_Image accum_dst1 = NULL, accum_dst2 = NULL, accum_dst3 = NULL;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(alpha_scalar1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar3 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input3 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final3 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image1 = ct_image_to_vx_image(input1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image2 = ct_image_to_vx_image(input2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image3 = ct_image_to_vx_image(input3, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final1 = ct_image_to_vx_image(accum_final1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final2 = ct_image_to_vx_image(accum_final2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final3 = ct_image_to_vx_image(accum_final3, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph3 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxAccumulateWeightedImageNode(graph1, input_image1, alpha_scalar1, accum_image_final1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxAccumulateWeightedImageNode(graph2, input_image2, alpha_scalar2, accum_image_final2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxAccumulateWeightedImageNode(graph3, input_image3, alpha_scalar3, accum_image_final3), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-2"));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph3));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph3));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));
    vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));
    vxQueryGraph(graph3, VX_GRAPH_PERFORMANCE, &perf_graph3, sizeof(perf_graph3));

    ASSERT_NO_FAILURE(accum_dst1 = ct_image_from_vx_image(accum_image_final1));

    ASSERT_NO_FAILURE(accum_dst2 = ct_image_from_vx_image(accum_image_final2));

    ASSERT_NO_FAILURE(accum_dst3 = ct_image_from_vx_image(accum_image_final3));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input1, arg_->alpha_intermediate, accum_final1, accum_dst1));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input2, arg_->alpha_final, accum_final2, accum_dst2));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input3, arg_->alpha_final, accum_final3, accum_dst3));

    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph3));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseGraph(&graph1));

    ASSERT(node3 == 0);
    ASSERT(node2 == 0);
    ASSERT(node1 == 0);
    ASSERT(graph3 == 0);
    ASSERT(graph2 == 0);
    ASSERT(graph1 == 0);

    VX_CALL(vxReleaseImage(&accum_image_final3));
    VX_CALL(vxReleaseImage(&accum_image_final2));
    VX_CALL(vxReleaseImage(&accum_image_final1));
    VX_CALL(vxReleaseImage(&input_image3));
    VX_CALL(vxReleaseImage(&input_image2));
    VX_CALL(vxReleaseImage(&input_image1));
    VX_CALL(vxReleaseScalar(&alpha_scalar3));
    VX_CALL(vxReleaseScalar(&alpha_scalar2));
    VX_CALL(vxReleaseScalar(&alpha_scalar1));

    ASSERT(accum_image_final1 == 0);
    ASSERT(accum_image_final2 == 0);
    ASSERT(accum_image_final3 == 0);
    ASSERT(input_image1 == 0);
    ASSERT(input_image2 == 0);
    ASSERT(input_image3 == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");

    printPerformance(perf_node2, arg_->width*arg_->height, "N2");

    printPerformance(perf_node3, arg_->width*arg_->height, "N3");

    printPerformance(perf_graph1, arg_->width*arg_->height, "G1");

    printPerformance(perf_graph2, arg_->width*arg_->height, "G2");

    printPerformance(perf_graph3, arg_->width*arg_->height, "G3");
}

TEST_WITH_ARG(AccumulateWeighted, testAlternatingNodes, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image_not = 0, accum_image_final = 0;
    vx_image input_image_acc_1 = 0, input_image_acc_2 = 0;
    vx_image accum_image_virtual_1 = 0, accum_image_virtual_2 = 0, accum_image_virtual_3 = 0;
    vx_scalar alpha_scalar_1, alpha_scalar_2, alpha_scalar_3 = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0, node6 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_node4, perf_node5, perf_node6, perf_graph;

    CT_Image input_acc_1 = NULL, input_acc_2 = NULL, input_not = NULL, accum_final = NULL, accum_dst = NULL;
    CT_Image virtual_dummy_1 = NULL, virtual_dummy_2 = NULL, virtual_dummy_3 = NULL;

    virtual_dummy_1 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    virtual_dummy_2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    virtual_dummy_3 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(alpha_scalar_1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    // Note: change this to add another intermediate scalar
    ASSERT_VX_OBJECT(alpha_scalar_2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar_3 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input_acc_1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input_acc_2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input_not = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image_acc_1 = ct_image_to_vx_image(input_acc_1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image_acc_2 = ct_image_to_vx_image(input_acc_2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image_not = ct_image_to_vx_image(input_not, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final = ct_image_to_vx_image(accum_final, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    // Must be after vxCreateGraph
    ASSERT_VX_OBJECT(accum_image_virtual_1 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_virtual_2 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_virtual_3 = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxNotNode(graph, input_image_not, accum_image_virtual_1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxAccumulateWeightedImageNode(graph, accum_image_virtual_1, alpha_scalar_1, input_image_acc_1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxNotNode(graph, input_image_acc_1, accum_image_virtual_2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node4 = vxAccumulateWeightedImageNode(graph, accum_image_virtual_2, alpha_scalar_2, input_image_acc_2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node5 = vxNotNode(graph, input_image_acc_2, accum_image_virtual_3), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node6 = vxAccumulateWeightedImageNode(graph, accum_image_virtual_3, alpha_scalar_3, accum_image_final), VX_TYPE_NODE);

    // if any or all are on DSP-2, then it fails on PC; w/ or w/out virtual images
    VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node3, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node4, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node5, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node6, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryNode(node4, VX_NODE_PERFORMANCE, &perf_node4, sizeof(perf_node4));
    vxQueryNode(node5, VX_NODE_PERFORMANCE, &perf_node5, sizeof(perf_node5));
    vxQueryNode(node6, VX_NODE_PERFORMANCE, &perf_node6, sizeof(perf_node6));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(accum_dst = ct_image_from_vx_image(accum_image_final));

    ASSERT_NO_FAILURE(alternate_node_check(input_not, input_acc_1, input_acc_2, virtual_dummy_1, virtual_dummy_2, 
                        virtual_dummy_3, arg_->alpha_intermediate, arg_->alpha_final, accum_final, accum_dst));

    VX_CALL(vxReleaseNode(&node6));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node6 == 0);
    ASSERT(node5 == 0);
    ASSERT(node4 == 0);
    ASSERT(node3 == 0);
    ASSERT(node2 == 0);
    ASSERT(node1 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&accum_image_final));
    VX_CALL(vxReleaseImage(&accum_image_virtual_3));
    VX_CALL(vxReleaseImage(&accum_image_virtual_2));
    VX_CALL(vxReleaseImage(&accum_image_virtual_1));
    VX_CALL(vxReleaseImage(&input_image_acc_2));
    VX_CALL(vxReleaseImage(&input_image_acc_1));
    VX_CALL(vxReleaseImage(&input_image_not));
    VX_CALL(vxReleaseScalar(&alpha_scalar_3));
    VX_CALL(vxReleaseScalar(&alpha_scalar_2));
    VX_CALL(vxReleaseScalar(&alpha_scalar_1));

    ASSERT(accum_image_final == 0);
    ASSERT(input_image_acc_2 == 0);
    ASSERT(input_image_acc_1 == 0);
    ASSERT(input_image_not == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");

    printPerformance(perf_node2, arg_->width*arg_->height, "N2");

    printPerformance(perf_node3, arg_->width*arg_->height, "N3");

    printPerformance(perf_node4, arg_->width*arg_->height, "N4");

    printPerformance(perf_node5, arg_->width*arg_->height, "N5");

    printPerformance(perf_node6, arg_->width*arg_->height, "N6");

    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TESTCASE_TESTS(AccumulateWeighted,
        testGraphProcessing,
        testParallelNodes,
        testVirtualDataObject,
        testParallelGraphs,
        testParallelGraphsMultipleNodes,
        testThreeParallelGraphs,
        testAlternatingNodes
)
