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
#include <TI/j7_tidl.h>
#include <TI/tivx_mem.h>
#include "test_engine/test.h"
#include <float.h>
#include <math.h>
#include "itidl_ti.h"
#include "tivx_utils_tidl_trace.h"
#include <TI/tivx_task.h>
#include <inttypes.h>

#define DEBUG_TEST_TIDL

#if defined(SOC_J784S4)
#define NUM_TIDL_TARGETS 4U
#else
#define NUM_TIDL_TARGETS 1U
#endif

/*
 * This is the size of trace buffer allocated in host memory and
 * shared with target.
 */
#define TIVX_TIDL_TRACE_DATA_SIZE  (128 * 1024 * 1024)

TESTCASE(tivxTIDL, CT_VXContext, ct_setup_vx_context, 0)

#define TEST_TIDL_MAX_TENSOR_DIMS   (4u)

static vx_user_data_object readConfig(vx_context context, char *config_file, uint32_t *num_input_tensors, uint32_t *num_output_tensors)
{
    vx_status status = VX_SUCCESS;
    tivxTIDLJ7Params  *tidlParams;
    sTIDL_IOBufDesc_t *ioBufDesc = NULL;

    vx_user_data_object   config = NULL;
    vx_uint32 capacity;
    vx_map_id map_id;
    vx_size read_count;

    FILE *fp_config;

    #ifdef DEBUG_TEST_TIDL
    printf("Reading IO config file %s ...\n", config_file);
    #endif

    fp_config = fopen(config_file, "rb");

    if(fp_config == NULL)
    {
        printf("ERROR: Unable to open IO config file %s \n", config_file);

        return NULL;
    }

    fseek(fp_config, 0, SEEK_END);
    capacity = ftell(fp_config);
    fseek(fp_config, 0, SEEK_SET);

    if( capacity != sizeof(sTIDL_IOBufDesc_t) )
    {
        printf("ERROR: Config file size (%d bytes) does not match size of sTIDL_IOBufDesc_t (%d bytes)\n", capacity, (vx_uint32)sizeof(sTIDL_IOBufDesc_t));
        fclose(fp_config);
        return NULL;
    }

    /* Create a user struct type for handling config data*/
    config = vxCreateUserDataObject(context, "tivxTIDLJ7Params", sizeof(tivxTIDLJ7Params), NULL );

    status = vxGetStatus((vx_reference)config);

    if (VX_SUCCESS == status)
    {
        status = vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id,
                            (void **)&tidlParams, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if (VX_SUCCESS == status)
        {
            if(tidlParams == NULL)
            {
                printf("ERROR: Map of config object failed\n");
                fclose(fp_config);
                return NULL;
            }

            tivx_tidl_j7_params_init(tidlParams);

            ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;

            read_count = fread(ioBufDesc, capacity, 1, fp_config);
            if(read_count != 1)
            {
              printf("ERROR: Unable to read config file\n");
            }
            fclose(fp_config);

            *num_input_tensors  = ioBufDesc->numInputBuf;
            *num_output_tensors = ioBufDesc->numOutputBuf;

            vxUnmapUserDataObject(config, map_id);

            #ifdef DEBUG_TEST_TIDL
            printf("Finished reading IO config file of %d bytes, num_input_tensors = %d, num_output_tensors = %d\n", capacity, *num_input_tensors, *num_output_tensors);
            #endif
        }
        else
        {
            fclose(fp_config);
        }
    }
    else
    {
        fclose(fp_config);
    }

    return config;
}

static vx_user_data_object readNetwork(vx_context context, char *network_file)
{
    vx_status status;

    vx_user_data_object  network;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void      *network_buffer = NULL;
    vx_size read_count;

    FILE *fp_network;

    #ifdef DEBUG_TEST_TIDL
    printf("Reading network file %s ...\n", network_file);
    #endif

    fp_network = fopen(network_file, "rb");

    if(fp_network == NULL)
    {
        printf("ERROR: Unable to open network file %s \n", network_file);

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
            if(network_buffer) {
                read_count = fread(network_buffer, capacity, 1, fp_network);
                if(read_count != 1)
                {
                    printf("ERROR: Unable to read network file\n");
                }
            } else {
                printf("ERROR: Unable to allocate memory for reading network file of %d bytes\n", capacity);
            }

            vxUnmapUserDataObject(network, map_id);

            #ifdef DEBUG_TEST_TIDL
            printf("Finished reading network file of %d bytes\n", capacity);
            #endif
        }
    }

    fclose(fp_network);

    return network;
}

static vx_user_data_object setCreateParams(vx_context context, uint32_t read_raw_padded, uint32_t trace_write_flag)
{
    vx_status status;

    vx_user_data_object  createParams;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *createParams_buffer = NULL;

    capacity = sizeof(TIDL_CreateParams);
    createParams = vxCreateUserDataObject(context, "TIDL_CreateParams", capacity, NULL );

    status = vxGetStatus((vx_reference)createParams);

    if (VX_SUCCESS == status)
    {
        status = vxMapUserDataObject(createParams, 0, capacity, &map_id,
                        (void **)&createParams_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if (VX_SUCCESS == status)
        {
            if(createParams_buffer)
            {
              TIDL_CreateParams *prms = createParams_buffer;
              //write create params here
              TIDL_createParamsInit(prms);

              if(read_raw_padded)
                 prms->isInbufsPaded                 = 0;
              else
                 prms->isInbufsPaded                 = 1;

              prms->quantRangeExpansionFactor     = 1.0;
              prms->quantRangeUpdateFactor        = 0.0;

              /* Default is float max, so to test preemption logic, set to 0 ms to preempt for each layer */
              prms->maxPreEmptDelay               = 0.0f;

              if(trace_write_flag == 1)
              {
                prms->traceLogLevel                 = 1;
                prms->traceWriteLevel               = 1;
              }
              else
              {
                prms->traceLogLevel                 = 0;
                prms->traceWriteLevel               = 0;
              }
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

static vx_user_data_object setInArgs(vx_context context)
{
    vx_status status;

    vx_user_data_object  inArgs;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *inArgs_buffer = NULL;

    capacity = sizeof(TIDL_InArgs);
    inArgs = vxCreateUserDataObject(context, "TIDL_InArgs", capacity, NULL );

    status = vxGetStatus((vx_reference)inArgs);

    if (VX_SUCCESS == status)
    {
        status = vxMapUserDataObject(inArgs, 0, capacity, &map_id,
                        (void **)&inArgs_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if (VX_SUCCESS == status)
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

static vx_user_data_object setOutArgs(vx_context context)
{
    vx_status status;

    vx_user_data_object  outArgs;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *outArgs_buffer = NULL;

    capacity = sizeof(TIDL_outArgs);
    outArgs = vxCreateUserDataObject(context, "TIDL_outArgs", capacity, NULL );

    status = vxGetStatus((vx_reference)outArgs);

    if (VX_SUCCESS == status)
    {
        status = vxMapUserDataObject(outArgs, 0, capacity, &map_id,
                        (void **)&outArgs_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if (VX_SUCCESS == status)
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

static vx_tensor createInputTensor(vx_context context, vx_user_data_object config)
{
    vx_size   input_sizes[TEST_TIDL_MAX_TENSOR_DIMS];
    vx_map_id map_id_config;
    sTIDL_IOBufDesc_t *ioBufDesc;
    tivxTIDLJ7Params *tidlParams;

    vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                      (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
    input_sizes[0] = ioBufDesc->inWidth[0]  + ioBufDesc->inPadL[0] + ioBufDesc->inPadR[0];
    input_sizes[1] = ioBufDesc->inHeight[0] + ioBufDesc->inPadT[0] + ioBufDesc->inPadB[0];
    input_sizes[2] = ioBufDesc->inNumChannels[0];

    #ifdef DEBUG_TEST_TIDL
    printf(" input_sizes[0] = %d, dim = %d padL = %d padR = %d\n", (uint32_t)input_sizes[0], ioBufDesc->inWidth[0], ioBufDesc->inPadL[0], ioBufDesc->inPadR[0]);
    printf(" input_sizes[1] = %d, dim = %d padL = %d padR = %d\n", (uint32_t)input_sizes[1], ioBufDesc->inHeight[0], ioBufDesc->inPadT[0], ioBufDesc->inPadB[0]);
    printf(" input_sizes[2] = %d, dim = %d \n", (uint32_t)input_sizes[2], ioBufDesc->inNumChannels[0]);
    #endif

    vxUnmapUserDataObject(config, map_id_config);

    return vxCreateTensor(context, 3, input_sizes, VX_TYPE_UINT8, 0);
}

static vx_tensor createOutputTensor(vx_context context, vx_user_data_object config)
{
    vx_size    output_sizes[TEST_TIDL_MAX_TENSOR_DIMS];
    vx_map_id map_id_config;
    sTIDL_IOBufDesc_t *ioBufDesc;
    tivxTIDLJ7Params *tidlParams;

    vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                      (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
    output_sizes[0] = ioBufDesc->outWidth[0]  + ioBufDesc->outPadL[0] + ioBufDesc->outPadR[0];
    output_sizes[1] = ioBufDesc->outHeight[0] + ioBufDesc->outPadT[0] + ioBufDesc->outPadB[0];
    output_sizes[2] = ioBufDesc->outNumChannels[0];

    #ifdef DEBUG_TEST_TIDL
    printf(" output_sizes[0] = %d, dim = %d padL = %d padR = %d\n", (uint32_t)output_sizes[0], ioBufDesc->outWidth[0], ioBufDesc->outPadL[0], ioBufDesc->outPadR[0]);
    printf(" output_sizes[1] = %d, dim = %d padL = %d padR = %d\n", (uint32_t)output_sizes[1], ioBufDesc->outHeight[0], ioBufDesc->outPadT[0], ioBufDesc->outPadB[0]);
    printf(" output_sizes[2] = %d, dim = %d \n", (uint32_t)output_sizes[2], ioBufDesc->outNumChannels[0]);
    #endif

    vxUnmapUserDataObject(config, map_id_config);

    return vxCreateTensor(context, 3, output_sizes, VX_TYPE_FLOAT32, 0);
}

static vx_status readInput(vx_context context, vx_user_data_object config, vx_tensor *input_tensors, char *input_file)
{
    vx_status status = VX_SUCCESS;

    void      *input_buffer = NULL;
    int32_t    capacity;
    uint32_t   id;

    vx_map_id map_id_config;
    vx_map_id map_id_input;

    vx_size    start[TEST_TIDL_MAX_TENSOR_DIMS];
    vx_size    input_strides[TEST_TIDL_MAX_TENSOR_DIMS];
    vx_size    input_sizes[TEST_TIDL_MAX_TENSOR_DIMS];

    sTIDL_IOBufDesc_t *ioBufDesc;
    tivxTIDLJ7Params  *tidlParams;

    vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                      (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
    for(id = 0; id < ioBufDesc->numInputBuf; id++)
    {
        input_sizes[0] = ioBufDesc->inWidth[id]  + ioBufDesc->inPadL[id] + ioBufDesc->inPadR[id];
        input_sizes[1] = ioBufDesc->inHeight[id] + ioBufDesc->inPadT[id] + ioBufDesc->inPadB[id];
        input_sizes[2] = ioBufDesc->inNumChannels[id];

        capacity = input_sizes[0] * input_sizes[1] * input_sizes[2];

        start[0] = start[1] = start[2] = 0;

        input_strides[0] = 1;
        input_strides[1] = input_sizes[0];
        input_strides[2] = input_sizes[1] * input_strides[1];

        status = tivxMapTensorPatch(input_tensors[id], 3, start, input_sizes, &map_id_input, input_strides, &input_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

        if (VX_SUCCESS == status)
        {
            vx_df_image df_image;
            void *data_ptr = NULL;
            void *bmp_context;
            vx_uint32 img_stride;
            vx_int32 start_offset;
            vx_uint8 *pR;
            vx_uint8 *pG;
            vx_uint8 *pB;
            vx_uint8 *pData;
            vx_int32 i, j, k;
            CT_Image image;

            image = ct_read_image(input_file, -1);
            ASSERT_(return 0, image);

            img_stride = image->stride * 3;
            data_ptr = image->data.y;

            /* Reset the input buffer, this will take care of padding requirement for TIDL */
            memset(input_buffer, 0, capacity);

            start_offset = (0 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + ioBufDesc->inPadL[id];
            pB = (vx_uint8 *)input_buffer + start_offset;

            start_offset = (1 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + ioBufDesc->inPadL[id];
            pG = (vx_uint8 *)input_buffer + start_offset;

            start_offset = (2 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + ioBufDesc->inPadL[id];
            pR = (vx_uint8 *)input_buffer + start_offset;

            for(i = 0; i < ioBufDesc->inHeight[id]; i++)
            {
                pData = (vx_uint8 *)data_ptr + ((16 + i) * img_stride) + (16 * 3);

                for(j = 0; j < ioBufDesc->inWidth[id]; j++)
                {
                    pR[j] = *pData++;
                    pG[j] = *pData++;
                    pB[j] = *pData++;
                }
                pR += input_strides[1];
                pG += input_strides[1];
                pB += input_strides[1];
            }

            CT_FreeObject(image);

            tivxUnmapTensorPatch(input_tensors[id], map_id_input);
        }
    }

    vxUnmapUserDataObject(config, map_id_config);

    return status;
}

static vx_status readInputRawPadded(vx_context context, vx_user_data_object config, vx_tensor *input_tensors, char *input_file)
{
    vx_status status = VX_SUCCESS;

    void      *input_buffer = NULL;
    int32_t    capacity;
    uint32_t   id;
    char       filepath[MAXPATHLENGTH];

    vx_map_id map_id_config;
    vx_map_id map_id_input;

    vx_size    start[TEST_TIDL_MAX_TENSOR_DIMS];
    vx_size    input_strides[TEST_TIDL_MAX_TENSOR_DIMS];
    vx_size    input_sizes[TEST_TIDL_MAX_TENSOR_DIMS];

    tivxTIDLJ7Params *tidlParams;
    sTIDL_IOBufDesc_t *ioBufDesc;
    FILE *fp;
    size_t sz;

    sz = snprintf(filepath, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file);
    if(sz >= MAXPATHLENGTH)
    {
        return VX_FAILURE;
    }

    fp = fopen(filepath, "rb");

    if(fp == NULL)
    {
        printf("ERROR: Unable to open input file %s \n", input_file);

        return VX_FAILURE;
    }

    vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                      (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
    for(id = 0; id < ioBufDesc->numInputBuf; id++)
    {
        input_sizes[0] = ioBufDesc->inWidth[id]  + ioBufDesc->inPadL[id] + ioBufDesc->inPadR[id];
        input_sizes[1] = ioBufDesc->inHeight[id] + ioBufDesc->inPadT[id] + ioBufDesc->inPadB[id];
        input_sizes[2] = ioBufDesc->inNumChannels[id];

        capacity = input_sizes[0] * input_sizes[1] * input_sizes[2];

        start[0] = start[1] = start[2] = 0;

        input_strides[0] = 1;
        input_strides[1] = input_sizes[0];
        input_strides[2] = input_sizes[1] * input_strides[1];

        status = tivxMapTensorPatch(input_tensors[id], 3, start, input_sizes, &map_id_input, input_strides, &input_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

        if (VX_SUCCESS == status)
        {
            vx_size read_count;

            /* Reset the input buffer, this will take care of padding requirement for TIDL */
            memset(input_buffer, 0, capacity);

            read_count = fread(input_buffer, capacity, 1, fp);
            if(read_count != 1)
            {
                printf("ERROR: Unable to read input file\n");
            }

            tivxUnmapTensorPatch(input_tensors[id], map_id_input);
        }
    }

    vxUnmapUserDataObject(config, map_id_config);

    fclose(fp);

    return status;
}

static vx_status displayOutput(vx_user_data_object config, vx_tensor *output_tensors, vx_int32 refid)
{
    vx_status status = VX_SUCCESS;
    float score[5];
    vx_uint32 classid[5] = {0};

    vx_size output_sizes[TEST_TIDL_MAX_TENSOR_DIMS];

    vx_map_id map_id_config;

    int32_t id, i, j;

    tivxTIDLJ7Params *tidlParams;
    sTIDL_IOBufDesc_t *ioBufDesc;

    vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                      (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
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
                float *pOut;

                pOut = (float *)output_buffer + (ioBufDesc->outPadT[id] * output_sizes[0]) + ioBufDesc->outPadL[id];

                for(i = 0; i < 5; i++)
                {
                    score[i] = FLT_MIN;
                    classid[i] = (uint32_t)-1;

                    for(j = 0; j < ioBufDesc->outWidth[id]; j++)
                    {
                        if(pOut[j] > score[i])
                        {
                            score[i] = pOut[j];
                            classid[i] = j;
                        }
                    }

                    pOut[classid[i]] = FLT_MIN;
                }

                printf("Image classification Top-5 results: \n");

                for(i = 0; i < 5; i++)
                {
                    printf(" class-id: %d, score: %f \n", classid[i], score[i]);
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

typedef struct {
    const char* testName;
    const char* config;
    const char* network;
    uint32_t read_raw_padded;
    uint32_t trace_write_flag;
    char* target_string_1;
} Arg;

typedef struct {
    const char* testName;
    const char* config;
    const char* network;
    uint32_t read_raw_padded;
    uint32_t trace_write_flag;
    char* target_string_1;
    char* target_string_2;
    char* target_string_3;
    char* target_string_4;
} ArgMulti;

typedef struct {
    const char* testName;
    const char* config;
    const char* network;
    uint32_t read_raw_padded;
    uint32_t trace_write_flag;
    char* target_string_1;
    char* target_string_2;
} ArgPriority;

#if defined(SOC_J784S4)
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP_C7_1_PRI_1", __VA_ARGS__, TIVX_TARGET_DSP_C7_1_PRI_1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP_C7_2_PRI_1", __VA_ARGS__, TIVX_TARGET_DSP_C7_2_PRI_1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP_C7_3_PRI_1", __VA_ARGS__, TIVX_TARGET_DSP_C7_3_PRI_1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP_C7_4_PRI_1", __VA_ARGS__, TIVX_TARGET_DSP_C7_4_PRI_1))

#define ADD_SET_TARGET_PARAMETERS_MULTI(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/ALL_C7X_TARGETS", __VA_ARGS__, TIVX_TARGET_DSP_C7_1_PRI_1, TIVX_TARGET_DSP_C7_2_PRI_1, TIVX_TARGET_DSP_C7_3_PRI_1, TIVX_TARGET_DSP_C7_4_PRI_1))

#define ADD_SET_TARGET_PRIORITY_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP_C7_1_PRI_1/TIVX_TARGET_DSP_C7_1_PRI_2", __VA_ARGS__, TIVX_TARGET_DSP_C7_1_PRI_1, TIVX_TARGET_DSP_C7_1_PRI_2))
#else
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP_C7_1_PRI_1", __VA_ARGS__, TIVX_TARGET_DSP_C7_1_PRI_1))

#define ADD_SET_TARGET_PARAMETERS_MULTI(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP_C7_1_PRI_1", __VA_ARGS__, TIVX_TARGET_DSP_C7_1_PRI_1, NULL, NULL, NULL))

#define ADD_SET_TARGET_PRIORITY_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP_C7_1_PRI_1/TIVX_TARGET_DSP_C7_1_PRI_2", __VA_ARGS__, TIVX_TARGET_DSP_C7_1_PRI_1, TIVX_TARGET_DSP_C7_1_PRI_2))
#endif

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("mobilenetv1", ADD_SET_TARGET_PARAMETERS, ARG, "tidl_io_mobilenet_v1_1.bin", "tidl_net_mobilenet_v1.bin", 0, 0), \
    CT_GENERATE_PARAMETERS("mobilenetv1", ADD_SET_TARGET_PARAMETERS, ARG, "tidl_io_mobilenet_v1_1.bin", "tidl_net_mobilenet_v1.bin", 0, 1)

#ifdef SOC_AM62A
/* Enabling layer level traces results into insufficicient memory in DDR shared buffer hence disabling layer level traces*/
#define PARAMETERS_PRIORITY \
    CT_GENERATE_PARAMETERS("mobilenetv1", ADD_SET_TARGET_PRIORITY_PARAMETERS, ARG, "tidl_io_mobilenet_v1_1.bin", "tidl_net_mobilenet_v1.bin", 0, 0),
#else
#define PARAMETERS_PRIORITY \
    CT_GENERATE_PARAMETERS("mobilenetv1", ADD_SET_TARGET_PRIORITY_PARAMETERS, ARG, "tidl_io_mobilenet_v1_1.bin", "tidl_net_mobilenet_v1.bin", 0, 0), \
    CT_GENERATE_PARAMETERS("mobilenetv1", ADD_SET_TARGET_PRIORITY_PARAMETERS, ARG, "tidl_io_mobilenet_v1_1.bin", "tidl_net_mobilenet_v1.bin", 0, 1)
#endif

#define PARAMETERS_MULTI \
    CT_GENERATE_PARAMETERS("mobilenetv1", ADD_SET_TARGET_PARAMETERS_MULTI, ARG, "tidl_io_mobilenet_v1_1.bin", "tidl_net_mobilenet_v1.bin", 0, 0)

/* Note: when adding the tracebuffer test with the PARAMETERS_MULTI, there is insufficient memory for all 4 C7X's */

static void ct_teardown_tidl_kernels(void/*vx_context*/ **context_)
{
    vx_context context = (vx_context)*context_;
    if (context == NULL)
        return;

    if (CT_HasFailure())
    {
        tivxTIDLUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxTIDL, testTIDL, Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_kernel kernel = 0;

    vx_perf_t perf_node;
    vx_perf_t perf_graph;

    vx_user_data_object  config;
    vx_user_data_object  network;
    vx_user_data_object  createParams;
    vx_user_data_object  inArgs;
    vx_user_data_object  outArgs;
    vx_user_data_object  traceData;

    vx_tensor input_tensor[1];
    vx_tensor output_tensor[1];

    vx_int32    refid = 895;

    vx_size output_sizes[TEST_TIDL_MAX_TENSOR_DIMS];
    char filepath[MAXPATHLENGTH];
    size_t sz;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_1));

    {
        uint32_t num_input_tensors  = 0;
        uint32_t num_output_tensors = 0;

        tivxTIDLLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_tidl_kernels, CT_GC_OBJECT);

        sz = snprintf(filepath, MAXPATHLENGTH, "%s/tivx/tidl_models/%s", ct_get_test_file_path(), arg_->config);
        ASSERT(sz < MAXPATHLENGTH);

        ASSERT_VX_OBJECT(config = readConfig(context, &filepath[0], &num_input_tensors, &num_output_tensors), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        kernel = tivxAddKernelTIDL(context, num_input_tensors, num_output_tensors);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        sz = snprintf(filepath, MAXPATHLENGTH, "%s/tivx/tidl_models/%s", ct_get_test_file_path(), arg_->network);
        ASSERT(sz < MAXPATHLENGTH);

        ASSERT_VX_OBJECT(network = readNetwork(context, &filepath[0]), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(createParams = setCreateParams(context, arg_->read_raw_padded, arg_->trace_write_flag), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(inArgs = setInArgs(context), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(outArgs = setOutArgs(context), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(input_tensor[0] = createInputTensor(context, config), (enum vx_type_e)VX_TYPE_TENSOR);

        ASSERT_VX_OBJECT(output_tensor[0] = createOutputTensor(context, config), (enum vx_type_e)VX_TYPE_TENSOR);

        /* This is an optional parameter can be NULL as well. */
        if(arg_->trace_write_flag == 1)
        {
            ASSERT_VX_OBJECT(traceData = vxCreateUserDataObject(context, "TIDL_traceData", TIVX_TIDL_TRACE_DATA_SIZE, NULL ), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        }
        else
        {
            traceData = NULL;
        }

        vx_reference params[] = {
                (vx_reference)config,
                (vx_reference)network,
                (vx_reference)createParams,
                (vx_reference)inArgs,
                (vx_reference)outArgs,
                (vx_reference)traceData
        };

        ASSERT_VX_OBJECT(node = tivxTIDLNode(graph, kernel, params, input_tensor, output_tensor), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string_1));

        if(arg_->read_raw_padded)
        {
            sz = snprintf(filepath, MAXPATHLENGTH, "tivx/tidl_models/airshow_256x256.y");
            ASSERT(sz < MAXPATHLENGTH);

            #ifdef DEBUG_TEST_TIDL
            printf("Reading input file %s ...\n", filepath);
            #endif

            VX_CALL(readInputRawPadded(context, config, &input_tensor[0], &filepath[0]));
        }
        else
        {
            sz = snprintf(filepath, MAXPATHLENGTH, "tivx/tidl_models/airshow_256x256.bmp");
            ASSERT(sz < MAXPATHLENGTH);

            #ifdef DEBUG_TEST_TIDL
            printf("Reading input file %s ...\n", filepath);
            #endif

            VX_CALL(readInput(context, config, &input_tensor[0], &filepath[0]));
        }
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

        VX_CALL(displayOutput(config, &output_tensor[0], refid));

        if(arg_->trace_write_flag == 1)
        {
            sz = snprintf(filepath, MAXPATHLENGTH, "%s/output/airshow_256x256.", ct_get_test_file_path());
            ASSERT(sz < MAXPATHLENGTH);
            VX_CALL(tivx_utils_tidl_trace_write(traceData, filepath));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseUserDataObject(&config));
        VX_CALL(vxReleaseUserDataObject(&network));
        VX_CALL(vxReleaseUserDataObject(&createParams));
        VX_CALL(vxReleaseUserDataObject(&inArgs));
        VX_CALL(vxReleaseUserDataObject(&outArgs));

        if(arg_->trace_write_flag == 1)
        {
           VX_CALL(vxReleaseUserDataObject(&traceData));
        }

        VX_CALL(vxReleaseTensor(&input_tensor[0]));
        VX_CALL(vxReleaseTensor(&output_tensor[0]));

        ASSERT(config == 0);
        ASSERT(network == 0);
        ASSERT(createParams == 0);
        ASSERT(inArgs == 0);
        ASSERT(outArgs == 0);

        if(arg_->trace_write_flag == 1)
        {
            ASSERT(outArgs == 0);
        }

        ASSERT(input_tensor[0]  == 0);
        ASSERT(output_tensor[0] == 0);

        tivxTIDLUnLoadKernels(context);

        vxRemoveKernel(kernel);
    }
    tivx_clr_debug_zone(VX_ZONE_INFO);
}

TEST_WITH_ARG(tivxTIDL, testMultiTIDL, ArgMulti, PARAMETERS_MULTI)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node[NUM_TIDL_TARGETS] = {0};
    vx_kernel kernel = 0;

    vx_perf_t perf_node[NUM_TIDL_TARGETS] = {0};
    vx_perf_t perf_graph;

    vx_user_data_object  config[NUM_TIDL_TARGETS], sample_config;
    vx_user_data_object  network[NUM_TIDL_TARGETS];
    vx_user_data_object  createParams[NUM_TIDL_TARGETS];
    vx_user_data_object  inArgs[NUM_TIDL_TARGETS];
    vx_user_data_object  outArgs[NUM_TIDL_TARGETS];
    vx_user_data_object  traceData[NUM_TIDL_TARGETS];

    vx_tensor input_tensor[NUM_TIDL_TARGETS][1];
    vx_tensor output_tensor[NUM_TIDL_TARGETS][1];

    vx_int32    refid = 895, i;

    vx_size output_sizes[TEST_TIDL_MAX_TENSOR_DIMS];
    char filepath[MAXPATHLENGTH];
    size_t sz;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_1));
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_3));
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_4));

    {
        uint32_t num_input_tensors  = 0;
        uint32_t num_output_tensors = 0;

        tivxTIDLLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_tidl_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        sz = snprintf(filepath, MAXPATHLENGTH, "%s/tivx/tidl_models/%s", ct_get_test_file_path(), arg_->config);
        ASSERT(sz < MAXPATHLENGTH);

        ASSERT_VX_OBJECT(sample_config = readConfig(context, &filepath[0], &num_input_tensors, &num_output_tensors), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        kernel = tivxAddKernelTIDL(context, num_input_tensors, num_output_tensors);

        for (i = 0; i < NUM_TIDL_TARGETS; i++)
        {
            sz = snprintf(filepath, MAXPATHLENGTH, "%s/tivx/tidl_models/%s", ct_get_test_file_path(), arg_->config);
            ASSERT(sz < MAXPATHLENGTH);

            ASSERT_VX_OBJECT(config[i] = readConfig(context, &filepath[0], &num_input_tensors, &num_output_tensors), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            sz = snprintf(filepath, MAXPATHLENGTH, "%s/tivx/tidl_models/%s", ct_get_test_file_path(), arg_->network);
            ASSERT(sz < MAXPATHLENGTH);

            ASSERT_VX_OBJECT(network[i] = readNetwork(context, &filepath[0]), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            ASSERT_VX_OBJECT(createParams[i] = setCreateParams(context, arg_->read_raw_padded, arg_->trace_write_flag), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            ASSERT_VX_OBJECT(inArgs[i] = setInArgs(context), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            ASSERT_VX_OBJECT(outArgs[i] = setOutArgs(context), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            ASSERT_VX_OBJECT(input_tensor[i][0] = createInputTensor(context, config[i]), (enum vx_type_e)VX_TYPE_TENSOR);

            ASSERT_VX_OBJECT(output_tensor[i][0] = createOutputTensor(context, config[i]), (enum vx_type_e)VX_TYPE_TENSOR);

            /* This is an optional parameter can be NULL as well. */
            if(arg_->trace_write_flag == 1)
            {
                ASSERT_VX_OBJECT(traceData[i] = vxCreateUserDataObject(context, "TIDL_traceData", TIVX_TIDL_TRACE_DATA_SIZE, NULL ), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            }
            else
            {
                traceData[i] = NULL;
            }

            vx_reference params[] = {
                    (vx_reference)config[i],
                    (vx_reference)network[i],
                    (vx_reference)createParams[i],
                    (vx_reference)inArgs[i],
                    (vx_reference)outArgs[i],
                    (vx_reference)traceData[i]
            };

            ASSERT_VX_OBJECT(node[i] = tivxTIDLNode(graph, kernel, params, input_tensor[i], output_tensor[i]), VX_TYPE_NODE);

            if (i == 0)
            {
                VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_1));
            }
            else if (i == 1)
            {
                VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_2));
            }
            else if (i == 2)
            {
                VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_3));
            }
            else if (i == 3)
            {
                VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_4));
            }

            if(arg_->read_raw_padded)
            {
                sz = snprintf(filepath, MAXPATHLENGTH, "tivx/tidl_models/airshow_256x256.y");
                ASSERT(sz < MAXPATHLENGTH);

                #ifdef DEBUG_TEST_TIDL
                printf("Reading input file %s ...\n", filepath);
                #endif

                VX_CALL(readInputRawPadded(context, config[i], &input_tensor[i][0], &filepath[0]));
            }
            else
            {
                sz = snprintf(filepath, MAXPATHLENGTH, "tivx/tidl_models/airshow_256x256.bmp");
                ASSERT(sz < MAXPATHLENGTH);

                #ifdef DEBUG_TEST_TIDL
                printf("Reading input file %s ...\n", filepath);
                #endif

                VX_CALL(readInput(context, config[i], &input_tensor[i][0], &filepath[0]));
            }
        }

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

        for (i = 0; i < NUM_TIDL_TARGETS; i++)
        {
            VX_CALL(displayOutput(config[i], &output_tensor[i][0], refid));

            if(arg_->trace_write_flag == 1)
            {
                sz = snprintf(filepath, MAXPATHLENGTH, "%s/output/airshow_256x256.", ct_get_test_file_path());
                ASSERT(sz < MAXPATHLENGTH);
                VX_CALL(tivx_utils_tidl_trace_write(traceData[i], filepath));
            }
        }

        for (i = 0; i < NUM_TIDL_TARGETS; i++)
        {
            VX_CALL(vxReleaseNode(&node[i]));
        }
        VX_CALL(vxReleaseGraph(&graph));

        for (i = 0; i < NUM_TIDL_TARGETS; i++)
        {
            ASSERT(node[i] == 0);
        }
        ASSERT(graph == 0);

        VX_CALL(vxReleaseUserDataObject(&sample_config));

        for (i = 0; i < NUM_TIDL_TARGETS; i++)
        {
            VX_CALL(vxReleaseUserDataObject(&config[i]));
            VX_CALL(vxReleaseUserDataObject(&network[i]));
            VX_CALL(vxReleaseUserDataObject(&createParams[i]));
            VX_CALL(vxReleaseUserDataObject(&inArgs[i]));
            VX_CALL(vxReleaseUserDataObject(&outArgs[i]));

            if(arg_->trace_write_flag == 1)
            {
               VX_CALL(vxReleaseUserDataObject(&traceData[i]));
            }

            VX_CALL(vxReleaseTensor(&input_tensor[i][0]));
            VX_CALL(vxReleaseTensor(&output_tensor[i][0]));
        }

        for (i = 0; i < NUM_TIDL_TARGETS; i++)
        {
            ASSERT(config[i] == 0);
            ASSERT(network[i] == 0);
            ASSERT(createParams[i] == 0);
            ASSERT(inArgs[i] == 0);
            ASSERT(outArgs[i] == 0);

            if(arg_->trace_write_flag == 1)
            {
                ASSERT(outArgs[i] == 0);
            }

            ASSERT(input_tensor[i][0]  == 0);
            ASSERT(output_tensor[i][0] == 0);
        }

        tivxTIDLUnLoadKernels(context);

        vxRemoveKernel(kernel);
    }
    tivx_clr_debug_zone(VX_ZONE_INFO);
}

TEST_WITH_ARG(tivxTIDL, testTIDLPreempt, ArgPriority, PARAMETERS_PRIORITY)
{
    vx_context context = context_->vx_context_;
    vx_graph graph1 = 0, graph2 = 0;
    vx_node node1 = 0, node2 = 0;
    vx_kernel kernel = 0;

    vx_perf_t perf_node;
    vx_perf_t perf_graph;

    vx_user_data_object  config;
    vx_user_data_object  network;
    vx_user_data_object  createParams;
    vx_user_data_object  inArgs;
    vx_user_data_object  outArgs;
    vx_user_data_object  traceData1;
    vx_user_data_object  traceData2;

    vx_tensor input_tensor[1];
    vx_tensor output_tensor1[1];
    vx_tensor output_tensor2[1];

    vx_int32    refid = 895;

    vx_size output_sizes[TEST_TIDL_MAX_TENSOR_DIMS];
    char filepath[MAXPATHLENGTH];
    size_t sz;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_1));
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));

    {
        uint32_t num_input_tensors  = 0;
        uint32_t num_output_tensors = 0;

        tivxTIDLLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_tidl_kernels, CT_GC_OBJECT);

        sz = snprintf(filepath, MAXPATHLENGTH, "%s/tivx/tidl_models/%s", ct_get_test_file_path(), arg_->config);
        ASSERT(sz < MAXPATHLENGTH);

        ASSERT_VX_OBJECT(config = readConfig(context, &filepath[0], &num_input_tensors, &num_output_tensors), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        kernel = tivxAddKernelTIDL(context, num_input_tensors, num_output_tensors);

        ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

        sz = snprintf(filepath, MAXPATHLENGTH, "%s/tivx/tidl_models/%s", ct_get_test_file_path(), arg_->network);
        ASSERT(sz < MAXPATHLENGTH);

        ASSERT_VX_OBJECT(network = readNetwork(context, &filepath[0]), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(createParams = setCreateParams(context, arg_->read_raw_padded, arg_->trace_write_flag), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(inArgs = setInArgs(context), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(outArgs = setOutArgs(context), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(input_tensor[0] = createInputTensor(context, config), (enum vx_type_e)VX_TYPE_TENSOR);

        ASSERT_VX_OBJECT(output_tensor1[0] = createOutputTensor(context, config), (enum vx_type_e)VX_TYPE_TENSOR);
        ASSERT_VX_OBJECT(output_tensor2[0] = createOutputTensor(context, config), (enum vx_type_e)VX_TYPE_TENSOR);

        /* This is an optional parameter can be NULL as well. */
        if(arg_->trace_write_flag == 1)
        {
            ASSERT_VX_OBJECT(traceData1 = vxCreateUserDataObject(context, "TIDL_traceData", TIVX_TIDL_TRACE_DATA_SIZE, NULL ), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            ASSERT_VX_OBJECT(traceData2 = vxCreateUserDataObject(context, "TIDL_traceData", TIVX_TIDL_TRACE_DATA_SIZE, NULL ), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        }
        else
        {
            traceData1 = NULL;
            traceData2 = NULL;
        }

        vx_reference params1[] = {
                (vx_reference)config,
                (vx_reference)network,
                (vx_reference)createParams,
                (vx_reference)inArgs,
                (vx_reference)outArgs,
                (vx_reference)traceData1
        };

        vx_reference params2[] = {
                (vx_reference)config,
                (vx_reference)network,
                (vx_reference)createParams,
                (vx_reference)inArgs,
                (vx_reference)outArgs,
                (vx_reference)traceData2
        };

        ASSERT_VX_OBJECT(node1 = tivxTIDLNode(graph1, kernel, params1, input_tensor, output_tensor1), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node2 = tivxTIDLNode(graph2, kernel, params2, input_tensor, output_tensor2), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, arg_->target_string_2));
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, arg_->target_string_1));

        if(arg_->read_raw_padded)
        {
            sz = snprintf(filepath, MAXPATHLENGTH, "tivx/tidl_models/airshow_256x256.y");
            ASSERT(sz < MAXPATHLENGTH);

            #ifdef DEBUG_TEST_TIDL
            printf("Reading input file %s ...\n", filepath);
            #endif

            VX_CALL(readInputRawPadded(context, config, &input_tensor[0], &filepath[0]));
        }
        else
        {
            sz = snprintf(filepath, MAXPATHLENGTH, "tivx/tidl_models/airshow_256x256.bmp");
            ASSERT(sz < MAXPATHLENGTH);

            #ifdef DEBUG_TEST_TIDL
            printf("Reading input file %s ...\n", filepath);
            #endif

            VX_CALL(readInput(context, config, &input_tensor[0], &filepath[0]));
        }
        #ifdef DEBUG_TEST_TIDL
        printf("Verifying graph ...\n");
        #endif

        VX_CALL(vxVerifyGraph(graph1));
        VX_CALL(vxVerifyGraph(graph2));

        tivxLogRtTraceEnable(graph1);
        tivxLogRtTraceEnable(graph2);

        #ifdef DEBUG_TEST_TIDL
        printf("Running graph ...\n");
        #endif
        VX_CALL(vxScheduleGraph(graph1));
        tivxTaskWaitMsecs(1+(10*arg_->trace_write_flag));
        VX_CALL(vxScheduleGraph(graph2));

        VX_CALL(vxWaitGraph(graph1));
        VX_CALL(vxWaitGraph(graph2));

        #ifdef DEBUG_TEST_TIDL
        printf("Showing output ...\n");
        #endif

        sz = snprintf(filepath, MAXPATHLENGTH, "%s/output/tidl_preempt%d.bin", ct_get_test_file_path(), arg_->trace_write_flag);
        ASSERT(sz < MAXPATHLENGTH);

        tivxLogRtTraceExportToFile(filepath);
        tivxLogRtTraceDisable(graph1);
        tivxLogRtTraceDisable(graph2);

        VX_CALL(displayOutput(config, &output_tensor1[0], refid));
        VX_CALL(displayOutput(config, &output_tensor2[0], refid));

        if(arg_->trace_write_flag == 1)
        {
            sz = snprintf(filepath, MAXPATHLENGTH, "%s/output/airshow_256x256_2.", ct_get_test_file_path());
            ASSERT(sz < MAXPATHLENGTH);
            VX_CALL(tivx_utils_tidl_trace_write(traceData2, filepath));
            sz = snprintf(filepath, MAXPATHLENGTH, "%s/output/airshow_256x256_1.", ct_get_test_file_path());
            ASSERT(sz < MAXPATHLENGTH);
            VX_CALL(tivx_utils_tidl_trace_write(traceData1, filepath));
       }
       {
#if !defined(PC)
            vx_perf_t perf1, perf2;

            VX_CALL(vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf1, sizeof(perf1)));
            VX_CALL(vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf2, sizeof(perf2)));

            /* Since node 1 is running first, but at lower priority, it gets preempted by node2, which is higher priority.
             * Node 1 therefore should be about twice the execution time as node 2 + context switch time */
            printf("perf1.avg = %" PRIu64 "\n", perf1.avg);
            printf("perf2.avg = %" PRIu64 "\n", perf2.avg);

            /*The intention of this test [TIOVX-1137] is to ensure that preemption has happened, rather than imposing a time constraint/deadline
            * Ideally, node 1 takes "2T + context switch" cycles and node 2 takes "T" cycles. However since only a single frame is being run from each
            * node (for the same network), when node 1 gets control back, cache has already been warmed up and it's possible for it to
            * run marginally faster. The condition for this has been modified to use a factor of 1.5x to account for this fact, and is  
            * sufficient for [TIOVX-1137] (as it shows that N1 was running slower & got preempted by N2) */
            ASSERT(perf1.avg > ((perf2.avg*3)/2));
#endif
       }

        VX_CALL(vxReleaseNode(&node1));
        VX_CALL(vxReleaseNode(&node2));
        VX_CALL(vxReleaseGraph(&graph1));
        VX_CALL(vxReleaseGraph(&graph2));

        ASSERT(node1 == 0);
        ASSERT(node2 == 0);
        ASSERT(graph1 == 0);
        ASSERT(graph2 == 0);

        VX_CALL(vxReleaseUserDataObject(&config));
        VX_CALL(vxReleaseUserDataObject(&network));
        VX_CALL(vxReleaseUserDataObject(&createParams));
        VX_CALL(vxReleaseUserDataObject(&inArgs));
        VX_CALL(vxReleaseUserDataObject(&outArgs));

        if(arg_->trace_write_flag == 1)
        {
           VX_CALL(vxReleaseUserDataObject(&traceData1));
           VX_CALL(vxReleaseUserDataObject(&traceData2));
        }

        VX_CALL(vxReleaseTensor(&input_tensor[0]));
        VX_CALL(vxReleaseTensor(&output_tensor1[0]));
        VX_CALL(vxReleaseTensor(&output_tensor2[0]));

        ASSERT(config == 0);
        ASSERT(network == 0);
        ASSERT(createParams == 0);
        ASSERT(inArgs == 0);
        ASSERT(outArgs == 0);

        if(arg_->trace_write_flag == 1)
        {
            ASSERT(outArgs == 0);
        }

        ASSERT(input_tensor[0]  == 0);
        ASSERT(output_tensor1[0] == 0);
        ASSERT(output_tensor2[0] == 0);

        tivxTIDLUnLoadKernels(context);

        vxRemoveKernel(kernel);
    }
    tivx_clr_debug_zone(VX_ZONE_INFO);
}


TESTCASE_TESTS(tivxTIDL,
    testTIDL,
    #if defined(SOC_J784S4)
    testMultiTIDL,
    testTIDLPreempt
    #else
    testTIDLPreempt,
    DISABLED_testMultiTIDL
    #endif
    )
