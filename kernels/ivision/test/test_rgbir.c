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

#include "tivx_rgb_ir_test_data.h"

#include "test_tiovx_ivision.h"

TESTCASE(tivxIVisionRgbIr, CT_VXContext, ct_setup_vx_context, 0)

#define IMG_WIDTH           (160u)
#define IMG_HEIGHT          (120u)

vx_uint16 gTivxRgbIrTestInput[IMG_WIDTH*IMG_HEIGHT] = TIVX_RGB_IR_TEST_INPUT;

vx_uint16 gTivxRgbIrTestRefBayerOutput[IMG_WIDTH*IMG_HEIGHT] = TIVX_RGB_IR_TEST_REFERENCE_BAYER_OUTPUT;
vx_uint16 gTivxRgbIrTestRefIrOutput[(IMG_WIDTH*IMG_HEIGHT)/4] = TIVX_RGB_IR_TEST_REFERENCE_IR_OUTPUT;

static void CheckOutput(vx_image outBayer, vx_image outIR)
{
  vx_status status;
  vx_rectangle_t rect;
  void* pOut;
  vx_imagepatch_addressing_t addr_pattern;
  vx_map_id map_id;
  vx_uint32 x = 0;
  vx_uint32 y = 0;

  status= vxGetValidRegionImage(outBayer, &rect);
  status|= vxMapImagePatch(outBayer, &rect, 0, &map_id, &addr_pattern, &pOut, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

  if ((vx_status)VX_SUCCESS == status)
  {
    for (y=0; y < addr_pattern.dim_y; y++)
    {
      for (x=0; x < addr_pattern.dim_x; x++)
      {
        vx_uint16* vx_ptr = (vx_uint16*)vxFormatImagePatchAddress2d(pOut, x, y, &addr_pattern);
        ASSERT(vx_ptr[0] == gTivxRgbIrTestRefBayerOutput[x + y*IMG_WIDTH]);
      }
    }
    status= vxUnmapImagePatch(outBayer, map_id);

    status= vxGetValidRegionImage(outIR, &rect);
    status|= vxMapImagePatch(outIR, &rect, 0, &map_id, &addr_pattern, &pOut, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    if ((vx_status)VX_SUCCESS == status)
    {
      for (y=0; y < addr_pattern.dim_y; y++)
      {
        for (x=0; x < addr_pattern.dim_x; x++)
        {
          vx_uint16* vx_ptr = (vx_uint16*)vxFormatImagePatchAddress2d(pOut, x, y, &addr_pattern);
          ASSERT(vx_ptr[0] == gTivxRgbIrTestRefIrOutput[x + y*IMG_WIDTH/2]);
        }
      }

      status= vxUnmapImagePatch(outIR, map_id);
    }
    else {
      ASSERT(0);
    }
  }
  else
  {
    ASSERT(0);
  }
}

typedef struct {
  const char* testName;
  vx_uint8  sensorPhase;
  vx_uint16  threshold;
  vx_float32  alphaR;
  vx_float32  alphaG;
  vx_float32  alphaB;
  vx_uint8  borderMode;
} Arg;

#define ADD_SENSOR_PHASE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/SENSOR_PHASE=0", __VA_ARGS__, 0))
#define ADD_THRESHOLD(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/THRESHOLD=50", __VA_ARGS__, 50))
#define ADD_ALPHA_R(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/ALPHA_R=0.25", __VA_ARGS__, 0.25))
#define ADD_ALPHA_G(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/ALPHA_G=0.1", __VA_ARGS__, 0.1))
#define ADD_ALPHA_B(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/ALPHA_B=0.7", __VA_ARGS__, 0.7))
#define ADD_BORDER_MODE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/BORDER_MODE=1", __VA_ARGS__, 1))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("Test_RGB_IR_ON_EVE", ADD_SENSOR_PHASE, ADD_THRESHOLD, ADD_ALPHA_R, ADD_ALPHA_G, ADD_ALPHA_B, ADD_BORDER_MODE, ARG)

TEST_WITH_ARG(tivxIVisionRgbIr, testRgbIrOnEve, Arg,
    PARAMETERS
)
{
  vx_context context = context_->vx_context_;
  vx_image input_image = 0;
  vx_image output_bayer = 0;
  vx_image output_ir = 0;
  vx_graph graph = 0;
  vx_node node = 0;
  vx_imagepatch_addressing_t addrs;
  void *image = 0;
  void *bayerOut = 0;
  void *irOut = 0;
  vx_perf_t perf_node;
  vx_perf_t perf_graph;

  if ((vx_bool)vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_EVE1))
  {
    IVisionLoadKernels(context);

    image = tivxMemAlloc(2*IMG_WIDTH*IMG_HEIGHT, TIVX_MEM_EXTERNAL);
    if(image)
    {
      /* Copy input image to allocated buffer */
      memcpy(image, gTivxRgbIrTestInput, 2*IMG_WIDTH*IMG_HEIGHT);
    }
    else
    {
      printf("RgbIr: Cannot allocate memory for input image !!!\n");
      return;
    }

    bayerOut = tivxMemAlloc(2*IMG_WIDTH*IMG_HEIGHT, TIVX_MEM_EXTERNAL);
    if (bayerOut== NULL) {
      printf("RgbIr: Cannot allocate memory for bayer output !!!\n");
      return;
    }

    irOut = tivxMemAlloc(2*(IMG_WIDTH/2)*(IMG_HEIGHT/2), TIVX_MEM_EXTERNAL);
    if (irOut== NULL) {
      printf("RgbIr: Cannot allocate memory for IR output !!!\n");
      return;
    }

    addrs.dim_x = IMG_WIDTH;
    addrs.dim_y = IMG_HEIGHT;
    addrs.stride_x = 2;
    addrs.stride_y = 2*IMG_WIDTH;
    addrs.step_x = 1;
    addrs.step_y = 1;
    ASSERT_VX_OBJECT(input_image = vxCreateImageFromHandle(context,
        (vx_df_image)VX_DF_IMAGE_U16, &addrs, &image, VX_MEMORY_TYPE_HOST), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(output_bayer = vxCreateImageFromHandle(context,
        (vx_df_image)VX_DF_IMAGE_U16, &addrs, &bayerOut, VX_MEMORY_TYPE_HOST), VX_TYPE_IMAGE);

    addrs.dim_x = IMG_WIDTH/2;
    addrs.dim_y = IMG_HEIGHT/2;
    addrs.stride_x = 2;
    addrs.stride_y = IMG_WIDTH;
    ASSERT_VX_OBJECT(output_ir = vxCreateImageFromHandle(context,
        (vx_df_image)VX_DF_IMAGE_U16, &addrs, &irOut, VX_MEMORY_TYPE_HOST), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxRgbIrNode(graph,
        input_image, arg_->sensorPhase, arg_->threshold,
        arg_->alphaR, arg_->alphaG, arg_->alphaB,
        arg_->borderMode,
        output_bayer, output_ir), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_EVE1));
    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    CheckOutput(output_bayer, output_ir);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&output_ir));
    VX_CALL(vxReleaseImage(&output_bayer));
    VX_CALL(vxReleaseImage(&input_image));

    if (image)
    {
      tivxMemFree(image, 2*IMG_WIDTH*IMG_HEIGHT, TIVX_MEM_EXTERNAL);
      image = NULL;
    }

    if (bayerOut)
    {
      tivxMemFree(bayerOut, 2*IMG_WIDTH*IMG_HEIGHT, TIVX_MEM_EXTERNAL);
      image = NULL;
    }

    if (irOut)
    {
      tivxMemFree(irOut, 2*(IMG_WIDTH/2)*(IMG_HEIGHT/2), TIVX_MEM_EXTERNAL);
      image = NULL;
    }

    ASSERT(output_ir == 0);
    ASSERT(output_bayer == 0);
    ASSERT(input_image == 0);

    IVisionUnLoadKernels(context);

    IVisionPrintPerformance(perf_node, IMG_WIDTH*IMG_HEIGHT, "N0");
    IVisionPrintPerformance(perf_graph, IMG_WIDTH*IMG_HEIGHT, "G0");
  }
}



TESTCASE_TESTS(tivxIVisionRgbIr, testRgbIrOnEve)
