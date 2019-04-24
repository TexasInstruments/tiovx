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
#include <TI/tivx_target_kernel.h>
#include "tivx_tidl_kernels.h"
#include "../../../common/xdais_types.h" /* In TIDL_PATH directory */
#include <tivx_alg_ivision_if.h>
#include "sTIDL_IOBufDesc.h"
#include "itidl_ti.h"

#ifndef HOST_EMULATION
#include <./src/rtos/utils_common/include/utils_mem_cfg.h> /* In Vision-sdk directory */
/* Minimum DSP L1 SIZE */
#define TIDL_LINK_MIN_DSPL1_SIZE    (8 * 1024)
/* Minimum DSP L1 SIZE */
#define TIDL_LINK_MIN_EVEL2_SIZE    (8 * 1024)
/* Minimum L3 SIZE */
#define TIDL_LINK_MIN_L3_SIZE       (320 * 1024)
#endif

#define L1_MEM_SIZE (20*1024 + 256)
#define L2_MEM_SIZE (148*1024)
#define L3_MEM_SIZE (320*1024)

#define TIDL_TB_CURR_CORE_ID            (1)
#define TIDL_TB_CURR_LAYERS_GROUP_ID    (1)

#define ALIGN_SIZE(x,y) (((x + (y-1)) / y) * y)

typedef struct
{
  IVISION_BufDesc     inBufDesc[TIDL_MAX_ALG_IN_BUFS];
  IVISION_BufDesc     outBufDesc[TIDL_MAX_ALG_OUT_BUFS];

  IVISION_BufDesc    *inBufDescList[TIDL_MAX_ALG_IN_BUFS];
  IVISION_BufDesc    *outBufDescList[TIDL_MAX_ALG_OUT_BUFS];

  IVISION_InBufs      inBufs;
  IVISION_OutBufs     outBufs;

  TIDL_InArgs         inArgs;
  TIDL_outArgs        outArgs;

  TIDL_CreateParams   createParams;

  void            *algHandle;
} tivxTIDLParams;

static tivx_target_kernel vx_tidl_target_kernel = NULL;

static void tivxTIDLFreeMem(tivxTIDLParams *prms);

static int TIDL_createParamsInit(TIDL_CreateParams * params)
{
  params->currCoreId                = TIDL_TB_CURR_CORE_ID;
  params->currLayersGroupId         = TIDL_TB_CURR_LAYERS_GROUP_ID;

  params->l1MemSize                 = L1_MEM_SIZE;
  params->l2MemSize                 = L2_MEM_SIZE;
  params->l3MemSize                 = L3_MEM_SIZE;
  params->quantHistoryParam1        = 20;
  params->quantHistoryParam2        = 5;
  params->quantMargin               = 0;
  params->optimiseExtMem            = TIDL_optimiseExtMemL1;

  return IALG_EOK;
}

/* The below function may not be required since in BIOS tivxMemShared2TargetPtr() calls
 * Utils_memPhysToVirt() which anyway returns the original pointer
 */
static int32_t tidl_convertNetParamsPtr(sTIDL_Network_t *net) {

  int32_t i;

  for(i = 0; i < net->numLayers; i++)
  {
    if((TIDL_ConvolutionLayer == net->TIDLLayers[i].layerType) ||
        (TIDL_Deconv2DLayer == net->TIDLLayers[i].layerType))
    {
      sTIDL_ConvParams_t *conv2dPrms = \
          &net->TIDLLayers[i].layerParams.convParams;

      conv2dPrms->weights.ptr= tivxMemShared2TargetPtr((uint64_t)(uintptr_t)conv2dPrms->weights.ptr, TIVX_MEM_EXTERNAL);
      conv2dPrms->bias.ptr = tivxMemShared2TargetPtr((uint64_t)(uintptr_t)conv2dPrms->bias.ptr, TIVX_MEM_EXTERNAL);

    }
    else if(TIDL_BiasLayer == net->TIDLLayers[i].layerType)
    {
      sTIDL_BiasParams_t *biasPrms = &net->TIDLLayers[i].layerParams.biasParams;
      biasPrms->bias.ptr = tivxMemShared2TargetPtr((uint64_t)(uintptr_t)biasPrms->bias.ptr, TIVX_MEM_EXTERNAL);
    }
    else if(TIDL_BatchNormLayer == net->TIDLLayers[i].layerType)
    {
      sTIDL_BatchNormParams_t *batchNormPrms = \
          &net->TIDLLayers[i].layerParams.batchNormParams;
      batchNormPrms->weights.ptr= tivxMemShared2TargetPtr((uint64_t)(uintptr_t)batchNormPrms->weights.ptr, TIVX_MEM_EXTERNAL);
      batchNormPrms->bias.ptr = tivxMemShared2TargetPtr((uint64_t)(uintptr_t)batchNormPrms->bias.ptr, TIVX_MEM_EXTERNAL);

      if(TIDL_PRelU == batchNormPrms->reluParams.reluType)
      {
        batchNormPrms->reluParams.slope.ptr= tivxMemShared2TargetPtr((uint64_t)(uintptr_t)batchNormPrms->reluParams.slope.ptr, TIVX_MEM_EXTERNAL);
      }
    }
    else if(TIDL_InnerProductLayer == net->TIDLLayers[i].layerType)
    {
      sTIDL_InnerProductParams_t *ipPrms = \
          &net->TIDLLayers[i].layerParams.innerProductParams;
      ipPrms->bias.ptr= tivxMemShared2TargetPtr((uint64_t)(uintptr_t)ipPrms->bias.ptr, TIVX_MEM_EXTERNAL);
      ipPrms->weights.ptr= tivxMemShared2TargetPtr((uint64_t)(uintptr_t)ipPrms->weights.ptr, TIVX_MEM_EXTERNAL);
    }
    else if(TIDL_DetectionOutputLayer == net->TIDLLayers[i].layerType)
    {
      sTIDL_DetectOutputParams_t *detectPrms = \
          &net->TIDLLayers[i].layerParams.detectOutParams;
      detectPrms->priorBox.ptr= tivxMemShared2TargetPtr((uint64_t)(uintptr_t)detectPrms->priorBox.ptr, TIVX_MEM_EXTERNAL);
    }
  }

  return 0;
}

static int32_t tidl_AllocNetInputMem(IVISION_BufDesc *BufDescList, sTIDL_IOBufDesc_t *pConfig)
{
  uint16_t numBuffs = 0;

  /* Currently only one input buffer supported */
  for(numBuffs = 0; numBuffs < pConfig->numInputBuf; numBuffs++)
  {
    BufDescList[numBuffs].numPlanes                          = 1;
    BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.x    = 0;
    BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.y    = 0;

    /* This has to be width + horizontal padding */
    BufDescList[numBuffs].bufPlanes[0].width                 = pConfig->inWidth[numBuffs] + pConfig->inPadL[numBuffs] + pConfig->inPadR[numBuffs];
    /* This has to be numCh * (height + vertical padding) */
    BufDescList[numBuffs].bufPlanes[0].height                = pConfig->inNumChannels[numBuffs] * (pConfig->inHeight[numBuffs]  + pConfig->inPadT[numBuffs] + pConfig->inPadB[numBuffs]);
    /* This has to be just width */
    BufDescList[numBuffs].bufPlanes[0].frameROI.width        = pConfig->inWidth[numBuffs];
    /* This has to be just height */
    BufDescList[numBuffs].bufPlanes[0].frameROI.height       = pConfig->inHeight[numBuffs];

    /* This comes from tidl_io_xxx.txt file (inDataId), not sure how to pass it. Currently hardcoding for Jacinto Net */
    BufDescList[numBuffs].reserved[0]     = pConfig->inDataId[numBuffs];

    /* This comes from tidl_io_xxx.txt file (inDataId), not sure how to pass it. Currently hardcoding for Jacinto Net */
    BufDescList[numBuffs].bufferId = pConfig->inDataId[numBuffs];
  }
  return numBuffs;
}

static int32_t tidl_AllocNetOutputMem(IVISION_BufDesc *BufDescList, sTIDL_IOBufDesc_t *pConfig)
{
  uint16_t numBuffs = 0;

  /* Currently only one output buffer supported */
  for(numBuffs = 0; numBuffs < pConfig->numOutputBuf; numBuffs++)
  {
    BufDescList[numBuffs].numPlanes                          = 1;
    BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.x    = 0;
    BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.y    = 0;

    /* This requires output width + horizontal padding */
    BufDescList[numBuffs].bufPlanes[0].width                 = pConfig->outWidth[numBuffs] + pConfig->outPadL[numBuffs] + pConfig->outPadR[numBuffs];
    /* This requires numOutCh * (output height + vertial padding) */
    BufDescList[numBuffs].bufPlanes[0].height                = pConfig->outNumChannels[numBuffs] * (pConfig->outHeight[numBuffs] + pConfig->outPadT[numBuffs] + pConfig->outPadB[numBuffs]);
    /* This requires just output width */
    BufDescList[numBuffs].bufPlanes[0].frameROI.width        = pConfig->outWidth[numBuffs];
    /* This requires just output height */
    BufDescList[numBuffs].bufPlanes[0].frameROI.height       = pConfig->outHeight[numBuffs];

    /* This comes from tidl_io_xxx.txt file (outDataId), not sure how to pass it. Currently hardcoding for Jacinto Net */
    BufDescList[numBuffs].reserved[0]      = pConfig->outDataId[numBuffs];

    /* This comes from tidl_io_xxx.txt file (outDataId), not sure how to pass it. Currently hardcoding for Jacinto Net */
    BufDescList[numBuffs].bufferId         = pConfig->outDataId[numBuffs];
  }
  return numBuffs;
}

static vx_status VX_CALLBACK tivxKernelTIDLProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
)
{
  vx_status status = VX_SUCCESS;

  tivxTIDLParams *prms;
  uint32_t i, size;

  for (i = 0U; i < num_params; i ++)
  {
    if (NULL == obj_desc[i])
    {
      status = VX_FAILURE;
      break;
    }
  }

  if (VX_SUCCESS == status)
  {
    status = tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);

    if ((VX_SUCCESS != status) || (NULL == prms) ||  (sizeof(tivxTIDLParams) != size))
    {
      status = VX_FAILURE;
    }
  }

  if (VX_SUCCESS == status)
  {
    tivx_obj_desc_tensor_t *inTensor;
    tivx_obj_desc_tensor_t *outTensor;

    void *in_tensor_target_ptr;
    void *out_tensor_target_ptr;

    /* Idx 0 - config data, Idx 1 - network data, Idx 2 - input tensor */
    uint32_t in_tensor_idx = TIVX_KERNEL_TIDL_IN_FIRST_TENSOR;
    uint32_t out_tensor_idx;
    uint32_t id;

    out_tensor_idx= in_tensor_idx + prms->inBufs.numBufs;

    for(id = 0; id < prms->inBufs.numBufs; id++) {
      inTensor  = (tivx_obj_desc_tensor_t *)obj_desc[in_tensor_idx + id];
      in_tensor_target_ptr  = tivxMemShared2TargetPtr(inTensor->mem_ptr.shared_ptr, inTensor->mem_ptr.mem_heap_region);
      tivxMemBufferMap(in_tensor_target_ptr, inTensor->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
      prms->inBufDesc[id].bufPlanes[0].buf = in_tensor_target_ptr;
      prms->inArgs.dataQ[id]= inTensor->scaling_divisor;
    }

    for(id = 0; id < prms->outBufs.numBufs; id++) {
      outTensor = (tivx_obj_desc_tensor_t *)obj_desc[out_tensor_idx + id];
      out_tensor_target_ptr = tivxMemShared2TargetPtr(outTensor->mem_ptr.shared_ptr, outTensor->mem_ptr.mem_heap_region);
      tivxMemBufferMap(out_tensor_target_ptr, outTensor->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
      prms->outBufDesc[id].bufPlanes[0].buf = out_tensor_target_ptr;
    }

    status = tivxAlgiVisionProcess
        (
            prms->algHandle,
            &prms->inBufs,
            &prms->outBufs,
            (IVISION_InArgs  *)&prms->inArgs,
            (IVISION_OutArgs *)&prms->outArgs
        );

    for(id = 0; id < prms->inBufs.numBufs; id++) {
      inTensor  = (tivx_obj_desc_tensor_t *)obj_desc[in_tensor_idx + id];
      in_tensor_target_ptr  = tivxMemShared2TargetPtr(inTensor->mem_ptr.shared_ptr, inTensor->mem_ptr.mem_heap_region);
      tivxMemBufferUnmap(in_tensor_target_ptr, inTensor->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }

    for(id = 0; id < prms->outBufs.numBufs; id++) {
      outTensor = (tivx_obj_desc_tensor_t *)obj_desc[out_tensor_idx + id];
      out_tensor_target_ptr = tivxMemShared2TargetPtr(outTensor->mem_ptr.shared_ptr, outTensor->mem_ptr.mem_heap_region);
      tivxMemBufferUnmap(out_tensor_target_ptr, outTensor->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
      outTensor->scaling_divisor= prms->outArgs.dataQ[id];
      outTensor->scaling_divisor_fixed_point_position= 8;
    }

  }

  return (status);
}

static vx_status VX_CALLBACK tivxKernelTIDLCreate
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
)
{
  vx_status status = VX_SUCCESS;

  tivx_obj_desc_user_data_object_t *config;
  tivx_obj_desc_user_data_object_t *network;

  tivxTIDLParams *prms = NULL;
  void *config_target_ptr;
  void *network_target_ptr;
  uint8_t *pFlagShared2Target;

  uint32_t i;

  for (i = 0U; i < num_params; i ++)
  {
    if (NULL == obj_desc[i])
    {
      status = VX_FAILURE;
      break;
    }
  }

  if (VX_SUCCESS == status)
  {
    /* IMPORTANT! Config data is assumed to be available at index 0 */
    config    = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_TIDL_IN_CONFIG_IDX];

    /* IMPORTANT! Network data is assumed to be available at index 1 */
    network   = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_TIDL_IN_NETWORK_IDX];

    prms = tivxMemAlloc(sizeof(tivxTIDLParams), TIVX_MEM_EXTERNAL);

    if (NULL != prms)
    {
      memset(prms, 0, sizeof(tivxTIDLParams));
    }
    else
    {
      status = VX_ERROR_NO_MEMORY;
    }

    TIDL_createParamsInit(&prms->createParams);

    prms->createParams.visionParams.algParams.size   = sizeof(TIDL_CreateParams);
    prms->createParams.visionParams.cacheWriteBack   = NULL;

#ifdef HOST_EMULATION
    uint32_t tivxTargetKernelInstanceGetIndex(tivx_target_kernel_instance target_kernel_instance);
    void tivxSetSelfCpuId(vx_enum cpu_id);

    uint32_t index= tivxTargetKernelInstanceGetIndex(kernel);
    if (index==0) {
      tivxSetSelfCpuId(TIVX_CPU_ID_EVE1);
    }
    else {
      tivxSetSelfCpuId(TIVX_CPU_ID_DSP1);
    }
#endif

    tivx_cpu_id_e cpuId= (tivx_cpu_id_e)tivxGetSelfCpuId();
    if ((cpuId== TIVX_CPU_ID_DSP1) || (cpuId== TIVX_CPU_ID_DSP2) ) {
      prms->createParams.currLayersGroupId=2 ;
#ifndef HOST_EMULATION
      prms->createParams.l1MemSize = TIDL_LINK_MIN_DSPL1_SIZE;
      prms->createParams.l2MemSize = UTILS_MEM_HEAP_L2_SIZE - (64 * 1024);
      prms->createParams.l3MemSize = TIDL_LINK_MIN_L3_SIZE;
#endif
    } else if ((cpuId== TIVX_CPU_ID_EVE1) || (cpuId== TIVX_CPU_ID_EVE2) || (cpuId== TIVX_CPU_ID_EVE3) || (cpuId== TIVX_CPU_ID_EVE4)) {
      prms->createParams.currLayersGroupId=1 ;
#ifndef HOST_EMULATION
      prms->createParams.l1MemSize = (UTILS_MEM_HEAP_L2_SIZE - (1 * 1024));
      prms->createParams.l2MemSize = TIDL_LINK_MIN_EVEL2_SIZE;
      prms->createParams.l3MemSize = TIDL_LINK_MIN_L3_SIZE;
#endif
    }

    prms->createParams.quantHistoryParam1     = 20;
    prms->createParams.quantHistoryParam2     = 5;
    prms->createParams.quantMargin            = 0;
    prms->createParams.optimiseExtMem         = TIDL_optimiseExtMemL1;

    network_target_ptr = tivxMemShared2TargetPtr(network->mem_ptr.shared_ptr, network->mem_ptr.mem_heap_region);
    tivxMemBufferMap(network_target_ptr, network->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

    prms->createParams.net = *((sTIDL_Network_t *)network_target_ptr);

    /* If *pFlagShared2Target==0, convert the pointers to each layer's parameter from shared to target */
    pFlagShared2Target= (uint8_t*)&prms->createParams.net + sizeof(sTIDL_Network_t);

    if (*pFlagShared2Target== 0) {
      tidl_convertNetParamsPtr(&prms->createParams.net);
      /* Set *pFlagShared2Target to 1 as we want to avoid calling tidl_convertNetParamsPtr() a second time
       * for the same network, which will lead to incorrect results.
       * Indeed the same network can be passed to more than one TI-DL node.
       *  */
      *pFlagShared2Target= 1;
    }

    prms->createParams.net.interElementSize = 4;

    if (VX_SUCCESS == status)
    {
      prms->algHandle = tivxAlgiVisionCreate
          (
              &TIDL_VISION_FXNS,
              (IALG_Params *)(&prms->createParams)
          );

      if (NULL == prms->algHandle)
      {
        status = VX_FAILURE;
      }
    }

    tivxMemBufferUnmap(network_target_ptr, network->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

    prms->inBufs.size     = sizeof(prms->inBufs);
    prms->outBufs.size    = sizeof(prms->outBufs);

    prms->inBufs.bufDesc  = prms->inBufDescList;
    prms->outBufs.bufDesc = prms->outBufDescList;

    config_target_ptr = tivxMemShared2TargetPtr(config->mem_ptr.shared_ptr, config->mem_ptr.mem_heap_region);
    tivxMemBufferMap(config_target_ptr, config->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

    prms->inBufs.numBufs  = tidl_AllocNetInputMem(prms->inBufDesc, (sTIDL_IOBufDesc_t *)config_target_ptr);
    prms->outBufs.numBufs = tidl_AllocNetOutputMem(prms->outBufDesc, (sTIDL_IOBufDesc_t *)config_target_ptr);

    tivxMemBufferUnmap(config_target_ptr, config->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

    for(i = 0; i < prms->inBufs.numBufs; i++)
    {
      prms->inBufDescList[i]     = &prms->inBufDesc[i];
    }
    for(i = 0; i < prms->outBufs.numBufs; i++)
    {
      prms->outBufDescList[i]     = &prms->outBufDesc[i];
    }

    prms->outArgs.iVisionOutArgs.size       = sizeof(TIDL_outArgs);
    prms->inArgs.iVisionInArgs.size         = sizeof(TIDL_InArgs);
    prms->inArgs.iVisionInArgs.subFrameInfo = 0;

    if (VX_SUCCESS == status)
    {
      tivxSetTargetKernelInstanceContext(kernel, prms,  sizeof(tivxTIDLParams));
    }
    else
    {
      if (NULL != prms)
      {
        tivxTIDLFreeMem(prms);
      }
    }
  }

  return (status);
}

static vx_status VX_CALLBACK tivxKernelTIDLDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
  vx_status status = VX_SUCCESS;
  uint32_t i;
  uint32_t size;
  tivxTIDLParams *prms = NULL;

  for (i = 0U; i < num_params; i ++)
  {
    if (NULL == obj_desc[i])
    {
      status = VX_FAILURE;
      break;
    }
  }

  if (VX_SUCCESS == status)
  {
    status = tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);

    if ((VX_SUCCESS == status) && (NULL != prms) && (sizeof(tivxTIDLParams) == size))
    {
      if (prms->algHandle)
      {
        tivxAlgiVisionDelete(prms->algHandle);
      }
      tivxTIDLFreeMem(prms);
    }
  }

  return (status);
}

void tivxAddTargetKernelTIDL()
{
  char target_name[TIVX_TARGET_MAX_NAME];
  vx_enum self_cpu;

  self_cpu = tivxGetSelfCpuId();

  if ((self_cpu == TIVX_CPU_ID_DSP1) || (self_cpu == TIVX_CPU_ID_DSP2) || (self_cpu == TIVX_CPU_ID_EVE1) || (self_cpu == TIVX_CPU_ID_EVE2) || (self_cpu == TIVX_CPU_ID_EVE3) || (self_cpu == TIVX_CPU_ID_EVE4))
  {
    if (self_cpu == TIVX_CPU_ID_DSP1)
    {
      strncpy(target_name, TIVX_TARGET_DSP1, TIVX_TARGET_MAX_NAME);
    }
    else if (self_cpu == TIVX_CPU_ID_DSP2)
    {
      strncpy(target_name, TIVX_TARGET_DSP2, TIVX_TARGET_MAX_NAME);
    }
    else if (self_cpu == TIVX_CPU_ID_EVE1)
    {
      strncpy(target_name, TIVX_TARGET_EVE1, TIVX_TARGET_MAX_NAME);
    }
    else if (self_cpu == TIVX_CPU_ID_EVE2)
    {
      strncpy(target_name, TIVX_TARGET_EVE2, TIVX_TARGET_MAX_NAME);
    }
    else if (self_cpu == TIVX_CPU_ID_EVE3)
    {
      strncpy(target_name, TIVX_TARGET_EVE3, TIVX_TARGET_MAX_NAME);
    }
    else if (self_cpu == TIVX_CPU_ID_EVE4)
    {
      strncpy(target_name, TIVX_TARGET_EVE4, TIVX_TARGET_MAX_NAME);
    }

    vx_tidl_target_kernel = tivxAddTargetKernelByName
        (
            TIVX_KERNEL_TIDL_NAME,
            target_name,
            tivxKernelTIDLProcess,
            tivxKernelTIDLCreate,
            tivxKernelTIDLDelete,
            NULL,
            NULL
        );
  }
}


void tivxRemoveTargetKernelTIDL()
{
  tivxRemoveTargetKernel(vx_tidl_target_kernel);
}

static void tivxTIDLFreeMem(tivxTIDLParams *prms)
{
  if (NULL != prms)
  {
    tivxMemFree(prms, sizeof(tivxTIDLParams), TIVX_MEM_EXTERNAL);
  }
}
