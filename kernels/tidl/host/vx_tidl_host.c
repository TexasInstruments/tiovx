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
#include "tivx_kernels_host_utils.h"
#ifndef _TI_DL_NEXT
#include "tivx_tidl_kernels.h"
#include "../../../common/xdais_types.h" /* In TIDL_PATH directory */
#include "sTIDL_IOBufDesc.h"
#endif
#include "itidl_ti.h"

static vx_status VX_CALLBACK tivxAddKernelTIDLValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_uint32 i;

    if (num_params < (TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + 2))
    {
        /* Number of parameters should be a minimum of TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + 2 */
        /* config, network, input dataQ_array, output dataQ_array, max_num_input_tensors, max_num_output_tensors, mininum 1-input, minimum 1-output */
        status = VX_FAILURE;
    }

      /* Verify that parameters[0[ and parameteres[1] and at least the first two tensors contain objects
       * There could be more tensors depending on the configuration specified but we are not checking here
       * as the information of the exact number of input and output tensors is not available here.
       * */
    for (i = 0U; i < (TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + 2); i ++)
    {
        /* Check for NULL */
        if (NULL == parameters[i])
        {
            status = VX_ERROR_NO_MEMORY;
            break;
        }
    }

    if (VX_SUCCESS == status)
    {
        vx_user_data_object config = NULL;
        vx_char config_name[VX_MAX_REFERENCE_NAME];
        vx_size config_size;

        config = (vx_user_data_object)parameters[TIVX_KERNEL_TIDL_IN_CONFIG_IDX];

        tivxCheckStatus(&status, vxQueryUserDataObject(config, VX_USER_DATA_OBJECT_NAME, &config_name, sizeof(config_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(config, VX_USER_DATA_OBJECT_SIZE, &config_size, sizeof(config_size)));

        if ((config_size != sizeof(sTIDL_IOBufDesc_t)) ||
            (strncmp(config_name, "sTIDL_IOBufDesc_t", sizeof(config_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'config' should be a user_data_object of type:\n sTIDL_IOBufDesc_t \n");
        }
    }

    if (VX_SUCCESS == status)
    {
        vx_user_data_object network = NULL;
        vx_char network_name[VX_MAX_REFERENCE_NAME];
        vx_size network_size;

        network = (vx_user_data_object)parameters[TIVX_KERNEL_TIDL_IN_NETWORK_IDX];

        tivxCheckStatus(&status, vxQueryUserDataObject(network, VX_USER_DATA_OBJECT_NAME, &network_name, sizeof(network_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(network, VX_USER_DATA_OBJECT_SIZE, &network_size, sizeof(network_size)));

        if ((network_size < 1) ||
            (strncmp(network_name, "TIDL_network", sizeof(network_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'network' should be a user_data_object of name:\n TIDL_network \n");
        }
    }

    return status;
}

vx_kernel tivxAddKernelTIDL(vx_context context,
                            uint32_t max_num_input_tensors,
                            uint32_t max_num_output_tensors)
{
    vx_kernel kernel;
    vx_status status;
    vx_enum kernel_id;
    uint32_t index;
    uint32_t i;

    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    if (status == VX_SUCCESS)
    {
        /* Number of parameters are config + network + input data Q array + output data Q array
         * + 2 scalars corresponding to max_num_input_tensors & max_num_output_tensors + input tensors + output tensors */
        uint32_t num_params = TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + max_num_input_tensors + max_num_output_tensors;

        kernel = vxAddUserKernel(
                                context,
                                TIVX_KERNEL_TIDL_NAME,
                                kernel_id,
                                NULL,
                                num_params,
                                tivxAddKernelTIDLValidate,
                                NULL,
                                NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    if ( status == VX_SUCCESS)
    {
        index = 0;

        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_INPUT,
                VX_TYPE_USER_DATA_OBJECT,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
          status = vxAddParameterToKernel(kernel,
              index,
              VX_INPUT,
              VX_TYPE_USER_DATA_OBJECT,
              VX_PARAMETER_STATE_REQUIRED
          );
          index++;
        }
        if ( status == VX_SUCCESS)
        {
          status = vxAddParameterToKernel(kernel,
              index,
              VX_INPUT,
              VX_TYPE_ARRAY,
              VX_PARAMETER_STATE_REQUIRED
          );
          index++;
        }
        if ( status == VX_SUCCESS)
        {
          status = vxAddParameterToKernel(kernel,
              index,
              VX_OUTPUT,
              VX_TYPE_ARRAY,
              VX_PARAMETER_STATE_REQUIRED
          );
          index++;
        }
        if ( status == VX_SUCCESS)
        {
          status = vxAddParameterToKernel(kernel,
              index,
              VX_INPUT,
              VX_TYPE_UINT32,
              VX_PARAMETER_STATE_REQUIRED
          );
          index++;
        }
        if ( status == VX_SUCCESS)
        {
          status = vxAddParameterToKernel(kernel,
              index,
              VX_INPUT,
              VX_TYPE_UINT32,
              VX_PARAMETER_STATE_REQUIRED
          );
          index++;
        }
        for(i = 0; i < max_num_input_tensors; i++)
        {
            if ( status == VX_SUCCESS)
            {
                status = vxAddParameterToKernel(kernel,
                    index,
                    VX_INPUT,
                    VX_TYPE_TENSOR,
                    VX_PARAMETER_STATE_REQUIRED
                    );
                index++;
            }
        }

        for(i = 0; i < max_num_output_tensors; i++)
        {
            if ( status == VX_SUCCESS)
            {
                status = vxAddParameterToKernel(kernel,
                    index,
                    VX_OUTPUT,
                    VX_TYPE_TENSOR,
                    VX_PARAMETER_STATE_REQUIRED
                    );
                index++;
            }
        }

        if ( status == VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP2);
#ifndef _TI_DL_NEXT
            tivxAddKernelTarget(kernel, TIVX_TARGET_EVE1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_EVE2);
            tivxAddKernelTarget(kernel, TIVX_TARGET_EVE3);
            tivxAddKernelTarget(kernel, TIVX_TARGET_EVE4);
#endif
        }

        if ( status == VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if( status != VX_SUCCESS)
        {
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
    }

    return kernel;
}
