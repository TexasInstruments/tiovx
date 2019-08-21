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
#include "shared_functions.h"

//#define CT_EXECUTE_ASYNC
#define MAX_NODES 10

void referenceAbsDiffSingle(CT_Image src0, CT_Image src1, CT_Image dst)
{
    uint32_t i, j;

    ASSERT(src0 && src1 && dst);
    ASSERT(src0->width = src1->width && src0->width == dst->width);
    ASSERT(src0->height = src1->height && src0->height == dst->height);
    ASSERT(src0->format == dst->format && src1->format == dst->format && dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
        {
            int32_t val = src0->data.y[i * src0->stride + j] - src1->data.y[i * src1->stride + j];
            dst->data.y[i * dst->stride + j] = val < 0 ? -val : val;
        }
}

void referenceAndSingle(CT_Image src0, CT_Image src1, CT_Image dst)
{
    uint32_t i, j;

    ASSERT(src0 && src1 && dst);
    ASSERT(src0->width = src1->width && src0->width == dst->width);
    ASSERT(src0->height = src1->height && src0->height == dst->height);
    ASSERT(src0->format == dst->format && src1->format == dst->format && dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = src0->data.y[i * src0->stride + j] & src1->data.y[i * src1->stride + j];
}

void referenceOrSingle(CT_Image src0, CT_Image src1, CT_Image dst)
{
    uint32_t i, j;

    ASSERT(src0 && src1 && dst);
    ASSERT(src0->width = src1->width && src0->width == dst->width);
    ASSERT(src0->height = src1->height && src0->height == dst->height);
    ASSERT(src0->format == dst->format && src1->format == dst->format && dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = src0->data.y[i * src0->stride + j] | src1->data.y[i * src1->stride + j];
}

void referenceXorSingle(CT_Image src0, CT_Image src1, CT_Image dst)
{
    uint32_t i, j;

    ASSERT(src0 && src1 && dst);
    ASSERT(src0->width = src1->width && src0->width == dst->width);
    ASSERT(src0->height = src1->height && src0->height == dst->height);
    ASSERT(src0->format == dst->format && src1->format == dst->format && dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = src0->data.y[i * src0->stride + j] ^ src1->data.y[i * src1->stride + j];
}

static void referenceAbsDiff(CT_Image src0, CT_Image src1, CT_Image src2, CT_Image src3, CT_Image virt1, CT_Image virt2, CT_Image dst)
{
    ASSERT_NO_FAILURE(referenceAbsDiffSingle(src0, src1, virt1));
    ASSERT_NO_FAILURE(referenceAbsDiffSingle(src2, src3, virt2));
    ASSERT_NO_FAILURE(referenceAbsDiffSingle(virt1, virt2, dst));
}

static void referenceAnd(CT_Image src0, CT_Image src1, CT_Image src2, CT_Image src3, CT_Image virt1, CT_Image virt2, CT_Image dst)
{
    ASSERT_NO_FAILURE(referenceAndSingle(src0, src1, virt1));
    ASSERT_NO_FAILURE(referenceAndSingle(src2, src3, virt2));
    ASSERT_NO_FAILURE(referenceAndSingle(virt1, virt2, dst));
}

static void referenceOr(CT_Image src0, CT_Image src1, CT_Image src2, CT_Image src3, CT_Image virt1, CT_Image virt2, CT_Image dst)
{
    ASSERT_NO_FAILURE(referenceOrSingle(src0, src1, virt1));
    ASSERT_NO_FAILURE(referenceOrSingle(src2, src3, virt2));
    ASSERT_NO_FAILURE(referenceOrSingle(virt1, virt2, dst));
}

static void referenceXor(CT_Image src0, CT_Image src1, CT_Image src2, CT_Image src3, CT_Image virt1, CT_Image virt2, CT_Image dst)
{
    ASSERT_NO_FAILURE(referenceXorSingle(src0, src1, virt1));
    ASSERT_NO_FAILURE(referenceXorSingle(src2, src3, virt2));
    ASSERT_NO_FAILURE(referenceXorSingle(virt1, virt2, dst));
}

typedef vx_node   (VX_API_CALL *vxBinopFunction) (vx_graph, vx_image, vx_image, vx_image);
typedef void      (*referenceFunction)(CT_Image, CT_Image, CT_Image, CT_Image, CT_Image, CT_Image, CT_Image);


TESTCASE(tivxBinOp8u,  CT_VXContext, ct_setup_vx_context, 0)

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
    FUZZY_ARG(func, 1600, 1200)


TEST_WITH_ARG(tivxBinOp8u, testFuzzy, fuzzy_arg, BINOP_SIZE_ARGS(AbsDiff), BINOP_SIZE_ARGS(And), BINOP_SIZE_ARGS(Or), BINOP_SIZE_ARGS(Xor))
{
    vx_image src1, src2, src3, src4, dst, virt1, virt2;
    vx_graph graph;
    CT_Image ref1, ref2, ref3, ref4, refdst, vxdst, virt_ctimage1, virt_ctimage2;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_graph;
    vx_rectangle_t src1_rect, src2_rect, dst_rect;
    vx_bool valid_rect;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));

    virt_ctimage1 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
    virt_ctimage2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

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

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src1, &src1_rect);
    vxGetValidRegionImage(src2, &src2_rect);
    vxGetValidRegionImage(dst, &dst_rect);

    ASSERT_EQ_INT((src1_rect.end_x - src1_rect.start_x), arg_->width);
    ASSERT_EQ_INT((src1_rect.end_y - src1_rect.start_y), arg_->height);

    ASSERT_EQ_INT((src2_rect.end_x - src2_rect.start_x), arg_->width);
    ASSERT_EQ_INT((src2_rect.end_y - src2_rect.start_y), arg_->height);

    ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), arg_->width);
    ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), arg_->height);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ref1  = ct_image_from_vx_image(src1);
    ref2  = ct_image_from_vx_image(src2);
    ref3  = ct_image_from_vx_image(src3);
    ref4  = ct_image_from_vx_image(src4);
    vxdst = ct_image_from_vx_image(dst);
    refdst = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

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

#define SUPERNODE_PARAMETERS(func)       \
    FUZZY_ARG(func, 800, 600)


TEST_WITH_ARG(tivxBinOp8u, testBinOp8uSupernode, fuzzy_arg, SUPERNODE_PARAMETERS(AbsDiff), SUPERNODE_PARAMETERS(And), SUPERNODE_PARAMETERS(Or), SUPERNODE_PARAMETERS(Xor))
{
    int node_count = 3;
    vx_image src1, src2, src3, src4, dst, virt1, virt2;
    vx_graph graph;
    CT_Image ref1, ref2, ref3, ref4, refdst, vxdst, virt_ctimage1, virt_ctimage2;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));

    virt_ctimage1 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
    virt_ctimage2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    // build one-node graph
    ASSERT_VX_OBJECT(node1 = arg_->vxFunc(graph, src1, src2, virt1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = arg_->vxFunc(graph, src3, src4, virt2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = arg_->vxFunc(graph, virt1, virt2, dst), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(node_list[0] = node1); 
    ASSERT_NO_FAILURE(node_list[1] = node2);
    ASSERT_NO_FAILURE(node_list[2] = node3);
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

    ref1  = ct_image_from_vx_image(src1);
    ref2  = ct_image_from_vx_image(src2);
    ref3  = ct_image_from_vx_image(src3);
    ref4  = ct_image_from_vx_image(src4);
    vxdst = ct_image_from_vx_image(dst);
    refdst = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    arg_->referenceFunc(ref1, ref2, ref3, ref4, virt_ctimage1, virt_ctimage2, refdst);

    ASSERT_EQ_CTIMAGE(refdst, vxdst);

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    printPerformance(perf_super_node, arg_->width * arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
}

#ifdef BUILD_BAM
#define testBinOp8uSupernode testBinOp8uSupernode
#else
#define testBinOp8uSupernode DISABLED_testBinOp8uSupernode
#endif

TESTCASE_TESTS(tivxBinOp8u, 
               testFuzzy,
               testBinOp8uSupernode)
