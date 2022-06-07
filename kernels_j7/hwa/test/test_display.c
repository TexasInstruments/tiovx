/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
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
#ifdef BUILD_DISPLAY

#include <VX/vx.h>
#include <TI/tivx.h>
#include "test_engine/test.h"
#include <string.h>
#include "test_hwa_common.h"

#define DISPLAY_NUM_RUN_COUNT 100

extern uint32_t gTiovxCtDisplayArray1[];
extern uint32_t gTiovxCtDisplayArray2[];

TESTCASE(tivxHwaDisplay, CT_VXContext, ct_setup_vx_context, 0)

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
    uint32_t enableCrop;
} Arg;

#define ADD_PIPE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pipeId=2", __VA_ARGS__, 2))

#define ADD_DATA_FORMAT_RGBX(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dataFormat=VX_DF_IMAGE_RGBX", __VA_ARGS__, VX_DF_IMAGE_RGBX))
#define ADD_DATA_FORMAT_BGRX(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dataFormat=TIVX_DF_IMAGE_BGRX", __VA_ARGS__, TIVX_DF_IMAGE_BGRX))
#define ADD_DATA_FORMAT_UYVY(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dataFormat=VX_DF_IMAGE_UYVY", __VA_ARGS__, VX_DF_IMAGE_UYVY))
#define ADD_DATA_FORMAT_YUYV(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dataFormat=VX_DF_IMAGE_YUYV", __VA_ARGS__, VX_DF_IMAGE_YUYV))

#define ADD_IN_WIDTH(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/inWidth=480", __VA_ARGS__, 480))
#define ADD_IN_HEIGHT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/inHeight=360", __VA_ARGS__, 360))

#define ADD_BPP_4(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/bpp=4", __VA_ARGS__, 4))
#define ADD_BPP_2(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/bpp=2", __VA_ARGS__, 2))

#define ADD_PITCH_Y(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pitchY=1920", __VA_ARGS__, 1920))
#define ADD_PITCH_UV(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pitchUV=0", __VA_ARGS__, 0))
#define ADD_PITCH_YUV(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pitchY=3840", __VA_ARGS__, 3840))

#define ADD_OUT_WIDTH(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/outWidth=480", __VA_ARGS__, 480))
#define ADD_OUT_HEIGHT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/outHeight=360", __VA_ARGS__, 360))
#define ADD_POS_X(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/posX=800", __VA_ARGS__, 800))
#define ADD_POS_Y(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/posY=440", __VA_ARGS__, 440))
#define ADD_LOOP_100(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loopCount=100", __VA_ARGS__, 100))
#define ADD_LOOP_1000(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/loopCount=1000", __VA_ARGS__, 1000))

#define ADD_DISABLE_CROP(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/enableCrop=0", __VA_ARGS__, 0))
#define ADD_ENABLE_CROP(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/enableCrop=1", __VA_ARGS__, 1))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("Display Node RGBX", ADD_PIPE, ADD_DATA_FORMAT_RGBX, ADD_IN_WIDTH, ADD_IN_HEIGHT, ADD_BPP_4, ADD_PITCH_Y, ADD_PITCH_UV, ADD_OUT_WIDTH, ADD_OUT_HEIGHT, ADD_POS_X, ADD_POS_Y, ADD_LOOP_1000, ADD_DISABLE_CROP, ARG), \
    CT_GENERATE_PARAMETERS("Display Node BGRX", ADD_PIPE, ADD_DATA_FORMAT_BGRX, ADD_IN_WIDTH, ADD_IN_HEIGHT, ADD_BPP_4, ADD_PITCH_Y, ADD_PITCH_UV, ADD_OUT_WIDTH, ADD_OUT_HEIGHT, ADD_POS_X, ADD_POS_Y, ADD_LOOP_1000, ADD_ENABLE_CROP, ARG), \
    CT_GENERATE_PARAMETERS("Display Node UYVY", ADD_PIPE, ADD_DATA_FORMAT_UYVY, ADD_IN_WIDTH, ADD_IN_HEIGHT, ADD_BPP_2, ADD_PITCH_YUV, ADD_PITCH_UV, ADD_OUT_WIDTH, ADD_OUT_HEIGHT, ADD_POS_X, ADD_POS_Y, ADD_LOOP_1000, ADD_DISABLE_CROP, ARG), \
    CT_GENERATE_PARAMETERS("Display Node YUYV", ADD_PIPE, ADD_DATA_FORMAT_YUYV, ADD_IN_WIDTH, ADD_IN_HEIGHT, ADD_BPP_2, ADD_PITCH_YUV, ADD_PITCH_UV, ADD_OUT_WIDTH, ADD_OUT_HEIGHT, ADD_POS_X, ADD_POS_Y, ADD_LOOP_1000, ADD_DISABLE_CROP, ARG)

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

TEST_WITH_ARG(tivxHwaDisplay, testBufferCopyMode, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image disp_image = 0;
    vx_imagepatch_addressing_t image_addr;
    tivx_display_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    uint32_t loop_count = arg_->loopCount;

    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(disp_image = vxCreateImage(context, arg_->inWidth, arg_->inHeight, arg_->dataFormat), VX_TYPE_IMAGE);

        image_addr.dim_x = arg_->inWidth;
        image_addr.dim_y = arg_->inHeight;
        image_addr.stride_x = arg_->bpp;
        image_addr.stride_y = arg_->pitchY;
        image_addr.scale_x = VX_SCALE_UNITY;
        image_addr.scale_y = VX_SCALE_UNITY;
        image_addr.step_x = 1;
        image_addr.step_y = 1;
        vx_rectangle_t rect;
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = arg_->inWidth;
        rect.end_y = arg_->inHeight;

        vxCopyImagePatch(disp_image,
                &rect,
                0,
                &image_addr,
                (void *)gTiovxCtDisplayArray1,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST
                );

        memset(&params, 0, sizeof(tivx_display_params_t));

        params.opMode=TIVX_KERNEL_DISPLAY_BUFFER_COPY_MODE;
        params.pipeId=arg_->pipeId;
        params.outWidth=arg_->outWidth;
        params.outHeight=arg_->outHeight;
        params.posX=arg_->posX;
        params.posY=arg_->posY;

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_display_params_t", sizeof(tivx_display_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxDisplayNode(graph, param_obj, disp_image), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_DISPLAY1));
        VX_CALL(vxVerifyGraph(graph));

        while(loop_count-- > 0)
        {
            VX_CALL(vxProcessGraph(graph));
            if((loop_count%2) == 1)
            {
                vxCopyImagePatch(disp_image,
                    &rect,
                    0,
                    &image_addr,
                    (void *)gTiovxCtDisplayArray2,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST
                    );
            }
            else
            {
                vxCopyImagePatch(disp_image,
                    &rect,
                    0,
                    &image_addr,
                    (void *)gTiovxCtDisplayArray1,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST
                    );
            }
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&disp_image));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(disp_image == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaDisplay, testZeroBufferCopyMode, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image disp_image[2];
    vx_imagepatch_addressing_t image_addr;
    tivx_display_params_t params;
    vx_user_data_object param_obj;
    vx_user_data_object crop_obj = NULL;
    tivx_display_crop_params_t crop_params;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image disp_image_temp;
    uint32_t num_refs;
    uint32_t loop_count = arg_->loopCount;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[1];

    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(disp_image[0] = vxCreateImage(context, arg_->inWidth, arg_->inHeight, arg_->dataFormat), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(disp_image[1] = vxCreateImage(context, arg_->inWidth, arg_->inHeight, arg_->dataFormat), VX_TYPE_IMAGE);

        image_addr.dim_x = arg_->inWidth;
        image_addr.dim_y = arg_->inHeight;
        image_addr.stride_x = arg_->bpp;
        image_addr.stride_y = arg_->pitchY;
        image_addr.scale_x = VX_SCALE_UNITY;
        image_addr.scale_y = VX_SCALE_UNITY;
        image_addr.step_x = 1;
        image_addr.step_y = 1;
        vx_rectangle_t rect;
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = arg_->inWidth;
        rect.end_y = arg_->inHeight;

        vxCopyImagePatch(disp_image[0],
                &rect,
                0,
                &image_addr,
                (void *)gTiovxCtDisplayArray1,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST
                );
        vxCopyImagePatch(disp_image[1],
                &rect,
                0,
                &image_addr,
                (void *)gTiovxCtDisplayArray2,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST
                );

        memset(&params, 0, sizeof(tivx_display_params_t));

        params.opMode=TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;
        params.pipeId=arg_->pipeId;
        params.outWidth=arg_->outWidth;
        params.outHeight=arg_->outHeight;
        params.posX=arg_->posX;
        params.posY=arg_->posY;

        if (1 == arg_->enableCrop)
        {
            params.enableCropping = 1;
            params.cropPrms.startX = 16;
            params.cropPrms.startY = 16;
            params.cropPrms.width = arg_->outWidth - 16;
            params.cropPrms.height = arg_->outHeight - 16;

            ASSERT_VX_OBJECT(crop_obj =
                vxCreateUserDataObject(context, "tivx_display_crop_params_t", sizeof(tivx_display_crop_params_t), &params.cropPrms), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        }

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_display_params_t", sizeof(tivx_display_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxDisplayNode(graph, param_obj, disp_image[0]), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_DISPLAY1));

        /* Image is parameter number 1 for Display Node and becomes graph parameter 0 */
        add_graph_parameter_by_node_index(graph, node, 1);

        /* Set graph schedule config such that graph parameter @ index 0 is enqueuable */
        graph_parameters_queue_params_list[0].graph_parameter_index = 0;
        graph_parameters_queue_params_list[0].refs_list_size = 2; /* Two images */
        graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&disp_image[0];

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        /* Explicitly set graph pipeline depth */
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_pipeline_depth(graph, 2));

        VX_CALL(vxVerifyGraph(graph));

        /* Enqueue input references */
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&disp_image[0], 1);
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&disp_image[1], 1);

        /* Wait for the loop count */
        while(loop_count-- > 0)
        {
            /* Dequeue one image buffer */
            vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&disp_image_temp, 1, &num_refs);

            if ((1 == arg_->enableCrop) && (loop_count == (arg_->loopCount / 2)))
            {
                vx_reference refs[1];

                params.cropPrms.startX = 100;
                params.cropPrms.startY = 100;
                params.cropPrms.width = arg_->outWidth - 100;
                params.cropPrms.height = arg_->outHeight - 100;

                vxCopyUserDataObject(crop_obj, 0,
                    sizeof(tivx_display_crop_params_t),
                    &params.cropPrms, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

                refs[0] = (vx_reference)crop_obj;
                tivxNodeSendCommand(node, 0, TIVX_DISPLAY_SET_CROP_PARAMS,
                    refs, 1u);
            }

            /* Enqueue the same image buffer */
            vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&disp_image_temp, 1);
        }

        /* ensure all graph processing is complete */
        vxWaitGraph(graph);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&disp_image[0]));
        VX_CALL(vxReleaseImage(&disp_image[1]));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        if (1 == arg_->enableCrop)
        {
            VX_CALL(vxReleaseUserDataObject(&crop_obj));
        }

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(disp_image[0] == 0);
        ASSERT(disp_image[1] == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaDisplay,
    testBufferCopyMode,
    testZeroBufferCopyMode
    )

#endif /* BUILD_DISPLAY */