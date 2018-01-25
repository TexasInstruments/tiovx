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

TESTCASE(tivxGraph, CT_VXContext, ct_setup_vx_context, 0)

static void referenceAddSingle(CT_Image src0, CT_Image src1, CT_Image dst, enum vx_convert_policy_e policy)
{
    int32_t min_bound, max_bound;
    uint32_t i, j;
    ASSERT(src0 && src1 && dst);
    ASSERT(src0->width = src1->width);
    ASSERT(src0->width == dst->width);
    ASSERT(src0->height = src1->height);
    ASSERT(src0->height == dst->height);

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

static CT_Image median3x3_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static int compare_for_median_get(const void * a, const void * b)
{
    return *(int*)a - *(int*)b;
}

static int32_t median_get(int32_t *values)
{
    qsort(values, 9, sizeof(values[0]), compare_for_median_get);
    return values[4];
}

static uint8_t median3x3_calculate(CT_Image src, uint32_t x, uint32_t y)
{
    int32_t values[9] = {
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 1)
    };
    return (uint8_t)median_get(values);
}

static uint8_t median3x3_calculate_replicate(CT_Image src, uint32_t x_, uint32_t y_)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 1)
    };
    return (uint8_t)median_get(values);
}

static uint8_t median3x3_calculate_constant(CT_Image src, uint32_t x_, uint32_t y_, vx_uint32 constant_value)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 1, constant_value)
    };
    return (uint8_t)median_get(values);
}


static CT_Image median3x3_create_reference_image(CT_Image src, vx_border_t border)
{
    CT_Image dst;

    CT_ASSERT_(return NULL, src->format == VX_DF_IMAGE_U8);

    dst = ct_allocate_image(src->width, src->height, src->format);

    if (border.mode == VX_BORDER_UNDEFINED)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                if (x >= 1 && y >= 1 && x < src->width - 1 && y < src->height - 1)
                {
                    uint8_t res = median3x3_calculate(src, x, y);
                    *dst_data = res;
                });
    }
    else if (border.mode == VX_BORDER_REPLICATE)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = median3x3_calculate_replicate(src, x, y);
                    *dst_data = res;
                });
    }
    else if (border.mode == VX_BORDER_CONSTANT)
    {
        vx_uint32 constant_value = border.constant_value.U32;
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = median3x3_calculate_constant(src, x, y, constant_value);
                    *dst_data = res;
                });
    }
    else
    {
        ASSERT_(return 0, 0);
    }
    return dst;
}

static void referenceAdd(CT_Image src0, CT_Image src1, CT_Image src2, CT_Image src3, CT_Image virt1, CT_Image virt2,
                         CT_Image dst1, CT_Image dst2, enum vx_convert_policy_e policy, vx_border_t border)
{
    CT_Image dst1_ref = NULL, dst2_ref = NULL;

    ASSERT(dst1 && dst2);

    ASSERT_NO_FAILURE(referenceAddSingle(src0, src1, virt1, policy));
    ASSERT_NO_FAILURE(referenceAddSingle(src2, src3, virt2, policy));

    ASSERT_NO_FAILURE(dst1_ref = median3x3_create_reference_image(virt1, border));
    ASSERT_NO_FAILURE(dst2_ref = median3x3_create_reference_image(virt2, border));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst1,  1, 1, 1, 1);
            ct_adjust_roi(dst1_ref, 1, 1, 1, 1);
            ct_adjust_roi(dst2,  1, 1, 1, 1);
            ct_adjust_roi(dst2_ref, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst1_ref, dst1);
    EXPECT_EQ_CTIMAGE(dst2_ref, dst2);
}

static void referenceThreeParallel(CT_Image src0, CT_Image src1, CT_Image virt, CT_Image dst, enum vx_convert_policy_e policy, vx_border_t border)
{
    CT_Image dst_ref = NULL;

    ASSERT(dst);

    ASSERT_NO_FAILURE(referenceAddSingle(src0, src1, virt, policy));

    ASSERT_NO_FAILURE(dst_ref = median3x3_create_reference_image(virt, border));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst,  1, 1, 1, 1);
            ct_adjust_roi(dst_ref, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
}

typedef vx_node   (VX_API_CALL *vxArithmFunction) (vx_graph, vx_image, vx_image, vx_enum, vx_image);
typedef void      (*referenceFunction)(CT_Image, CT_Image, CT_Image, CT_Image, CT_Image, CT_Image, CT_Image, CT_Image, enum vx_convert_policy_e, vx_border_t);

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
    FUZZY_ARG(func, SATURATE, 644, 258, U8, U8, U8),   \
    FUZZY_ARG(func, SATURATE, 1600, 1200, U8, U8, U8)

TEST_WITH_ARG(tivxGraph, testParallelNodesDifferentTarget, fuzzy_arg, ARITHM_FUZZY_ARGS(Add))
{
    vx_image src1, src2, src3, src4, dst1, dst2, virt1, virt2;
    vx_graph graph;
    CT_Image ref1, ref2, ref3, ref4, vxdst1, vxdst2, virt_ctimage1, virt_ctimage2;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_node4, perf_graph;
    vx_border_t border = { VX_BORDER_UNDEFINED, {{ 0 }} };
    int widthHardCoded = 640, heightHardCoded = 480;
    vx_uint16 vendor_id, version;
    vx_char implementation[VX_MAX_IMPLEMENTATION_NAME];
    vx_size extensions_size, nonlinear_max_dim;
    vx_uint32 nkernels = 0;
    vx_kernel kernel   = 0;

    VX_CALL(vxQueryContext(context, VX_CONTEXT_VENDOR_ID, &vendor_id, sizeof(vendor_id)));
    ASSERT(vendor_id == VX_ID_TI);
    VX_CALL(vxQueryContext(context, VX_CONTEXT_VERSION, &version, sizeof(version)));
    ASSERT(version == VX_VERSION_1_1);
    VX_CALL(vxQueryContext(context, VX_CONTEXT_IMPLEMENTATION, &implementation, sizeof(implementation)));
    ASSERT(strcmp(implementation, "tiovx") == 0);
    VX_CALL(vxQueryContext(context, VX_CONTEXT_EXTENSIONS_SIZE, &extensions_size, sizeof(extensions_size)));
    ASSERT(extensions_size == 2);
    VX_CALL(vxQueryContext(context, VX_CONTEXT_NONLINEAR_MAX_DIMENSION, &nonlinear_max_dim, sizeof(nonlinear_max_dim)));
    ASSERT(nonlinear_max_dim == 9);

    vx_char *extensions = malloc(extensions_size*sizeof(vx_char));
    VX_CALL(vxQueryContext(context, VX_CONTEXT_EXTENSIONS, extensions, extensions_size*sizeof(vx_char)));
    ASSERT(strcmp(extensions, " ") == 0);
    free(extensions);

    VX_CALL(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, &nkernels, sizeof(nkernels)));

    vx_kernel_info_t *kernel_table = (vx_kernel_info_t *)malloc(nkernels*sizeof(vx_kernel_info_t));
    VX_CALL(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNEL_TABLE, kernel_table, nkernels*sizeof(vx_kernel_info_t)));
    int i;
    for (i = 0; i < nkernels; i++)
    {
        ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, kernel_table[i].enumeration), VX_TYPE_KERNEL);
        VX_CALL(vxReleaseKernel(&kernel));
        ASSERT_VX_OBJECT(kernel = vxGetKernelByName(context, kernel_table[i].name), VX_TYPE_KERNEL);
        VX_CALL(vxReleaseKernel(&kernel));
    }
    free(kernel_table);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst1   = vxCreateImage(context, arg_->width, arg_->height, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst2   = vxCreateImage(context, widthHardCoded, heightHardCoded, arg_->result_format), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg2_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, widthHardCoded, heightHardCoded, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, widthHardCoded, heightHardCoded, arg_->arg2_format),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));

    virt_ctimage1 = ct_allocate_image(arg_->width, arg_->height, arg_->result_format);
    virt_ctimage2 = ct_allocate_image(widthHardCoded, heightHardCoded, arg_->result_format);

    ASSERT_VX_OBJECT(node1 = arg_->vxFunc(graph, src1, src2, arg_->policy, virt1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = arg_->vxFunc(graph, src3, src4, arg_->policy, virt2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxMedian3x3Node(graph, virt1, dst1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node4 = vxMedian3x3Node(graph, virt2, dst2), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node3, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxSetNodeAttribute(node4, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, "DSP-1"));

    if (vx_true_e == tivxIsTargetEnabled("DSP-2"))
    {
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-2"));
        VX_CALL(vxSetNodeTarget(node4, VX_TARGET_STRING, "DSP-2"));
    }
    else
    {
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-1"));
        VX_CALL(vxSetNodeTarget(node4, VX_TARGET_STRING, "DSP-1"));
    }

    VX_CALL(vxSetNodeTarget(node3, VX_TARGET_STRING, "DSP-1"));


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
    vxQueryNode(node4, VX_NODE_PERFORMANCE, &perf_node4, sizeof(perf_node4));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ref1  = ct_image_from_vx_image(src1);
    ref2  = ct_image_from_vx_image(src2);
    ref3  = ct_image_from_vx_image(src3);
    ref4  = ct_image_from_vx_image(src4);
    vxdst1 = ct_image_from_vx_image(dst1);
    vxdst2 = ct_image_from_vx_image(dst2);

    arg_->referenceFunc(ref1, ref2, ref3, ref4, virt_ctimage1, virt_ctimage2, vxdst1, vxdst2, arg_->policy, border);

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst1));
    VX_CALL(vxReleaseImage(&dst2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, widthHardCoded*heightHardCoded, "N2");
    printPerformance(perf_node3, arg_->width*arg_->height, "N3");
    printPerformance(perf_node4, widthHardCoded*heightHardCoded, "N4");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TEST_WITH_ARG(tivxGraph, testParallelNodesSameTarget, fuzzy_arg, ARITHM_FUZZY_ARGS(Add))
{
    vx_image src1, src2, src3, src4, dst1, dst2, virt1, virt2;
    vx_graph graph;
    CT_Image ref1, ref2, ref3, ref4, vxdst1, vxdst2, virt_ctimage1, virt_ctimage2;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_node4, perf_graph;
    vx_border_t border = { VX_BORDER_UNDEFINED, {{ 0 }} };
    int widthHardCoded = 640, heightHardCoded = 480;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst1   = vxCreateImage(context, arg_->width, arg_->height, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst2   = vxCreateImage(context, widthHardCoded, heightHardCoded, arg_->result_format), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg2_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, widthHardCoded, heightHardCoded, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, widthHardCoded, heightHardCoded, arg_->arg2_format),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));

    virt_ctimage1 = ct_allocate_image(arg_->width, arg_->height, arg_->result_format);
    virt_ctimage2 = ct_allocate_image(widthHardCoded, heightHardCoded, arg_->result_format);

    ASSERT_VX_OBJECT(node1 = arg_->vxFunc(graph, src1, src2, arg_->policy, virt1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = arg_->vxFunc(graph, src3, src4, arg_->policy, virt2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxMedian3x3Node(graph, virt1, dst1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node4 = vxMedian3x3Node(graph, virt2, dst2), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node3, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxSetNodeAttribute(node4, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node3, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node4, VX_TARGET_STRING, "DSP-1"));

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
    vxQueryNode(node4, VX_NODE_PERFORMANCE, &perf_node4, sizeof(perf_node4));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ref1  = ct_image_from_vx_image(src1);
    ref2  = ct_image_from_vx_image(src2);
    ref3  = ct_image_from_vx_image(src3);
    ref4  = ct_image_from_vx_image(src4);
    vxdst1 = ct_image_from_vx_image(dst1);
    vxdst2 = ct_image_from_vx_image(dst2);

    arg_->referenceFunc(ref1, ref2, ref3, ref4, virt_ctimage1, virt_ctimage2, vxdst1, vxdst2, arg_->policy, border);

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&dst1));
    VX_CALL(vxReleaseImage(&dst2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, widthHardCoded*heightHardCoded, "N2");
    printPerformance(perf_node3, arg_->width*arg_->height, "N3");
    printPerformance(perf_node4, widthHardCoded*heightHardCoded, "N4");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TEST_WITH_ARG(tivxGraph, testThreeParallelNodes, fuzzy_arg, ARITHM_FUZZY_ARGS(Add))
{
    vx_image src1, src2, src3, src4, src5, src6, dst1, dst2, dst3, virt1, virt2, virt3;
    vx_graph graph;
    CT_Image ref1, ref2, ref3, ref4, ref5, ref6;
    CT_Image vxdst1, vxdst2, vxdst3, virt_ctimage1, virt_ctimage2, virt_ctimage3;
    vx_context context = context_->vx_context_;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0, node6 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_node4,  perf_node5, perf_node6, perf_graph;
    vx_border_t border = { VX_BORDER_UNDEFINED, {{ 0 }} };
    int widthHardCoded1 = 640, heightHardCoded1 = 480, widthHardCoded2 = 68, heightHardCoded2 = 42;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt3   = vxCreateVirtualImage(graph, 0, 0, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst1   = vxCreateImage(context, arg_->width, arg_->height, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst2   = vxCreateImage(context, widthHardCoded1, heightHardCoded1, arg_->result_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst3   = vxCreateImage(context, widthHardCoded2, heightHardCoded2, arg_->result_format), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = vxCreateImage(context, arg_->width, arg_->height, arg_->arg2_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = vxCreateImage(context, widthHardCoded1, heightHardCoded1, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = vxCreateImage(context, widthHardCoded1, heightHardCoded1, arg_->arg2_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src5 = vxCreateImage(context, widthHardCoded2, heightHardCoded2, arg_->arg1_format),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src6 = vxCreateImage(context, widthHardCoded2, heightHardCoded2, arg_->arg2_format),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src5, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src6, &CT()->seed_));

    virt_ctimage1 = ct_allocate_image(arg_->width, arg_->height, arg_->result_format);
    virt_ctimage2 = ct_allocate_image(widthHardCoded1, heightHardCoded1, arg_->result_format);
    virt_ctimage3 = ct_allocate_image(widthHardCoded2, heightHardCoded2, arg_->result_format);

    ASSERT_VX_OBJECT(node1 = arg_->vxFunc(graph, src1, src2, arg_->policy, virt1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = arg_->vxFunc(graph, src3, src4, arg_->policy, virt2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = arg_->vxFunc(graph, src5, src6, arg_->policy, virt3), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node4 = vxMedian3x3Node(graph, virt1, dst1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node5 = vxMedian3x3Node(graph, virt2, dst2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node6 = vxMedian3x3Node(graph, virt3, dst3), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node4, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxSetNodeAttribute(node5, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxSetNodeAttribute(node6, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, "DSP-1"));

    if (vx_true_e == tivxIsTargetEnabled("DSP-2"))
    {
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-2"));
        VX_CALL(vxSetNodeTarget(node5, VX_TARGET_STRING, "DSP-2"));
        VX_CALL(vxSetNodeTarget(node6, VX_TARGET_STRING, "DSP-2"));
    }
    else
    {
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-1"));
        VX_CALL(vxSetNodeTarget(node5, VX_TARGET_STRING, "DSP-1"));
        VX_CALL(vxSetNodeTarget(node6, VX_TARGET_STRING, "DSP-1"));
    }

    VX_CALL(vxSetNodeTarget(node3, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node4, VX_TARGET_STRING, "DSP-1"));


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
    vxQueryNode(node4, VX_NODE_PERFORMANCE, &perf_node4, sizeof(perf_node4));
    vxQueryNode(node5, VX_NODE_PERFORMANCE, &perf_node5, sizeof(perf_node5));
    vxQueryNode(node6, VX_NODE_PERFORMANCE, &perf_node6, sizeof(perf_node6));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ref1  = ct_image_from_vx_image(src1);
    ref2  = ct_image_from_vx_image(src2);
    ref3  = ct_image_from_vx_image(src3);
    ref4  = ct_image_from_vx_image(src4);
    ref5  = ct_image_from_vx_image(src5);
    ref6  = ct_image_from_vx_image(src6);
    vxdst1 = ct_image_from_vx_image(dst1);
    vxdst2 = ct_image_from_vx_image(dst2);
    vxdst3 = ct_image_from_vx_image(dst3);

    referenceThreeParallel(ref1, ref2, virt_ctimage1, vxdst1, arg_->policy, border);
    referenceThreeParallel(ref3, ref4, virt_ctimage2, vxdst2, arg_->policy, border);
    referenceThreeParallel(ref5, ref6, virt_ctimage3, vxdst3, arg_->policy, border);

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&src5));
    VX_CALL(vxReleaseImage(&src6));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&virt3));
    VX_CALL(vxReleaseImage(&dst1));
    VX_CALL(vxReleaseImage(&dst2));
    VX_CALL(vxReleaseImage(&dst3));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseNode(&node6));
    VX_CALL(vxReleaseGraph(&graph));

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, widthHardCoded1*heightHardCoded1, "N2");
    printPerformance(perf_node3, widthHardCoded2*heightHardCoded2, "N3");
    printPerformance(perf_node4, arg_->width*arg_->height, "N4");
    printPerformance(perf_node5, widthHardCoded1*heightHardCoded1, "N5");
    printPerformance(perf_node6, widthHardCoded2*heightHardCoded2, "N6");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}


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
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

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
}

static void alternate_node_check(CT_Image input_not, CT_Image input_acc_1, CT_Image input_acc_2, CT_Image virtual_dummy_1,
            CT_Image virtual_dummy_2, CT_Image virtual_dummy_3, vx_float32 alpha_intermediate1, vx_float32 alpha_intermediate2,
            vx_float32 alpha_final, CT_Image accum_final, CT_Image accum_dst)
{
    CT_Image accum_ref = NULL, accum_intermediate_1 = NULL, accum_intermediate_2 = NULL;

    ASSERT(input_not && input_acc_1 && input_acc_2 && accum_final && accum_dst);

    ASSERT_NO_FAILURE(accum_ref = ct_image_create_clone(accum_final));

    ASSERT_NO_FAILURE(accum_intermediate_1 = ct_image_create_clone(input_acc_1));

    ASSERT_NO_FAILURE(accum_intermediate_2 = ct_image_create_clone(input_acc_2));

    ASSERT_NO_FAILURE(referenceNot(input_not, virtual_dummy_1));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(virtual_dummy_1, alpha_intermediate1, accum_intermediate_1));

    ASSERT_NO_FAILURE(referenceNot(accum_intermediate_1, virtual_dummy_2));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(virtual_dummy_2, alpha_intermediate2, accum_intermediate_2));

    ASSERT_NO_FAILURE(referenceNot(accum_intermediate_2, virtual_dummy_3));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(virtual_dummy_3, alpha_final, accum_ref));

    EXPECT_CTIMAGE_NEAR(accum_ref, accum_dst, 2); // Changed tolerance to 2
}

static void referenceAddThree(CT_Image src0, CT_Image src1, CT_Image src2, CT_Image src3, CT_Image virt1, CT_Image virt2, CT_Image dst, enum vx_convert_policy_e policy)
{
    ASSERT_NO_FAILURE(referenceAddSingle(src0, src1, virt1, policy));
    ASSERT_NO_FAILURE(referenceAddSingle(src2, src3, virt2, policy));
    ASSERT_NO_FAILURE(referenceAddSingle(virt1, virt2, dst, policy));
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

TEST_WITH_ARG(tivxGraph, testVirtualDataObject, Arg,
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

TEST_WITH_ARG(tivxGraph, testParallelGraphs, Arg,
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

    if (vx_true_e == tivxIsTargetEnabled("DSP-2"))
    {
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-2"));
    }
    else
    {
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-1"));
    }

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

TEST_WITH_ARG(tivxGraph, testParallelGraphsMultipleNodes, Arg,
    PARAMETERS
)
{
    vx_image src1_graph1, src2_graph1, src3_graph1, src4_graph1, dst_graph1, virt1_graph1, virt2_graph1;
    vx_image src1_graph2, src2_graph2, src3_graph2, src4_graph2, dst_graph2, virt1_graph2, virt2_graph2;
    vx_graph graph1, graph2;
    CT_Image ref1_graph1, ref2_graph1, ref3_graph1, ref4_graph1, refdst_graph1, vxdst_graph1, virt_ctimage1_graph1, virt_ctimage2_graph1;
    CT_Image ref1_graph2, ref2_graph2, ref3_graph2, ref4_graph2, refdst_graph2, vxdst_graph2, virt_ctimage1_graph2, virt_ctimage2_graph2;
    vx_context context = context_->vx_context_;
    vx_node node1_graph1 = 0, node2_graph1 = 0, node3_graph1 = 0;
    vx_node node1_graph2 = 0, node2_graph2 = 0, node3_graph2 = 0;
    vx_perf_t perf_node1_graph1, perf_node2_graph1, perf_node3_graph1, perf_graph1;
    vx_perf_t perf_node1_graph2, perf_node2_graph2, perf_node3_graph2, perf_graph2;

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(virt1_graph1   = vxCreateVirtualImage(graph1, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2_graph1   = vxCreateVirtualImage(graph1, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt1_graph2   = vxCreateVirtualImage(graph2, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt2_graph2   = vxCreateVirtualImage(graph2, 0, 0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_graph1   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_graph2   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    refdst_graph1 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_S16);
    refdst_graph2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_S16);

    ASSERT_VX_OBJECT(src1_graph1 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2_graph1 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3_graph1 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4_graph1 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1_graph2 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2_graph2 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3_graph2 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4_graph2 = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1_graph1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2_graph1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3_graph1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4_graph1, &CT()->seed_));

    ASSERT_NO_FAILURE(ct_fill_image_random(src1_graph2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2_graph2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src3_graph2, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src4_graph2, &CT()->seed_));

    virt_ctimage1_graph1 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
    virt_ctimage2_graph1 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
    virt_ctimage1_graph2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
    virt_ctimage2_graph2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    ASSERT_VX_OBJECT(node1_graph1 = vxAddNode(graph1, src1_graph1, src2_graph1, VX_CONVERT_POLICY_SATURATE, virt1_graph1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2_graph1 = vxAddNode(graph1, src3_graph1, src4_graph1, VX_CONVERT_POLICY_SATURATE, virt2_graph1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3_graph1 = vxAddNode(graph1, virt1_graph1, virt2_graph1, VX_CONVERT_POLICY_SATURATE, dst_graph1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node1_graph2 = vxAddNode(graph2, src1_graph2, src2_graph2, VX_CONVERT_POLICY_SATURATE, virt1_graph2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2_graph2 = vxAddNode(graph2, src3_graph2, src4_graph2, VX_CONVERT_POLICY_SATURATE, virt2_graph2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3_graph2 = vxAddNode(graph2, virt1_graph2, virt2_graph2, VX_CONVERT_POLICY_SATURATE, dst_graph2), VX_TYPE_NODE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph2));

    vxQueryNode(node1_graph1, VX_NODE_PERFORMANCE, &perf_node1_graph1, sizeof(perf_node1_graph1));
    vxQueryNode(node2_graph1, VX_NODE_PERFORMANCE, &perf_node2_graph1, sizeof(perf_node2_graph1));
    vxQueryNode(node3_graph1, VX_NODE_PERFORMANCE, &perf_node3_graph1, sizeof(perf_node3_graph1));
    vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));
    vxQueryNode(node1_graph2, VX_NODE_PERFORMANCE, &perf_node1_graph2, sizeof(perf_node1_graph2));
    vxQueryNode(node2_graph2, VX_NODE_PERFORMANCE, &perf_node2_graph2, sizeof(perf_node2_graph2));
    vxQueryNode(node3_graph2, VX_NODE_PERFORMANCE, &perf_node3_graph2, sizeof(perf_node3_graph2));
    vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));

    ref1_graph1  = ct_image_from_vx_image(src1_graph1);
    ref2_graph1  = ct_image_from_vx_image(src2_graph1);
    ref3_graph1  = ct_image_from_vx_image(src3_graph1);
    ref4_graph1  = ct_image_from_vx_image(src4_graph1);
    vxdst_graph1 = ct_image_from_vx_image(dst_graph1);
    ref1_graph2  = ct_image_from_vx_image(src1_graph2);
    ref2_graph2  = ct_image_from_vx_image(src2_graph2);
    ref3_graph2  = ct_image_from_vx_image(src3_graph2);
    ref4_graph2  = ct_image_from_vx_image(src4_graph2);
    vxdst_graph2 = ct_image_from_vx_image(dst_graph2);

    referenceAddThree(ref1_graph1, ref2_graph1, ref3_graph1, ref4_graph1, virt_ctimage1_graph1, virt_ctimage2_graph1, refdst_graph1, VX_CONVERT_POLICY_SATURATE);
    referenceAddThree(ref1_graph2, ref2_graph2, ref3_graph2, ref4_graph2, virt_ctimage1_graph2, virt_ctimage2_graph2, refdst_graph2, VX_CONVERT_POLICY_SATURATE);

    ASSERT_EQ_CTIMAGE(refdst_graph1, vxdst_graph1);
    ASSERT_EQ_CTIMAGE(refdst_graph2, vxdst_graph2);

    printf("First execution performance:\n");
    printPerformance(perf_node1_graph1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2_graph1, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3_graph1, arg_->width*arg_->height, "N3");
    printPerformance(perf_graph1, arg_->width*arg_->height, "G1");
    printPerformance(perf_node1_graph2, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2_graph2, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3_graph2, arg_->width*arg_->height, "N3");
    printPerformance(perf_graph2, arg_->width*arg_->height, "G2");

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph2));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph1));

    vxQueryNode(node1_graph1, VX_NODE_PERFORMANCE, &perf_node1_graph1, sizeof(perf_node1_graph1));
    vxQueryNode(node2_graph1, VX_NODE_PERFORMANCE, &perf_node2_graph1, sizeof(perf_node2_graph1));
    vxQueryNode(node3_graph1, VX_NODE_PERFORMANCE, &perf_node3_graph1, sizeof(perf_node3_graph1));
    vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));
    vxQueryNode(node1_graph2, VX_NODE_PERFORMANCE, &perf_node1_graph2, sizeof(perf_node1_graph2));
    vxQueryNode(node2_graph2, VX_NODE_PERFORMANCE, &perf_node2_graph2, sizeof(perf_node2_graph2));
    vxQueryNode(node3_graph2, VX_NODE_PERFORMANCE, &perf_node3_graph2, sizeof(perf_node3_graph2));
    vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));

    ref1_graph1  = ct_image_from_vx_image(src1_graph1);
    ref2_graph1  = ct_image_from_vx_image(src2_graph1);
    ref3_graph1  = ct_image_from_vx_image(src3_graph1);
    ref4_graph1  = ct_image_from_vx_image(src4_graph1);
    vxdst_graph1 = ct_image_from_vx_image(dst_graph1);
    ref1_graph2  = ct_image_from_vx_image(src1_graph2);
    ref2_graph2  = ct_image_from_vx_image(src2_graph2);
    ref3_graph2  = ct_image_from_vx_image(src3_graph2);
    ref4_graph2  = ct_image_from_vx_image(src4_graph2);
    vxdst_graph2 = ct_image_from_vx_image(dst_graph2);

    referenceAddThree(ref1_graph1, ref2_graph1, ref3_graph1, ref4_graph1, virt_ctimage1_graph1, virt_ctimage2_graph1, refdst_graph1, VX_CONVERT_POLICY_SATURATE);
    referenceAddThree(ref1_graph2, ref2_graph2, ref3_graph2, ref4_graph2, virt_ctimage1_graph2, virt_ctimage2_graph2, refdst_graph2, VX_CONVERT_POLICY_SATURATE);

    ASSERT_EQ_CTIMAGE(refdst_graph1, vxdst_graph1);
    ASSERT_EQ_CTIMAGE(refdst_graph2, vxdst_graph2);

    printf("Second execution performance:\n");
    printPerformance(perf_node1_graph1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2_graph1, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3_graph1, arg_->width*arg_->height, "N3");
    printPerformance(perf_graph1, arg_->width*arg_->height, "G1");
    printPerformance(perf_node1_graph2, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2_graph2, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3_graph2, arg_->width*arg_->height, "N3");
    printPerformance(perf_graph2, arg_->width*arg_->height, "G2");

    VX_CALL(vxReleaseImage(&src1_graph1));
    VX_CALL(vxReleaseImage(&src2_graph1));
    VX_CALL(vxReleaseImage(&src3_graph1));
    VX_CALL(vxReleaseImage(&src4_graph1));
    VX_CALL(vxReleaseImage(&virt1_graph1));
    VX_CALL(vxReleaseImage(&virt2_graph1));
    VX_CALL(vxReleaseImage(&dst_graph1));
    VX_CALL(vxReleaseNode(&node1_graph1));
    VX_CALL(vxReleaseNode(&node2_graph1));
    VX_CALL(vxReleaseNode(&node3_graph1));
    VX_CALL(vxReleaseGraph(&graph1));

    VX_CALL(vxReleaseImage(&src1_graph2));
    VX_CALL(vxReleaseImage(&src2_graph2));
    VX_CALL(vxReleaseImage(&src3_graph2));
    VX_CALL(vxReleaseImage(&src4_graph2));
    VX_CALL(vxReleaseImage(&virt1_graph2));
    VX_CALL(vxReleaseImage(&virt2_graph2));
    VX_CALL(vxReleaseImage(&dst_graph2));
    VX_CALL(vxReleaseNode(&node1_graph2));
    VX_CALL(vxReleaseNode(&node2_graph2));
    VX_CALL(vxReleaseNode(&node3_graph2));
    VX_CALL(vxReleaseGraph(&graph2));
}

TEST_WITH_ARG(tivxGraph, testThreeParallelGraphs, Arg,
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

    if (vx_true_e == tivxIsTargetEnabled("DSP-2"))
    {
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-2"));
    }
    else
    {
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-1"));
    }

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

TEST_WITH_ARG(tivxGraph, testMaxParallelGraphs, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image1 = 0, input_image2 = 0, input_image3 = 0, input_image4 = 0;
    vx_image input_image5 = 0, input_image6 = 0, input_image7 = 0, input_image8 = 0;
    vx_image accum_image_final1 = 0, accum_image_final2 = 0, accum_image_final3 = 0, accum_image_final4 = 0;
    vx_image accum_image_final5 = 0, accum_image_final6 = 0, accum_image_final7 = 0, accum_image_final8 = 0;
    vx_scalar alpha_scalar1, alpha_scalar2 = 0, alpha_scalar3 = 0, alpha_scalar4 = 0;
    vx_scalar alpha_scalar5, alpha_scalar6 = 0, alpha_scalar7 = 0, alpha_scalar8 = 0;
    vx_graph graph1 = 0, graph2 = 0, graph3 = 0, graph4 = 0, graph5 = 0, graph6 = 0, graph7 = 0, graph8 = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0, node6 = 0, node7 = 0, node8 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_node4, perf_node5, perf_node6, perf_node7, perf_node8;
    vx_perf_t perf_graph1, perf_graph2, perf_graph3, perf_graph4, perf_graph5, perf_graph6, perf_graph7, perf_graph8;

    CT_Image input1 = NULL, input2 = NULL, input3 = NULL, input4 = NULL, input5 = NULL, input6 = NULL, input7 = NULL, input8 = NULL;
    CT_Image accum_final1 = NULL, accum_final2 = NULL, accum_final3 = NULL, accum_final4 = NULL;
    CT_Image accum_final5 = NULL, accum_final6 = NULL, accum_final7 = NULL, accum_final8 = NULL;
    CT_Image accum_dst1 = NULL, accum_dst2 = NULL, accum_dst3 = NULL, accum_dst4 = NULL;
    CT_Image accum_dst5 = NULL, accum_dst6 = NULL, accum_dst7 = NULL, accum_dst8 = NULL;
    vx_float32 sh_node5 = 0.4;
    vx_float32 sh_node6 = 0.2;
    vx_float32 sh_node7 = 0.99;
    vx_float32 sh_node8 = 0.56;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(alpha_scalar1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(alpha_scalar2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(alpha_scalar3 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(alpha_scalar4 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(alpha_scalar5 = vxCreateScalar(context, VX_TYPE_FLOAT32, &sh_node5), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(alpha_scalar6 = vxCreateScalar(context, VX_TYPE_FLOAT32, &sh_node6), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(alpha_scalar7 = vxCreateScalar(context, VX_TYPE_FLOAT32, &sh_node7), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(alpha_scalar8 = vxCreateScalar(context, VX_TYPE_FLOAT32, &sh_node8), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));
    ASSERT_NO_FAILURE(input2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));
    ASSERT_NO_FAILURE(input3 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));
    ASSERT_NO_FAILURE(input4 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));
    ASSERT_NO_FAILURE(input5 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));
    ASSERT_NO_FAILURE(input6 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));
    ASSERT_NO_FAILURE(input7 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));
    ASSERT_NO_FAILURE(input8 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));
    ASSERT_NO_FAILURE(accum_final2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));
    ASSERT_NO_FAILURE(accum_final3 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));
    ASSERT_NO_FAILURE(accum_final4 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));
    ASSERT_NO_FAILURE(accum_final5 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));
    ASSERT_NO_FAILURE(accum_final6 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));
    ASSERT_NO_FAILURE(accum_final7 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));
    ASSERT_NO_FAILURE(accum_final8 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image1 = ct_image_to_vx_image(input1, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(input_image2 = ct_image_to_vx_image(input2, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(input_image3 = ct_image_to_vx_image(input3, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(input_image4 = ct_image_to_vx_image(input4, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(input_image5 = ct_image_to_vx_image(input5, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(input_image6 = ct_image_to_vx_image(input6, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(input_image7 = ct_image_to_vx_image(input7, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(input_image8 = ct_image_to_vx_image(input8, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final1 = ct_image_to_vx_image(accum_final1, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum_image_final2 = ct_image_to_vx_image(accum_final2, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum_image_final3 = ct_image_to_vx_image(accum_final3, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum_image_final4 = ct_image_to_vx_image(accum_final4, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum_image_final5 = ct_image_to_vx_image(accum_final5, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum_image_final6 = ct_image_to_vx_image(accum_final6, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum_image_final7 = ct_image_to_vx_image(accum_final7, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum_image_final8 = ct_image_to_vx_image(accum_final8, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph3 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph4 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph5 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph6 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph7 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph8 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxAccumulateWeightedImageNode(graph1, input_image1, alpha_scalar1, accum_image_final1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxAccumulateWeightedImageNode(graph2, input_image2, alpha_scalar2, accum_image_final2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxAccumulateWeightedImageNode(graph3, input_image3, alpha_scalar3, accum_image_final3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxAccumulateWeightedImageNode(graph4, input_image4, alpha_scalar4, accum_image_final4), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node5 = vxAccumulateWeightedImageNode(graph5, input_image5, alpha_scalar5, accum_image_final5), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node6 = vxAccumulateWeightedImageNode(graph6, input_image6, alpha_scalar6, accum_image_final6), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node7 = vxAccumulateWeightedImageNode(graph7, input_image7, alpha_scalar7, accum_image_final7), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node8 = vxAccumulateWeightedImageNode(graph8, input_image8, alpha_scalar8, accum_image_final8), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, "DSP-1"));
    if (vx_true_e == tivxIsTargetEnabled("DSP-2"))
    {
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-2"));
    }
    else
    {
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-1"));
    }

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph3));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph4));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph5));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph6));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph7));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph8));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph3));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph4));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph5));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph6));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph7));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph8));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryNode(node4, VX_NODE_PERFORMANCE, &perf_node4, sizeof(perf_node4));
    vxQueryNode(node5, VX_NODE_PERFORMANCE, &perf_node5, sizeof(perf_node5));
    vxQueryNode(node6, VX_NODE_PERFORMANCE, &perf_node6, sizeof(perf_node6));
    vxQueryNode(node7, VX_NODE_PERFORMANCE, &perf_node7, sizeof(perf_node7));
    vxQueryNode(node8, VX_NODE_PERFORMANCE, &perf_node8, sizeof(perf_node8));

    vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));
    vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));
    vxQueryGraph(graph3, VX_GRAPH_PERFORMANCE, &perf_graph3, sizeof(perf_graph3));
    vxQueryGraph(graph4, VX_GRAPH_PERFORMANCE, &perf_graph4, sizeof(perf_graph4));
    vxQueryGraph(graph5, VX_GRAPH_PERFORMANCE, &perf_graph5, sizeof(perf_graph5));
    vxQueryGraph(graph6, VX_GRAPH_PERFORMANCE, &perf_graph6, sizeof(perf_graph6));
    vxQueryGraph(graph7, VX_GRAPH_PERFORMANCE, &perf_graph7, sizeof(perf_graph7));
    vxQueryGraph(graph8, VX_GRAPH_PERFORMANCE, &perf_graph8, sizeof(perf_graph8));

    ASSERT_NO_FAILURE(accum_dst1 = ct_image_from_vx_image(accum_image_final1));
    ASSERT_NO_FAILURE(accum_dst2 = ct_image_from_vx_image(accum_image_final2));
    ASSERT_NO_FAILURE(accum_dst3 = ct_image_from_vx_image(accum_image_final3));
    ASSERT_NO_FAILURE(accum_dst4 = ct_image_from_vx_image(accum_image_final4));
    ASSERT_NO_FAILURE(accum_dst5 = ct_image_from_vx_image(accum_image_final5));
    ASSERT_NO_FAILURE(accum_dst6 = ct_image_from_vx_image(accum_image_final6));
    ASSERT_NO_FAILURE(accum_dst7 = ct_image_from_vx_image(accum_image_final7));
    ASSERT_NO_FAILURE(accum_dst8 = ct_image_from_vx_image(accum_image_final8));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input1, arg_->alpha_intermediate, accum_final1, accum_dst1));
    ASSERT_NO_FAILURE(accumulate_weighted_check(input2, arg_->alpha_final, accum_final2, accum_dst2));
    ASSERT_NO_FAILURE(accumulate_weighted_check(input3, arg_->alpha_final, accum_final3, accum_dst3));
    ASSERT_NO_FAILURE(accumulate_weighted_check(input4, arg_->alpha_intermediate, accum_final4, accum_dst4));
    ASSERT_NO_FAILURE(accumulate_weighted_check(input5, sh_node5, accum_final5, accum_dst5));
    ASSERT_NO_FAILURE(accumulate_weighted_check(input6, sh_node6, accum_final6, accum_dst6));
    ASSERT_NO_FAILURE(accumulate_weighted_check(input7, sh_node7, accum_final7, accum_dst7));
    ASSERT_NO_FAILURE(accumulate_weighted_check(input8, sh_node8, accum_final8, accum_dst8));

    VX_CALL(vxReleaseNode(&node8));
    VX_CALL(vxReleaseNode(&node7));
    VX_CALL(vxReleaseNode(&node6));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));

    VX_CALL(vxReleaseGraph(&graph8));
    VX_CALL(vxReleaseGraph(&graph7));
    VX_CALL(vxReleaseGraph(&graph6));
    VX_CALL(vxReleaseGraph(&graph5));
    VX_CALL(vxReleaseGraph(&graph4));
    VX_CALL(vxReleaseGraph(&graph3));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseGraph(&graph1));

    ASSERT(node8 == 0);
    ASSERT(node7 == 0);
    ASSERT(node6 == 0);
    ASSERT(node5 == 0);
    ASSERT(node4 == 0);
    ASSERT(node3 == 0);
    ASSERT(node2 == 0);
    ASSERT(node1 == 0);

    ASSERT(graph8 == 0);
    ASSERT(graph7 == 0);
    ASSERT(graph6 == 0);
    ASSERT(graph5 == 0);
    ASSERT(graph4 == 0);
    ASSERT(graph3 == 0);
    ASSERT(graph2 == 0);
    ASSERT(graph1 == 0);

    VX_CALL(vxReleaseImage(&accum_image_final8));
    VX_CALL(vxReleaseImage(&accum_image_final7));
    VX_CALL(vxReleaseImage(&accum_image_final6));
    VX_CALL(vxReleaseImage(&accum_image_final5));
    VX_CALL(vxReleaseImage(&accum_image_final4));
    VX_CALL(vxReleaseImage(&accum_image_final3));
    VX_CALL(vxReleaseImage(&accum_image_final2));
    VX_CALL(vxReleaseImage(&accum_image_final1));

    VX_CALL(vxReleaseImage(&input_image8));
    VX_CALL(vxReleaseImage(&input_image7));
    VX_CALL(vxReleaseImage(&input_image6));
    VX_CALL(vxReleaseImage(&input_image5));
    VX_CALL(vxReleaseImage(&input_image4));
    VX_CALL(vxReleaseImage(&input_image3));
    VX_CALL(vxReleaseImage(&input_image2));
    VX_CALL(vxReleaseImage(&input_image1));

    VX_CALL(vxReleaseScalar(&alpha_scalar8));
    VX_CALL(vxReleaseScalar(&alpha_scalar7));
    VX_CALL(vxReleaseScalar(&alpha_scalar6));
    VX_CALL(vxReleaseScalar(&alpha_scalar5));
    VX_CALL(vxReleaseScalar(&alpha_scalar4));
    VX_CALL(vxReleaseScalar(&alpha_scalar3));
    VX_CALL(vxReleaseScalar(&alpha_scalar2));
    VX_CALL(vxReleaseScalar(&alpha_scalar1));

    ASSERT(accum_image_final1 == 0);
    ASSERT(accum_image_final2 == 0);
    ASSERT(accum_image_final3 == 0);
    ASSERT(accum_image_final4 == 0);
    ASSERT(accum_image_final5 == 0);
    ASSERT(accum_image_final6 == 0);
    ASSERT(accum_image_final7 == 0);
    ASSERT(accum_image_final8 == 0);

    ASSERT(input_image1 == 0);
    ASSERT(input_image2 == 0);
    ASSERT(input_image3 == 0);
    ASSERT(input_image4 == 0);
    ASSERT(input_image5 == 0);
    ASSERT(input_image6 == 0);
    ASSERT(input_image7 == 0);
    ASSERT(input_image8 == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3, arg_->width*arg_->height, "N3");
    printPerformance(perf_node4, arg_->width*arg_->height, "N4");
    printPerformance(perf_node5, arg_->width*arg_->height, "N5");
    printPerformance(perf_node6, arg_->width*arg_->height, "N6");
    printPerformance(perf_node7, arg_->width*arg_->height, "N7");
    printPerformance(perf_node8, arg_->width*arg_->height, "N8");

    printPerformance(perf_graph1, arg_->width*arg_->height, "G1");
    printPerformance(perf_graph2, arg_->width*arg_->height, "G2");
    printPerformance(perf_graph3, arg_->width*arg_->height, "G3");
    printPerformance(perf_graph4, arg_->width*arg_->height, "G4");
    printPerformance(perf_graph5, arg_->width*arg_->height, "G5");
    printPerformance(perf_graph6, arg_->width*arg_->height, "G6");
    printPerformance(perf_graph7, arg_->width*arg_->height, "G7");
    printPerformance(perf_graph8, arg_->width*arg_->height, "G8");
}

// Testing alternating nodes
// vxNot -> vxAccumulateSquare -> vxNot -> vxAccumulateSquare -> vxNot -> vxAccumulateSquare
TEST_WITH_ARG(tivxGraph, testAlternatingNodes, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image_not = 0, accum_image_final = 0;
    vx_image input_image_acc_1 = 0, input_image_acc_2 = 0;
    vx_image accum_image_virtual_1 = 0, accum_image_virtual_2 = 0, accum_image_virtual_3 = 0;
    vx_scalar alpha_scalar_1, alpha_scalar_2, alpha_scalar_3 = 0;
    vx_float32 sh = 0.5f;
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

    ASSERT_VX_OBJECT(alpha_scalar_2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &sh), VX_TYPE_SCALAR);

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
                        virtual_dummy_3, arg_->alpha_intermediate, sh, arg_->alpha_final, accum_final, accum_dst));

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

TESTCASE_TESTS(tivxGraph,
        testParallelNodesDifferentTarget,
        testParallelNodesSameTarget,
        testThreeParallelNodes,
        testVirtualDataObject,
        testParallelGraphs,
        testParallelGraphsMultipleNodes,
        testThreeParallelGraphs,
        testMaxParallelGraphs,
        testAlternatingNodes
)
