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


TESTCASE(tivxAccumulateSquare, CT_VXContext, ct_setup_vx_context, 0)

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


static CT_Image accumulate_square_generate_random_8u(int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}


static CT_Image accumulate_square_generate_random_16s_non_negative(int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_S16, &CT()->seed_, 0, 32768));

    return image;
}


static void accumulate_square_reference(CT_Image input, vx_uint32 shift, CT_Image accum)
{
    CT_FILL_IMAGE_16S(return, accum,
            {
                uint8_t* input_data = CT_IMAGE_DATA_PTR_8U(input, x, y);
                int32_t res32 = ((int32_t)(*dst_data)) + ((((int32_t)(*input_data))*((int32_t)(*input_data))) >> shift);
                int16_t res = CT_SATURATE_S16(res32);
                *dst_data = res;
            });
}


static void accumulate_square_check(CT_Image input, vx_uint32 shift, CT_Image accum_src, CT_Image accum_dst)
{
    CT_Image accum_ref = NULL;

    ASSERT(input && accum_src && accum_dst);

    ASSERT_NO_FAILURE(accum_ref = ct_image_create_clone(accum_src));

    ASSERT_NO_FAILURE(accumulate_square_reference(input, shift, accum_ref));

    EXPECT_EQ_CTIMAGE(accum_ref, accum_dst);
}

static void accumulate_square_chain_check(CT_Image input, CT_Image virtual_img, CT_Image accum_input1,
                                          vx_uint32 shift1, vx_uint32 shift2, CT_Image accum_input2,
                                          CT_Image accum_dst)
{
    CT_Image accum_ref = NULL, accum_intermediate = NULL;

    ASSERT(input && virtual_img && accum_input1 && accum_input2 && accum_dst);

    ASSERT_NO_FAILURE(accum_intermediate = ct_image_create_clone(accum_input1));

    ASSERT_NO_FAILURE(accum_ref = ct_image_create_clone(accum_input2));

    ASSERT_NO_FAILURE(accumulate_square_reference(input, shift1, accum_intermediate));

    ASSERT_NO_FAILURE(referenceConvertDepth(accum_intermediate, virtual_img, 0, VX_CONVERT_POLICY_SATURATE));

    ASSERT_NO_FAILURE(accumulate_square_reference(virtual_img, shift2, accum_ref));

    EXPECT_EQ_CTIMAGE(accum_ref, accum_dst);
}

typedef struct {
    const char* testName;
    vx_uint32 shift1, shift2;
    int width, height;
} Arg;


#define PARAMETERS \
    CT_GENERATE_PARAMETERS("random/shift0", ADD_SIZE_18x18, ARG, 0, 15), \
    CT_GENERATE_PARAMETERS("random/shift1", ADD_SIZE_644x258, ARG, 1, 3), \
    CT_GENERATE_PARAMETERS("random/shift8", ADD_SIZE_1600x1200, ARG, 8, 5)

TEST_WITH_ARG(tivxAccumulateSquare, testParallelNodes, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image1 = 0, input_image2 = 0, accum_image1 = 0, accum_image2 = 0;
    vx_scalar shift_scalar1 = 0, shift_scalar2 = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;

    CT_Image input1 = NULL, input2 = NULL, accum_src1 = NULL, accum_src2 = NULL, accum_dst1 = NULL, accum_dst2 = NULL;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(shift_scalar1 = vxCreateScalar(context, VX_TYPE_UINT32, &arg_->shift1), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(shift_scalar2 = vxCreateScalar(context, VX_TYPE_UINT32, &arg_->shift2), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input1 = accumulate_square_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input2 = accumulate_square_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_src1 = accumulate_square_generate_random_16s_non_negative(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_src2 = accumulate_square_generate_random_16s_non_negative(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image1 = ct_image_to_vx_image(input1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image2 = ct_image_to_vx_image(input2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image1 = ct_image_to_vx_image(accum_src1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image2 = ct_image_to_vx_image(accum_src2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxAccumulateSquareImageNode(graph, input_image1, shift_scalar1, accum_image1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxAccumulateSquareImageNode(graph, input_image2, shift_scalar2, accum_image2), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(accum_dst1 = ct_image_from_vx_image(accum_image1));

    ASSERT_NO_FAILURE(accum_dst2 = ct_image_from_vx_image(accum_image2));

    ASSERT_NO_FAILURE(accumulate_square_check(input1, arg_->shift1, accum_src1, accum_dst1));

    ASSERT_NO_FAILURE(accumulate_square_check(input2, arg_->shift2, accum_src2, accum_dst2));

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
    VX_CALL(vxReleaseScalar(&shift_scalar2));
    VX_CALL(vxReleaseScalar(&shift_scalar1));

    ASSERT(accum_image2 == 0);
    ASSERT(accum_image1 == 0);
    ASSERT(input_image2 == 0);
    ASSERT(input_image1 == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");

    printPerformance(perf_node2, arg_->width*arg_->height, "N2");

    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TEST_WITH_ARG(tivxAccumulateSquare, testSequentialNodes, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image1 = 0, accum_image1 = 0, accum_image2 = 0, virtual_image = 0;
    vx_scalar shift_scalar1 = 0, shift_scalar2 = 0, shift_convertdepth = 0;
    vx_int32 sh = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_graph;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    CT_Image input1 = NULL, accum_src1 = NULL, accum_src2 = NULL, accum_dst = NULL, virtual_ctimage = NULL;

    virtual_ctimage = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(shift_convertdepth = vxCreateScalar(context, VX_TYPE_INT32, &sh), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(shift_scalar1 = vxCreateScalar(context, VX_TYPE_UINT32, &arg_->shift1), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(shift_scalar2 = vxCreateScalar(context, VX_TYPE_UINT32, &arg_->shift2), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input1 = accumulate_square_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_src1 = accumulate_square_generate_random_16s_non_negative(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_src2 = accumulate_square_generate_random_16s_non_negative(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image1 = ct_image_to_vx_image(input1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image1 = ct_image_to_vx_image(accum_src1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image2 = ct_image_to_vx_image(accum_src2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virtual_image = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxAccumulateSquareImageNode(graph, input_image1, shift_scalar1, accum_image1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxConvertDepthNode(graph, accum_image1, virtual_image, VX_CONVERT_POLICY_SATURATE, shift_convertdepth), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxAccumulateSquareImageNode(graph, virtual_image, shift_scalar2, accum_image2), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(input_image1, &src_rect);
    vxGetValidRegionImage(accum_image2, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), arg_->width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), arg_->height);

    ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), arg_->width);
    ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), arg_->height);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(accum_dst = ct_image_from_vx_image(accum_image2));

    ASSERT_NO_FAILURE(accumulate_square_chain_check(input1, virtual_ctimage, accum_src1, arg_->shift1, arg_->shift2, accum_src2, accum_dst));

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
    VX_CALL(vxReleaseScalar(&shift_scalar2));
    VX_CALL(vxReleaseScalar(&shift_scalar1));

    ASSERT(accum_image2 == 0);
    ASSERT(accum_image1 == 0);
    ASSERT(input_image1 == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3, arg_->width*arg_->height, "N3");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TESTCASE_TESTS(tivxAccumulateSquare,
        testParallelNodes,
        testSequentialNodes
)
