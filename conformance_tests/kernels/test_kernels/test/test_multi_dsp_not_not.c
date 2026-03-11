/*
 * Copyright (c) 2026 The Khronos Group Inc.
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


#include "test_engine/test.h"
#include "test_tiovx.h"
#include <VX/vx.h>
#include <TI/tivx_test_kernels.h>

static void fillSequence(CT_Image dst, uint32_t seq_init)
{
    uint32_t i, j;
    uint32_t val = seq_init;

    ASSERT(dst);
    ASSERT(dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = ++val;
}

TESTCASE(tivxTestKernelsMCNotNot,  CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    uint32_t width;
    uint32_t height;
} size_arg;

#define SIZE_ARG(w,h) ARG(#w "x" #h, w, h)

#define NOT_SIZE_ARGS       \
    SIZE_ARG(640, 480),     \
    SIZE_ARG(1231, 1234),   \
    SIZE_ARG(1280, 720),    \
    SIZE_ARG(1920, 1080)

TEST_WITH_ARG(tivxTestKernelsMCNotNot, testMultiDSPNested, size_arg, NOT_SIZE_ARGS)
{
    vx_image src, dst_mc, dst_tn, int_img;
    CT_Image ref_src, vxdst;
    vx_graph graph_mc, graph_nn;
    vx_node node_mc, node_tn1, node_tn2;
    vx_context context = context_->vx_context_;
    vx_perf_t perf_graph_mc, perf_graph_nn, p_node_nn1, p_node_nn2;
    vx_bool speed_check_passed = vx_true_e;

    printf("NUM DSPs: %d\n", C7X_COUNT);

    ASSERT_NO_FAILURE({
        ref_src = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
        fillSequence(ref_src, (uint32_t)CT()->seed_);
        src = ct_image_to_vx_image(ref_src, context);
    });

    tivxTestKernelsLoadKernels(context);

    // Build graph with single MultiDSPNotNotNode
    ASSERT_VX_OBJECT(graph_mc = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(dst_mc   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node_mc  = tivxMultiDSPNotNotNode(graph_mc, src, dst_mc), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node_mc, VX_TARGET_STRING, TIVX_TARGET_MPU_1));

    // run graph and report results
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph_mc));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph_mc));

    ASSERT_NO_FAILURE({
        vxdst = ct_image_from_vx_image(dst_mc);
    });

    ASSERT_EQ_CTIMAGE(ref_src, vxdst);
    VX_CALL(vxQueryGraph(graph_mc, VX_GRAPH_PERFORMANCE, &perf_graph_mc, sizeof(perf_graph_mc)));
    printPerformance(perf_graph_mc, arg_->width*arg_->height, "MC");

    // Build graph with two TestNotNodes as the control
    ASSERT_VX_OBJECT(graph_nn = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(dst_tn   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(int_img  = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node_tn1 = tivxTestNotNode(graph_nn, src, int_img), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node_tn2 = tivxTestNotNode(graph_nn, int_img, dst_tn), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node_tn1, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_1));
    VX_CALL(vxSetNodeTarget(node_tn2, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_1));

    // run graph and report results
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph_nn));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph_nn));

    ASSERT_NO_FAILURE({
        vxdst = ct_image_from_vx_image(dst_tn);
    });

    ASSERT_EQ_CTIMAGE(ref_src, vxdst);

    vxQueryGraph(graph_nn, VX_GRAPH_PERFORMANCE, &perf_graph_nn, sizeof(perf_graph_nn));
    vxQueryNode(node_tn1, VX_NODE_PERFORMANCE, &p_node_nn1, sizeof(p_node_nn1));
    vxQueryNode(node_tn2, VX_NODE_PERFORMANCE, &p_node_nn2, sizeof(p_node_nn2));
    printPerformance(perf_graph_nn, arg_->width*arg_->height, "NN");

    if (perf_graph_mc.avg >= perf_graph_nn.avg)
    {
        VX_PRINT(VX_ZONE_ERROR, "MultiDSPNotNot is SLOWER than NotNot!\n");
        VX_PRINT(VX_ZONE_ERROR, "Is the test image big enough for multichannel to have an advantage?\nAre all the DSPs being utilized?\n");
        speed_check_passed = vx_false_e;
    }

    VX_CALL(vxReleaseNode(&node_mc));
    VX_CALL(vxReleaseNode(&node_tn1));
    VX_CALL(vxReleaseNode(&node_tn2));
    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst_tn));
    VX_CALL(vxReleaseImage(&int_img));
    VX_CALL(vxReleaseImage(&dst_mc));
    VX_CALL(vxReleaseGraph(&graph_mc));
    VX_CALL(vxReleaseGraph(&graph_nn));

    tivxTestKernelsUnLoadKernels(context);

    if (speed_check_passed != vx_true_e){
#if (C7X_COUNT < 2)
        VX_PRINT(VX_ZONE_WARNING, "MultiDSPNested Test called with only %d DSP(s); Speed test skipped\n", C7X_COUNT);
#elif !defined(PC)
        ASSERT(0);
#endif
    }
}

/* Proof of concept to ensure validity */
TEST_WITH_ARG(tivxTestKernelsMCNotNot, testMultiDSPReplicate, size_arg, NOT_SIZE_ARGS)
{
    vx_object_array oa_src, oa_dst;
    vx_image exemplar, src_first_item, dst_first_item, src_item, dst_item;
    CT_Image ref_src[10], vxdst;
    vx_graph graph;
    vx_node node;
    vx_context context = context_->vx_context_;
    vx_size i, size = 10;
    vx_perf_t perf_graph;

    ASSERT_VX_OBJECT(exemplar = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(oa_src = vxCreateObjectArray(context, (vx_reference)exemplar, size), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(oa_dst = vxCreateObjectArray(context, (vx_reference)exemplar, size), VX_TYPE_OBJECT_ARRAY);
    ASSERT_NO_FAILURE(vxReleaseImage(&exemplar));

    for (i = 0; i < size; i++)
    {
        ASSERT_VX_OBJECT(src_item = (vx_image)vxGetObjectArrayItem(oa_src, i), VX_TYPE_IMAGE);
        ASSERT_NO_FAILURE({
            ref_src[i] = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
            fillSequence(ref_src[i], (uint32_t)CT()->seed_);
            ct_image_copyto_vx_image(src_item, ref_src[i]);
        });
        vxReleaseImage(&src_item);
    }

    tivxTestKernelsLoadKernels(context);

    // build one-node graph
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    // Get oa[0] and replicate
    ASSERT_VX_OBJECT(src_first_item = (vx_image)vxGetObjectArrayItem(oa_src, 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_first_item = (vx_image)vxGetObjectArrayItem(oa_dst, 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node  = tivxMultiDSPNotNotNode(graph, src_first_item, dst_first_item), VX_TYPE_NODE);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_MPU_1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReplicateNode(graph, node, (vx_bool[]){(vx_bool)vx_true_e, (vx_bool)vx_true_e}, 2u));

    // run graph
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    for (i = 0; i < size; i++)
    {
        ASSERT_VX_OBJECT(dst_item = (vx_image)vxGetObjectArrayItem(oa_dst, i), VX_TYPE_IMAGE);
        ASSERT_NO_FAILURE({
            vxdst = ct_image_from_vx_image(dst_item);
        });
        ASSERT_EQ_CTIMAGE(ref_src[i], vxdst);
        vxReleaseImage(&dst_item);
    }

    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));
    printPerformance(perf_graph, arg_->width*arg_->height, "Rp");

    VX_CALL(vxReleaseImage(&dst_first_item));
    VX_CALL(vxReleaseImage(&src_first_item));
    VX_CALL(vxReleaseObjectArray(&oa_dst));
    VX_CALL(vxReleaseObjectArray(&oa_src));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

TEST(tivxTestKernelsMCNotNot, negativeTestMultiDSPNested)
{
    vx_image src, dst;
    vx_graph graph;
    vx_node node;
    vx_context context = context_->vx_context_;
    vx_status status;
    vx_bool passed = vx_true_e;

    tivxTestKernelsLoadKernels(context);

    /* Negative: 1x1 image should fail with VX_ERROR_INVALID_PARAMETERS */
    src  = vxCreateImage(context, 1, 1, VX_DF_IMAGE_U8);
    dst  = vxCreateImage(context, 1, 1, VX_DF_IMAGE_U8);
    graph = vxCreateGraph(context);
    node = tivxMultiDSPNotNotNode(graph, src, dst);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseGraph(&graph));

    /* Negative Speed Test: 16x16 SHOULD BE too small for MC to beat sequential 
       on same C7 due to striping overhead */
    vx_image int_img;
    vx_graph graph_nn;
    vx_node node_tn1, node_tn2;
    vx_perf_t perf_graph_mc, perf_graph_nn;
    CT_Image ref_src;

    ASSERT_NO_FAILURE({
        ref_src = ct_allocate_image(16, 16, VX_DF_IMAGE_U8);
        fillSequence(ref_src, (uint32_t)CT()->seed_);
        src = ct_image_to_vx_image(ref_src, context);
    });
    dst   = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    graph = vxCreateGraph(context);
    node  = tivxMultiDSPNotNotNode(graph, src, dst);
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_MPU_1));

    ASSERT_NO_FAILURE(vxVerifyGraph(graph));

    VX_CALL(vxProcessGraph(graph));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph_mc, sizeof(perf_graph_mc)));
    printPerformance(perf_graph_mc, 16*16, "MC");

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseGraph(&graph));

    int_img  = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    dst      = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    graph_nn = vxCreateGraph(context);
    node_tn1 = tivxTestNotNode(graph_nn, src, int_img);
    node_tn2 = tivxTestNotNode(graph_nn, int_img, dst);
    VX_CALL(vxSetNodeTarget(node_tn1, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_1));
    VX_CALL(vxSetNodeTarget(node_tn2, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph_nn));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph_nn));
    VX_CALL(vxQueryGraph(graph_nn, VX_GRAPH_PERFORMANCE, &perf_graph_nn, sizeof(perf_graph_nn)));
    printPerformance(perf_graph_nn, 16*16, "NN");

    if (perf_graph_mc.avg < perf_graph_nn.avg)
    {
        VX_PRINT(VX_ZONE_ERROR, "Expected MC to be SLOWER than sequential for 64x64 image\n");
        passed = vx_false_e;
    }

    VX_CALL(vxReleaseNode(&node_tn1));
    VX_CALL(vxReleaseNode(&node_tn2));
    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseImage(&int_img));
    VX_CALL(vxReleaseGraph(&graph_nn));
    tivxTestKernelsUnLoadKernels(context);

    if (passed != vx_true_e){
#if !defined(PC)
        ASSERT(0);
#endif
    }
}

TESTCASE_TESTS(tivxTestKernelsMCNotNot,
    testMultiDSPNested,
#if (C7X_COUNT >= 2)
    negativeTestMultiDSPNested,     // Commented out because both negative tests would be invalid fail on C7X_COUNT = 1
#endif
    testMultiDSPReplicate
)
