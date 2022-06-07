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


TESTCASE(tivxTimestamp,  CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* testName;
    int width, height;
    int pipe_depth;
    int num_buf;
    int loop_count;
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

#define ADD_LOOP_0(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=0", __VA_ARGS__, 0))

#define ADD_LOOP_1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=1", __VA_ARGS__, 1))

#define ADD_LOOP_10(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=10", __VA_ARGS__, 10))

#define ADD_LOOP_1000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=1000", __VA_ARGS__, 1000))

#define ADD_LOOP_100000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=100000", __VA_ARGS__, 100000))

#define ADD_LOOP_1000000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=1000000", __VA_ARGS__, 1000000))

#define MEASURE_PERF_OFF(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/measure_perf=OFF", __VA_ARGS__, 0))

#define MEASURE_PERF_ON(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/measure_perf=ON", __VA_ARGS__, 1))

#define ADD_SIZE_2048x1024(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=2048x1024", __VA_ARGS__, 2048, 1024))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_3, ADD_BUF_3, ADD_LOOP_0, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_1, ADD_BUF_1, ADD_LOOP_0, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_3, ADD_BUF_3, ADD_LOOP_1, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_3, ADD_LOOP_1000, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_MAX, ADD_BUF_3, ADD_LOOP_1000, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_1, ADD_BUF_1, ADD_LOOP_1000, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_2, ADD_LOOP_1000, MEASURE_PERF_OFF, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_64x64, ADD_PIPE_6, ADD_BUF_2, ADD_LOOP_100000, MEASURE_PERF_ON, ARG), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_2048x1024, ADD_PIPE_3, ADD_BUF_3, ADD_LOOP_1000, MEASURE_PERF_ON, ARG), \

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
    CT_GENERATE_PARAMETERS("streaming", ARG, 1000), \
    CT_GENERATE_PARAMETERS("streaming", ARG, 10000), \

/* TIOVX-674 */
TEST_WITH_ARG(tivxTimestamp, testTimestampPropagation, Arg, STREAMING_PARAMETERS)
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
        vx_uint64 timestamp1 = 0, timestamp2 = 0;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&capture_arr, 1, &num_refs);

        vxQueryReference((vx_reference)capture_arr, TIVX_REFERENCE_TIMESTAMP, &timestamp1, sizeof(timestamp1));

        vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&scalar, 1, &num_refs);

        vxQueryReference((vx_reference)scalar, TIVX_REFERENCE_TIMESTAMP, &timestamp2, sizeof(timestamp2));

        ASSERT(timestamp1==timestamp2);

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_arr, 1);

        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&scalar, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);
#endif

    {
        vx_reference refs[1];
        vx_user_data_object replicate_query;
        tivx_scalar_intermediate_control_t *replicate_struct;
        vx_map_id map_id;
        uint32_t *data_ptr;
        vx_imagepatch_addressing_t addr;
        vx_rectangle_t rect;

        replicate_query =
            vxCreateUserDataObject(context, "tivx_scalar_intermediate_control_t" ,
            sizeof(tivx_scalar_intermediate_control_t), NULL);

        refs[0] = (vx_reference)replicate_query;
        tivxNodeSendCommand(n1, 0,
            TIVX_SCALAR_INTERMEDIATE_REPLICATE_QUERY, refs, 1u);

        vxMapUserDataObject(
                (vx_user_data_object)refs[0],
                0,
                sizeof(tivx_scalar_intermediate_control_t),
                &map_id,
                (void **)&data_ptr,
                VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            );

        replicate_struct = (tivx_scalar_intermediate_control_t*)data_ptr;

        ASSERT(replicate_struct->is_target_kernel_replicated==(vx_bool)vx_true_e);

        vxUnmapUserDataObject((vx_user_data_object)refs[0], map_id);

        VX_CALL(vxReleaseUserDataObject(&replicate_query));
    }

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

TESTCASE_TESTS(tivxTimestamp,
               testTimestampPropagation)

