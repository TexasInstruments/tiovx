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
#define NUM_CAPT_CHANNELS                   (3U)
#define CAPT_INST_ID                       (0U)
#define CHANNEL_SWITCH_FRAME_COUNT          (300u)
#define NUM_BUFS                            (4u)

TESTCASE(tivxHwaCaptureVpacDisplay, CT_VXContext, ct_setup_vx_context, 0)

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
    CT_EXPAND(nextmacro(testArgName "/dataFormat=VX_DF_IMAGE_U16", __VA_ARGS__,\
        VX_DF_IMAGE_U16))
#define ADD_IN_WIDTH(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/inWidth=1936", __VA_ARGS__, 1936))
#define ADD_IN_HEIGHT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/inHeight=1100", __VA_ARGS__, 1100))
#define ADD_BPP_4(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/bpp=2", __VA_ARGS__, 2))   /* Unused */
#define ADD_PITCH_Y(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pitchY=3872", __VA_ARGS__, 3872))
#define ADD_PITCH_UV(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pitchUV=3872", __VA_ARGS__, 3872))
#define ADD_OUT_WIDTH(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/outWidth=1280", __VA_ARGS__, 1280))
#define ADD_OUT_HEIGHT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/outHeight=720", __VA_ARGS__, 720))
#define ADD_POS_X(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/posX=320", __VA_ARGS__, 320))
#define ADD_POS_Y(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/posY=180", __VA_ARGS__, 180))
#define ADD_LOOP_100(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loopCount=1188000", __VA_ARGS__, 1188000))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("Display Node", ADD_PIPE, ADD_DATA_FORMAT, \
        ADD_IN_WIDTH, ADD_IN_HEIGHT, ADD_BPP_4, ADD_PITCH_Y, ADD_PITCH_UV, \
        ADD_OUT_WIDTH, ADD_OUT_HEIGHT, ADD_POS_X, ADD_POS_Y, ADD_LOOP_100, ARG)

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node,
    vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

/*
 * Utility API to set pipeline depth for a graph
 */
static vx_status set_graph_pipeline_depth(vx_graph graph,
    vx_uint32 pipeline_depth)
{
    return tivxSetGraphPipelineDepth(graph, pipeline_depth);
}

/* Running capture display loop back using pipelining and no streaming */
TEST_WITH_ARG(tivxHwaCaptureVpacDisplay, testCaptureVpacDisplayLoopback, Arg,
    PARAMETERS)
{
    /* Common Params */
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_image sample_nv12_img = NULL;
    uint32_t num_refs, buf_id, num_buf, loop_id;
    uint32_t loop_count = arg_->loopCount;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];

    /* Capture Params */
    tivx_raw_image sample_raw_img = NULL;
    tivx_raw_image raw_img = NULL;
    vx_object_array capt_frames[MAX_NUM_BUF];
    vx_user_data_object capture_param_obj;
    tivx_capture_params_t capture_params;
    tivx_raw_image_create_params_t raw_params;
    vx_node captureNode = 0;
    AppSensorCmdParams cmdPrms;
    vx_reference refs[1];

    /* VISS Params */
    vx_image viss_nv12_out_img = NULL;
    vx_object_array viss_out_frames;
    tivx_vpac_viss_params_t viss_params;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_ae_awb_params_t ae_awb_params;
    vx_node vissNode = 0;
    vx_bool viss_prms_replicate[] =
        {vx_false_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e,
         vx_true_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e};

    /* Scaler Params */
    vx_image scaler_nv12_out_img = NULL;
    vx_object_array scaler_out_frames;
    vx_user_data_object sc_coeff_obj;
    tivx_vpac_msc_coefficients_t sc_coeffs;
    vx_node scalerNode = 0;
    vx_bool scaler_prms_replicate[] =
        {vx_true_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e};

    /* Display Params */
    tivx_display_params_t display_params;
    vx_user_data_object display_param_obj;
    vx_node displayNode = 0;
    vx_user_data_object switch_ch_obj;
    tivx_display_select_channel_params_t channel_prms;

    if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1)) &&
        (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_CAPTURE1)) &&
        (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1)) )
    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        raw_params.width = arg_->inWidth;
        raw_params.height = arg_->inHeight;
        raw_params.num_exposures = 1;
        raw_params.line_interleaved = vx_false_e;
        raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
        raw_params.format[0].msb = 11;
        raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_16_BIT;
        raw_params.format[1].msb = 11;
        raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
        raw_params.format[2].msb = 11;
        raw_params.meta_height_before = 0;
        raw_params.meta_height_after = 0;

        ASSERT_VX_OBJECT(sample_raw_img =
           tivxCreateRawImage(context, &raw_params),
           (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        /* Allocate frames */
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            ASSERT_VX_OBJECT(capt_frames[buf_id] =
                vxCreateObjectArray(context,
                (vx_reference)sample_raw_img, NUM_CAPT_CHANNELS),
                VX_TYPE_OBJECT_ARRAY);
        }
        VX_CALL(tivxReleaseRawImage(&sample_raw_img));

        /***************** Capture initialization *****************/
        tivx_capture_params_init(&capture_params);
        capture_params.numInst                          = 1U;
        capture_params.numCh                            = NUM_CAPT_CHANNELS;
        capture_params.instId[0U]                       = CAPT_INST_ID;
        capture_params.instCfg[0U].enableCsiv2p0Support = (uint32_t)vx_true_e;
        capture_params.instCfg[0U].numDataLanes         = 4U;
        for (loop_id=0U; loop_id < capture_params.instCfg[0U].numDataLanes; loop_id++)
        {
            capture_params.instCfg[0U].dataLanesMap[loop_id] = loop_id+1;
        }
        for (loop_id = 0U; loop_id < NUM_CAPT_CHANNELS; loop_id++)
        {
            capture_params.chVcNum[loop_id]   = loop_id;
            capture_params.chInstMap[loop_id] = CAPT_INST_ID;
        }

        ASSERT_VX_OBJECT(capture_param_obj =
            vxCreateUserDataObject(context, "tivx_capture_params_t" ,
            sizeof(tivx_capture_params_t), &capture_params),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(captureNode =
            tivxCaptureNode(graph, capture_param_obj, capt_frames[0]),
            VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(captureNode, VX_TARGET_STRING,
            TIVX_TARGET_CAPTURE1));

        /***************** VISS Initialization *****************/

        /* Allocate sample NV12 image, using which object array of NV12
         * would be created */
        ASSERT_VX_OBJECT(sample_nv12_img =
            vxCreateImage(context, arg_->inWidth, arg_->inHeight,
            VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Allocate object array for the output frames */
        ASSERT_VX_OBJECT(viss_out_frames = vxCreateObjectArray(context,
            (vx_reference)sample_nv12_img, NUM_CAPT_CHANNELS),
            VX_TYPE_OBJECT_ARRAY);

        /* Sample image is no longer required */
        VX_CALL(vxReleaseImage(&sample_nv12_img));

        /* Get one NV12 image for VISS and Display initialization */
        if (NULL != viss_out_frames)
        {
           ASSERT_VX_OBJECT(viss_nv12_out_img = (vx_image) vxGetObjectArrayItem(viss_out_frames, 0), VX_TYPE_IMAGE);
        }

        memset(&viss_params, 0, sizeof(tivx_vpac_viss_params_t));
        ASSERT_VX_OBJECT(configuration =
            vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
            sizeof(tivx_vpac_viss_params_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* VISS Initialize parameters */
        tivx_vpac_viss_params_init(&viss_params);

        viss_params.sensor_dcc_id = 0;
        viss_params.use_case = 0;
        viss_params.fcp[0].ee_mode = 0;
        viss_params.fcp[0].mux_output0 = 4;
        viss_params.fcp[0].mux_output1 = 0;
        viss_params.fcp[0].mux_output2 = 4;
        viss_params.fcp[0].mux_output3 = 0;
        viss_params.fcp[0].mux_output4 = 3;
        viss_params.bypass_glbce = 1;
        viss_params.bypass_nsf4 = 1;
        viss_params.h3a_in = 0;
        viss_params.h3a_aewb_af_mode = 0;
        viss_params.fcp[0].chroma_mode = 0;

        VX_CALL(vxCopyUserDataObject(configuration, 0,
            sizeof(tivx_vpac_viss_params_t), &viss_params, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result =
            vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0,
            sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        /* Get sample RAW Image */
        raw_img = (tivx_raw_image) vxGetObjectArrayItem(capt_frames[0], 0);

        ASSERT_VX_OBJECT(vissNode =
            tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
            raw_img, NULL, NULL, viss_nv12_out_img, NULL, NULL,
            NULL, NULL, NULL, NULL), VX_TYPE_NODE);
        VX_CALL(tivxReleaseRawImage(&raw_img));

        VX_CALL(tivxSetNodeParameterNumBufByIndex(vissNode, 6u, NUM_BUFS));

        VX_CALL(vxSetNodeTarget(vissNode, VX_TARGET_STRING,
            TIVX_TARGET_VPAC_VISS1));

        /* Now Replicate VISS Node */
        VX_CALL(vxReplicateNode(graph, vissNode, viss_prms_replicate, 11u));


        /***************** MSC Initialization *****************/

        /* Allocate sample NV12 image, using which object array of NV12
         * would be created */
        ASSERT_VX_OBJECT(sample_nv12_img =
            vxCreateImage(context, arg_->outWidth, arg_->outHeight,
            VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Allocate object array for the output frames */
        ASSERT_VX_OBJECT(scaler_out_frames = vxCreateObjectArray(context,
            (vx_reference)sample_nv12_img, NUM_CAPT_CHANNELS),
            VX_TYPE_OBJECT_ARRAY);

        /* Sample image is no longer required */
        VX_CALL(vxReleaseImage(&sample_nv12_img));

        /* Get one NV12 image for Scaler and Display initialization */
        if (NULL != scaler_out_frames)
        {
            ASSERT_VX_OBJECT(scaler_nv12_out_img =
                (vx_image)vxGetObjectArrayItem(scaler_out_frames, 0), VX_TYPE_IMAGE);
        }

        ASSERT_VX_OBJECT(scalerNode = tivxVpacMscScaleNode(graph,
            viss_nv12_out_img, scaler_nv12_out_img, NULL, NULL, NULL, NULL),
            VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(scalerNode,
            VX_TARGET_STRING, TIVX_TARGET_VPAC_MSC1));

        VX_CALL(tivxSetNodeParameterNumBufByIndex(scalerNode, 1u, NUM_BUFS));

        /* Now Replicate Scaler Node */
        VX_CALL(vxReplicateNode(graph, scalerNode, scaler_prms_replicate, 6u));

        /***************** Display initialization *****************/

        memset(&display_params, 0, sizeof(tivx_display_params_t));
        display_params.opMode=TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;
        display_params.pipeId=arg_->pipeId;
        display_params.outWidth=arg_->outWidth;
        display_params.outHeight=arg_->outHeight;
        display_params.posX=arg_->posX;
        display_params.posY=arg_->posY;

        ASSERT_VX_OBJECT(display_param_obj =
            vxCreateUserDataObject(context, "tivx_display_params_t",
            sizeof(tivx_display_params_t), &display_params),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(displayNode =
            tivxDisplayNode(graph, display_param_obj, scaler_nv12_out_img),
            VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(displayNode, VX_TARGET_STRING,
            TIVX_TARGET_DISPLAY1));

        /* Create User Data object for channel switching */
        channel_prms.active_channel_id = 0;
        ASSERT_VX_OBJECT(switch_ch_obj = vxCreateUserDataObject(context,
            "tivx_display_select_channel_params_t",
            sizeof(tivx_display_select_channel_params_t), &channel_prms),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        refs[0] = (vx_reference)switch_ch_obj;

        /* Frame is parameter number 1 for Capture Node and
         * becomes graph parameter 0 */
        add_graph_parameter_by_node_index(graph, captureNode, 1);

        /* Set graph schedule config such that graph parameter @ index 0 is
         * enqueuable */
        graph_parameters_queue_params_list[0].graph_parameter_index = 0;
        graph_parameters_queue_params_list[0].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[0].refs_list =
            (vx_reference*)&capt_frames[0];

        VX_CALL(tivxSetGraphPipelineDepth(graph, NUM_BUFS));

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

        /* Set Scaler Coefficients, can be called only after verify Graph */
        tivx_vpac_msc_coefficients_params_init(&sc_coeffs, VX_INTERPOLATION_BILINEAR);

        ASSERT_VX_OBJECT(sc_coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(sc_coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &sc_coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)sc_coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(scalerNode, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        /* Enqueue buf for pipe up but don't trigger graph execution */
        for(buf_id=0; buf_id<NUM_BUFS-2; buf_id++)
        {
            tivxGraphParameterEnqueueReadyRef(graph, 0,
                (vx_reference*)&capt_frames[buf_id], 1,
                TIVX_GRAPH_PARAMETER_ENQUEUE_FLAG_PIPEUP);
        }

        /* After pipe up, now enqueue a buffer to trigger graph scheduling */
        vxGraphParameterEnqueueReadyRef(graph, 0,
            (vx_reference*)&capt_frames[NUM_BUFS-2], 1);

        cmdPrms.numSensors = NUM_CAPT_CHANNELS;
        /*cmdPrms.portNum = 1U;
        for (loop_id = 0U; loop_id < cmdPrms.portNum; loop_id++)
        {
            cmdPrms.portIdMap[loop_id] = CAPT_INST_ID;
        }*/
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, appRemoteServiceRun(APP_IPC_CPU_MCU2_1,
            APP_REMOTE_SERVICE_SENSOR_NAME,
            APP_REMOTE_SERVICE_SENSOR_CMD_CONFIG_IMX390, &cmdPrms,
            sizeof(cmdPrms), 0));

        /* Need to trigger again since display holds on to a buffer */
        vxGraphParameterEnqueueReadyRef(graph, 0,
            (vx_reference*)&capt_frames[NUM_BUFS-1], 1);

        refs[0] = (vx_reference)switch_ch_obj;
        /* wait for graph instances to complete, compare output and
         * recycle data buffers, schedule again */
        for(loop_id=0; loop_id<(loop_count+NUM_BUFS); loop_id++)
        {
            vx_object_array frame;

            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(graph, 0,
                (vx_reference*)&frame, 1, &num_refs);

            /* Recycles dequeued input and output refs */
            /* input and output can be enqueued in any order */
            vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&frame, 1);

            if (0 == (loop_id % CHANNEL_SWITCH_FRAME_COUNT))
            {
                channel_prms.active_channel_id =
                    (channel_prms.active_channel_id + 1) % NUM_CAPT_CHANNELS;
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
        VX_CALL(vxReleaseNode(&vissNode));
        VX_CALL(vxReleaseNode(&scalerNode));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&display_param_obj));
        VX_CALL(vxReleaseUserDataObject(&capture_param_obj));
        VX_CALL(vxReleaseUserDataObject(&switch_ch_obj));
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            VX_CALL(vxReleaseObjectArray(&capt_frames[buf_id]));
        }
        VX_CALL(vxReleaseObjectArray(&viss_out_frames));
        VX_CALL(vxReleaseObjectArray(&scaler_out_frames));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&sc_coeff_obj));
        VX_CALL(vxReleaseImage(&viss_nv12_out_img));
        VX_CALL(vxReleaseImage(&scaler_nv12_out_img));

        ASSERT(displayNode == 0);
        ASSERT(captureNode == 0);
        ASSERT(scalerNode == 0);
        ASSERT(vissNode == 0);
        ASSERT(graph == 0);
        ASSERT(display_param_obj == 0);
        ASSERT(capture_param_obj == 0);
        ASSERT(switch_ch_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaCaptureVpacDisplay, testCaptureVpacDisplayLoopback)

#endif /* BUILD_CAPTURE & BUILD_DISPLAY */