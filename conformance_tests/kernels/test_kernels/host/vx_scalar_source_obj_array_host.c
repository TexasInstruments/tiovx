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

#include "TI/tivx.h"
#include "TI/tivx_test_kernels.h"
#include "tivx_test_kernels_kernels.h"
#include "tivx_kernel_scalar_source_obj_array.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_scalar_source_obj_array_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelScalarSourceObjArrayValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelScalarSourceObjArrayInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelScalarSourceObjArray(vx_context context);
vx_status tivxRemoveKernelScalarSourceObjArray(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelScalarSourceObjArrayValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    if ( (num != TIVX_KERNEL_SCALAR_SOURCE_OBJ_ARRAY_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_SCALAR_SOURCE_OBJ_ARRAY_OUT_OBJECT_ARRAY_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelScalarSourceObjArrayInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_SCALAR_SOURCE_OBJ_ARRAY_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_SCALAR_SOURCE_OBJ_ARRAY_OUT_OBJECT_ARRAY_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    return status;
}

vx_status tivxAddKernelScalarSourceObjArray(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    if (status == VX_SUCCESS)
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_SCALAR_SOURCE_OBJ_ARRAY_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_SCALAR_SOURCE_OBJ_ARRAY_MAX_PARAMS,
                    tivxAddKernelScalarSourceObjArrayValidate,
                    tivxAddKernelScalarSourceObjArrayInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == VX_SUCCESS)
    {
        index = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_OBJECT_ARRAY,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            /* add supported target's */
            #if defined(SOC_AM62A)
            tivxAddKernelTarget(kernel, TIVX_TARGET_MCU1_0);
            #else
            tivxAddKernelTarget(kernel, TIVX_TARGET_MCU2_0);
            tivxAddKernelTarget(kernel, TIVX_TARGET_MCU2_1);
            #endif
        }
        if (status == VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if (status != VX_SUCCESS)
        {
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
    }
    vx_scalar_source_obj_array_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelScalarSourceObjArray(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_scalar_source_obj_array_kernel;

    status = vxRemoveKernel(kernel);
    vx_scalar_source_obj_array_kernel = NULL;

    return status;
}


