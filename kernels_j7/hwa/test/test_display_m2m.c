/*
 *
 * Copyright (c) 2021 Texas Instruments Incorporated
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
#include <TI/tivx_config.h>
#include <string.h>
#include <TI/tivx_task.h>
#include <TI/tivx_event.h>
#include "math.h"
#include <limits.h>
#include <utils/mem/include/app_mem.h>
#include "test_tiovx/test_tiovx.h"
#include "test_hwa_common.h"

#define TIVX_TARGET_DEFAULT_STACK_SIZE      (256U * 1024U)
#define TIVX_TARGET_DEFAULT_TASK_PRIORITY1   (8u)

#define DSS_M2M_NUM_CH                              (2U)
#define DSS_M2M_NUM_CH_MAX                          (4U)

/* Common Configurations across channels */
#define DSS_M2M_WB_PIPE_INST_ID                     (0U)
#define DSS_M2M_PIPE_NUM                            (1U)
#define DSS_M2M_PIPE_INST_ID                        (3U)
/* Currently Only Overlay2 can be used for M2M operations,
   this can be changed through DSS initialization API available in vision_apps */
#define DSS_M2M_OVERLAY_ID                          (3U)

/* Channel 0 configurations */
#define DSS_M2M_CH0_IN_FRAME_FORMAT                 (VX_DF_IMAGE_YUYV)
#define DSS_M2M_CH0_IN_FRAME_WIDTH                  (1920U)
#define DSS_M2M_CH0_IN_FRAME_HEIGHT                 (1080U)
#define DSS_M2M_CH0_IN_FRAME_BPP                    (2U)
#define DSS_M2M_CH0_IN_FRAME_PITCH                  (DSS_M2M_CH0_IN_FRAME_WIDTH * \
                                                     DSS_M2M_CH0_IN_FRAME_BPP)
#define DSS_M2M_CH0_OUT_FRAME_FORMAT                (VX_DF_IMAGE_NV12)
#define DSS_M2M_CH0_OUT_FRAME_WIDTH                 (1920U)
#define DSS_M2M_CH0_OUT_FRAME_HEIGHT                (1080U)

/* Channel 1 configurations */
#define DSS_M2M_CH1_IN_FRAME_FORMAT                 (VX_DF_IMAGE_RGB)
#define DSS_M2M_CH1_IN_FRAME_WIDTH                  (1920U)
#define DSS_M2M_CH1_IN_FRAME_HEIGHT                 (1080U)
#define DSS_M2M_CH1_IN_FRAME_BPP                    (3U)
#define DSS_M2M_CH1_IN_FRAME_PITCH                  (DSS_M2M_CH1_IN_FRAME_WIDTH * \
                                                     DSS_M2M_CH1_IN_FRAME_BPP)
#define DSS_M2M_CH1_OUT_FRAME_FORMAT                (VX_DF_IMAGE_NV12)
#define DSS_M2M_CH1_OUT_FRAME_WIDTH                 (1920U)
#define DSS_M2M_CH1_OUT_FRAME_HEIGHT                (1080U)

/* Channel 2 configurations */
#define DSS_M2M_CH2_IN_FRAME_FORMAT                 (VX_DF_IMAGE_NV12)
#define DSS_M2M_CH2_IN_FRAME_WIDTH                  (1920U)
#define DSS_M2M_CH2_IN_FRAME_HEIGHT                 (1080U)
#define DSS_M2M_CH2_IN_FRAME_BPP                    (1U)
#define DSS_M2M_CH2_IN_FRAME_PITCH                  (DSS_M2M_CH2_IN_FRAME_WIDTH * \
                                                     DSS_M2M_CH2_IN_FRAME_BPP)
#define DSS_M2M_CH2_OUT_FRAME_FORMAT                (VX_DF_IMAGE_RGB)
#define DSS_M2M_CH2_OUT_FRAME_WIDTH                 (1920U)
#define DSS_M2M_CH2_OUT_FRAME_HEIGHT                (1080U)

/* Channel 3 configurations */
#define DSS_M2M_CH3_IN_FRAME_FORMAT                 (VX_DF_IMAGE_NV12)
#define DSS_M2M_CH3_IN_FRAME_WIDTH                  (1920U)
#define DSS_M2M_CH3_IN_FRAME_HEIGHT                 (1080U)
#define DSS_M2M_CH3_IN_FRAME_BPP                    (1U)
#define DSS_M2M_CH3_IN_FRAME_PITCH                  (DSS_M2M_CH3_IN_FRAME_WIDTH * \
                                                     DSS_M2M_CH3_IN_FRAME_BPP)
#define DSS_M2M_CH3_OUT_FRAME_FORMAT                (VX_DF_IMAGE_YUYV)
#define DSS_M2M_CH3_OUT_FRAME_WIDTH                 (1920U)
#define DSS_M2M_CH3_OUT_FRAME_HEIGHT                (1080U)

#define DSS_M2M_NODE_NAME_LEN_MAX                   (100U)

/* Preloaded image buffers */
/* 1920 x 1080 buffers */
extern uint32_t gTiovxCtDisplayArrayBGR888[1555200];
extern uint32_t gTiovxCtDisplayArrayYUV420NV12[777600];
extern uint32_t gTiovxCtDisplayArrayYUV422[1036800];

TESTCASE(tivxHwaDisplayM2M, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    uint32_t taskId;
    uint32_t instId;
    uint32_t numPipe;
    uint32_t pipeId[TIVX_DISPLAY_M2M_MAX_PIPE];
    uint32_t overlayId;
    vx_df_image inFmt;
    uint32_t inWidth;
    uint32_t inHeight;
    uint32_t inBpp;
    uint32_t inPitch;
    vx_df_image outFmt;
    uint32_t outWidth;
    uint32_t outHeight;
    uint32_t posX;
    uint32_t posY;
    tivx_event eventHandle_TaskFinished;
    tivx_task taskHandle_m2m;
    tivx_task_create_params_t taskParams_m2m;
    char nodeName[DSS_M2M_NODE_NAME_LEN_MAX];
    uint32_t iterationCnt;
} tivx_display_m2m_test_params_t;

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
    CT_GENERATE_PARAMETERS("DisplayM2M", ADD_WIDTH, ADD_HEIGHT, ADD_LOOP_1000 ,ARG), \

static vx_context context;
static uint32_t gLoop_cnt;
tivx_display_m2m_test_params_t gTestParams[DSS_M2M_NUM_CH_MAX];

static void VX_CALLBACK tivxTask_m2m(void *app_var)
{
    vx_node m2m_node = 0;
    vx_image in_image = 0;
    vx_image out_image = 0;
    vx_user_data_object m2m_config;
    tivx_display_m2m_params_t local_m2m_config;
    uint32_t wbFrmCnt = 0U;
    vx_graph_parameter_queue_params_t m2m_graph_parameters_queue_params_list[1];
    vx_map_id map_id;
    vx_int32 i,j;
    vx_imagepatch_addressing_t addr;
    uint16_t *ptr = NULL;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_graph m2m_graph = 0;
    tivx_display_m2m_test_params_t *testParams =
                    (tivx_display_m2m_test_params_t *)app_var;


    ASSERT_VX_OBJECT(m2m_graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    printf("Graph %d: created...\n", testParams->taskId);

    /* allocate Input and Output frame refs */
    ASSERT_VX_OBJECT(in_image = vxCreateImage(context,
                                              testParams->inWidth,
                                              testParams->inHeight,
                                              testParams->inFmt),
                                              VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(out_image = vxCreateImage(context,
                                               testParams->outWidth,
                                               testParams->outHeight,
                                               testParams->outFmt),
                                               VX_TYPE_IMAGE);

    printf("Graph %d: input and output images created...\n", testParams->taskId);

    image_addr.dim_x    = testParams->inWidth;
    image_addr.dim_y    = testParams->inHeight;
    image_addr.stride_x = testParams->inBpp;
    image_addr.stride_y = testParams->inPitch;
    image_addr.scale_x  = VX_SCALE_UNITY;
    image_addr.scale_y  = VX_SCALE_UNITY;
    image_addr.step_x   = 1;
    image_addr.step_y   = 1;
    rect.start_x        = 0;
    rect.start_y        = 0;
    rect.end_x          = testParams->inWidth;
    rect.end_y          = testParams->inHeight;

    /* Copy reference input image to input buffer */
    if (testParams->inFmt == VX_DF_IMAGE_RGB)
    {
        vxCopyImagePatch(in_image,
                         &rect,
                         0,
                         &image_addr,
                         (void *)gTiovxCtDisplayArrayBGR888,
                         VX_WRITE_ONLY,
                         VX_MEMORY_TYPE_HOST);
    }
    else if (testParams->inFmt == VX_DF_IMAGE_YUYV)
    {
        vxCopyImagePatch(in_image,
                         &rect,
                         0,
                         &image_addr,
                         (void *)gTiovxCtDisplayArrayYUV422,
                         VX_WRITE_ONLY,
                         VX_MEMORY_TYPE_HOST);
    }
    else
    {
        /* Update input buffer here for other formats */
    }

    /* DSS M2M initialization */
    tivx_display_m2m_params_init(&local_m2m_config);
    local_m2m_config.instId     = testParams->instId;
    /* Only one pipeline is supported */
    local_m2m_config.numPipe    = testParams->numPipe;
    local_m2m_config.pipeId[0U] = testParams->pipeId[0U];
    local_m2m_config.overlayId  = testParams->overlayId;

    ASSERT_VX_OBJECT(m2m_config = vxCreateUserDataObject(context,
                                        "tivx_display_m2m_params_t",
                                        sizeof(tivx_display_m2m_params_t),
                                        &local_m2m_config),
                                        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(m2m_node = tivxDisplayM2MNode(m2m_graph, m2m_config, in_image, out_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(m2m_node, VX_TARGET_STRING, &testParams->nodeName[0U]));

    printf("Added \'%s\' node in graph %d\n", &testParams->nodeName[0U], testParams->taskId);

    printf("Graph %d: verifying...\n", testParams->taskId);

    VX_CALL(vxVerifyGraph(m2m_graph));

    printf("Graph %d: verify done...\n", testParams->taskId);

    for (wbFrmCnt = 0U ; wbFrmCnt < testParams->iterationCnt ; wbFrmCnt++)
    {
        VX_CALL(vxProcessGraph(m2m_graph));
    }

    VX_CALL(vxReleaseNode(&m2m_node));
    VX_CALL(vxReleaseImage(&in_image));
    VX_CALL(vxReleaseImage(&out_image));
    VX_CALL(vxReleaseUserDataObject(&m2m_config));

    VX_CALL(vxReleaseGraph(&m2m_graph));

    printf("Graph %d: released...\n", testParams->taskId);

    /*Signal the completion of m2m graph processing*/
    tivxEventPost(testParams->eventHandle_TaskFinished);
}

TEST_WITH_ARG(tivxHwaDisplayM2M, tivxHwaDisplayM2Mtest, Arg, PARAMETERS)
{
    context = context_->vx_context_;
    uint32_t taskIdx;
    tivx_display_m2m_test_params_t *testParams;
    uint32_t createTask = 0U;

    printf("Starting Display M2M  Conformance Test...\n");

    /* Initialize global test parameters structure to '0' */
    memset(&gTestParams[0U], 0, sizeof(gTestParams));

    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY_M2M1));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        tivx_clr_debug_zone(VX_ZONE_INFO);

        gLoop_cnt = arg_->loopCount;

        for (taskIdx = 0U ; taskIdx < DSS_M2M_NUM_CH ; taskIdx++)
        {
            createTask         = 0U;
            testParams         = &gTestParams[taskIdx];
            testParams->taskId = taskIdx;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventCreate(&testParams->eventHandle_TaskFinished));
            testParams->instId     = DSS_M2M_WB_PIPE_INST_ID;
            testParams->numPipe    = DSS_M2M_PIPE_NUM;
            /* Note: Directly assigning as only one pipe is supported currently */
            testParams->pipeId[0U] = DSS_M2M_PIPE_INST_ID;
            testParams->overlayId  = DSS_M2M_OVERLAY_ID;
            testParams->inFmt      = DSS_M2M_CH0_IN_FRAME_FORMAT;
            testParams->iterationCnt = (gLoop_cnt / (taskIdx + 1U));
            switch (taskIdx)
            {
                case 0U:
                    /* Initialize test parameters for task 0 */
                    createTask               = 1U;
                    testParams->inWidth      = DSS_M2M_CH0_IN_FRAME_WIDTH;
                    testParams->inHeight     = DSS_M2M_CH0_IN_FRAME_HEIGHT;
                    testParams->inBpp        = DSS_M2M_CH0_IN_FRAME_BPP;
                    testParams->inPitch      = DSS_M2M_CH0_IN_FRAME_PITCH;
                    testParams->outFmt       = DSS_M2M_CH0_OUT_FRAME_FORMAT;
                    testParams->outWidth     = DSS_M2M_CH0_OUT_FRAME_WIDTH;
                    testParams->outHeight    = DSS_M2M_CH0_OUT_FRAME_HEIGHT;
                    strcpy(&testParams->nodeName[0U], TIVX_TARGET_DISPLAY_M2M1);
                break;
                case 1U:
                    createTask               = 1U;
                    /* Initialize test parameters for task 1 */
                    testParams->inFmt        = DSS_M2M_CH1_IN_FRAME_FORMAT;
                    testParams->inWidth      = DSS_M2M_CH1_IN_FRAME_WIDTH;
                    testParams->inHeight     = DSS_M2M_CH1_IN_FRAME_HEIGHT;
                    testParams->inBpp        = DSS_M2M_CH1_IN_FRAME_BPP;
                    testParams->inPitch      = DSS_M2M_CH1_IN_FRAME_PITCH;
                    testParams->outFmt       = DSS_M2M_CH1_OUT_FRAME_FORMAT;
                    testParams->outWidth     = DSS_M2M_CH1_OUT_FRAME_WIDTH;
                    testParams->outHeight    = DSS_M2M_CH1_OUT_FRAME_HEIGHT;
                    strcpy(&testParams->nodeName[0U], TIVX_TARGET_DISPLAY_M2M2);
                break;
                case 2U:
                    createTask               = 1U;
                    /* Initialize test parameters for task 1 */
                    testParams->inFmt        = DSS_M2M_CH2_IN_FRAME_FORMAT;
                    testParams->inWidth      = DSS_M2M_CH2_IN_FRAME_WIDTH;
                    testParams->inHeight     = DSS_M2M_CH2_IN_FRAME_HEIGHT;
                    testParams->inBpp        = DSS_M2M_CH2_IN_FRAME_BPP;
                    testParams->inPitch      = DSS_M2M_CH2_IN_FRAME_PITCH;
                    testParams->outFmt       = DSS_M2M_CH2_OUT_FRAME_FORMAT;
                    testParams->outWidth     = DSS_M2M_CH2_OUT_FRAME_WIDTH;
                    testParams->outHeight    = DSS_M2M_CH2_OUT_FRAME_HEIGHT;
                    strcpy(&testParams->nodeName[0U], TIVX_TARGET_DISPLAY_M2M3);
                break;
                case 3U:
                    createTask               = 1U;
                    /* Initialize test parameters for task 1 */
                    testParams->inFmt        = DSS_M2M_CH3_IN_FRAME_FORMAT;
                    testParams->inWidth      = DSS_M2M_CH3_IN_FRAME_WIDTH;
                    testParams->inHeight     = DSS_M2M_CH3_IN_FRAME_HEIGHT;
                    testParams->inBpp        = DSS_M2M_CH3_IN_FRAME_BPP;
                    testParams->inPitch      = DSS_M2M_CH3_IN_FRAME_PITCH;
                    testParams->outFmt       = DSS_M2M_CH3_OUT_FRAME_FORMAT;
                    testParams->outWidth     = DSS_M2M_CH3_OUT_FRAME_WIDTH;
                    testParams->outHeight    = DSS_M2M_CH3_OUT_FRAME_HEIGHT;
                    strcpy(&testParams->nodeName[0U], TIVX_TARGET_DISPLAY_M2M4);
                break;
                default:
                break;
            }
            if (createTask == 1U)
            {
                /* Setting up task params for m2m_task */
                tivxTaskSetDefaultCreateParams(&testParams->taskParams_m2m);
                testParams->taskParams_m2m.task_main     = &tivxTask_m2m;
                testParams->taskParams_m2m.app_var       = testParams;
                testParams->taskParams_m2m.stack_ptr     = NULL;
                testParams->taskParams_m2m.stack_size    = TIVX_TARGET_DEFAULT_STACK_SIZE;
                testParams->taskParams_m2m.core_affinity = TIVX_TASK_AFFINITY_ANY;
                testParams->taskParams_m2m.priority      = TIVX_TARGET_DEFAULT_TASK_PRIORITY1;

                printf("Creating Task %d...\n", testParams->taskId);
                /* Create Tasks */
                ASSERT_EQ_VX_STATUS(VX_SUCCESS,
                                    tivxTaskCreate(&testParams->taskHandle_m2m,
                                    &testParams->taskParams_m2m));
            }
        }

        /* wait here for all tasks to finish */
        printf("Waiting for graphs to finish execution...\n");
        for (taskIdx = 0U ; taskIdx < DSS_M2M_NUM_CH ; taskIdx++)
        {
            testParams = &gTestParams[taskIdx];
            tivxEventWait(testParams->eventHandle_TaskFinished, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);
            printf("Received events from Task%d\n", testParams->taskId);
        }

        /* Delete tasks and sync events */
        for (taskIdx = 0U ; taskIdx < DSS_M2M_NUM_CH ; taskIdx++)
        {
            testParams = &gTestParams[taskIdx];
            ASSERT_EQ_VX_STATUS(VX_SUCCESS,
                                tivxTaskDelete(&testParams->taskHandle_m2m));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS,
                                tivxEventDelete(&testParams->eventHandle_TaskFinished));
        }

        tivxHwaUnLoadKernels(context);

        tivx_clr_debug_zone(VX_ZONE_INFO);
    }

    printf("Display M2M  Conformance Test Finished...\n");
}

TESTCASE_TESTS(tivxHwaDisplayM2M,
               tivxHwaDisplayM2Mtest)

#endif /* BUILD_DISPLAY */