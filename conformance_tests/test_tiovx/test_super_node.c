/*

 * Copyright (c) 2012-2018 The Khronos Group Inc.
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

#ifdef BUILD_BAM

#include "test_tiovx.h"
#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx_ext_super_node.h>
#include <TI/tivx.h>
#include "shared_functions.h"

#define MAX_NODES 10

TESTCASE(tivxSuperNode, CT_VXContext, ct_setup_vx_context, 0)

static void referenceAbsDiff(CT_Image src0, CT_Image src1, CT_Image dst)
{
    uint32_t i, j;

    ASSERT(src0 && src1 && dst);
    ASSERT(src0->width == src1->width && src0->width == dst->width);
    ASSERT(src0->height == src1->height && src0->height == dst->height);
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
    ASSERT(src0->width == src1->width && src0->width == dst->width);
    ASSERT(src0->height == src1->height && src0->height == dst->height);
    ASSERT(src0->format == dst->format && src1->format == dst->format && dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = src0->data.y[i * src0->stride + j] & src1->data.y[i * src1->stride + j];
}

static void referenceOr(CT_Image src0, CT_Image src1, CT_Image dst)
{
    uint32_t i, j;

    ASSERT(src0 && src1 && dst);
    ASSERT(src0->width == src1->width && src0->width == dst->width);
    ASSERT(src0->height == src1->height && src0->height == dst->height);
    ASSERT(src0->format == dst->format && src1->format == dst->format && dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = src0->data.y[i * src0->stride + j] | src1->data.y[i * src1->stride + j];
}

static void referenceXor(CT_Image src0, CT_Image src1, CT_Image dst)
{
    uint32_t i, j;

    ASSERT(src0 && src1 && dst);
    ASSERT(src0->width == src1->width && src0->width == dst->width);
    ASSERT(src0->height == src1->height && src0->height == dst->height);
    ASSERT(src0->format == dst->format && src1->format == dst->format && dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = src0->data.y[i * src0->stride + j] ^ src1->data.y[i * src1->stride + j];
}

static void referenceAdd(CT_Image src0, CT_Image src1, CT_Image dst, enum vx_convert_policy_e policy)
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

static int16_t sobel_x_get(int32_t *values)
{
    int32_t res = (-values[0])     + (values[2]) +
                  (-values[3] * 2) + (values[5] * 2) +
                  (-values[6])     + (values[8]);
    return (int16_t)res;
}

static int16_t sobel_y_get(int32_t *values)
{
    int32_t res = (-values[0])     + (values[6]) +
                  (-values[1] * 2) + (values[7] * 2) +
                  (-values[2])     + (values[8]);
    return (int16_t)res;
}

static void sobel3x3_calculate(CT_Image src, uint32_t x, uint32_t y, int16_t *sobel_x, int16_t *sobel_y)
{
    int32_t values[9] = {
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 1)
    };
    *sobel_x = sobel_x_get(values);
    *sobel_y = sobel_y_get(values);
}

static void sobel3x3_calculate_replicate(CT_Image src, uint32_t x_, uint32_t y_, int16_t *sobel_x, int16_t *sobel_y)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 1)
    };
    *sobel_x = sobel_x_get(values);
    *sobel_y = sobel_y_get(values);
}

static void sobel3x3_calculate_constant(CT_Image src, uint32_t x_, uint32_t y_, vx_uint32 constant_value, int16_t *sobel_x, int16_t *sobel_y)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 1, constant_value)
    };
    *sobel_x = sobel_x_get(values);
    *sobel_y = sobel_y_get(values);
}

void referenceSobel(CT_Image src, vx_border_t border, CT_Image *p_dst_x, CT_Image *p_dst_y)
{
    CT_Image dst_x = NULL, dst_y = NULL;

    CT_ASSERT(src->format == VX_DF_IMAGE_U8);

    dst_x = ct_allocate_image(src->width, src->height, VX_DF_IMAGE_S16);
    dst_y = ct_allocate_image(src->width, src->height, VX_DF_IMAGE_S16);

    if (border.mode == VX_BORDER_UNDEFINED)
    {
        CT_FILL_IMAGE_16S(return, dst_x,
                if (x >= 1 && y >= 1 && x < src->width - 1 && y < src->height - 1)
                {
                    int16_t* dst_y_data = CT_IMAGE_DATA_PTR_16S(dst_y, x, y);
                    sobel3x3_calculate(src, x, y, dst_data, dst_y_data);
                });
    }
    else if (border.mode == VX_BORDER_REPLICATE)
    {
        CT_FILL_IMAGE_16S(return, dst_x,
                {
                    int16_t* dst_y_data = CT_IMAGE_DATA_PTR_16S(dst_y, x, y);
                    sobel3x3_calculate_replicate(src, x, y, dst_data, dst_y_data);
                });
    }
    else if (border.mode == VX_BORDER_CONSTANT)
    {
        vx_uint32 constant_value = border.constant_value.U32;
        CT_FILL_IMAGE_16S(return, dst_x,
                {
                    int16_t* dst_y_data = CT_IMAGE_DATA_PTR_16S(dst_y, x, y);
                    sobel3x3_calculate_constant(src, x, y, constant_value, dst_data, dst_y_data);
                });
    }
    else
    {
        ASSERT_(return, 0);
    }

    *p_dst_x = dst_x;
    *p_dst_y = dst_y;
}


/*
 * tivxCreateSuperNode Example
 * take the linear graph -->NOT-->NOT-->NOT-->NOT-->
 * this test will exhaustively test all possible create call for supernode (test_tivxCreateSuperNode)
 * Assumption: each supernode should have at least 2 nodes in it.
 * this test will do 2 calls for the test_tivxCreateSuperNode
 * each test_tivxCreateSuperNode will need a nodelist, which will create the supernodes

 * few examples of node lists each supernode can take, (NOT nodes are numbered from 0 to 3)
 * [0 1] [2 3] (each node list size here is 2) - this shows  2 valid supernodes
 * [0 1] [1 2] (each node list size here is 2) - 2nd nodelist can't create a supernode because of the common node 1
 * [0 1 2] [2 3] (first nodelist size is 3, second nodelist size is 2) - 2nd nodelist can't create a supernode because of the common node 2
 * etc
 * NODE_LIST_ARG is setting the parameters for the 2 supernodes
 * first 2 parameters assign the nodelist sizes
 * last 2 parameters assign the starting positions for the nodelists
 */


typedef struct {
    const char* name; // this is test_name
    uint32_t node_list1_size;
    uint32_t node_list2_size;
    uint32_t node_list1_start_index;
    uint32_t node_list2_start_index;
} node_list_arg;

#define NODE_LIST_ARG(nl1_size, nl2_size, nl1_i, nl2_i) ARG("[" #nl1_size " , " #nl2_size "] [" #nl1_i " , " #nl2_i "]", nl1_size, nl2_size, nl1_i, nl2_i)

#define SUPER_NODE_NODE_LIST_ARGS  \
    NODE_LIST_ARG(2, 2, 0, 0),   \
    NODE_LIST_ARG(2, 2, 0, 1),   \
    NODE_LIST_ARG(2, 2, 0, 2),   \
    NODE_LIST_ARG(2, 2, 1, 0),   \
    NODE_LIST_ARG(2, 2, 1, 1),   \
    NODE_LIST_ARG(2, 2, 1, 2),   \
    NODE_LIST_ARG(2, 2, 2, 0),   \
    NODE_LIST_ARG(2, 2, 2, 1),   \
    NODE_LIST_ARG(2, 2, 2, 2),   \
    NODE_LIST_ARG(2, 3, 0, 0),   \
    NODE_LIST_ARG(2, 3, 0, 1),   \
    NODE_LIST_ARG(2, 3, 1, 0),   \
    NODE_LIST_ARG(2, 3, 1, 1),   \
    NODE_LIST_ARG(2, 3, 2, 0),   \
    NODE_LIST_ARG(2, 3, 2, 1),   \
    NODE_LIST_ARG(2, 4, 0, 0),   \
    NODE_LIST_ARG(2, 4, 1, 0),   \
    NODE_LIST_ARG(2, 4, 2, 0),   \
    NODE_LIST_ARG(3, 2, 0, 0),   \
    NODE_LIST_ARG(3, 2, 0, 1),   \
    NODE_LIST_ARG(3, 2, 0, 2),   \
    NODE_LIST_ARG(3, 2, 1, 0),   \
    NODE_LIST_ARG(3, 2, 1, 1),   \
    NODE_LIST_ARG(3, 2, 1, 2),   \
    NODE_LIST_ARG(3, 3, 0, 0),   \
    NODE_LIST_ARG(3, 3, 0, 1),   \
    NODE_LIST_ARG(3, 3, 1, 0),   \
    NODE_LIST_ARG(3, 3, 1, 1),   \
    NODE_LIST_ARG(3, 4, 0, 0),   \
    NODE_LIST_ARG(3, 4, 1, 0),   \
    NODE_LIST_ARG(4, 2, 0, 0),   \
    NODE_LIST_ARG(4, 2, 0, 1),   \
    NODE_LIST_ARG(4, 2, 0, 2),   \
    NODE_LIST_ARG(4, 3, 0, 0),   \
    NODE_LIST_ARG(4, 3, 0, 1),   \
    NODE_LIST_ARG(4, 4, 0, 0),   \
    NODE_LIST_ARG(1, 1, 0, 0),   \
    NODE_LIST_ARG(1, 1, 0, 1),   \
    NODE_LIST_ARG(1, 1, 0, 2),   \
    NODE_LIST_ARG(1, 1, 0, 3),   \
    NODE_LIST_ARG(1, 3, 0, 1),   \
    NODE_LIST_ARG(1, 2, 0, 1),   \
    NODE_LIST_ARG(1, 1, 1, 2),   \
    NODE_LIST_ARG(1, 3, 1, 0),   \
    NODE_LIST_ARG(1, 2, 1, 1),   \

TEST_WITH_ARG(tivxSuperNode, test_tivxCreateSuperNode, node_list_arg, SUPER_NODE_NODE_LIST_ARGS)
{
    vx_context context = context_->vx_context_;
    vx_status super_node_1_status, super_node_2_status;
    vx_bool is_test_success;
    tivx_super_node super_node_1 = 0, super_node_2 = 0;
    vx_image src, dst, intermediate_1, intermediate_2, intermediate_3;
    CT_Image ref_src, ref_dst;
    vx_graph graph;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    vx_node node_list_1[4], node_list_2[4];
    int widthHardCoded = 640, heightHardCoded = 480;


    ASSERT_VX_OBJECT(src = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_1 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_2 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_3 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src, &CT()->seed_));


    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node1 = vxNotNode(graph, src, intermediate_1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, intermediate_1, intermediate_2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxNotNode(graph, intermediate_2, intermediate_3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxNotNode(graph, intermediate_3, dst), VX_TYPE_NODE);


    switch (arg_->node_list1_start_index)
    {
        case 0:
            node_list_1[0]   = node1;
            node_list_1[1]   = node2;
            node_list_1[2]   = node3;
            node_list_1[3]   = node4;
            break;
        case 1:
            node_list_1[0]   = node2;
            node_list_1[1]   = node3;
            node_list_1[2]   = node4;
            break;
        case 2:
            node_list_1[0]   = node3;
            node_list_1[1]   = node4;
            break;
        case 3:
            node_list_1[0]   = node4;
            break;
        default:
            // graph is only 4 nodes, max indexv = 3
            ASSERT_(return, 0);
            break;
    }


    switch (arg_->node_list2_start_index)
    {
        case 0:
            node_list_2[0]   = node1;
            node_list_2[1]   = node2;
            node_list_2[2]   = node3;
            node_list_2[3]   = node4;
            break;
        case 1:
            node_list_2[0]   = node2;
            node_list_2[1]   = node3;
            node_list_2[2]   = node4;
            break;
        case 2:
            node_list_2[0]   = node3;
            node_list_2[1]   = node4;
            break;
        case 3:
            node_list_2[0]   = node4;
            break;
        default:
            // graph is only 4 nodes, max indexv = 3
            ASSERT_(return, 0);
            break;
    }

    /* check if super node can be created */
    super_node_1 = tivxCreateSuperNode(graph, node_list_1, arg_->node_list1_size);
    super_node_2 = tivxCreateSuperNode(graph, node_list_2, arg_->node_list2_size);

    super_node_1_status = vxGetStatus((vx_reference)super_node_1);
    super_node_2_status = vxGetStatus((vx_reference)super_node_2);

    if (arg_->node_list1_start_index == arg_->node_list2_start_index) {
        if (super_node_2_status != VX_SUCCESS) {
            is_test_success = vx_true_e;
        }
        else {
            is_test_success = vx_false_e;
        }
    }
    else if (arg_->node_list2_start_index > arg_->node_list1_start_index) {
        if (arg_->node_list2_start_index - arg_->node_list1_start_index >= arg_->node_list1_size) {
            if (super_node_2_status == VX_SUCCESS) {
                is_test_success = vx_true_e;
            }
            else {
                is_test_success = vx_false_e;
            }
        }
        else {
            if (super_node_2_status == VX_SUCCESS) {
                is_test_success = vx_false_e;
            }
            else {
                is_test_success = vx_true_e;
            }
        }
    }
    else if (arg_->node_list1_start_index > arg_->node_list2_start_index) {
        if (arg_->node_list1_start_index - arg_->node_list2_start_index >= arg_->node_list2_size) {
            if (super_node_2_status == VX_SUCCESS) {
                is_test_success = vx_true_e;
            }
            else {
                is_test_success = vx_false_e;
            }
        }
        else {
            if (super_node_2_status == VX_SUCCESS) {
                is_test_success = vx_false_e;
            }
            else {
                is_test_success = vx_true_e;
            }
        }
    }

    // upto here we assumed that super node 1 is successfully created, if it isn't whole test fails
    is_test_success &= (super_node_1_status == VX_SUCCESS);

    ASSERT(is_test_success);

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&intermediate_1));
    VX_CALL(vxReleaseImage(&intermediate_2));
    VX_CALL(vxReleaseImage(&intermediate_3));
    VX_CALL(vxReleaseImage(&dst));
    if (super_node_1_status == VX_SUCCESS)
        VX_CALL(tivxReleaseSuperNode(&super_node_1));
    if (super_node_2_status == VX_SUCCESS)
        VX_CALL(tivxReleaseSuperNode(&super_node_2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));
}


/*
 * Super node edge compliance example 1
 * this test will check if there are unconnected nodes in a supernode
 * Assumption: each supernode should have at least 2 nodes in it.
 * This test will create 1 super node by using parameters given for the nodelist
 * Then it will run graph_verify to verify that test is successfull or not
 * Parameters
 * there are 5 parameters passed to the test
 * first 4 parameters assigns the non zero node indexes for the super node node list.
 * If an index is 0, it means there's no node. And the maximum number of nodes in this node list is 4.
 * 5th parameters says whether the supernode is valid or not (valid = 1, not valid = 0)
 *
 * _
 *  \
 *   OR(1)---NOT(2)\
 * _/               \
 *                   ADD(5)-->
 *                  /
 * --NOT(3)---NOT(4)
 */

typedef struct {
    const char* name; // this is test_name
    uint32_t node_list_index0;
    uint32_t node_list_index1;
    uint32_t node_list_index2;
    uint32_t node_list_index3;
    uint32_t is_valid;
} test_edge_vector_arg;

#define TEST_EDGE_VECTOR(nl_idx0, nl_idx1, nl_idx2, nl_idx3, is_valid) \
        ARG("Supernode Node List: [" #nl_idx0 ", " #nl_idx1 ", "#nl_idx2 ", " #nl_idx3 "]", \
                              nl_idx0, nl_idx1, nl_idx2, nl_idx3, is_valid)

#define TEST_EDGE_VECTOR_ARGS1  \
    TEST_EDGE_VECTOR(1, 3, 0, 0,   0),   \
    TEST_EDGE_VECTOR(1, 2, 0, 0,   1),   \
    TEST_EDGE_VECTOR(2, 4, 5, 0,   1),   \
    TEST_EDGE_VECTOR(2, 5, 0, 0,   1),   \
    TEST_EDGE_VECTOR(1, 5, 3, 0,   0),   \
    TEST_EDGE_VECTOR(1, 3, 4, 5,   0),   \
    TEST_EDGE_VECTOR(5, 1, 3, 2,   0),   \
    TEST_EDGE_VECTOR(1, 2, 5, 0,   1),   \
    TEST_EDGE_VECTOR(1, 2, 5, 4,   1),   \

TEST_WITH_ARG(tivxSuperNode, testSuperNodeEdgeCompliance1, test_edge_vector_arg, TEST_EDGE_VECTOR_ARGS1)
{
    int node_count = 4;
    vx_context context = context_->vx_context_;
    vx_status status;
    tivx_super_node super_node = 0;
    vx_image src1, src2, src3, dst, intermediate_1, intermediate_2, intermediate_3, intermediate_4;
    CT_Image ref_src1, ref_src2, ref_src3, ref_dst, ref_intermediate_1, ref_intermediate_2, ref_intermediate_3, ref_intermediate_4, vxdst;
    vx_graph graph;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0;
    vx_perf_t perf_super_node, perf_graph;
    vx_node node_list[MAX_NODES];
    int widthHardCoded = 1280, heightHardCoded = 720;
    int node_list_size = 0;
    int i = 0;
    vx_uint32 num_nodes = 0;
    int node_vector[] = {arg_->node_list_index0, arg_->node_list_index1, arg_->node_list_index2, arg_->node_list_index3};

    ASSERT_VX_OBJECT(intermediate_1 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_2 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_3 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_4 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ref_src1 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_src2 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_src3 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_intermediate_1 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_intermediate_2 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_intermediate_3 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_intermediate_4 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_dst = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));

    ASSERT_VX_OBJECT(src1 = ct_image_to_vx_image(ref_src1, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = ct_image_to_vx_image(ref_src2, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = ct_image_to_vx_image(ref_src3, context), VX_TYPE_IMAGE);


    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node1 = vxOrNode(graph, src1, src2, intermediate_1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, intermediate_1, intermediate_2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxNotNode(graph, src3, intermediate_3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxNotNode(graph, intermediate_3, intermediate_4), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node5 = vxAddNode(graph, intermediate_2, intermediate_4, VX_CONVERT_POLICY_SATURATE, dst), VX_TYPE_NODE);


    for (i = 0; i < node_count; ++i)
    {
        switch (node_vector[i])
        {
            case 1:
                node_list[i] = node1;
                node_list_size++;
                break;
            case 2:
                node_list[i] = node2;
                node_list_size++;
                break;
            case 3:
                node_list[i] = node3;
                node_list_size++;
                break;
            case 4:
                node_list[i] = node4;
                node_list_size++;
                break;
            case 5:
                node_list[i] = node5;
                node_list_size++;
                break;
            default:
                node_list[i] = 0;
                break;
        }
    }

    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_list_size), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_NUM_NODES, &num_nodes, sizeof(num_nodes)));
    ASSERT_EQ_INT(node_list_size, num_nodes);

    status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS) {
        if (arg_->is_valid) {

            vx_status node_status = VX_FAILURE;

            // run graph
#ifdef CT_EXECUTE_ASYNC
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

            VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_STATUS, &node_status, sizeof(vx_status)));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, node_status);
            tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node));
            vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

            ASSERT_NO_FAILURE(referenceOr(ref_src1, ref_src2, ref_intermediate_1));
            ASSERT_NO_FAILURE(referenceNot(ref_intermediate_1, ref_intermediate_2));
            ASSERT_NO_FAILURE(referenceNot(ref_src3, ref_intermediate_3));
            ASSERT_NO_FAILURE(referenceNot(ref_intermediate_3, ref_intermediate_4));
            ASSERT_NO_FAILURE(referenceAdd(ref_intermediate_2, ref_intermediate_4, ref_dst, VX_CONVERT_POLICY_SATURATE));

            ASSERT_NO_FAILURE(vxdst = ct_image_from_vx_image(dst));
            ASSERT_EQ_CTIMAGE(ref_dst, vxdst);

            printPerformance(perf_super_node, widthHardCoded*heightHardCoded, "SN");
            printPerformance(perf_graph, widthHardCoded*heightHardCoded, "G");
        }
        else {
            // verify should have failed according to the testcase, but it didn't
            ASSERT_(return, 0);
        }
    }
    else {
        if (arg_->is_valid) {
            // expected the verify to pass according to the testcase, but it didn't
            ASSERT_(return, 0);
        }
    }

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&intermediate_1));
    VX_CALL(vxReleaseImage(&intermediate_2));
    VX_CALL(vxReleaseImage(&intermediate_3));
    VX_CALL(vxReleaseImage(&intermediate_4));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseGraph(&graph));

}

/*
 * Same description as testSuperNodeEdgeCompliance1
 * Graph
 *
 * _                   CON_DPTH(4)-->
 *  \                 /
 *   OR(1)----SOBEL(3)
 * _/                 \
 *                     SUB(5)-->
 *                    /
 * --NOT(2)-----------
 */


#define TEST_EDGE_VECTOR_ARGS2  \
    TEST_EDGE_VECTOR(1, 3, 4, 0,   1),   \
    TEST_EDGE_VECTOR(1, 2, 4, 5,   0),   \
    TEST_EDGE_VECTOR(1, 2, 3, 4,   0),   \
    TEST_EDGE_VECTOR(1, 2, 3, 0,   0),   \
    TEST_EDGE_VECTOR(1, 2, 0, 0,   0),   \
    TEST_EDGE_VECTOR(4, 5, 0, 0,   0),   \
    TEST_EDGE_VECTOR(1, 5, 0, 0,   0),   \
    TEST_EDGE_VECTOR(1, 2, 5, 0,   0),   \

/*  Following fail since block size reduction of the 2 branches is not
 *  symetric TIOVX-690
    TEST_EDGE_VECTOR(1, 3, 5, 0,   1),   \
    TEST_EDGE_VECTOR(2, 3, 4, 5,   1),   \
*/

TEST_WITH_ARG(tivxSuperNode, testSuperNodeEdgeCompliance2, test_edge_vector_arg, TEST_EDGE_VECTOR_ARGS2)
{
    int node_count = 4;
    vx_context context = context_->vx_context_;
    vx_status status;
    tivx_super_node super_node = 0;
    vx_image src1, src2, src3, dst1, dst2, intermediate_1, intermediate_2, intermediate_3, intermediate_4;
    vx_scalar convdepth_shift = 0;
    uint32_t convdepth_shift_val=0;
    CT_Image ref_src1, ref_src2, ref_src3, ref_dst1, ref_dst2, ref_intermediate_1, ref_intermediate_2, ref_intermediate_3, ref_intermediate_4, vxdst1, vxdst2;
    vx_graph graph;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0;
    vx_perf_t perf_super_node, perf_graph;
    vx_node node_list[MAX_NODES];
    int widthHardCoded = 1280, heightHardCoded = 720;
    int node_list_size = 0;
    int node_vector[] = {arg_->node_list_index0, arg_->node_list_index1, arg_->node_list_index2, arg_->node_list_index3};
    int i = 0;
    vx_uint32 num_nodes = 0;
    vx_border_t border = { VX_BORDER_UNDEFINED, {{ 0 }} };

    ASSERT_VX_OBJECT(convdepth_shift = vxCreateScalar(context, VX_TYPE_INT32, &convdepth_shift_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(intermediate_1 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_2 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_3 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_4 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst1 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst2 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ref_src1 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_src2 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_src3 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_intermediate_1 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_intermediate_2 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_intermediate_3 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_S16));
    ASSERT_NO_FAILURE(ref_intermediate_4 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_S16));
    ASSERT_NO_FAILURE(ref_dst1 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_dst2 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_S16 ));

    ASSERT_VX_OBJECT(src1 = ct_image_to_vx_image(ref_src1, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = ct_image_to_vx_image(ref_src2, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = ct_image_to_vx_image(ref_src3, context), VX_TYPE_IMAGE);


    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node1 = vxOrNode(graph, src1, src2, intermediate_1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, src3, intermediate_2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxSobel3x3Node(graph, intermediate_1, intermediate_3, intermediate_4), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxConvertDepthNode(graph, intermediate_3, dst1, VX_CONVERT_POLICY_SATURATE, convdepth_shift), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node5 = vxSubtractNode(graph, intermediate_2, intermediate_4, VX_CONVERT_POLICY_SATURATE, dst2), VX_TYPE_NODE);

    for (i = 0; i < node_count; ++i)
    {
        switch (node_vector[i])
        {
            case 1:
                node_list[i] = node1;
                node_list_size++;
                break;
            case 2:
                node_list[i] = node2;
                node_list_size++;
                break;
            case 3:
                node_list[i] = node3;
                node_list_size++;
                break;
            case 4:
                node_list[i] = node4;
                node_list_size++;
                break;
            case 5:
                node_list[i] = node5;
                node_list_size++;
                break;
            default:
                node_list[i] = 0;
                break;
        }
    }

    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_list_size), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_NUM_NODES, &num_nodes, sizeof(num_nodes)));
    ASSERT_EQ_INT(node_list_size, num_nodes);

    status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS) {
        if (arg_->is_valid) {

            vx_status node_status = VX_FAILURE;

            // run graph
#ifdef CT_EXECUTE_ASYNC
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

            VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_STATUS, &node_status, sizeof(vx_status)));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, node_status);
            tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node));
            vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

            ASSERT_NO_FAILURE(referenceOr(ref_src1, ref_src2, ref_intermediate_1));
            ASSERT_NO_FAILURE(referenceSobel(ref_intermediate_1, border, &ref_intermediate_3, &ref_intermediate_4));
            ASSERT_NO_FAILURE(referenceNot(ref_src3, ref_intermediate_2));
            ASSERT_NO_FAILURE(referenceConvertDepth(ref_intermediate_3, ref_dst1, 0, VX_CONVERT_POLICY_SATURATE));
            ASSERT_NO_FAILURE(referenceSubtractSingle(ref_intermediate_2, ref_intermediate_4, ref_dst2, VX_CONVERT_POLICY_SATURATE));
            ASSERT_NO_FAILURE(vxdst1 = ct_image_from_vx_image(dst1));
            ASSERT_NO_FAILURE(vxdst2 = ct_image_from_vx_image(dst2));

            ASSERT_NO_FAILURE(
                if (border.mode == VX_BORDER_UNDEFINED)
                {
                    ct_adjust_roi(vxdst1,  1, 1, 1, 1);
                    ct_adjust_roi(ref_dst1, 1, 1, 1, 1);
                    ct_adjust_roi(vxdst2,  1, 1, 1, 1);
                    ct_adjust_roi(ref_dst2, 1, 1, 1, 1);
                }
            );

            ASSERT_EQ_CTIMAGE(ref_dst1, vxdst1);

#if 0
            ct_dump_image_info_ex(ref_intermediate_2, 8, 8);
            ct_dump_image_info_ex(ref_intermediate_4, 8, 8);
            ct_dump_image_info_ex(ref_dst2, 8, 8);
            ct_dump_image_info_ex(vxdst2, 8, 8);
#endif

            ASSERT_EQ_CTIMAGE(ref_dst2, vxdst2);

            printPerformance(perf_super_node, widthHardCoded*heightHardCoded, "SN");
            printPerformance(perf_graph, widthHardCoded*heightHardCoded, "G");
        }
        else {
            // verify should have failed according to the testcase, but it didn't
            ASSERT_(return, 0);
        }
    }
    else {
        if (arg_->is_valid) {
            // expected the verify to pass according to the testcase, but it didn't
            ASSERT_(return, 0);
        }
    }

    VX_CALL(vxReleaseScalar(&convdepth_shift));
    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&intermediate_1));
    VX_CALL(vxReleaseImage(&intermediate_2));
    VX_CALL(vxReleaseImage(&intermediate_3));
    VX_CALL(vxReleaseImage(&intermediate_4));
    VX_CALL(vxReleaseImage(&dst1));
    VX_CALL(vxReleaseImage(&dst2));
    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseGraph(&graph));

}


/*
 * Same description as testSuperNodeEdgeCompliance1
 * Graph
 *                ____
 *                    \
 * _                    XOR(3)
 *  \                 /
 *   OR(1)---NOT(2)---
 * _/                 \
 *                      NOT(4)-->
 *
 *
 */
#define TEST_EDGE_VECTOR_ARGS3  \
    TEST_EDGE_VECTOR(1, 2, 3, 4,   1),   \
    TEST_EDGE_VECTOR(1, 2, 3, 0,   1),   \
    TEST_EDGE_VECTOR(2, 3, 4, 0,   1),   \
    TEST_EDGE_VECTOR(1, 3, 0, 0,   0),   \
    TEST_EDGE_VECTOR(3, 4, 3, 0,   0),   \
    TEST_EDGE_VECTOR(2, 3, 0, 0,   1),   \

TEST_WITH_ARG(tivxSuperNode, testSuperNodeEdgeCompliance3, test_edge_vector_arg, TEST_EDGE_VECTOR_ARGS3)
{
    int node_count = 4;
    vx_context context = context_->vx_context_;
    vx_status status;
    tivx_super_node super_node = 0;
    vx_image src1, src2, src3, dst1, dst2, intermediate_1, intermediate_2;
    CT_Image ref_src1, ref_src2, ref_src3, ref_dst1, ref_dst2, ref_intermediate_1, ref_intermediate_2, vxdst1, vxdst2;
    vx_graph graph;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    vx_perf_t perf_super_node, perf_graph;
    vx_node node_list[MAX_NODES];
    int widthHardCoded = 1280, heightHardCoded = 720;
    int node_list_size = 0;
    int i = 0;
    vx_uint32 num_nodes = 0;
    int node_vector[] = {arg_->node_list_index0, arg_->node_list_index1, arg_->node_list_index2, arg_->node_list_index3};


    ASSERT_VX_OBJECT(intermediate_1 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_2 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst1 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst2 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ref_src1 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_src2 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_src3 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_intermediate_1 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_intermediate_2 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_dst1 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_dst2 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));

    ASSERT_VX_OBJECT(src1 = ct_image_to_vx_image(ref_src1, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = ct_image_to_vx_image(ref_src2, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = ct_image_to_vx_image(ref_src3, context), VX_TYPE_IMAGE);


    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node1 = vxOrNode(graph, src1, src2, intermediate_1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, intermediate_1, intermediate_2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxXorNode(graph, src3, intermediate_2, dst1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxNotNode(graph, intermediate_2, dst2), VX_TYPE_NODE);


    for (i = 0; i < node_count; ++i)
    {
        switch (node_vector[i])
        {
            case 1:
                node_list[i] = node1;
                node_list_size++;
                break;
            case 2:
                node_list[i] = node2;
                node_list_size++;
                break;
            case 3:
                node_list[i] = node3;
                node_list_size++;
                break;
            case 4:
                node_list[i] = node4;
                node_list_size++;
                break;
            default:
                node_list[i] = 0;
                break;
        }
    }

    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_list_size), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_NUM_NODES, &num_nodes, sizeof(num_nodes)));
    ASSERT_EQ_INT(node_list_size, num_nodes);

    status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS) {
        if (arg_->is_valid) {

            vx_status node_status = VX_FAILURE;

            // run graph
#ifdef CT_EXECUTE_ASYNC
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

            VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_STATUS, &node_status, sizeof(vx_status)));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, node_status);
            tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node));
            vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

            ASSERT_NO_FAILURE(referenceOr(ref_src1, ref_src2, ref_intermediate_1));
            ASSERT_NO_FAILURE(referenceNot(ref_intermediate_1, ref_intermediate_2));
            ASSERT_NO_FAILURE(referenceXor(ref_src3, ref_intermediate_2, ref_dst1));
            ASSERT_NO_FAILURE(referenceNot(ref_intermediate_2, ref_dst2));

            ASSERT_NO_FAILURE(vxdst1 = ct_image_from_vx_image(dst1));
            ASSERT_NO_FAILURE(vxdst2 = ct_image_from_vx_image(dst2));
            ASSERT_EQ_CTIMAGE(ref_dst1, vxdst1);
            ASSERT_EQ_CTIMAGE(ref_dst2, vxdst2);

            printPerformance(perf_super_node, widthHardCoded*heightHardCoded, "SN");
            printPerformance(perf_graph, widthHardCoded*heightHardCoded, "G");
        }
        else {
            // verify should have failed according to the testcase, but it didn't
            ASSERT_(return, 0);
        }
    }
    else {
        if (arg_->is_valid) {
            // expected the verify to pass according to the testcase, but it didn't
            ASSERT_(return, 0);
        }
    }

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&intermediate_1));
    VX_CALL(vxReleaseImage(&intermediate_2));
    VX_CALL(vxReleaseImage(&dst1));
    VX_CALL(vxReleaseImage(&dst2));
    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));

}

/*
 * Same description as testSuperNodeEdgeCompliance1
 * Graph
 *                __________________
 *               /                  \
 * _          __/                    \
 *  \        /  \                     \
 *   AND(1)--    OR(2)---XOR(3)----ABS_DIFF(4)---->
 * _/        \__/        /
 *                      /
 * _                   /
 *  \                 /
 *   OR(5)----------XOR(6)
 * _/_______________/
 *
 */

#define TEST_EDGE_VECTOR_ARGS4  \
    TEST_EDGE_VECTOR(1, 2, 3, 4,   1),   \
    TEST_EDGE_VECTOR(5, 6, 3, 0,   1),   \
    TEST_EDGE_VECTOR(2, 6, 3, 4,   1),   \
    TEST_EDGE_VECTOR(1, 2, 5, 6,   0),   \
    TEST_EDGE_VECTOR(2, 3, 6, 0,   1),   \
    TEST_EDGE_VECTOR(1, 3, 0, 0,   0),   \
    TEST_EDGE_VECTOR(5, 6, 4, 0,   0),   \
    TEST_EDGE_VECTOR(2, 3, 0, 0,   1),   \
    TEST_EDGE_VECTOR(1, 3, 4, 0,   0),   \
    TEST_EDGE_VECTOR(1, 6, 3, 4,   0),   \
    TEST_EDGE_VECTOR(1, 2, 4, 0,   0),   \
    TEST_EDGE_VECTOR(1, 4, 0, 0,   0),   \


TEST_WITH_ARG(tivxSuperNode, testSuperNodeEdgeCompliance4, test_edge_vector_arg, TEST_EDGE_VECTOR_ARGS4)
{
    int node_count = 4;
    vx_context context = context_->vx_context_;
    vx_status status;
    tivx_super_node super_node = 0;
    vx_image src1, src2, src3, src4, dst, intermediate_1, intermediate_2, intermediate_3, intermediate_4, intermediate_5;
    CT_Image ref_src1, ref_src2, ref_src3, ref_src4, ref_dst, ref_intermediate_1, ref_intermediate_2, ref_intermediate_3, ref_intermediate_4, ref_intermediate_5, vxdst;
    vx_graph graph;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0, node6 = 0;
    vx_perf_t perf_super_node, perf_graph;
    vx_node node_list[MAX_NODES];
    int widthHardCoded = 1280, heightHardCoded = 720;
    int node_list_size = 0;
    int i = 0;
    vx_uint32 num_nodes = 0;
    int node_vector[] = {arg_->node_list_index0, arg_->node_list_index1, arg_->node_list_index2, arg_->node_list_index3};


    ASSERT_VX_OBJECT(intermediate_1 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_2 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_3 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_4 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_5 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ref_src1 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_src2 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_src3 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_src4 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_intermediate_1 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_intermediate_2 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_intermediate_3 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_intermediate_4 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_intermediate_5 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_dst = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));

    ASSERT_VX_OBJECT(src1 = ct_image_to_vx_image(ref_src1, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = ct_image_to_vx_image(ref_src2, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3 = ct_image_to_vx_image(ref_src3, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src4 = ct_image_to_vx_image(ref_src4, context), VX_TYPE_IMAGE);


    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node1 = vxAndNode(graph, src1, src2, intermediate_1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxOrNode(graph, intermediate_1, intermediate_1, intermediate_2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node5 = vxOrNode(graph, src3, src4, intermediate_4), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node6 = vxXorNode(graph, intermediate_4, src4, intermediate_5), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxXorNode(graph, intermediate_2, intermediate_5, intermediate_3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxAbsDiffNode(graph, intermediate_1, intermediate_3, dst), VX_TYPE_NODE);



    for (i = 0; i < node_count; ++i)
    {
        switch (node_vector[i])
        {
            case 1:
                node_list[i] = node1;
                node_list_size++;
                break;
            case 2:
                node_list[i] = node2;
                node_list_size++;
                break;
            case 3:
                node_list[i] = node3;
                node_list_size++;
                break;
            case 4:
                node_list[i] = node4;
                node_list_size++;
                break;
            case 5:
                node_list[i] = node5;
                node_list_size++;
                break;
            case 6:
                node_list[i] = node6;
                node_list_size++;
                break;
            default:
                node_list[i] = 0;
                break;
        }
    }

    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_list_size), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_NUM_NODES, &num_nodes, sizeof(num_nodes)));
    ASSERT_EQ_INT(node_list_size, num_nodes);

    status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS) {
        if (arg_->is_valid) {

            vx_status node_status = VX_FAILURE;

            // run graph
#ifdef CT_EXECUTE_ASYNC
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

            VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_STATUS, &node_status, sizeof(vx_status)));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, node_status);
            VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
            VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

            ASSERT_NO_FAILURE(referenceAnd(ref_src1, ref_src2, ref_intermediate_1));
            ASSERT_NO_FAILURE(referenceOr(ref_intermediate_1, ref_intermediate_1, ref_intermediate_2));
            ASSERT_NO_FAILURE(referenceOr(ref_src3, ref_src4, ref_intermediate_4));
            ASSERT_NO_FAILURE(referenceXor(ref_intermediate_4, ref_src4, ref_intermediate_5));
            ASSERT_NO_FAILURE(referenceXor(ref_intermediate_2, ref_intermediate_5, ref_intermediate_3));
            ASSERT_NO_FAILURE(referenceAbsDiff(ref_intermediate_1, ref_intermediate_3, ref_dst));

            ASSERT_NO_FAILURE(vxdst = ct_image_from_vx_image(dst));
            ASSERT_EQ_CTIMAGE(ref_dst, vxdst);

            printPerformance(perf_super_node, widthHardCoded*heightHardCoded, "SN");
            printPerformance(perf_graph, widthHardCoded*heightHardCoded, "G");
        }
        else {
            // verify should have failed according to the testcase, but it didn't
            ASSERT_(return, 0);
        }
    }
    else {
        if (arg_->is_valid) {
            // expected the verify to pass according to the testcase, but it didn't
            ASSERT_(return, 0);
        }
    }

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&src3));
    VX_CALL(vxReleaseImage(&src4));
    VX_CALL(vxReleaseImage(&intermediate_1));
    VX_CALL(vxReleaseImage(&intermediate_2));
    VX_CALL(vxReleaseImage(&intermediate_3));
    VX_CALL(vxReleaseImage(&intermediate_4));
    VX_CALL(vxReleaseImage(&intermediate_5));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseNode(&node6));
    VX_CALL(vxReleaseGraph(&graph));

}


/*
 * Following test will set the supernode target to another target than the default one and check if it was correctly set
 * No Parameters
 * Supernode1 (OR(1),NOT(2))
 * Supernode2 (XOR(3),NOT(4))
 * _
 *  \
 *   OR(1)---NOT(2)\
 * _/               \
 * _                 ADD(5)-->
 *  \               /
 *   XOR(3)--NOT(4)
 * _/
 */

TEST(tivxSuperNode, testSuperNodeTargetConstraint1)
{
    int node_count = 2;
    vx_context context = context_->vx_context_;
    vx_status status;
    tivx_super_node super_node_1 = 0, super_node_2 = 0;
    vx_image src1, src2, dst, intermediate_1, intermediate_2, intermediate_3, intermediate_4;
    CT_Image ref_src1, ref_src2, ref_dst, ref_intermediate_1, ref_intermediate_2, ref_intermediate_3, ref_intermediate_4, vxdst;
    vx_graph graph;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0;
    vx_perf_t perf_super_node_1, perf_super_node_2, perf_graph;
    vx_node node_list[MAX_NODES];
    int widthHardCoded = 360, heightHardCoded = 240;
    char supernodeTarget1[TIVX_TARGET_MAX_NAME], supernodeTarget2[TIVX_TARGET_MAX_NAME];
    char nodeTarget1[TIVX_TARGET_MAX_NAME], nodeTarget2[TIVX_TARGET_MAX_NAME], nodeTarget3[TIVX_TARGET_MAX_NAME], nodeTarget4[TIVX_TARGET_MAX_NAME];
    vx_status node_status = VX_FAILURE;

    ASSERT_VX_OBJECT(intermediate_1 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_2 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_3 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_4 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ref_src1 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_src2 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_intermediate_1 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_intermediate_2 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_intermediate_3 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_intermediate_4 = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(ref_dst = ct_allocate_image(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8));

    ASSERT_VX_OBJECT(src1 = ct_image_to_vx_image(ref_src1, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = ct_image_to_vx_image(ref_src2, context), VX_TYPE_IMAGE);


    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node1 = vxOrNode(graph, src1, src2, intermediate_1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, intermediate_1, intermediate_2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxXorNode(graph, src1, src2, intermediate_3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxNotNode(graph, intermediate_3, intermediate_4), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node5 = vxAddNode(graph, intermediate_2, intermediate_4, VX_CONVERT_POLICY_WRAP, dst), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(node_list[0] = node1);
    ASSERT_NO_FAILURE(node_list[1] = node2);
    ASSERT_VX_OBJECT(super_node_1 = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    VX_CALL(tivxSetSuperNodeTarget(super_node_1, VX_TARGET_STRING, TIVX_TARGET_DSP1));
    ASSERT_NO_FAILURE(node_list[0] = node3);
    ASSERT_NO_FAILURE(node_list[1] = node4);
    ASSERT_VX_OBJECT(super_node_2 = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    VX_CALL(tivxSetSuperNodeTarget(super_node_2, VX_TARGET_STRING, TIVX_TARGET_DSP2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node_1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node_2));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    VX_CALL(tivxQuerySuperNode(super_node_1, TIVX_SUPER_NODE_TARGET_STRING, &supernodeTarget1, sizeof(supernodeTarget1)));
    VX_CALL(tivxQuerySuperNode(super_node_2, TIVX_SUPER_NODE_TARGET_STRING, &supernodeTarget2, sizeof(supernodeTarget2)));
    VX_CALL(vxQueryNode(node1, TIVX_NODE_TARGET_STRING, &nodeTarget1, sizeof(nodeTarget1)));
    VX_CALL(vxQueryNode(node2, TIVX_NODE_TARGET_STRING, &nodeTarget2, sizeof(nodeTarget2)));
    VX_CALL(vxQueryNode(node3, TIVX_NODE_TARGET_STRING, &nodeTarget3, sizeof(nodeTarget3)));
    VX_CALL(vxQueryNode(node4, TIVX_NODE_TARGET_STRING, &nodeTarget4, sizeof(nodeTarget4)));

    ASSERT((strncmp(supernodeTarget1, TIVX_TARGET_DSP1, TIVX_TARGET_MAX_NAME) == 0) &&
           (strncmp(supernodeTarget2, TIVX_TARGET_DSP2, TIVX_TARGET_MAX_NAME) == 0) &&
           (strncmp(nodeTarget1, TIVX_TARGET_DSP1, TIVX_TARGET_MAX_NAME) == 0) &&
           (strncmp(nodeTarget2, TIVX_TARGET_DSP1, TIVX_TARGET_MAX_NAME) == 0) &&
           (strncmp(nodeTarget3, TIVX_TARGET_DSP2, TIVX_TARGET_MAX_NAME) == 0) &&
           (strncmp(nodeTarget4, TIVX_TARGET_DSP2, TIVX_TARGET_MAX_NAME) == 0));

            // run graph
#ifdef CT_EXECUTE_ASYNC
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
#else
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
#endif

    VX_CALL(tivxQuerySuperNode(super_node_1, TIVX_SUPER_NODE_STATUS, &node_status, sizeof(vx_status)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, node_status);
    VX_CALL(tivxQuerySuperNode(super_node_2, TIVX_SUPER_NODE_STATUS, &node_status, sizeof(vx_status)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, node_status);
    VX_CALL(tivxQuerySuperNode(super_node_1, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node_1, sizeof(perf_super_node_1)));
    VX_CALL(tivxQuerySuperNode(super_node_2, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node_2, sizeof(perf_super_node_2)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    ASSERT_NO_FAILURE(referenceOr(ref_src1, ref_src2, ref_intermediate_1));
    ASSERT_NO_FAILURE(referenceNot(ref_intermediate_1, ref_intermediate_2));
    ASSERT_NO_FAILURE(referenceXor(ref_src1, ref_src2, ref_intermediate_3));
    ASSERT_NO_FAILURE(referenceNot(ref_intermediate_3, ref_intermediate_4));
    ASSERT_NO_FAILURE(referenceAdd(ref_intermediate_2, ref_intermediate_4, ref_dst, VX_CONVERT_POLICY_WRAP));

    ASSERT_NO_FAILURE(vxdst = ct_image_from_vx_image(dst));
    ASSERT_EQ_CTIMAGE(ref_dst, vxdst);

    printPerformance(perf_super_node_1, widthHardCoded*heightHardCoded, "SN1");
    printPerformance(perf_super_node_2, widthHardCoded*heightHardCoded, "SN2");
    printPerformance(perf_graph, widthHardCoded*heightHardCoded, "G");


    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&intermediate_1));
    VX_CALL(vxReleaseImage(&intermediate_2));
    VX_CALL(vxReleaseImage(&intermediate_3));
    VX_CALL(vxReleaseImage(&intermediate_4));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(tivxReleaseSuperNode(&super_node_1));
    VX_CALL(tivxReleaseSuperNode(&super_node_2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseGraph(&graph));

}

/*
 * This is a negative test
 * This test will select the supernode based on the first 3 input parameters
 * Last 3 parameters will set if the supernode's nodes belong to DSP1 (0) or DSP2 (1)
 * All the tests should fail vxGraphVerify()
 * _
 *  \
 *   OR(1)---NOT(2)\
 * _/               \
 * _                 ADD(5)-->
 *  \               /
 *   OR(3)---NOT(4)
 * _/
 */

typedef struct {
    const char* name; // this is test_name
    uint32_t node_list_index0;
    uint32_t node_list_index1;
    uint32_t node_list_index2;
    uint32_t target0;
    uint32_t target1;
    uint32_t target2;
} test_target_vector_arg;

#define TEST_TARGET_VECTOR(nl_idx0, nl_idx1, nl_idx2, trgt0, trgt1, trgt2) \
        ARG("Supernode Node List: [" #nl_idx0 ", " #nl_idx1 ", "#nl_idx2 "], Target Identifier: [" #trgt0 ", " #trgt1 ", " #trgt2 "]", \
                              nl_idx0, nl_idx1, nl_idx2, trgt0, trgt1, trgt2)

#define TEST_TARGET_VECTOR_ARGS  \
    TEST_TARGET_VECTOR(1, 2, 5,   0, 1, 1),   \
    TEST_TARGET_VECTOR(3, 4, 0,   1, 0, 0),   \
    TEST_TARGET_VECTOR(2, 4, 5,   1, 0, 1),   \

TEST_WITH_ARG(tivxSuperNode, testSuperNodeTargetConstraint2, test_target_vector_arg, TEST_TARGET_VECTOR_ARGS)
{
    int node_count = 3;
    vx_context context = context_->vx_context_;
    vx_status status;
    tivx_super_node super_node = 0;
    vx_image src1, src2, dst, intermediate_1, intermediate_2, intermediate_3, intermediate_4;
    CT_Image ref_src1, ref_src2;
    vx_graph graph;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0;
    vx_perf_t perf_super_node, perf_graph;
    vx_node node_list[MAX_NODES];
    int widthHardCoded = 800, heightHardCoded = 600;
    int node_list_size = 0;
    int i = 0;
    int node_vector[] = {arg_->node_list_index0, arg_->node_list_index1, arg_->node_list_index2};
    int target_vector[] = {arg_->target0, arg_->target1, arg_->target2};


    ASSERT_VX_OBJECT(intermediate_1 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_2 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_3 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_4 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ref_src1 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_src2 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    ASSERT_VX_OBJECT(src1 = ct_image_to_vx_image(ref_src1, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = ct_image_to_vx_image(ref_src2, context), VX_TYPE_IMAGE);


    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node1 = vxOrNode(graph, src1, src2, intermediate_1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, intermediate_1, intermediate_2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxOrNode(graph, src1, src2, intermediate_3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxNotNode(graph, intermediate_3, intermediate_4), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node5 = vxAddNode(graph, intermediate_2, intermediate_4, VX_CONVERT_POLICY_SATURATE, dst), VX_TYPE_NODE);


    for (i = 0; i < node_count; ++i)
    {
        switch (node_vector[i])
        {
            case 1:
                node_list[i] = node1;
                VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, !target_vector[i] ? TIVX_TARGET_DSP1 : TIVX_TARGET_DSP2));
                node_list_size++;
                break;
            case 2:
                node_list[i] = node2;
                VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, !target_vector[i] ? TIVX_TARGET_DSP1 : TIVX_TARGET_DSP2));
                node_list_size++;
                break;
            case 3:
                node_list[i] = node3;
                VX_CALL(vxSetNodeTarget(node3, VX_TARGET_STRING, !target_vector[i] ? TIVX_TARGET_DSP1 : TIVX_TARGET_DSP2));
                node_list_size++;
                break;
            case 4:
                node_list[i] = node4;
                VX_CALL(vxSetNodeTarget(node4, VX_TARGET_STRING, !target_vector[i] ? TIVX_TARGET_DSP1 : TIVX_TARGET_DSP2));
                node_list_size++;
                break;
            case 5:
                node_list[i] = node5;
                VX_CALL(vxSetNodeTarget(node5, VX_TARGET_STRING, !target_vector[i] ? TIVX_TARGET_DSP1 : TIVX_TARGET_DSP2));
                node_list_size++;
                break;
            default:
                node_list[i] = 0;
                break;
        }
    }

    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_list_size), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    status = vxVerifyGraph(graph);


    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&intermediate_1));
    VX_CALL(vxReleaseImage(&intermediate_2));
    VX_CALL(vxReleaseImage(&intermediate_3));
    VX_CALL(vxReleaseImage(&intermediate_4));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseGraph(&graph));

}

TEST(tivxSuperNode, testSuperNodeNegativeQuery)
{
    int node_count = 3;
    vx_context context = context_->vx_context_;
    vx_status status;
    tivx_super_node super_node = 0;
    vx_image src1, src2, dst, intermediate_1, intermediate_2, intermediate_3, intermediate_4;
    CT_Image ref_src1, ref_src2;
    vx_graph graph;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0;
    vx_perf_t perf_super_node, perf_graph;
    vx_node node_list[MAX_NODES];
    int widthHardCoded = 800, heightHardCoded = 600;
    int node_list_size = 0;
    int i = 0;

    ASSERT_VX_OBJECT(intermediate_1 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_2 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_3 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_4 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ref_src1 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));
    ASSERT_NO_FAILURE(ref_src2 = ct_allocate_ct_image_random(widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    ASSERT_VX_OBJECT(src1 = ct_image_to_vx_image(ref_src1, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2 = ct_image_to_vx_image(ref_src2, context), VX_TYPE_IMAGE);


    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node1 = vxOrNode(graph, src1, src2, intermediate_1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, intermediate_1, intermediate_2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxOrNode(graph, src1, src2, intermediate_3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxNotNode(graph, intermediate_3, intermediate_4), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node5 = vxAddNode(graph, intermediate_2, intermediate_4, VX_CONVERT_POLICY_SATURATE, dst), VX_TYPE_NODE);

    node_list[0] = node1;
    node_list[1] = node2;
    node_list[2] = node3;
    node_list[3] = node4;
    node_list[4] = node5;
    node_list_size = 5;

    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_list_size), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(tivxSetSuperNodeTarget(super_node, VX_TARGET_STRING, TIVX_TARGET_DSP1));

    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_super_node, sizeof(vx_perf_t)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryNode(node1, VX_NODE_STATUS, &status, sizeof(vx_status)));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&intermediate_1));
    VX_CALL(vxReleaseImage(&intermediate_2));
    VX_CALL(vxReleaseImage(&intermediate_3));
    VX_CALL(vxReleaseImage(&intermediate_4));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseGraph(&graph));

}

TEST(tivxSuperNode, testSuperNodeTimeStamp)
{
    vx_context context = context_->vx_context_;
    vx_status super_node_status;
    vx_bool is_test_success;
    tivx_super_node super_node = 0;
    vx_image src, dst, intermediate_1, intermediate_2, intermediate_3;
    CT_Image ref_src, ref_dst;
    vx_graph graph;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    vx_node node_list[4];
    int widthHardCoded = 640, heightHardCoded = 480;
    vx_uint64 timestamp = 0, set_timestamp = 10;

    ASSERT_VX_OBJECT(src = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_1 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_2 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(intermediate_3 = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, widthHardCoded, heightHardCoded, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(src, &CT()->seed_));


    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node1 = vxNotNode(graph, src, intermediate_1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, intermediate_1, intermediate_2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxNotNode(graph, intermediate_2, intermediate_3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxNotNode(graph, intermediate_3, dst), VX_TYPE_NODE);

    node_list[0]   = node1;
    node_list[1]   = node2;
    node_list[2]   = node3;
    node_list[3]   = node4;

    VX_CALL(tivxSetReferenceAttribute((vx_reference)src, TIVX_REFERENCE_TIMESTAMP, &set_timestamp, sizeof(set_timestamp)));

    /* check if super node can be created */
    super_node = tivxCreateSuperNode(graph, node_list, 4);

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxQueryReference((vx_reference)dst, TIVX_REFERENCE_TIMESTAMP, &timestamp, sizeof(timestamp)));

    printf("timestamp = %llu\n", timestamp);

    ASSERT(timestamp==set_timestamp);

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&intermediate_1));
    VX_CALL(vxReleaseImage(&intermediate_2));
    VX_CALL(vxReleaseImage(&intermediate_3));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));
}

/* On PC, targets get mapped to same target, so this test will fail */
#ifdef PLATFORM_PC
#define testSuperNodeTargetConstraint1 DISABLED_testSuperNodeTargetConstraint1
#else
#define testSuperNodeTargetConstraint1 testSuperNodeTargetConstraint1
#endif

TESTCASE_TESTS(tivxSuperNode,
        test_tivxCreateSuperNode,
        testSuperNodeEdgeCompliance1,
        testSuperNodeEdgeCompliance2,
        testSuperNodeEdgeCompliance3,
        testSuperNodeEdgeCompliance4,
        testSuperNodeTargetConstraint1,
        testSuperNodeTargetConstraint2,
        testSuperNodeNegativeQuery,
        testSuperNodeTimeStamp
        )

#endif
