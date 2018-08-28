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
#include "tivx_kernels_host_utils.h"

static vx_status VX_CALLBACK tivxAddKernelTIDLValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_uint32 i;

    if (num_params < 4)
    {
        /* Number of parameters should be a minimum of 4 */
        /* config, network, mininum 1-input, minimum 1-output */
        status = VX_FAILURE;
    }

    for (i = 0U; i < num_params; i ++)
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
        vx_array config = NULL;
        vx_enum config_item_type;
        vx_size config_capacity, config_item_size;

        config = (const vx_array)parameters[0];

        tivxCheckStatus(&status, vxQueryArray(config, VX_ARRAY_ITEMTYPE, &config_item_type, sizeof(config_item_type)));
        tivxCheckStatus(&status, vxQueryArray(config, VX_ARRAY_CAPACITY, &config_capacity, sizeof(config_capacity)));
        tivxCheckStatus(&status, vxQueryArray(config, VX_ARRAY_ITEMSIZE, &config_item_size, sizeof(config_item_size)));
    }

    if (VX_SUCCESS == status)
    {
        vx_tensor network = NULL;
        vx_enum network_data_type;
        vx_size network_dims;

        network = (const vx_tensor)parameters[1];

        tivxCheckStatus(&status, vxQueryTensor(network, VX_TENSOR_DATA_TYPE, &network_data_type, sizeof(network_data_type)));
        tivxCheckStatus(&status, vxQueryTensor(network, VX_TENSOR_NUMBER_OF_DIMS, &network_dims, sizeof(network_dims)));

        if (network_data_type != VX_TYPE_UINT8)
        {
            status = VX_FAILURE;
        }

        if (network_dims != 1)
        {
            status = VX_FAILURE;
        }

    }

    return status;
}

vx_kernel tivxAddKernelTIDL(vx_context context,
                            uint32_t num_input_tensors,
                            uint32_t num_output_tensors)
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
        /* Number of parameters are config + network + input tensors + output tensors */
        uint32_t num_params = 2 + num_input_tensors + num_output_tensors;

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
                VX_TYPE_TENSOR,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }

        for(i = 0; i < num_input_tensors; i++)
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

        for(i = 0; i < num_output_tensors; i++)
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
