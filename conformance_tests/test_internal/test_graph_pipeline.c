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

TEST(tivxInternalGraphPipeline, negativeTestGraphParameterEnqueueReadyRef)
{
    vx_context context = context_->vx_context_;
    vx_graph graph =NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* To hit NULL data_ref_queue condition */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxGraphParameterEnqueueReadyRef(NULL,0,NULL,0,0));

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

    /* To hit condition 'graph_parameters_queue_params_list.graph_parameter_index >= graph->num_params' */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxSetGraphScheduleConfig(graph, graph_schedule_mode, graph_parameters_list_size, graph_parameters_queue_params_list));
       
    graph->num_params = 0;
    VX_CALL(vxReleaseGraph(&graph));
}

TESTCASE_TESTS(tivxInternalGraphPipeline,
    negativeTestownCheckGraphCompleted,
    negativeTestGraphParameterEnqueueReadyRef,
    negativeTestownGraphAllocAndEnqueueObjDescForPipeline,
    negativeTestSetGraphScheduleConfig
    )