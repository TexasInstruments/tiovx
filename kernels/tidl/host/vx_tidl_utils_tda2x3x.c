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


#define align(val, alignv) ( (( (val) + (alignv) -1)/(alignv))*(alignv) )

static void sparseConv2dCoffesS8(int8_t *ptr, int32_t n, uint8_t thr)
{
  int32_t i0;

  for(i0 = 0; i0 < n; i0++)
  {
    if(((uint8_t)(rand() & 0xFF)) > thr)
    {
      ptr[i0] =  0;
    }
  }
}

static void createRandPatternS16(int16_t *ptr, int16_t n, int16_t width, int16_t height,
    int16_t pitch, int32_t chOffset)
{
  int16_t val;
  int32_t i0, i1, i2;
  for(i0 = 0; i0 < n; i0++)
  {
    for(i1 = 0; i1 < height; i1++)
    {
      for(i2 = 0; i2 < width; i2++)
      {
        val = rand() & 0x7FFF;
        ptr[i0 * chOffset + i1 * pitch + i2] = (rand() & 1) ? val : -val;
      }
    }
  }
}

static void createRandPatternS8(int8_t *ptr, int16_t roi, int16_t n,
    int16_t width, int16_t height, int16_t pitch,
    int32_t chOffset)
{
  int16_t val;
  int32_t   i0, i1, i2, i3;

  for(i3 = 0; i3 < roi; i3++)
  {
    for(i0 = 0; i0 < n; i0++)
    {
      for(i1 = 0; i1 < height; i1++)
      {
        for(i2 = 0; i2 < width; i2++)
        {
          val = rand() & 0x7F;
          ptr[i3 * n * chOffset + i0 * chOffset + i1 * pitch +i2] = \
              (rand() & 1) ? val : -val;
        }
      }
    }
  }
}


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
 * \brief tidlIsInDataBuff() validates whether the particular buffer with 'dataId' is an output buffer from a layer of layersGroupId
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

vx_user_data_object vx_tidl_utils_readNetwork(vx_context context, char *network_file)
{
  vx_status status;

  vx_user_data_object  network;
  vx_map_id  map_id;
  vx_uint32  capacity;
  vx_size read_count;
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

  network = vxCreateUserDataObject(context, "TIDL_network", capacity, NULL );
  status = vxGetStatus((vx_reference)network);

  if (VX_SUCCESS == status)
  {
    status = vxMapUserDataObject(network, 0, capacity, &map_id,
        (void **)&network_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

    if (VX_SUCCESS == status)
    {
      if(network_buffer)
      {
        read_count = fread(network_buffer, capacity, 1, fp_network);
        if(read_count != 1)
        {
          printf("Unable to read network file %s !\n", network_file);
        }
      }
      else
      {
        printf("Unable to allocate memory for reading network! %d bytes\n", capacity);
      }

      vxUnmapUserDataObject(network, map_id);
    }
  }

  fclose(fp_network);

  return network;
}

vx_user_data_object vx_tidl_utils_getConfig(vx_context context, vx_user_data_object  network, uint32_t *num_input_tensors, uint32_t *num_output_tensors)
{
  vx_status status = VX_SUCCESS;
  vx_map_id map_id_config;
  sTIDL_IOBufDesc_t *ioBufDesc;
  vx_user_data_object   config= NULL;

  /* Create a user struct type for handling config data*/
  config = vxCreateUserDataObject(context, "sTIDL_IOBufDesc_t", sizeof(sTIDL_IOBufDesc_t), NULL );

  status = vxGetStatus((vx_reference)config);

  if (VX_SUCCESS == status)
  {
    status = vxMapUserDataObject(config, 0, sizeof(sTIDL_IOBufDesc_t), &map_id_config,
        (void **)&ioBufDesc, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

    if (VX_SUCCESS == status)
    {
      void      *network_buffer = NULL;
      vx_map_id  map_id_network;
      sTIDL_Network_t *net;

      status = vxMapUserDataObject(network, 0, sizeof(sTIDL_Network_t), &map_id_network,
          (void **)&network_buffer, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);

      if (VX_SUCCESS == status)
      {
        if(network_buffer)
        {
          net= (sTIDL_Network_t *)network_buffer;
          vx_enum target_cpu;
          uint32_t currLayersGroupId;

          target_cpu = VX_TIDL_UTILS_TARGET_CPU;

          if ((target_cpu == TIVX_CPU_ID_DSP1) || (target_cpu == TIVX_CPU_ID_DSP2)) {
            currLayersGroupId= 2;
          }
          else if ((target_cpu == TIVX_CPU_ID_EVE1) || (target_cpu == TIVX_CPU_ID_EVE2) || (target_cpu == TIVX_CPU_ID_EVE3) || (target_cpu == TIVX_CPU_ID_EVE4)) {
            currLayersGroupId= 1;
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

int32_t vx_tidl_utils_allocNetParams(vx_user_data_object  network) {

  vx_status status;
  int32_t i;
  void      *network_buffer = NULL;
  vx_map_id  map_id;
  sTIDL_Network_t *net;

  status = vxMapUserDataObject(network, 0, sizeof(sTIDL_Network_t), &map_id,
      (void **)&network_buffer, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);

  if (VX_SUCCESS == status)
  {
    if(network_buffer)
    {
      net= (sTIDL_Network_t *)network_buffer;

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
          conv2dPrms->weights.ptr = \
              tivxMemAlloc(conv2dPrms->weights.bufSize, TIVX_MEM_EXTERNAL);

          conv2dPrms->bias.bufSize = net->biasElementSize * conv2dPrms->numOutChannels;
          conv2dPrms->bias.ptr = \
              tivxMemAlloc(conv2dPrms->bias.bufSize, TIVX_MEM_EXTERNAL);
        }
        else if(TIDL_BiasLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_BiasParams_t *biasPrms = &net->TIDLLayers[i].layerParams.biasParams;
          biasPrms->bias.bufSize = net->biasElementSize * biasPrms->numChannels;
          biasPrms->bias.ptr = \
              tivxMemAlloc(biasPrms->bias.bufSize, TIVX_MEM_EXTERNAL);
        }
        else if(TIDL_BatchNormLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_BatchNormParams_t *batchNormPrms = \
              &net->TIDLLayers[i].layerParams.batchNormParams;
          batchNormPrms->weights.bufSize = \
              net->weightsElementSize * batchNormPrms->numChannels;
          batchNormPrms->weights.ptr =
              tivxMemAlloc(batchNormPrms->weights.bufSize, TIVX_MEM_EXTERNAL);

          batchNormPrms->bias.bufSize = \
              net->biasElementSize * batchNormPrms->numChannels;
          batchNormPrms->bias.ptr =
              tivxMemAlloc(batchNormPrms->bias.bufSize, TIVX_MEM_EXTERNAL);

          batchNormPrms->reluParams.slope.bufSize =
              net->slopeElementSize * batchNormPrms->numChannels;
          if(TIDL_PRelU == batchNormPrms->reluParams.reluType)
          {
            batchNormPrms->reluParams.slope.ptr = \
                tivxMemAlloc(
                    batchNormPrms->reluParams.slope.bufSize, TIVX_MEM_EXTERNAL);
          }
        }
        else if(TIDL_InnerProductLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_InnerProductParams_t *ipPrms = \
              &net->TIDLLayers[i].layerParams.innerProductParams;
          ipPrms->bias.bufSize =  net->biasElementSize * ipPrms->numOutNodes;
          ipPrms->bias.ptr = \
              tivxMemAlloc(
                  align(ipPrms->bias.bufSize, 128), TIVX_MEM_EXTERNAL);

          ipPrms->weights.bufSize = \
              net->weightsElementSize* ipPrms->numInNodes * ipPrms->numOutNodes;
          ipPrms->weights.ptr = \
              tivxMemAlloc(
                  align((ipPrms->weights.bufSize + 16 * \
                      net->TIDLLayers[i].layerParams.innerProductParams.numInNodes),
                      1024), TIVX_MEM_EXTERNAL);
        }
        else if(TIDL_DetectionOutputLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_DetectOutputParams_t *detectPrms = \
              &net->TIDLLayers[i].layerParams.detectOutParams;
          detectPrms->priorBox.bufSize =  detectPrms->priorBoxSize * sizeof(float);
          detectPrms->priorBox.ptr = tivxMemAlloc(
              align(detectPrms->priorBoxSize *
                  detectPrms->priorBox.bufSize, 128), TIVX_MEM_EXTERNAL);
        }
      }
      vxUnmapUserDataObject(network, map_id);
    }
  }
  return 0;
}

int32_t vx_tidl_utils_freeNetParams(vx_user_data_object  network)
{
  vx_status status;
  int32_t i;
  void      *network_buffer = NULL;
  vx_map_id  map_id;
  sTIDL_Network_t *net;

  status = vxMapUserDataObject(network, 0, sizeof(sTIDL_Network_t), &map_id,
      (void **)&network_buffer, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);

  if (VX_SUCCESS == status)
  {
    if(network_buffer)
    {
      net= (sTIDL_Network_t *)network_buffer;

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
          tivxMemFree(
              conv2dPrms->weights.ptr, conv2dPrms->weights.bufSize, TIVX_MEM_EXTERNAL);

          conv2dPrms->bias.bufSize = net->biasElementSize * conv2dPrms->numOutChannels;
          tivxMemFree(
              conv2dPrms->bias.ptr, conv2dPrms->bias.bufSize, TIVX_MEM_EXTERNAL);
        }
        else if(TIDL_BiasLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_BiasParams_t *biasPrms = &net->TIDLLayers[i].layerParams.biasParams;
          biasPrms->bias.bufSize = net->biasElementSize * biasPrms->numChannels;
          tivxMemFree(
              biasPrms->bias.ptr, biasPrms->bias.bufSize, TIVX_MEM_EXTERNAL);
        }
        else if(TIDL_BatchNormLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_BatchNormParams_t *batchNormPrms = \
              &net->TIDLLayers[i].layerParams.batchNormParams;
          batchNormPrms->weights.bufSize = \
              net->weightsElementSize * batchNormPrms->numChannels;
          tivxMemFree(
              batchNormPrms->weights.ptr, batchNormPrms->weights.bufSize, TIVX_MEM_EXTERNAL);

          batchNormPrms->bias.bufSize = \
              net->biasElementSize * batchNormPrms->numChannels;
          tivxMemFree(
              batchNormPrms->bias.ptr, batchNormPrms->bias.bufSize, TIVX_MEM_EXTERNAL);

          batchNormPrms->reluParams.slope.bufSize =
              net->slopeElementSize * batchNormPrms->numChannels;
          if(TIDL_PRelU == batchNormPrms->reluParams.reluType)
          {
            tivxMemFree(
                batchNormPrms->reluParams.slope.ptr,
                batchNormPrms->reluParams.slope.bufSize, TIVX_MEM_EXTERNAL);
          }
        }
        else if(TIDL_InnerProductLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_InnerProductParams_t *ipPrms = \
              &net->TIDLLayers[i].layerParams.innerProductParams;
          ipPrms->bias.bufSize =  net->biasElementSize * ipPrms->numOutNodes;
          tivxMemFree(
              ipPrms->bias.ptr, align(ipPrms->bias.bufSize, 128), TIVX_MEM_EXTERNAL);


          ipPrms->weights.bufSize = \
              net->weightsElementSize* ipPrms->numInNodes * ipPrms->numOutNodes;
          tivxMemFree(
              ipPrms->weights.ptr,
              align((ipPrms->weights.bufSize + 16 * \
                  net->TIDLLayers[i].layerParams.innerProductParams.numInNodes),
                  1024), TIVX_MEM_EXTERNAL);
        }
      }
      vxUnmapUserDataObject(network, map_id);
    }
  }
  return 0;
}




vx_status vx_tidl_utils_readParams(vx_user_data_object  network, char *params_file)
{
  vx_status status = VX_SUCCESS;

  int32_t i;
  uint32_t dataSize, readSize;
  void      *network_buffer = NULL;
  vx_map_id  map_id;
  sTIDL_Network_t *net;

  status = vxMapUserDataObject(network, 0, sizeof(sTIDL_Network_t), &map_id,
      (void **)&network_buffer, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);

  if (VX_SUCCESS == status)
  {
    if(network_buffer) {

      FILE *fp_params;

      net= (sTIDL_Network_t *)network_buffer;
      fp_params = fopen(params_file, "rb");

      if(fp_params == NULL)
      {
        printf("Unable to open file! %s \n", params_file);
        return VX_FAILURE;
      }

      for(i = 0; i < net->numLayers; i++)
      {
        if((TIDL_ConvolutionLayer == net->TIDLLayers[i].layerType) ||
            (TIDL_Deconv2DLayer == net->TIDLLayers[i].layerType))
        {
          sTIDL_ConvParams_t *conv2dPrms = &net->TIDLLayers[i].layerParams.convParams;

          if(VX_TIDL_UTILS_RANDOM_INPUT)
          {
            createRandPatternS8(
                (int8_t *)conv2dPrms->weights.ptr, 1,
                conv2dPrms->numInChannels/conv2dPrms->numGroups,
                conv2dPrms->numOutChannels,
                conv2dPrms->kernelW * conv2dPrms->kernelH,
                conv2dPrms->numOutChannels,
                conv2dPrms->kernelW * conv2dPrms->kernelW * conv2dPrms->numOutChannels);

            if(VX_TIDL_UTILS_NO_ZERO_COEFF_PERCENT < 100)
            {
              sparseConv2dCoffesS8((int8_t *)conv2dPrms->weights.ptr,
                  conv2dPrms->weights.bufSize,
                  VX_TIDL_UTILS_NO_ZERO_COEFF_PERCENT * 2.55);
            }

            if(conv2dPrms->enableBias)
            {
              dataSize = conv2dPrms->numOutChannels;
              createRandPatternS16((Int16 *)conv2dPrms->bias.ptr,1,dataSize,1,1,1);
            }
            else
            {
              memset(
                  (int8_t *)conv2dPrms->bias.ptr,0,conv2dPrms->numOutChannels * 2);
            }
          }
          else
          {
            dataSize = (conv2dPrms->numInChannels * conv2dPrms->numOutChannels *
                conv2dPrms->kernelW * conv2dPrms->kernelH)/conv2dPrms->numGroups;

            /* Read weights based on its size */
            if(net->weightsElementSize == 2)
            {
              tivxMemBufferMap(conv2dPrms->weights.ptr, dataSize*2,
                  VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

              readSize= fread(
                  (uint8_t*)conv2dPrms->weights.ptr,
                  1, (dataSize * 2),
                  fp_params);
              if (readSize != (dataSize * 2)) {
                assert(readSize == (dataSize * 2));
              }
            }
            else
            {
              tivxMemBufferMap(conv2dPrms->weights.ptr, dataSize,
                  VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

              readSize= fread(
                  (uint8_t*)conv2dPrms->weights.ptr,
                  1, dataSize,
                  fp_params);
              assert(readSize == dataSize);

            }

            if(VX_TIDL_UTILS_NO_ZERO_COEFF_PERCENT < 100)
            {
              sparseConv2dCoffesS8((int8_t *)conv2dPrms->weights.ptr,
                  conv2dPrms->weights.bufSize,
                  VX_TIDL_UTILS_NO_ZERO_COEFF_PERCENT * 2.55);
            }

            if(conv2dPrms->enableBias)
            {
              dataSize = conv2dPrms->numOutChannels;

              tivxMemBufferMap(conv2dPrms->bias.ptr, dataSize*2,
                  VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

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

          /* Cache Wb of the buffers */
          dataSize = (conv2dPrms->numInChannels * conv2dPrms->numOutChannels *
              conv2dPrms->kernelW * conv2dPrms->kernelH)/conv2dPrms->numGroups;
          if(net->weightsElementSize == 2)
          {
            dataSize *= 2;
          }

          tivxMemBufferUnmap(conv2dPrms->weights.ptr, dataSize,
              VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

          dataSize = conv2dPrms->numOutChannels;

          tivxMemBufferUnmap(conv2dPrms->bias.ptr, dataSize,
              VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        }
        else if(TIDL_BiasLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_BiasParams_t *biasPrms =&net->TIDLLayers[i].layerParams.biasParams;
          dataSize = biasPrms->numChannels;
          if(VX_TIDL_UTILS_RANDOM_INPUT)
          {
            tivxMemBufferMap(biasPrms->bias.ptr, dataSize,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            createRandPatternS16((Int16 *)biasPrms->bias.ptr,1,dataSize,1,1,1);
            tivxMemBufferUnmap(biasPrms->bias.ptr, dataSize,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
          }
          else
          {
            tivxMemBufferMap(biasPrms->bias.ptr, dataSize*2,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            readSize= fread(
                (uint8_t*)biasPrms->bias.ptr,
                1, (dataSize * 2),
                fp_params);
            assert(readSize == (dataSize * 2));
            tivxMemBufferUnmap(biasPrms->bias.ptr, dataSize*2,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
          }
        }
        else if(TIDL_BatchNormLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_BatchNormParams_t *bNPrms = \
              &net->TIDLLayers[i].layerParams.batchNormParams;
          dataSize = bNPrms->numChannels;
          if(VX_TIDL_UTILS_RANDOM_INPUT)
          {
            tivxMemBufferMap(bNPrms->weights.ptr, dataSize,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            createRandPatternS16((Int16 *)bNPrms->weights.ptr,1,dataSize,1,1,1);
            tivxMemBufferUnmap(bNPrms->weights.ptr, dataSize,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
          }
          else
          {
            if(net->weightsElementSize == 2)
            {
              tivxMemBufferMap(bNPrms->weights.ptr, dataSize*2,
                  VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
              readSize= fread(
                  (uint8_t*)bNPrms->weights.ptr,
                  1, (dataSize * 2),
                  fp_params);
              assert(readSize == (dataSize * 2));
              tivxMemBufferUnmap(bNPrms->weights.ptr, dataSize*2,
                  VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }
            else
            {
              tivxMemBufferMap(bNPrms->weights.ptr, dataSize,
                  VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
              readSize= fread(
                  (uint8_t*)bNPrms->weights.ptr,
                  1, dataSize,
                  fp_params);
              assert(readSize == dataSize);
              tivxMemBufferUnmap(bNPrms->weights.ptr, dataSize,
                  VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }
          }

          if(VX_TIDL_UTILS_RANDOM_INPUT)
          {
            tivxMemBufferMap(bNPrms->bias.ptr, dataSize,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            createRandPatternS16((Int16 *)bNPrms->bias.ptr, 1,dataSize,1,1, 1);
            tivxMemBufferUnmap(bNPrms->bias.ptr, dataSize,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
          }
          else
          {
            tivxMemBufferMap(bNPrms->bias.ptr, dataSize*2,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            readSize= fread(
                (uint8_t*)bNPrms->bias.ptr,
                1, (dataSize * 2),
                fp_params);
            assert(readSize == (dataSize * 2));
            tivxMemBufferUnmap(bNPrms->bias.ptr, dataSize*2,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
          }

          if(TIDL_PRelU == bNPrms->reluParams.reluType)
          {
            if(VX_TIDL_UTILS_RANDOM_INPUT)
            {
              tivxMemBufferMap(bNPrms->reluParams.slope.ptr, dataSize,
                  VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
              createRandPatternS16((Int16 *)bNPrms->reluParams.slope.ptr,1,dataSize,1,1,1);
              tivxMemBufferUnmap(bNPrms->reluParams.slope.ptr, dataSize,
                  VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }
            else
            {
              if(net->slopeElementSize == 2)
              {
                tivxMemBufferMap(bNPrms->reluParams.slope.ptr, dataSize*2,
                    VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
                readSize= fread(
                    (uint8_t*)bNPrms->reluParams.slope.ptr,
                    1, (dataSize * 2),
                    fp_params);
                assert(readSize == (dataSize * 2));
                tivxMemBufferUnmap(bNPrms->reluParams.slope.ptr, dataSize*2,
                    VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
              }
              else
              {
                tivxMemBufferMap(bNPrms->reluParams.slope.ptr, dataSize,
                    VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
                readSize= fread(
                    (uint8_t*)bNPrms->reluParams.slope.ptr,
                    1, dataSize,
                    fp_params);
                assert(readSize == dataSize);
                tivxMemBufferUnmap(bNPrms->reluParams.slope.ptr, dataSize,
                    VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
              }
            }
          }
        }
        else if(TIDL_InnerProductLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_InnerProductParams_t *ipPrms = \
              &net->TIDLLayers[i].layerParams.innerProductParams;
          dataSize = ipPrms->numInNodes * ipPrms->numOutNodes;
          if(VX_TIDL_UTILS_RANDOM_INPUT)
          {
            tivxMemBufferMap(ipPrms->weights.ptr, dataSize,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            createRandPatternS16((Int16 *)ipPrms->weights.ptr,1,dataSize,1,1,1);
            tivxMemBufferUnmap(ipPrms->weights.ptr, dataSize,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
          }
          else
          {
            /* Read weights based on its size */
            if(net->weightsElementSize == 2)
            {
              tivxMemBufferMap(ipPrms->weights.ptr, dataSize*2,
                  VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
              readSize= fread(
                  (uint8_t*)ipPrms->weights.ptr,
                  1, (dataSize * 2),
                  fp_params);
              assert(readSize == (dataSize * 2));
              tivxMemBufferUnmap(ipPrms->weights.ptr, dataSize*2,
                  VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }
            else
            {
              tivxMemBufferMap(ipPrms->weights.ptr, dataSize,
                  VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
              readSize= fread(
                  (uint8_t*)ipPrms->weights.ptr,
                  1, dataSize,
                  fp_params);
              assert(readSize == dataSize);
              tivxMemBufferUnmap(ipPrms->weights.ptr, dataSize,
                  VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }
          }

          dataSize = ipPrms->numOutNodes;
          if(VX_TIDL_UTILS_RANDOM_INPUT)
          {
            tivxMemBufferMap(ipPrms->bias.ptr, dataSize,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            createRandPatternS16((Int16 *)ipPrms->bias.ptr,1,dataSize,1,1,1);
            tivxMemBufferUnmap(ipPrms->bias.ptr, dataSize,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
          }
          else
          {
            tivxMemBufferMap(ipPrms->bias.ptr, dataSize*2,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            readSize= fread(
                (uint8_t*)ipPrms->bias.ptr,
                1, (dataSize * 2),
                fp_params);
            assert(readSize == (dataSize * 2));
            tivxMemBufferUnmap(ipPrms->bias.ptr, dataSize*2,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
          }
        }
        else if(TIDL_DetectionOutputLayer == net->TIDLLayers[i].layerType)
        {
          sTIDL_DetectOutputParams_t *detectPrms = \
              &net->TIDLLayers[i].layerParams.detectOutParams;
          dataSize = detectPrms->priorBoxSize;
          if(VX_TIDL_UTILS_RANDOM_INPUT)
          {
            tivxMemBufferMap(detectPrms->priorBox.ptr, dataSize*4,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            createRandPatternS16((Int16 *)detectPrms->priorBox.ptr,1,dataSize*4,1,1,1);
            tivxMemBufferUnmap(detectPrms->priorBox.ptr, dataSize*4,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
          }
          else
          {
            tivxMemBufferMap(detectPrms->priorBox.ptr, dataSize*4,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            readSize= fread(
                (uint8_t*)detectPrms->priorBox.ptr,
                1, (dataSize * 4),
                fp_params);
            assert(readSize == (dataSize * 4));
            tivxMemBufferUnmap(detectPrms->priorBox.ptr, dataSize*4,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
          }
        }
      }

      fclose(fp_params);

      vxUnmapUserDataObject(network, map_id);

    }
  }

  return status;
}

