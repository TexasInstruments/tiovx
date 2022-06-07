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
#ifdef BUILD_CAPTURE

#include <VX/vx.h>
#include <TI/tivx.h>
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
#include "test_hwa_common.h"
#include <utils/iss/include/app_iss.h>

#define MAX_NUM_BUF         (8u)

#define NUM_CHANNELS        (4U)
#define CAPT_INST_ID        (0U)

#define CAPTURE_NODE_ERROR  (1U)

#define CHANNEL_SWITCH_FRAME_COUNT          (150u)

#define STREAMING_START_OFF  0
#define STREAMING_START_ON   1

#define ERROR_GENERATION_OFF  0
#define ERROR_GENERATION_ON   1

#define DISABLE_CH_0              0
#define DISABLE_CH_1              1
#define DISABLE_CH_2              2
#define DISABLE_CH_3              3
#define DISABLE_CH_2_3            4
#define DISABLE_ALL_CH            5
#define ENABLE_ALL_CH             6
#define DISABLE_REENABLE_ALL_CH   7

#define CAPTURE_TEST_FRAME_RATE   32000

static const vx_char user_data_object_name[] = "tivx_capture_params_t";

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

static uint32_t initSensorParams(uint32_t sensor_features_supported)
{
    uint32_t sensor_features_enabled = 0;

    if(ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE == (sensor_features_supported & ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE))
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE;
    }else
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_LINEAR_MODE;
    }

    if(ISS_SENSOR_FEATURE_MANUAL_EXPOSURE == (sensor_features_supported & ISS_SENSOR_FEATURE_MANUAL_EXPOSURE))
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_MANUAL_EXPOSURE;
    }

    if(ISS_SENSOR_FEATURE_MANUAL_GAIN == (sensor_features_supported & ISS_SENSOR_FEATURE_MANUAL_GAIN))
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_MANUAL_GAIN;
    }

    if(ISS_SENSOR_FEATURE_DCC_SUPPORTED == (sensor_features_supported & ISS_SENSOR_FEATURE_DCC_SUPPORTED))
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_DCC_SUPPORTED;
    }

    return sensor_features_enabled;
}

TESTCASE(tivxHwaCapture, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    int loop_cnt;
    int raw_capture;
} Arg_Capture;

#define CAPTURE_PARAMETERS \
    CT_GENERATE_PARAMETERS("capture", ARG, 1000, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 1000, 1)

TEST_WITH_ARG(tivxHwaCapture, testRawImageCapture, Arg_Capture, CAPTURE_PARAMETERS)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n0;

    /* Data objects for graph */
    vx_object_array capture_frames[MAX_NUM_BUF];
    vx_user_data_object capture_config;
    tivx_raw_image raw_image = 0;
    vx_image image = 0;

    /* Local objects */
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    tivx_capture_params_t local_capture_config;
    uint32_t num_capture_channels = NUM_CHANNELS;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, instIdx, chIdx, out_num_refs;
    uint64_t exe_time, timestamp = 0, prev_timestamp = 0;
    vx_bool done;
    vx_object_array dequeue_capture_array;

    /* Image objects */
    vx_reference refs[1];
    vx_user_data_object capture_stats_obj;
    tivx_capture_statistics_t *capture_stats_struct;
    vx_map_id capture_stats_map_id;
    uint32_t *data_ptr;
    vx_imagepatch_addressing_t addr;
    vx_rectangle_t rect;

    /* Sensor Parameters */
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_uint8 num_sensors_found, count = 0;
    IssSensor_CreateParams sensorParams;
    char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];
    char *sensor_name;
    uint32_t sensor_features_enabled = 0, sensor_features_supported = 0;

    /* Setting to num buf of capture node */
    num_buf = 3;
    loop_cnt = arg_->loop_cnt;

    /* Init for test case */
    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Imaging Initialization */
    {
        memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
        for(count=0;count<ISS_SENSORS_MAX_SUPPORTED_SENSOR;count++)
        {
            sensor_list[count] = availableSensorNames[count];
        }
        VX_CALL(appEnumerateImageSensor(sensor_list, &num_sensors_found));

        sensor_name = sensor_list[0]; /* Note: hardcoding to IMX390 */

        memset(&sensorParams, 0, sizeof(sensorParams));
        VX_CALL(appQueryImageSensor(sensor_name, &sensorParams));

        sensor_features_supported = sensorParams.sensorInfo.features;

        sensor_features_enabled = initSensorParams(sensor_features_supported);

        VX_CALL(appInitImageSensor(sensor_name, sensor_features_enabled, (1<<(num_capture_channels))-1)); /*Mask = 0xF for 4 cameras*/
    }

    /* Creating objects for graph */
    {
        if (0 == arg_->raw_capture)
        {
            ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(context, &(sensorParams.sensorInfo.raw_params)), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(image = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        }

        /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            if (0 == arg_->raw_capture)
            {
                ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)raw_image, num_capture_channels), VX_TYPE_OBJECT_ARRAY);
            }
            else
            {
                ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)image, num_capture_channels), VX_TYPE_OBJECT_ARRAY);
            }
        }

        /* Config initialization */
        tivx_capture_params_init(&local_capture_config);
        local_capture_config.numInst = 1;
        local_capture_config.numCh   = num_capture_channels;
        chIdx = 0U;
        for (instIdx = 0U ; instIdx < 1 ; instIdx++)
        {
            local_capture_config.instId[instIdx] = instIdx;
            local_capture_config.instCfg[instIdx].enableCsiv2p0Support = (uint32_t)vx_true_e;
            local_capture_config.instCfg[instIdx].numDataLanes         = 4U;
            for (loop_id = 0U; loop_id < local_capture_config.instCfg[0U].numDataLanes ; loop_id++)
            {
                local_capture_config.instCfg[instIdx].dataLanesMap[loop_id] = (loop_id + 1u);
            }
            for (loop_id = 0U; loop_id < num_capture_channels; loop_id++)
            {
                local_capture_config.chVcNum[chIdx]   = loop_id;
                local_capture_config.chInstMap[chIdx] = instIdx;
                chIdx++;
            }
        }

        ASSERT_VX_OBJECT(capture_config = vxCreateUserDataObject(context, user_data_object_name, sizeof(tivx_capture_params_t), &local_capture_config), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(n0 = tivxCaptureNode(graph, capture_config, capture_frames[0]), VX_TYPE_NODE);
    }

    /* Pipelining and graph verification */
    {
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
    }


    /* iniitalizing sensor */
    VX_CALL(appStartImageSensor(sensor_name, (1<<(num_capture_channels))-1));/*Mask for 4 cameras*/

    exe_time = tivxPlatformGetTimeInUsecs();

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
        uint64_t timestamp0 = 0, timestamp1 = 0;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);

        if (0 == arg_->raw_capture)
        {
            tivx_raw_image element0, element1;

            ASSERT_VX_OBJECT(element0 = (tivx_raw_image) vxGetObjectArrayItem(out_capture_frames, 0), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

            ASSERT_VX_OBJECT(element1 = (tivx_raw_image) vxGetObjectArrayItem(out_capture_frames, 1), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

            VX_CALL(vxQueryReference((vx_reference)element0, TIVX_REFERENCE_TIMESTAMP, &timestamp0, sizeof(timestamp0)));
            VX_CALL(vxQueryReference((vx_reference)element1, TIVX_REFERENCE_TIMESTAMP, &timestamp1, sizeof(timestamp1)));

            ASSERT(timestamp0!=timestamp1);

            VX_CALL(vxQueryReference((vx_reference)element0, TIVX_REFERENCE_TIMESTAMP, &timestamp, sizeof(timestamp)));

            /* The 30 FPS sensor parameters of IMX390 is around 32.4 ms latency in capture */
            ASSERT(timestamp > (prev_timestamp+CAPTURE_TEST_FRAME_RATE));

            prev_timestamp = timestamp;

            VX_CALL(tivxReleaseRawImage(&element0));
            VX_CALL(tivxReleaseRawImage(&element1));
        }
        else
        {
            vx_image element0, element1;

            ASSERT_VX_OBJECT(element0 = (vx_image) vxGetObjectArrayItem(out_capture_frames, 0), VX_TYPE_IMAGE);

            ASSERT_VX_OBJECT(element1 = (vx_image) vxGetObjectArrayItem(out_capture_frames, 1), VX_TYPE_IMAGE);

            VX_CALL(vxQueryReference((vx_reference)element0, TIVX_REFERENCE_TIMESTAMP, &timestamp0, sizeof(timestamp0)));
            VX_CALL(vxQueryReference((vx_reference)element1, TIVX_REFERENCE_TIMESTAMP, &timestamp1, sizeof(timestamp1)));

            ASSERT(timestamp0!=timestamp1);

            VX_CALL(vxQueryReference((vx_reference)element0, TIVX_REFERENCE_TIMESTAMP, &timestamp, sizeof(timestamp)));

            /* The 30 FPS sensor parameters of IMX390 is around 32.4 ms latency in capture */
            /* TIOVX-995: Currently is around 32.5ms in broadcast mode */
            ASSERT(timestamp > (prev_timestamp+CAPTURE_TEST_FRAME_RATE));

            prev_timestamp = timestamp;

            VX_CALL(vxReleaseImage(&element0));
            VX_CALL(vxReleaseImage(&element1));
        }

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    /* Dequeue all buffers */
    done = vx_false_e;
    while(!done)
    {
        VX_CALL(vxGraphParameterCheckDoneRef(graph, 0, &out_num_refs));

        if(out_num_refs>0)
        {
            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&dequeue_capture_array, 1, &out_num_refs));
        }

        if(out_num_refs == 0)
        {
            done = vx_true_e;
        }
    }

    VX_CALL(appStopImageSensor(sensor_name, (1<<(num_capture_channels))-1)); /*Mask for 4 cameras*/

    /* Querying node for sensor stats */
    {
        capture_stats_obj =
            vxCreateUserDataObject(context, "tivx_capture_statistics_t" ,
            sizeof(tivx_capture_statistics_t), NULL);

        refs[0] = (vx_reference)capture_stats_obj;
        tivxNodeSendCommand(n0, 0,
            TIVX_CAPTURE_GET_STATISTICS, refs, 1u);

        vxMapUserDataObject(
                (vx_user_data_object)refs[0],
                0,
                sizeof(tivx_capture_statistics_t),
                &capture_stats_map_id,
                (void **)&data_ptr,
                VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            );

        capture_stats_struct = (tivx_capture_statistics_t*)data_ptr;

        /* As this is single instance app, array index to access status will be always '0U' */
        printf("\n\r==========================================================\r\n");
        printf(": Capture Status:\r\n");
        printf("==========================================================\r\n");
        printf(": FIFO Overflow Count: %d\r\n",
                  capture_stats_struct->overflowCount[0U]);
        printf(": Spurious UDMA interrupt count: %d\r\n",
                  capture_stats_struct->spuriousUdmaIntrCount[0U]);
        printf("  [Channel No] | Frame Queue Count |"
            " Frame De-queue Count | Frame Drop Count |\n");
        for(chIdx = 0U ; chIdx < num_capture_channels ; chIdx ++)
        {
            printf("\t\t%d|\t\t%d|\t\t%d|\t\t%d|\n",
                  chIdx,
                  capture_stats_struct->queueCount[0U][chIdx],
                  capture_stats_struct->dequeueCount[0U][chIdx],
                  capture_stats_struct->dropCount[0U][chIdx]);
        }
        vxUnmapUserDataObject((vx_user_data_object)refs[0], capture_stats_map_id);
    }

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseGraph(&graph));

    if (0 == arg_->raw_capture)
    {
        VX_CALL(tivxReleaseRawImage(&raw_image));
    }
    else
    {
        VX_CALL(vxReleaseImage(&image));
    }

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&capture_frames[buf_id]));
    }
    VX_CALL(vxReleaseUserDataObject(&capture_config));
    VX_CALL(vxReleaseUserDataObject(&capture_stats_obj));

    tivxHwaUnLoadKernels(context);

    appDeInitImageSensor(sensor_name);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

typedef struct {
    const char* name;
    int loop_cnt;
    int start_sensors;
    int generate_error;
    int camera_disable;
    int measure_perf;
} Arg_CaptureTimeout;

#define CAPTURE_TIMEOUT_PARAMETERS \
    CT_GENERATE_PARAMETERS("capture", ARG, 100, STREAMING_START_ON, ERROR_GENERATION_ON,  DISABLE_CH_0, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 100, STREAMING_START_ON, ERROR_GENERATION_ON,  DISABLE_CH_1, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 100, STREAMING_START_ON, ERROR_GENERATION_ON,  DISABLE_CH_2, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 100, STREAMING_START_ON, ERROR_GENERATION_ON,  DISABLE_CH_3, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 100, STREAMING_START_ON, ERROR_GENERATION_OFF, 0, 0)

TEST_WITH_ARG(tivxHwaCapture, testRawImageCaptureTimeout, Arg_CaptureTimeout, CAPTURE_TIMEOUT_PARAMETERS)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n0;

    /* Data objects for graph */
    vx_object_array capture_frames[MAX_NUM_BUF];
    vx_user_data_object capture_config;
    tivx_raw_image raw_image = 0, black_raw_image = 0;

    /* Local objects */
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    tivx_capture_params_t local_capture_config;
    uint32_t num_capture_channels = NUM_CHANNELS;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, instIdx, chIdx, out_num_refs;
    uint64_t exe_time, timestamp = 0, prev_timestamp = 0;

    /* Image objects */
    vx_reference refs[1];
    vx_user_data_object capture_stats_obj;
    tivx_capture_statistics_t *capture_stats_struct;
    vx_map_id capture_stats_map_id;
    uint32_t *data_ptr;
    vx_imagepatch_addressing_t addr;
    vx_rectangle_t rect;
    vx_object_array dequeue_capture_array;
    vx_bool done;

    /* Sensor Parameters */
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_uint8 num_sensors_found, count = 0;
    IssSensor_CreateParams sensorParams;
    char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];
    char *sensor_name;
    uint32_t sensor_features_enabled = 0, sensor_features_supported = 0;

    /* Error frame params */
    vx_event_t event;
    vx_bool is_invalid;

    /* Setting to num buf of capture node */
    num_buf = 4;
    loop_cnt = arg_->loop_cnt;

    /* Init for test case */
    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Imaging Initialization */
    {
        memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
        for(count=0;count<ISS_SENSORS_MAX_SUPPORTED_SENSOR;count++)
        {
            sensor_list[count] = availableSensorNames[count];
        }
        VX_CALL(appEnumerateImageSensor(sensor_list, &num_sensors_found));

        sensor_name = sensor_list[0]; /* Note: hardcoding to IMX390 */

        memset(&sensorParams, 0, sizeof(sensorParams));
        VX_CALL(appQueryImageSensor(sensor_name, &sensorParams));

        sensor_features_supported = sensorParams.sensorInfo.features;

        sensor_features_enabled = initSensorParams(sensor_features_supported);

        VX_CALL(appInitImageSensor(sensor_name, sensor_features_enabled, (1<<(num_capture_channels))-1)); /*Mask = 0xF for 4 cameras*/
    }

    /* Creating objects for graph */
    {
        ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(context, &(sensorParams.sensorInfo.raw_params)), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)raw_image, num_capture_channels), VX_TYPE_OBJECT_ARRAY);
        }

        /* Config initialization */
        tivx_capture_params_init(&local_capture_config);
        local_capture_config.timeout        = 90;
        local_capture_config.timeoutInitial = 500;
        local_capture_config.numInst = 1;
        local_capture_config.numCh   = num_capture_channels;
        chIdx = 0U;
        for (instIdx = 0U ; instIdx < 1 ; instIdx++)
        {
            local_capture_config.instId[instIdx] = instIdx;
            local_capture_config.instCfg[instIdx].enableCsiv2p0Support = (uint32_t)vx_true_e;
            local_capture_config.instCfg[instIdx].numDataLanes         = 4U;
            for (loop_id = 0U; loop_id < local_capture_config.instCfg[0U].numDataLanes ; loop_id++)
            {
                local_capture_config.instCfg[instIdx].dataLanesMap[loop_id] = (loop_id + 1u);
            }
            for (loop_id = 0U; loop_id < num_capture_channels; loop_id++)
            {
                local_capture_config.chVcNum[chIdx]   = loop_id;
                local_capture_config.chInstMap[chIdx] = instIdx;
                chIdx++;
            }
        }

        ASSERT_VX_OBJECT(capture_config = vxCreateUserDataObject(context, user_data_object_name, sizeof(tivx_capture_params_t), &local_capture_config), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(n0 = tivxCaptureNode(graph, capture_config, capture_frames[0]), VX_TYPE_NODE);

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)n0, VX_EVENT_NODE_ERROR, 0, CAPTURE_NODE_ERROR));

    }

    /* Pipelining and graph verification */
    {
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
    }

    /* Allocating black frame */
    {
        ASSERT_VX_OBJECT(black_raw_image = tivxCreateRawImage(context, &(sensorParams.sensorInfo.raw_params)), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        /* Initialize to black_frame black */

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(n0, (vx_reference)black_raw_image));

        VX_CALL(vxQueryReference((vx_reference)black_raw_image, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

        ASSERT(is_invalid==vx_true_e);
    }

    /* iniitalizing sensor */
    VX_CALL(appStartImageSensor(sensor_name, (1<<(num_capture_channels))-1));/*Mask for 4 cameras*/

    exe_time = tivxPlatformGetTimeInUsecs();

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_frames[buf_id], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array out_capture_frames;

        /* Error frame params */
        vx_event_t event;

        vxWaitEvent(context, &event, vx_true_e);

        /* Verifying that the capture node error event is not hit */
        ASSERT(event.app_value!=CAPTURE_NODE_ERROR);

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);
    }
    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    /* In the event of error generation, disable sensor as indicated by test case arguments */
    if (ERROR_GENERATION_ON == arg_->generate_error)
    {
        VX_CALL(appStopImageSensor(sensor_name, (1<<(num_capture_channels))-1)); /*Disabling camera 4 */
        if (DISABLE_CH_0 == arg_->camera_disable)
        {
            VX_CALL(appStartImageSensor(sensor_name, 0xE)); // re-enabling all but 0th camera
        }
        else if (DISABLE_CH_1 == arg_->camera_disable)
        {
            VX_CALL(appStartImageSensor(sensor_name, 0xD)); // re-enabling all but 1st camera
        }
        else if (DISABLE_CH_2 == arg_->camera_disable)
        {
            VX_CALL(appStartImageSensor(sensor_name, 0xB)); // re-enabling all but 2nd camera
        }
        else if (DISABLE_CH_3 == arg_->camera_disable)
        {
            VX_CALL(appStartImageSensor(sensor_name, (1<<(3))-1)); // re-enabling all but 3rd camera
        }

        /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
        for(loop_id=0; loop_id<loop_cnt; loop_id++)
        {
            uint32_t num_refs;
            vx_object_array out_capture_frames;
            tivx_raw_image image_element;

            if (VX_SUCCESS == vxWaitEvent(context, &event, vx_true_e))
            {
                /* Verifying that the capture node error event is not hit */
                ASSERT(event.app_value==CAPTURE_NODE_ERROR);
            }

            /* Get output reference, waits until a reference is available */
            vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);

            if (loop_id > num_buf)
            {
                ASSERT_VX_OBJECT(image_element = (tivx_raw_image) vxGetObjectArrayItem(out_capture_frames, arg_->camera_disable), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

                VX_CALL(vxQueryReference((vx_reference)image_element, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

                ASSERT(is_invalid==vx_true_e);

                VX_CALL(tivxReleaseRawImage(&image_element));
            }

            vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);
        }

        vxWaitGraph(graph);
    }

    /* Dequeue all buffers */
    done = vx_false_e;
    while(!done)
    {
        VX_CALL(vxGraphParameterCheckDoneRef(graph, 0, &out_num_refs));

        if(out_num_refs>0)
        {
            tivx_raw_image image_element;
            vx_bool is_invalid;

            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&dequeue_capture_array, 1, &out_num_refs));

            if (ERROR_GENERATION_ON == arg_->generate_error)
            {
                ASSERT_VX_OBJECT(image_element = (tivx_raw_image) vxGetObjectArrayItem(dequeue_capture_array, arg_->camera_disable), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

                VX_CALL(vxQueryReference((vx_reference)image_element, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

                ASSERT(is_invalid==vx_true_e);

                VX_CALL(tivxReleaseRawImage(&image_element));
            }
        }

        if(out_num_refs == 0)
        {
            done = vx_true_e;
        }
    }

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    VX_CALL(appStopImageSensor(sensor_name, (1<<(num_capture_channels))-1)); /*Mask for 4 cameras*/

    /* Querying node for sensor stats */
    {
        capture_stats_obj =
            vxCreateUserDataObject(context, "tivx_capture_statistics_t" ,
            sizeof(tivx_capture_statistics_t), NULL);

        refs[0] = (vx_reference)capture_stats_obj;
        tivxNodeSendCommand(n0, 0,
            TIVX_CAPTURE_GET_STATISTICS, refs, 1u);

        vxMapUserDataObject(
                (vx_user_data_object)refs[0],
                0,
                sizeof(tivx_capture_statistics_t),
                &capture_stats_map_id,
                (void **)&data_ptr,
                VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            );

        capture_stats_struct = (tivx_capture_statistics_t*)data_ptr;

        /* As this is single instance app, array index to access status will be always '0U' */
        printf("\n\r==========================================================\r\n");
        printf(": Capture Status:\r\n");
        printf("==========================================================\r\n");
        printf(": FIFO Overflow Count: %d\r\n",
                  capture_stats_struct->overflowCount[0U]);
        printf(": Spurious UDMA interrupt count: %d\r\n",
                  capture_stats_struct->spuriousUdmaIntrCount[0U]);
        printf("  [Channel No] | Frame Queue Count |"
            " Frame De-queue Count | Frame Drop Count |\n");
        for(chIdx = 0U ; chIdx < num_capture_channels ; chIdx ++)
        {
            printf("\t\t%d|\t\t%d|\t\t%d|\t\t%d|\n",
                  chIdx,
                  capture_stats_struct->queueCount[0U][chIdx],
                  capture_stats_struct->dequeueCount[0U][chIdx],
                  capture_stats_struct->dropCount[0U][chIdx]);
        }
        if (ERROR_GENERATION_ON == arg_->generate_error)
        {
            for(chIdx = 0U ; chIdx < num_capture_channels ; chIdx ++)
            {
                uint8_t bitmask = 1<<(chIdx);
                if (chIdx == arg_->camera_disable)
                {
                    ASSERT((capture_stats_struct->activeChannelMask & bitmask)==0U);
                }
                else
                {
                    ASSERT((capture_stats_struct->activeChannelMask & bitmask)!=0U);
                }

            }
        }
        vxUnmapUserDataObject((vx_user_data_object)refs[0], capture_stats_map_id);
    }

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseGraph(&graph));

    VX_CALL(tivxReleaseRawImage(&black_raw_image));
    VX_CALL(tivxReleaseRawImage(&raw_image));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&capture_frames[buf_id]));
    }
    VX_CALL(vxReleaseUserDataObject(&capture_config));
    VX_CALL(vxReleaseUserDataObject(&capture_stats_obj));

    tivxHwaUnLoadKernels(context);

    appDeInitImageSensor(sensor_name);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

/* Test case 1: Start with all cameras streaming then turn off camera 0
 * Test case 2: Start with all cameras streaming then turn off camera 1
 * Test case 3: Start with all cameras streaming then turn off camera 2
 * Test case 4: Start with all cameras streaming then turn off camera 3
 * Test case 5: Start with all cameras streaming then turn off cameras 2 and 3
 * Test case 6: Start with all cameras streaming then turn off all cameras
 * Test case 7: Start with all cameras not streaming then turn on all cameras
 * Test case 8: Start with all cameras streaming then turn off all cameras then turn back on all cameras
 * Test case 9: Start with all cameras on and don't generate any errors
 */
#define CAPTURE_DISPLAY_PARAMETERS \
    CT_GENERATE_PARAMETERS("capture", ARG, 100, STREAMING_START_ON,  ERROR_GENERATION_ON,  DISABLE_CH_0,   0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 100, STREAMING_START_ON,  ERROR_GENERATION_ON,  DISABLE_CH_1,   0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 100, STREAMING_START_ON,  ERROR_GENERATION_ON,  DISABLE_CH_2,   0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 100, STREAMING_START_ON,  ERROR_GENERATION_ON,  DISABLE_CH_3,   0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 100, STREAMING_START_ON,  ERROR_GENERATION_ON,  DISABLE_CH_2_3, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 100, STREAMING_START_ON,  ERROR_GENERATION_ON,  DISABLE_ALL_CH, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 100, STREAMING_START_OFF, ERROR_GENERATION_ON,  ENABLE_ALL_CH,  0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 100, STREAMING_START_ON,  ERROR_GENERATION_ON,  DISABLE_REENABLE_ALL_CH, 0), \
    CT_GENERATE_PARAMETERS("capture", ARG, 100, STREAMING_START_ON,  ERROR_GENERATION_OFF, 0, 0)

TEST_WITH_ARG(tivxHwaCapture, testRawImageCaptureDisplay, Arg_CaptureTimeout, CAPTURE_DISPLAY_PARAMETERS)
{
    /* Graph objects */
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node captureNode, displayNode;

    /* Data objects for graph */
    vx_image image, sample_image, black_frame, black_frame_invalid_params;
    vx_object_array capture_frames[MAX_NUM_BUF];
    vx_user_data_object capture_config;
    vx_user_data_object display_param_obj;

    /* Local objects */
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    tivx_capture_params_t local_capture_config;
    uint32_t num_capture_channels = NUM_CHANNELS;
    uint32_t buf_id, loop_id, loop_cnt, num_buf, instIdx, chIdx, out_num_refs;
    uint64_t exe_time, timestamp = 0, prev_timestamp = 0;
    tivx_display_params_t display_params;

    /* Image objects */
    vx_reference refs[1];
    vx_user_data_object capture_stats_obj;
    tivx_capture_statistics_t *capture_stats_struct;
    vx_map_id capture_stats_map_id;
    uint32_t *data_ptr;
    vx_imagepatch_addressing_t addr;
    vx_rectangle_t rect;
    vx_object_array dequeue_capture_array;
    vx_bool done;

    /* Display command objects */
    tivx_display_select_channel_params_t channel_prms;
    vx_user_data_object switch_ch_obj;

    /* Sensor Parameters */
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_uint8 num_sensors_found, count = 0;
    IssSensor_CreateParams sensorParams;
    char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];
    char *sensor_name;
    uint32_t sensor_features_enabled = 0, sensor_features_supported = 0;

    /* Error frame params */
    vx_event_t event;
    vx_bool is_invalid;

    /* Setting to num buf of capture node */
    num_buf = 4;
    loop_cnt = 1000;

    /* Init for test case */
    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Imaging Initialization */
    {
        memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
        for(count=0;count<ISS_SENSORS_MAX_SUPPORTED_SENSOR;count++)
        {
            sensor_list[count] = availableSensorNames[count];
        }
        VX_CALL(appEnumerateImageSensor(sensor_list, &num_sensors_found));

        sensor_name = sensor_list[0]; /* Note: hardcoding to IMX390 */

        memset(&sensorParams, 0, sizeof(sensorParams));
        VX_CALL(appQueryImageSensor(sensor_name, &sensorParams));

        sensor_features_supported = sensorParams.sensorInfo.features;

        sensor_features_enabled = initSensorParams(sensor_features_supported);

        VX_CALL(appInitImageSensor(sensor_name, sensor_features_enabled, (1<<(num_capture_channels))-1)); /*Mask = 0xF for 4 cameras*/
    }

    /* Creating objects for capture */
    {
        ASSERT_VX_OBJECT(image = vxCreateImage(context, 1936, 1096, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);

        /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)image, num_capture_channels), VX_TYPE_OBJECT_ARRAY);
        }

        /* Config initialization */
        tivx_capture_params_init(&local_capture_config);
        local_capture_config.timeout        = 90;
        local_capture_config.timeoutInitial = 500;
        local_capture_config.numInst = 1;
        local_capture_config.numCh   = num_capture_channels;
        chIdx = 0U;
        for (instIdx = 0U ; instIdx < 1 ; instIdx++)
        {
            local_capture_config.instId[instIdx] = instIdx;
            local_capture_config.instCfg[instIdx].enableCsiv2p0Support = (uint32_t)vx_true_e;
            local_capture_config.instCfg[instIdx].numDataLanes         = 4U;
            for (loop_id = 0U; loop_id < local_capture_config.instCfg[0U].numDataLanes ; loop_id++)
            {
                local_capture_config.instCfg[instIdx].dataLanesMap[loop_id] = (loop_id + 1u);
            }
            for (loop_id = 0U; loop_id < num_capture_channels; loop_id++)
            {
                local_capture_config.chVcNum[chIdx]   = loop_id;
                local_capture_config.chInstMap[chIdx] = instIdx;
                chIdx++;
            }
        }

        ASSERT_VX_OBJECT(capture_config = vxCreateUserDataObject(context, user_data_object_name, sizeof(tivx_capture_params_t), &local_capture_config), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(captureNode = tivxCaptureNode(graph, capture_config, capture_frames[0]), VX_TYPE_NODE);

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)captureNode, VX_EVENT_NODE_ERROR, 0, CAPTURE_NODE_ERROR));

    }

    /* Creating objects for display */
    {
        /* Display initialization */
        memset(&display_params, 0, sizeof(tivx_display_params_t));
        display_params.opMode=TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;
        display_params.pipeId=2;
        display_params.outWidth=1920;
        display_params.outHeight=1080;
        display_params.posX=0;
        display_params.posY=0;

        ASSERT_VX_OBJECT(display_param_obj = vxCreateUserDataObject(context, "tivx_display_params_t", sizeof(tivx_display_params_t), &display_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        sample_image = (vx_image) vxGetObjectArrayItem(capture_frames[0], 0);

        ASSERT_VX_OBJECT(displayNode = tivxDisplayNode(graph, display_param_obj, sample_image), VX_TYPE_NODE);

        VX_CALL(vxReleaseImage(&sample_image));
        /* Create User Data object for channel switching */
        channel_prms.active_channel_id = 0;
        ASSERT_VX_OBJECT(switch_ch_obj = vxCreateUserDataObject(context,
            "tivx_display_select_channel_params_t",
            sizeof(tivx_display_select_channel_params_t), &channel_prms),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        refs[0] = (vx_reference)switch_ch_obj;
    }

    /* Pipelining and graph verification */
    {
        /* input @ node index 0, becomes graph parameter 1 */
        add_graph_parameter_by_node_index(graph, captureNode, 1);

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

        VX_CALL(vxSetNodeTarget(captureNode, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));
        VX_CALL(vxSetNodeTarget(displayNode, VX_TARGET_STRING, TIVX_TARGET_DISPLAY1));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    }


    /* Allocating black frame */
    {
        vx_pixel_value_t init_val;

        init_val.U16  = 0U;

        ASSERT_VX_OBJECT(black_frame = vxCreateUniformImage(context, 1936, 1096, VX_DF_IMAGE_U16, &init_val), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(black_frame_invalid_params = vxCreateImage(context, 1920, 1096, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        /* Initialize to black_frame black */

        /* Note: sending image w/ incorrect params to validate error checking */
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxCaptureRegisterErrorFrame(captureNode, (vx_reference)black_frame_invalid_params));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxCaptureRegisterErrorFrame(captureNode, (vx_reference)black_frame));

        VX_CALL(vxQueryReference((vx_reference)black_frame, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

        ASSERT(is_invalid==vx_true_e);
    }

    /* initializing sensor */
    if (STREAMING_START_ON == arg_->start_sensors)
    {
        VX_CALL(appStartImageSensor(sensor_name, (1<<(num_capture_channels))-1));/*Mask for 4 cameras*/
    }

    exe_time = tivxPlatformGetTimeInUsecs();

    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&capture_frames[buf_id], 1);
    }

    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        uint32_t num_refs;
        vx_object_array out_capture_frames;
        uint64_t timestamp0 = 0, timestamp1 = 0;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);

        if (0 == (loop_id % CHANNEL_SWITCH_FRAME_COUNT))
        {
            channel_prms.active_channel_id =
                (channel_prms.active_channel_id + 1) % num_capture_channels;
            VX_CALL(vxCopyUserDataObject(switch_ch_obj, 0,
                sizeof(tivx_display_select_channel_params_t),
                &channel_prms, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            VX_CALL(tivxNodeSendCommand(displayNode, 0,
                TIVX_DISPLAY_SELECT_CHANNEL, refs, 1u));
        }

        if (STREAMING_START_ON == arg_->start_sensors)
        {
            vx_image element0, element1;

            ASSERT_VX_OBJECT(element0 = (vx_image) vxGetObjectArrayItem(out_capture_frames, 0), VX_TYPE_IMAGE);

            ASSERT_VX_OBJECT(element1 = (vx_image) vxGetObjectArrayItem(out_capture_frames, 1), VX_TYPE_IMAGE);

            VX_CALL(vxQueryReference((vx_reference)element0, TIVX_REFERENCE_TIMESTAMP, &timestamp0, sizeof(timestamp0)));
            VX_CALL(vxQueryReference((vx_reference)element1, TIVX_REFERENCE_TIMESTAMP, &timestamp1, sizeof(timestamp1)));

            ASSERT(timestamp0!=timestamp1);

            VX_CALL(vxReleaseImage(&element0));
            VX_CALL(vxReleaseImage(&element1));
        }

        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);

        vxWaitEvent(context, &event, vx_true_e);
    }

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    /* In the event of error generation, disable sensor as indicated by test case arguments */
    if (ERROR_GENERATION_ON == arg_->generate_error)
    {
        if (STREAMING_START_ON == arg_->start_sensors)
        {
            VX_CALL(appStopImageSensor(sensor_name, (1<<(num_capture_channels))-1)); /*Disabling camera 4 */
            if (DISABLE_CH_0 == arg_->camera_disable)
            {
                VX_CALL(appStartImageSensor(sensor_name, 0xE)); // re-enabling all but 0th camera
            }
            else if (DISABLE_CH_1 == arg_->camera_disable)
            {
                VX_CALL(appStartImageSensor(sensor_name, 0xD)); // re-enabling all but 1st camera
            }
            else if (DISABLE_CH_2 == arg_->camera_disable)
            {
                VX_CALL(appStartImageSensor(sensor_name, 0xB)); // re-enabling all but 2nd camera
            }
            else if (DISABLE_CH_3 == arg_->camera_disable)
            {
                VX_CALL(appStartImageSensor(sensor_name, (1<<(3))-1)); // re-enabling all but 3rd camera
            }
            else if (DISABLE_CH_2_3 == arg_->camera_disable)
            {
                VX_CALL(appStartImageSensor(sensor_name, 0xC)); // re-enabling all 0th and 1st camera
            }
        }
        else
        {
            VX_CALL(appStartImageSensor(sensor_name, (1<<(num_capture_channels))-1));/*Mask for 4 cameras*/
        }


        /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
        for(loop_id=0; loop_id<loop_cnt; loop_id++)
        {
            uint32_t num_refs;
            vx_object_array out_capture_frames;

            /* Get output reference, waits until a reference is available */
            vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);

            if (0 == (loop_id % CHANNEL_SWITCH_FRAME_COUNT))
            {
                channel_prms.active_channel_id =
                    (channel_prms.active_channel_id + 1) % num_capture_channels;
                VX_CALL(vxCopyUserDataObject(switch_ch_obj, 0,
                    sizeof(tivx_display_select_channel_params_t),
                    &channel_prms, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                VX_CALL(tivxNodeSendCommand(displayNode, 0,
                    TIVX_DISPLAY_SELECT_CHANNEL, refs, 1u));
            }

            if ( (loop_id > num_buf) &&
                 (ERROR_GENERATION_ON == arg_->generate_error) &&
                 (arg_->camera_disable < 4) )
            {
                vx_image image_element;
                ASSERT_VX_OBJECT(image_element = (vx_image) vxGetObjectArrayItem(out_capture_frames, arg_->camera_disable), VX_TYPE_IMAGE);

                VX_CALL(vxQueryReference((vx_reference)image_element, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

                ASSERT(is_invalid==vx_true_e);

                VX_CALL(vxReleaseImage(&image_element));
            }

            vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);

            vxWaitEvent(context, &event, vx_true_e);
        }

        if (DISABLE_REENABLE_ALL_CH == arg_->camera_disable)
        {
            /* ensure all graph processing is complete */
            vxWaitGraph(graph);

            VX_CALL(appStartImageSensor(sensor_name, (1<<(num_capture_channels))-1));/*Mask for 4 cameras*/

            /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
            for(loop_id=0; loop_id<loop_cnt; loop_id++)
            {
                uint32_t num_refs;
                vx_object_array out_capture_frames;

                /* Get output reference, waits until a reference is available */
                vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&out_capture_frames, 1, &num_refs);

                if (0 == (loop_id % CHANNEL_SWITCH_FRAME_COUNT))
                {
                    channel_prms.active_channel_id =
                        (channel_prms.active_channel_id + 1) % num_capture_channels;
                    VX_CALL(vxCopyUserDataObject(switch_ch_obj, 0,
                        sizeof(tivx_display_select_channel_params_t),
                        &channel_prms, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                    VX_CALL(tivxNodeSendCommand(displayNode, 0,
                        TIVX_DISPLAY_SELECT_CHANNEL, refs, 1u));
                }

                /* Ensuring that the capture frame is valid after reenabling sensor */
                if (loop_id > num_buf)
                {
                    vx_image image_element;
                    ASSERT_VX_OBJECT(image_element = (vx_image) vxGetObjectArrayItem(out_capture_frames, 0), VX_TYPE_IMAGE);

                    VX_CALL(vxQueryReference((vx_reference)image_element, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

                    ASSERT(is_invalid==vx_false_e);

                    VX_CALL(vxReleaseImage(&image_element));
                }

                vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&out_capture_frames, 1);

                vxWaitEvent(context, &event, vx_true_e);
            }
        }
    }

    /* Dequeue all buffers */
    done = vx_false_e;
    while(!done)
    {
        VX_CALL(vxGraphParameterCheckDoneRef(graph, 0, &out_num_refs));

        if(out_num_refs>0)
        {
            VX_CALL(vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&dequeue_capture_array, 1, &out_num_refs));

            if ( (ERROR_GENERATION_ON == arg_->generate_error) &&
                 (arg_->camera_disable < 4) )
            {
                vx_image image_element;
                ASSERT_VX_OBJECT(image_element = (vx_image) vxGetObjectArrayItem(dequeue_capture_array, arg_->camera_disable), VX_TYPE_IMAGE);

                VX_CALL(vxQueryReference((vx_reference)image_element, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

                ASSERT(is_invalid==vx_true_e);

                VX_CALL(vxReleaseImage(&image_element));
            }
        }

        if(out_num_refs == 0)
        {
            done = vx_true_e;
        }
    }

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    /* Querying node for sensor stats */
    {
        capture_stats_obj =
            vxCreateUserDataObject(context, "tivx_capture_statistics_t" ,
            sizeof(tivx_capture_statistics_t), NULL);

        refs[0] = (vx_reference)capture_stats_obj;
        tivxNodeSendCommand(captureNode, 0,
            TIVX_CAPTURE_GET_STATISTICS, refs, 1u);

        vxMapUserDataObject(
                (vx_user_data_object)refs[0],
                0,
                sizeof(tivx_capture_statistics_t),
                &capture_stats_map_id,
                (void **)&data_ptr,
                VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            );

        capture_stats_struct = (tivx_capture_statistics_t*)data_ptr;

        /* As this is single instance app, array index to access status will be always '0U' */
        printf("\n\r==========================================================\r\n");
        printf(": Capture Status:\r\n");
        printf("==========================================================\r\n");
        printf(": FIFO Overflow Count: %d\r\n",
                  capture_stats_struct->overflowCount[0U]);
        printf(": Spurious UDMA interrupt count: %d\r\n",
                  capture_stats_struct->spuriousUdmaIntrCount[0U]);
        printf("  [Channel No] | Frame Queue Count |"
            " Frame De-queue Count | Frame Drop Count |\n");
        for(chIdx = 0U ; chIdx < num_capture_channels ; chIdx ++)
        {
            printf("\t\t%d|\t\t%d|\t\t%d|\t\t%d|\n",
                  chIdx,
                  capture_stats_struct->queueCount[0U][chIdx],
                  capture_stats_struct->dequeueCount[0U][chIdx],
                  capture_stats_struct->dropCount[0U][chIdx]);
        }
        if (ERROR_GENERATION_ON == arg_->generate_error)
        {
            for(chIdx = 0U ; chIdx < num_capture_channels ; chIdx ++)
            {
                uint8_t bitmask = 1<<(chIdx);
                if (chIdx == arg_->camera_disable)
                {
                    ASSERT((capture_stats_struct->activeChannelMask & bitmask)==0U);
                }
                else if ((DISABLE_CH_2_3 == arg_->camera_disable) &&
                         ((chIdx == 0) || (chIdx == 1)))
                {
                    ASSERT((capture_stats_struct->activeChannelMask & bitmask)==0U);
                }
                else if (DISABLE_ALL_CH == arg_->camera_disable)
                {
                    ASSERT((capture_stats_struct->activeChannelMask & bitmask)==0U);
                }
                else
                {
                    ASSERT((capture_stats_struct->activeChannelMask & bitmask)!=0U);
                }
            }
        }
        vxUnmapUserDataObject((vx_user_data_object)refs[0], capture_stats_map_id);
    }

    VX_CALL(appStopImageSensor(sensor_name, (1<<(num_capture_channels))-1)); /*Mask for 4 cameras*/

    VX_CALL(vxReleaseImage(&black_frame_invalid_params));
    VX_CALL(vxReleaseNode(&displayNode));
    VX_CALL(vxReleaseNode(&captureNode));
    VX_CALL(vxReleaseGraph(&graph));

    VX_CALL(vxReleaseImage(&black_frame));
    VX_CALL(vxReleaseImage(&image));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&capture_frames[buf_id]));
    }
    VX_CALL(vxReleaseUserDataObject(&capture_config));
    VX_CALL(vxReleaseUserDataObject(&capture_stats_obj));
    VX_CALL(vxReleaseUserDataObject(&display_param_obj));
    VX_CALL(vxReleaseUserDataObject(&switch_ch_obj));

    tivxHwaUnLoadKernels(context);

    appDeInitImageSensor(sensor_name);

    tivx_clr_debug_zone(VX_ZONE_INFO);
}

TESTCASE_TESTS(tivxHwaCapture,
               testRawImageCapture,
               testRawImageCaptureTimeout,
               testRawImageCaptureDisplay)


#endif /* BUILD_CAPTURE */