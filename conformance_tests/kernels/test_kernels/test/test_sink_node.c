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
#include "test_tiovx.h"

#include <VX/vx.h>
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_capture_nodes.h>
#include <TI/tivx_task.h>

#if !defined(SOC_AM62A)

TESTCASE(tivxTestSinkNode,  CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    uint32_t pipe_depth;
    uint32_t num_buf;
    uint32_t loop_count;
    uint32_t stream_time;
} Arg;

#define MAX_NUM_BUF               (8u)

#define ADD_BUF_3(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/buf=3", __VA_ARGS__, 3))

#define ADD_PIPE_3(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pipe_depth=3", __VA_ARGS__, 3))

#define ADD_LOOP_10(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=10", __VA_ARGS__, 10))

#define ADD_LOOP_100(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=100", __VA_ARGS__, 100))

#define ADD_LOOP_1000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loop_count=1000", __VA_ARGS__, 1000))

#define ADD_STREAM_TIME_100(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/stream_time=100", __VA_ARGS__, 100))

#define ADD_STREAM_TIME_1000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/stream_time=1000", __VA_ARGS__, 1000))

#define ADD_STREAM_TIME_10000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/stream_time=10000", __VA_ARGS__, 10000))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("sink_node", ADD_PIPE_3, ADD_BUF_3, ADD_LOOP_10, ADD_STREAM_TIME_100, ARG), \


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

/*
 * Utility API to set trigger node for a graph
 */
static vx_status set_graph_trigger_node(vx_graph graph, vx_node node)
{
    return vxEnableGraphStreaming(graph, node);
}

static void printGraphPipelinePerformance(vx_graph graph,
            vx_node nodes[], uint32_t num_nodes,
            uint64_t exe_time, uint32_t loop_cnt, uint32_t numPixels)
{
    #define MAX_TEST_NAME (8u)

    vx_perf_t perf_ref;
    char ref_name[MAX_TEST_NAME];
    uint32_t i;
    uint64_t avg_exe_time;

    avg_exe_time = exe_time / loop_cnt;

    for(i=0; i<num_nodes; i++)
    {
         vxQueryNode(nodes[i], VX_NODE_PERFORMANCE, &perf_ref, sizeof(perf_ref));
         snprintf(ref_name,MAX_TEST_NAME, "N%d ", i);
         printPerformance(perf_ref, numPixels, ref_name);
    }

    #if 0
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_ref, sizeof(perf_ref));
    snprintf(ref_name,MAX_TEST_NAME, "G0 ");
    printPerformance(perf_ref, numPixels, ref_name);
    #endif

    printf("[ SYS ] Execution time (avg = %4d.%03d ms, sum = %4d.%03d ms, num = %d)\n",
        (uint32_t)(avg_exe_time/1000u), (uint32_t)(avg_exe_time%1000u),
        (uint32_t)(exe_time/1000u), (uint32_t)(exe_time%1000u),
        loop_cnt
        );
}

/*
 * User enqueuing data to two ScalarSink2Node's
 *
 *   scalar (data)-----> ScalarSink2Node
 *      |
 *      |--------------> ScalarSink2Node
 */
TEST_WITH_ARG(tivxTestSinkNode, testSinkNode, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n1, n2;
    vx_scalar scalar[MAX_NUM_BUF] = {NULL};
    uint8_t scalar_val = 0;
    uint32_t loop_cnt, pipeline_depth, num_buf, i, buf_id, loop_id;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    uint64_t exe_time;

    loop_cnt = arg_->loop_count;
    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT(num_buf <= MAX_NUM_BUF);

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for(i=0; i<num_buf; i++)
    {
        ASSERT_VX_OBJECT(scalar[i] = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
    }

    ASSERT_VX_OBJECT(n1 = tivxScalarSink2Node(graph, scalar[0]), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    ASSERT_VX_OBJECT(n2 = tivxScalarSink2Node(graph, scalar[0]), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));

    /* input @ node index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n1, 0);

    /* set graph schedule config such that graph parameter @ index 0 is enqueuable */
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

    /* explicitly set graph pipeline depth */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_sink_node");
    log_graph_rt_trace(graph);

    scalar_val = 0;

    exe_time = tivxPlatformGetTimeInUsecs();

    /* enqueue input references,
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        scalar_val++;
        VX_CALL(vxCopyScalar(scalar[buf_id], &scalar_val, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&scalar[buf_id], 1);
    }

    buf_id = 0;

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_scalar in_scalar;
        uint8_t in_scalar_val;
        int16_t expected_val;
        uint32_t num_refs;

        /* Get consumed input reference, waits until a reference is available
         */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&in_scalar, 1, &num_refs);

        /* A graph execution completed, since we dequeued input */
        {
            if(loop_cnt > 100)
            {
                ct_update_progress(loop_id, loop_cnt+num_buf);
            }

            VX_CALL(vxCopyScalar(in_scalar, &in_scalar_val, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

            /* compare output */
            expected_val = (int16_t)scalar_val-num_buf+1;
            if(expected_val<0)
                expected_val += 256;

            /* printf("%d %d\n", in_scalar_val, expected_val); */
            ASSERT(in_scalar_val==((uint8_t)expected_val));

            /* fill new data */
            scalar_val++;
            VX_CALL(vxCopyScalar(in_scalar, &scalar_val, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        }

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input refs 'loop_cnt' times */
        if(loop_id<(loop_cnt+1))
        {
            vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_scalar, 1);
        }
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    {
        vx_node nodes[] = { n1 };

        printGraphPipelinePerformance(graph, nodes, 1, exe_time, loop_cnt+num_buf, 1);
    }

    for(i=0; i<num_buf; i++)
    {
        VX_CALL(vxReleaseScalar(&scalar[i]));
    }
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 * ScalarSourceNode "streaming" data to two ScalarSink2Node's
 *
 *   ScalarSourceNode --> scalar (data) -----> ScalarSink2Node
 *                          |
 *                          |----------------> ScalarSink2Node
 */

TEST_WITH_ARG(tivxTestSinkNode, testSourceSinkNode, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n1, n2, n0;
    vx_scalar scalar[MAX_NUM_BUF] = {NULL};
    uint8_t scalar_val = 0;
    uint32_t pipeline_depth, num_buf, i, buf_id, loop_id, num_streams;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    uint64_t exe_time;

    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT(num_buf <= MAX_NUM_BUF);

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for(i=0; i<num_buf; i++)
    {
        ASSERT_VX_OBJECT(scalar[i] = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
    }

    ASSERT_VX_OBJECT(n0 = tivxScalarSourceNode(graph, scalar[0]), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    ASSERT_VX_OBJECT(n1 = tivxScalarSink2Node(graph, scalar[0]), VX_TYPE_NODE);

    /* Note: to cause TIOVX-520 bug, change to MCU2_1 */
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    ASSERT_VX_OBJECT(n2 = tivxScalarSink2Node(graph, scalar[0]), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));

    /* explicitly set graph pipeline depth */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, pipeline_depth));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n0, 0, num_buf));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n0));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_source_sink_node");
    log_graph_rt_trace(graph);

    exe_time = tivxPlatformGetTimeInUsecs();

    VX_CALL(vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams, sizeof(num_streams)));

    ASSERT(num_streams != 0);

    printf(" Graph executed %d times\n", num_streams);

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    {
        vx_node nodes[] = { n0, n1, n2  };

        printGraphPipelinePerformance(graph, nodes, 3, exe_time, num_streams, 1);
    }

    for(i=0; i<num_buf; i++)
    {
        VX_CALL(vxReleaseScalar(&scalar[i]));
    }
    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 * ScalarSource2Node "streaming" data to two ScalarSink2Node's
 *
 * Here Source2Node has a pipeup requirement
 *
 *   ScalarSource2Node --> scalar (data) -----> ScalarSink2Node
 *                          |
 *                          |-----------------> ScalarSink2Node
 */

TEST_WITH_ARG(tivxTestSinkNode, testSourceSinkNode2, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n1, n2, n0;
    vx_scalar scalar[MAX_NUM_BUF] = {NULL};
    uint8_t scalar_val = 0;
    uint32_t pipeline_depth, num_buf, i, buf_id, loop_id, num_streams;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    uint64_t exe_time;

    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT(num_buf <= MAX_NUM_BUF);

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for(i=0; i<num_buf; i++)
    {
        ASSERT_VX_OBJECT(scalar[i] = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
    }

    ASSERT_VX_OBJECT(n0 = tivxScalarSource2Node(graph, scalar[0]), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    ASSERT_VX_OBJECT(n1 = tivxScalarSink2Node(graph, scalar[0]), VX_TYPE_NODE);

    /* Note: to cause TIOVX-520 bug, change to MCU2_1 */
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));

    ASSERT_VX_OBJECT(n2 = tivxScalarSink2Node(graph, scalar[0]), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));

    /* explicitly set graph pipeline depth */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, pipeline_depth));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n0, 0, num_buf));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n0));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_source_sink_node");
    log_graph_rt_trace(graph);

    exe_time = tivxPlatformGetTimeInUsecs();

    VX_CALL(vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams, sizeof(num_streams)));

    ASSERT(num_streams != 0);

    printf(" Graph executed %d times\n", num_streams);

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    {
        vx_node nodes[] = { n0, n1, n2  };

        printGraphPipelinePerformance(graph, nodes, 3, exe_time, num_streams, 1);
    }

    for(i=0; i<num_buf; i++)
    {
        VX_CALL(vxReleaseScalar(&scalar[i]));
    }
    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}


TESTCASE_TESTS(tivxTestSinkNode,
    testSinkNode,
    testSourceSinkNode,
    testSourceSinkNode2)

#endif