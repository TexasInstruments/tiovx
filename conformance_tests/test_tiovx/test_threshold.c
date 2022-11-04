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
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_NODES 10

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

static void reference_threshold(CT_Image src, CT_Image dst, vx_enum ttype, int ta, int tb, int true_val, int false_val)
{
    uint32_t x, y, width, height, srcstride, dststride;

    ASSERT(src && dst);
    ASSERT(src->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_U8);
    ASSERT(src->width > 0 && src->height > 0 &&
           src->width == dst->width && src->height == dst->height);
    width = src->width;
    height = src->height;
    srcstride = ct_stride_bytes(src);
    dststride = ct_stride_bytes(dst);

    for( y = 0; y < height; y++ )
    {
        const uint8_t* srcptr = src->data.y + y*srcstride;
        uint8_t* dstptr = dst->data.y + y*dststride;
        if( ttype == VX_THRESHOLD_TYPE_BINARY )
        {
            for( x = 0; x < width; x++ )
                dstptr[x] = srcptr[x] > ta ? true_val : false_val;
        }
        else
        {
            for( x = 0; x < width; x++ )
                dstptr[x] = srcptr[x] < ta || srcptr[x] > tb ? false_val : true_val;
        }
    }
}

static void referenceFunction(CT_Image src, CT_Image srcAdd, CT_Image virt, CT_Image dst, enum vx_convert_policy_e policy,
                              vx_enum ttype, int ta, int tb, int true_val, int false_val)
{
    reference_threshold(src, virt, ttype, ta, tb, true_val, false_val);
    referenceAddSingle(srcAdd, virt, dst, policy);
}


TESTCASE(tivxThreshold, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    int mode;
    vx_enum ttype;
} format_arg;

#define THRESHOLD_CASE(imm, ttype) { #imm "/" #ttype, CT_##imm##_MODE, VX_THRESHOLD_TYPE_##ttype }

#define CT_THRESHOLD_TRUE_VALUE  255
#define CT_THRESHOLD_FALSE_VALUE 0

TEST_WITH_ARG(tivxThreshold, testOnRandom, format_arg,
              THRESHOLD_CASE(Graph, BINARY),
              THRESHOLD_CASE(Graph, RANGE),
              )
{
    int format = VX_DF_IMAGE_U8;
    int ttype = arg_->ttype;
    int mode = arg_->mode;
    vx_image src, dst, srcAdd, virt;
    vx_threshold vxt;
    CT_Image src0, dst0, dst1, srcAdd_ctimage, virt_ctimage;
    vx_node node1 = 0, node2 = 0;
    vx_graph graph = 0;
    vx_context context = context_->vx_context_;
    int iter, niters = 3;
    uint64_t rng;
    int a = 0, b = 256;
    int true_val = CT_THRESHOLD_TRUE_VALUE;
    int false_val = CT_THRESHOLD_FALSE_VALUE;
    vx_perf_t perf_node1, perf_node2, perf_graph;

    rng = CT()->seed_;

    for( iter = 0; iter < niters; iter++ )
    {
        int width, height;

        uint8_t _ta = CT_RNG_NEXT_INT(rng, 0, 256), _tb = CT_RNG_NEXT_INT(rng, 0, 256);
        vx_int32 ta = CT_MIN(_ta, _tb), tb = CT_MAX(_ta, _tb);

        if( ct_check_any_size() )
        {
            width = ct_roundf(ct_log_rng(&rng, 0, 10));
            height = ct_roundf(ct_log_rng(&rng, 0, 10));
            width = CT_MAX(width, 1);
            height = CT_MAX(height, 1);
        }
        else
        {
            width = 640;
            height = 480;
        }

        ASSERT_NO_FAILURE(src0 = ct_allocate_ct_image_random(width, height, format, &rng, a, b));
        ASSERT_NO_FAILURE(srcAdd_ctimage = ct_allocate_ct_image_random(width, height, format, &rng, a, b));
        virt_ctimage = ct_allocate_image(width, height, format);

        ASSERT_NO_FAILURE(dst0 = ct_allocate_image(width, height, format));

        src = ct_image_to_vx_image(src0, context);
        srcAdd = ct_image_to_vx_image(srcAdd_ctimage, context);
        dst = vxCreateImage(context, width, height, format);
        ASSERT_VX_OBJECT(dst, VX_TYPE_IMAGE);
        vxt = vxCreateThreshold(context, ttype, VX_TYPE_UINT8);
        if( ttype == VX_THRESHOLD_TYPE_BINARY )
        {
            vx_int32 v = 0;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_THRESHOLD_VALUE, &ta, sizeof(ta)));
            VX_CALL(vxQueryThreshold(vxt, VX_THRESHOLD_THRESHOLD_VALUE, &v, sizeof(v)));
            if (v != ta)
            {
                CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_VALUE failed\n");
            }
        }
        else
        {
            vx_int32 v1 = 0;
            vx_int32 v2 = 0;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_THRESHOLD_LOWER, &ta, sizeof(ta)));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_THRESHOLD_UPPER, &tb, sizeof(tb)));

            VX_CALL(vxQueryThreshold(vxt, VX_THRESHOLD_THRESHOLD_LOWER, &v1, sizeof(v1)));
            if (v1 != ta)
            {
                CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_LOWER failed\n");
            }

            VX_CALL(vxQueryThreshold(vxt, VX_THRESHOLD_THRESHOLD_UPPER, &v2, sizeof(v2)));
            if (v2 != tb)
            {
                CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_UPPER failed\n");
            }
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_TRUE_VALUE, &true_val, sizeof(true_val)));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_FALSE_VALUE, &false_val, sizeof(false_val)));

        graph = vxCreateGraph(context);
        ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, 0, 0, format), VX_TYPE_IMAGE);
        node1 = vxThresholdNode(graph, src, vxt, virt);
        ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
        node2 = vxAddNode(graph, srcAdd, virt, VX_CONVERT_POLICY_SATURATE, dst);
        ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

        ASSERT_NO_FAILURE(referenceFunction(src0, srcAdd_ctimage, virt_ctimage, dst0, VX_CONVERT_POLICY_SATURATE, ttype, ta, tb, true_val, false_val));

        dst1 = ct_image_from_vx_image(dst);

        ASSERT_CTIMAGE_NEAR(dst0, dst1, 0);
        VX_CALL(vxReleaseImage(&src));
        VX_CALL(vxReleaseImage(&srcAdd));
        VX_CALL(vxReleaseImage(&virt));
        VX_CALL(vxReleaseImage(&dst));
        VX_CALL(vxReleaseThreshold(&vxt));
        VX_CALL(vxReleaseNode(&node1));
        VX_CALL(vxReleaseNode(&node2));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node1 == 0 && node2 == 0 && graph == 0);
        CT_CollectGarbage(CT_GC_IMAGE);

        printPerformance(perf_node1, width*height, "N1");
        printPerformance(perf_node2, width*height, "N2");
        printPerformance(perf_graph, width*height, "G1");
    }
}


TEST_WITH_ARG(tivxThreshold, testThresholdSupernode, format_arg,
              THRESHOLD_CASE(Graph, BINARY),
              THRESHOLD_CASE(Graph, RANGE),
              )
{
    int node_count = 2;
    int format = VX_DF_IMAGE_U8;
    int ttype = arg_->ttype;
    int mode = arg_->mode;
    vx_image src, dst, srcAdd, virt;
    vx_threshold vxt;
    CT_Image src0, dst0, dst1, srcAdd_ctimage, virt_ctimage;
    vx_node node1 = 0, node2 = 0;
    vx_graph graph = 0;
    vx_context context = context_->vx_context_;
    int iter, niters = 3;
    uint64_t rng;
    int a = 0, b = 256;
    int true_val = CT_THRESHOLD_TRUE_VALUE;
    int false_val = CT_THRESHOLD_FALSE_VALUE;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    rng = CT()->seed_;

    for( iter = 0; iter < niters; iter++ )
    {
        int width, height;

        uint8_t _ta = CT_RNG_NEXT_INT(rng, 0, 256), _tb = CT_RNG_NEXT_INT(rng, 0, 256);
        vx_int32 ta = CT_MIN(_ta, _tb), tb = CT_MAX(_ta, _tb);

        if( ct_check_any_size() )
        {
            width = ct_roundf(ct_log_rng(&rng, 0, 10));
            height = ct_roundf(ct_log_rng(&rng, 0, 10));
            width = CT_MAX(width, 1);
            height = CT_MAX(height, 1);
        }
        else
        {
            width = 640;
            height = 480;
        }

        ASSERT_NO_FAILURE(src0 = ct_allocate_ct_image_random(width, height, format, &rng, a, b));
        ASSERT_NO_FAILURE(srcAdd_ctimage = ct_allocate_ct_image_random(width, height, format, &rng, a, b));
        virt_ctimage = ct_allocate_image(width, height, format);

        ASSERT_NO_FAILURE(dst0 = ct_allocate_image(width, height, format));

        src = ct_image_to_vx_image(src0, context);
        srcAdd = ct_image_to_vx_image(srcAdd_ctimage, context);
        dst = vxCreateImage(context, width, height, format);
        ASSERT_VX_OBJECT(dst, VX_TYPE_IMAGE);
        vxt = vxCreateThreshold(context, ttype, VX_TYPE_UINT8);
        if( ttype == VX_THRESHOLD_TYPE_BINARY )
        {
            vx_int32 v = 0;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_THRESHOLD_VALUE, &ta, sizeof(ta)));
            VX_CALL(vxQueryThreshold(vxt, VX_THRESHOLD_THRESHOLD_VALUE, &v, sizeof(v)));
            if (v != ta)
            {
                CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_VALUE failed\n");
            }
        }
        else
        {
            vx_int32 v1 = 0;
            vx_int32 v2 = 0;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_THRESHOLD_LOWER, &ta, sizeof(ta)));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_THRESHOLD_UPPER, &tb, sizeof(tb)));

            VX_CALL(vxQueryThreshold(vxt, VX_THRESHOLD_THRESHOLD_LOWER, &v1, sizeof(v1)));
            if (v1 != ta)
            {
                CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_LOWER failed\n");
            }

            VX_CALL(vxQueryThreshold(vxt, VX_THRESHOLD_THRESHOLD_UPPER, &v2, sizeof(v2)));
            if (v2 != tb)
            {
                CT_FAIL("check for query threshold attribute VX_THRESHOLD_THRESHOLD_UPPER failed\n");
            }
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_TRUE_VALUE, &true_val, sizeof(true_val)));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(vxt, VX_THRESHOLD_FALSE_VALUE, &false_val, sizeof(false_val)));

        graph = vxCreateGraph(context);
        ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, 0, 0, format), VX_TYPE_IMAGE);
        node1 = vxThresholdNode(graph, src, vxt, virt);
        ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
        node2 = vxAddNode(graph, srcAdd, virt, VX_CONVERT_POLICY_SATURATE, dst);
        ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

        ASSERT_NO_FAILURE(node_list[0] = node1); 
        ASSERT_NO_FAILURE(node_list[1] = node2);
        ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
        
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
        VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

        ASSERT_NO_FAILURE(referenceFunction(src0, srcAdd_ctimage, virt_ctimage, dst0, VX_CONVERT_POLICY_SATURATE, ttype, ta, tb, true_val, false_val));

        dst1 = ct_image_from_vx_image(dst);

        ASSERT_CTIMAGE_NEAR(dst0, dst1, 0);
        VX_CALL(vxReleaseImage(&src));
        VX_CALL(vxReleaseImage(&srcAdd));
        VX_CALL(vxReleaseImage(&virt));
        VX_CALL(vxReleaseImage(&dst));
        VX_CALL(vxReleaseThreshold(&vxt));
        VX_CALL(tivxReleaseSuperNode(&super_node));
        VX_CALL(vxReleaseNode(&node1));
        VX_CALL(vxReleaseNode(&node2));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(super_node == 0 && node1 == 0 && node2 == 0 && graph == 0);
        CT_CollectGarbage(CT_GC_IMAGE);

        printPerformance(perf_super_node, width * height, "SN");
        printPerformance(perf_graph, width * height, "G");
    }
}

#ifdef BUILD_BAM
#define testThresholdSupernode testThresholdSupernode
#else
#define testThresholdSupernode DISABLED_testThresholdSupernode
#endif

TEST(tivxThreshold, negativeTestCreateThreshold)
{
    #define VX_THRESHOLD_TYPE_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_threshold thresh = NULL;
    vx_enum thr_type = VX_THRESHOLD_TYPE_DEFAULT, data_type = VX_TYPE_INT64;

    ASSERT(NULL == (thresh = vxCreateThreshold(NULL, thr_type, data_type)));
    ASSERT(NULL == (thresh = vxCreateThreshold(context, thr_type, data_type)));
    thr_type = VX_THRESHOLD_TYPE_BINARY;
    ASSERT(NULL == (thresh = vxCreateThreshold(context, thr_type, data_type)));
}

TEST(tivxThreshold, negativeTestQueryThreshold)
{
    #define VX_THRESHOLD_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_threshold thresh = NULL;
    vx_enum attribute = VX_THRESHOLD_DEFAULT;
    vx_uint32 udata = 0;
    vx_size size = 0;
    vx_enum thr_type = VX_THRESHOLD_TYPE_BINARY, data_type = VX_TYPE_UINT32;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryThreshold(thresh, attribute, &udata, size));
    ASSERT_VX_OBJECT(thresh = vxCreateThreshold(context, thr_type, data_type), VX_TYPE_THRESHOLD);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryThreshold(thresh, VX_THRESHOLD_TYPE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryThreshold(thresh, VX_THRESHOLD_THRESHOLD_VALUE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryThreshold(thresh, VX_THRESHOLD_THRESHOLD_LOWER, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryThreshold(thresh, VX_THRESHOLD_THRESHOLD_UPPER, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryThreshold(thresh, VX_THRESHOLD_TRUE_VALUE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryThreshold(thresh, VX_THRESHOLD_FALSE_VALUE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryThreshold(thresh, VX_THRESHOLD_DATA_TYPE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryThreshold(thresh, attribute, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryThreshold(thresh, VX_THRESHOLD_TRUE_VALUE, &udata, sizeof(vx_int32)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryThreshold(thresh, VX_THRESHOLD_FALSE_VALUE, &udata, sizeof(vx_int32)));
    VX_CALL(vxReleaseThreshold(&thresh));
}

TEST(tivxThreshold, negativeTestSetThresholdAttribute)
{
    #define VX_THRESHOLD_DEFAULT 0
    #define VX_THRESHOLD_TYPE_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_threshold thresh = NULL;
    vx_enum attribute = VX_THRESHOLD_DEFAULT;
    vx_uint32 udata = 0;
    vx_size size = 0;
    vx_enum thr_type = VX_THRESHOLD_TYPE_DEFAULT, data_type = VX_TYPE_UINT32;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetThresholdAttribute(thresh, attribute, &udata, size));
    ASSERT_VX_OBJECT(thresh = vxCreateThreshold(context, VX_THRESHOLD_TYPE_BINARY, data_type), VX_TYPE_THRESHOLD);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetThresholdAttribute(thresh, VX_THRESHOLD_THRESHOLD_VALUE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetThresholdAttribute(thresh, VX_THRESHOLD_THRESHOLD_LOWER, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetThresholdAttribute(thresh, VX_THRESHOLD_THRESHOLD_UPPER, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetThresholdAttribute(thresh, VX_THRESHOLD_TRUE_VALUE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetThresholdAttribute(thresh, VX_THRESHOLD_FALSE_VALUE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetThresholdAttribute(thresh, VX_THRESHOLD_TYPE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetThresholdAttribute(thresh, VX_THRESHOLD_TYPE, &thr_type, sizeof(vx_enum)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetThresholdAttribute(thresh, attribute, &udata, size));
    thr_type = VX_THRESHOLD_TYPE_BINARY;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(thresh, VX_THRESHOLD_TYPE, &thr_type, sizeof(vx_enum)));
    thr_type = VX_THRESHOLD_TYPE_RANGE;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(thresh, VX_THRESHOLD_TYPE, &thr_type, sizeof(vx_enum)));
    VX_CALL(vxReleaseThreshold(&thresh));
}

TESTCASE_TESTS(
    tivxThreshold,
    testOnRandom,
    testThresholdSupernode,
    negativeTestCreateThreshold,
    negativeTestQueryThreshold,
    negativeTestSetThresholdAttribute
)

