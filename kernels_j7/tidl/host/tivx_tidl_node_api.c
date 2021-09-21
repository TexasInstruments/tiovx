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


#include <stdlib.h>
#include <TI/tivx.h>
#include <TI/tivx_config.h>
#include <TI/j7_tidl.h>

VX_API_ENTRY vx_node VX_API_CALL tivxTIDLNode(vx_graph  graph,
                                              vx_kernel kernel,
                                              vx_reference appParams[],
                                              vx_tensor input_tensors[],
                                              vx_tensor output_tensors[])
{
    int32_t i;
    vx_reference params[TIVX_KERNEL_MAX_PARAMS];
    vx_uint32 num_input_tensors, num_output_tensors;
    vx_map_id map_id_config;
    tivxTIDLJ7Params *tidlParams;
    sTIDL_IOBufDesc_t *ioBufDesc;
    vx_int32 num_params;
    vx_user_data_object config;
    vx_node node = NULL;

    config= (vx_user_data_object)appParams[TIVX_KERNEL_TIDL_IN_CONFIG_IDX];
    vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
        (void **)&tidlParams, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;

    num_input_tensors  = (uint32_t)ioBufDesc->numInputBuf;
    num_output_tensors = (uint32_t)ioBufDesc->numOutputBuf;

    num_params= (int32_t)TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + (int32_t)num_input_tensors + (int32_t)num_output_tensors;

    vxUnmapUserDataObject(config, map_id_config);

    if(num_params > (int32_t)TIVX_KERNEL_MAX_PARAMS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Exceeded max parameters for a kernel\n");
    }
    else
    {
        params[0]=  (vx_reference)config;
        params[1]=  appParams[TIVX_KERNEL_TIDL_IN_NETWORK_IDX];
        params[2]=  appParams[TIVX_KERNEL_TIDL_IN_CREATE_PARAMS_IDX];
        params[3]=  appParams[TIVX_KERNEL_TIDL_IN_IN_ARGS_IDX];
        params[4]=  appParams[TIVX_KERNEL_TIDL_IN_OUT_ARGS_IDX];
        params[5]=  appParams[TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX];

        for (i= 0; i < (int32_t)num_input_tensors; i++) {
          params[(int32_t)TIVX_KERNEL_TIDL_IN_FIRST_TENSOR + i]=  (vx_reference)input_tensors[i];
        }

        for (i= 0; i < (int32_t)num_output_tensors; i++) {
          params[(int32_t)TIVX_KERNEL_TIDL_IN_FIRST_TENSOR + (int32_t)num_input_tensors + i]=  (vx_reference)output_tensors[i];
        }

        node = tivxCreateNodeByKernelRef(graph,
                                         kernel,
                                         params,
                                         (uint32_t)num_params);
    }

    return node;
}
