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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <tivx_utils_file_rd_wr.h>
#include "../../../common/xdais_types.h" /* In TIDL_PATH directory */
#include "sTIDL_IOBufDesc.h"
#include "tivx_tidl_utils.h"
#include "itidl_ti.h"

#define DEFAULT_ALIGN 4

#define align(val, alignv) ( (( (val) + (alignv) -1)/(alignv))*(alignv) )

/**
 *******************************************************************************
 *
 * \brief tidlIsInDataBuff() validates whether the particular buffer with 'dataId' is an input buffer to any layer belonging to layersGroupID of the network
 * \return  0: the buffer is not an input buffer of a layer with layersGroupId | 1: the buffer is an input buffer of a layer with layersGroupId
 *
 *******************************************************************************
 */
static int32_t tidlIsInDataBuff(
    sTIDL_Network_t *pTIDLNetStructure,
    int32_t dataId,
    int32_t layersGroupId)
{
  int32_t i, j;

  for(i = 0 ; i < pTIDLNetStructure->numLayers; i++)
  {
    for(j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
    {
      if((pTIDLNetStructure->TIDLLayers[i].layersGroupId == layersGroupId) &&
          (pTIDLNetStructure->TIDLLayers[i].inData[j].dataId == dataId))
      {
        return 1;
      }
    }
  }

  return 0;
}

/**
 *******************************************************************************
 *
 * \brief tidlGetNumInputBuffers() counts the number of input buffers to the network
 *  and populates the structure ioBufDesc with information on each input buffer.
 * \return  Number of input buffers
 *
 *******************************************************************************
 */
static uint32_t tidlGetNumInputBuffers(
    sTIDL_Network_t *net,
    sTIDL_IOBufDesc_t *ioBufDesc,
    int32_t layersGroupId)
{
  int32_t i, j;
  uint16_t numBuffs = 0;
  uint16_t tidlMaxPad = TIDL_MAX_PAD_SIZE;

  for(i = 0; i < net->numLayers; i++)
  {
    if(net->TIDLLayers[i].layersGroupId != layersGroupId)
    {

      for(j = 0; j < net->TIDLLayers[i].numOutBufs; j++)
      {
        if(tidlIsInDataBuff(net, net->TIDLLayers[i].outData[j].dataId, layersGroupId))
        {
          /** Feature width of each input buffer */
          ioBufDesc->inWidth[numBuffs]= net->TIDLLayers[i].outData[j].dimValues[TIDL_DIM_WIDTH];
          /** Feature Height of each input buffer */
          ioBufDesc->inHeight[numBuffs]= net->TIDLLayers[i].outData[j].dimValues[TIDL_DIM_HEIGHT];
          /** Number of channels in each input buffer */
          ioBufDesc->inNumChannels[numBuffs]= net->TIDLLayers[i].outData[j].dimValues[TIDL_DIM_NUMCH];
          /** Left zero padding required for each input buffer */
          ioBufDesc->inPadL[numBuffs]= tidlMaxPad;
          /** Top zero padding required for each input buffer */
          ioBufDesc->inPadT[numBuffs]= tidlMaxPad;
          /** Right zero padding required for each input buffer */
          ioBufDesc->inPadR[numBuffs]= tidlMaxPad;
          /** Bottom zero padding required for each input buffer */
          ioBufDesc->inPadB[numBuffs]= tidlMaxPad;
          /** Element type of each input buffer @ref eTIDL_ElementType */
          ioBufDesc->inElementType[numBuffs]= net->TIDLLayers[i].outData[j].elementType;
          /** Data ID as per Net structure for each input buffer */
          ioBufDesc->inDataId[numBuffs]= net->TIDLLayers[i].outData[j].dataId;;

          numBuffs++;
        }
      }
    }
  }

  ioBufDesc->numInputBuf= numBuffs;

  return numBuffs;
}

/**
 *******************************************************************************
 *
 * \brief tidlIsOutDataBuff() validates whether the particular buffer with 'dataId' is an output buffer from a layer of layersGroupId
 * The search actually look for a buffer with id 'dataId' that is input to a layer different than layersGroupId
 * \return  0: the buffer is not an output buffer from layersGroupId | 1: the buffer is an output buffer from layer layersGroupId
 *
 *******************************************************************************
 */
static int32_t tidlIsOutDataBuff(
    sTIDL_Network_t *pTIDLNetStructure,
    int32_t dataId,
    int32_t layersGroupId)
{
  int32_t i,j;

  for(i = 0 ; i < pTIDLNetStructure->numLayers; i++)
  {
    for(j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
    {
      if((pTIDLNetStructure->TIDLLayers[i].layersGroupId != layersGroupId) &&
          (pTIDLNetStructure->TIDLLayers[i].inData[j].dataId == dataId))
      {
        return 1;
      }
    }
  }

  return 0;
}

/**
 *******************************************************************************
 *
 * \brief tidlGetNumOutputBuffers() counts the number of output buffers from the network
 *  and populates the structure ioBufDesc with information on each output buffer.
 * \return  Number of output buffers
 *
 *******************************************************************************
 */
static uint32_t tidlGetNumOutputBuffers(
    sTIDL_Network_t *net,
    sTIDL_IOBufDesc_t *ioBufDesc,
    int32_t layersGroupId)
{
  int32_t i, j;
  uint16_t numBuffs = 0;
  uint16_t tidlMaxPad = TIDL_MAX_PAD_SIZE;

  for(i = 0; i < net->numLayers; i++)
  {
    if(net->TIDLLayers[i].layersGroupId == layersGroupId)
    {

      for(j = 0; j < net->TIDLLayers[i].numOutBufs; j++)
      {
        if(tidlIsOutDataBuff(net, net->TIDLLayers[i].outData[j].dataId, layersGroupId))
        {
          /** Feature width of each input buffer */
          ioBufDesc->outWidth[numBuffs]= net->TIDLLayers[i].outData[j].dimValues[TIDL_DIM_WIDTH];
          /** Feature Height of each input buffer */
          ioBufDesc->outHeight[numBuffs]= net->TIDLLayers[i].outData[j].dimValues[TIDL_DIM_HEIGHT];
          /** Number of channels in each input buffer */
          ioBufDesc->outNumChannels[numBuffs]= net->TIDLLayers[i].outData[j].dimValues[TIDL_DIM_NUMCH];
          /** Left zero padding required for each input buffer */
          ioBufDesc->outPadL[numBuffs]= tidlMaxPad;
          /** Top zero padding required for each input buffer */
          ioBufDesc->outPadT[numBuffs]= tidlMaxPad;
          /** Right zero padding required for each input buffer */
          ioBufDesc->outPadR[numBuffs]= tidlMaxPad;
          /** Bottom zero padding required for each input buffer */
          ioBufDesc->outPadB[numBuffs]= tidlMaxPad;
          /** Element type of each input buffer @ref eTIDL_ElementType */
          ioBufDesc->outElementType[numBuffs]= net->TIDLLayers[i].outData[j].elementType;
          /** Data ID as per Net structure for each input buffer */
          ioBufDesc->outDataId[numBuffs]= net->TIDLLayers[i].outData[j].dataId;;

          numBuffs++;
        }
      }
    }
  }

  ioBufDesc->numOutputBuf= numBuffs;

  return numBuffs;
}

int32_t vx_tidl_utils_getNetParamsTotalSize(sTIDL_Network_t *net, uint32_t *pTotalSize) {

  int32_t i;
  uint32_t totalSize;

  totalSize= DEFAULT_ALIGN;

  for(i = 0; i < net->numLayers; i++)
  {
    if((TIDL_ConvolutionLayer == net->TIDLLayers[i].layerType) ||
        (TIDL_Deconv2DLayer == net->TIDLLayers[i].layerType))
    {
      sTIDL_ConvParams_t *conv2dPrms = \
          &net->TIDLLayers[i].layerParams.convParams;
      conv2dPrms->weights.bufSize = \
          net->weightsElementSize * (conv2dPrms->kernelW * conv2dPrms->kernelH *
              conv2dPrms->numInChannels * conv2dPrms->numOutChannels)
              /conv2dPrms->numGroups;
      totalSize+= conv2dPrms->weights.bufSize + DEFAULT_ALIGN;

      conv2dPrms->bias.bufSize = net->biasElementSize * conv2dPrms->numOutChannels;
      totalSize+= conv2dPrms->bias.bufSize + DEFAULT_ALIGN;

    }
    else if(TIDL_BiasLayer == net->TIDLLayers[i].layerType)
    {
      sTIDL_BiasParams_t *biasPrms = &net->TIDLLayers[i].layerParams.biasParams;
      biasPrms->bias.bufSize = net->biasElementSize * biasPrms->numChannels;
      totalSize+= biasPrms->bias.bufSize + DEFAULT_ALIGN;
    }
    else if(TIDL_BatchNormLayer == net->TIDLLayers[i].layerType)
    {
      sTIDL_BatchNormParams_t *batchNormPrms = \
          &net->TIDLLayers[i].layerParams.batchNormParams;
      batchNormPrms->weights.bufSize = \
          net->weightsElementSize * batchNormPrms->numChannels;
      totalSize+= batchNormPrms->weights.bufSize + DEFAULT_ALIGN;

      batchNormPrms->bias.bufSize = \
          net->biasElementSize * batchNormPrms->numChannels;
      totalSize+= batchNormPrms->bias.bufSize;

      batchNormPrms->reluParams.slope.bufSize =
          net->slopeElementSize * batchNormPrms->numChannels;
      totalSize+= batchNormPrms->reluParams.slope.bufSize + DEFAULT_ALIGN;
      if(TIDL_PRelU == batchNormPrms->reluParams.reluType)
      {
      }
    }
    else if(TIDL_InnerProductLayer == net->TIDLLayers[i].layerType)
    {
      sTIDL_InnerProductParams_t *ipPrms = \
          &net->TIDLLayers[i].layerParams.innerProductParams;
      ipPrms->bias.bufSize =  net->biasElementSize * ipPrms->numOutNodes;
      totalSize+= ipPrms->bias.bufSize + 128 + DEFAULT_ALIGN;

      ipPrms->weights.bufSize = \
          net->weightsElementSize* ipPrms->numInNodes * ipPrms->numOutNodes;
      totalSize+= ipPrms->weights.bufSize + 16*ipPrms->numInNodes + 1024 + DEFAULT_ALIGN;
    }
    else if(TIDL_DetectionOutputLayer == net->TIDLLayers[i].layerType)
    {
      sTIDL_DetectOutputParams_t *detectPrms = \
          &net->TIDLLayers[i].layerParams.detectOutParams;
      detectPrms->priorBox.bufSize =  detectPrms->priorBoxSize * sizeof(float);
      totalSize+= detectPrms->priorBox.bufSize + DEFAULT_ALIGN;
    }
  }

  *pTotalSize= totalSize;

  return 0;
}

vx_user_data_object vx_tidl_utils_readNetwork(vx_context context, char *network_file)
{
  vx_status status;

  vx_user_data_object  network;
  vx_map_id  map_id;
  vx_uint32  capacity;
  vx_uint32  paramsSize;
  vx_size read_count;
  void      *tmpNetBuf= NULL;
  void      *network_buffer = NULL;

  FILE *fp_network;

  fp_network = fopen(network_file, "rb");

  if(fp_network == NULL)
  {
    printf("Unable to open file! %s \n", network_file);
    return NULL;
  }

  fseek(fp_network, 0, SEEK_END);
  capacity = ftell(fp_network);
  fseek(fp_network, 0, SEEK_SET);

  if (capacity != sizeof(sTIDL_Network_t)) {
    printf("Network file size is different than sTIDL_Network_t structure size !\n");
    return NULL;
  }

  tmpNetBuf= tivxMemAlloc(capacity, (vx_enum)TIVX_MEM_EXTERNAL);

  if(tmpNetBuf)
  {
    read_count = fread(tmpNetBuf, capacity, 1, fp_network);
    if(read_count != 1)
    {
      printf("Unable to read network file %s !\n", network_file);
    }
    vx_tidl_utils_getNetParamsTotalSize((sTIDL_Network_t *)tmpNetBuf, &paramsSize);
  }
  else
  {
    printf("Unable to allocate memory for reading network! %d bytes\n", capacity);
  }
  /* the 1 extra byte will be used as flag to indicate if each pointer to different layer's parameters has already been be converted from shared to target
   * in order to avoid doing the conversion twice which would be incorrect
   */
  network = vxCreateUserDataObject(context, "TIDL_network", capacity + 1 + paramsSize, NULL );
  status = vxGetStatus((vx_reference)network);

  if ((vx_status)VX_SUCCESS == status)
  {
    status = vxMapUserDataObject(network, 0, capacity + 1 + paramsSize, &map_id,
        (void **)&network_buffer, (vx_enum)VX_WRITE_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

    if ((vx_status)VX_SUCCESS == status)
    {
      if(network_buffer)
      {
        memcpy(network_buffer, tmpNetBuf, capacity);
      }
      else
      {
        printf("Unable to map user data object to TIDL_network\n");
      }

      vxUnmapUserDataObject(network, map_id);
      tivxMemFree(tmpNetBuf, capacity, (vx_enum)TIVX_MEM_EXTERNAL);
    }
  }

  fclose(fp_network);

  return network;
}

vx_status vx_tidl_utils_updateLayersGroup(vx_user_data_object  network, vx_enum target_cpu) {

  vx_status status = (vx_status)VX_SUCCESS;
  void      *network_buffer = NULL;
  vx_map_id  map_id_network;
  sTIDL_Network_t *net;
  int32_t i;

  status = vxMapUserDataObject(network, 0, sizeof(sTIDL_Network_t), &map_id_network,
      (void **)&network_buffer, (vx_enum)VX_READ_AND_WRITE, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

  if ((vx_status)VX_SUCCESS == status)
        {
          if(network_buffer)
          {
            net= (sTIDL_Network_t *)network_buffer;
            uint32_t layersGroupId;

            if ((target_cpu == (vx_enum)TIVX_CPU_ID_DSP1) || (target_cpu == (vx_enum)TIVX_CPU_ID_DSP2)) {
              layersGroupId= 2;
            }
            else if ((target_cpu == (vx_enum)TIVX_CPU_ID_EVE1) || (target_cpu == (vx_enum)TIVX_CPU_ID_EVE2) || (target_cpu == (vx_enum)TIVX_CPU_ID_EVE3) || (target_cpu == (vx_enum)TIVX_CPU_ID_EVE4)) {
              layersGroupId= 1;
            }
            else {
              layersGroupId= 0;
            }

            for (i = 0; i < net->numLayers; i++)
            {
              if (net->TIDLLayers[i].layerType != TIDL_DataLayer)
              {
                net->TIDLLayers[i].layersGroupId = layersGroupId;
              }
            }

            vxUnmapUserDataObject(network, map_id_network);
          }
        }

  return status;
}

int32_t vx_tidl_utils_countLayersGroup(vx_user_data_object  network, int32_t layersGroupCount[(vx_enum)TIVX_CPU_ID_MAX]) {

  vx_status status = (vx_status)VX_SUCCESS;
  void      *network_buffer = NULL;
  vx_map_id  map_id_network;
  sTIDL_Network_t *net;
  int32_t i;
  uint32_t numLayersGroup;

  numLayersGroup= 0;

  status = vxMapUserDataObject(network, 0, sizeof(sTIDL_Network_t), &map_id_network,
      (void **)&network_buffer, (vx_enum)VX_READ_AND_WRITE, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

  if ((vx_status)VX_SUCCESS == status)
        {
          if(network_buffer)
          {
            net= (sTIDL_Network_t *)network_buffer;

            for (i=0; i < (vx_enum)TIVX_CPU_ID_MAX; i++) {
              layersGroupCount[i]= 0;
            }

            for (i = 0; i < net->numLayers; i++)
            {
              if (net->TIDLLayers[i].layerType != TIDL_DataLayer)
              {
                if (net->TIDLLayers[i].layersGroupId < (vx_enum)TIVX_CPU_ID_MAX) {
                  layersGroupCount[net->TIDLLayers[i].layersGroupId]++;
                }
              }
            }

            for (i=0; i < (vx_enum)TIVX_CPU_ID_MAX; i++) {
              if (layersGroupCount[i]!= 0) {
                numLayersGroup++;
              }
            }

            vxUnmapUserDataObject(network, map_id_network);
          }

        }

  return numLayersGroup;
}

vx_user_data_object vx_tidl_utils_getConfig(vx_context context, vx_user_data_object  network, uint32_t *num_input_tensors, uint32_t *num_output_tensors, vx_enum target_cpu)
{
  vx_status status = (vx_status)VX_SUCCESS;
  vx_map_id map_id_config;
  sTIDL_IOBufDesc_t *ioBufDesc;
  vx_user_data_object   config= NULL;

  /* Create a user struct type for handling config data*/
  config = vxCreateUserDataObject(context, "sTIDL_IOBufDesc_t", sizeof(sTIDL_IOBufDesc_t), NULL );

  status = vxGetStatus((vx_reference)config);

  if ((vx_status)VX_SUCCESS == status)
  {
    status = vxMapUserDataObject(config, 0, sizeof(sTIDL_IOBufDesc_t), &map_id_config,
        (void **)&ioBufDesc, (vx_enum)VX_WRITE_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

    if ((vx_status)VX_SUCCESS == status)
    {
      void      *network_buffer = NULL;
      vx_map_id  map_id_network;
      sTIDL_Network_t *net;

      status = vxMapUserDataObject(network, 0, sizeof(sTIDL_Network_t), &map_id_network,
          (void **)&network_buffer, (vx_enum)VX_READ_AND_WRITE, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

      if ((vx_status)VX_SUCCESS == status)
      {
        if(network_buffer)
        {
          net= (sTIDL_Network_t *)network_buffer;
          uint32_t currLayersGroupId;

          if ((target_cpu == (vx_enum)TIVX_CPU_ID_DSP1) || (target_cpu == (vx_enum)TIVX_CPU_ID_DSP2)) {
            currLayersGroupId= 2;
          }
          else if ((target_cpu == (vx_enum)TIVX_CPU_ID_EVE1) || (target_cpu == (vx_enum)TIVX_CPU_ID_EVE2) || (target_cpu == (vx_enum)TIVX_CPU_ID_EVE3) || (target_cpu == (vx_enum)TIVX_CPU_ID_EVE4)) {
            currLayersGroupId= 1;
          }
          else {
            currLayersGroupId= 0;
          }
          *num_input_tensors= tidlGetNumInputBuffers(net, ioBufDesc, currLayersGroupId);
          *num_output_tensors= tidlGetNumOutputBuffers(net, ioBufDesc, currLayersGroupId);

          vxUnmapUserDataObject(network, map_id_network);
        }
      }
    }
    *num_input_tensors  = ioBufDesc->numInputBuf;
    *num_output_tensors = ioBufDesc->numOutputBuf;

    vxUnmapUserDataObject(config, map_id_config);
  }
  else
  {
    printf("Unable to allocate memory for generating the configs! %d bytes\n", sizeof(sTIDL_IOBufDesc_t));
  }

  return config;
}

vx_status vx_tidl_utils_readParams(vx_user_data_object  network, char *params_file)
{
  vx_status status = (vx_status)VX_SUCCESS;

  int32_t i;
  uint32_t dataSize, readSize;
  void      *network_buffer = NULL;
  vx_map_id  map_id;
  sTIDL_Network_t *net;
  uint8_t *pParams, *pFlagShared2Target;

  status = vxMapUserDataObject(network, 0, 0, &map_id,
      (void **)&network_buffer, (vx_enum)VX_READ_AND_WRITE, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

  if ((vx_status)VX_SUCCESS == status)
  {
    if(network_buffer) {

      FILE *fp_params;

      net= (sTIDL_Network_t *)network_buffer;
      fp_params = fopen(params_file, "rb");

      if(fp_params == NULL)
      {
        printf("Unable to open file! %s \n", params_file);
        return (vx_status)VX_FAILURE;
      }

      /* pFlagShared2Target flag is used to indicate if each pointer to different layer's parameters has already been be converted from shared to target
       * in order to avoid doing the conversion twice which would be incorrect.
       * Initially set it to 0.
       * After call to tidl_convertNetParamsPtr() in vx_tidl_target_tda2x3x.c, the kernel will set this flag to 1
       * in order to prevent multiple call to tidl_convertNetParamsPtr(), which would be incorrect.
       * Indeed the same network can be passed to more than one TI-DL node.
       */
      pFlagShared2Target= (uint8_t*)net + sizeof(sTIDL_Network_t);
      *pFlagShared2Target= 0;

      pParams= (uint8_t*)net + sizeof(sTIDL_Network_t) + 1;

      for(i = 0; i < net->numLayers; i++)
      {
        if((TIDL_ConvolutionLayer == net->TIDLLayers[i].layerType) ||
            (TIDL_Deconv2DLayer == net->TIDLLayers[i].layerType))
        {
          sTIDL_ConvParams_t *conv2dPrms = &net->TIDLLayers[i].layerParams.convParams;
          pParams= (uint8_t *)align((uintptr_t)pParams, DEFAULT_ALIGN);
          conv2dPrms->weights.ptr= (void*)pParams;
          pParams+= conv2dPrms->weights.bufSize;

          pParams= (uint8_t *)align((uintptr_t)pParams, DEFAULT_ALIGN);
          conv2dPrms->bias.ptr= (void*)pParams;
          pParams+= conv2dPrms->bias.bufSize;

          {
            dataSize = (conv2dPrms->numInChannels * conv2dPrms->numOutChannels *
                conv2dPrms->kernelW * conv2dPrms->kernelH)/conv2dPrms->numGroups;

            /* Read weights based on its size */

            readSize= fread(
                (uint8_t*)conv2dPrms->weights.ptr,
                1, (dataSize * net->weightsElementSize),
                fp_params);
            if (readSize != (dataSize * net->weightsElementSize)) {
              assert(readSize == (dataSize * net->weightsElementSize));
            }

            if(conv2dPrms->enableBias)
            {
              dataSize = conv2dPrms->numOutChannels;

              readSize= fread(
                  (uint8_t*)conv2dPrms->bias.ptr,
                  1, (dataSize * 2),
                  fp_params);
              assert(readSize == (dataSize * 2));
            }
            else
            {
              memset(
                  (int8_t *)conv2dPrms->bias.ptr,0,conv2dPrms->numOutChannels * 2);
            }
          }

          conv2dPrms->weights.ptr= (void*)(uintptr_t)tivxMemHost2SharedPtr((uint64_t)(uintptr_t)conv2dPrms->weights.ptr, (vx_enum)TIVX_MEM_EXTERNAL);
          conv2dPrms->bias.ptr= (void*)(uintptr_t)tivxMemHost2SharedPtr((uint64_t)(uintptr_t)conv2dPrms->bias.ptr, (vx_enum)TIVX_MEM_EXTERNAL);
        }
        else if(TIDL_BiasLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_BiasParams_t *biasPrms =&net->TIDLLayers[i].layerParams.biasParams;
          pParams= (uint8_t *)align((uintptr_t)pParams, DEFAULT_ALIGN);
          biasPrms->bias.ptr= (void*)pParams;
          pParams+= biasPrms->bias.bufSize;
          dataSize = biasPrms->numChannels;

          readSize= fread(
              (uint8_t*)biasPrms->bias.ptr,
              1, (dataSize * 2),
              fp_params);
          assert(readSize == (dataSize * 2));

          biasPrms->bias.ptr= (void*)(uintptr_t)tivxMemHost2SharedPtr((uint64_t)(uintptr_t)biasPrms->bias.ptr, (vx_enum)TIVX_MEM_EXTERNAL);
        }
        else if(TIDL_BatchNormLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_BatchNormParams_t *bNPrms = \
              &net->TIDLLayers[i].layerParams.batchNormParams;
          pParams= (uint8_t *)align((uintptr_t)pParams, DEFAULT_ALIGN);
          bNPrms->weights.ptr= (void*)pParams;
          pParams+= bNPrms->weights.bufSize;
          dataSize= bNPrms->numChannels;

          readSize= fread(
              (uint8_t*)bNPrms->weights.ptr,
              1, (dataSize * net->weightsElementSize),
              fp_params);
          assert(readSize == (dataSize * net->weightsElementSize));

          bNPrms->weights.ptr= (void*)(uintptr_t)tivxMemHost2SharedPtr((uint64_t)(uintptr_t)bNPrms->weights.ptr, (vx_enum)TIVX_MEM_EXTERNAL);

          pParams= (uint8_t *)align((uintptr_t)pParams, DEFAULT_ALIGN);
          bNPrms->bias.ptr= (void*)pParams;
          pParams+= bNPrms->bias.bufSize;

          readSize= fread(
              (uint8_t*)bNPrms->bias.ptr,
              1, (dataSize * 2),
              fp_params);
          assert(readSize == (dataSize * 2));

          bNPrms->bias.ptr= (void*)(uintptr_t)tivxMemHost2SharedPtr((uint64_t)(uintptr_t)bNPrms->bias.ptr, (vx_enum)TIVX_MEM_EXTERNAL);

          if(TIDL_PRelU == bNPrms->reluParams.reluType)
          {
            pParams= (uint8_t *)align((uintptr_t)pParams, DEFAULT_ALIGN);
            bNPrms->reluParams.slope.ptr= (void*)pParams;
            pParams+= bNPrms->reluParams.slope.bufSize;

            readSize= fread(
                (uint8_t*)bNPrms->reluParams.slope.ptr,
                1, (dataSize * net->slopeElementSize),
                fp_params);
            assert(readSize == (dataSize * net->slopeElementSize));

            bNPrms->reluParams.slope.ptr= (void*)(uintptr_t)tivxMemHost2SharedPtr((uint64_t)(uintptr_t)bNPrms->reluParams.slope.ptr, (vx_enum)TIVX_MEM_EXTERNAL);
          }
        }
        else if(TIDL_InnerProductLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_InnerProductParams_t *ipPrms = \
              &net->TIDLLayers[i].layerParams.innerProductParams;
          pParams= (uint8_t *)align((uintptr_t)pParams, DEFAULT_ALIGN);
          ipPrms->weights.ptr= (void*)pParams;
          pParams+= ipPrms->weights.bufSize;
          dataSize = ipPrms->numInNodes * ipPrms->numOutNodes;

          /* Read weights */

          readSize= fread(
              (uint8_t*)ipPrms->weights.ptr,
              1, (dataSize * net->weightsElementSize),
              fp_params);
          assert(readSize == (dataSize * net->weightsElementSize));

          ipPrms->weights.ptr= (void*)(uintptr_t)tivxMemHost2SharedPtr((uint64_t)(uintptr_t)ipPrms->weights.ptr, (vx_enum)TIVX_MEM_EXTERNAL);
          pParams= (uint8_t *)align((uintptr_t)pParams, DEFAULT_ALIGN);
          ipPrms->bias.ptr= (void*)pParams;
          pParams+= ipPrms->bias.bufSize;
          dataSize = ipPrms->numOutNodes;

          readSize= fread(
              (uint8_t*)ipPrms->bias.ptr,
              1, (dataSize * 2),
              fp_params);
          assert(readSize == (dataSize * 2));

          ipPrms->bias.ptr= (void*)(uintptr_t)tivxMemHost2SharedPtr((uint64_t)(uintptr_t)ipPrms->bias.ptr, (vx_enum)TIVX_MEM_EXTERNAL);
        }
        else if(TIDL_DetectionOutputLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_DetectOutputParams_t *detectPrms = \
              &net->TIDLLayers[i].layerParams.detectOutParams;
          pParams= (uint8_t *)align((uintptr_t)pParams, DEFAULT_ALIGN);
          detectPrms->priorBox.ptr= (void*)pParams;
          pParams+= detectPrms->priorBox.bufSize;
          dataSize = detectPrms->priorBoxSize;

          readSize= fread(
              (uint8_t*)detectPrms->priorBox.ptr,
              1, (dataSize * 4),
              fp_params);
          assert(readSize == (dataSize * 4));

          detectPrms->priorBox.ptr= (void*)(uintptr_t)tivxMemHost2SharedPtr((uint64_t)(uintptr_t)detectPrms->priorBox.ptr, (vx_enum)TIVX_MEM_EXTERNAL);
        }
      }

      fclose(fp_params);

      vxUnmapUserDataObject(network, map_id);

    }
  }

  return status;
}

vx_user_data_object vx_tidl_utils_setCreateParams(vx_context context, int32_t quantHistoryBoot, int32_t quantHistory, int32_t quantMargin)
{
    vx_status status;

    vx_user_data_object  createParams;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *createParams_buffer = NULL;

    capacity = sizeof(TIDL_CreateParams);
    createParams = vxCreateUserDataObject(context, "TIDL_CreateParams", capacity, NULL );

    status = vxGetStatus((vx_reference)createParams);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = vxMapUserDataObject(createParams, 0, capacity, &map_id,
                        (void **)&createParams_buffer, (vx_enum)VX_WRITE_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

        if ((vx_status)VX_SUCCESS == status)
        {
            if(createParams_buffer)
            {
              TIDL_CreateParams *prms = createParams_buffer;
              //write create params here

              prms->quantHistoryParam1     = quantHistoryBoot;
              prms->quantHistoryParam2     = quantHistory;
              prms->quantMargin            = quantMargin;

            }
            else
            {
                printf("Unable to allocate memory for create time params! %d bytes\n", capacity);
            }

            vxUnmapUserDataObject(createParams, map_id);
        }
    }

    return createParams;
}

vx_user_data_object vx_tidl_utils_setInArgs(vx_context context)
{
    vx_status status;

    vx_user_data_object  inArgs;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *inArgs_buffer = NULL;

    capacity = sizeof(TIDL_InArgs);
    inArgs = vxCreateUserDataObject(context, "TIDL_InArgs", capacity, NULL );

    status = vxGetStatus((vx_reference)inArgs);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = vxMapUserDataObject(inArgs, 0, capacity, &map_id,
                        (void **)&inArgs_buffer, (vx_enum)VX_WRITE_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

        if ((vx_status)VX_SUCCESS == status)
        {
            if(inArgs_buffer)
            {
              TIDL_InArgs *prms = inArgs_buffer;
              prms->iVisionInArgs.size         = sizeof(TIDL_InArgs);
              prms->iVisionInArgs.subFrameInfo = 0;
            }
            else
            {
                printf("Unable to allocate memory for inArgs! %d bytes\n", capacity);
            }

            vxUnmapUserDataObject(inArgs, map_id);
        }
    }

    return inArgs;
}

vx_user_data_object vx_tidl_utils_setOutArgs(vx_context context)
{
    vx_status status;

    vx_user_data_object  outArgs;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *outArgs_buffer = NULL;

    capacity = sizeof(TIDL_outArgs);
    outArgs = vxCreateUserDataObject(context, "TIDL_outArgs", capacity, NULL );

    status = vxGetStatus((vx_reference)outArgs);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = vxMapUserDataObject(outArgs, 0, capacity, &map_id,
                        (void **)&outArgs_buffer, (vx_enum)VX_WRITE_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

        if ((vx_status)VX_SUCCESS == status)
        {
            if(outArgs_buffer)
            {
              TIDL_outArgs *prms = outArgs_buffer;
              prms->iVisionOutArgs.size         = sizeof(TIDL_outArgs);
            }
            else
            {
                printf("Unable to allocate memory for outArgs! %d bytes\n", capacity);
            }

            vxUnmapUserDataObject(outArgs, map_id);
        }
    }

    return outArgs;
}
