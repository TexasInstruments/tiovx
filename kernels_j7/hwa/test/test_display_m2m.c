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
#include <utils/mem/include/app_mem.h>
#include "test_tiovx/test_tiovx.h"
#include "test_hwa_common.h"

#define TIVX_TARGET_DEFAULT_STACK_SIZE      (256U * 1024U)
#define TIVX_TARGET_DEFAULT_TASK_PRIORITY1   (8u)

#define DSS_M2M_CH0_WB_PIPE_INST_ID                 (0U)
#define DSS_M2M_CH0_PIPE_NUM                        (1U)
#define DSS_M2M_CH0_PIPE_INST_ID                    (0U)
#define DSS_M2M_CH0_OVERLAY_ID                      (0U)
#define DSS_M2M_CH0_IN_FRAME_FORMAT                 (VX_DF_IMAGE_YUYV)
#define DSS_M2M_CH0_IN_FRAME_WIDTH                  (1920U)
#define DSS_M2M_CH0_IN_FRAME_HEIGHT                 (1080U)
#define DSS_M2M_CH0_OUT_FRAME_FORMAT                (VX_DF_IMAGE_NV12)
#define DSS_M2M_CH0_OUT_FRAME_WIDTH                 (1920U)
#define DSS_M2M_CH0_OUT_FRAME_HEIGHT                (1080U)
#define DSS_M2M_CH0_POSX                            (0U)
#define DSS_M2M_CH0_POSY                            (0U)

TESTCASE(tivxHwaDisplayM2M, CT_VXContext, ct_setup_vx_context, 0)

static vx_context context;
static uint32_t loop_cnt;
static tivx_event eventHandle_TaskFinished;

typedef struct {
    uint32_t instId;
    uint32_t numPipe;
    uint32_t pipeId[TIVX_DISPLAY_M2M_MAX_PIPE];
    uint32_t overlayId;
    vx_df_image inFmt;
    uint32_t inWidth;
    uint32_t inHeight;
    vx_df_image outFmt;
    uint32_t outWidth;
    uint32_t outHeight;
    uint32_t posX;
    uint32_t posY;
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

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

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
    vx_graph m2m_graph = 0;
    tivx_display_m2m_test_params_t *testParams =
                    (tivx_display_m2m_test_params_t *)app_var;


    ASSERT_VX_OBJECT(m2m_graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    printf("Graph 1: created...\n");

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

    printf("Graph 1: input and output images created...\n");

    /* DSS M2M initialization */
    tivx_display_m2m_params_init(&local_m2m_config);

    ASSERT_VX_OBJECT(m2m_config = vxCreateUserDataObject(context,
                                        "tivx_display_m2m_params_t",
                                        sizeof(tivx_display_m2m_params_t),
                                        &local_m2m_config),
                                        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(m2m_node = tivxDisplayM2MNode(m2m_graph, m2m_config, in_image, out_image), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(m2m_node, VX_TARGET_STRING, TIVX_TARGET_DISPLAY_M2M));

    printf("Graph 1: verifying...\n");

    VX_CALL(vxVerifyGraph(m2m_graph));

    printf("Graph 1: verify done...\n");

    /* input @ node index 0, becomes csitx_graph parameter 1 */
    add_graph_parameter_by_node_index(m2m_graph, m2m_node, 1);

    printf("Graph 1: Added M2M Node to the graph...\n");

    for (wbFrmCnt = 0U ; wbFrmCnt < loop_cnt ; wbFrmCnt++)
    {
        VX_CALL(vxProcessGraph(m2m_graph));
    }

    VX_CALL(vxReleaseNode(&m2m_node));
    VX_CALL(vxReleaseImage(&in_image));
    VX_CALL(vxReleaseImage(&out_image));
    VX_CALL(vxReleaseUserDataObject(&m2m_config));

    VX_CALL(vxReleaseGraph(&m2m_graph));

    printf("Graph 1: released...\n");

    /*Signal the completion of m2m graph processing*/
    tivxEventPost(eventHandle_TaskFinished);
}

TEST_WITH_ARG(tivxHwaDisplayM2M, tivxHwaDisplayM2Mtest, Arg, PARAMETERS)
{
    tivx_task taskHandle_m2m;
    tivx_task_create_params_t taskParams_m2m;
    tivx_display_m2m_test_params_t testParams1;

    context = context_->vx_context_;

    printf("Starting Display M2M  Conformance Test...\n");
    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY_M2M))
    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        tivx_clr_debug_zone(VX_ZONE_INFO);

        loop_cnt = arg_->loopCount;

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventCreate(&eventHandle_TaskFinished));

        /* Initialize test parameters for task */
        testParams1.instId     = DSS_M2M_CH0_WB_PIPE_INST_ID;
        testParams1.numPipe    = DSS_M2M_CH0_PIPE_NUM;
        /* Note: Directly assigning as only one pipe is supported currently */
        testParams1.pipeId[0U] = DSS_M2M_CH0_PIPE_INST_ID;
        testParams1.overlayId  = DSS_M2M_CH0_OVERLAY_ID;
        testParams1.inFmt      = DSS_M2M_CH0_IN_FRAME_FORMAT;
        testParams1.inWidth    = DSS_M2M_CH0_IN_FRAME_WIDTH;
        testParams1.inHeight   = DSS_M2M_CH0_IN_FRAME_HEIGHT;
        testParams1.outFmt     = DSS_M2M_CH0_OUT_FRAME_FORMAT;
        testParams1.outWidth   = DSS_M2M_CH0_OUT_FRAME_WIDTH;
        testParams1.outHeight  = DSS_M2M_CH0_OUT_FRAME_HEIGHT;
        testParams1.posX       = DSS_M2M_CH0_POSX;
        testParams1.posY       = DSS_M2M_CH0_POSY;

        /* Setting up task params for m2m_task */
        tivxTaskSetDefaultCreateParams(&taskParams_m2m);
        taskParams_m2m.task_main = &tivxTask_m2m;
        taskParams_m2m.app_var = &testParams1;
        taskParams_m2m.stack_ptr = NULL;
        taskParams_m2m.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
        taskParams_m2m.core_affinity = TIVX_TASK_AFFINITY_ANY;
        taskParams_m2m.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY1;
        /* Create Tasks */
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle_m2m, &taskParams_m2m));

        tivxEventWait(eventHandle_TaskFinished, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle_m2m));

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxEventDelete(&eventHandle_TaskFinished));

        tivxHwaUnLoadKernels(context);

        tivx_clr_debug_zone(VX_ZONE_INFO);
    }

    printf("Display M2M  Conformance Test Finished...\n");
}

TESTCASE_TESTS(tivxHwaDisplayM2M,
               tivxHwaDisplayM2Mtest)
