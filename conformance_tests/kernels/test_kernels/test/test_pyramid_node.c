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


TESTCASE(tivxPyramidNode,  CT_VXContext, ct_setup_vx_context, 0)

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
    CT_GENERATE_PARAMETERS("streaming", ARG, 2), \

TEST(tivxPyramidNode, testIntermediateNodeCreation)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node n1;
    vx_pyramid pyr_in, pyr_out;
    uint32_t width, height;

    width = 16;
    height = 16;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(pyr_in = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_VX_OBJECT(pyr_out = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_VX_OBJECT(n1 = tivxPyramidIntermediateNode(graph, pyr_in, pyr_out), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleasePyramid(&pyr_in));
    VX_CALL(vxReleasePyramid(&pyr_out));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

TEST(tivxPyramidNode, testSourceNodeCreation)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node n1;
    vx_pyramid pyr_out;
    uint32_t width, height;

    width = 16;
    height = 16;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(pyr_out = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_VX_OBJECT(n1 = tivxPyramidSourceNode(graph, pyr_out), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleasePyramid(&pyr_out));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

#define NUM_REPLICAS 4

/* TIOVX-711 */
TEST_WITH_ARG(tivxPyramidNode, testObjectArrayPyramidPipeline, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node n1, n2;
    vx_pyramid pyr_1[MAX_NUM_BUF], pyr_2[MAX_NUM_BUF], pyr_exemplar;
    vx_object_array pyr_1_arr[MAX_NUM_BUF], pyr_2_arr[MAX_NUM_BUF];
    uint32_t width, height, i, j, k, val;
    uint32_t pipeline_depth, loop_cnt, num_buf, reference_val = 1;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_bool pyr_source_replicate[] =
        {vx_true_e};
    vx_bool pyr_intermediate_replicate[] =
        {vx_true_e, vx_true_e};
    width = 16;
    height = 16;

    pipeline_depth = 3;
    num_buf = 3;
    loop_cnt = arg_->stream_time;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(pyr_exemplar = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    for (i = 0; i < num_buf; i++)
    {
        ASSERT_VX_OBJECT(pyr_1_arr[i] = vxCreateObjectArray(context, (vx_reference)pyr_exemplar, NUM_REPLICAS), VX_TYPE_OBJECT_ARRAY);

        ASSERT_VX_OBJECT(pyr_1[i] = (vx_pyramid)vxGetObjectArrayItem(pyr_1_arr[i], 0), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(pyr_2_arr[i] = vxCreateObjectArray(context, (vx_reference)pyr_exemplar, NUM_REPLICAS), VX_TYPE_OBJECT_ARRAY);

        ASSERT_VX_OBJECT(pyr_2[i] = (vx_pyramid)vxGetObjectArrayItem(pyr_2_arr[i], 0), VX_TYPE_PYRAMID);
    }

    VX_CALL(vxReleasePyramid(&pyr_exemplar));

    ASSERT_VX_OBJECT(n1 = tivxPyramidSourceNode(graph, pyr_1[0]), VX_TYPE_NODE);
    vxReplicateNode(graph, n1, pyr_source_replicate, 1u);
    ASSERT_VX_OBJECT(n2 = tivxPyramidIntermediateNode(graph, pyr_1[0], pyr_2[0]), VX_TYPE_NODE);
    vxReplicateNode(graph, n2, pyr_intermediate_replicate, 2u);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 0);
    add_graph_parameter_by_node_index(graph, n2, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&pyr_1[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&pyr_2[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            2,
            graph_parameters_queue_params_list
            );

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, pipeline_depth));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_obj_arr_pyr_pipeline");

    /* enqueue buf for pipeup but dont trigger graph execution */
    for(i=0; i<num_buf; i++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&pyr_1[i], 1);
        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&pyr_2[i], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(i=0; i<(loop_cnt+num_buf); i++)
    {
        uint32_t num_refs;
        vx_size levels, num_items;
        vx_pyramid out_pyr, out_pyr_2;
        vx_object_array out_objarr;
        CT_Image test_image;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_pyr, 1, &num_refs);
        vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_pyr_2, 1, &num_refs);

        ASSERT_VX_OBJECT(out_objarr = (vx_object_array)tivxGetReferenceParent((vx_reference)out_pyr_2), VX_TYPE_OBJECT_ARRAY);

        VX_CALL(vxQueryObjectArray(out_objarr, VX_OBJECT_ARRAY_NUMITEMS, &num_items, sizeof(num_items)));

        VX_CALL(vxQueryPyramid(out_pyr_2, VX_PYRAMID_LEVELS, &levels, sizeof(levels)));
        if (NULL != out_objarr)
        {
            for (j = 0; j < num_items; j++)
            {
                vx_pyramid tmp_out_pyr;

                ASSERT_VX_OBJECT(tmp_out_pyr = (vx_pyramid)vxGetObjectArrayItem(out_objarr, j), VX_TYPE_PYRAMID);

                for (k = 0; k < levels; k++)
                {
                    CT_Image test_image;
                    vx_image vx_test_image = vxGetPyramidLevel(tmp_out_pyr, k);

                    test_image = ct_image_from_vx_image(vx_test_image);

                    val = test_image->data.y[0];

                    if (val != reference_val)
                    {
                        printf("val = %d\n", val);
                        printf("reference_val = %d\n", reference_val);
                        ASSERT(val == reference_val);
                    }

                    vxReleaseImage(&vx_test_image);
                }

                vxReleasePyramid(&tmp_out_pyr);
            }
        }

        if (255 == reference_val)
        {
            reference_val = 0;
        }
        else
        {
            reference_val++;
        }

        /* recycles dequeued input and output refs */
        /* input and output can be enqueued in any order */
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_pyr, 1);
        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_pyr_2, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    for (i = 0; i < num_buf; i++)
    {
        VX_CALL(vxReleaseObjectArray(&pyr_1_arr[i]));
        VX_CALL(vxReleasePyramid(&pyr_1[i]));
        VX_CALL(vxReleaseObjectArray(&pyr_2_arr[i]));
        VX_CALL(vxReleasePyramid(&pyr_2[i]));
    }
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

/* TIOVX-710 */
TEST_WITH_ARG(tivxPyramidNode, testDelayPyramidPipeline, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node n1, n2, n3, n4;
    vx_pyramid pyr_1[MAX_NUM_BUF], pyr_2, pyr_3[MAX_NUM_BUF];
    uint32_t width, height, i, j, val;
    uint32_t pipeline_depth, loop_cnt, num_buf, reference_val = 0;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_delay delay;

    width = 16;
    height = 16;

    pipeline_depth = 3;
    num_buf = 3;
    loop_cnt = arg_->stream_time;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for (i = 0; i < num_buf; i++)
    {
        ASSERT_VX_OBJECT(pyr_1[i] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(pyr_3[i] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    }

    ASSERT_VX_OBJECT(pyr_2 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)(pyr_2), 2), VX_TYPE_DELAY);

    ASSERT_VX_OBJECT(n1 = tivxPyramidSourceNode(graph, pyr_1[0]), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n2 = tivxPyramidIntermediateNode(graph, pyr_1[0],
                          (vx_pyramid)vxGetReferenceFromDelay(delay, -1)), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(n3 = tivxPyramidIntermediateNode(graph, (vx_pyramid)vxGetReferenceFromDelay(delay, 0),
                          pyr_3[0]), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));    
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 0);
    add_graph_parameter_by_node_index(graph, n3, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&pyr_1[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&pyr_3[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            2,
            graph_parameters_queue_params_list
            );

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, pipeline_depth));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n2, 1, num_buf));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_delay_pyr_pipeline");

    /* enqueue buf for pipeup but dont trigger graph execution */
    for(i=0; i<num_buf; i++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&pyr_1[i], 1);
        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&pyr_3[i], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(i=0; i<(loop_cnt+num_buf); i++)
    {
        uint32_t num_refs;
        vx_size levels;
        vx_pyramid out_pyr, out_pyr_2;
        CT_Image test_image;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_pyr, 1, &num_refs);

        vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_pyr_2, 1, &num_refs);

        VX_CALL(vxQueryPyramid(out_pyr_2, VX_PYRAMID_LEVELS, &levels, sizeof(levels)));

        for (j = 0; j < levels; j++)
        {
            vx_image vx_test_image = vxGetPyramidLevel(out_pyr_2, j);
            test_image = ct_image_from_vx_image(vx_test_image);

            val = test_image->data.y[0];

            /* Ignore the first val since it is a delay and the first value is garbage */
            if (i > 0)
            {
                ASSERT(val == reference_val);
            }

            vxReleaseImage(&vx_test_image);
        }

        if (255 == reference_val)
        {
            reference_val = 0;
        }
        else
        {
            reference_val++;
        }

        /* recycles dequeued input and output refs */
        /* input and output can be enqueued in any order */
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_pyr, 1);

        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_pyr_2, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    for (i = 0; i < num_buf; i++)
    {
        VX_CALL(vxReleasePyramid(&pyr_1[i]));
        VX_CALL(vxReleasePyramid(&pyr_3[i]));
    }
    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleasePyramid(&pyr_2));
    VX_CALL(vxReleaseNode(&n3));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

/* TIOVX-712 */
TEST_WITH_ARG(tivxPyramidNode, testDelayObjectArrayPyramidPipeline, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node n1, n2, n3, n4;
    vx_pyramid pyr_1[MAX_NUM_BUF], pyr_3[MAX_NUM_BUF], pyr_exemplar;
    uint32_t width, height, i, j, k, val;
    uint32_t pipeline_depth, loop_cnt, num_buf, reference_val = 0;
    vx_object_array pyr_1_arr[MAX_NUM_BUF], pyr_2_arr, pyr_3_arr[MAX_NUM_BUF];
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_delay delay;
    vx_bool pyr_source_replicate[] =
        {vx_true_e};
    vx_bool pyr_intermediate_replicate[] =
        {vx_true_e, vx_true_e};
    vx_object_array tmp_obj_arr_1;
    vx_object_array tmp_obj_arr_0;
    vx_pyramid tmp_pyr_1;
    vx_pyramid tmp_pyr_0;

    width = 16;
    height = 16;

    pipeline_depth = 3;
    num_buf = 3;
    loop_cnt = arg_->stream_time;

    tivxTestKernelsLoadKernels(context);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(pyr_exemplar = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    for (i = 0; i < num_buf; i++)
    {
        ASSERT_VX_OBJECT(pyr_1_arr[i] = vxCreateObjectArray(context, (vx_reference)pyr_exemplar, NUM_REPLICAS), VX_TYPE_OBJECT_ARRAY);

        ASSERT_VX_OBJECT(pyr_1[i] = (vx_pyramid)vxGetObjectArrayItem(pyr_1_arr[i], 0), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(pyr_3_arr[i] = vxCreateObjectArray(context, (vx_reference)pyr_exemplar, NUM_REPLICAS), VX_TYPE_OBJECT_ARRAY);

        ASSERT_VX_OBJECT(pyr_3[i] = (vx_pyramid)vxGetObjectArrayItem(pyr_3_arr[i], 0), VX_TYPE_PYRAMID);
    }

    ASSERT_VX_OBJECT(pyr_2_arr = vxCreateObjectArray(context, (vx_reference)pyr_exemplar, NUM_REPLICAS), VX_TYPE_OBJECT_ARRAY);

    VX_CALL(vxReleasePyramid(&pyr_exemplar));

    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)(pyr_2_arr), 2), VX_TYPE_DELAY);

    ASSERT_VX_OBJECT(tmp_obj_arr_1 = (vx_object_array)vxGetReferenceFromDelay(delay, -1), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(tmp_obj_arr_0 = (vx_object_array)vxGetReferenceFromDelay(delay, 0), VX_TYPE_OBJECT_ARRAY);

    ASSERT_VX_OBJECT(tmp_pyr_1 = (vx_pyramid)vxGetObjectArrayItem(tmp_obj_arr_1, 0), VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(tmp_pyr_0 = (vx_pyramid)vxGetObjectArrayItem(tmp_obj_arr_0, 0), VX_TYPE_PYRAMID);

    ASSERT_VX_OBJECT(n1 = tivxPyramidSourceNode(graph, pyr_1[0]), VX_TYPE_NODE);
    vxReplicateNode(graph, n1, pyr_source_replicate, 1u);
    ASSERT_VX_OBJECT(n2 = tivxPyramidIntermediateNode(graph, pyr_1[0],
                          tmp_pyr_1), VX_TYPE_NODE);
    vxReplicateNode(graph, n2, pyr_intermediate_replicate, 2u);
    ASSERT_VX_OBJECT(n3 = tivxPyramidIntermediateNode(graph, tmp_pyr_0,
                          pyr_3[0]), VX_TYPE_NODE);
    vxReplicateNode(graph, n3, pyr_intermediate_replicate, 2u);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n1, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n2, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    VX_CALL(vxSetNodeTarget(n3, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 0);
    add_graph_parameter_by_node_index(graph, n3, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&pyr_1[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&pyr_3[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            2,
            graph_parameters_queue_params_list
            );

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, pipeline_depth));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_num_buf_by_node_index(n2, 1, num_buf));

    VX_CALL(vxVerifyGraph(graph));

    export_graph_to_file(graph, "test_delay_obj_arr_pyr_pipeline");

    /* enqueue buf for pipeup but dont trigger graph execution */
    for(i=0; i<num_buf; i++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&pyr_1[i], 1);
        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&pyr_3[i], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(i=0; i<(loop_cnt+num_buf); i++)
    {
        uint32_t num_refs;
        vx_size levels, num_items;
        vx_pyramid out_pyr, out_pyr_2;
        vx_object_array out_objarr;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_pyr, 1, &num_refs);

        vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&out_pyr_2, 1, &num_refs);

        ASSERT_VX_OBJECT(out_objarr = (vx_object_array)tivxGetReferenceParent((vx_reference)out_pyr_2), VX_TYPE_OBJECT_ARRAY);

        VX_CALL(vxQueryObjectArray(out_objarr, VX_OBJECT_ARRAY_NUMITEMS, &num_items, sizeof(num_items)));

        VX_CALL(vxQueryPyramid(out_pyr_2, VX_PYRAMID_LEVELS, &levels, sizeof(levels)));

        if (out_objarr != NULL)
        {
            for (j = 0; j < num_items; j++)
            {
                vx_pyramid tmp_out_pyr;

                ASSERT_VX_OBJECT(tmp_out_pyr = (vx_pyramid)vxGetObjectArrayItem(out_objarr, j), VX_TYPE_PYRAMID);

                for (k = 0; k < levels; k++)
                {
                    CT_Image test_image;
                    vx_image vx_test_image = vxGetPyramidLevel(tmp_out_pyr, k);

                    test_image = ct_image_from_vx_image(vx_test_image);

                    val = test_image->data.y[0];

                    /* Ignore the first val since it is a delay and the first value is garbage */
                    if (i > 0)
                    {
                        ASSERT(val == reference_val);
                    }

                    vxReleaseImage(&vx_test_image);
                }

                vxReleasePyramid(&tmp_out_pyr);
            }
        }

        if (255 == reference_val)
        {
            reference_val = 0;
        }
        else
        {
            reference_val++;
        }

        /* recycles dequeued input and output refs */
        /* input and output can be enqueued in any order */
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_pyr, 1);

        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_pyr_2, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    for (i = 0; i < num_buf; i++)
    {
        VX_CALL(vxReleaseObjectArray(&pyr_1_arr[i]));
        VX_CALL(vxReleasePyramid(&pyr_1[i]));
        VX_CALL(vxReleaseObjectArray(&pyr_3_arr[i]));
        VX_CALL(vxReleasePyramid(&pyr_3[i]));
    }
    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleaseObjectArray(&pyr_2_arr));
    VX_CALL(vxReleasePyramid(&tmp_pyr_1));
    VX_CALL(vxReleasePyramid(&tmp_pyr_0));
    VX_CALL(vxReleaseNode(&n3));
    VX_CALL(vxReleaseNode(&n2));
    VX_CALL(vxReleaseNode(&n1));
    VX_CALL(vxReleaseGraph(&graph));
    tivxTestKernelsUnLoadKernels(context);
}

TESTCASE_TESTS(tivxPyramidNode,
               testIntermediateNodeCreation,
               testSourceNodeCreation,
               testObjectArrayPyramidPipeline,
               testDelayPyramidPipeline,
               testDelayObjectArrayPyramidPipeline)

