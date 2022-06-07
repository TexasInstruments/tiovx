/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
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
#include <TI/tivx_test_kernels.h>
#include <tivx_test_kernels_kernels.h>
#include <tivx_kernel_cmd_timeout_test.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_cmd_timeout_test_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelCmdTimeoutTestValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelCmdTimeoutTestInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelCmdTimeoutTest(vx_context context);
vx_status tivxRemoveKernelCmdTimeoutTest(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelCmdTimeoutTestValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_user_data_object configuration = NULL;
    vx_char configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size configuration_size;

    vx_scalar in = NULL;
    vx_enum in_scalar_type;

    vx_scalar out = NULL;
    vx_enum out_scalar_type;

    if ( (num != TIVX_KERNEL_CMD_TIMEOUT_TEST_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_CMD_TIMEOUT_TEST_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_CMD_TIMEOUT_TEST_IN_IDX])
        || (NULL == parameters[TIVX_KERNEL_CMD_TIMEOUT_TEST_OUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_CMD_TIMEOUT_TEST_CONFIGURATION_IDX];
        in = (vx_scalar)parameters[TIVX_KERNEL_CMD_TIMEOUT_TEST_IN_IDX];
        out = (vx_scalar)parameters[TIVX_KERNEL_CMD_TIMEOUT_TEST_OUT_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));

        tivxCheckStatus(&status, vxQueryScalar(in, VX_SCALAR_TYPE, &in_scalar_type, sizeof(in_scalar_type)));

        tivxCheckStatus(&status, vxQueryScalar(out, VX_SCALAR_TYPE, &out_scalar_type, sizeof(out_scalar_type)));
    }

    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((configuration_size != sizeof(tivx_cmd_timeout_params_t)) ||
            (strncmp(configuration_name, "tivx_cmd_timeout_params_t", sizeof(configuration_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be a user_data_object of type:\n tivx_cmd_timeout_params_t \n");
        }

        if (VX_TYPE_UINT8 != in_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in' should be a scalar of type:\n VX_SCALAR_TYPE \n");
        }

        if (VX_TYPE_UINT8 != out_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'out' should be a scalar of type:\n VX_SCALAR_TYPE \n");
        }
    }


    /* CUSTOM PARAMETER CHECKING */

    /* < DEVELOPER_TODO: (Optional) Add any custom parameter type or range checking not */
    /*                   covered by the code-generation script.) > */

    /* < DEVELOPER_TODO: (Optional) If intending to use a virtual data object, set metas using appropriate TI API. */
    /*                   For a code example, please refer to the validate callback of the follow file: */
    /*                   tiovx/kernels/openvx-core/host/vx_absdiff_host.c. For further information regarding metas, */
    /*                   please refer to the OpenVX 1.1 spec p. 260, or search for vx_kernel_validate_f. > */

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelCmdTimeoutTestInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_CMD_TIMEOUT_TEST_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_CMD_TIMEOUT_TEST_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_CMD_TIMEOUT_TEST_IN_IDX])
        || (NULL == parameters[TIVX_KERNEL_CMD_TIMEOUT_TEST_OUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    return status;
}

vx_status tivxAddKernelCmdTimeoutTest(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_CMD_TIMEOUT_TEST_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_CMD_TIMEOUT_TEST_MAX_PARAMS,
                    tivxAddKernelCmdTimeoutTestValidate,
                    tivxAddKernelCmdTimeoutTestInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        index = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_SCALAR,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_SCALAR,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_A72_0);
            #if defined(SOC_AM62A)
            tivxAddKernelTarget(kernel, TIVX_TARGET_MCU1_0);
            #else
            tivxAddKernelTarget(kernel, TIVX_TARGET_MCU2_0);
            tivxAddKernelTarget(kernel, TIVX_TARGET_MCU2_1);
            #endif
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if (status != (vx_status)VX_SUCCESS)
        {
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
    }
    vx_cmd_timeout_test_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelCmdTimeoutTest(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_cmd_timeout_test_kernel;

    status = vxRemoveKernel(kernel);
    vx_cmd_timeout_test_kernel = NULL;

    return status;
}


