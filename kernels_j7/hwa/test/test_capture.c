/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <VX/vx.h>
#include <TI/tivx.h>
#include <TI/j7.h>
#include "test_engine/test.h"
#include <TI/tivx_config.h>
#include <string.h>
#include "tivx_utils_file_rd_wr.h"
#include <TI/tivx_task.h>
#include "math.h"
#include <limits.h>
#include "test_tiovx/test_tiovx.h"
#include "tivx_utils_file_rd_wr.h"

#include <utils/sensors/include/app_sensors.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/ipc/include/app_ipc.h>


#define MAX_NUM_BUF         (8u)
#define MAX_ABS_FILENAME    (1024u)

#define NUM_CHANNELS        (4U)

#define IMAGE_WIDTH         (1936)
#define IMAGE_HEIGHT        (1100)
#define IMAGE_FORMAT        (VX_DF_IMAGE_U16)

static const vx_char user_data_object_name[] = "tivx_capture_params_t";

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
 * Utility API used to add a graph parameter from a node, node parameter index
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
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
 * Utility API to log graph run-time trace
 */
static vx_status log_graph_rt_trace(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    #if LOG_RT_TRACE_ENABLE
    status = tivxLogRtTrace(graph);
    #endif
    return status;
}

TESTCASE(tivxHwaCapture, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    int loop_cnt;
    int measure_perf;
} Arg_Capture;

#define CAPTURE_PARAMETERS \
    CT_GENERATE_PARAMETERS("capture", ARG, 1000, 0)

TEST_WITH_ARG(tivxHwaCapture, testGraphProcessing, Arg_Capture, CAPTURE_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n0;
    vx_object_array capture_frames[MAX_NUM_BUF];
    vx_user_data_object capture_config;
    tivx_capture_params_t local_capture_config;
    vx_image img_exemplar;
    uint32_t width = IMAGE_WIDTH, height = IMAGE_HEIGHT;
    uint32_t objarr_idx, num_capture_frames = NUM_CHANNELS;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, loopCnt, frameIdx;
    CT_Image tst_img;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    uint64_t exe_time;
    char filename[MAX_ABS_FILENAME];
    AppSensorCmdParams cmdPrms;
    vx_reference refs[1];
    vx_user_data_object capture_stats_obj;
    tivx_capture_status_t *capture_stats_struct;
    vx_map_id capture_stats_map_id;
    uint32_t *data_ptr;
    uint8_t i;

    /* Setting to num buf of capture node */
    num_buf = 3;
    loop_cnt = arg_->loop_cnt;

    tivxHwaLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Hardcoding since reading from sample image */
    ASSERT_VX_OBJECT(img_exemplar = vxCreateImage(context, width, height, IMAGE_FORMAT), VX_TYPE_IMAGE);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)img_exemplar, num_capture_frames), VX_TYPE_OBJECT_ARRAY);
    }

    /* Config initialization */
    tivx_capture_params_init(&local_capture_config);
    local_capture_config.enableCsiv2p0Support = (uint32_t)vx_true_e;
    local_capture_config.numDataLanes = 4U;
    for (loopCnt = 0U ;
         loopCnt < local_capture_config.numDataLanes ;
         loopCnt++)
    {
        local_capture_config.dataLanesMap[loopCnt] = (loopCnt + 1u);
    }
    for (loopCnt = 0U; loopCnt < NUM_CHANNELS; loopCnt++)
    {
        local_capture_config.vcNum[loopCnt] = loopCnt;
    }

    ASSERT_VX_OBJECT(capture_config = vxCreateUserDataObject(context, user_data_object_name, sizeof(tivx_capture_params_t), &local_capture_config), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(n0 = tivxCaptureNode(graph, capture_config, capture_frames[0]), VX_TYPE_NODE);

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&capture_frames[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            1,
            graph_parameters_queue_params_list
            );


    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs();

    cmdPrms.numSensors = num_capture_frames;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, appRemoteServiceRun(APP_IPC_CPU_MCU2_1,
        APP_REMOTE_SERVICE_SENSOR_NAME,
        APP_REMOTE_SERVICE_SENSOR_CMD_CONFIG_IMX390, &cmdPrms, sizeof(cmdPrms), 0));

    /* enqueue buf for pipeup but dont trigger graph execution */
    for(buf_id=0; buf_id<num_buf-1; buf_id++)
    {
        tivxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_frames[buf_id], 1, TIVX_GRAPH_PARAMETER_ENQUEUE_FLAG_PIPEUP);
    }

    /* after pipeup, now enqueue a buffer to trigger graph scheduling */
    vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_frames[buf_id], 1);

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array out_capture_frames;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);

    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    capture_stats_obj =
        vxCreateUserDataObject(context, "tivx_capture_status_t" ,
        sizeof(tivx_capture_status_t), NULL);

    refs[0] = (vx_reference)capture_stats_obj;
    tivxNodeSendCommand(n0, 0,
        TIVX_CAPTURE_GET_STATISTICS, refs, 1u);

    vxMapUserDataObject(
            (vx_user_data_object)refs[0],
            0,
            sizeof(tivx_capture_status_t),
            &capture_stats_map_id,
            (void **)&data_ptr,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

    capture_stats_struct = (tivx_capture_status_t*)data_ptr;

    printf("\n\r==========================================================\r\n");
    printf(": Capture Status:\r\n");
    printf("==========================================================\r\n");
    printf(": FIFO Overflow Count: %d\r\n",
              capture_stats_struct->overflowCount);
    printf(": Spurious UDMA interrupt count: %d\r\n",
              capture_stats_struct->spuriousUdmaIntrCount);
    printf("  [Channel No] | Frame Queue Count |"
        " Frame De-queue Count | Frame Drop Count |\n");
    for(i = 0U ; i < num_capture_frames ; i ++)
    {
        printf("\t\t%d|\t\t%d|\t\t%d|\t\t%d|\n",
              i,
              capture_stats_struct->queueCount[i],
              capture_stats_struct->dequeueCount[i],
              capture_stats_struct->dropCount[i]);
    }
    vxUnmapUserDataObject((vx_user_data_object)refs[0], capture_stats_map_id);

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseGraph(&graph));

    VX_CALL(vxReleaseImage(&img_exemplar));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&capture_frames[buf_id]));
    }
    VX_CALL(vxReleaseUserDataObject(&capture_config));
    VX_CALL(vxReleaseUserDataObject(&capture_stats_obj));

    tivxHwaUnLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

TEST_WITH_ARG(tivxHwaCapture, testRawImageCapture, Arg_Capture, CAPTURE_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n0;
    vx_object_array capture_frames[MAX_NUM_BUF];
    vx_user_data_object capture_config;
    tivx_capture_params_t local_capture_config;
    tivx_raw_image raw_image = 0;
    uint32_t width = IMAGE_WIDTH, height = IMAGE_HEIGHT, i;
    uint32_t objarr_idx, num_capture_frames = NUM_CHANNELS;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, loopCnt, frameIdx;
    CT_Image tst_img;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    uint64_t exe_time;
    char filename[MAX_ABS_FILENAME];
    tivx_raw_image_create_params_t params;
    vx_reference refs[1];
    vx_user_data_object capture_stats_obj;
    tivx_capture_status_t *capture_stats_struct;
    vx_map_id capture_stats_map_id;
    uint32_t *data_ptr;

    tivx_raw_image out_img;

    vx_imagepatch_addressing_t addr;
    vx_rectangle_t rect;

    uint16_t img[1];

    AppSensorCmdParams cmdPrms;

    /* Setting to num buf of capture node */
    num_buf = 3;
    loop_cnt = arg_->loop_cnt;

    tivxHwaLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    params.width = width;
    params.height = height;
    params.num_exposures = 1;
    params.line_interleaved = vx_true_e;
    params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    params.format[0].msb = 12;
    params.meta_height_before = 0;
    params.meta_height_after = 0;

    ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(context, &params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)raw_image, num_capture_frames), VX_TYPE_OBJECT_ARRAY);
    }

    /* Config initialization */
    tivx_capture_params_init(&local_capture_config);
    local_capture_config.enableCsiv2p0Support = (uint32_t)vx_true_e;
    local_capture_config.numDataLanes = 4U;
    for (loopCnt = 0U ;
         loopCnt < local_capture_config.numDataLanes ;
         loopCnt++)
    {
        local_capture_config.dataLanesMap[loopCnt] = (loopCnt + 1u);
    }
    for (loopCnt = 0U; loopCnt < NUM_CHANNELS; loopCnt++)
    {
        local_capture_config.vcNum[loopCnt] = loopCnt;
    }

    ASSERT_VX_OBJECT(capture_config = vxCreateUserDataObject(context, user_data_object_name, sizeof(tivx_capture_params_t), &local_capture_config), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(n0 = tivxCaptureNode(graph, capture_config, capture_frames[0]), VX_TYPE_NODE);

    /* input @ node index 0, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n0, 1);

    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&capture_frames[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            1,
            graph_parameters_queue_params_list
            );


    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    exe_time = tivxPlatformGetTimeInUsecs();

    cmdPrms.numSensors = num_capture_frames;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, appRemoteServiceRun(APP_IPC_CPU_MCU2_1,
        APP_REMOTE_SERVICE_SENSOR_NAME,
        APP_REMOTE_SERVICE_SENSOR_CMD_CONFIG_IMX390, &cmdPrms, sizeof(cmdPrms), 0));

    /* enqueue buf for pipeup but dont trigger graph execution */
    for(buf_id=0; buf_id<num_buf-1; buf_id++)
    {
        tivxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_frames[buf_id], 1, TIVX_GRAPH_PARAMETER_ENQUEUE_FLAG_PIPEUP);
    }

    /* after pipeup, now enqueue a buffer to trigger graph scheduling */
    vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_frames[buf_id], 1);

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array out_capture_frames;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);

    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    capture_stats_obj =
        vxCreateUserDataObject(context, "tivx_capture_status_t" ,
        sizeof(tivx_capture_status_t), NULL);

    refs[0] = (vx_reference)capture_stats_obj;
    tivxNodeSendCommand(n0, 0,
        TIVX_CAPTURE_GET_STATISTICS, refs, 1u);

    vxMapUserDataObject(
            (vx_user_data_object)refs[0],
            0,
            sizeof(tivx_capture_status_t),
            &capture_stats_map_id,
            (void **)&data_ptr,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

    capture_stats_struct = (tivx_capture_status_t*)data_ptr;

    printf("\n\r==========================================================\r\n");
    printf(": Capture Status:\r\n");
    printf("==========================================================\r\n");
    printf(": FIFO Overflow Count: %d\r\n",
              capture_stats_struct->overflowCount);
    printf(": Spurious UDMA interrupt count: %d\r\n",
              capture_stats_struct->spuriousUdmaIntrCount);
    printf("  [Channel No] | Frame Queue Count |"
        " Frame De-queue Count | Frame Drop Count |\n");
    for(i = 0U ; i < num_capture_frames ; i ++)
    {
        printf("\t\t%d|\t\t%d|\t\t%d|\t\t%d|\n",
              i,
              capture_stats_struct->queueCount[i],
              capture_stats_struct->dequeueCount[i],
              capture_stats_struct->dropCount[i]);
    }
    vxUnmapUserDataObject((vx_user_data_object)refs[0], capture_stats_map_id);

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseGraph(&graph));

    VX_CALL(tivxReleaseRawImage(&raw_image));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&capture_frames[buf_id]));
    }
    VX_CALL(vxReleaseUserDataObject(&capture_config));
    VX_CALL(vxReleaseUserDataObject(&capture_stats_obj));

    tivxHwaUnLoadKernels(context);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}


TESTCASE_TESTS(tivxHwaCapture,
               testRawImageCapture
               /* testGraphProcessing */)

