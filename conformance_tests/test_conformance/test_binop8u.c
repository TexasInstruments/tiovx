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

#include "test_engine/test.h"

#include <VX/vx.h>
#include <VX/vxu.h>

//#define CT_EXECUTE_ASYNC

static void referenceAbsDiff(CT_Image src0, CT_Image src1, CT_Image dst)
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

static void referenceAnd(CT_Image src0, CT_Image src1, CT_Image dst)
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

static void referenceOr(CT_Image src0, CT_Image src1, CT_Image dst)
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

static void referenceXor(CT_Image src0, CT_Image src1, CT_Image dst)
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

typedef vx_status (VX_API_CALL *vxuBinopFunction)(vx_context, vx_image, vx_image, vx_image);
typedef vx_node   (VX_API_CALL *vxBinopFunction) (vx_graph, vx_image, vx_image, vx_image);
typedef void      (*referenceFunction)(CT_Image, CT_Image, CT_Image);


TESTCASE(vxuBinOp8u, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(vxBinOp8u,  CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    vxuBinopFunction  vxuFunc;
    vxBinopFunction   vxFunc;
    referenceFunction referenceFunc;
} func_arg;

#define FUNC_ARG(func) ARG(#func, vxu##func, vx##func##Node, reference##func)

TEST_WITH_ARG(vxuBinOp8u, testNegativeSizes, func_arg, FUNC_ARG(AbsDiff), FUNC_ARG(And), FUNC_ARG(Or), FUNC_ARG(Xor))
{
    vx_image src1_32x32, src1_64x64, src2_32x32, src2_32x64, dst32x32, dst88x16;
    vx_context context = context_->vx_context_;

    ASSERT_VX_OBJECT(src1_32x32 = vxCreateImage(context, 32, 32, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src1_64x64 = vxCreateImage(context, 64, 64, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2_32x32 = vxCreateImage(context, 32, 32, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2_32x64 = vxCreateImage(context, 32, 64, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst32x32 = vxCreateImage(context, 32, 32, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst88x16 = vxCreateImage(context, 88, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    // initialize to guarantee that images are allocated
    ASSERT_NO_FAILURE(ct_fill_image_random(src1_32x32, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src1_64x64, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2_32x32, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2_32x64, &CT()->seed_));

    EXPECT_NE_VX_STATUS(VX_SUCCESS, arg_->vxuFunc(context, src1_32x32, src2_32x32, dst88x16));
    EXPECT_NE_VX_STATUS(VX_SUCCESS, arg_->vxuFunc(context, src1_32x32, src2_32x64, dst32x32));
    EXPECT_NE_VX_STATUS(VX_SUCCESS, arg_->vxuFunc(context, src1_64x64, src2_32x32, dst32x32));
    EXPECT_NE_VX_STATUS(VX_SUCCESS, arg_->vxuFunc(context, src1_64x64, src2_32x64, dst32x32));

    VX_CALL(vxReleaseImage(&src1_32x32));
    VX_CALL(vxReleaseImage(&src2_32x32));
    VX_CALL(vxReleaseImage(&src1_64x64));
    VX_CALL(vxReleaseImage(&src2_32x64));
    VX_CALL(vxReleaseImage(&dst32x32));
    VX_CALL(vxReleaseImage(&dst88x16));
}

TEST_WITH_ARG(vxBinOp8u, testNegativeSizes, func_arg, FUNC_ARG(AbsDiff), FUNC_ARG(And), FUNC_ARG(Or), FUNC_ARG(Xor))
{
    vx_image src1_32x32, src1_64x64, src2_32x32, src2_32x64, dst32x32, dst88x16;
    vx_graph graph1, graph2, graph3, graph4;
    vx_context context = context_->vx_context_;

    ASSERT_VX_OBJECT(src1_32x32 = vxCreateImage(context, 32, 32, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src1_64x64 = vxCreateImage(context, 64, 64, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2_32x32 = vxCreateImage(context, 32, 32, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2_32x64 = vxCreateImage(context, 32, 64, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst32x32 = vxCreateImage(context, 32, 32, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst88x16 = vxCreateImage(context, 88, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(arg_->vxFunc(graph1, src1_32x32, src2_32x32, dst88x16), VX_TYPE_NODE);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph1));

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(arg_->vxFunc(graph2, src1_32x32, src2_32x64, dst32x32), VX_TYPE_NODE);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph2));

    ASSERT_VX_OBJECT(graph3 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(arg_->vxFunc(graph3, src1_64x64, src2_32x32, dst32x32), VX_TYPE_NODE);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph3));

    ASSERT_VX_OBJECT(graph4 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(arg_->vxFunc(graph4, src1_64x64, src2_32x64, dst32x32), VX_TYPE_NODE);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph4));

    VX_CALL(vxReleaseImage(&src1_32x32));
    VX_CALL(vxReleaseImage(&src2_32x32));
    VX_CALL(vxReleaseImage(&src1_64x64));
    VX_CALL(vxReleaseImage(&src2_32x64));
    VX_CALL(vxReleaseImage(&dst32x32));
    VX_CALL(vxReleaseImage(&dst88x16));
    VX_CALL(vxReleaseGraph(&graph1));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseGraph(&graph3));
    VX_CALL(vxReleaseGraph(&graph4));
}

static vx_image inference_image;
static vx_action VX_CALLBACK inference_image_test(vx_node node)
{
    vx_uint32 width  = 0;
    vx_uint32 height = 0;
    vx_df_image format = 0;

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(inference_image, VX_IMAGE_WIDTH,   &width,   sizeof(width)));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(inference_image, VX_IMAGE_HEIGHT,  &height,  sizeof(height)));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(inference_image, VX_IMAGE_FORMAT,  &format,  sizeof(format)));

    EXPECT_EQ_INT(640, width);
    EXPECT_EQ_INT(480, height);
    EXPECT_EQ_INT(VX_DF_IMAGE_U8, format);

    return VX_ACTION_CONTINUE;
}

TEST_WITH_ARG(vxBinOp8u, testInference, func_arg, FUNC_ARG(AbsDiff), FUNC_ARG(And), FUNC_ARG(Or), FUNC_ARG(Xor))
{
    vx_image src1, src2, dst, gr;
    vx_graph graph;
    vx_node n, tmp;
    vx_context context = context_->vx_context_;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(src1  = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2  = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(n     = arg_->vxFunc(graph, src1, src2, dst), VX_TYPE_NODE);

    // grounding
    ASSERT_VX_OBJECT(gr    = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(tmp   = vxAddNode(graph, dst, src2, VX_CONVERT_POLICY_WRAP, gr), VX_TYPE_NODE);

    // test
    inference_image = dst;
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxAssignNodeCallback(n, inference_image_test));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&n));
    VX_CALL(vxReleaseNode(&tmp));
    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseImage(&gr));
    VX_CALL(vxReleaseGraph(&graph));
}

typedef struct {
    const char* name;
    uint32_t width;
    uint32_t height;
    vxuBinopFunction  vxuFunc;
    vxBinopFunction   vxFunc;
    referenceFunction referenceFunc;
} fuzzy_arg;

#define FUZZY_ARG(func,w,h) ARG(#func ": " #w "x" #h, w, h, vxu##func, vx##func##Node, reference##func)

#define BINOP_SIZE_ARGS(func)       \
    FUZZY_ARG(func, 640, 480),      \
    ARG_EXTENDED_BEGIN(),           \
    FUZZY_ARG(func, 1, 1),          \
    FUZZY_ARG(func, 15, 17),        \
    FUZZY_ARG(func, 32, 32),        \
    FUZZY_ARG(func, 1231, 1234),    \
    FUZZY_ARG(func, 1280, 720),     \
    FUZZY_ARG(func, 1920, 1080),    \
    ARG_EXTENDED_END()

TEST_WITH_ARG(vxuBinOp8u, testFuzzy, fuzzy_arg, BINOP_SIZE_ARGS(AbsDiff), BINOP_SIZE_ARGS(And), BINOP_SIZE_ARGS(Or), BINOP_SIZE_ARGS(Xor))
{
    vx_image src1, src2, dst;
    CT_Image ref1, ref2, refdst, vxdst;
    vx_context context = context_->vx_context_;

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst  = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, arg_->vxuFunc(context, src1, src2, dst));

    ref1  = ct_image_from_vx_image(src1);
    ref2  = ct_image_from_vx_image(src2);
    vxdst = ct_image_from_vx_image(dst);
    refdst = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    arg_->referenceFunc(ref1, ref2, refdst);

    ASSERT_EQ_CTIMAGE(refdst, vxdst);

    // checked release vx images
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    EXPECT_EQ_PTR(NULL, dst);
    EXPECT_EQ_PTR(NULL, src1);
    EXPECT_EQ_PTR(NULL, src2);
}

TEST_WITH_ARG(vxBinOp8u, testFuzzy, fuzzy_arg, BINOP_SIZE_ARGS(AbsDiff), BINOP_SIZE_ARGS(And), BINOP_SIZE_ARGS(Or), BINOP_SIZE_ARGS(Xor))
{
    vx_image src1, src2, dst;
    vx_graph graph;
    CT_Image ref1, ref2, refdst, vxdst;
    vx_context context = context_->vx_context_;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));

    // build one-node graph
    ASSERT_VX_OBJECT(arg_->vxFunc(graph, src1, src2, dst), VX_TYPE_NODE);

    // run graph
#ifdef CT_EXECUTE_ASYNC
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

    ref1  = ct_image_from_vx_image(src1);
    ref2  = ct_image_from_vx_image(src2);
    vxdst = ct_image_from_vx_image(dst);
    refdst = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    arg_->referenceFunc(ref1, ref2, refdst);

    ASSERT_EQ_CTIMAGE(refdst, vxdst);

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseGraph(&graph));
}

TESTCASE_TESTS(vxuBinOp8u, DISABLED_testNegativeSizes,                testFuzzy)
TESTCASE_TESTS(vxBinOp8u,  DISABLED_testNegativeSizes, testInference, testFuzzy)
