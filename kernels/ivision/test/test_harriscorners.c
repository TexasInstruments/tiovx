/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
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
#include <VX/vxu.h>
#include <TI/tivx.h>
#include <TI/tivx_mem.h>
#include <float.h>
#include <math.h>

#include "tivx_harris_corners_test_data.h"

#include "test_tiovx_ivision.h"

TESTCASE(tivxIVisionHarrisCorners, CT_VXContext, ct_setup_vx_context, 0)

#define GRADIENT_SIZE       (7u)
#define BLOCK_SIZE          (7u)

#define IMG_WIDTH           (400u)
#define IMG_HEIGHT          (400u)

#define STRENGTH_THRESHOLD  (286870912)

#define MAX_CORNERS         (1023U)


vx_uint8 gTivxHcTestInput[TIVX_HC_TEST_INPUT_NUM_ELEM] = TIVX_HC_TEST_INPUT_CFG;

vx_int16 ReferenceOutput[TIVX_HC_NUM_REF_KEY_POINTS][2u] =
    TIVX_HC_TEST_REFERENCE_OUTPUT;

static void CheckOutput(vx_array arr)
{
    vx_status status;
    vx_size num_items;
    vx_uint32 cnt;
    vx_map_id map_id;
    vx_size stride = sizeof(vx_keypoint_t);
    void *base = NULL;
    vx_int16 x, y, num_missmatch = 0;

    status = vxQueryArray (arr, VX_ARRAY_NUMITEMS, &num_items, sizeof(num_items));
    if ((vx_status)VX_SUCCESS == status)
    {
        ASSERT(num_items == TIVX_HC_NUM_REF_KEY_POINTS);

        status = vxMapArrayRange(arr, 0, num_items, &map_id,
            &stride, &base, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);
        if ((vx_status)VX_SUCCESS == status)
        {

            for (cnt = 0u; cnt < num_items; cnt ++)
            {
                x = vxArrayItem(vx_keypoint_t, base, cnt, stride).x;
                y = vxArrayItem(vx_keypoint_t, base, cnt, stride).y;

                ASSERT(x == ReferenceOutput[cnt][0u]);
                ASSERT(y == ReferenceOutput[cnt][1u]);
            }

            vxUnmapArrayRange(arr, map_id);
        }

    }
    else
    {
        ASSERT(0);
    }
}

typedef struct {
    const char* testName;
    vx_uint32 scaling_factor;
    vx_int32  nms_threshold;
    vx_uint8  q_shift;
    vx_uint8  win_size;
    vx_uint8  score_method;
    vx_uint8  suppression_method;
} Arg;

#define ADD_SCALE_FACTOR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/SCALING_FACTOR=1310", __VA_ARGS__, 1310))
#define ADD_NMS_THR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/NMS_THRESHOLD=286870912", __VA_ARGS__, 286870912))
#define ADD_Q_SHIFT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/Q_SHIFT=2", __VA_ARGS__, 2))
#define ADD_WIN_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/WIN_SIZE=7", __VA_ARGS__, 7))
#define ADD_SCORE_METHOD(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/SCORE_METHOD=0", __VA_ARGS__, 0))
#define ADD_SUPPR_METHOD(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/SUPPRESSION_METHOD=7", __VA_ARGS__, 7))


#define PARAMETERS \
    CT_GENERATE_PARAMETERS("Test_HD_ON_EVE", ADD_SCALE_FACTOR, ADD_NMS_THR, ADD_Q_SHIFT, ADD_WIN_SIZE, ADD_SCORE_METHOD, ADD_SUPPR_METHOD, ARG)

TEST_WITH_ARG(tivxIVisionHarrisCorners, testHarrisCornerOnEve, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_size num_corners;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_imagepatch_addressing_t addrs;
    void *image = 0;
    vx_scalar num_corners_scalar;
    vx_array corners;
    vx_perf_t perf_node;
    vx_perf_t perf_graph;

    if ((vx_bool)vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_EVE1))
    {
        IVisionLoadKernels(context);

        image = tivxMemAlloc(IMG_WIDTH*IMG_HEIGHT, TIVX_MEM_EXTERNAL);
        if(image)
        {
            /* Copy input image to allocated buffer */
            memcpy(image, gTivxHcTestInput, IMG_WIDTH*IMG_HEIGHT);
        }
        else
        {
            printf("HarrisCorners: Cannot allocate memory !!!\n");
            return;
        }

        addrs.dim_x = IMG_WIDTH;
        addrs.dim_y = IMG_HEIGHT;
        addrs.stride_x = 1;
        addrs.stride_y = IMG_WIDTH;
        addrs.step_x = 1;
        addrs.step_y = 1;
        ASSERT_VX_OBJECT(input_image = vxCreateImageFromHandle(context,
            (vx_df_image)VX_DF_IMAGE_U8, &addrs, &image, VX_MEMORY_TYPE_HOST), VX_TYPE_IMAGE);

        num_corners = MAX_CORNERS;

        ASSERT_VX_OBJECT(num_corners_scalar = vxCreateScalar(context,
            VX_TYPE_SIZE, &num_corners), VX_TYPE_SCALAR);

        ASSERT_VX_OBJECT(corners = vxCreateArray(context, VX_TYPE_KEYPOINT,
            num_corners), VX_TYPE_ARRAY);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxHarrisCornersNode(graph,
            input_image, arg_->scaling_factor, arg_->nms_threshold,
            arg_->q_shift, arg_->win_size, arg_->score_method,
            arg_->suppression_method,
            corners, num_corners_scalar), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_EVE1));
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
        vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

        CheckOutput(corners);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseArray(&corners));
        VX_CALL(vxReleaseImage(&input_image));
        VX_CALL(vxReleaseScalar(&num_corners_scalar));

        if (image)
        {
            tivxMemFree(image, IMG_WIDTH*IMG_HEIGHT, TIVX_MEM_EXTERNAL);
            image = NULL;
        }

        ASSERT(corners == 0);
        ASSERT(num_corners_scalar == 0);
        ASSERT(input_image == 0);

        IVisionUnLoadKernels(context);

        IVisionPrintPerformance(perf_node, IMG_WIDTH*IMG_HEIGHT, "N0");
        IVisionPrintPerformance(perf_graph, IMG_WIDTH*IMG_HEIGHT, "G0");
    }
}



TESTCASE_TESTS(tivxIVisionHarrisCorners, testHarrisCornerOnEve)
