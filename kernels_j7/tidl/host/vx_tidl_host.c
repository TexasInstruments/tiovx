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
#include <TI/tivx_config.h>
#include <TI/j7_tidl.h>
#include "tivx_kernels_host_utils.h"
#include <stdio.h>

static vx_status VX_CALLBACK tivxTIDLValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params,
            vx_meta_format metas[]);

static vx_status VX_CALLBACK tivxTIDLValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_uint32 i;

    if (num_params < TIVX_KERNEL_TIDL_NUM_MIN_PARAMETERS)
    {
        /* Number of parameters should be a minimum of 7 */
        /* config, network, createParams, inArgs, outArgs, mininum 1-input, minimum 1-output */
        status = (vx_status)VX_FAILURE;
    }

    for (i = 0U; i < num_params; i ++)
    {
        /* Check for NULL for all except 5th index as traceData is optional*/
        if ((i != TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX) && (NULL == parameters[i]))
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
            break;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        vx_user_data_object config = NULL;
        vx_char config_name[VX_MAX_REFERENCE_NAME];
        vx_size config_size;

        config = (vx_user_data_object)parameters[TIVX_KERNEL_TIDL_IN_CONFIG_IDX];

        tivxCheckStatus(&status, vxQueryUserDataObject(config, (vx_enum)VX_USER_DATA_OBJECT_NAME, &config_name, sizeof(config_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(config, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &config_size, sizeof(config_size)));

        if ((config_size != sizeof(tivxTIDLJ7Params)) ||
            (strncmp(config_name, "tivxTIDLJ7Params", sizeof(config_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'config' should be a user_data_object of type:\n tivxTIDLParms \n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        vx_user_data_object network = NULL;
        vx_char network_name[VX_MAX_REFERENCE_NAME];
        vx_size network_size;

        network = (vx_user_data_object)parameters[TIVX_KERNEL_TIDL_IN_NETWORK_IDX];

        tivxCheckStatus(&status, vxQueryUserDataObject(network, (vx_enum)VX_USER_DATA_OBJECT_NAME, &network_name, sizeof(network_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(network, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &network_size, sizeof(network_size)));

        if ((network_size < 1U) ||
            (strncmp(network_name, "TIDL_network", sizeof(network_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'network' should be a user_data_object of name:\n TIDL_network \n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        vx_user_data_object createParams = NULL;
        vx_char createParams_name[VX_MAX_REFERENCE_NAME];
        vx_size createParams_size;

        createParams = (vx_user_data_object)parameters[TIVX_KERNEL_TIDL_IN_CREATE_PARAMS_IDX];

        tivxCheckStatus(&status, vxQueryUserDataObject(createParams, (vx_enum)VX_USER_DATA_OBJECT_NAME, &createParams_name, sizeof(createParams_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(createParams, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &createParams_size, sizeof(createParams_size)));

        if ((createParams_size != sizeof(TIDL_CreateParams)) ||
            (strncmp(createParams_name, "TIDL_CreateParams", sizeof(createParams_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'createParams' should be a user_data_object of name:\n TIDL_CreateParams \n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        vx_user_data_object inArgs = NULL;
        vx_char inArgs_name[VX_MAX_REFERENCE_NAME];
        vx_size inArgs_size;

        inArgs = (vx_user_data_object)parameters[TIVX_KERNEL_TIDL_IN_IN_ARGS_IDX];

        tivxCheckStatus(&status, vxQueryUserDataObject(inArgs, (vx_enum)VX_USER_DATA_OBJECT_NAME, &inArgs_name, sizeof(inArgs_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(inArgs, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &inArgs_size, sizeof(inArgs_size)));

        if ((inArgs_size != sizeof(TIDL_InArgs)) ||
            (strncmp(inArgs_name, "TIDL_InArgs", sizeof(inArgs_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'inArgs' should be a user_data_object of name:\n TIDL_inArgs \n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        vx_user_data_object outArgs = NULL;
        vx_char outArgs_name[VX_MAX_REFERENCE_NAME];
        vx_size outArgs_size;

        outArgs = (vx_user_data_object)parameters[TIVX_KERNEL_TIDL_IN_OUT_ARGS_IDX];

        tivxCheckStatus(&status, vxQueryUserDataObject(outArgs, (vx_enum)VX_USER_DATA_OBJECT_NAME, &outArgs_name, sizeof(outArgs_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(outArgs, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &outArgs_size, sizeof(outArgs_size)));

        if ((outArgs_size != sizeof(TIDL_outArgs)) ||
            (strncmp(outArgs_name, "TIDL_outArgs", sizeof(outArgs_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'outArgs' should be a user_data_object of name:\n TIDL_outArgs \n");
        }
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        vx_user_data_object traceData = NULL;
        vx_char traceData_name[VX_MAX_REFERENCE_NAME];
        vx_size traceData_size;

        traceData = (vx_user_data_object)parameters[TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX];

        if(traceData != NULL)
        {
            tivxCheckStatus(&status, vxQueryUserDataObject(traceData, (vx_enum)VX_USER_DATA_OBJECT_NAME, &traceData_name, sizeof(traceData_name)));
            tivxCheckStatus(&status, vxQueryUserDataObject(traceData, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &traceData_size, sizeof(traceData_size)));

            if ((traceData_size < 1) ||
                (strncmp(traceData_name, "TIDL_traceData", sizeof(traceData_name)) != 0))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'traceData' should be a user_data_object of name:\n TIDL_traceData \n");
            }
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
    vx_char tidl_kernel_name[VX_MAX_KERNEL_NAME];

    /* Create kernel name by concatonating TIDL kernel name with number of input and output tensors to create a unique kernel */
    snprintf( tidl_kernel_name, VX_MAX_KERNEL_NAME, "%s:%d:%d", TIVX_KERNEL_TIDL_NAME, num_input_tensors, num_output_tensors );

    kernel = vxGetKernelByName(context, tidl_kernel_name);

    if ( NULL == kernel)
    {
        status = vxAllocateUserKernelId(context, &kernel_id);
        if(status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
        }

        if (status == (vx_status)VX_SUCCESS)
        {
            /* Number of parameters are config + network + createParams + inArgs + outArgs + traceData + input tensors + output tensors */
            uint32_t num_params = TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + num_input_tensors + num_output_tensors;

            if ( (num_params <= TIVX_KERNEL_MAX_PARAMS) &&
                 (num_input_tensors != 0U) &&
                 (num_output_tensors != 0U))
            {
                kernel = vxAddUserKernel(
                                    context,
                                    tidl_kernel_name,
                                    kernel_id,
                                    NULL,
                                    num_params,
                                    tivxTIDLValidate,
                                    NULL,
                                    NULL);

                status = vxGetStatus((vx_reference)kernel);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "invalid values for num_input_tensors or num_output_tensors \n");
                status = (vx_status)VX_FAILURE;
            }
        }
        if ( status == (vx_status)VX_SUCCESS)
        {
            index = 0;

            status = vxAddParameterToKernel(kernel,
                index,
                (vx_enum)VX_INPUT,
                VX_TYPE_USER_DATA_OBJECT,
                (vx_enum)VX_PARAMETER_STATE_REQUIRED
                );
            index++;

            if ( status == (vx_status)VX_SUCCESS)
            {
                status = vxAddParameterToKernel(kernel,
                    index,
                    (vx_enum)VX_INPUT,
                    VX_TYPE_USER_DATA_OBJECT,
                    (vx_enum)VX_PARAMETER_STATE_REQUIRED
                    );
                index++;
            }
            if ( status == (vx_status)VX_SUCCESS)
            {
                status = vxAddParameterToKernel(kernel,
                    index,
                    (vx_enum)VX_INPUT,
                    VX_TYPE_USER_DATA_OBJECT,
                    (vx_enum)VX_PARAMETER_STATE_REQUIRED
                    );
                index++;
            }
            if ( status == (vx_status)VX_SUCCESS)
            {
                status = vxAddParameterToKernel(kernel,
                    index,
                    (vx_enum)VX_INPUT,
                    VX_TYPE_USER_DATA_OBJECT,
                    (vx_enum)VX_PARAMETER_STATE_REQUIRED
                    );
                index++;
            }
            if ( status == (vx_status)VX_SUCCESS)
            {
                status = vxAddParameterToKernel(kernel,
                    index,
                    (vx_enum)VX_OUTPUT,
                    VX_TYPE_USER_DATA_OBJECT,
                    (vx_enum)VX_PARAMETER_STATE_REQUIRED
                    );
                index++;
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = vxAddParameterToKernel(kernel,
                    index,
                    (vx_enum)VX_OUTPUT,
                    VX_TYPE_USER_DATA_OBJECT,
                    (vx_enum)VX_PARAMETER_STATE_OPTIONAL
                    );
                index++;
            }
            if (status == VX_SUCCESS)
            {
                for(i = 0; i < num_input_tensors; i++)
                {
                    if ( status == (vx_status)VX_SUCCESS)
                    {
                        status = vxAddParameterToKernel(kernel,
                            index,
                            (vx_enum)VX_INPUT,
                            (vx_enum)VX_TYPE_TENSOR,
                            (vx_enum)VX_PARAMETER_STATE_REQUIRED
                            );
                        index++;
                    }
                }

                for(i = 0; i < num_output_tensors; i++)
                {
                    if ( status == (vx_status)VX_SUCCESS)
                    {
                        status = vxAddParameterToKernel(kernel,
                            index,
                            (vx_enum)VX_OUTPUT,
                            (vx_enum)VX_TYPE_TENSOR,
                            (vx_enum)VX_PARAMETER_STATE_REQUIRED
                            );
                        index++;
                    }
                }
            }

            if ( status == (vx_status)VX_SUCCESS)
            {
                /* add supported target's */
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_1);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_1_PRI_2);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_1_PRI_3);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_1_PRI_4);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_1_PRI_5);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_1_PRI_6);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_1_PRI_7);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_1_PRI_8);
                #if defined(SOC_J784S4)
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_2);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_2_PRI_2);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_2_PRI_3);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_2_PRI_4);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_2_PRI_5);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_2_PRI_6);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_2_PRI_7);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_2_PRI_8);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_3);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_3_PRI_2);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_3_PRI_3);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_3_PRI_4);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_3_PRI_5);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_3_PRI_6);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_3_PRI_7);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_3_PRI_8);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_4);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_4_PRI_2);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_4_PRI_3);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_4_PRI_4);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_4_PRI_5);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_4_PRI_6);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_4_PRI_7);
                tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_4_PRI_8);
                #endif
            }

            if ( status == (vx_status)VX_SUCCESS)
            {
                status = vxFinalizeKernel(kernel);
            }
            if( status != (vx_status)VX_SUCCESS)
            {
                vxReleaseKernel(&kernel);
                kernel = NULL;
            }
        }
        else
        {
            kernel = NULL;
        }
    }

    return kernel;
}
