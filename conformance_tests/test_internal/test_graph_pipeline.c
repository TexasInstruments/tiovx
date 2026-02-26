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

#include <TI/tivx_test_kernels.h>
#include <TI/tivx_config.h>
#include <TI/tivx_capture.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include <string.h>
#include <math.h>

#include <vx_internal.h>
#include "shared_functions.h"


TESTCASE(tivxInternalGraphPipeline, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalGraphPipeline, negativeTestownCheckGraphCompleted)
{
    vx_context context = context_->vx_context_;
    vx_graph graph =NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* To fail condition 'graph object descriptor != NULL' */
    ASSERT((vx_bool)vx_false_e == ownCheckGraphCompleted(graph, 1));

    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalGraphPipeline, negativeTestownGraphAllocAndEnqueueObjDescForPipeline)
{
    vx_context context = context_->vx_context_;
    vx_graph graph =NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    graph->pipeline_depth = TIVX_GRAPH_MAX_PIPELINE_DEPTH;
    /*To hit 'graph -> pipeline_depth >= TIVX_GRAPH_MAX_PIPELINE_DEPTH' condition*/
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownGraphAllocAndEnqueueObjDescForPipeline(graph));
    graph->pipeline_depth = 1;

    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalGraphPipeline, negativeTestSetGraphScheduleConfig)
{
    vx_context context = context_->vx_context_;
    vx_graph graph =NULL;
    vx_enum graph_schedule_mode;
    vx_uint32 graph_parameters_list_size;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    graph_schedule_mode =(vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO;
    graph_parameters_list_size = 1;
    graph->num_params = 1;
    graph_parameters_queue_params_list[0].graph_parameter_index = 1;
    graph_parameters_queue_params_list [ 0 ].refs_list = (vx_reference *)&graph;
    graph_parameters_queue_params_list[0].refs_list_size = 1;

    /* To hit condition 'graph_parameters_queue_params_list.graph_parameter_index >= graph->num_params' */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetGraphScheduleConfig(graph, graph_schedule_mode, graph_parameters_list_size, graph_parameters_queue_params_list));
    /* To hit the condition ref list size > TIVX_OBJ_DESC_QUEUE_MAX_DEPTH and graph num param*/
    vx_image img[TIVX_OBJ_DESC_QUEUE_MAX_DEPTH+1];
    for (uint32_t i = 0; i < TIVX_OBJ_DESC_QUEUE_MAX_DEPTH+1; i++)
    {
        img[i] = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        graph_parameters_queue_params_list[0].refs_list = (vx_reference *)&img[i];
    }
    graph_parameters_queue_params_list[0].refs_list_size = TIVX_OBJ_DESC_QUEUE_MAX_DEPTH+1;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetGraphScheduleConfig(graph, graph_schedule_mode, graph_parameters_list_size, graph_parameters_queue_params_list));
    /* To hit only the condition ref list size > TIVX_OBJ_DESC_QUEUE_MAX_DEPTH */
    graph->num_params = 2;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetGraphScheduleConfig(graph, graph_schedule_mode, graph_parameters_list_size, graph_parameters_queue_params_list));

    for (uint32_t i = 0; i < TIVX_OBJ_DESC_QUEUE_MAX_DEPTH+1; i++)
    {
        VX_CALL(vxReleaseImage(&img[i]));
    }
    graph->num_params = 0;
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalGraphPipeline, negativeTestownGraphPipeDepthBoundary)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    node->node_depth = TIVX_GRAPH_MAX_PIPELINE_DEPTH;
    graph->is_pipeline_depth_set = vx_false_e;
    graph->leaf_nodes[0] = node;
    
    /*To hit 'pipe_depth >= TIVX_GRAPH_MAX_PIPELINE_DEPTH' condition*/
    ASSERT((int32_t)TIVX_GRAPH_MAX_PIPELINE_DEPTH-1 == ownGraphGetPipeDepth(graph));

    graph->leaf_nodes[0] = NULL;
    node->node_depth = 1;

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalGraphPipeline, negativeTestownGraphGetNumSchedule)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    uint32_t num_schedule = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* To hit condition 'min_count == (uint32_t)-1' inside ownGraphGetNumSchedule()*/
    graph -> schedule_mode = ( vx_enum ) VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL;
    ASSERT(num_schedule == ownGraphGetNumSchedule(graph));

    graph->schedule_mode = (vx_enum)VX_GRAPH_SCHEDULE_MODE_NORMAL;
    VX_CALL(vxReleaseGraph(&graph));
}

/* To fail ownReferenceLock() inside ownCheckGraphCompleted API */
TEST(tivxInternalGraphPipeline, negativeTestownCheckGraphCompleted1)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    tivx_mutex mutex, mutex1;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    mutex = graph->base.lock;
    mutex1 = graph->base.context->base.lock;
    graph->base.lock = NULL;
    graph->base.context->base.lock = NULL;
    ASSERT(vx_false_e == ownCheckGraphCompleted(graph, 0));

    graph->base.lock = mutex;
    graph->base.context->base.lock = mutex1;
    VX_CALL(vxReleaseGraph(&graph));
}

/* To fail ownReferenceLock() inside ownGraphScheduleGraph API */
TEST(tivxInternalGraphPipeline, negativeTestownGraphScheduleGraph)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    tivx_mutex mutex, mutex1;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    mutex = graph->base.lock;
    mutex1 = graph->base.context->base.lock;
    graph->base.lock = NULL;
    graph->base.context->base.lock = NULL;
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownGraphScheduleGraph(graph, 0));

    graph->base.lock = mutex;
    graph->base.context->base.lock = mutex1;
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalGraphPipeline, negativeTestownGraphDoScheduleGraphAfterEnqueue)
{
    ASSERT(vx_false_e == ownGraphDoScheduleGraphAfterEnqueue(NULL, 0));
}

TEST(tivxInternalGraphPipeline, negativeTestownGraphGetNumSchedule1)
{
    vx_context context = context_->vx_context_;
    uint32_t num_schedule = 0;
    vx_graph graph;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT(num_schedule == ownGraphGetNumSchedule(NULL));
    /* To fail condition 'graph->schedule_mode==(vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL' */
    ASSERT(num_schedule == ownGraphGetNumSchedule(graph));

    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalGraphPipeline, negativeTestGraphParameterDequeueDoneRefNullHostRef)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_image input = NULL, output = NULL;
    vx_image input_images[2], output_images[2];
    vx_uint32 num_buf = 2;
    vx_uint32 buf_id;
    vx_uint32 num_refs;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    uint64_t original_host_ref = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(input = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node = vxNotNode(graph, input, output), VX_TYPE_NODE);

    for(buf_id = 0; buf_id < num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(input_images[buf_id] = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_images[buf_id] = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    vx_parameter input_param = vxGetParameterByIndex(node, 0);
    vx_parameter output_param = vxGetParameterByIndex(node, 1);
    vxAddParameterToGraph(graph, input_param);
    vxAddParameterToGraph(graph, output_param);
    vxReleaseParameter(&input_param);
    vxReleaseParameter(&output_param);

    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&input_images[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&output_images[0];

    VX_CALL(vxSetGraphScheduleConfig(graph,
        VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
        2,
        graph_parameters_queue_params_list));

    VX_CALL(tivxSetGraphPipelineDepth(graph, 2));

    VX_CALL(vxVerifyGraph(graph));

    for(buf_id = 0; buf_id < num_buf; buf_id++)
    {
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 0,
            (vx_reference*)&input_images[buf_id], 1));
        VX_CALL(vxGraphParameterEnqueueReadyRef(graph, 1,
            (vx_reference*)&output_images[buf_id], 1));
    }

    {
        vx_uint32 max_wait_iterations = 1000;
        vx_uint32 wait_iteration = 0;
        vx_bool ref_available = vx_false_e;

        while((wait_iteration < max_wait_iterations) && (ref_available == vx_false_e))
        {
            vx_status check_status = vxGraphParameterCheckDoneRef(graph, 1, &num_refs);
            if((check_status == VX_SUCCESS) && (num_refs > 0))
            {
                ref_available = vx_true_e;
            }
            else
            {
                tivxTaskWaitMsecs(1);
                wait_iteration++;
            }
        }

        ASSERT(ref_available == vx_true_e);
    }

    if(output_images[0]->base.obj_desc != NULL)
    {
        tivx_obj_desc_t *obj_desc = (tivx_obj_desc_t*)output_images[0]->base.obj_desc;

        original_host_ref = obj_desc->host_ref;

        obj_desc->host_ref = 0;

        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&output, 1, &num_refs));

        obj_desc->host_ref = original_host_ref;
    }

    for(buf_id = 0; buf_id < (num_buf - 1); buf_id++)
    {
        vx_image out_img, in_img;
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 1,
            (vx_reference*)&out_img, 1, &num_refs));
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0,
            (vx_reference*)&in_img, 1, &num_refs));
    }

    {
        vx_image in_img;
        VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0,
            (vx_reference*)&in_img, 1, &num_refs));
    }

    VX_CALL(vxWaitGraph(graph));

    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleaseImage(&output));

    for(buf_id = 0; buf_id < num_buf; buf_id++)
    {
        VX_CALL(vxReleaseImage(&input_images[buf_id]));
        VX_CALL(vxReleaseImage(&output_images[buf_id]));
    }

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TESTCASE_TESTS(tivxInternalGraphPipeline,
    negativeTestownCheckGraphCompleted,
    negativeTestownGraphAllocAndEnqueueObjDescForPipeline,
    negativeTestSetGraphScheduleConfig,
    negativeTestownGraphPipeDepthBoundary,
    negativeTestownGraphGetNumSchedule,
    negativeTestownCheckGraphCompleted1,
    negativeTestownGraphScheduleGraph,
    negativeTestownGraphDoScheduleGraphAfterEnqueue,
    negativeTestownGraphGetNumSchedule1,
    negativeTestGraphParameterDequeueDoneRefNullHostRef
    )