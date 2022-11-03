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

#define MAX_NODES 10

TESTCASE(tivxChannelCombine, CT_VXContext, ct_setup_vx_context, 0)

static CT_Image channel_combine_image_generate_random(int width, int height, vx_df_image format)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, format, &CT()->seed_, 0, 256));

    return image;
}


static void channel_combine_fill_chanel(CT_Image src, vx_enum channel, CT_Image dst)
{
    uint8_t *dst_base = NULL;
    int x, y;

    int plane, component;

    int x_subsampling = ct_image_get_channel_subsampling_x(dst, channel);
    int y_subsampling = ct_image_get_channel_subsampling_y(dst, channel);

    int xstep = ct_image_get_channel_step_x(dst, channel);
    int ystep = ct_image_get_channel_step_y(dst, channel);

    int src_width = dst->width / x_subsampling;
    int src_height = dst->height / y_subsampling;

    // Check that src was subsampled (by spec)
    ASSERT_EQ_INT(src_width, src->width);
    ASSERT_EQ_INT(src_height, src->height);

    ASSERT_NO_FAILURE(plane = ct_image_get_channel_plane(dst, channel));
    ASSERT_NO_FAILURE(component = ct_image_get_channel_component(dst, channel));

    ASSERT(dst_base = ct_image_get_plane_base(dst, plane));

    for (y = 0; y < src_height; y++)
    {
        for (x = 0; x < src_width; x++)
        {
            uint8_t *src_data = CT_IMAGE_DATA_PTR_8U(src, x, y);
            uint8_t *dst_data = dst_base + (x * xstep) + (y * ystep);
            dst_data[component] = *src_data;
        }
    }

    return;
}


static CT_Image channel_combine_create_reference_image(CT_Image src1, CT_Image src2, CT_Image src3, CT_Image src4, vx_df_image format)
{
    CT_Image dst = NULL;

    ASSERT_(return NULL, src1);
    ASSERT_NO_FAILURE_(return NULL, dst = ct_allocate_image(src1->width, src1->height, format));

    switch (format)
    {
        case VX_DF_IMAGE_RGB:
        case VX_DF_IMAGE_RGBX:
            ASSERT_(return NULL, src1);
            ASSERT_(return NULL, src2);
            ASSERT_(return NULL, src3);
            if (format == VX_DF_IMAGE_RGB)
                ASSERT_(return NULL, src4 == NULL);
            ASSERT_NO_FAILURE_(return NULL, channel_combine_fill_chanel(src1, VX_CHANNEL_R, dst));
            ASSERT_NO_FAILURE_(return NULL, channel_combine_fill_chanel(src2, VX_CHANNEL_G, dst));
            ASSERT_NO_FAILURE_(return NULL, channel_combine_fill_chanel(src3, VX_CHANNEL_B, dst));
            if (format == VX_DF_IMAGE_RGBX)
                ASSERT_NO_FAILURE_(return NULL, channel_combine_fill_chanel(src4, VX_CHANNEL_A, dst));
            return dst;
        case VX_DF_IMAGE_NV12:
        case VX_DF_IMAGE_NV21:
        case VX_DF_IMAGE_UYVY:
        case VX_DF_IMAGE_YUYV:
        case VX_DF_IMAGE_IYUV:
        case VX_DF_IMAGE_YUV4:
            ASSERT_(return NULL, src1);
            ASSERT_(return NULL, src2);
            ASSERT_(return NULL, src3);
            ASSERT_(return NULL, src4 == NULL);
            ASSERT_NO_FAILURE_(return NULL, channel_combine_fill_chanel(src1, VX_CHANNEL_Y, dst));
            ASSERT_NO_FAILURE_(return NULL, channel_combine_fill_chanel(src2, VX_CHANNEL_U, dst));
            ASSERT_NO_FAILURE_(return NULL, channel_combine_fill_chanel(src3, VX_CHANNEL_V, dst));
            return dst;
    }

    CT_FAIL_(return NULL, "Not supported");
}

static void channel_combine_check(CT_Image src1, CT_Image src2, CT_Image src3, CT_Image src4, CT_Image dst)
{
    CT_Image dst_ref = NULL;

    ASSERT_NO_FAILURE(dst_ref = channel_combine_create_reference_image(src1, src2, src3, src4, dst->format));

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
}

typedef struct {
    const char* testName;
    vx_df_image dst_format;
    int width, height;
} Arg;

#define ADD_CASE(testArgName, nextmacro, format, ...) \
    CT_EXPAND(nextmacro(testArgName "/" #format, __VA_ARGS__, format))

#define ADD_CASES(testArgName, nextmacro, ...) \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_RGB, __VA_ARGS__),  \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_RGBX, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_NV12, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_NV21, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_UYVY, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_YUYV, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_YUV4, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_IYUV, __VA_ARGS__)


#define ADD_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=18x18", __VA_ARGS__, 18, 18)), \
    CT_EXPAND(nextmacro(testArgName "/sz=644x258", __VA_ARGS__, 644, 258)), \
    CT_EXPAND(nextmacro(testArgName "/sz=1600x1200", __VA_ARGS__, 1600, 1200))

#define ADD_SUPERNODE_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=18x18", __VA_ARGS__, 18, 18)), \
    CT_EXPAND(nextmacro(testArgName "/sz=644x258", __VA_ARGS__, 644, 258))

#define ChannelCombine_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_CASES, ADD_SIZE, ARG)

#define ChannelCombineSupernode_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_CASES, ADD_SUPERNODE_SIZE, ARG)

/* Also tests optional parameter */
TEST_WITH_ARG(tivxChannelCombine, testGraphProcessing, Arg,
    ChannelCombine_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src1_image[4] = {0, 0, 0, 0}, src2_image[4] = {0, 0, 0, 0};
    vx_image dst1_image = 0, dst2_image = 0;
    vx_graph graph1 = 0, graph2 = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph1, perf_graph2;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    int channels = 0, i;
    CT_Image src1[4] = {NULL, NULL, NULL, NULL}, src2[4] = {NULL, NULL, NULL, NULL};
    CT_Image dst1 = NULL, dst_dummy = NULL, dst2 = NULL;
    vx_enum channel_ref;

    ASSERT_NO_FAILURE(dst_dummy = ct_allocate_image(4, 4, arg_->dst_format));

    ASSERT_NO_FAILURE(channels = ct_get_num_channels(arg_->dst_format));
    channel_ref = (arg_->dst_format==VX_DF_IMAGE_RGB)||(arg_->dst_format==VX_DF_IMAGE_RGBX)?VX_CHANNEL_R:VX_CHANNEL_Y;
    for (i = 0; i < channels; i++)
    {
        int w = arg_->width / ct_image_get_channel_subsampling_x(dst_dummy, channel_ref + i);
        int h = arg_->height / ct_image_get_channel_subsampling_y(dst_dummy, channel_ref + i);
        ASSERT_NO_FAILURE(src1[i] = channel_combine_image_generate_random(w, h, VX_DF_IMAGE_U8));
        ASSERT_NO_FAILURE(src2[i] = channel_combine_image_generate_random(w, h, VX_DF_IMAGE_U8));
        ASSERT_VX_OBJECT(src1_image[i] = ct_image_to_vx_image(src1[i], context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src2_image[i] = ct_image_to_vx_image(src2[i], context), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(dst1_image = vxCreateImage(context, arg_->width, arg_->height, arg_->dst_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst2_image = vxCreateImage(context, arg_->width, arg_->height, arg_->dst_format), VX_TYPE_IMAGE);

    graph1 = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph1, VX_TYPE_GRAPH);

    graph2 = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph2, VX_TYPE_GRAPH);

    node1 = vxChannelCombineNode(graph1, src1_image[0], src1_image[1], src1_image[2], src1_image[3], dst1_image);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    node2 = vxChannelCombineNode(graph2, src2_image[0], src2_image[1], src2_image[2], src2_image[3], dst2_image);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph1));
    VX_CALL(vxProcessGraph(graph1));

    VX_CALL(vxVerifyGraph(graph2));
    VX_CALL(vxProcessGraph(graph2));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));
    vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));

    ASSERT_NO_FAILURE(dst1 = ct_image_from_vx_image(dst1_image));
    ASSERT_NO_FAILURE(dst2 = ct_image_from_vx_image(dst2_image));

    ASSERT_NO_FAILURE(channel_combine_check(src1[0], src1[1], src1[2], src1[3], dst1));
    ASSERT_NO_FAILURE(channel_combine_check(src2[0], src2[1], src2[2], src2[3], dst2));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph1));
    VX_CALL(vxReleaseGraph(&graph2));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph1 == 0);
    ASSERT(graph2 == 0);

    VX_CALL(vxReleaseImage(&dst1_image));
    VX_CALL(vxReleaseImage(&dst2_image));
    ASSERT(dst1_image == 0);
    ASSERT(dst2_image == 0);

    for (i = 0; i < channels; i++)
    {
        VX_CALL(vxReleaseImage(&src1_image[i]));
        VX_CALL(vxReleaseImage(&src2_image[i]));
        ASSERT(src1_image[i] == 0);
        ASSERT(src2_image[i] == 0);
    }

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_graph1, arg_->width*arg_->height, "G1");
    printPerformance(perf_graph2, arg_->width*arg_->height, "G2");
}

TEST_WITH_ARG(tivxChannelCombine, testChannelCombineSingleSupernode, Arg,
    ChannelCombineSupernode_PARAMETERS
)
{
    int node_count = 1;
    vx_context context = context_->vx_context_;
    vx_image src_image[4] = {0, 0, 0, 0};
    vx_image dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_node, perf_graph;
    vx_perf_t perf_super_node;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    int channels = 0, i;
    CT_Image src[4] = {NULL, NULL, NULL, NULL};
    CT_Image dst = NULL, dst_dummy = NULL;
    vx_enum channel_ref;

    ASSERT_NO_FAILURE(dst_dummy = ct_allocate_image(4, 4, arg_->dst_format));

    ASSERT_NO_FAILURE(channels = ct_get_num_channels(arg_->dst_format));
    channel_ref = (arg_->dst_format==VX_DF_IMAGE_RGB)||(arg_->dst_format==VX_DF_IMAGE_RGBX)?VX_CHANNEL_R:VX_CHANNEL_Y;
    for (i = 0; i < channels; i++)
    {
        int w = arg_->width / ct_image_get_channel_subsampling_x(dst_dummy, channel_ref + i);
        int h = arg_->height / ct_image_get_channel_subsampling_y(dst_dummy, channel_ref + i);
        ASSERT_NO_FAILURE(src[i] = channel_combine_image_generate_random(w, h, VX_DF_IMAGE_U8));
        ASSERT_VX_OBJECT(src_image[i] = ct_image_to_vx_image(src[i], context), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, arg_->width, arg_->height, arg_->dst_format), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxChannelCombineNode(graph, src_image[0], src_image[1], src_image[2], src_image[3], dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);
    ASSERT_NO_FAILURE(node_list[0] = node); 
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
    
    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(channel_combine_check(src[0], src[1], src[2], src[3], dst));

    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    ASSERT(dst_image == 0);

    for (i = 0; i < channels; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
        ASSERT(src_image[i] == 0);
    }

    printPerformance(perf_super_node, arg_->width*arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
}

TEST_WITH_ARG(tivxChannelCombine, testBasicTest, Arg,
    ChannelCombine_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image[4] = {0, 0, 0, 0};
    vx_image dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    int channels = 0, i;
    CT_Image src[4] = {NULL, NULL, NULL, NULL};
    CT_Image dst = NULL, dst_dummy = NULL;
    vx_enum channel_ref;

    ASSERT_NO_FAILURE(dst_dummy = ct_allocate_image(4, 4, arg_->dst_format));

    ASSERT_NO_FAILURE(channels = ct_get_num_channels(arg_->dst_format));
    channel_ref = (arg_->dst_format==VX_DF_IMAGE_RGB)||(arg_->dst_format==VX_DF_IMAGE_RGBX)?VX_CHANNEL_R:VX_CHANNEL_Y;
    for (i = 0; i < channels; i++)
    {
        int w = arg_->width / ct_image_get_channel_subsampling_x(dst_dummy, channel_ref + i);
        int h = arg_->height / ct_image_get_channel_subsampling_y(dst_dummy, channel_ref + i);
        ASSERT_NO_FAILURE(src[i] = channel_combine_image_generate_random(w, h, VX_DF_IMAGE_U8));
        ASSERT_VX_OBJECT(src_image[i] = ct_image_to_vx_image(src[i], context), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, arg_->width, arg_->height, arg_->dst_format), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxChannelCombineNode(graph, src_image[0], src_image[1], src_image[2], src_image[3], dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(channel_combine_check(src[0], src[1], src[2], src[3], dst));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    ASSERT(dst_image == 0);

    for (i = 0; i < channels; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
        ASSERT(src_image[i] == 0);
    }
}

#ifdef BUILD_BAM
#define testChannelCombineSingleSupernode testChannelCombineSingleSupernode
#else
#define testChannelCombineSingleSupernode DISABLED_testChannelCombineSingleSupernode
#endif

TESTCASE_TESTS(tivxChannelCombine,
        testGraphProcessing,
        testBasicTest,
        testChannelCombineSingleSupernode
)
