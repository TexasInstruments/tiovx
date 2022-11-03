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

//#define CT_EXECUTE_ASYNC
#define MAX_NODES 10

void referenceNot(CT_Image src, CT_Image dst)
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

static void fillSquence(CT_Image dst, uint32_t seq_init)
{
    uint32_t i, j;
    uint32_t val = seq_init;

    ASSERT(dst);
    ASSERT(dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = ++val;
}

TESTCASE(tivxNot,  CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    uint32_t width;
    uint32_t height;
} size_arg;

#define SIZE_ARG(w,h) ARG(#w "x" #h, w, h)

#define NOT_SIZE_ARGS       \
    SIZE_ARG(18, 18),       \
    SIZE_ARG(644, 258),     \
    SIZE_ARG(1600, 1200),   \
    ARG_EXTENDED_BEGIN(),   \
    SIZE_ARG(1, 1),         \
    SIZE_ARG(15, 17),       \
    SIZE_ARG(32, 32),       \
    SIZE_ARG(1231, 1234),   \
    SIZE_ARG(1280, 720),    \
    SIZE_ARG(1920, 1080),   \
    ARG_EXTENDED_END()

TEST_WITH_ARG(tivxNot, testSizes, size_arg, NOT_SIZE_ARGS)
{
    vx_image src, dst, virt;
    CT_Image ref_src, refdst, vxdst, virt_ctimage;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    ASSERT_NO_FAILURE({
        ref_src = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
        virt_ctimage = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
        fillSquence(ref_src, (uint32_t)CT()->seed_);
        src = ct_image_to_vx_image(ref_src, context);
    });

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    // build one-node graph
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node1 = vxNotNode(graph, src, virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, virt, dst), VX_TYPE_NODE);

    // run graph
#ifdef CT_EXECUTE_ASYNC
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src, &src_rect);
    vxGetValidRegionImage(dst, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), arg_->width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), arg_->height);

    ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), arg_->width);
    ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), arg_->height);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE({
        vxdst = ct_image_from_vx_image(dst);
        refdst = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
        referenceNot(ref_src, virt_ctimage);
        referenceNot(virt_ctimage, refdst);
    });

    ASSERT_EQ_CTIMAGE(refdst, vxdst);

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");

    printPerformance(perf_node2, arg_->width*arg_->height, "N2");

    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}


#define SUPERNODE_NOT_SIZE_ARGS       \
    SIZE_ARG(1600, 1200)

TEST_WITH_ARG(tivxNot, testNotSupernode, size_arg, SUPERNODE_NOT_SIZE_ARGS)
{
    int node_count = 2;
    vx_image src, dst, virt;
    CT_Image ref_src, refdst, vxdst, virt_ctimage;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    ASSERT_NO_FAILURE({
        ref_src = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
        virt_ctimage = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
        fillSquence(ref_src, (uint32_t)CT()->seed_);
        src = ct_image_to_vx_image(ref_src, context);
    });

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    // build one-node graph
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node1 = vxNotNode(graph, src, virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, virt, dst), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(node_list[0] = node1); 
    ASSERT_NO_FAILURE(node_list[1] = node2);
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    
    VX_CALL(vxVerifyGraph(graph));

    // run graph
#ifdef CT_EXECUTE_ASYNC
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src, &src_rect);
    vxGetValidRegionImage(dst, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), arg_->width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), arg_->height);

    ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), arg_->width);
    ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), arg_->height);

    ASSERT_NO_FAILURE({
        vxdst = ct_image_from_vx_image(dst);
        refdst = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
        referenceNot(ref_src, virt_ctimage);
        referenceNot(virt_ctimage, refdst);
    });

    ASSERT_EQ_CTIMAGE(refdst, vxdst);

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    printPerformance(perf_super_node, arg_->width * arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
}

#ifdef BUILD_BAM
#define testNotSupernode testNotSupernode
#else
#define testNotSupernode DISABLED_testNotSupernode
#endif

TESTCASE_TESTS(tivxNot, 
               testSizes,
               testNotSupernode)

