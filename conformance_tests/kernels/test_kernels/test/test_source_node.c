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
#include "test_tiovx/test_tiovx.h"

#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_khr_pipelining.h>
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_config.h>
#include <TI/tivx_capture.h>
#include <TI/tivx_task.h>
#include <TI/tivx_target_kernel.h>
#include "math.h"
#include <limits.h>

#define MAX_LINE_LEN   (256U)
#define NUM_CAMERAS    (4U)

#define LOG_RT_TRACE_ENABLE       (0u)

#define MAX_NUM_BUF               (8u)
#define MAX_IMAGE_PLANES          (3u)
#define MAX_NUM_OBJ_ARR_ELEMENTS  (4u)


TESTCASE(tivxSourceNode,  CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* testName;
    int width, height;
    int pipe_depth;
    int num_buf;
    int measure_perf;
} Pipeline_Arg;

#define ADD_BUF_1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/buf=1", __VA_ARGS__, 1))

#define ADD_BUF_2(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/buf=2", __VA_ARGS__, 2))

#define ADD_BUF_3(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/buf=3", __VA_ARGS__, 3))

#define ADD_PIPE_1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pipe_depth=1", __VA_ARGS__, 1))

#define ADD_PIPE_3(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pipe_depth=3", __VA_ARGS__, 3))

#define ADD_PIPE_6(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pipe_depth=6", __VA_ARGS__, 6))

#define ADD_PIPE_MAX(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pipe_depth=MAX", __VA_ARGS__, TIVX_GRAPH_MAX_PIPELINE_DEPTH-1))

#define MEASURE_PERF_OFF(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/measure_perf=OFF", __VA_ARGS__, 0))

#define MEASURE_PERF_ON(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/measure_perf=ON", __VA_ARGS__, 1))

#define ADD_SIZE_2048x1024(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=2048x1024", __VA_ARGS__, 2048, 1024))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_3, ADD_BUF_3, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_1, ADD_BUF_1, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_3, ADD_BUF_3, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_3, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_MAX, ADD_BUF_3, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_1, ADD_BUF_1, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_2, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_2, MEASURE_PERF_ON, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_2048x1024, ADD_PIPE_3, ADD_BUF_3, MEASURE_PERF_ON, ARG), \

/*
 * Utility API to set number of buffers at a node parameter
 * The parameter MUST be a output or bidirectonal parameter for the setting
 * to take effect
 */
static vx_status set_num_buf_by_node_index(vx_node node, vx_uint32 node_parameter_index, vx_uint32 num_buf)
{
    return tivxSetNodeParameterNumBufByIndex(node, node_parameter_index, num_buf);
}

/*
 * Utility API to set pipeline depth for a graph
 */
static vx_status set_graph_pipeline_depth(vx_graph graph, vx_uint32 pipeline_depth)
{
    return tivxSetGraphPipelineDepth(graph, pipeline_depth);
}

/*
 * Utility API to set trigger node for a graph
 */
static vx_status set_graph_trigger_node(vx_graph graph, vx_node node)
{
    return vxEnableGraphStreaming(graph, node);
}

/*
 * Utility API to export graph information to file for debug and visualization
 */
static vx_status export_graph_to_file(vx_graph graph, char *filename_prefix)
{
    size_t sz = 0;
    void* buf = 0;
    char filepath[MAXPATHLENGTH];

    sz = snprintf(filepath, MAXPATHLENGTH, "%s/output", ct_get_test_file_path());
    ASSERT_(return 0, (sz < MAXPATHLENGTH));
    return tivxExportGraphToDot(graph, filepath, filename_prefix);
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

/*
 * Utility API to log graph run-time trace
 */
static vx_status log_graph_rt_trace(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    #if LOG_RT_TRACE_ENABLE
    /* If run time logging is needed, update to use tivxLogRtTraceEnable,
     * tivxLogRtTraceDisable and tivxLogRtTraceExportToFile */
    #endif
    return status;
}

typedef struct {
    const char* name;
    int stream_time;
} Arg;

#define STREAMING_PARAMETERS \
    CT_GENERATE_PARAMETERS("streaming", ARG, 100), \
    CT_GENERATE_PARAMETERS("streaming", ARG, 1000)

int test = 0;
TEST_WITH_ARG(tivxSourceNode, testSourceObjArray, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar;
    uint32_t num_streams = 0;
    vx_node n1, n2;
    vx_object_array obj_array_scalar;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(obj_array_scalar = vxCreateObjectArray(context, (vx_reference)scalar, 4), VX_TYPE_OBJECT_ARRAY);

    VX_CALL(vxReleaseScalar(&scalar));

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceObjArrayNode(graph, obj_array_scalar), VX_TYPE_NODE);

    scalar = (vx_scalar)vxGetObjectArrayItem(obj_array_scalar, 0);

    ASSERT_VX_OBJECT(n2 = tivxScalarSink2Node(graph, scalar), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    if (test == 0)
    {
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, 3));

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n1, 0, 3));
    }

    test++;

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseObjectArray(&obj_array_scalar));
    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

/* test case for TIOVX-650 bug
   Note: currently disabled */
TEST(tivxSourceNode, testSourceObjArray2)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0, scalar_out_val[4];
    vx_scalar scalar[4], scalar_out[4];
    uint32_t num_streams = 0, i = 0;
    vx_node n1, n2[4];
    vx_object_array obj_array_scalar;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar[0] = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(obj_array_scalar = vxCreateObjectArray(context, (vx_reference)scalar[0], 4), VX_TYPE_OBJECT_ARRAY);

    VX_CALL(vxReleaseScalar(&scalar[0]));

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceObjArrayNode(graph, obj_array_scalar), VX_TYPE_NODE);

    for (i = 0; i < 4; i++)
    {
        scalar[i] = (vx_scalar)vxGetObjectArrayItem(obj_array_scalar, i);
        ASSERT_VX_OBJECT(scalar_out[i] = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(n2[i] = tivxScalarIntermediateNode(graph, scalar[i], scalar_out[i]), VX_TYPE_NODE);
        #if defined(SOC_AM62A)
        VX_CALL(vxSetNodeTarget(n2[i], VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
        #else
        VX_CALL(vxSetNodeTarget(n2[i], VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
        #endif
    }

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, 3));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n1, 0, 3));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_source_obj_array_2");

    VX_CALL(vxProcessGraph(graph));

    for (i = 0; i < 4; i++)
    {
        vxCopyScalar(scalar_out[i],
            &scalar_out_val[i],
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST
            );

        ASSERT(scalar_out_val[i] == (i+1));
    }

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseObjectArray(&obj_array_scalar));
    for (i = 0; i < 4; i++)
    {
        VX_CALL(vxReleaseScalar(&scalar[i]));
        VX_CALL(vxReleaseScalar(&scalar_out[i]));
        VX_CALL(vxReleaseNode(&n2[i]));
    }
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

TEST_WITH_ARG(tivxSourceNode, testSinkObjArray, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar;
    uint32_t num_streams = 0;
    vx_node n1, n2;
    vx_object_array obj_array_scalar;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(obj_array_scalar = vxCreateObjectArray(context, (vx_reference)scalar, 1), VX_TYPE_OBJECT_ARRAY);

    VX_CALL(vxReleaseScalar(&scalar));

    scalar = (vx_scalar)vxGetObjectArrayItem(obj_array_scalar, 0);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkObjArrayNode(graph, obj_array_scalar), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseObjectArray(&obj_array_scalar));
    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

/* TIOVX-656 partial bug fix */
TEST_WITH_ARG(tivxSourceNode, testSinkObjArray2, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_in, scalar_out, scalar_exemplar;
    uint32_t num_streams = 0;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, loopCnt;
    vx_node n0, n1, n2;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    vx_object_array obj_array_source[MAX_NUM_BUF], obj_array_sink;
    vx_bool prms_replicate[] =
        {vx_true_e, vx_true_e};

    /* Setting to num buf of capture node */
    num_buf = 3;
    loop_cnt = 1; //arg_->stream_time;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_exemplar  = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
         ASSERT_VX_OBJECT(obj_array_source[buf_id] = vxCreateObjectArray(context, (vx_reference)scalar_exemplar, 4), VX_TYPE_OBJECT_ARRAY);
    }

    ASSERT_VX_OBJECT(obj_array_sink   = vxCreateObjectArray(context, (vx_reference)scalar_exemplar, 4), VX_TYPE_OBJECT_ARRAY);

    VX_CALL(vxReleaseScalar(&scalar_exemplar));

    ASSERT_VX_OBJECT(scalar_in = (vx_scalar)vxGetObjectArrayItem(obj_array_source[0], 0), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = (vx_scalar)vxGetObjectArrayItem(obj_array_sink, 0), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n0 = tivxScalarSourceObjArrayNode(graph, obj_array_source[0]), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n0, "Source_node");

    ASSERT_VX_OBJECT(n1 = tivxScalarIntermediateNode(graph, scalar_in, scalar_out), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n1, "Intermediate_node");

    tivxSetNodeParameterNumBufByIndex(n1, 1u, num_buf);

    VX_CALL(vxReplicateNode(graph, n1, prms_replicate, 2u));

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkObjArrayNode(graph, obj_array_sink), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n2, "Sink_node");

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 0);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&obj_array_source[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            1,
            graph_parameters_queue_params_list
            );

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, num_buf));

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));

    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_sink_objarray_2");
    log_graph_rt_trace(graph);

    /* enqueue buf for pipeup but dont trigger graph execution */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&obj_array_source[buf_id], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array capture_arr;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&capture_arr, 1, &num_refs);

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_arr, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseScalar(&scalar_in));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseGraph(&graph));

    /* since buffers could be held by source node, first release graph
     * to delete source node, then free the buffers
     */
    VX_CALL(vxReleaseObjectArray(&obj_array_sink));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&obj_array_source[buf_id]));
    }
    tivxTestKernelsUnLoadKernels(context);
}

/* TIOVX-674 */
TEST_WITH_ARG(tivxSourceNode, testSinkObjArray3, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_in, scalar_out[MAX_NUM_BUF], scalar_exemplar;
    uint32_t num_streams = 0;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, loopCnt;
    vx_node n0, n1, n2;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_object_array obj_array_source[MAX_NUM_BUF], obj_array_sink[MAX_NUM_BUF];
    vx_bool prms_replicate[] =
        {vx_true_e, vx_true_e};

    /* Setting to num buf of capture node */
    num_buf = 3;
    loop_cnt = 1; //arg_->stream_time;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_exemplar  = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
         ASSERT_VX_OBJECT(obj_array_source[buf_id] = vxCreateObjectArray(context, (vx_reference)scalar_exemplar, 4), VX_TYPE_OBJECT_ARRAY);
         ASSERT_VX_OBJECT(obj_array_sink[buf_id]   = vxCreateObjectArray(context, (vx_reference)scalar_exemplar, 4), VX_TYPE_OBJECT_ARRAY);
         ASSERT_VX_OBJECT(scalar_out[buf_id] = (vx_scalar)vxGetObjectArrayItem(obj_array_sink[buf_id], 0), VX_TYPE_SCALAR);
    }


    VX_CALL(vxReleaseScalar(&scalar_exemplar));

    ASSERT_VX_OBJECT(scalar_in = (vx_scalar)vxGetObjectArrayItem(obj_array_source[0], 0), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n0 = tivxScalarSourceObjArrayNode(graph, obj_array_source[0]), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n0, "Source_node");

    ASSERT_VX_OBJECT(n1 = tivxScalarIntermediateNode(graph, scalar_in, scalar_out[0]), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n1, "Intermediate_node");

    VX_CALL(vxReplicateNode(graph, n1, prms_replicate, 2u));

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkObjArrayNode(graph, obj_array_sink[0]), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n2, "Sink_node");

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    add_graph_parameter_by_node_index(graph, n1, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&obj_array_source[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&scalar_out[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            2,
            graph_parameters_queue_params_list
            );

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));

    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, num_buf));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_sink_objarray_3");
    log_graph_rt_trace(graph);

#if 1
    /* enqueue buf for pipeup but dont trigger graph execution */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&obj_array_source[buf_id], 1);
        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&scalar_out[buf_id], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array capture_arr;
        vx_scalar scalar;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&capture_arr, 1, &num_refs);

        vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&scalar, 1, &num_refs);

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_arr, 1);

        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&scalar, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);
#endif
    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseScalar(&scalar_in));
    VX_CALL(vxReleaseGraph(&graph));

    /* since buffers could be held by source node, first release graph
     * to delete source node, then free the buffers
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&obj_array_source[buf_id]));
        VX_CALL(vxReleaseObjectArray(&obj_array_sink[buf_id]));
        VX_CALL(vxReleaseScalar(&scalar_out[buf_id]));
    }
    tivxTestKernelsUnLoadKernels(context);
}

/* Tests the following:
   1. Obj array produced as graph parameter (TIOVX-691)
   2. Obj array consumed as non-graph parameter (TIOVX-692)
*/
TEST_WITH_ARG(tivxSourceNode, testSinkObjArray4, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_in[4], scalar_out[4], scalar_exemplar;
    uint32_t num_streams = 0;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, loopCnt, ch_id;
    vx_node n0, n1[4], n2;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    vx_object_array obj_array_source[MAX_NUM_BUF], obj_array_sink;

    /* Setting to num buf of capture node */
    num_buf = 3;
    loop_cnt = 1; //arg_->stream_time;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_exemplar  = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
         ASSERT_VX_OBJECT(obj_array_source[buf_id] = vxCreateObjectArray(context, (vx_reference)scalar_exemplar, 4), VX_TYPE_OBJECT_ARRAY);
    }
    ASSERT_VX_OBJECT(obj_array_sink   = vxCreateObjectArray(context, (vx_reference)scalar_exemplar, 4), VX_TYPE_OBJECT_ARRAY);

    VX_CALL(vxReleaseScalar(&scalar_exemplar));

    ASSERT_VX_OBJECT(n0 = tivxScalarSourceObjArrayNode(graph, obj_array_source[0]), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n0, "Source_node");

    for(ch_id=0; ch_id<4; ch_id++)
    {
        ASSERT_VX_OBJECT(scalar_in[ch_id] = (vx_scalar)vxGetObjectArrayItem(obj_array_source[0], ch_id), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(scalar_out[ch_id] = (vx_scalar)vxGetObjectArrayItem(obj_array_sink, ch_id), VX_TYPE_SCALAR);

        ASSERT_VX_OBJECT(n1[ch_id] = tivxScalarIntermediateNode(graph, scalar_in[ch_id], scalar_out[ch_id]), VX_TYPE_NODE);

        vxSetReferenceName((vx_reference)n1[ch_id], "Intermediate_node");

        tivxSetNodeParameterNumBufByIndex(n1[ch_id], 1u, num_buf);
    }

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkObjArrayNode(graph, obj_array_sink), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n2, "Sink_node");

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 0);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&obj_array_source[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            1,
            graph_parameters_queue_params_list
            );

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    for(ch_id=0; ch_id<4; ch_id++)
    {
        #if defined(SOC_AM62A)
        VX_CALL(vxSetNodeTarget(n1[ch_id], VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
        #else
        VX_CALL(vxSetNodeTarget(n1[ch_id], VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
        #endif
    }

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, num_buf));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_sink_objarray_4");
    log_graph_rt_trace(graph);

#if 1
    /* enqueue buf for pipeup but dont trigger graph execution */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&obj_array_source[buf_id], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array capture_arr;
        vx_scalar scalar;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&capture_arr, 1, &num_refs);

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_arr, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);
#endif
    VX_CALL(vxReleaseNode(&n0));
    for(ch_id=0; ch_id<4; ch_id++)
    {
        VX_CALL(vxReleaseNode(&n1[ch_id]));
        VX_CALL(vxReleaseScalar(&scalar_in[ch_id]));
        VX_CALL(vxReleaseScalar(&scalar_out[ch_id]));
    }
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseGraph(&graph));

    /* since buffers could be held by source node, first release graph
     * to delete source node, then free the buffers
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&obj_array_source[buf_id]));
    }
    VX_CALL(vxReleaseObjectArray(&obj_array_sink));
    tivxTestKernelsUnLoadKernels(context);
}

/* Tests the following:
   1. Obj array produced as graph parameter (TIOVX-691)
   2. Obj array consumed as graph parameter (TIOVX-677)
*/
TEST_WITH_ARG(tivxSourceNode, testSinkObjArray5, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_in[4], scalar_out[4][MAX_NUM_BUF], scalar_exemplar;
    uint32_t num_streams = 0;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, loopCnt, ch_id;
    vx_node n0, n1[4], n2;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[5];
    vx_object_array obj_array_source[MAX_NUM_BUF], obj_array_sink[MAX_NUM_BUF];

    /* Setting to num buf of capture node */
    num_buf = 3;
    loop_cnt = 1; //arg_->stream_time;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_exemplar  = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(obj_array_source[buf_id] = vxCreateObjectArray(context, (vx_reference)scalar_exemplar, 4), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(obj_array_sink[buf_id]   = vxCreateObjectArray(context, (vx_reference)scalar_exemplar, 4), VX_TYPE_OBJECT_ARRAY);
    }

    VX_CALL(vxReleaseScalar(&scalar_exemplar));

    ASSERT_VX_OBJECT(n0 = tivxScalarSourceObjArrayNode(graph, obj_array_source[0]), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n0, "Source_node");

    for(ch_id=0; ch_id<4; ch_id++)
    {
        ASSERT_VX_OBJECT(scalar_in[ch_id] = (vx_scalar)vxGetObjectArrayItem(obj_array_source[0], ch_id), VX_TYPE_SCALAR);

        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            ASSERT_VX_OBJECT(scalar_out[ch_id][buf_id] = (vx_scalar)vxGetObjectArrayItem(obj_array_sink[buf_id], ch_id), VX_TYPE_SCALAR);
        }

        ASSERT_VX_OBJECT(n1[ch_id] = tivxScalarIntermediateNode(graph, scalar_in[ch_id], scalar_out[ch_id][0]), VX_TYPE_NODE);

        vxSetReferenceName((vx_reference)n1[ch_id], "Intermediate_node");

        tivxSetNodeParameterNumBufByIndex(n1[ch_id], 1u, num_buf);
    }

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkObjArrayNode(graph, obj_array_sink[0]), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n2, "Sink_node");

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    add_graph_parameter_by_node_index(graph, n1[0], 1);
    add_graph_parameter_by_node_index(graph, n1[1], 1);
    add_graph_parameter_by_node_index(graph, n1[2], 1);
    add_graph_parameter_by_node_index(graph, n1[3], 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&obj_array_source[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&scalar_out[0][0];

    graph_parameters_queue_params_list[2].graph_parameter_index = 2;
    graph_parameters_queue_params_list[2].refs_list_size = num_buf;
    graph_parameters_queue_params_list[2].refs_list = (vx_reference*)&scalar_out[1][0];

    graph_parameters_queue_params_list[3].graph_parameter_index = 3;
    graph_parameters_queue_params_list[3].refs_list_size = num_buf;
    graph_parameters_queue_params_list[3].refs_list = (vx_reference*)&scalar_out[2][0];

    graph_parameters_queue_params_list[4].graph_parameter_index = 4;
    graph_parameters_queue_params_list[4].refs_list_size = num_buf;
    graph_parameters_queue_params_list[4].refs_list = (vx_reference*)&scalar_out[3][0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            5,
            graph_parameters_queue_params_list
            );

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    for(ch_id=0; ch_id<4; ch_id++)
    {
        #if defined(SOC_AM62A)
        VX_CALL(vxSetNodeTarget(n1[ch_id], VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
        #else
        VX_CALL(vxSetNodeTarget(n1[ch_id], VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
        #endif
    }

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, num_buf));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_sink_objarray_5");
    log_graph_rt_trace(graph);

#if 1
    /* enqueue buf for pipeup but dont trigger graph execution */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&obj_array_source[buf_id], 1);
        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&scalar_out[0][buf_id], 1);
        vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&scalar_out[1][buf_id], 1);
        vxGraphParameterEnqueueReadyRef(graph, 3, (vx_reference*)&scalar_out[2][buf_id], 1);
        vxGraphParameterEnqueueReadyRef(graph, 4, (vx_reference*)&scalar_out[3][buf_id], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array capture_arr;
        vx_scalar scalar[4];

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&capture_arr, 1, &num_refs);
        vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&scalar[0], 1, &num_refs);
        vxGraphParameterDequeueDoneRef(graph, 2, (vx_reference*)&scalar[1], 1, &num_refs);
        vxGraphParameterDequeueDoneRef(graph, 3, (vx_reference*)&scalar[2], 1, &num_refs);
        vxGraphParameterDequeueDoneRef(graph, 4, (vx_reference*)&scalar[3], 1, &num_refs);

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_arr, 1);
        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&scalar[0], 1);
        vxGraphParameterEnqueueReadyRef(graph, 2, (vx_reference*)&scalar[1], 1);
        vxGraphParameterEnqueueReadyRef(graph, 3, (vx_reference*)&scalar[2], 1);
        vxGraphParameterEnqueueReadyRef(graph, 4, (vx_reference*)&scalar[3], 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);
#endif
    VX_CALL(vxReleaseNode(&n0));
    for(ch_id=0; ch_id<4; ch_id++)
    {
        VX_CALL(vxReleaseNode(&n1[ch_id]));
        VX_CALL(vxReleaseScalar(&scalar_in[ch_id]));
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            VX_CALL(vxReleaseScalar(&scalar_out[ch_id][buf_id]));
        }
    }
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseGraph(&graph));

    /* since buffers could be held by source node, first release graph
     * to delete source node, then free the buffers
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&obj_array_source[buf_id]));
        VX_CALL(vxReleaseObjectArray(&obj_array_sink[buf_id]));
    }
    tivxTestKernelsUnLoadKernels(context);
}


/* TIOVX-956 test case */
/*
 * ScalarSourceNode "streaming" data to two ScalarSink2Node's
 *
 *   tivxScalarIntermediateNode --> scalar (data) --->    Full obj arr    ---> tivxScalarSinkObjArrayNode
 *                                     |
 *                                     |------------->  Obj Array element ---> tivxScalarIntermediate2Node
 */
TEST(tivxSourceNode, testSinkObjArray6)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_in[MAX_NUM_BUF], scalar_out[MAX_NUM_BUF], scalar_exemplar, scalar_final;
    uint32_t num_streams = 0;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, loopCnt, ch_id, num_ch;
    vx_node n1, n2, n3;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_object_array obj_array_source[MAX_NUM_BUF], obj_array_sink[MAX_NUM_BUF];
    vx_bool prms_replicate[] =
        {vx_true_e, vx_true_e};

    /* Setting to num buf of capture node */
    num_buf = 3;
    loop_cnt = 10;
    num_ch = 4;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_exemplar  = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(scalar_final     = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
         ASSERT_VX_OBJECT(obj_array_source[buf_id] = vxCreateObjectArray(context, (vx_reference)scalar_exemplar, num_ch), VX_TYPE_OBJECT_ARRAY);
         scalar_val = buf_id+1;
         for (ch_id = 0; ch_id < num_ch; ch_id++)
         {
            vx_scalar tmp_scalar;
            ASSERT_VX_OBJECT(tmp_scalar  = (vx_scalar)vxGetObjectArrayItem(obj_array_source[buf_id], ch_id), VX_TYPE_SCALAR);
            VX_CALL(vxCopyScalar(tmp_scalar, &scalar_val, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            scalar_val++;
            VX_CALL(vxReleaseScalar(&tmp_scalar));
         }
         ASSERT_VX_OBJECT(obj_array_sink[buf_id]   = vxCreateObjectArray(context, (vx_reference)scalar_exemplar, num_ch), VX_TYPE_OBJECT_ARRAY);
         ASSERT_VX_OBJECT(scalar_in[buf_id]  = (vx_scalar)vxGetObjectArrayItem(obj_array_source[buf_id], 0), VX_TYPE_SCALAR);
         ASSERT_VX_OBJECT(scalar_out[buf_id] = (vx_scalar)vxGetObjectArrayItem(obj_array_sink[buf_id], 0), VX_TYPE_SCALAR);
    }

    VX_CALL(vxReleaseScalar(&scalar_exemplar));

    ASSERT_VX_OBJECT(n1 = tivxScalarIntermediateNode(graph, scalar_in[0], scalar_out[0]), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n1, "Intermediate_node");

    VX_CALL(vxReplicateNode(graph, n1, prms_replicate, 2u));

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkObjArrayNode(graph, obj_array_sink[0]), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n2, "Sink_node");

    ASSERT_VX_OBJECT(n3 = tivxScalarIntermediate2Node(graph, scalar_out[0], scalar_final), VX_TYPE_NODE);

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 0);
    add_graph_parameter_by_node_index(graph, n1, 1);

    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&scalar_in[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&scalar_out[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            2,
            graph_parameters_queue_params_list
            );

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));

    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, num_buf));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_sink_objarray_6");
    log_graph_rt_trace(graph);

#if 1
    /* enqueue buf for pipeup but dont trigger graph execution */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&scalar_in[buf_id], 1);
        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&scalar_out[buf_id], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array in_objarr;
        vx_scalar tmp_scalar_in, tmp_scalar_out;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&tmp_scalar_in, 1, &num_refs);
        vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&tmp_scalar_out, 1, &num_refs);

        ASSERT_VX_OBJECT(in_objarr = (vx_object_array)tivxGetReferenceParent((vx_reference)tmp_scalar_in), VX_TYPE_OBJECT_ARRAY);

        scalar_val = num_buf+loop_id+1;
        for (ch_id = 0; ch_id < num_ch; ch_id++)
        {
            vx_scalar tmp_scalar;
            ASSERT_VX_OBJECT(tmp_scalar  = (vx_scalar)vxGetObjectArrayItem(in_objarr, ch_id), VX_TYPE_SCALAR);
            VX_CALL(vxCopyScalar(tmp_scalar, &scalar_val, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            scalar_val++;
            VX_CALL(vxReleaseScalar(&tmp_scalar));
        }

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&tmp_scalar_in, 1);
        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&tmp_scalar_out, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);
#endif
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n3));
    VX_CALL(vxReleaseGraph(&graph));

    VX_CALL(vxReleaseScalar(&scalar_final));
    /* since buffers could be held by source node, first release graph
     * to delete source node, then free the buffers
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&obj_array_source[buf_id]));
        VX_CALL(vxReleaseObjectArray(&obj_array_sink[buf_id]));
        VX_CALL(vxReleaseScalar(&scalar_in[buf_id]));
        VX_CALL(vxReleaseScalar(&scalar_out[buf_id]));
    }
    tivxTestKernelsUnLoadKernels(context);
}

/* TIOVX-1151 test case */
/*
 * ScalarSourceNode "streaming" data to two ScalarSink2Node's
 *
 *   tivxScalarIntermediateNode (Repl) --> scalar (data) --->  Obj Array element ---> tivxScalarSink2Node
 *                                           |
 *                                           |-------------->  Obj Array element ---> tivxScalarIntermediate2Node (Repl) --->  Obj Array element ---> tivxScalarSink2Node
 */
TEST(tivxSourceNode, testSinkObjArray7)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_in[MAX_NUM_BUF], scalar_out, scalar_exemplar, scalar_exemplar2, scalar_final;
    uint32_t num_streams = 0;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, loopCnt, ch_id, num_ch;
    vx_node n1, n2, n3, n4;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_object_array obj_array_source[MAX_NUM_BUF], obj_array_final, obj_array_sink;
    vx_bool prms_replicate[] =
        {vx_true_e, vx_true_e};

    /* Setting to num buf of capture node */
    num_buf = 4;
    loop_cnt = 100;
    num_ch = 3;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_exemplar  = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(scalar_exemplar2     = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(obj_array_final = vxCreateObjectArray(context, (vx_reference)scalar_exemplar2, num_ch), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(scalar_final    = (vx_scalar)vxGetObjectArrayItem(obj_array_final, 0), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(obj_array_sink   = vxCreateObjectArray(context, (vx_reference)scalar_exemplar, num_ch), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(scalar_out       = (vx_scalar)vxGetObjectArrayItem(obj_array_sink, 0), VX_TYPE_SCALAR);

    VX_CALL(vxReleaseScalar(&scalar_exemplar2));

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
         ASSERT_VX_OBJECT(obj_array_source[buf_id] = vxCreateObjectArray(context, (vx_reference)scalar_exemplar, num_ch), VX_TYPE_OBJECT_ARRAY);
         scalar_val = buf_id+1;
         for (ch_id = 0; ch_id < num_ch; ch_id++)
         {
            vx_scalar tmp_scalar;
            ASSERT_VX_OBJECT(tmp_scalar  = (vx_scalar)vxGetObjectArrayItem(obj_array_source[buf_id], ch_id), VX_TYPE_SCALAR);
            VX_CALL(vxCopyScalar(tmp_scalar, &scalar_val, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            scalar_val++;
            VX_CALL(vxReleaseScalar(&tmp_scalar));
         }
         ASSERT_VX_OBJECT(scalar_in[buf_id]  = (vx_scalar)vxGetObjectArrayItem(obj_array_source[buf_id], 0), VX_TYPE_SCALAR);
    }

    VX_CALL(vxReleaseScalar(&scalar_exemplar));

    ASSERT_VX_OBJECT(n1 = tivxScalarIntermediateNode(graph, scalar_in[0], scalar_out), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n1, "Intermediate_node");

    VX_CALL(vxReplicateNode(graph, n1, prms_replicate, 2u));

    ASSERT_VX_OBJECT(n2 = tivxScalarSink2Node(graph, scalar_out), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n2, "Sink_node_n2");

    ASSERT_VX_OBJECT(n3 = tivxScalarIntermediate2Node(graph, scalar_out, scalar_final), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n3, "Intermediate_node_2");

    VX_CALL(vxReplicateNode(graph, n3, prms_replicate, 2u));

    ASSERT_VX_OBJECT(n4 = tivxScalarSink2Node(graph, scalar_final), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n4, "Sink_node_n4");

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 0);

    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&scalar_in[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            1,
            graph_parameters_queue_params_list
            );

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));

    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));

    VX_CALL(vxSetNodeTarget(n4, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    VX_CALL(vxSetNodeTarget(n4, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, num_buf));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n1, 1, 3));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n3, 1, 3));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_sink_objarray_7");
    log_graph_rt_trace(graph);

#if 1
    /* enqueue buf for pipeup but dont trigger graph execution */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&scalar_in[buf_id], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array in_objarr;
        vx_scalar tmp_scalar_in;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&tmp_scalar_in, 1, &num_refs);

        ASSERT_VX_OBJECT(in_objarr = (vx_object_array)tivxGetReferenceParent((vx_reference)tmp_scalar_in), VX_TYPE_OBJECT_ARRAY);

        scalar_val = num_buf+loop_id+1;
        for (ch_id = 0; ch_id < num_ch; ch_id++)
        {
            vx_scalar tmp_scalar;
            ASSERT_VX_OBJECT(tmp_scalar  = (vx_scalar)vxGetObjectArrayItem(in_objarr, ch_id), VX_TYPE_SCALAR);
            VX_CALL(vxCopyScalar(tmp_scalar, &scalar_val, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            scalar_val++;
            VX_CALL(vxReleaseScalar(&tmp_scalar));
        }

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&tmp_scalar_in, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);
#endif
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n3));
    VX_CALL(vxReleaseNode(&n4));
    VX_CALL(vxReleaseGraph(&graph));

    VX_CALL(vxReleaseScalar(&scalar_final));
    /* since buffers could be held by source node, first release graph
     * to delete source node, then free the buffers
     */
    VX_CALL(vxReleaseObjectArray(&obj_array_sink));
    VX_CALL(vxReleaseScalar(&scalar_out));

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&obj_array_source[buf_id]));
        VX_CALL(vxReleaseScalar(&scalar_in[buf_id]));
    }
    VX_CALL(vxReleaseObjectArray(&obj_array_final));
    tivxTestKernelsUnLoadKernels(context);
}

/*
 *       n1         scalar         n2
 * SCALAR_SOURCE2 -- SCALAR -- SCALAR_SINK2
 *
 * Scalar source2 node connected to scalar sink2 node
 * Error will be shown in a print statement if the scalar sink fails
 *
 */
TEST_WITH_ARG(tivxSourceNode, testNewSourceSink, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar;
    uint32_t num_streams = 0;
    vx_node n1, n2;


    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSource2Node(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSink2Node(graph, scalar), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    /* If i remove this, the test case hangs */
    //ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, 3));

    /* If i remove this, I get several more failures */
    //ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n1, 0, 3));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n1));

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams, sizeof(num_streams)));

    ASSERT(num_streams != 0);

    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

TEST_WITH_ARG(tivxSourceNode, testNewSourcePipeline, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar[MAX_NUM_BUF];
    uint32_t num_streams = 0;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, loopCnt;
    vx_node n1;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];

    /* Setting to num buf of capture node */
    num_buf = 3;
    loop_cnt = arg_->stream_time;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(scalar[buf_id] = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
    }

    ASSERT_VX_OBJECT(n1 = tivxScalarSource2Node(graph, scalar[0]), VX_TYPE_NODE);

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 0);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&scalar[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            1,
            graph_parameters_queue_params_list
            );

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_capture_pipeline_only");
    log_graph_rt_trace(graph);

    /* enqueue buf for pipeup but dont trigger graph execution */
    for(buf_id=0; buf_id<num_buf-1; buf_id++)
    {
        tivxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&scalar[buf_id], 1, TIVX_GRAPH_PARAMETER_ENQUEUE_FLAG_PIPEUP);
    }

    /* after pipeup, now enqueue a buffer to trigger graph scheduling */
    vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&scalar[buf_id], 1);

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_scalar out_scalar;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_scalar, 1, &num_refs);

        /* recycles dequeued input and output refs */
        /* input and output can be enqueued in any order */
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_scalar, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));

    /* since buffers could be held by source node, first release graph
     * to delete source node, then free the buffers
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseScalar(&scalar[buf_id]));
    }

    tivxTestKernelsUnLoadKernels(context);
}

TEST_WITH_ARG(tivxSourceNode, testNewSourceSinkPipeline, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar[MAX_NUM_BUF];
    uint32_t num_streams = 0;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, loopCnt;
    vx_node n1, n2;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];

    /* Setting to num buf of capture node */
    num_buf = 4;
    loop_cnt = 1; //arg_->stream_time;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(scalar[buf_id] = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
    }

    ASSERT_VX_OBJECT(n1 = tivxScalarSource2Node(graph, scalar[0]), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSink2Node(graph, scalar[0]), VX_TYPE_NODE);

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 0);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&scalar[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            1,
            graph_parameters_queue_params_list
            );

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_source_sink_pipelining");
    log_graph_rt_trace(graph);

    /* enqueue buf for pipeup but dont trigger graph execution */
    for(buf_id=0; buf_id<num_buf-2; buf_id++)
    {
        tivxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&scalar[buf_id], 1, TIVX_GRAPH_PARAMETER_ENQUEUE_FLAG_PIPEUP);
    }

    /* after pipeup, now enqueue a buffer to trigger graph scheduling */
    vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&scalar[num_buf-2], 1);

    /* need to trigger again since display holds on to a buffer, so need to trigger source node once more */
    vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&scalar[num_buf-1], 1);

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_scalar out_scalar;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_scalar, 1, &num_refs);

        /* recycles dequeued input and output refs */
        /* input and output can be enqueued in any order */
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_scalar, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));

    /* since buffers could be held by source node, first release graph
     * to delete source node, then free the buffers
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseScalar(&scalar[buf_id]));
    }
    tivxTestKernelsUnLoadKernels(context);
}

TEST_WITH_ARG(tivxSourceNode, testNewSourceIntermediatePipeline, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar[MAX_NUM_BUF], scalar_out[MAX_NUM_BUF];
    uint32_t num_streams = 0;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, loopCnt;
    vx_node n1, n2;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

    /* Setting to num buf of capture node */
    num_buf = 3;
    loop_cnt = 1; //arg_->stream_time;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(scalar[buf_id] = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
        ASSERT_VX_OBJECT(scalar_out[buf_id] = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
    }

    ASSERT_VX_OBJECT(n1 = tivxScalarSource2Node(graph, scalar[0]), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarIntermediateNode(graph, scalar[0], scalar_out[0]), VX_TYPE_NODE);

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 0);
    add_graph_parameter_by_node_index(graph, n2, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&scalar[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&scalar_out[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            2,
            graph_parameters_queue_params_list
            );

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_source_intermediate_pipelining");
    log_graph_rt_trace(graph);

    /* enqueue buf for pipeup but dont trigger graph execution */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&scalar[buf_id], 1);
        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&scalar_out[buf_id], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_scalar capture_scalar, out_scalar;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_scalar, 1, &num_refs);

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&capture_scalar, 1, &num_refs);

        /* recycles dequeued input and output refs */
        /* input and output can be enqueued in any order */
        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_scalar, 1);

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_scalar, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseGraph(&graph));

    /* since buffers could be held by source node, first release graph
     * to delete source node, then free the buffers
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseScalar(&scalar[buf_id]));
        VX_CALL(vxReleaseScalar(&scalar_out[buf_id]));
    }
    tivxTestKernelsUnLoadKernels(context);
}

/* Note: this test case does not enqueue/dequeue from graph parameter at capture node */
TEST_WITH_ARG(tivxSourceNode, testNewSourceIntermediatePipeline2, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar, scalar_out[MAX_NUM_BUF];
    uint32_t num_streams = 0;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, loopCnt;
    vx_node n1, n2;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];

    /* Setting to num buf of capture node */
    num_buf = 3;
    loop_cnt = 10; //arg_->stream_time;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(scalar_out[buf_id] = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
    }

    ASSERT_VX_OBJECT(n1 = tivxScalarSource2Node(graph, scalar), VX_TYPE_NODE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n1, 0, num_buf));

    ASSERT_VX_OBJECT(n2 = tivxScalarIntermediateNode(graph, scalar, scalar_out[0]), VX_TYPE_NODE);

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n2, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&scalar_out[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            1,
            graph_parameters_queue_params_list
            );

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_source_intermediate_pipelining2");
    log_graph_rt_trace(graph);

    /* after pipeup, now enqueue a buffer to trigger graph scheduling */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&scalar_out[buf_id], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_scalar capture_scalar, out_scalar;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_scalar, 1, &num_refs);

        /* recycles dequeued input and output refs */
        /* input and output can be enqueued in any order */
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_scalar, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseGraph(&graph));

    VX_CALL(vxReleaseScalar(&scalar));
    /* since buffers could be held by source node, first release graph
     * to delete source node, then free the buffers
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseScalar(&scalar_out[buf_id]));
    }
    tivxTestKernelsUnLoadKernels(context);
}

/*
 * Graph1
 *       n1         scalar         n2
 * SCALAR_SOURCE2 -- SCALAR -- SCALAR_SINK2
 *
 * Graph2
 *       n1         scalar         n2
 * SCALAR_SOURCE2 -- SCALAR -- SCALAR_SINK2
 *
 * Tests multiple graphs with streaming and pipelining enabled
 * Both nodes of graph1 are on MCU2_0
 * Both nodes of graph2 are on MCU2_1
 *
 */
#if !defined(SOC_AM62A)
TEST_WITH_ARG(tivxSourceNode, testMultiGraphPipelined1, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph1, graph2;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_g1, scalar_g2;
    vx_node n1, n2, n3, n4;
    uint32_t pipeline_depth, num_buf;
    uint32_t num_streams_g1 = 0, num_streams_g2 = 0;

    pipeline_depth = 3;
    num_buf = 3;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_g1 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g2 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSource2Node(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSink2Node(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n3 = tivxScalarSource2Node(graph2, scalar_g2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n4 = tivxScalarSink2Node(graph2, scalar_g2), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));
    VX_CALL(vxSetNodeTarget(n4, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph1, n1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph2, n3));

    /* explicitly set graph pipeline depth */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph1, pipeline_depth));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n1, 0, num_buf));

    /* explicitly set graph pipeline depth */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph2, pipeline_depth));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n3, 0, num_buf));

    VX_CALL(vxVerifyGraph(graph1));

    export_graph_to_file(graph1, "test_capture_multi_graph1_pipeline_streaming_graph1");
    log_graph_rt_trace(graph1);

    VX_CALL(vxVerifyGraph(graph2));

    export_graph_to_file(graph2, "test_capture_multi_graph1_pipeline_streaming_graph2");
    log_graph_rt_trace(graph2);

    VX_CALL(vxStartGraphStreaming(graph1));

    VX_CALL(vxStartGraphStreaming(graph2));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph1));

    VX_CALL(vxStopGraphStreaming(graph2));

    /* Beginning of error checking */
    VX_CALL(vxQueryGraph(graph1, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams_g1, sizeof(num_streams_g1)));

    ASSERT(num_streams_g1 != 0);

    VX_CALL(vxQueryGraph(graph2, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams_g2, sizeof(num_streams_g2)));

    ASSERT(num_streams_g2 != 0);
    /* End of error checking */

    VX_CALL(vxReleaseScalar(&scalar_g1));
    VX_CALL(vxReleaseScalar(&scalar_g2));
    VX_CALL(vxReleaseNode(&n4));
    VX_CALL(vxReleaseNode(&n3));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseGraph(&graph1));
    tivxTestKernelsUnLoadKernels(context);
}
#endif
/*
 * Graph1
 *       n1         scalar         n2
 * SCALAR_SOURCE2 -- SCALAR -- SCALAR_SINK2
 *
 * Graph2
 *       n1         scalar         n2
 * SCALAR_SOURCE2 -- SCALAR -- SCALAR_SINK2
 *
 * Tests multiple graphs with streaming and pipelining enabled
 * Both nodes of graph1 are on MCU2_0
 * Both nodes of graph2 are on MCU2_0
 *
 */
TEST_WITH_ARG(tivxSourceNode, testMultiGraphPipelined2, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph1, graph2;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_g1, scalar_g2;
    vx_node n1, n2, n3, n4;
    uint32_t pipeline_depth, num_buf;
    uint32_t num_streams_g1 = 0, num_streams_g2 = 0;

    pipeline_depth = 3;
    num_buf = 3;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_g1 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g2 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSource2Node(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSink2Node(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n3 = tivxScalarSource2Node(graph2, scalar_g2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n4 = tivxScalarSink2Node(graph2, scalar_g2), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n4, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n4, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph1, n1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph2, n3));

    /* explicitly set graph pipeline depth */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph1, pipeline_depth));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n1, 0, num_buf));

    /* explicitly set graph pipeline depth */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph2, pipeline_depth));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n3, 0, num_buf));

    VX_CALL(vxVerifyGraph(graph1));

    export_graph_to_file(graph1, "test_capture_multi_graph2_pipeline_streaming_graph1");
    log_graph_rt_trace(graph1);

    VX_CALL(vxVerifyGraph(graph2));

    export_graph_to_file(graph2, "test_capture_multi_graph2_pipeline_streaming_graph2");
    log_graph_rt_trace(graph2);

    VX_CALL(vxStartGraphStreaming(graph1));

    VX_CALL(vxStartGraphStreaming(graph2));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph1));

    VX_CALL(vxStopGraphStreaming(graph2));

    /* Beginning of error checking */
    VX_CALL(vxQueryGraph(graph1, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams_g1, sizeof(num_streams_g1)));

    ASSERT(num_streams_g1 != 0);

    VX_CALL(vxQueryGraph(graph2, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams_g2, sizeof(num_streams_g2)));

    ASSERT(num_streams_g2 != 0);
    /* End of error checking */

    VX_CALL(vxReleaseScalar(&scalar_g1));
    VX_CALL(vxReleaseScalar(&scalar_g2));
    VX_CALL(vxReleaseNode(&n4));
    VX_CALL(vxReleaseNode(&n3));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseGraph(&graph1));
    tivxTestKernelsUnLoadKernels(context);
}

/*
 * Graph1
 *       n1         scalar         n2
 * SCALAR_SOURCE2 -- SCALAR -- SCALAR_SINK2
 *
 * Graph2
 *       n1         scalar             n2            scalar         n3
 * SCALAR_SOURCE2 -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR -- SCALAR_SINK2
 *
 * Tests multiple graphs with streaming and pipelining enabled
 * Node1 of graph1 is on MCU2_0 while node2 of graph1 is on MCU2_1
 * Node1 of graph2 is on MCU2_0 while node2 and node3 of graph2 are on MCU2_1
 *
 */
#if !defined(SOC_AM62A)
TEST_WITH_ARG(tivxSourceNode, testMultiGraphPipelined3, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph1, graph2;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_g1, scalar_g2, scalar_out_g2;
    vx_node n1, n2, n3, n4, n5;
    uint32_t pipeline_depth, num_buf;
    uint32_t num_streams_g1 = 0, num_streams_g2 = 0;

    pipeline_depth = 3;
    num_buf = 3;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_g1 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g2 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out_g2 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSource2Node(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSink2Node(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n3 = tivxScalarSource2Node(graph2, scalar_g2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n4 = tivxScalarIntermediateNode(graph2, scalar_g2, scalar_out_g2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n5 = tivxScalarSink2Node(graph2, scalar_out_g2), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n4, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));
    VX_CALL(vxSetNodeTarget(n5, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph1, n1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph2, n3));

    /* explicitly set graph pipeline depth */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph1, pipeline_depth));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n1, 0, num_buf));

    /* explicitly set graph pipeline depth */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph2, pipeline_depth));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n3, 0, num_buf));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n4, 1, 2));

    VX_CALL(vxVerifyGraph(graph1));

    export_graph_to_file(graph1, "test_capture_multi_graph3_pipeline_streaming_graph1");
    log_graph_rt_trace(graph1);

    VX_CALL(vxVerifyGraph(graph2));

    export_graph_to_file(graph2, "test_capture_multi_graph3_pipeline_streaming_graph2");
    log_graph_rt_trace(graph2);

    VX_CALL(vxStartGraphStreaming(graph1));

    VX_CALL(vxStartGraphStreaming(graph2));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph1));

    VX_CALL(vxStopGraphStreaming(graph2));

    /* Beginning of error checking */
    VX_CALL(vxQueryGraph(graph1, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams_g1, sizeof(num_streams_g1)));

    ASSERT(num_streams_g1 != 0);

    VX_CALL(vxQueryGraph(graph2, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams_g2, sizeof(num_streams_g2)));

    ASSERT(num_streams_g2 != 0);
    /* End of error checking */

    VX_CALL(vxReleaseScalar(&scalar_g1));
    VX_CALL(vxReleaseScalar(&scalar_g2));
    VX_CALL(vxReleaseScalar(&scalar_out_g2));
    VX_CALL(vxReleaseNode(&n5));
    VX_CALL(vxReleaseNode(&n4));
    VX_CALL(vxReleaseNode(&n3));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseGraph(&graph1));
    tivxTestKernelsUnLoadKernels(context);
}
#endif
/*
 *       n0         scalar             n1             scalar         n2                scalar         n3
 * SCALAR_SOURCE2 -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR -- SCALAR_SINK2
 *
 * Scalar source node connected to scalar sink node with streaming and pipelining enabled
 * All nodes are on MCU2_0
 * Error will be shown in a print statement if the scalar sink fails
 *
 */
TEST_WITH_ARG(tivxSourceNode, testPipeliningStreaming1, Pipeline_Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n0, n1, n2, n3;

    uint64_t exe_time;
    uint32_t num_streams = 0;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar, scalar_int, scalar_out;

    tivxTestKernelsLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_int = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n0 = tivxScalarSource2Node(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n1 = tivxScalarIntermediateNode(graph, scalar, scalar_int), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarIntermediateNode(graph, scalar_int, scalar_out), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n3 = tivxScalarSink2Node(graph, scalar_out), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    /* Does not need explicit setting of num buf and pipeline depth */

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n2, 1, 2));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n0));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_capture_pipeline_streaming1");
    log_graph_rt_trace(graph);

    exe_time = tivxPlatformGetTimeInUsecs();

    VX_CALL(vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(1000);

    VX_CALL(vxStopGraphStreaming(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams, sizeof(num_streams)));

    ASSERT(num_streams != 0);

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n3));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseScalar(&scalar_int));
    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *       n0         scalar             n1            scalar         n2
 * SCALAR_SOURCE2 -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR -- SCALAR_SINK2
 *
 * Scalar source node connected to scalar sink node with streaming and pipelining enabled
 * Trigger node is intermediate node
 * All nodes are on MCU2_0
 * Error will be shown in a print statement if the scalar sink fails
 *
 */
TEST_WITH_ARG(tivxSourceNode, testPipeliningStreaming2, Pipeline_Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n0, n1, n2;

    uint32_t pipeline_depth, num_buf;
    uint64_t exe_time;
    uint32_t num_streams = 0;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar, scalar_out;

    tivxTestKernelsLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);

    pipeline_depth = 3;
    num_buf = 3;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n0 = tivxScalarSource2Node(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n1 = tivxScalarIntermediateNode(graph, scalar, scalar_out), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSink2Node(graph, scalar_out), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    /* explicitly set graph pipeline depth */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, pipeline_depth));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n0, 0, num_buf));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n1, 1, 2));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_capture_pipeline_streaming2");
    log_graph_rt_trace(graph);

    exe_time = tivxPlatformGetTimeInUsecs();

    VX_CALL(vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(1000);

    VX_CALL(vxStopGraphStreaming(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams, sizeof(num_streams)));

    ASSERT(num_streams != 0);

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *       n0         scalar             n1            scalar         n2
 * SCALAR_SOURCE2 -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR -- SCALAR_SINK2
 *
 * Scalar source node connected to scalar sink node with streaming and pipelining enabled
 * Trigger node is sink node
 * All nodes are on MCU2_0
 * Error will be shown in a print statement if the scalar sink fails
 *
 */
TEST_WITH_ARG(tivxSourceNode, testPipeliningStreaming3, Pipeline_Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n0, n1, n2;

    uint32_t pipeline_depth, num_buf;
    uint64_t exe_time;
    uint32_t num_streams = 0;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar, scalar_out;

    tivxTestKernelsLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);

    pipeline_depth = 3;
    num_buf = 3;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n0 = tivxScalarSource2Node(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n1 = tivxScalarIntermediateNode(graph, scalar, scalar_out), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSink2Node(graph, scalar_out), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    /* explicitly set graph pipeline depth */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, pipeline_depth));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n0, 0, num_buf));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n1, 1, 2));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n2));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_capture_pipeline_streaming3");
    log_graph_rt_trace(graph);

    exe_time = tivxPlatformGetTimeInUsecs();

    VX_CALL(vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(1000);

    VX_CALL(vxStopGraphStreaming(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams, sizeof(num_streams)));

    ASSERT(num_streams != 0);

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 * Test for TIOVX-1002
 * Calling load kernels and having the garbage collection perform the remove kernels
 */
TEST(tivxSourceNode, testContextRelease)
{
    vx_context context = context_->vx_context_;
    tivxTestKernelsLoadKernels(context);

    tivxTestKernelsUnSetLoadKernelsFlag();
}

TESTCASE_TESTS(tivxSourceNode,
               testSourceObjArray,
               DISABLED_testSourceObjArray2,
               testSinkObjArray,
               testSinkObjArray2,
               testSinkObjArray3,
               DISABLED_testSinkObjArray4,
               DISABLED_testSinkObjArray5,
               testSinkObjArray6,
               testSinkObjArray7,
               testNewSourceSink,
               testNewSourcePipeline,
               testNewSourceSinkPipeline,
               testNewSourceIntermediatePipeline,
               testNewSourceIntermediatePipeline2,
#if !defined(SOC_AM62A)
               testMultiGraphPipelined1,
#endif
               testMultiGraphPipelined2,
#if !defined(SOC_AM62A)
               testMultiGraphPipelined3,
#endif
               testPipeliningStreaming1,
               testPipeliningStreaming2,
               testPipeliningStreaming3,
               testContextRelease)

