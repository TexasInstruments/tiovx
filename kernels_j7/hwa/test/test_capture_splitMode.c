/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
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
#include <TI/tivx_task.h>
#include <TI/tivx_event.h>
#include "math.h"
#include <limits.h>
#include <utils/mem/include/app_mem.h>
#include "test_tiovx/test_tiovx.h"
#include "test_hwa_common.h"

#include <utils/sensors/include/app_sensors.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/iss/include/app_iss.h>

#define CAPTURE1_ENABLE        (1U)
#define CAPTURE2_ENABLE        (1U)

#define MAX_NUM_BUF             (8u)
#define CAPTURE_MIN_PIPEUP_BUFS (3u)

#define NUM_CHANNELS        (2U)
#define CAPT1_INST_ID       (0U)
#define CAPT2_INST_ID       (1U)

#define CAPT1_INST_SENSOR_MASK       ((1 << NUM_CHANNELS) - 1U)
#define CAPT2_INST_SENSOR_MASK       (((1 << NUM_CHANNELS) - 1U) << 4U)

#define CAPTURE1_NODE_ERROR  (1U)
#define CAPTURE2_NODE_ERROR  (1U)

/* Default is RAW image */
#define CAPTURE_FORMAT         (TIVX_TYPE_RAW_IMAGE)

#define TIVX_TARGET_DEFAULT_STACK_SIZE      (256U * 1024U)
#define TIVX_TARGET_DEFAULT_TASK_PRIORITY1   (8u)

TESTCASE(tivxHwaCaptureSplitMode, CT_VXContext, ct_setup_vx_context, 0)

#if (CAPTURE1_ENABLE == 1U)
static tivx_event  eventHandle_RxFinished;
#endif
#if (CAPTURE2_ENABLE == 1U)
static tivx_event  eventHandle_RxFinished1;
static tivx_event  eventHandle_SensorCfgDone;
uint32_t sensorCfgDone = 0U;
#endif

static vx_context context;
static tivx_raw_image raw_image_exemplar;
static vx_image capt_image_exemplar;
static uint32_t width, height, loop_cnt, num_buf;
static vx_rectangle_t rect;

/* Sensor Parameters */
char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
vx_uint8 num_sensors_found, count = 0;
IssSensor_CreateParams sensorParams;
char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];
uint32_t sensor_features_enabled = 0, sensor_features_supported = 0;
char *sensor_name;
uint32_t sensorMask = 0U;

typedef struct {
    const char* name;
    uint32_t Width;
    uint32_t Height;
    uint32_t loopCount;
} Arg;

#define ADD_WIDTH(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/Width=1936", __VA_ARGS__, 1936))
#define ADD_HEIGHT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/Height=1096", __VA_ARGS__, 1096))
#define ADD_LOOP_1000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loopCount=1000", __VA_ARGS__, 1000))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("CaptureSplitMode", ADD_WIDTH, ADD_HEIGHT, ADD_LOOP_1000 ,ARG), \


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

#if (CAPTURE1_ENABLE == 1U)
static void VX_CALLBACK tivxTask_capture(void *app_var)
{
    vx_node csirx_node = 0;
    vx_object_array capture_frames[MAX_NUM_BUF];
    vx_user_data_object capture_config;
    tivx_capture_params_t local_capture_config;
    uint32_t buf_id, loop_id, loopCnt;
    vx_graph_parameter_queue_params_t csirx_graph_parameters_queue_params_list[1];

    vx_graph csirx_graph = (vx_graph)app_var;

    ASSERT(num_buf > 0);

    /* allocate Input and Output refs, multiple refs created to allow pipelining of csirx_graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
#if (CAPTURE_FORMAT == TIVX_TYPE_RAW_IMAGE)
        ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)raw_image_exemplar, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);
#endif
    }

    /* CSIRX Config initialization */
    tivx_capture_params_init(&local_capture_config);
    local_capture_config.numInst                          = 1U;
    local_capture_config.numCh                            = NUM_CHANNELS;
    local_capture_config.instId[0U]                       = CAPT1_INST_ID;
    local_capture_config.instCfg[0U].enableCsiv2p0Support = (uint32_t)vx_true_e;
    local_capture_config.instCfg[0U].numDataLanes         = 4U;
    for (loopCnt = 0U ;
        loopCnt < local_capture_config.instCfg[0U].numDataLanes ;
        loopCnt++)
    {
        local_capture_config.instCfg[0U].dataLanesMap[loopCnt] = (loopCnt + 1u);
    }
    for (loopCnt = 0U; loopCnt < NUM_CHANNELS; loopCnt++)
    {
        local_capture_config.chVcNum[loopCnt]   = loopCnt;
        local_capture_config.chInstMap[loopCnt] = CAPT1_INST_ID;
    }

    ASSERT_VX_OBJECT(capture_config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &local_capture_config), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(csirx_node = tivxCaptureNode(csirx_graph, capture_config, capture_frames[0]), VX_TYPE_NODE);

    /* input @ node index 0, becomes csirx_graph parameter 1 */
    add_graph_parameter_by_node_index(csirx_graph, csirx_node, 1);

    /* set csirx_graph schedule config such that csirx_graph parameter @ index 0 and 1 are enqueuable */
    csirx_graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    csirx_graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    csirx_graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&capture_frames[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
    * Graph gets scheduled automatically as refs are enqueued to it
    */
    vxSetGraphScheduleConfig(csirx_graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            1,
            csirx_graph_parameters_queue_params_list
            );

    VX_CALL(vxSetNodeTarget(csirx_node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(csirx_graph));

    /* iniitalizing sensor */
    VX_CALL(appStartImageSensor(sensor_name, CAPT1_INST_SENSOR_MASK));/*Mask for 2 cameras*/

    /* enqueue capture buffers for pipeup but dont trigger graph executions */
    for(buf_id=0; buf_id<num_buf-1; buf_id++)
    {
        tivxGraphParameterEnqueueReadyRef(csirx_graph, 0, (vx_reference*)&capture_frames[buf_id], 1, TIVX_GRAPH_PARAMETER_ENQUEUE_FLAG_PIPEUP);
    }

    /* after pipeup, now enqueue a buffer to trigger graph scheduling */
    vxGraphParameterEnqueueReadyRef(csirx_graph, 0, (vx_reference*)&capture_frames[buf_id], 1);

    /* wait for csirx_graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<loop_cnt; loop_id++)
    {
        uint32_t num_refs;
        vx_object_array captured_frames = NULL;

        /* Get captured frame reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(csirx_graph, 0, (vx_reference*)&captured_frames, 1, &num_refs);

#if (CAPTURE2_ENABLE == 1U)
        if (sensorCfgDone == 0U)
        {
            tivxEventPost(eventHandle_SensorCfgDone);
            sensorCfgDone = 1U;
        }
#endif
        
        vxGraphParameterEnqueueReadyRef(csirx_graph, 0, (vx_reference*)&captured_frames, 1);

    }
    /* ensure all csirx_graph processing is complete */
    vxWaitGraph(csirx_graph);

    VX_CALL(appStopImageSensor(sensor_name, CAPT1_INST_SENSOR_MASK)); /*Mask for 2 cameras*/

    VX_CALL(vxReleaseNode(&csirx_node));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&capture_frames[buf_id]));
    }
    VX_CALL(vxReleaseUserDataObject(&capture_config));

    /*Signal the completion of csirx graph processing*/
    tivxEventPost(eventHandle_RxFinished);
}
#endif

#if (CAPTURE2_ENABLE == 1U)
static void VX_CALLBACK tivxTask_capture1(void *app_var)
{
    vx_node csirx_node = 0;
    vx_object_array capture_frames[MAX_NUM_BUF];
    vx_user_data_object capture_config;
    tivx_capture_params_t local_capture_config;
    uint32_t buf_id, loop_id, loopCnt;
    vx_graph_parameter_queue_params_t csirx_graph_parameters_queue_params_list[1];

    vx_graph csirx_graph = (vx_graph)app_var;

    ASSERT(num_buf > 0);
    
    tivxEventWait(eventHandle_SensorCfgDone, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);
    
    /* allocate Input and Output refs, multiple refs created to allow pipelining of csirx_graph */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
#if (CAPTURE_FORMAT == TIVX_TYPE_RAW_IMAGE)
        ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)raw_image_exemplar, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);
#endif
    }

    /* CSIRX Config initialization */
    tivx_capture_params_init(&local_capture_config);
    local_capture_config.numInst                          = 1U;
    local_capture_config.numCh                            = NUM_CHANNELS;
    local_capture_config.instId[0U]                       = CAPT2_INST_ID;
    local_capture_config.instCfg[0U].enableCsiv2p0Support = (uint32_t)vx_true_e;
    local_capture_config.instCfg[0U].numDataLanes         = 4U;
    for (loopCnt = 0U ;
        loopCnt < local_capture_config.instCfg[0U].numDataLanes ;
        loopCnt++)
    {
        local_capture_config.instCfg[0U].dataLanesMap[loopCnt] = (loopCnt + 1u);
    }
    for (loopCnt = 0U; loopCnt < NUM_CHANNELS; loopCnt++)
    {
        local_capture_config.chVcNum[loopCnt]   = loopCnt;
        local_capture_config.chInstMap[loopCnt] = CAPT2_INST_ID;
    }

    ASSERT_VX_OBJECT(capture_config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &local_capture_config), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(csirx_node = tivxCaptureNode(csirx_graph, capture_config, capture_frames[0]), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(csirx_node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE2));

    /* input @ node index 0, becomes csirx_graph parameter 1 */
    add_graph_parameter_by_node_index(csirx_graph, csirx_node, 1);

    /* set csirx_graph schedule config such that csirx_graph parameter @ index 0 and 1 are enqueuable */
    csirx_graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    csirx_graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    csirx_graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&capture_frames[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
    * Graph gets scheduled automatically as refs are enqueued to it
    */
    vxSetGraphScheduleConfig(csirx_graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            1,
            csirx_graph_parameters_queue_params_list
            );

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(csirx_graph));

    /* iniitalizing sensor */
    VX_CALL(appStartImageSensor(sensor_name, CAPT2_INST_SENSOR_MASK));/*Mask for 2 cameras*/
    /* enqueue capture buffers for pipeup but dont trigger graph executions */
    for(buf_id=0; buf_id<num_buf-(CAPTURE_MIN_PIPEUP_BUFS-1); buf_id++)
    {
        tivxGraphParameterEnqueueReadyRef(csirx_graph, 0, (vx_reference*)&capture_frames[buf_id], 1, TIVX_GRAPH_PARAMETER_ENQUEUE_FLAG_PIPEUP);
    }

    /*Now enqueue a buffer to trigger csirx_graph scheduling */
    vxGraphParameterEnqueueReadyRef(csirx_graph, 0, (vx_reference*)&capture_frames[num_buf-(CAPTURE_MIN_PIPEUP_BUFS-1)], 1);

    /*after pipeup, trigger again with last buffer */
    vxGraphParameterEnqueueReadyRef(csirx_graph, 0, (vx_reference*)&capture_frames[num_buf-1], 1);

    /* wait for csirx_graph instances to complete, compare output and recycle data buffers, schedule again */
    for(loop_id=0; loop_id<loop_cnt; loop_id++)
    {
        uint32_t num_refs;
        vx_object_array captured_frames = NULL;

        /* Get captured frame reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(csirx_graph, 0, (vx_reference*)&captured_frames, 1, &num_refs);

        vxGraphParameterEnqueueReadyRef(csirx_graph, 0, (vx_reference*)&captured_frames, 1);

    }
    /* ensure all csirx_graph processing is complete */
    vxWaitGraph(csirx_graph);

    VX_CALL(appStopImageSensor(sensor_name, CAPT2_INST_SENSOR_MASK));

    VX_CALL(vxReleaseNode(&csirx_node));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&capture_frames[buf_id]));
    }
    VX_CALL(vxReleaseUserDataObject(&capture_config));

    /*Signal the completion of csirx graph processing*/
    tivxEventPost(eventHandle_RxFinished1);
}
#endif

TEST_WITH_ARG(tivxHwaCaptureSplitMode, testCaptureSplitModeTest, Arg, PARAMETERS)
{
#if (CAPTURE1_ENABLE == 1U)
    vx_graph csirx_graph = 0 ;
    tivx_task taskHandle_capture;
#endif
#if (CAPTURE2_ENABLE == 1U)
    vx_graph csirx_graph1 = 0 ;
    tivx_task taskHandle_capture1;
#endif
#if (CAPTURE1_ENABLE == 1U) || (CAPTURE2_ENABLE == 1U)
    tivx_task_create_params_t taskParams_capture;
#endif

    context = context_->vx_context_;

    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_CAPTURE1));
    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_CAPTURE2));

    {
        /* Setting to num buf */
        num_buf = 4;

        loop_cnt = arg_->loopCount;
        width = arg_->Width;
        height = arg_->Height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        tivx_clr_debug_zone(VX_ZONE_INFO);

#if (CAPTURE1_ENABLE == 1U)
        ASSERT_VX_OBJECT(csirx_graph = vxCreateGraph(context), VX_TYPE_GRAPH);
#endif
#if (CAPTURE2_ENABLE == 1U)
        ASSERT_VX_OBJECT(csirx_graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);
#endif

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

#if (CAPTURE1_ENABLE == 1U)
            sensorMask |= CAPT1_INST_SENSOR_MASK;
#endif
#if (CAPTURE2_ENABLE == 1U)
            sensorMask |= CAPT2_INST_SENSOR_MASK;
#endif
            VX_CALL(appInitImageSensor(sensor_name, sensor_features_enabled, sensorMask));
        }

#if (CAPTURE_FORMAT == TIVX_TYPE_RAW_IMAGE)
        ASSERT_VX_OBJECT(raw_image_exemplar = tivxCreateRawImage(context, &(sensorParams.sensorInfo.raw_params)), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
#endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        //Create events for Sync
#if (CAPTURE1_ENABLE == 1U)
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventCreate(&eventHandle_RxFinished));
        // Setting up task params for capture_task
        tivxTaskSetDefaultCreateParams(&taskParams_capture);
        taskParams_capture.task_main = &tivxTask_capture;
        taskParams_capture.app_var = csirx_graph;
        taskParams_capture.stack_ptr = NULL;
        taskParams_capture.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
        taskParams_capture.core_affinity = TIVX_TASK_AFFINITY_ANY;
        taskParams_capture.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY1;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle_capture, &taskParams_capture));
#endif
#if (CAPTURE2_ENABLE == 1U)
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventCreate(&eventHandle_RxFinished1));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventCreate(&eventHandle_SensorCfgDone));
        // Setting up task params for capture_task 1
        tivxTaskSetDefaultCreateParams(&taskParams_capture);
        taskParams_capture.task_main = &tivxTask_capture1;
        taskParams_capture.app_var = csirx_graph1;
        taskParams_capture.stack_ptr = NULL;
        taskParams_capture.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
        taskParams_capture.core_affinity = TIVX_TASK_AFFINITY_ANY;
        taskParams_capture.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY1;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle_capture1, &taskParams_capture));
#endif

    //Wait for both Graph Processing to complete
#if (CAPTURE1_ENABLE == 1U)
        tivxEventWait(eventHandle_RxFinished, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);
#endif
#if (CAPTURE2_ENABLE == 1U)
        tivxEventWait(eventHandle_RxFinished1, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);
#endif

        appDeInitImageSensor(sensor_name);

#if (CAPTURE_FORMAT == TIVX_TYPE_RAW_IMAGE)
        VX_CALL(tivxReleaseRawImage(&raw_image_exemplar));
#endif

#if (CAPTURE1_ENABLE == 1U)
        VX_CALL(vxReleaseGraph(&csirx_graph));
#endif
#if (CAPTURE2_ENABLE == 1U)
        VX_CALL(vxReleaseGraph(&csirx_graph1));
#endif

#if (CAPTURE1_ENABLE == 1U)
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventDelete(&eventHandle_RxFinished));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle_capture));
#endif
#if (CAPTURE2_ENABLE == 1U)
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventDelete(&eventHandle_RxFinished1));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventDelete(&eventHandle_SensorCfgDone));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle_capture1));
#endif
        tivxHwaUnLoadKernels(context);

        tivx_clr_debug_zone(VX_ZONE_INFO);
    }
}

TESTCASE_TESTS(tivxHwaCaptureSplitMode,
               testCaptureSplitModeTest)

#endif /* BUILD_CAPTURE */