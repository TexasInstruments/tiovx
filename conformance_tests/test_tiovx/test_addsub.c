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

//#define CT_EXECUTE_ASYNC

static void referenceAddSingle(CT_Image src0, CT_Image src1, CT_Image dst, enum vx_convert_policy_e policy)
{
    int32_t min_bound, max_bound;
    uint32_t i, j;
    ASSERT(src0 && src1 && dst);
    ASSERT(src0->width = src1->width && src0->width == dst->width);
    ASSERT(src0->height = src1->height && src0->height == dst->height);

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

#define ADD_LOOP(s0, s1, r)                                                                                     \
    do{                                                                                                         \
        for (i = 0; i < dst->height; ++i)                                                                       \
            for (j = 0; j < dst->width; ++j)                                                                    \
            {                                                                                                   \
                int32_t val = src0->data.s0[i * src0->stride + j];                                              \
                val += src1->data.s1[i * src1->stride + j];                                                     \
                dst->data.r[i * dst->stride + j] = (val < min_bound ? min_bound :                               \
                                                                        (val > max_bound ? max_bound : val));   \
            }                                                                                                   \
    }while(0)

    if (src0->format == VX_DF_IMAGE_U8 && src1->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_U8)
        ADD_LOOP(y, y, y);
    else if (src0->format == VX_DF_IMAGE_U8 && src1->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_S16)
        ADD_LOOP(y, y, s16);
    else if (src0->format == VX_DF_IMAGE_U8 && src1->format == VX_DF_IMAGE_S16 && dst->format == VX_DF_IMAGE_S16)
        ADD_LOOP(y, s16, s16);
    else if (src0->format == VX_DF_IMAGE_S16 && src1->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_S16)
        ADD_LOOP(s16, y, s16);
    else if (src0->format == VX_DF_IMAGE_S16 && src1->format == VX_DF_IMAGE_S16 && dst->format == VX_DF_IMAGE_S16)
        ADD_LOOP(s16, s16, s16);
    else
        FAIL("Unsupported combination of argument formats: %.4s + %.4s = %.4s", &src0->format, &src1->format, &dst->format);

#undef ADD_LOOP
}

static void referenceSubtractSingle(CT_Image src0, CT_Image src1, CT_Image dst, enum vx_convert_policy_e policy)
{
    int32_t min_bound, max_bound;
    uint32_t i, j;
    ASSERT(src0 && src1 && dst);
    ASSERT(src0->width = src1->width && src0->width == dst->width);
    ASSERT(src0->height = src1->height && src0->height == dst->height);

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

#define SUB_LOOP(s0, s1, r)                                                                                     \
    do{                                                                                                         \
        for (i = 0; i < dst->height; ++i)                                                                       \
            for (j = 0; j < dst->width; ++j)                                                                    \
            {                                                                                                   \
                int32_t val = src0->data.s0[i * src0->stride + j];                                              \
                val -= src1->data.s1[i * src1->stride + j];                                                     \
                dst->data.r[i * dst->stride + j] = (val < min_bound ? min_bound :                               \
                                                                        (val > max_bound ? max_bound : val));   \
            }                                                                                                   \
    }while(0)

    if (src0->format == VX_DF_IMAGE_U8 && src1->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_U8)
        SUB_LOOP(y, y, y);
    else if (src0->format == VX_DF_IMAGE_U8 && src1->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_S16)
        SUB_LOOP(y, y, s16);
    else if (src0->format == VX_DF_IMAGE_U8 && src1->format == VX_DF_IMAGE_S16 && dst->format == VX_DF_IMAGE_S16)
        SUB_LOOP(y, s16, s16);
    else if (src0->format == VX_DF_IMAGE_S16 && src1->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_S16)
        SUB_LOOP(s16, y, s16);
    else if (src0->format == VX_DF_IMAGE_S16 && src1->format == VX_DF_IMAGE_S16 && dst->format == VX_DF_IMAGE_S16)
        SUB_LOOP(s16, s16, s16);
    else
        FAIL("Unsupported combination of argument formats: %.4s + %.4s = %.4s", &src0->format, &src1->format, &dst->format);

#undef SUB_LOOP
}

static void referenceAdd(CT_Image src0, CT_Image src1, CT_Image src2, CT_Image src3, CT_Image virt1, CT_Image virt2, CT_Image dst, enum vx_convert_policy_e policy)
{
    ASSERT_NO_FAILURE(referenceAddSingle(src0, src1, virt1, policy));
    ASSERT_NO_FAILURE(referenceAddSingle(src2, src3, virt2, policy));
    ASSERT_NO_FAILURE(referenceAddSingle(virt1, virt2, dst, policy));
}

static void referenceSubtract(CT_Image src0, CT_Image src1, CT_Image src2, CT_Image src3, CT_Image virt1, CT_Image virt2, CT_Image dst, enum vx_convert_policy_e policy)
{
    ASSERT_NO_FAILURE(referenceSubtractSingle(src0, src1, virt1, policy));
    ASSERT_NO_FAILURE(referenceSubtractSingle(src2, src3, virt2, policy));
    ASSERT_NO_FAILURE(referenceSubtractSingle(virt1, virt2, dst, policy));
}

typedef vx_node   (VX_API_CALL *vxArithmFunction) (vx_graph, vx_image, vx_image, vx_enum, vx_image);
typedef void      (*referenceFunction)(CT_Image, CT_Image, CT_Image, CT_Image, CT_Image, CT_Image, CT_Image, enum vx_convert_policy_e);

#define SGN_Add "+"
#define SGN_Subtract "-"

typedef struct {
    const char* name;
    enum vx_convert_policy_e policy;
    int width, height;
    vx_df_image arg1_format, arg2_format, result_format;
    vxArithmFunction vxFunc;
    referenceFunction referenceFunc;
} fuzzy_arg;

#define FUZZY_ARG(func, p, w, h, f1, f2, fr)        \
    ARG(#func ": " #p " " #w "x" #h " " #f1 SGN_##func #f2 "=" #fr,   \
        VX_CONVERT_POLICY_##p, w, h, VX_DF_IMAGE_##f1, VX_DF_IMAGE_##f2, VX_DF_IMAGE_##fr, vx##func##Node, reference##func)

#define ARITHM_FUZZY_ARGS(func)                         \
    FUZZY_ARG(func, SATURATE, 18, 18, U8, U8, U8),    \
    FUZZY_ARG(func, SATURATE, 644, 258, U8, U8, S16),   \
    FUZZY_ARG(func, SATURATE, 1600, 1200, U8, S16, S16)

TESTCASE(tivxAddSub,  CT_VXContext, ct_setup_vx_context, 0)

TEST_WITH_ARG(tivxAddSub, testFuzzy, fuzzy_arg, ARITHM_FUZZY_ARGS(Add), ARITHM_FUZZY_ARGS(Subtract))
{
    vx_image src1, src2, src3, src4, dst, virt1, virt2;
    vx_graph graph;
    CT_Image ref1, ref2, ref3, ref4, refdst, vxdst, virt_ctimage1, virt_ctimage2;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_graph;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, arg_->result_format), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg2_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg2_format),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));

    virt_ctimage1 = ct_allocate_image(arg_->width, arg_->height, arg_->result_format);
    virt_ctimage2 = ct_allocate_image(arg_->width, arg_->height, arg_->result_format);

    ASSERT_VX_OBJECT(node1 = arg_->vxFunc(graph, src1, src2, arg_->policy, virt1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = arg_->vxFunc(graph, src3, src4, arg_->policy, virt2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = arg_->vxFunc(graph, virt1, virt2, arg_->policy, dst), VX_TYPE_NODE);

    // run graph
#ifdef CT_EXECUTE_ASYNC
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src1, &src_rect);
    vxGetValidRegionImage(dst, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), arg_->width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), arg_->height);

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
    refdst = ct_allocate_image(arg_->width, arg_->height, arg_->result_format);

    arg_->referenceFunc(ref1, ref2, ref3, ref4, virt_ctimage1, virt_ctimage2, refdst, arg_->policy);

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

TESTCASE_TESTS(tivxAddSub, testFuzzy)
