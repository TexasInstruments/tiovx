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
#if defined (BUILD_CAPTURE) && defined(BUILD_DISPLAY)

#include <VX/vx.h>
#include <TI/tivx.h>
#include "test_engine/test.h"
#include <string.h>

#include <utils/sensors/include/app_sensors.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/ipc/include/app_ipc.h>
#include "test_hwa_common.h"


#define MAX_NUM_BUF                         (8u)

/* Number of capture channels per port/instance  */
#define NUM_CAPT_CHANNELS                   (4u)
/* Number of instances to use   */
#define NUM_CAPT_INST                       (1U)
#define CHANNEL_SWITCH_FRAME_COUNT          (150u)

TESTCASE(tivxHwaCaptureDisplay, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    uint32_t pipeId;
    uint32_t dataFormat;
    uint32_t inWidth;
    uint32_t inHeight;
    uint32_t bpp;
    uint32_t pitchY;
    uint32_t pitchUV;
    uint32_t outWidth;
    uint32_t outHeight;
    uint32_t posX;
    uint32_t posY;
    uint32_t loopCount;
} Arg;

#define ADD_PIPE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pipeId=2", __VA_ARGS__, 2))
#define ADD_DATA_FORMAT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dataFormat=VX_DF_IMAGE_U16", __VA_ARGS__, VX_DF_IMAGE_U16))
#define ADD_IN_WIDTH(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/inWidth=1920", __VA_ARGS__, 1936))
#define ADD_IN_HEIGHT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/inHeight=1080", __VA_ARGS__, 1100))
#define ADD_BPP_2(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/bpp=2", __VA_ARGS__, 2))
#define ADD_BPP_4(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/bpp=4", __VA_ARGS__, 4))
#define ADD_PITCH_Y(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pitchY=3840", __VA_ARGS__, 3872))
#define ADD_PITCH_UV(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pitchUV=3840", __VA_ARGS__, 3872))
#define ADD_OUT_WIDTH(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/outWidth=1920", __VA_ARGS__, 1920))
#define ADD_OUT_HEIGHT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/outHeight=1080", __VA_ARGS__, 1080))
#define ADD_POS_X(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/posX=0", __VA_ARGS__, 0))
#define ADD_POS_Y(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/posY=0", __VA_ARGS__, 0))
#define ADD_LOOP_1000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loopCount=1000", __VA_ARGS__, 1000))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("Display Node", ADD_PIPE, ADD_DATA_FORMAT, ADD_IN_WIDTH, ADD_IN_HEIGHT, ADD_BPP_2, ADD_PITCH_Y, ADD_PITCH_UV, ADD_OUT_WIDTH, ADD_OUT_HEIGHT, ADD_POS_X, ADD_POS_Y, ADD_LOOP_1000, ARG), \

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

/* Running capture display loop back using pipelining and no streaming */
TEST_WITH_ARG(tivxHwaCaptureDisplay, testCaptureDisplayLoopback1, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image sample_image;
    vx_object_array frames[MAX_NUM_BUF];
    tivx_display_params_t display_params;
    vx_user_data_object display_param_obj;
    vx_graph graph = 0;
    vx_node displayNode = 0, captureNode = 0;
    vx_user_data_object capture_param_obj;
    tivx_capture_params_t capture_params;
    uint32_t num_refs, buf_id, num_buf, num_channels=(NUM_CAPT_CHANNELS * NUM_CAPT_INST), loop_id;
    uint32_t instIdx, chIdx;
    uint32_t loop_count = arg_->loopCount;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];
    AppSensorCmdParams cmdPrms;
    vx_reference refs[1];
    vx_user_data_object switch_ch_obj;
    tivx_display_select_channel_params_t channel_prms;

    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1));
    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_CAPTURE1));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(sample_image = vxCreateImage(context, arg_->inWidth, arg_->inHeight, arg_->dataFormat), VX_TYPE_IMAGE);

        /* Setting number of buffers for capture display use case */
        num_buf = 4;

        /* Allocate frames */
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            ASSERT_VX_OBJECT(frames[buf_id] = vxCreateObjectArray(context, (vx_reference)sample_image, num_channels), VX_TYPE_OBJECT_ARRAY);
        }
        VX_CALL(vxReleaseImage(&sample_image));

        /* Capture initialization */
        tivx_capture_params_init(&capture_params);
        capture_params.numInst = NUM_CAPT_INST;
        capture_params.numCh   =
                                (NUM_CAPT_CHANNELS * NUM_CAPT_INST);
        chIdx = 0U;
        for (instIdx = 0U ; instIdx < NUM_CAPT_INST ; instIdx++)
        {
            capture_params.instId[instIdx] = instIdx;
            capture_params.instCfg[instIdx].enableCsiv2p0Support = (uint32_t)vx_true_e;
            capture_params.instCfg[instIdx].numDataLanes         = 4U;
            for (loop_id = 0U; loop_id < capture_params.instCfg[0U].numDataLanes ; loop_id++)
            {
                capture_params.instCfg[instIdx].dataLanesMap[loop_id] = (loop_id + 1u);
            }
            for (loop_id = 0U; loop_id < NUM_CAPT_CHANNELS; loop_id++)
            {
                capture_params.chVcNum[chIdx]   = loop_id;
                capture_params.chInstMap[chIdx] = instIdx;
                chIdx++;
            }
        }

        ASSERT_VX_OBJECT(capture_param_obj = vxCreateUserDataObject(context, "tivx_capture_params_t" , sizeof(tivx_capture_params_t), &capture_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(captureNode = tivxCaptureNode(graph, capture_param_obj, frames[0]), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(captureNode, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1));

        /* Display initialization */
        memset(&display_params, 0, sizeof(tivx_display_params_t));
        display_params.opMode=TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;
        display_params.pipeId=arg_->pipeId;
        display_params.outWidth=arg_->outWidth;
        display_params.outHeight=arg_->outHeight;
        display_params.posX=arg_->posX;
        display_params.posY=arg_->posY;

        ASSERT_VX_OBJECT(display_param_obj = vxCreateUserDataObject(context, "tivx_display_params_t", sizeof(tivx_display_params_t), &display_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        sample_image = (vx_image) vxGetObjectArrayItem(frames[0], 0);
        ASSERT_VX_OBJECT(displayNode = tivxDisplayNode(graph, display_param_obj, sample_image), VX_TYPE_NODE);
        VX_CALL(vxReleaseImage(&sample_image));

        VX_CALL(vxSetNodeTarget(displayNode, VX_TARGET_STRING, TIVX_TARGET_DISPLAY1));

        /* Create User Data object for channel switching */
        channel_prms.active_channel_id = 0;
        ASSERT_VX_OBJECT(switch_ch_obj = vxCreateUserDataObject(context,
            "tivx_display_select_channel_params_t",
            sizeof(tivx_display_select_channel_params_t), &channel_prms),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        refs[0] = (vx_reference)switch_ch_obj;

        /* Frame is parameter number 1 for Capture Node and becomes graph parameter 0 */
        add_graph_parameter_by_node_index(graph, captureNode, 1);

        /* Set graph schedule config such that graph parameter @ index 0 is enqueuable */
        graph_parameters_queue_params_list[0].graph_parameter_index = 0;
        graph_parameters_queue_params_list[0].refs_list_size = num_buf;
        graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&frames[0];

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

        /* Enqueue buf for pipe up but don't trigger graph execution */
        for(buf_id=0; buf_id<num_buf-2; buf_id++)
        {
            tivxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&frames[buf_id], 1, TIVX_GRAPH_PARAMETER_ENQUEUE_FLAG_PIPEUP);
        }

        /* After pipe up, now enqueue a buffer to trigger graph scheduling */
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&frames[num_buf-2], 1);

        /* After first trigger, configure and start the sensor */
        cmdPrms.numSensors = NUM_CAPT_CHANNELS;
        /*cmdPrms.portNum = NUM_CAPT_INST;
        for (loop_id = 0U; loop_id < cmdPrms.portNum; loop_id++)
        {
            cmdPrms.portIdMap[loop_id] = loop_id;
        }*/
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, appRemoteServiceRun(APP_IPC_CPU_MCU2_1,
            APP_REMOTE_SERVICE_SENSOR_NAME,
            APP_REMOTE_SERVICE_SENSOR_CMD_CONFIG_IMX390, &cmdPrms, sizeof(cmdPrms), 0));

        /* Need to trigger again since display holds on to a buffer */
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&frames[num_buf-1], 1);

        /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
        for(loop_id=0; loop_id<(loop_count+num_buf); loop_id++)
        {
            vx_object_array frame;
            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&frame, 1, &num_refs);

            /* Recycles dequeued input and output refs */
            /* input and output can be enqueued in any order */
            vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&frame, 1);

            if (0 == (loop_id % CHANNEL_SWITCH_FRAME_COUNT))
            {
                channel_prms.active_channel_id =
                    (channel_prms.active_channel_id + 1) % num_channels;
                VX_CALL(vxCopyUserDataObject(switch_ch_obj, 0,
                    sizeof(tivx_display_select_channel_params_t),
                    &channel_prms, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                VX_CALL(tivxNodeSendCommand(displayNode, 0,
                    TIVX_DISPLAY_SELECT_CHANNEL, refs, 1u));
            }
        }

        /* ensure all graph processing is complete */
        vxWaitGraph(graph);

        VX_CALL(vxReleaseNode(&displayNode));
        VX_CALL(vxReleaseNode(&captureNode));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&display_param_obj));
        VX_CALL(vxReleaseUserDataObject(&capture_param_obj));
        VX_CALL(vxReleaseUserDataObject(&switch_ch_obj));
        for(buf_id=0; buf_id<num_buf; buf_id++)
        {
            VX_CALL(vxReleaseObjectArray(&frames[buf_id]));
        }

        ASSERT(displayNode == 0);
        ASSERT(captureNode == 0);
        ASSERT(graph == 0);
        ASSERT(display_param_obj == 0);
        ASSERT(capture_param_obj == 0);
        ASSERT(switch_ch_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaCaptureDisplay, testCaptureDisplayLoopback1)

#endif /* BUILD_CAPTURE & BUILD_DISPLAY */
