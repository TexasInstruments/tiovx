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
#include <TI/tivx_mem.h>
#include "test_engine/test.h"
#include <float.h>
#include <math.h>
#include "itidl_ti.h"

TESTCASE(tivxTIDL, CT_VXContext, ct_setup_vx_context, 0)

#define TEST_TIDL_MAX_TENSOR_DIMS   (4u)

vx_status tivxAddKernelTIDL(vx_context context, uint32_t num_input_tensors, uint32_t num_output_tensors);
vx_status tivxRemoveKernelTIDL(vx_context context);

static vx_array readConfig(vx_context context, char *config_file, uint32_t *num_input_tensors, uint32_t *num_output_tensors)
{
  vx_status status = VX_SUCCESS;

  sTIDL_IOBufDesc_t *ioBufDesc;

  vx_array   config_array;
  vx_size    stride = sizeof(vx_uint8);
  vx_uint32  capacity;

  FILE *fp_config;

  fp_config = fopen(config_file, "rb");

  if(fp_config == NULL)
  {
     printf("Unable to open file! %s \n", config_file);

     return NULL;
  }

  fseek(fp_config, 0, SEEK_END);
  capacity = ftell(fp_config);
  fseek(fp_config, 0, SEEK_SET);

  ioBufDesc = (sTIDL_IOBufDesc_t *)tivxMemAlloc(capacity, TIVX_MEM_EXTERNAL);

  if(ioBufDesc) {
    fread(ioBufDesc, capacity, stride, fp_config);
  } else {
    printf("Unable to allocate memory for reading network! %d bytes\n", capacity);
    return NULL;
  }

  config_array = vxCreateArray(context, VX_TYPE_UINT8, sizeof(sTIDL_IOBufDesc_t));

  status = vxGetStatus((vx_reference)config_array);

  if (VX_SUCCESS == status)
  {
    vxAddArrayItems(config_array, sizeof(sTIDL_IOBufDesc_t), (void *)ioBufDesc, stride);
  }

  *num_input_tensors  = ioBufDesc->numInputBuf;
  *num_output_tensors = ioBufDesc->numOutputBuf;

  tivxMemFree(ioBufDesc, capacity, TIVX_MEM_EXTERNAL);

  return config_array;
}

static vx_tensor readNetwork(vx_context context, char *network_file)
{
  vx_status status;

  vx_tensor  network_tensor;
  vx_map_id  map_id;
  vx_uint32  capacity;
  vx_size    stride = sizeof(vx_uint8);
  void      *network_buffer = NULL;

  vx_size    start[TEST_TIDL_MAX_TENSOR_DIMS];
  vx_size    network_sizes[TEST_TIDL_MAX_TENSOR_DIMS];
  vx_size    network_strides[TEST_TIDL_MAX_TENSOR_DIMS];

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

  network_sizes[0] = capacity;
  network_tensor = vxCreateTensor(context, 1, network_sizes, VX_TYPE_UINT8, 0);

  status = vxGetStatus((vx_reference)network_tensor);

  if (VX_SUCCESS == status)
  {
    start[0] = 0;
    network_strides[0] = stride;

    status = tivxMapTensorPatch(network_tensor, 1, start, network_sizes, &map_id, network_sizes, network_strides, &network_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

    if (VX_SUCCESS == status)
    {
      if(network_buffer) {
        fread(network_buffer, capacity, stride, fp_network);
      } else {
        printf("Unable to allocate memory for reading network! %d bytes\n", capacity);
      }

      tivxUnmapTensorPatch(network_tensor, map_id);
    }
  }

  fclose(fp_network);

  return network_tensor;
}

static vx_tensor createInputTensor(vx_context context, vx_array config)
{
  vx_size   input_sizes[TEST_TIDL_MAX_TENSOR_DIMS];

  void     *config_buffer = NULL;
  vx_map_id map_id_config;

  sTIDL_IOBufDesc_t *ioBufDesc;
  vx_size stride = sizeof(vx_uint8);

  vxMapArrayRange(config, 0, sizeof(sTIDL_IOBufDesc_t), &map_id_config, &stride, &config_buffer, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

  ioBufDesc = (sTIDL_IOBufDesc_t *)config_buffer;

  input_sizes[0] = ioBufDesc->inWidth[0]  + ioBufDesc->inPadL[0] + ioBufDesc->inPadR[0];
  input_sizes[1] = ioBufDesc->inHeight[0] + ioBufDesc->inPadT[0] + ioBufDesc->inPadB[0];
  input_sizes[2] = ioBufDesc->inNumChannels[0];

  vxUnmapArrayRange(config, map_id_config);

  return vxCreateTensor(context, 3, input_sizes, VX_TYPE_UINT8, 0);
}

static vx_tensor createOutputTensor(vx_context context, vx_array config)
{
  vx_size    output_sizes[TEST_TIDL_MAX_TENSOR_DIMS];

  void     *config_buffer = NULL;
  vx_map_id map_id_config;
  vx_size stride = sizeof(vx_uint8);

  sTIDL_IOBufDesc_t *ioBufDesc;

  vxMapArrayRange(config, 0, sizeof(sTIDL_IOBufDesc_t), &map_id_config, &stride, &config_buffer, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

  ioBufDesc = (sTIDL_IOBufDesc_t *)config_buffer;

  output_sizes[0] = ioBufDesc->outWidth[0]  + ioBufDesc->outPadL[0] + ioBufDesc->outPadR[0];
  output_sizes[1] = ioBufDesc->outHeight[0] + ioBufDesc->outPadT[0] + ioBufDesc->outPadB[0];
  output_sizes[2] = ioBufDesc->outNumChannels[0];

  vxUnmapArrayRange(config, map_id_config);

  return vxCreateTensor(context, 3, output_sizes, VX_TYPE_FLOAT32, 0);
}

static vx_status readInput(vx_context context, vx_array config, vx_tensor input_tensor, char *input_file)
{
  vx_status status;

  status = vxGetStatus((vx_reference)input_tensor);

  if (VX_SUCCESS == status)
  {
    vx_size    stride = sizeof(vx_uint8);
    int32_t    capacity;
    void      *input_buffer = NULL;
    void      *config_buffer = NULL;

    vx_map_id map_id_config;
    vx_map_id map_id_input;

    vx_size    start[TEST_TIDL_MAX_TENSOR_DIMS];
    vx_size    input_strides[TEST_TIDL_MAX_TENSOR_DIMS];
    vx_size    input_sizes[TEST_TIDL_MAX_TENSOR_DIMS];

    sTIDL_IOBufDesc_t *ioBufDesc;

    vxMapArrayRange(config, 0, sizeof(sTIDL_IOBufDesc_t), &map_id_config, &stride, &config_buffer, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)config_buffer;

    input_sizes[0] = ioBufDesc->inWidth[0]  + ioBufDesc->inPadL[0] + ioBufDesc->inPadR[0];
    input_sizes[1] = ioBufDesc->inHeight[0] + ioBufDesc->inPadT[0] + ioBufDesc->inPadB[0];
    input_sizes[2] = ioBufDesc->inNumChannels[0];

    capacity = input_sizes[0] * input_sizes[1] * input_sizes[2];

    start[0] = start[1] = start[2] = 0;

    input_strides[0] = 1;
    input_strides[1] = input_sizes[0];
    input_strides[2] = input_sizes[1] * input_strides[1];

    status = tivxMapTensorPatch(input_tensor, 3, start, input_sizes, &map_id_input, input_sizes, input_strides, &input_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

    if (VX_SUCCESS == status)
    {
      FILE *fp_input;
      fp_input = fopen(input_file, "rb");

      if(fp_input == NULL)
      {
        printf("Unable to open file! %s \n", input_file);
        return VX_FAILURE;
      }

      if(input_buffer) {
    	vx_int32 i, j;

    	//Reset the input buffer, this will take care of padding requirement for TIDL
    	memset(input_buffer, 0, capacity);

    	//Copy the input data at a location of (padH * stride) + padW for each channel
    	for(j = 0; j < ioBufDesc->inNumChannels[0]; j++){

    	  vx_int32 start_offset = (j * input_strides[2]) + (ioBufDesc->inPadT[0] * input_strides[1]) + ioBufDesc->inPadL[0];
      	  vx_uint8 *pIn = (vx_uint8 *)input_buffer + start_offset;

    	  for(i = 0; i < ioBufDesc->inHeight[0]; i++){
            fread(pIn, ioBufDesc->inWidth[0], 1, fp_input);
            pIn += input_strides[1];
    	  }
    	}

      } else {
        printf("Unable to allocate memory for reading network! %d bytes\n", capacity);
      }

      fclose(fp_input);
      tivxUnmapTensorPatch(input_tensor, map_id_input);
    }

    vxUnmapArrayRange(config, map_id_config);

  }

  return status;
}

static void checkOutput(vx_array config, vx_tensor output_tensor, vx_int32 refid, vx_float32 refscore)
{
    vx_status status;
    vx_size output_sizes[TEST_TIDL_MAX_TENSOR_DIMS];

    void     *config_buffer = NULL;
    vx_map_id map_id_config;

    sTIDL_IOBufDesc_t *ioBufDesc;
    vx_size stride = sizeof(vx_uint8);

    vxMapArrayRange(config, 0, sizeof(sTIDL_IOBufDesc_t), &map_id_config, &stride, &config_buffer, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)config_buffer;

    output_sizes[0] = ioBufDesc->outWidth[0]  + ioBufDesc->outPadL[0] + ioBufDesc->outPadR[0];
    output_sizes[1] = ioBufDesc->outHeight[0] + ioBufDesc->outPadT[0] + ioBufDesc->outPadB[0];
    output_sizes[2] = ioBufDesc->outNumChannels[0];

    vxUnmapArrayRange(config, map_id_config);

    status = vxGetStatus((vx_reference)output_tensor);

    if (VX_SUCCESS == status)
    {
      vx_float32 score = FLT_MIN;

      vx_int32 classid = -1, i;
      void *output_buffer;

      vx_map_id map_id_output;

      vx_size output_strides[TEST_TIDL_MAX_TENSOR_DIMS];
 	  vx_size start[TEST_TIDL_MAX_TENSOR_DIMS];

 	  start[0] = start[1] = start[2] = start[3] = 0;

 	  output_strides[0] = 1;
 	  output_strides[1] = output_sizes[0];
 	  output_strides[2] = output_sizes[1] * output_strides[1];

 	  tivxMapTensorPatch(output_tensor, 3, start, output_sizes, &map_id_output, output_sizes, output_strides, &output_buffer, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

      if (VX_SUCCESS == status)
      {
        float *pOut = (float *)output_buffer;

        for(i = 0; i < output_sizes[0]; i++)
        {
          if( pOut[i] > score)
          {
            score = pOut[i];
            classid = i;
          }
        }
      }

      tivxUnmapTensorPatch(output_tensor, map_id_output);

      ASSERT(refid == classid);
      ASSERT(abs(refscore - score) < 0.001);
    }
    else
    {
      ASSERT(0);
    }
}

typedef struct {
    const char* testName;
    const char* network;
} Arg;

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("jacintonet11v2", ARG, "jacintonet11v2"), \

#if 0
    CT_GENERATE_PARAMETERS("inception_v1", ARG, "inception_v1"), \
    CT_GENERATE_PARAMETERS("mobilenetv1", ARG, "mobilenetv1"), \
    CT_GENERATE_PARAMETERS("resnet10", ARG, "resnet10"), \
    CT_GENERATE_PARAMETERS("squeez1", ARG, "squeez1")
#endif

TEST_WITH_ARG(tivxTIDL, testTIDL, Arg, PARAMETERS)
{
  vx_context context = context_->vx_context_;
  vx_graph graph = 0;
  vx_node node = 0;
  vx_kernel kernel = 0;

  vx_perf_t perf_node;
  vx_perf_t perf_graph;

  vx_array  config;
  vx_tensor network;
  vx_tensor input_tensor;
  vx_tensor output_tensor;

  vx_int32    network_id = 0;
  vx_int32    refid[] = {896, 895, 895, 895, 895};
  vx_float32  refscore[] = {1672.485962f, 650.004700f, 824.762573f, 542.191040f, 4.546313};

  if(strcmp(arg_->network, "inception_v1") == 0)
    network_id = 0;
  if(strcmp(arg_->network, "jacintonet11v2") == 0)
    network_id = 1;
  if(strcmp(arg_->network, "resnet10") == 0)
    network_id = 2;
  if(strcmp(arg_->network, "mobilenetv1") == 0)
    network_id = 3;
  if(strcmp(arg_->network, "squeez1") == 0)
    network_id = 4;

  vx_size output_sizes[TEST_TIDL_MAX_TENSOR_DIMS];
  char filepath[MAXPATHLENGTH];
  size_t sz;

  if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DSP1))
  {
    uint32_t num_input_tensors  = 0;
    uint32_t num_output_tensors = 0;

    tivxTIDLLoadKernels(context);

    sz = snprintf(filepath, MAXPATHLENGTH, "%s/tidl_models/%s/config.bin", ct_get_test_file_path(), arg_->network);
    ASSERT(sz < MAXPATHLENGTH);

    ASSERT_VX_OBJECT(config = readConfig(context, &filepath[0], &num_input_tensors, &num_output_tensors), VX_TYPE_ARRAY);

    kernel = tivxAddKernelTIDL(context, num_input_tensors, num_output_tensors);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    sz = snprintf(filepath, MAXPATHLENGTH, "%s/tidl_models/%s/network.bin", ct_get_test_file_path(), arg_->network);
    ASSERT(sz < MAXPATHLENGTH);

    ASSERT_VX_OBJECT(network = readNetwork(context, &filepath[0]), VX_TYPE_TENSOR);

    ASSERT_VX_OBJECT(input_tensor = createInputTensor(context, config), VX_TYPE_TENSOR);

    ASSERT_VX_OBJECT(output_tensor = createOutputTensor(context, config), VX_TYPE_TENSOR);

    vx_reference params[] = {
            (vx_reference)config,
            (vx_reference)network,
            (vx_reference)input_tensor,
            (vx_reference)output_tensor,
    };

    ASSERT_VX_OBJECT(node = tivxTIDLNode(graph, kernel, params, dimof(params)), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_DSP1));

    sz = snprintf(filepath, MAXPATHLENGTH, "%s/tidl_models/%s/input.rgb", ct_get_test_file_path(), arg_->network);
    ASSERT(sz < MAXPATHLENGTH);

    readInput(context, config, input_tensor, &filepath[0]);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    checkOutput(config, output_tensor, refid[network_id], refscore[network_id]);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(kernel == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseArray(&config));
    VX_CALL(vxReleaseTensor(&network));
    VX_CALL(vxReleaseTensor(&input_tensor));
    VX_CALL(vxReleaseTensor(&output_tensor));

    ASSERT(config == 0);
    ASSERT(network == 0);
    ASSERT(input_tensor  == 0);
    ASSERT(output_tensor == 0);

    tivxTIDLUnLoadKernels(context);

  }
}

TESTCASE_TESTS(tivxTIDL, testTIDL)
