/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
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

#include "TI/tivx.h"
#include "TI/tivx_test_kernels.h"
#include "tivx_test_kernels_kernels.h"
#include "tivx_kernel_multi_in_out.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_multi_in_out_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelMultiInOutValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelMultiInOutInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelMultiInOut(vx_context context);
vx_status tivxRemoveKernelMultiInOut(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelMultiInOutValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_scalar in1_scalar = NULL;
    vx_enum in1_scalar_scalar_type;

    vx_scalar in2_scalar = NULL;
    vx_enum in2_scalar_scalar_type;

    vx_scalar out1_scalar = NULL;
    vx_enum out1_scalar_scalar_type;

    vx_scalar out2_scalar = NULL;
    vx_enum out2_scalar_scalar_type;

    if ( (num != TIVX_KERNEL_MULTI_IN_OUT_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_MULTI_IN_OUT_IN1_SCALAR_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTI_IN_OUT_IN2_SCALAR_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTI_IN_OUT_OUT1_SCALAR_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTI_IN_OUT_OUT2_SCALAR_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        in1_scalar = (vx_scalar)parameters[TIVX_KERNEL_MULTI_IN_OUT_IN1_SCALAR_IDX];
        in2_scalar = (vx_scalar)parameters[TIVX_KERNEL_MULTI_IN_OUT_IN2_SCALAR_IDX];
        out1_scalar = (vx_scalar)parameters[TIVX_KERNEL_MULTI_IN_OUT_OUT1_SCALAR_IDX];
        out2_scalar = (vx_scalar)parameters[TIVX_KERNEL_MULTI_IN_OUT_OUT2_SCALAR_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryScalar(in1_scalar, (vx_enum)VX_SCALAR_TYPE, &in1_scalar_scalar_type, sizeof(in1_scalar_scalar_type)));

        tivxCheckStatus(&status, vxQueryScalar(in2_scalar, (vx_enum)VX_SCALAR_TYPE, &in2_scalar_scalar_type, sizeof(in2_scalar_scalar_type)));

        tivxCheckStatus(&status, vxQueryScalar(out1_scalar, (vx_enum)VX_SCALAR_TYPE, &out1_scalar_scalar_type, sizeof(out1_scalar_scalar_type)));

        tivxCheckStatus(&status, vxQueryScalar(out2_scalar, (vx_enum)VX_SCALAR_TYPE, &out2_scalar_scalar_type, sizeof(out2_scalar_scalar_type)));
    }

    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_enum)VX_TYPE_UINT8 != in1_scalar_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in1_scalar' should be a scalar of type:\n VX_TYPE_UINT8 \n");
        }

        if ((vx_enum)VX_TYPE_UINT8 != in2_scalar_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in2_scalar' should be a scalar of type:\n VX_TYPE_UINT8 \n");
        }

        if ((vx_enum)VX_TYPE_UINT8 != out1_scalar_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'out1_scalar' should be a scalar of type:\n VX_TYPE_UINT8 \n");
        }

        if ((vx_enum)VX_TYPE_UINT8 != out2_scalar_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'out2_scalar' should be a scalar of type:\n VX_TYPE_UINT8 \n");
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelMultiInOutInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_MULTI_IN_OUT_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_MULTI_IN_OUT_IN1_SCALAR_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTI_IN_OUT_IN2_SCALAR_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTI_IN_OUT_OUT1_SCALAR_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTI_IN_OUT_OUT2_SCALAR_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    return status;
}

vx_status tivxAddKernelMultiInOut(vx_context context)
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
                    TIVX_KERNEL_MULTI_IN_OUT_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_MULTI_IN_OUT_MAX_PARAMS,
                    tivxAddKernelMultiInOutValidate,
                    tivxAddKernelMultiInOutInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        index = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP1);
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
    vx_multi_in_out_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelMultiInOut(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_multi_in_out_kernel;

    status = vxRemoveKernel(kernel);
    vx_multi_in_out_kernel = NULL;

    return status;
}


