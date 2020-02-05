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

#include <VX/vx.h>
#include <TI/tivx.h>
#include <TI/j7.h>
#include "test_engine/test.h"
#include <TI/tivx_config.h>
#include <string.h>
#include <TI/tivx_task.h>
#include <TI/tivx_event.h>
#include "math.h"
#include <limits.h>
#include "test_tiovx/test_tiovx.h"
#include "test_hwa_common.h"

#define MAX_NUM_BUF         (8u)
#define CAPTURE_MIN_PIPEUP_BUFS (3u)

#define NUM_CHANNELS        (1U)
#define CSITX_INST_ID       (0U)
#define CAPT_INST_ID        (0U)

#define TIVX_TARGET_DEFAULT_STACK_SIZE      (64U * 1024U)
#define TIVX_TARGET_DEFAULT_TASK_PRIORITY1   (8u)

TESTCASE(tivxHwaCsitxCsirx, CT_VXContext, ct_setup_vx_context, 0)

static tivx_event  eventHandle_TxStart;
static tivx_event  eventHandle_TxFinished;
static tivx_event  eventHandle_RxFinished;

static vx_context context;
static tivx_raw_image raw_image_exemplar;
static uint32_t width, height, loop_cnt, num_buf;
static vx_rectangle_t rect;

typedef struct {
    const char* name;
    uint32_t Width;
    uint32_t Height;
    uint32_t loopCount;
} Arg;

#define ADD_WIDTH(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/Width=1920", __VA_ARGS__, 1920))
#define ADD_HEIGHT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/Height=1080", __VA_ARGS__, 1080))
#define ADD_LOOP_1000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loopCount=1000", __VA_ARGS__, 1000))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("CsitxCsirx", ADD_WIDTH, ADD_HEIGHT, ADD_LOOP_1000 ,ARG), \


/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

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
        ASSERT_VX_OBJECT(capture_frames[buf_id] = vxCreateObjectArray(context, (vx_reference)raw_image_exemplar, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);
    }

    /* CSIRX Config initialization */
    tivx_capture_params_init(&local_capture_config);
    local_capture_config.numInst                          = 1U;
    local_capture_config.numCh                            = NUM_CHANNELS;
    local_capture_config.instId[0U]                       = CAPT_INST_ID;
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
        local_capture_config.chInstMap[loopCnt] = CAPT_INST_ID;
    }

    ASSERT_VX_OBJECT(capture_config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &local_capture_config), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(csirx_node = tivxCaptureNode(csirx_graph, capture_config, capture_frames[0]), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(csirx_node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

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

    /* enqueue capture buffers for pipeup but dont trigger graph executions */
    for(buf_id=0; buf_id<num_buf-(CAPTURE_MIN_PIPEUP_BUFS-1); buf_id++)
    {
        tivxGraphParameterEnqueueReadyRef(csirx_graph, 0, (vx_reference*)&capture_frames[buf_id], 1, TIVX_GRAPH_PARAMETER_ENQUEUE_FLAG_PIPEUP);
    }

    /*Now enqueue a buffer to trigger csirx_graph scheduling */
    vxGraphParameterEnqueueReadyRef(csirx_graph, 0, (vx_reference*)&capture_frames[num_buf-(CAPTURE_MIN_PIPEUP_BUFS-1)], 1);

    /*Signal to start the csitx graph processing*/
    tivxEventPost(eventHandle_TxStart);

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

    tivx_raw_image captured_frame_array_item=0;
    vx_map_id map_id;
    vx_int32 i,j;
    vx_imagepatch_addressing_t addr;
    uint16_t *ptr2 = NULL;
    uint16_t frame_cnt = 0;

    /* Check data in captured image*/
    /* Find which frame is present in the first buffer, by comparing the starting data for each frame*/
    ASSERT_VX_OBJECT(captured_frame_array_item = (tivx_raw_image)vxGetObjectArrayItem(capture_frames[0] , 0), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
    VX_CALL(tivxMapRawImagePatch(captured_frame_array_item, &rect, 0, &map_id, &addr, (void **)&ptr2,
                                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, TIVX_RAW_IMAGE_PIXEL_BUFFER));
    for(frame_cnt=0; frame_cnt<num_buf; frame_cnt++)
    {
        if(ptr2[0]==frame_cnt)
        {
            break;
        }
    }
    VX_CALL(tivxUnmapRawImagePatch(captured_frame_array_item, map_id));   
    VX_CALL(tivxReleaseRawImage(&captured_frame_array_item));
    /*Check if the starting data in first buffer dosent correspond to any frame*/
    ASSERT(frame_cnt < num_buf);
    /* Check data in all buffers*/
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        ASSERT_VX_OBJECT(captured_frame_array_item = (tivx_raw_image)vxGetObjectArrayItem(capture_frames[buf_id] , 0), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxMapRawImagePatch(captured_frame_array_item, &rect, 0, &map_id, &addr, (void **)&ptr2,
                                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, TIVX_RAW_IMAGE_PIXEL_BUFFER));
        for (i = 0; i <height; i++)
        {
            for(j=0; j<width; j++)
            {
                ASSERT(ptr2[(i*width)+j] == (j + frame_cnt));
            }
        }
        frame_cnt++;
        frame_cnt = (frame_cnt  == num_buf)? 0 : frame_cnt;
        VX_CALL(tivxUnmapRawImagePatch(captured_frame_array_item, map_id));   
        VX_CALL(tivxReleaseRawImage(&captured_frame_array_item));
    }
    

    VX_CALL(vxReleaseNode(&csirx_node));
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        VX_CALL(vxReleaseObjectArray(&capture_frames[buf_id]));
    }
    VX_CALL(vxReleaseUserDataObject(&capture_config));

    /*Signal the completion of csirx graph processing*/
    tivxEventPost(eventHandle_RxFinished);
}

static void VX_CALLBACK tivxTask_csitx(void *app_var)
{
    vx_node csitx_node = 0;
    tivx_raw_image tx_frame_array_item=0;
    vx_object_array tx_frame = 0;
    vx_user_data_object csitx_config;
    tivx_csitx_params_t local_csitx_config;
    uint32_t buf_id, loop_id, loopCnt;
    vx_graph_parameter_queue_params_t csitx_graph_parameters_queue_params_list[1];
    vx_map_id map_id;
    vx_int32 i,j;
    vx_imagepatch_addressing_t addr;
    uint16_t *ptr = NULL;
    uint16_t frame_cnt = 0;

    vx_graph csitx_graph = (vx_graph)app_var;

    /* allocate Input and Output refs*/
    ASSERT_VX_OBJECT(tx_frame = vxCreateObjectArray(context, (vx_reference)raw_image_exemplar, NUM_CHANNELS), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(tx_frame_array_item = (tivx_raw_image)vxGetObjectArrayItem(tx_frame , 0), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

    /* Initialize raw_image with running pattern using WRITE ONLY MAP */
    VX_CALL(tivxMapRawImagePatch(tx_frame_array_item, &rect, 0, &map_id, &addr, (void **)&ptr,
                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, TIVX_RAW_IMAGE_PIXEL_BUFFER));
    ASSERT(ptr != NULL);

    for (i = 0; i <height; i++)
    {
        for(j=0; j<width; j++)
        {
            ptr[(i*width)+j] = j + frame_cnt;
        }
    }
    frame_cnt++;
    VX_CALL(tivxUnmapRawImagePatch(tx_frame_array_item, map_id));

    /* CSITX Config initialization */
    tivx_csitx_params_init(&local_csitx_config);
    local_csitx_config.numInst                          = 1U;
    local_csitx_config.numCh                            = NUM_CHANNELS;
    local_csitx_config.instId[0U]                       = CSITX_INST_ID;
    local_csitx_config.instCfg[0U].rxCompEnable         = (uint32_t)vx_true_e;
    local_csitx_config.instCfg[0U].rxv1p3MapEnable      = (uint32_t)vx_true_e;
    local_csitx_config.instCfg[0U].numDataLanes         = 4U;
    for (loopCnt = 0U ;
        loopCnt < local_csitx_config.instCfg[0U].numDataLanes ;
        loopCnt++)
    {
        local_csitx_config.instCfg[0U].lanePolarityCtrl[loopCnt] = 0u;
    }
    for (loopCnt = 0U; loopCnt < NUM_CHANNELS; loopCnt++)
    {
        local_csitx_config.chVcNum[loopCnt]   = loopCnt;
        local_csitx_config.chInstMap[loopCnt] = CSITX_INST_ID;
    }

    ASSERT_VX_OBJECT(csitx_config = vxCreateUserDataObject(context, "tivx_csitx_params_t", sizeof(tivx_csitx_params_t), &local_csitx_config), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(csitx_node = tivxCsitxNode(csitx_graph, csitx_config, tx_frame), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(csitx_node, VX_TARGET_STRING, TIVX_TARGET_CSITX));

    /* input @ node index 0, becomes csitx_graph parameter 1 */
    add_graph_parameter_by_node_index(csitx_graph, csitx_node, 1);

    /* set csitx_graph schedule config such that csitx_graph parameter @ index 0 and 1 are enqueuable */
    csitx_graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    csitx_graph_parameters_queue_params_list[0].refs_list_size = 1;
    csitx_graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&tx_frame;

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
    * Graph gets scheduled automatically as refs are enqueued to it
    */
    vxSetGraphScheduleConfig(csitx_graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            1,
            csitx_graph_parameters_queue_params_list
            );

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(csitx_graph));

    /* Pend sync event here,Wait for csirx graph to complete enqueue frames
    so that Tx task can start graph processing  */
    tivxEventWait(eventHandle_TxStart, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

    /* Now enqueue a buffer to trigger csitx_graph scheduling */
    vxGraphParameterEnqueueReadyRef(csitx_graph, 0, (vx_reference*)&tx_frame, 1);

    /* wait for csitx_graph instances to complete, schedule again */
    /* loop_id limit is loop_count + no.of capture buffers - num.of pipeup capture buffers*/
    for(loop_id=0; loop_id<(loop_cnt + num_buf - CAPTURE_MIN_PIPEUP_BUFS) ; loop_id++)
    {
        uint32_t num_refs;
        vx_object_array transmitted_frames = NULL;

        /* Get tramsnitted frame reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(csitx_graph, 0, (vx_reference*)&transmitted_frames, 1, &num_refs);

        /*To transmit 4(num_buf) different frames,Update each frame to be transmitted*/
        VX_CALL(tivxMapRawImagePatch(tx_frame_array_item, &rect, 0, &map_id, &addr, (void **)&ptr,
                                        VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, TIVX_RAW_IMAGE_PIXEL_BUFFER));
        ASSERT(ptr != NULL);

        for (i = 0; i <height; i++)
        {
            for(j=0; j<width; j++)
            {
                ptr[(i*width)+j] = (j + frame_cnt);
            }
        }
        frame_cnt++;
        frame_cnt = (frame_cnt  == num_buf)? 0 : frame_cnt;
        VX_CALL(tivxUnmapRawImagePatch(tx_frame_array_item, map_id));

        vxGraphParameterEnqueueReadyRef(csitx_graph, 0, (vx_reference*)&transmitted_frames, 1);

    }
    /* ensure all csitx_graph processing is complete */
    vxWaitGraph(csitx_graph);

    VX_CALL(vxReleaseNode(&csitx_node));
    VX_CALL(vxReleaseObjectArray(&tx_frame));
    VX_CALL(tivxReleaseRawImage(&tx_frame_array_item));
    VX_CALL(vxReleaseUserDataObject(&csitx_config));

    /*Signal the completion of csitx graph processing*/
    tivxEventPost(eventHandle_TxFinished);
}

TEST_WITH_ARG(tivxHwaCsitxCsirx, testCsitxCsirxloopback, Arg, PARAMETERS)
{
    vx_graph csitx_graph = 0 , csirx_graph = 0 ;
    tivx_raw_image_create_params_t params;
    tivx_task_create_params_t taskParams_csitx, taskParams_capture;
    tivx_task taskHandle_csitx, taskHandle_capture;

    context = context_->vx_context_;

    if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_CSITX)) &&
        (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_CAPTURE1)))
    {
        /* Setting to num buf */
        num_buf = 4;

        loop_cnt = arg_->loopCount;
        width = arg_->Width;
        height = arg_->Height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        tivx_clr_debug_zone(VX_ZONE_INFO);

        ASSERT_VX_OBJECT(csirx_graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(csitx_graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        params.width = width;
        params.height = height;
        params.num_exposures = 1;
        params.line_interleaved = vx_true_e;
        params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
        params.format[0].msb = 12;
        params.meta_height_before = 0;
        params.meta_height_after = 0;

        ASSERT_VX_OBJECT(raw_image_exemplar = tivxCreateRawImage(context, &params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        // Setting up task params for capture_task
        tivxTaskSetDefaultCreateParams(&taskParams_capture);
        taskParams_capture.task_main = &tivxTask_capture;
        taskParams_capture.app_var = csirx_graph;
        taskParams_capture.stack_ptr = NULL;
        taskParams_capture.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
        taskParams_capture.core_affinity = TIVX_TASK_AFFINITY_ANY;
        taskParams_capture.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY1;

        // Setting up task params for csitx_task
        tivxTaskSetDefaultCreateParams(&taskParams_csitx);
        taskParams_csitx.task_main = &tivxTask_csitx;
        taskParams_csitx.app_var = csitx_graph;
        taskParams_csitx.stack_ptr = NULL;
        taskParams_csitx.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
        taskParams_csitx.core_affinity = TIVX_TASK_AFFINITY_ANY;
        taskParams_csitx.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY1;

        //Create events for Sync
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventCreate(&eventHandle_TxStart));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventCreate(&eventHandle_TxFinished));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventCreate(&eventHandle_RxFinished));

        //Create Capture and Csitx Tasks
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle_capture, &taskParams_capture));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle_csitx, &taskParams_csitx));

        //Wait for both Graph Processing to complete
        tivxEventWait(eventHandle_TxFinished, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);
        tivxEventWait(eventHandle_RxFinished, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

        VX_CALL(tivxReleaseRawImage(&raw_image_exemplar));

        VX_CALL(vxReleaseGraph(&csitx_graph));
        VX_CALL(vxReleaseGraph(&csirx_graph));

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventDelete(&eventHandle_TxStart));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventDelete(&eventHandle_TxFinished));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventDelete(&eventHandle_RxFinished));

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle_capture));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle_csitx));

        tivxHwaUnLoadKernels(context);

        tivx_clr_debug_zone(VX_ZONE_INFO);
    }
}

TESTCASE_TESTS(tivxHwaCsitxCsirx,
               testCsitxCsirxloopback)

