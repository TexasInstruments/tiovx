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

TESTCASE(tivxChannelExtractCombine, CT_VXContext, ct_setup_vx_context, 0)

CT_Image channel_extract_image_generate_random(int width, int height, vx_df_image format)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, format, &CT()->seed_, 0, 256));

    return image;
}


void channel_extract_plane(CT_Image src, vx_enum channel, CT_Image* dst)
{
    uint8_t *src_base = NULL;
    int x, y;

    int plane, component;

    int x_subsampling = ct_image_get_channel_subsampling_x(src, channel);
    int y_subsampling = ct_image_get_channel_subsampling_y(src, channel);

    int xstep = ct_image_get_channel_step_x(src, channel);
    int ystep = ct_image_get_channel_step_y(src, channel);

    int dst_width = src->width / x_subsampling;
    int dst_height = src->height / y_subsampling;

    ASSERT_NO_FAILURE(plane = ct_image_get_channel_plane(src, channel));
    ASSERT_NO_FAILURE(component = ct_image_get_channel_component(src, channel));

    ASSERT(src_base = ct_image_get_plane_base(src, plane));

    ASSERT_NO_FAILURE(*dst = ct_allocate_image(dst_width, dst_height, VX_DF_IMAGE_U8));

    for (y = 0; y < dst_height; y++)
    {
        for (x = 0; x < dst_width; x++)
        {
            uint8_t* dst_data = CT_IMAGE_DATA_PTR_8U(*dst, x, y);
            uint8_t *src_data = src_base + (x * xstep) + (y * ystep);
            *dst_data = src_data[component];
        }
    }

    return;
}


CT_Image channel_extract_create_reference_image(CT_Image src, vx_enum channelNum)
{
    CT_Image dst = NULL;

    ASSERT_NO_FAILURE_(return NULL, channel_extract_plane(src, channelNum, &dst));

    ASSERT_(return NULL, dst);
    return dst;
}


typedef struct {
    const char* testName;
    vx_df_image format;
    vx_enum channel;
    int width, height;
} Arg;

#define ADD_CASE(testArgName, nextmacro, format, channel, ...) \
    CT_EXPAND(nextmacro(testArgName "/" #format "/" #channel, __VA_ARGS__, format, channel))

#define ADD_CASES(testArgName, nextmacro, ...) \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_RGB, VX_CHANNEL_G, __VA_ARGS__)

#define ADD_SUPERNODE_CASES(testArgName, nextmacro, ...) \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_RGB, VX_CHANNEL_G, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_RGBX, VX_CHANNEL_A, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_YUYV, VX_CHANNEL_Y, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_UYVY, VX_CHANNEL_U, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_YUYV, VX_CHANNEL_V, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_NV12, VX_CHANNEL_Y, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_NV12, VX_CHANNEL_V, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_NV21, VX_CHANNEL_U, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_NV21, VX_CHANNEL_V, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_YUV4, VX_CHANNEL_Y, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_YUV4, VX_CHANNEL_U, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_YUV4, VX_CHANNEL_V, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_IYUV, VX_CHANNEL_Y, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_IYUV, VX_CHANNEL_U, __VA_ARGS__), \
    ADD_CASE(testArgName, nextmacro, VX_DF_IMAGE_IYUV, VX_CHANNEL_V, __VA_ARGS__), \

#define ADD_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=18x18", __VA_ARGS__, 18, 18)), \
    CT_EXPAND(nextmacro(testArgName "/sz=644x258", __VA_ARGS__, 644, 258)), \
    CT_EXPAND(nextmacro(testArgName "/sz=1600x1200", __VA_ARGS__, 1600, 1200))
	
#define ADD_SUPERNODE_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=640x480", __VA_ARGS__, 640, 480))

#define ChannelExtractCombine_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_CASES, ADD_SIZE, ARG)
	
#define ChannelExtractSupernode_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SUPERNODE_CASES, ADD_SUPERNODE_SIZE, ARG)

#define ChannelExtractCombineSupernode_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_CASES, ADD_SUPERNODE_SIZE, ARG)


TEST_WITH_ARG(tivxChannelExtractCombine, testExtractCombine, Arg,
    ChannelExtractCombine_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt1 = 0, virt2 = 0, virt3 = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_node4, perf_graph;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect = vx_false_e;

    CT_Image src = NULL, dst = NULL;
    CT_Image dst_ref = NULL;

    ASSERT_NO_FAILURE(src = channel_extract_image_generate_random(arg_->width, arg_->height, arg_->format));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, arg_->width, arg_->height, arg_->format), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    if (NULL != src)
    {
        ASSERT_NO_FAILURE(dst_ref = channel_extract_create_reference_image(src, arg_->channel));
    }

    if (NULL != dst_ref)
    {
        ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, dst_ref->width, dst_ref->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, dst_ref->width, dst_ref->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(virt3   = vxCreateVirtualImage(graph, dst_ref->width, dst_ref->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    if (NULL != virt1)
    {
        node1 = vxChannelExtractNode(graph, src_image, VX_CHANNEL_R, virt1);
        ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
    }

    if (NULL != virt2)
    {
        node2 = vxChannelExtractNode(graph, src_image, VX_CHANNEL_G, virt2);
        ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);
    }

    if (NULL != virt3)
    {
        node3 = vxChannelExtractNode(graph, src_image, VX_CHANNEL_B, virt3);
        ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);
    }

    node4 = vxChannelCombineNode(graph, virt1, virt2, virt3, NULL, dst_image);
    ASSERT_VX_OBJECT(node4, VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (NULL != node1)
    {
        vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    }
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src_image, &src_rect);
    vxGetValidRegionImage(dst_image, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), arg_->width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), arg_->height);

    ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), arg_->width);
    ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), arg_->height);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryNode(node4, VX_NODE_PERFORMANCE, &perf_node4, sizeof(perf_node4));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    EXPECT_EQ_CTIMAGE(src, dst);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&virt3));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3, arg_->width*arg_->height, "N3");
    printPerformance(perf_node4, arg_->width*arg_->height, "N4");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TEST_WITH_ARG(tivxChannelExtractCombine, testCombineExtract, Arg,
    ChannelExtractCombine_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image[3] = {0, 0, 0};
    vx_image dst_image[3] = {0, 0, 0};
    vx_image virt;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_node4, perf_graph;
    int i;

    CT_Image src[3] = {NULL, NULL, NULL};
    CT_Image dst[3] = {NULL, NULL, NULL};
    CT_Image dst_dummy = NULL;
    vx_enum channel_ref;

    ASSERT_NO_FAILURE(dst_dummy = ct_allocate_image(4, 4, arg_->format));
    channel_ref = (arg_->format==VX_DF_IMAGE_RGB)||(arg_->format==VX_DF_IMAGE_RGBX)?VX_CHANNEL_R:VX_CHANNEL_Y;
    for (i = 0; i < 3; i++)
    {
        int w = arg_->width / ct_image_get_channel_subsampling_x(dst_dummy, channel_ref + i);
        int h = arg_->height / ct_image_get_channel_subsampling_y(dst_dummy, channel_ref + i);
        ASSERT_NO_FAILURE(src[i] = channel_extract_image_generate_random(w, h, VX_DF_IMAGE_U8));
        ASSERT_VX_OBJECT(src_image[i] = ct_image_to_vx_image(src[i], context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, arg_->width, arg_->height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);

    node1 = vxChannelCombineNode(graph, src_image[0], src_image[1], src_image[2], NULL, virt);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    node2 = vxChannelExtractNode(graph, virt, VX_CHANNEL_R, dst_image[0]);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    node3 = vxChannelExtractNode(graph, virt, VX_CHANNEL_G, dst_image[1]);
    ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);

    node4 = vxChannelExtractNode(graph, virt, VX_CHANNEL_B, dst_image[2]);
    ASSERT_VX_OBJECT(node4, VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryNode(node4, VX_NODE_PERFORMANCE, &perf_node4, sizeof(perf_node4));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(dst[0] = ct_image_from_vx_image(dst_image[0]));
    ASSERT_NO_FAILURE(dst[1] = ct_image_from_vx_image(dst_image[1]));
    ASSERT_NO_FAILURE(dst[2] = ct_image_from_vx_image(dst_image[2]));

    EXPECT_EQ_CTIMAGE(src[0], dst[0]);
    EXPECT_EQ_CTIMAGE(src[1], dst[1]);
    EXPECT_EQ_CTIMAGE(src[2], dst[2]);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&virt));

    for (i = 0; i < 3; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(src_image[i] == 0);
        ASSERT(dst_image[i] == 0);
    }

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3, arg_->width*arg_->height, "N3");
    printPerformance(perf_node4, arg_->width*arg_->height, "N4");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}


TEST_WITH_ARG(tivxChannelExtractCombine, testExtractCombineSupernode, Arg,
    ChannelExtractCombineSupernode_PARAMETERS
)
{
    int node_count = 4;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt1 = 0, virt2 = 0, virt3 = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    vx_perf_t perf_super_node, perf_graph;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect = vx_false_e;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    CT_Image src = NULL, dst = NULL;
    CT_Image dst_ref = NULL;

    ASSERT_NO_FAILURE(src = channel_extract_image_generate_random(arg_->width, arg_->height, arg_->format));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, arg_->width, arg_->height, arg_->format), VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    if (NULL != src)
    {
        ASSERT_NO_FAILURE(dst_ref = channel_extract_create_reference_image(src, arg_->channel));
    }

    if (NULL != dst_ref)
    {
        ASSERT_VX_OBJECT(virt1   = vxCreateVirtualImage(graph, dst_ref->width, dst_ref->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(virt2   = vxCreateVirtualImage(graph, dst_ref->width, dst_ref->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(virt3   = vxCreateVirtualImage(graph, dst_ref->width, dst_ref->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    if (NULL != virt1)
    {
        node1 = vxChannelExtractNode(graph, src_image, VX_CHANNEL_R, virt1);
        ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
    }

    if (NULL != virt2)
    {
        node2 = vxChannelExtractNode(graph, src_image, VX_CHANNEL_G, virt2);
        ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);
    }

    if (NULL != virt2)
    {
        node3 = vxChannelExtractNode(graph, src_image, VX_CHANNEL_B, virt3);
        ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);
    }

    node4 = vxChannelCombineNode(graph, virt1, virt2, virt3, NULL, dst_image);
    ASSERT_VX_OBJECT(node4, VX_TYPE_NODE);

    ASSERT_NO_FAILURE(node_list[0] = node1); 
    ASSERT_NO_FAILURE(node_list[1] = node2);
    ASSERT_NO_FAILURE(node_list[2] = node3);
    ASSERT_NO_FAILURE(node_list[3] = node4);
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
	
    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    if (NULL != node1)
    {
        vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    }
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src_image, &src_rect);
    vxGetValidRegionImage(dst_image, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), arg_->width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), arg_->height);

    ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), arg_->width);
    ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), arg_->height);

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    EXPECT_EQ_CTIMAGE(src, dst);

    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));
    VX_CALL(vxReleaseImage(&virt1));
    VX_CALL(vxReleaseImage(&virt2));
    VX_CALL(vxReleaseImage(&virt3));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_super_node, arg_->width * arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
}

TEST_WITH_ARG(tivxChannelExtractCombine, testCombineExtractSupernode, Arg,
    ChannelExtractCombineSupernode_PARAMETERS
)
{
    int node_count = 4;
    vx_context context = context_->vx_context_;
    vx_image src_image[3] = {0, 0, 0};
    vx_image dst_image[3] = {0, 0, 0};
    vx_image virt;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    int i;

    CT_Image src[3] = {NULL, NULL, NULL};
    CT_Image dst[3] = {NULL, NULL, NULL};
    CT_Image dst_dummy = NULL;
    vx_enum channel_ref;

    ASSERT_NO_FAILURE(dst_dummy = ct_allocate_image(4, 4, arg_->format));
    channel_ref = (arg_->format==VX_DF_IMAGE_RGB)||(arg_->format==VX_DF_IMAGE_RGBX)?VX_CHANNEL_R:VX_CHANNEL_Y;
    for (i = 0; i < 3; i++)
    {
        int w = arg_->width / ct_image_get_channel_subsampling_x(dst_dummy, channel_ref + i);
        int h = arg_->height / ct_image_get_channel_subsampling_y(dst_dummy, channel_ref + i);
        ASSERT_NO_FAILURE(src[i] = channel_extract_image_generate_random(w, h, VX_DF_IMAGE_U8));
        ASSERT_VX_OBJECT(src_image[i] = ct_image_to_vx_image(src[i], context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, arg_->width, arg_->height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);

    node1 = vxChannelCombineNode(graph, src_image[0], src_image[1], src_image[2], NULL, virt);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    node2 = vxChannelExtractNode(graph, virt, VX_CHANNEL_R, dst_image[0]);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    node3 = vxChannelExtractNode(graph, virt, VX_CHANNEL_G, dst_image[1]);
    ASSERT_VX_OBJECT(node3, VX_TYPE_NODE);

    node4 = vxChannelExtractNode(graph, virt, VX_CHANNEL_B, dst_image[2]);
    ASSERT_VX_OBJECT(node4, VX_TYPE_NODE);

    ASSERT_NO_FAILURE(node_list[0] = node1); 
    ASSERT_NO_FAILURE(node_list[1] = node2);
    ASSERT_NO_FAILURE(node_list[2] = node3);
    ASSERT_NO_FAILURE(node_list[3] = node4);
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
	
    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    ASSERT_NO_FAILURE(dst[0] = ct_image_from_vx_image(dst_image[0]));
    ASSERT_NO_FAILURE(dst[1] = ct_image_from_vx_image(dst_image[1]));
    ASSERT_NO_FAILURE(dst[2] = ct_image_from_vx_image(dst_image[2]));

    EXPECT_EQ_CTIMAGE(src[0], dst[0]);
    EXPECT_EQ_CTIMAGE(src[1], dst[1]);
    EXPECT_EQ_CTIMAGE(src[2], dst[2]);

    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&virt));

    for (i = 0; i < 3; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(src_image[i] == 0);
        ASSERT(dst_image[i] == 0);
    }

    printPerformance(perf_super_node, arg_->width * arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
}


TEST_WITH_ARG(tivxChannelExtractCombine, testChannelExtractSupernode, Arg,
    ChannelExtractSupernode_PARAMETERS
)
{
    int node_count = 1;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_super_node, perf_graph;
    vx_rectangle_t src_rect = {0}, dst_rect = {0};
    vx_bool valid_rect;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];

    CT_Image src = NULL, dst = NULL;
    CT_Image dst_ref = NULL;

    ASSERT_NO_FAILURE(src = channel_extract_image_generate_random(arg_->width, arg_->height, arg_->format));

    if (NULL != src)
    {
        ASSERT_NO_FAILURE(dst_ref = channel_extract_create_reference_image(src, arg_->channel));
    }

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    if (NULL != dst_ref)
    {
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_ref->width, dst_ref->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);    
    }

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxChannelExtractNode(graph, src_image, arg_->channel, dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    ASSERT_NO_FAILURE(node_list[0] = node);
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));
	
    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src_image, &src_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), arg_->width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), arg_->height);

    if (NULL != dst_ref)
    {
        vxGetValidRegionImage(dst_image, &dst_rect);
        ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), dst_ref->width);
        ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), dst_ref->height);
    }

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
    //EXPECT_CTIMAGE_NEARWRAP(dst_ref, dst, 1, CTIMAGE_ALLOW_WRAP);

    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(super_node == 0);
    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_super_node, arg_->width * arg_->height, "SN");
    printPerformance(perf_graph, arg_->width*arg_->height, "G");
}

#ifdef BUILD_BAM
#define testExtractCombineSupernode testExtractCombineSupernode
#define testCombineExtractSupernode testCombineExtractSupernode
#define testChannelExtractSupernode testChannelExtractSupernode
#else
#define testExtractCombineSupernode DISABLED_testExtractCombineSupernode
#define testCombineExtractSupernode DISABLED_testCombineExtractSupernode
#define testChannelExtractSupernode DISABLED_testChannelExtractSupernode
#endif

TESTCASE_TESTS(tivxChannelExtractCombine,
        testExtractCombine,
        testCombineExtract,
        testExtractCombineSupernode,
        testCombineExtractSupernode,
        testChannelExtractSupernode
)
