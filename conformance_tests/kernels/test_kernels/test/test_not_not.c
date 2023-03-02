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
#include <TI/tivx_test_kernels.h>

static void fillSquence(CT_Image dst, uint32_t seq_init)
{
    uint32_t i, j;
    uint32_t val = seq_init;

    ASSERT(dst);
    ASSERT(dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = ++val;
}

TESTCASE(tivxTestKernelsNotNot,  CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    uint32_t width;
    uint32_t height;
} size_arg;

#define SIZE_ARG(w,h) ARG(#w "x" #h, w, h)

#define NOT_SIZE_ARGS       \
    SIZE_ARG(640, 480),     \
    ARG_EXTENDED_BEGIN(),   \
    SIZE_ARG(1, 1),         \
    SIZE_ARG(15, 17),       \
    SIZE_ARG(32, 32),       \
    SIZE_ARG(1231, 1234),   \
    SIZE_ARG(1280, 720),    \
    SIZE_ARG(1920, 1080),   \
    ARG_EXTENDED_END()

TEST_WITH_ARG(tivxTestKernelsNotNot, testSizes, size_arg, NOT_SIZE_ARGS)
{
    vx_image src, dst;
    CT_Image ref_src, vxdst;
    vx_graph graph;
    vx_node node;
    vx_context context = context_->vx_context_;

    ASSERT_NO_FAILURE({
        ref_src = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
        fillSquence(ref_src, (uint32_t)CT()->seed_);
        src = ct_image_to_vx_image(ref_src, context);
    });

    tivxTestKernelsLoadKernels(context);

    // build one-node graph
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node  = tivxNotNotNode(graph, src, dst), VX_TYPE_NODE);

    // run graph
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    ASSERT_NO_FAILURE({
        vxdst = ct_image_from_vx_image(dst);
    });

    ASSERT_EQ_CTIMAGE(ref_src, vxdst);

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

TEST_WITH_ARG(tivxTestKernelsNotNot, testNested, size_arg, NOT_SIZE_ARGS)
{
    vx_image src, dst;
    CT_Image ref_src, vxdst;
    vx_graph graph;
    vx_node node;
    vx_context context = context_->vx_context_;

    ASSERT_NO_FAILURE({
        ref_src = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
        fillSquence(ref_src, (uint32_t)CT()->seed_);
        src = ct_image_to_vx_image(ref_src, context);
    });

    tivxTestKernelsLoadKernels(context);

    // build one-node graph
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node  = tivxNotNotNode(graph, src, dst), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_A72_1));

    // run graph
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    ASSERT_NO_FAILURE({
        vxdst = ct_image_from_vx_image(dst);
    });

    ASSERT_EQ_CTIMAGE(ref_src, vxdst);

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

#define MAX_NUM_BUF               (8u)

TEST_WITH_ARG(tivxTestKernelsNotNot, testNestedPipelined, size_arg, NOT_SIZE_ARGS)
{
    vx_graph graph;
    vx_node node;
    vx_context context = context_->vx_context_;
    vx_image src[MAX_NUM_BUF] = {NULL}, dst[MAX_NUM_BUF] = {NULL};
    CT_Image ref_src[MAX_NUM_BUF], vxdst;
    uint32_t width, height, seq_init, num_buf;
    uint32_t buf_id, loop_id, loop_cnt;
    uint64_t exe_time;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    tivx_clr_debug_zone(VX_ZONE_INFO);

    seq_init = 1;
    width = arg_->width;
    height = arg_->height;
    loop_cnt = 20;
    num_buf = 2;

    ASSERT(num_buf <= MAX_NUM_BUF);

    /* fill reference data */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE({
            ref_src[buf_id] = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
            fillSquence(ref_src[buf_id], (uint32_t)(seq_init+buf_id*10));
        });
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(src[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst[buf_id]    = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    tivxTestKernelsLoadKernels(context);

    // build one-node graph
    ASSERT_VX_OBJECT(node  = tivxNotNotNode(graph, src[0], dst[0]), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_A72_1));


    /* input @ node index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, node, 0);
    /* output @ node index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, node, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&src[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&dst[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    VX_CALL(vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                2,
                graph_parameters_queue_params_list
                ));

    VX_CALL(vxVerifyGraph(graph));

    /* fill reference data into input data reference */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(src[buf_id], ref_src[buf_id]));
    }

    /* enqueue input and output references,
     * input and output can be enqueued in any order
     * can be enqueued all together, here they are enqueue one by one just as a example
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&src[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&dst[buf_id], 1));
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image out_img, in_img;
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_img, 1, &num_refs));
        /* Get consumed input reference, waits until a reference is available
         */
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_img, 1, &num_refs));

        /* A graph execution completed, since we dequeued both input and output refs */

        /* when measuring performance dont check output since it affects graph performance numbers
         */

        if(loop_cnt > 100)
        {
            ct_update_progress(loop_id, loop_cnt+num_buf);
        }

        ASSERT_NO_FAILURE({
            vxdst = ct_image_from_vx_image(out_img);
        });

        //printf("loopid=%d\n", loop_id);

        /* compare output */
        ASSERT_EQ_CTIMAGE(ref_src[buf_id], vxdst);

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_img, 1));
            VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_img, 1));
        }
    }

    /* ensure all graph processing is complete */
    VX_CALL(vxWaitGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&src[buf_id]));
        VX_CALL(vxReleaseImage(&dst[buf_id]));
    }
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}



TESTCASE_TESTS(tivxTestKernelsNotNot, testSizes, testNested, testNestedPipelined)
