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



#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_kernel_rgb_ir.h>
#include <TI/tivx_target_kernel.h>
#include <common/xdais_types.h> /* Adding to support vx_uint32/vx_int32 data types */
#include <ti/xdais/ires.h>
#include <tivx_alg_ivision_if.h>
#include <apps/rgb_ir/algo/inc/irgb_ir_ti.h>
#include <tivx_kernels_target_utils.h>
#include <math.h>

typedef struct
{
  IVISION_BufDesc     inBufDesc;
  IVISION_BufDesc    *inBufDescList[1u];
  IVISION_InBufs      inBufs;

  IVISION_BufDesc     outBufDesc;
  IVISION_BufDesc    *outBufDescList[1u];
  IVISION_OutBufs     outBufs;

  RGB_IR_TI_CreateParams  createParams;

  RGB_IR_TI_InArgs      inArgs;
  RGB_IR_TI_OutArgs     outArgs;

  RGB_IR_TI_ControlInParams ctlInParams;
  RGB_IR_TI_ControlOutParams ctlOutParams;

  Void               *algHandle;
} tivxRgbIrParams;


static tivx_target_kernel vx_rgb_ir_target_kernel = NULL;

static void tivxRgbIrFreeMem(tivxRgbIrParams *prms);

static vx_status VX_CALLBACK tivxKernelRgbIrProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
  vx_status status = (vx_status)VX_SUCCESS;
  uint32_t i;
  tivxRgbIrParams *prms = NULL;
  tivx_obj_desc_image_t *src;
  tivx_obj_desc_image_t *dstBayer, *dstIR;
  uint32_t size;
  tivx_obj_desc_scalar_t *sc_thr, *sc_alphaR, *sc_alphaG, *sc_alphaB, *sc_borderMode;

  if (num_params != TIVX_KERNEL_RGB_IR_MAX_PARAMS)
  {
    status = (vx_status)VX_FAILURE;
  }
  else
  {
    for (i = 0U; i < TIVX_KERNEL_RGB_IR_MAX_PARAMS; i ++)
    {
      if (NULL == obj_desc[i])
      {
        status = (vx_status)VX_FAILURE;
        break;
      }
    }
  }

  if ((vx_status)VX_SUCCESS == status)
  {
    src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_RGB_IR_IN_IMG_IDX];
    dstBayer = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_RGB_IR_OUT_BAYER_IDX];
    dstIR = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_RGB_IR_OUT_IR_IDX];
    sc_thr = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_RGB_IR_THR_IDX];
    sc_alphaR= (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_RGB_IR_ALPHA_R_IDX];
    sc_alphaG= (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_RGB_IR_ALPHA_G_IDX];
    sc_alphaB= (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_RGB_IR_ALPHA_B_IDX];
    sc_borderMode= (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_RGB_IR_BORDER_MODE_IDX];

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
        (sizeof(tivxRgbIrParams) != size))
    {
      status = (vx_status)VX_FAILURE;
    }
  }

  if ((vx_status)VX_SUCCESS == status)
  {
    void *src_desc_target_ptr;
    void *dstBayer_desc_target_ptr, *dstIR_desc_target_ptr;

    src_desc_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[0]);
    dstBayer_desc_target_ptr = tivxMemShared2TargetPtr(&dstBayer->mem_ptr[0]);
    dstIR_desc_target_ptr = tivxMemShared2TargetPtr(&dstIR->mem_ptr[0]);

    tivxCheckStatus(&status, tivxMemBufferMap(src_desc_target_ptr, src->mem_size[0],
        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    tivxCheckStatus(&status, tivxMemBufferMap(dstBayer_desc_target_ptr, dstBayer->mem_size[0],
        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
    tivxCheckStatus(&status, tivxMemBufferMap(dstIR_desc_target_ptr, dstBayer->mem_size[0],
        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

    prms->inBufDesc.numPlanes= 1;
    prms->inBufDesc.bufPlanes[0u].buf= src_desc_target_ptr;
    prms->inBufDesc.bufPlanes[0u].frameROI.topLeft.x= src->valid_roi.start_x;
    prms->inBufDesc.bufPlanes[0u].frameROI.topLeft.y= src->valid_roi.start_y;
    prms->inBufDesc.bufPlanes[0u].frameROI.width= src->valid_roi.end_x - src->valid_roi.start_x;
    prms->inBufDesc.bufPlanes[0u].frameROI.height= src->valid_roi.end_y - src->valid_roi.start_y;
    prms->inBufDesc.bufPlanes[0u].width= src->imagepatch_addr[0].dim_x;
    prms->inBufDesc.bufPlanes[0u].height= src->imagepatch_addr[0].dim_y;

    prms->outBufDesc.numPlanes= 2;
    prms->outBufDesc.bufPlanes[0u].buf = dstBayer_desc_target_ptr;
    prms->outBufDesc.bufPlanes[0u].frameROI.topLeft.x= dstBayer->valid_roi.start_x;
    prms->outBufDesc.bufPlanes[0u].frameROI.topLeft.y= dstBayer->valid_roi.start_y;
    prms->outBufDesc.bufPlanes[0u].frameROI.width= dstBayer->valid_roi.end_x - src->valid_roi.start_x;
    prms->outBufDesc.bufPlanes[0u].frameROI.height= dstBayer->valid_roi.end_y - src->valid_roi.start_y;
    prms->outBufDesc.bufPlanes[0u].width= dstBayer->imagepatch_addr[0].dim_x;
    prms->outBufDesc.bufPlanes[0u].height= dstBayer->imagepatch_addr[0].dim_y;

    prms->outBufDesc.bufPlanes[1u].buf = dstIR_desc_target_ptr;
    prms->outBufDesc.bufPlanes[1u].frameROI.topLeft.x= dstIR->valid_roi.start_x;
    prms->outBufDesc.bufPlanes[1u].frameROI.topLeft.y= dstIR->valid_roi.start_y;
    prms->outBufDesc.bufPlanes[1u].frameROI.width= dstIR->valid_roi.end_x - src->valid_roi.start_x;
    prms->outBufDesc.bufPlanes[1u].frameROI.height= dstIR->valid_roi.end_y - src->valid_roi.start_y;
    prms->outBufDesc.bufPlanes[1u].width= dstIR->imagepatch_addr[0].dim_x;
    prms->outBufDesc.bufPlanes[1u].height= dstIR->imagepatch_addr[0].dim_y;

    prms->inArgs.threshold= sc_thr->data.u16;
    prms->inArgs.alphaR= round(sc_alphaR->data.f32 * 32768.0);
    prms->inArgs.alphaG= round(sc_alphaG->data.f32 * 32768.0);
    prms->inArgs.alphaB= round(sc_alphaB->data.f32 * 32768.0);
    prms->inArgs.borderMode= sc_borderMode->data.u08;

    status = tivxAlgiVisionProcess(
        prms->algHandle,
        &prms->inBufs,
        &prms->outBufs,
        (IVISION_InArgs*)&prms->inArgs,
        (IVISION_OutArgs *)&prms->outArgs,
        0);

    tivxCheckStatus(&status, tivxMemBufferUnmap(src_desc_target_ptr, src->mem_size[0],
        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    tivxCheckStatus(&status, tivxMemBufferUnmap(dstBayer_desc_target_ptr, dstBayer->mem_size[0],
        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
    tivxCheckStatus(&status, tivxMemBufferUnmap(dstIR_desc_target_ptr, dstIR->mem_size[0],
        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
  }

  return (status);
}

static vx_status VX_CALLBACK tivxKernelRgbIrCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
  uint32_t i;
  vx_status status = (vx_status)VX_SUCCESS;
  tivx_obj_desc_image_t *img;
  tivxRgbIrParams *prms = NULL;
  tivx_obj_desc_scalar_t *sc_sensorPhase;

  if (num_params != TIVX_KERNEL_RGB_IR_MAX_PARAMS)
  {
    status = (vx_status)VX_FAILURE;
  }
  else
  {
    for (i = 0U; i < TIVX_KERNEL_RGB_IR_MAX_PARAMS; i ++)
    {
      if (NULL == obj_desc[i])
      {
        status = (vx_status)VX_FAILURE;
        break;
      }
    }
  }

  if ((vx_status)VX_SUCCESS == status)
  {
    img = (tivx_obj_desc_image_t *)obj_desc[
                                            TIVX_KERNEL_RGB_IR_IN_IMG_IDX];
    sc_sensorPhase = (tivx_obj_desc_scalar_t *)obj_desc[
                                                        TIVX_KERNEL_RGB_IR_SENSOR_PHASE_IDX];

    prms = tivxMemAlloc(sizeof(tivxRgbIrParams), (vx_enum)TIVX_MEM_EXTERNAL);
    if (NULL != prms)
    {
      memset(prms, 0, sizeof(tivxRgbIrParams));
    }
    else
    {
      status = (vx_status)VX_ERROR_NO_MEMORY;
    }

    if ((vx_status)VX_SUCCESS == status)
    {

      prms->createParams.visionParams.algParams.size =
          sizeof(RGB_IR_TI_CreateParams);
      prms->createParams.visionParams.cacheWriteBack = NULL;
      prms->createParams.sensorPhase = sc_sensorPhase->data.u08;
      prms->createParams.imgFrameWidth = img->valid_roi.end_x - img->valid_roi.start_x;
      prms->createParams.imgFrameHeight = img->valid_roi.end_y - img->valid_roi.start_y;

      /* Initialize Buffer Descriptors */
      prms->inBufs.size       = sizeof(prms->inBufs);
      prms->inBufs.numBufs    = 1u;
      prms->inBufs.bufDesc    = prms->inBufDescList;
      prms->inBufDescList[0u] = &prms->inBufDesc;

      prms->outBufs.size       = sizeof(prms->outBufs);
      prms->outBufs.numBufs    = 1u;
      prms->outBufDescList[0u] = &prms->outBufDesc;
      prms->outBufs.bufDesc    = prms->outBufDescList;

      memset(&prms->inArgs, 0x0, sizeof(prms->inArgs));

      prms->inArgs.iVisionInArgs.subFrameInfo = 0u;
      prms->inArgs.iVisionInArgs.size = sizeof(RGB_IR_TI_InArgs);

      prms->ctlInParams.algParams.size =
          sizeof(RGB_IR_TI_ControlInParams);
      prms->ctlOutParams.algParams.size =
          sizeof(RGB_IR_TI_ControlOutParams);

      prms->outArgs.iVisionOutArgs.size =
          sizeof(RGB_IR_TI_OutArgs);

      prms->algHandle = tivxAlgiVisionCreate(
          &RGB_IR_TI_VISION_FXNS,
          (IALG_Params *)(&prms->createParams));

      if (NULL == prms->algHandle)
      {
        status = (vx_status)VX_FAILURE;
      }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
      tivxSetTargetKernelInstanceContext(kernel, prms,
          sizeof(tivxRgbIrParams));
    }
    else
    {
      if (NULL != prms)
      {
        tivxRgbIrFreeMem(prms);
      }
    }
  }

  return (status);
}

static vx_status VX_CALLBACK tivxKernelRgbIrDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
  vx_status status = (vx_status)VX_SUCCESS;
  uint32_t i;
  uint32_t size;
  tivxRgbIrParams *prms = NULL;

  if (num_params != TIVX_KERNEL_RGB_IR_MAX_PARAMS)
  {
    status = (vx_status)VX_FAILURE;
  }
  else
  {
    for (i = 0U; i < TIVX_KERNEL_RGB_IR_MAX_PARAMS; i ++)
    {
      if (NULL == obj_desc[i])
      {
        status = (vx_status)VX_FAILURE;
        break;
      }
    }
  }

  if ((vx_status)VX_SUCCESS == status)
  {
    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
        (sizeof(tivxRgbIrParams) == size))
    {
      if (prms->algHandle)
      {
        tivxAlgiVisionDelete(prms->algHandle);
      }
      tivxRgbIrFreeMem(prms);
    }
  }

  return (status);
}

void tivxAddTargetKernelRgbIr()
{
  char target_name[TIVX_TARGET_MAX_NAME];
  vx_enum self_cpu;

  self_cpu = tivxGetSelfCpuId();

  if ((self_cpu == (vx_enum)TIVX_CPU_ID_EVE1) || (self_cpu == (vx_enum)TIVX_CPU_ID_EVE2) ||
      (self_cpu == (vx_enum)TIVX_CPU_ID_EVE3) || (self_cpu == (vx_enum)TIVX_CPU_ID_EVE4))
  {
    if (self_cpu == (vx_enum)TIVX_CPU_ID_EVE1)
    {
      strncpy(target_name, TIVX_TARGET_EVE1,
          TIVX_TARGET_MAX_NAME);
    }
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_EVE2)
    {
      strncpy(target_name, TIVX_TARGET_EVE2,
          TIVX_TARGET_MAX_NAME);
    }
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_EVE3)
    {
      strncpy(target_name, TIVX_TARGET_EVE3,
          TIVX_TARGET_MAX_NAME);
    }
    else
    {
      strncpy(target_name, TIVX_TARGET_EVE4,
          TIVX_TARGET_MAX_NAME);
    }

    vx_rgb_ir_target_kernel = tivxAddTargetKernel(
        TIVX_KERNEL_IVISION_RGB_IR,
        target_name,
        tivxKernelRgbIrProcess,
        tivxKernelRgbIrCreate,
        tivxKernelRgbIrDelete,
        NULL,
        NULL);
  }
}


void tivxRemoveTargetKernelRgbIr()
{
  tivxRemoveTargetKernel(vx_rgb_ir_target_kernel);
}

static void tivxRgbIrFreeMem(tivxRgbIrParams *prms)
{
  if (NULL != prms)
  {
    tivxMemFree(prms, sizeof(tivxRgbIrParams), (vx_enum)TIVX_MEM_EXTERNAL);
  }
}

