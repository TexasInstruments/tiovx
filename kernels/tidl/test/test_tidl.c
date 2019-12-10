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
#include <TI/tivx_mem.h>
#include "test_engine/test.h"
#include <float.h>
#include <math.h>

#include "../../../common/xdais_types.h" /* In TIDL_PATH directory */
#include "sTIDL_IOBufDesc.h"
#include "tivx_tidl_utils.h"

#include "itidl_ti.h"

#define DEBUG_TEST_TIDL

#ifdef HOST_EMULATION
/* This is a workaround to support spanning graphs on different EVE and DSP cores in PC host emulation environment
 * Even though here, we only use one EVE in the context of this test-bench, this array must be defined.
 * Plan to remove this workaround in the future ...
 * */
tivx_cpu_id_e gTidlNodeCpuId[1];
#endif

TESTCASE(tivxTIDL, CT_VXContext, ct_setup_vx_context, 0)

#define PERCENT 0.01
#define TEST_TIDL_MAX_TENSOR_DIMS   (4u)

static vx_tensor createInputTensor(vx_context context, vx_user_data_object config);
static vx_tensor createOutputTensor(vx_context context, vx_user_data_object config);
static vx_status readInput(vx_context context, vx_user_data_object config, vx_tensor *input_tensors, char *input_file);
static vx_status displayOutput(vx_user_data_object config, vx_tensor *output_tensors, vx_int32 refid, vx_uint8 refscore);

typedef struct {
    const char* testName;
    const char* network;
} Arg;

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("jacintonet11v2", ARG, "jacintonet11v2"), \

#if 0
    CT_GENERATE_PARAMETERS("jacintonet11v2", ARG, "jacintonet11v2"), \
    CT_GENERATE_PARAMETERS("inception_v1", ARG, "inception_v1"), \
    CT_GENERATE_PARAMETERS("mobilenetv1", ARG, "mobilenetv1"),
#endif

TEST_WITH_ARG(tivxTIDL, testTIDL, Arg, PARAMETERS)
{
  vx_context context = context_->vx_context_;
  vx_graph graph = 0;
  vx_node node = 0;
  vx_kernel kernel = 0;

  vx_perf_t perf_node;
  vx_perf_t perf_graph;
  int32_t quantHistoryBoot, quantHistory, quantMargin;

  vx_user_data_object  config;
  vx_user_data_object  network;
  vx_user_data_object createParams;
  vx_user_data_object inArgs;
  vx_user_data_object outArgs;
  vx_tensor input_tensors[1];
  vx_tensor output_tensors[1];
  vx_reference params[5];

  vx_int32    network_id = 0;
  vx_int32    refid[] = {896, 895, 0xDEAD, 895, 0xDEAD};
  vx_uint8    refscore[] = {125, 185, 0, 0, 0};
  char *networkFile[]= {"tidl_inception_v1_net.bin",   "tidl_net_imagenet_jacintonet11v2.bin",   "", "tidl_net_mobilenet_1_224.bin",  ""};
  char *paramFile[]=   {"tidl_inception_v1_param.bin", "tidl_param_imagenet_jacintonet11v2.bin", "", "tidl_param_mobilenet_1_224.bin",""};
  char *inputFile[]=   {"preproc_2_224x224.y",         "preproc_0_224x224.y",                    "", "preproc_2_224x224.y",           ""};

  if(strcmp(arg_->network, "inception_v1") == 0)
    network_id = 0;
  if(strcmp(arg_->network, "jacintonet11v2") == 0)
    network_id = 1;
  if(strcmp(arg_->network, "resnet10") == 0) /* Not tested for now */
    network_id = 2;
  if(strcmp(arg_->network, "mobilenetv1") == 0)
    network_id = 3;
  if(strcmp(arg_->network, "squeez1") == 0) /* Not tested for now */
    network_id = 4;

  vx_size output_sizes[TEST_TIDL_MAX_TENSOR_DIMS];
  char filepath[MAXPATHLENGTH];
  size_t sz;

  tivx_clr_debug_zone(VX_ZONE_INFO);

  if ((vx_bool)vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_EVE1))
  {
    uint32_t num_input_tensors  = 0;
    uint32_t num_output_tensors = 0;

    sz = snprintf(filepath, MAXPATHLENGTH, "%s/tivx/tidl/%s", ct_get_test_file_path(), networkFile[network_id]);
    ASSERT(sz < MAXPATHLENGTH);

    ASSERT_VX_OBJECT(network = vx_tidl_utils_readNetwork(context, &filepath[0]), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(config = vx_tidl_utils_getConfig(context, network, &num_input_tensors, &num_output_tensors, TIVX_CPU_ID_EVE1), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    sz = snprintf(filepath, MAXPATHLENGTH, "%s/tivx/tidl/%s", ct_get_test_file_path(), paramFile[network_id]);
    ASSERT(sz < MAXPATHLENGTH);

    VX_CALL(vx_tidl_utils_readParams(network, &filepath[0]));

    kernel = tivxAddKernelTIDL(context, num_input_tensors, num_output_tensors);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(input_tensors[0] = createInputTensor(context, config), (enum vx_type_e)VX_TYPE_TENSOR);

    ASSERT_VX_OBJECT(output_tensors[0] = createOutputTensor(context, config), (enum vx_type_e)VX_TYPE_TENSOR);

    /*
     * TIDL maintains range statistics for previously processed frames. It quantizes the current inference activations using range statistics from history for processes (weighted average range).
     * Below is the parameters controls quantization.
     * quantMargin is margin added to the average in percentage.
     * quantHistoryBoot weights used for previously processed inference during application boot time
     * quantHistory weights used for previously processed inference during application execution (After initial few frames)
     *
     * Below settings are adequate for running on videos sequences.
     * For still images, set all settings to 0.
     */
    quantHistoryBoot= 20;
    quantHistory= 5;
    quantMargin= 0;

    ASSERT_VX_OBJECT(createParams=  vx_tidl_utils_setCreateParams(context, quantHistoryBoot, quantHistory, quantMargin), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
    ASSERT_VX_OBJECT(inArgs= vx_tidl_utils_setInArgs(context), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
    ASSERT_VX_OBJECT(outArgs= vx_tidl_utils_setOutArgs(context), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    params[0]= (vx_reference)config;
    params[1]= (vx_reference)network;
    params[2]= (vx_reference)createParams;
    params[3]= (vx_reference)inArgs;
    params[4]= (vx_reference)outArgs;

    ASSERT_VX_OBJECT(node = tivxTIDLNode(graph, kernel, params, input_tensors, output_tensors), VX_TYPE_NODE);

    /* Set target node to EVE1 */
    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_EVE1));
#ifdef HOST_EMULATION
    /* This is a workaround to support spanning graphs on different EVE and DSP cores in PC host emulation environment
     * Even though here, we only use one EVE in the context of this test-bench, this array must be initialized.
     * */
    gTidlNodeCpuId[0]= TIVX_CPU_ID_EVE1;
#endif

    /* Read input from file and populate the input tensors */
    sz = snprintf(filepath, MAXPATHLENGTH, "%s/tivx/tidl/%s", ct_get_test_file_path(), inputFile[network_id]);
    ASSERT(sz < MAXPATHLENGTH);
    VX_CALL(readInput(context, config, &input_tensors[0], &filepath[0]));

    #ifdef DEBUG_TEST_TIDL
    printf("Verifying graph ...\n");
    #endif
    VX_CALL(vxVerifyGraph(graph));
    #ifdef DEBUG_TEST_TIDL
    printf("Running graph ...\n");
    #endif
    VX_CALL(vxProcessGraph(graph));
    #ifdef DEBUG_TEST_TIDL
    printf("Showing output ...\n");
    #endif

    VX_CALL(displayOutput(config, &output_tensors[0], refid[network_id], refscore[network_id]));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseUserDataObject(&config));
    VX_CALL(vxReleaseUserDataObject(&network));
    VX_CALL(vxReleaseTensor(&input_tensors[0]));
    VX_CALL(vxReleaseTensor(&output_tensors[0]));
    VX_CALL(vxReleaseUserDataObject(&createParams));
    VX_CALL(vxReleaseUserDataObject(&inArgs));
    VX_CALL(vxReleaseUserDataObject(&outArgs));

    ASSERT(config == 0);
    ASSERT(network == 0);

    ASSERT(input_tensors[0]  == 0);
    ASSERT(output_tensors[0] == 0);
    
    vxRemoveKernel(kernel);
  }

#ifdef HOST_EMULATION
  void tivxSetSelfCpuId(vx_enum cpu_id);
  tivxSetSelfCpuId(TIVX_CPU_ID_DSP1);
#endif

  tivx_clr_debug_zone(VX_ZONE_INFO);
}

TESTCASE_TESTS(tivxTIDL, testTIDL)

static vx_tensor createInputTensor(vx_context context, vx_user_data_object config)
{
  vx_size   input_sizes[TEST_TIDL_MAX_TENSOR_DIMS];
  vx_map_id map_id_config;
  sTIDL_IOBufDesc_t *ioBufDesc;

  vxMapUserDataObject(config, 0, sizeof(sTIDL_IOBufDesc_t), &map_id_config,
      (void **)&ioBufDesc, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

  input_sizes[0] = ioBufDesc->inWidth[0]  + ioBufDesc->inPadL[0] + ioBufDesc->inPadR[0];
  input_sizes[1] = ioBufDesc->inHeight[0] + ioBufDesc->inPadT[0] + ioBufDesc->inPadB[0];
  input_sizes[2] = ioBufDesc->inNumChannels[0];

  vxUnmapUserDataObject(config, map_id_config);

  return vxCreateTensor(context, 3, input_sizes, VX_TYPE_UINT8, 0);
}

static vx_tensor createOutputTensor(vx_context context, vx_user_data_object config)
{
  vx_size    output_sizes[TEST_TIDL_MAX_TENSOR_DIMS];
  vx_map_id map_id_config;
  sTIDL_IOBufDesc_t *ioBufDesc;

  vxMapUserDataObject(config, 0, sizeof(sTIDL_IOBufDesc_t), &map_id_config,
      (void **)&ioBufDesc, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

  output_sizes[0] = ioBufDesc->outWidth[0]  + ioBufDesc->outPadL[0] + ioBufDesc->outPadR[0];
  output_sizes[1] = ioBufDesc->outHeight[0] + ioBufDesc->outPadT[0] + ioBufDesc->outPadB[0];
  output_sizes[2] = ioBufDesc->outNumChannels[0];

  vxUnmapUserDataObject(config, map_id_config);

  return vxCreateTensor(context, 3, output_sizes, VX_TYPE_FLOAT32, 0);
}



static vx_status readDataS8(FILE *fp, int8_t *ptr, int32_t n,
    int32_t width, int32_t height, int32_t pitch,
    int32_t chOffset)
{
  int32_t   i0, i1;
  uint32_t readSize;
  vx_status status = VX_SUCCESS;

  for(i0 = 0; i0 < n; i0++)
  {
    for(i1 = 0; i1 < height; i1++)
    {
      readSize= fread(&ptr[i0*chOffset + i1*pitch], 1, width, fp);
      if (readSize != width) {
        status= VX_FAILURE;
        goto exit;
      }
    }
  }

  exit:
  return status;

}


static vx_status readInput(vx_context context, vx_user_data_object config, vx_tensor *input_tensors, char *input_file)
{
  vx_status status = VX_SUCCESS;

  int8_t      *input_buffer = NULL;
  uint32_t   id;

  vx_map_id map_id_config;
  vx_map_id map_id_input;

  vx_size    start[TEST_TIDL_MAX_TENSOR_DIMS];
  vx_size    input_strides[TEST_TIDL_MAX_TENSOR_DIMS];
  vx_size    input_sizes[TEST_TIDL_MAX_TENSOR_DIMS];

  sTIDL_IOBufDesc_t *ioBufDesc;

  FILE *fp;

  fp= fopen(input_file, "rb");

  if(fp==NULL)
  {
    printf("# ERROR: Unable to open input file [%s]\n", input_file);
    return(VX_FAILURE);
  }

  vxMapUserDataObject(config, 0, sizeof(sTIDL_IOBufDesc_t), &map_id_config,
      (void **)&ioBufDesc, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

  for(id = 0; id < ioBufDesc->numInputBuf; id++)
  {
    input_sizes[0] = ioBufDesc->inWidth[id]  + ioBufDesc->inPadL[id] + ioBufDesc->inPadR[id];
    input_sizes[1] = ioBufDesc->inHeight[id] + ioBufDesc->inPadT[id] + ioBufDesc->inPadB[id];
    input_sizes[2] = ioBufDesc->inNumChannels[id];

    start[0] = start[1] = start[2] = 0;

    input_strides[0] = 1;
    input_strides[1] = input_sizes[0];
    input_strides[2] = input_sizes[1] * input_strides[1];

    status = tivxMapTensorPatch(input_tensors[id], 3, start, input_sizes, &map_id_input, input_strides, (void **)&input_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

    if (VX_SUCCESS == status)
    {
      status= readDataS8(
          fp,
          &input_buffer[(ioBufDesc->inPadT[id] * input_strides[1]) + ioBufDesc->inPadL[id]],
          ioBufDesc->inNumChannels[id],
          ioBufDesc->inWidth[id],
          ioBufDesc->inHeight[id],
          input_strides[1],
          input_strides[2]);

      tivxUnmapTensorPatch(input_tensors[id], map_id_input);

      if (status== VX_FAILURE) {
        goto exit;
      }
    }
  }

  exit:
  vxUnmapUserDataObject(config, map_id_config);

  fclose(fp);

  return status;
}


static vx_status displayOutput(vx_user_data_object config, vx_tensor *output_tensors, vx_int32 refid, vx_uint8 refscore)
{
    vx_status status = VX_SUCCESS;
    vx_uint8 score[5];
    vx_uint32 classid[5];

    vx_size output_sizes[TEST_TIDL_MAX_TENSOR_DIMS];

    vx_map_id map_id_config;

    int32_t id, i, j;

    sTIDL_IOBufDesc_t *ioBufDesc;

    vxMapUserDataObject(config, 0, sizeof(sTIDL_IOBufDesc_t), &map_id_config,
                      (void **)&ioBufDesc, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    for(id = 0; id < ioBufDesc->numOutputBuf; id++)
    {
        output_sizes[0] = ioBufDesc->outWidth[id]  + ioBufDesc->outPadL[id] + ioBufDesc->outPadR[id];
        output_sizes[1] = ioBufDesc->outHeight[id] + ioBufDesc->outPadT[id] + ioBufDesc->outPadB[id];
        output_sizes[2] = ioBufDesc->outNumChannels[id];

        status = vxGetStatus((vx_reference)output_tensors[id]);

        if (VX_SUCCESS == status)
        {
            void *output_buffer;

            vx_map_id map_id_output;

            vx_size output_strides[TEST_TIDL_MAX_TENSOR_DIMS];
            vx_size start[TEST_TIDL_MAX_TENSOR_DIMS];

            start[0] = start[1] = start[2] = start[3] = 0;

            output_strides[0] = 1;
            output_strides[1] = output_sizes[0];
            output_strides[2] = output_sizes[1] * output_strides[1];

            tivxMapTensorPatch(output_tensors[id], 3, start, output_sizes, &map_id_output, output_strides, &output_buffer, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

            {
              vx_uint8 *pOut;

                pOut = (vx_uint8 *)output_buffer + (ioBufDesc->outPadT[id] * output_sizes[0]) + ioBufDesc->outPadL[id];

                for(i = 0; i < 5; i++)
                {
                  score[i] = 0;
                  classid[i] = 0xFFFFFFFF;

                  for(j = 0; j < output_sizes[0]; j++)
                  {
                    if(pOut[j] > score[i])
                    {
                      score[i] = pOut[j];
                      classid[i] = j;
                    }
                  }

                  pOut[classid[i]] = 0;
                }

                printf("Image classification Top-5 results: \n");

                for(i = 0; i < 5; i++)
                {
                  printf(" class-id: %d, score: %u \n", classid[i], score[i]);
                }
            }

            tivxUnmapTensorPatch(output_tensors[id], map_id_output);
        }
    }

    vxUnmapUserDataObject(config, map_id_config);

    #if 1
    /* only checking classid */
    if(refid != classid[0])
        return VX_FAILURE;
    #endif

    return VX_SUCCESS;
}
