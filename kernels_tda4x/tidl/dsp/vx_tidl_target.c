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



#include <TI/tivx.h>
#include <TI/tda4x.h>
#include <TI/tivx_target_kernel.h>
#include <tivx_alg_ivision_if.h>
#include "itidl_ti.h"

/* TODO: remove this once TIDL fixes the need for having this */
uint8_t pBlock[256] 
__attribute__ ((aligned(256)))
    ;

#define L1_MEM_SIZE (16*1024)
#define L2_MEM_SIZE (448*1024)
#define L3_MEM_SIZE (6*1024*1024)

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
        uint32_t in_tensor_idx = 2;

        /* Idx N - output tensors, where N = Idx 2 + number of input tensors */
        uint32_t out_tensor_idx = in_tensor_idx + prms->inBufs.numBufs;
        uint32_t id;

        for(id = 0; id < prms->inBufs.numBufs; id++) {
            inTensor  = (tivx_obj_desc_tensor_t *)obj_desc[in_tensor_idx + id];
            in_tensor_target_ptr  = tivxMemShared2TargetPtr(inTensor->mem_ptr.shared_ptr, inTensor->mem_ptr.mem_heap_region);
            tivxMemBufferMap(in_tensor_target_ptr, inTensor->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            prms->inBufDesc[id].bufPlanes[0].buf = in_tensor_target_ptr;
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
        config    = (tivx_obj_desc_user_data_object_t *)obj_desc[0];

        /* IMPORTANT! Network data is assumed to be available at index 1 */
        network   = (tivx_obj_desc_user_data_object_t *)obj_desc[1];

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
        prms->createParams.currLayersGroupId             = TIDL_TB_CURR_LAYERS_GROUP_ID;
        prms->createParams.isInbufsPaded                 = 1;
        prms->createParams.TIDLGetPhysicalAddress        = NULL;
        prms->createParams.optimiseExtMem                = TIDL_optimiseExtMemL1;

        prms->createParams.TIDLGetPhysicalAddress        = NULL;
        prms->createParams.quantRangeExpansionFactor     = 1.0;
        prms->createParams.quantRangeUpdateFactor        = 0.0;

        prms->createParams.l1MemSize = L1_MEM_SIZE;
        prms->createParams.l2MemSize = L2_MEM_SIZE;
        prms->createParams.l3MemSize = L3_MEM_SIZE;

        network_target_ptr = tivxMemShared2TargetPtr(network->mem_ptr.shared_ptr, network->mem_ptr.mem_heap_region);
        tivxMemBufferMap(network_target_ptr, network->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        prms->createParams.net = (sTIDL_Network_t *)network_target_ptr;

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

static vx_status VX_CALLBACK tivxKernelTIDLControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelTIDL()
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_EVE1))
    {
        strncpy(target_name, TIVX_TARGET_EVE1, TIVX_TARGET_MAX_NAME);

        vx_tidl_target_kernel = tivxAddTargetKernelByName
                                (
                                  TIVX_KERNEL_TIDL_NAME,
                                  target_name,
                                  tivxKernelTIDLProcess,
                                  tivxKernelTIDLCreate,
                                  tivxKernelTIDLDelete,
                                  tivxKernelTIDLControl,
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
