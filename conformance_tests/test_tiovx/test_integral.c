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

TESTCASE(tivxIntegral, CT_VXContext, ct_setup_vx_context, 0)

static CT_Image integral_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static CT_Image integral_create_reference_image(CT_Image src)
{
    CT_Image dst;

    CT_ASSERT_(return NULL, src->format == VX_DF_IMAGE_U8);

    dst = ct_allocate_image(src->width, src->height, VX_DF_IMAGE_U32);

    CT_FILL_IMAGE_32U(return 0, dst,
            {
                uint32_t res = *CT_IMAGE_DATA_PTR_8U(src, x, y);
                if (y > 0)
                    res += *CT_IMAGE_DATA_PTR_32U(dst, x, y - 1);
                if (x > 0)
                    res += *CT_IMAGE_DATA_PTR_32U(dst, x - 1, y);
                if (y > 0 && x > 0)
                    res -= *CT_IMAGE_DATA_PTR_32U(dst, x - 1, y - 1);
                *dst_data = res;
            });
    return dst;
}


static void integral_check(CT_Image src, CT_Image dst)
{
    CT_Image dst_ref = NULL;

    ASSERT(src && dst);

    ASSERT_NO_FAILURE(dst_ref = integral_create_reference_image(src));

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
#if 0
    if (CT_HasFailure())
    {
        printf("=== SRC ===\n");
        ct_dump_image_info_ex(src, 16, 4);
        printf("=== DST ===\n");
        ct_dump_image_info_ex(dst, 16, 4);
        printf("=== EXPECTED ===\n");
        ct_dump_image_info_ex(dst_ref, 16, 4);
    }
#endif
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int width, height;
} Arg;


#define PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_18x18, ARG, integral_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_644x258, ARG, integral_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_1600x1200, ARG, integral_generate_random, NULL)

TEST_WITH_ARG(tivxIntegral, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src0_image = 0, dst0_image = 0, src1_image = 0, dst1_image = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;
    vx_bool valid_rect;

    CT_Image src0 = NULL, dst0 = NULL, src1 = NULL, dst1 = NULL;

    ASSERT_NO_FAILURE(src0 = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_NO_FAILURE(src1 = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src0_image = ct_image_to_vx_image(src0, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src1_image = ct_image_to_vx_image(src1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst0_image = vxCreateImage(context, src0->width, src0->height, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst1_image = vxCreateImage(context, src1->width, src1->height, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxIntegralImageNode(graph, src0_image, dst0_image), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxIntegralImageNode(graph, src1_image, dst1_image), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(dst0 = ct_image_from_vx_image(dst0_image));
    ASSERT_NO_FAILURE(dst1 = ct_image_from_vx_image(dst1_image));

    ASSERT_NO_FAILURE(integral_check(src0, dst0));
    ASSERT_NO_FAILURE(integral_check(src1, dst1));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst1_image));
    VX_CALL(vxReleaseImage(&dst0_image));
    VX_CALL(vxReleaseImage(&src1_image));
    VX_CALL(vxReleaseImage(&src0_image));

    ASSERT(dst1_image == 0);
    ASSERT(dst0_image == 0);
    ASSERT(src1_image == 0);
    ASSERT(src0_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");

    printPerformance(perf_node2, arg_->width*arg_->height, "N2");

    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TESTCASE_TESTS(tivxIntegral, testGraphProcessing)
