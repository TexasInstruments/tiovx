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
#include "math.h"
#include <limits.h>

#define MAX_LINE_LEN   (256U)
#define NUM_CAMERAS    (4U)

#define LOG_RT_TRACE_ENABLE       (0u)

#define MAX_NUM_BUF               (8u)
#define MAX_IMAGE_PLANES          (3u)
#define MAX_NUM_OBJ_ARR_ELEMENTS  (4u)

#define NODE1_EVENT     (1u)
#define NODE2_EVENT     (2u)

TESTCASE(tivxGraphStreaming,  CT_VXContext, ct_setup_vx_context, 0)

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

TEST_WITH_ARG(tivxGraphStreaming, testScalar, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar, scalar_out;
    uint8_t output = 0;
    uint32_t num_streams = 0, golden_output = 0;
    vx_node n1, n2;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarIntermediateNode(graph, scalar, scalar_out), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n1));

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams, sizeof(num_streams)));

    ASSERT(num_streams != 0);

    golden_output = num_streams % 256;

    output = ct_scalar_as_int(scalar_out);

    ASSERT(output==golden_output);
    if(output!=golden_output)
        printf("ERROR: %d != %d\n", output, golden_output);

    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

/*
 *       n1         scalar         n2
 * SCALAR_SOURCE -- SCALAR -- SCALAR_SINK
 *
 * Scalar source node connected to scalar sink node
 * Does not a set a trigger node
 * Both nodes on MCU2_0
 * Expecting an error since trigger node is not enabled with streaming
 *
 */
TEST(tivxGraphStreaming, negativeTestSourceSinkNoTrigger)
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

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkNode(graph, scalar), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, NULL));

    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

/*
 *       n1         scalar         n2
 * SCALAR_SOURCE -- SCALAR -- SCALAR_SINK
 *
 * Scalar source node connected to scalar sink node
 * Both nodes on MCU2_0
 * Error will be shown in a print statement if the scalar sink fails
 *
 */
TEST_WITH_ARG(tivxGraphStreaming, testSourceSink1, Arg, STREAMING_PARAMETERS)
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

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkNode(graph, scalar), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

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

/*
 *       n1         scalar         n2
 * SCALAR_SOURCE -- SCALAR -- SCALAR_SINK
 *
 * Scalar source node connected to scalar sink node
 * One node on MCU2_0 and the other on MCU2_1
 * Error will be shown in a print statement if the scalar sink fails
 *
 */
#if !defined(SOC_AM62A)

TEST_WITH_ARG(tivxGraphStreaming, testSourceSink2, Arg, STREAMING_PARAMETERS)
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

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkNode(graph, scalar), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));

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
#endif
/*
 *       n1          scalar            n2             scalar_out
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR
 *
 * Negative test for streaming
 * Tries to start streaming prior to trigger node being set
 * Tries to stop streaming before streaming has begun
 * Tries to start streaming after streaming has begun
 * Tries to stop streaming after streaming has stopped
 *
 */
TEST(tivxGraphStreaming, negativeTestStreamingState)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar, scalar_out;
    vx_node n1, n2;
    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarIntermediateNode(graph, scalar, scalar_out), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n1));

    VX_CALL(vxVerifyGraph(graph));

    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxStopGraphStreaming(graph));

    VX_CALL(vxStartGraphStreaming(graph));

    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(100);

    VX_CALL(vxStopGraphStreaming(graph));

    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxStopGraphStreaming(graph));

    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

/*
 *       n1               scalar            n2             scalar_out
 * SCALAR_SOURCE_ERROR -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR
 *
 * Test for node error event
 * Uses a special "error" source node that returns an error
 *
 */
TEST(tivxGraphStreaming, negativeTestStreamingError)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar, scalar_out;
    vx_node n1, n2;
    vx_bool done;
    vx_event_t event;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    /* modify to use an "error" node */
    ASSERT_VX_OBJECT(n1 = tivxScalarSourceErrorNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarIntermediateNode(graph, scalar, scalar_out), VX_TYPE_NODE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)n1, VX_EVENT_NODE_ERROR, 0, NODE1_EVENT));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)n2, VX_EVENT_NODE_ERROR, 0, NODE2_EVENT));

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n1));

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxStartGraphStreaming(graph));

    done = vx_false_e;

    while(!done)
    {
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxWaitEvent(context, &event, vx_false_e));

        if(event.app_value==NODE1_EVENT)
        {
            done = vx_true_e;
        }
    }

    VX_CALL(vxStopGraphStreaming(graph));

    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

TEST(tivxGraphStreaming, testScalarCtrlCmd)
{
    vx_status status;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 33;
    vx_scalar scalar[1];
    vx_node n1;
    int i;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar[0] = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph, scalar[0]), VX_TYPE_NODE);
    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    /* Negative test for sending control command before vxVerify (should fail) */
    status = tivxNodeSendCommand(n1, 0u, TIVX_SCALAR_SRC_NODE_INC_SCALAR,
        (vx_reference *)scalar, 1u);
    ASSERT_NE_VX_STATUS(VX_SUCCESS, status);

    VX_CALL(vxVerifyGraph(graph));

    for (i = 1; i <= 5; i ++)
    {
        status = tivxNodeSendCommand(n1, 0u, TIVX_SCALAR_SRC_NODE_INC_SCALAR,
            (vx_reference *)scalar, 1u);
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, status);

        ASSERT_EQ_VX_STATUS(ct_scalar_as_int(scalar[0]), scalar_val + i);
    }

    scalar_val = scalar_val + 5;
    for (i = 1; i <= 5; i ++)
    {
        status = tivxNodeSendCommand(n1, 0u, TIVX_SCALAR_SRC_NODE_DEC_SCALAR,
            (vx_reference *)scalar, 1u);
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, status);

        ASSERT_EQ_VX_STATUS(ct_scalar_as_int(scalar[0]), scalar_val - i);
    }

    VX_CALL(vxReleaseScalar(&scalar[0]));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

/*
 *       n1         scalar             n2             scalar_out
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR
 *
 * Negative test for streaming
 * Tries to call schedule graph while graph is streaming
 * Tries to call process graph while graph is streaming
 *
 */
TEST(tivxGraphStreaming, negativeTestScalar)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar, scalar_out;
    vx_node n1, n2;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarIntermediateNode(graph, scalar, scalar_out), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxStartGraphStreaming(graph));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n1));

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxStartGraphStreaming(graph));

    /* Checking that a call to schedule graph while streaming fails */
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));

    /* Checking that a call to process graph while streaming fails */
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    tivxTaskWaitMsecs(100);

    /* Checking that a call to schedule graph while streaming fails */
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));

    VX_CALL(vxStopGraphStreaming(graph));

    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

/*
 *       n1         scalar             n2             scalar_out
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR
 *
 * Negative test for streaming
 * Tries to enable streaming by calling set trigger node and enable auto pipelining
 * Verify will fail in this case
 *
 */
TEST(tivxGraphStreaming, negativeTestStreamingPipelining1)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar, scalar_out;
    vx_node n1, n2;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarIntermediateNode(graph, scalar, scalar_out), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n1));

    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            0,
            NULL
            );

    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

/*
 *       n1         scalar         n2                scalar_out
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR
 *
 * Negative test for streaming
 * Tries to enable streaming by calling set trigger node and enable manual pipelining
 * Verify will fail in this case
 *
 */
TEST(tivxGraphStreaming, negativeTestStreamingPipelining2)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar, scalar_out;
    vx_node n1, n2;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarIntermediateNode(graph, scalar, scalar_out), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n1));

    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL,
            0,
            NULL
            );

    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

/*
 *       n1         scalar         n2                scalar_out
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR
 *
 * Negative test for streaming
 * Does not set trigger node
 * Verify will fail in this case
 *
 */
TEST(tivxGraphStreaming, negativeTestPipeliningStreamingNoTrigger)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar, scalar_out;
    vx_node n1, n2;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarIntermediateNode(graph, scalar, scalar_out), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, 3));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, NULL));

    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

/*
 * Graph1
 *       n1         scalar             n2             scalar_out
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR
 *
 * Graph2
 *       n1         scalar             n2             scalar_out
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR
 *
 * Tests multiple graphs with streaming enabled
 * Both nodes of graph1 are on MCU2_0
 * Both nodes of graph2 are on MCU2_1
 *
 */
#if !defined(SOC_AM62A)
TEST_WITH_ARG(tivxGraphStreaming, testMultiGraph1, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph1, graph2;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_g1, scalar_g1_out, scalar_g2, scalar_g2_out;
    uint8_t output_g1 = 0, output_g2 = 0;
    uint32_t num_streams_g1 = 0, golden_output_g1 = 0, num_streams_g2 = 0, golden_output_g2 = 0;
    vx_node n1, n2, n3, n4;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_g1 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g2 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g1_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g2_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarIntermediateNode(graph1, scalar_g1, scalar_g1_out), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n3 = tivxScalarSourceNode(graph2, scalar_g2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n4 = tivxScalarIntermediateNode(graph2, scalar_g2, scalar_g2_out), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));
    VX_CALL(vxSetNodeTarget(n4, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph1, n1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph2, n3));

    VX_CALL(vxVerifyGraph(graph1));

    VX_CALL(vxVerifyGraph(graph2));

    VX_CALL(vxStartGraphStreaming(graph1));

    VX_CALL(vxStartGraphStreaming(graph2));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph1));

    VX_CALL(vxStopGraphStreaming(graph2));

    /* Beginning of error checking */
    VX_CALL(vxQueryGraph(graph1, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams_g1, sizeof(num_streams_g1)));

    ASSERT(num_streams_g1 != 0);

    golden_output_g1 = num_streams_g1 % 256;

    output_g1 = ct_scalar_as_int(scalar_g1_out);

    VX_CALL(vxQueryGraph(graph2, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams_g2, sizeof(num_streams_g2)));

    ASSERT(num_streams_g2 != 0);

    golden_output_g2 = num_streams_g2 % 256;

    output_g2 = ct_scalar_as_int(scalar_g2_out);

    if(output_g2!=golden_output_g2)
        printf("ERROR: %d != %d\n", output_g2, golden_output_g2);

    if(output_g1!=golden_output_g1)
        printf("ERROR: %d != %d\n", output_g2, golden_output_g2);
    /* End of error checking */

    VX_CALL(vxReleaseScalar(&scalar_g1));
    VX_CALL(vxReleaseScalar(&scalar_g2));
    VX_CALL(vxReleaseScalar(&scalar_g1_out));
    VX_CALL(vxReleaseScalar(&scalar_g2_out));
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
 *       n1         scalar             n2             scalar_out
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR
 *
 * Graph2
 *       n1         scalar             n2             scalar_out
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR
 *
 * Tests multiple graphs with streaming enabled
 * Both nodes of graph1 are on MCU2_0
 * Both nodes of graph2 are on MCU2_0
 *
 */
TEST_WITH_ARG(tivxGraphStreaming, testMultiGraph2, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph1, graph2;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_g1, scalar_g1_out, scalar_g2, scalar_g2_out;
    uint8_t output_g1 = 0, output_g2 = 0;
    uint32_t num_streams_g1 = 0, golden_output_g1 = 0, num_streams_g2 = 0, golden_output_g2 = 0;
    vx_node n1, n2, n3, n4;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_g1 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g2 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g1_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g2_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarIntermediateNode(graph1, scalar_g1, scalar_g1_out), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n3 = tivxScalarSourceNode(graph2, scalar_g2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n4 = tivxScalarIntermediateNode(graph2, scalar_g2, scalar_g2_out), VX_TYPE_NODE);

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

    VX_CALL(vxVerifyGraph(graph1));

    VX_CALL(vxVerifyGraph(graph2));

    VX_CALL(vxStartGraphStreaming(graph1));

    VX_CALL(vxStartGraphStreaming(graph2));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph1));

    VX_CALL(vxStopGraphStreaming(graph2));

    /* Beginning of error checking */
    VX_CALL(vxQueryGraph(graph1, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams_g1, sizeof(num_streams_g1)));

    ASSERT(num_streams_g1 != 0);

    golden_output_g1 = num_streams_g1 % 256;

    output_g1 = ct_scalar_as_int(scalar_g1_out);

    VX_CALL(vxQueryGraph(graph2, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams_g2, sizeof(num_streams_g2)));

    ASSERT(num_streams_g2 != 0);

    golden_output_g2 = num_streams_g2 % 256;

    output_g2 = ct_scalar_as_int(scalar_g2_out);

    if(output_g2!=golden_output_g2)
        printf("ERROR: %d != %d\n", output_g2, golden_output_g2);

    if(output_g1!=golden_output_g1)
        printf("ERROR: %d != %d\n", output_g2, golden_output_g2);
    /* End of error checking */

    VX_CALL(vxReleaseScalar(&scalar_g1));
    VX_CALL(vxReleaseScalar(&scalar_g2));
    VX_CALL(vxReleaseScalar(&scalar_g1_out));
    VX_CALL(vxReleaseScalar(&scalar_g2_out));
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
 *       n1         scalar             n2             scalar_out
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR
 *
 * Graph2
 *       n1         scalar             n2             scalar_out
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR
 *
 * Tests multiple graphs with streaming enabled
 * Node1 of graph1 is on MCU2_0 while node2 of graph1 is on MCU2_1
 * Node1 of graph2 is on MCU2_0 while node2 of graph2 is on MCU2_1
 *
 */
#if !defined(SOC_AM62A)
TEST_WITH_ARG(tivxGraphStreaming, testMultiGraph3, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph1, graph2;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_g1, scalar_g1_out, scalar_g2, scalar_g2_out;
    uint8_t output_g1 = 0, output_g2 = 0;
    uint32_t num_streams_g1 = 0, golden_output_g1 = 0, num_streams_g2 = 0, golden_output_g2 = 0;
    vx_node n1, n2, n3, n4;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_g1 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g2 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g1_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g2_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarIntermediateNode(graph1, scalar_g1, scalar_g1_out), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n3 = tivxScalarSourceNode(graph2, scalar_g2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n4 = tivxScalarIntermediateNode(graph2, scalar_g2, scalar_g2_out), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n4, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph1, n1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph2, n3));

    VX_CALL(vxVerifyGraph(graph1));

    VX_CALL(vxVerifyGraph(graph2));

    VX_CALL(vxStartGraphStreaming(graph1));

    VX_CALL(vxStartGraphStreaming(graph2));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph1));

    VX_CALL(vxStopGraphStreaming(graph2));

    /* Beginning of error checking */
    VX_CALL(vxQueryGraph(graph1, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams_g1, sizeof(num_streams_g1)));

    ASSERT(num_streams_g1 != 0);

    golden_output_g1 = num_streams_g1 % 256;

    output_g1 = ct_scalar_as_int(scalar_g1_out);

    VX_CALL(vxQueryGraph(graph2, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams_g2, sizeof(num_streams_g2)));

    ASSERT(num_streams_g2 != 0);

    golden_output_g2 = num_streams_g2 % 256;

    output_g2 = ct_scalar_as_int(scalar_g2_out);

    if(output_g2!=golden_output_g2)
        printf("ERROR: %d != %d\n", output_g2, golden_output_g2);

    if(output_g1!=golden_output_g1)
        printf("ERROR: %d != %d\n", output_g2, golden_output_g2);
    /* End of error checking */

    VX_CALL(vxReleaseScalar(&scalar_g1));
    VX_CALL(vxReleaseScalar(&scalar_g2));
    VX_CALL(vxReleaseScalar(&scalar_g1_out));
    VX_CALL(vxReleaseScalar(&scalar_g2_out));
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
 * SCALAR_SOURCE -- SCALAR -- SCALAR_SINK
 *
 * Graph2
 *       n1         scalar         n2
 * SCALAR_SOURCE -- SCALAR -- SCALAR_SINK
 *
 * Tests multiple graphs with streaming and pipelining enabled
 * Both nodes of graph1 are on MCU2_0
 * Both nodes of graph2 are on MCU2_1
 *
 */
#if !defined(SOC_AM62A)
TEST_WITH_ARG(tivxGraphStreaming, testMultiGraphPipelined1, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph1, graph2;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_g1, scalar_g2;
    vx_node n1, n2, n3, n4;
    uint32_t pipeline_depth, num_buf;
    uint32_t num_streams_g1 = 0, num_streams_g2 = 0;

    pipeline_depth = 2;
    num_buf = 2;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_g1 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g2 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkNode(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n3 = tivxScalarSourceNode(graph2, scalar_g2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n4 = tivxScalarSinkNode(graph2, scalar_g2), VX_TYPE_NODE);

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

    export_graph_to_file(graph1, "test_multi_graph1_pipeline_streaming_graph1");
    log_graph_rt_trace(graph1);

    VX_CALL(vxVerifyGraph(graph2));

    export_graph_to_file(graph2, "test_multi_graph1_pipeline_streaming_graph2");
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
 * SCALAR_SOURCE -- SCALAR -- SCALAR_SINK
 *
 * Graph2
 *       n1         scalar         n2
 * SCALAR_SOURCE -- SCALAR -- SCALAR_SINK
 *
 * Tests multiple graphs with streaming and pipelining enabled
 * Both nodes of graph1 are on MCU2_0
 * Both nodes of graph2 are on MCU2_0
 *
 */
TEST_WITH_ARG(tivxGraphStreaming, testMultiGraphPipelined2, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph1, graph2;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_g1, scalar_g2;
    vx_node n1, n2, n3, n4;
    uint32_t pipeline_depth, num_buf;
    uint32_t num_streams_g1 = 0, num_streams_g2 = 0;

    pipeline_depth = 2;
    num_buf = 2;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_g1 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g2 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkNode(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n3 = tivxScalarSourceNode(graph2, scalar_g2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n4 = tivxScalarSinkNode(graph2, scalar_g2), VX_TYPE_NODE);

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

    export_graph_to_file(graph1, "test_multi_graph2_pipeline_streaming_graph1");
    log_graph_rt_trace(graph1);

    VX_CALL(vxVerifyGraph(graph2));

    export_graph_to_file(graph2, "test_multi_graph2_pipeline_streaming_graph2");
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
 * SCALAR_SOURCE -- SCALAR -- SCALAR_SINK
 *
 * Graph2
 *       n1         scalar         n2
 * SCALAR_SOURCE -- SCALAR -- SCALAR_SINK
 *
 * Tests multiple graphs with streaming and pipelining enabled
 * Node1 of graph1 is on MCU2_0 while node2 of graph1 is on MCU2_1
 * Node1 of graph2 is on MCU2_0 while node2 of graph2 is on MCU2_1
 *
 */
#if !defined(SOC_AM62A)
TEST_WITH_ARG(tivxGraphStreaming, testMultiGraphPipelined3, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph1, graph2;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_g1, scalar_g2;
    vx_node n1, n2, n3, n4;
    uint32_t pipeline_depth, num_buf;
    uint32_t num_streams_g1 = 0, num_streams_g2 = 0;

    pipeline_depth = 2;
    num_buf = 2;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_g1 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g2 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkNode(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n3 = tivxScalarSourceNode(graph2, scalar_g2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n4 = tivxScalarSinkNode(graph2, scalar_g2), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
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

    export_graph_to_file(graph1, "test_multi_graph3_pipeline_streaming_graph1");
    log_graph_rt_trace(graph1);

    VX_CALL(vxVerifyGraph(graph2));

    export_graph_to_file(graph2, "test_multi_graph3_pipeline_streaming_graph2");
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
 * SCALAR_SOURCE -- SCALAR -- SCALAR_SINK
 *
 * Graph2
 *       n1         scalar             n2            scalar         n3
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR -- SCALAR_SINK
 *
 * Tests multiple graphs with streaming and pipelining enabled
 * Node1 of graph1 is on MCU2_0 while node2 of graph1 is on MCU2_1
 * Node1 of graph2 is on MCU2_0 while node2 and node3 of graph2 are on MCU2_1
 *
 */
#if !defined(SOC_AM62A)
TEST_WITH_ARG(tivxGraphStreaming, testMultiGraphPipelined4, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph1, graph2;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_g1, scalar_g2, scalar_out_g2;
    vx_node n1, n2, n3, n4, n5;
    uint32_t pipeline_depth, num_buf;
    uint32_t num_streams_g1 = 0, num_streams_g2 = 0;

    pipeline_depth = 2;
    num_buf = 2;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_g1 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_g2 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out_g2 = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkNode(graph1, scalar_g1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n3 = tivxScalarSourceNode(graph2, scalar_g2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n4 = tivxScalarIntermediateNode(graph2, scalar_g2, scalar_out_g2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n5 = tivxScalarSinkNode(graph2, scalar_out_g2), VX_TYPE_NODE);

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

    VX_CALL(vxVerifyGraph(graph1));

    export_graph_to_file(graph1, "test_multi_graph4_pipeline_streaming_graph1");
    log_graph_rt_trace(graph1);

    VX_CALL(vxVerifyGraph(graph2));

    export_graph_to_file(graph2, "test_multi_graph4_pipeline_streaming_graph2");
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
 *       n1         scalar             n2            scalar
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR
 *
 * Tests starting and stopping capabilities of streaming
 *
 */
TEST_WITH_ARG(tivxGraphStreaming, testStreamStartStop, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar, scalar_out;
    uint8_t output = 0;
    uint32_t num_streams = 0, golden_output = 0;
    vx_node n1, n2;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n1 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarIntermediateNode(graph, scalar, scalar_out), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n1));

    VX_CALL(vxVerifyGraph(graph));

    /* First start and stop of graph */

    VX_CALL(vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams, sizeof(num_streams)));

    ASSERT(num_streams != 0);

    golden_output = num_streams % 256;

    output = ct_scalar_as_int(scalar_out);

    if(output!=golden_output)
        printf("ERROR: %d != %d\n", output, golden_output);

    /* Second start and stop of graph */

    VX_CALL(vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams, sizeof(num_streams)));

    ASSERT(num_streams != 0);

    golden_output = num_streams % 256;

    output = ct_scalar_as_int(scalar_out);

    if(output!=golden_output)
        printf("ERROR: %d != %d\n", output, golden_output);

    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

/*
 *       n1         scalar         n2
 * SCALAR_SOURCE -- SCALAR -- SCALAR_SINK
 *
 * Scalar source node connected to scalar sink node with streaming and pipelining enabled
 * Both nodes on MCU2_0
 * Error will be shown in a print statement if the scalar sink fails
 *
 */
TEST_WITH_ARG(tivxGraphStreaming, testPipeliningStreaming1, Arg, STREAMING_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n0, n1;

    uint32_t pipeline_depth, num_buf;
    uint64_t exe_time;
    uint32_t num_streams = 0;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar;

    tivxTestKernelsLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);

    pipeline_depth = 2;
    num_buf = 2;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n0 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n1 = tivxScalarSinkNode(graph, scalar), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    /* explicitly set graph pipeline depth */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, pipeline_depth));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n0, 0, num_buf));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n0));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_streaming1");
    log_graph_rt_trace(graph);

    exe_time = tivxPlatformGetTimeInUsecs();

    VX_CALL(vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams, sizeof(num_streams)));

    ASSERT(num_streams != 0);

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/*
 *       n0         scalar         n1
 * SCALAR_SOURCE -- SCALAR -- SCALAR_SINK
 *
 * Scalar source node connected to scalar sink node with streaming and pipelining enabled
 * N0 is on MCU2_0 while N1 is on MCU2_1
 * Error will be shown in a print statement if the scalar sink fails
 *
 */
#if !defined(SOC_AM62A)
TEST_WITH_ARG(tivxGraphStreaming, testPipeliningStreaming2, Arg, STREAMING_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n0, n1;

    uint32_t pipeline_depth, num_buf;
    uint64_t exe_time;
    uint32_t num_streams = 0;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar;

    tivxTestKernelsLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);

    pipeline_depth = 2;
    num_buf = 2;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n0 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n1 = tivxScalarSinkNode(graph, scalar), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_1));

    /* explicitly set graph pipeline depth */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, pipeline_depth));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n0, 0, num_buf));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n0));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_streaming2");
    log_graph_rt_trace(graph);

    exe_time = tivxPlatformGetTimeInUsecs();

    VX_CALL(vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph));

    VX_CALL(vxQueryGraph(graph, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams, sizeof(num_streams)));

    ASSERT(num_streams != 0);

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}
#endif
/*
 *       n0         scalar             n1            scalar         n2
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR -- SCALAR_SINK
 *
 * Scalar source node connected to scalar sink node with streaming and pipelining enabled
 * All nodes are on MCU2_0
 * Error will be shown in a print statement if the scalar sink fails
 *
 */
TEST_WITH_ARG(tivxGraphStreaming, testPipeliningStreaming3, Pipeline_Arg, PARAMETERS)
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

    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n0 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n1 = tivxScalarIntermediateNode(graph, scalar, scalar_out), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkNode(graph, scalar_out), VX_TYPE_NODE);

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

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n0));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_streaming3");
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
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR -- SCALAR_SINK
 *
 * Scalar source node connected to scalar sink node with streaming and pipelining enabled
 * Trigger node is intermediate node
 * All nodes are on MCU2_0
 * Error will be shown in a print statement if the scalar sink fails
 *
 */
TEST_WITH_ARG(tivxGraphStreaming, testPipeliningStreaming4, Pipeline_Arg, PARAMETERS)
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

    pipeline_depth = arg_->pipe_depth;
    num_buf = arg_->num_buf;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n0 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n1 = tivxScalarIntermediateNode(graph, scalar, scalar_out), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkNode(graph, scalar_out), VX_TYPE_NODE);

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

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_streaming4");
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
 * SCALAR_SOURCE -- SCALAR -- SCALAR_INTERMEDIATE -- SCALAR -- SCALAR_SINK
 *
 * Scalar source node connected to scalar sink node with streaming and pipelining enabled
 * Trigger node is sink node
 * All nodes are on MCU2_0
 * Error will be shown in a print statement if the scalar sink fails
 *
 */
TEST_WITH_ARG(tivxGraphStreaming, testPipeliningStreaming5, Pipeline_Arg, PARAMETERS)
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

    /* Needs to be at least 2 since trigger node is the third node */
    if (arg_->pipe_depth == 1)
    {
        pipeline_depth = 2;
    }
    else
    {
        pipeline_depth = arg_->pipe_depth;
    }

    if (arg_->num_buf == 1)
    {
        num_buf = 2;
    }
    else
    {
        num_buf = arg_->num_buf;
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n0 = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n1 = tivxScalarIntermediateNode(graph, scalar, scalar_out), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(n2 = tivxScalarSinkNode(graph, scalar_out), VX_TYPE_NODE);

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

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, n2));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_graph_pipeline_streaming5");
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

TEST(tivxGraphStreaming, negativeTestEnableGraphStreaming)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_kernel kernel = NULL;
    vx_enum kernel_id = VX_KERNEL_SOBEL_3x3;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxEnableGraphStreaming(graph, node));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, kernel_id), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxEnableGraphStreaming(graph, node));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxGraphStreaming, negativeTestStartGraphStreaming)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_kernel kernel = NULL;
    vx_enum kernel_id = VX_KERNEL_SOBEL_3x3;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxStartGraphStreaming(graph));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, kernel_id), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxEnableGraphStreaming(graph, node));
    ASSERT_EQ_VX_STATUS(VX_FAILURE, vxStartGraphStreaming(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxGraphStreaming, negativeTestStopGraphStreaming)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_node node = NULL;
    vx_kernel kernel = NULL;
    vx_enum kernel_id = VX_KERNEL_SOBEL_3x3;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxStopGraphStreaming(graph));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, kernel_id), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxEnableGraphStreaming(graph, node));
    ASSERT_EQ_VX_STATUS(VX_FAILURE, vxStartGraphStreaming(graph));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxStopGraphStreaming(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxGraphStreaming, negativeTestVerifyGraph)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxVerifyGraph(graph));
}

TESTCASE_TESTS(tivxGraphStreaming,
               negativeTestSourceSinkNoTrigger,
               testSourceSink1,
#if !defined(SOC_AM62A)
               testSourceSink2,
#endif
               testStreamStartStop,
#if !defined(SOC_AM62A)
               testMultiGraph1,
#endif
               testMultiGraph2,
#if !defined(SOC_AM62A)
               testMultiGraph3,
               testMultiGraphPipelined1,
#endif
               testMultiGraphPipelined2,
#if !defined(SOC_AM62A)
               testMultiGraphPipelined3,
               testMultiGraphPipelined4,
#endif
               testPipeliningStreaming1,
#if !defined(SOC_AM62A)
               testPipeliningStreaming2,
#endif
               testPipeliningStreaming3,
               testPipeliningStreaming4,
               testPipeliningStreaming5,
               testScalar,
               testScalarCtrlCmd,
               negativeTestStreamingState,
               negativeTestScalar,
               negativeTestStreamingPipelining1,
               negativeTestStreamingPipelining2,
               negativeTestStreamingError,
               negativeTestPipeliningStreamingNoTrigger,
               negativeTestEnableGraphStreaming,
               negativeTestStartGraphStreaming,
               negativeTestStopGraphStreaming,
               negativeTestVerifyGraph
)

