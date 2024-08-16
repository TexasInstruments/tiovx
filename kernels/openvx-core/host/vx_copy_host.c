/*
 * Copyright (c) 2012-2024 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
Implementation file for the COPY, SWAP, and MOVE kernels
*/

#include <TI/tivx.h>
#include <VX/vx.h>
#include <vx_internal.h>
#include <VX/vx_khr_swap_move.h>
#include <tivx_core_host_priv.h>

#define VX_KERNEL_COPY_MAX_PARAMS (2U)
#define VX_KERNEL_COPY_INPUT_IDX (0U)
#define VX_KERNEL_COPY_OUTPUT_IDX (1U)
#define VX_KERNEL_SWAP_MAX_PARAMS (2U)
#define VX_KERNEL_SWAP_FIRST_IDX (0U)
#define VX_KERNEL_SWAP_SECOND_IDX (1U)
#define VX_KERNEL_MOVE_MAX_PARAMS (2U)
#define VX_KERNEL_MOVE_FIRST_IDX (0U)
#define VX_KERNEL_MOVE_SECOND_IDX (1U)

static vx_kernel vx_copy_kernel = NULL;
static vx_kernel vx_swap_kernel = NULL;
static vx_kernel vx_move_kernel = NULL;

static vx_status VX_CALLBACK vxAddKernelCopySwapMoveValidate(vx_node node,
            const vx_reference parameters[],
            vx_uint32 num,
            vx_meta_format metas[]);

static vx_status VX_CALLBACK vxAddKernelCopySwapMoveInitialize(vx_node node,
            const vx_reference parameters[],
            vx_uint32 num_params);

static vx_status VX_CALLBACK vxKernelCopySwapMoveProcess(vx_node node,
            const vx_reference parameters[],
            vx_uint32 num);

static inline vx_status call_kernel_func(vx_enum kernel_enum, vx_bool validate_only, const vx_reference params[])
{
    vx_status status = (vx_status)VX_SUCCESS;
    if (NULL != params[0]->kernel_callback)
    {
        status = (params[0]->kernel_callback)(kernel_enum, validate_only, params[0], params[1]);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Not supported\n");
        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
    }
    return status;
}

static vx_status VX_CALLBACK vxAddKernelCopySwapMoveValidate(vx_node node,
            const vx_reference parameters[],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num != VX_KERNEL_COPY_MAX_PARAMS)
        || (NULL == parameters[VX_KERNEL_COPY_INPUT_IDX])
        || (NULL == parameters[VX_KERNEL_COPY_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = call_kernel_func(node->kernel->enumeration, (vx_bool)vx_true_e, parameters);
        if (((vx_status)VX_SUCCESS == status) && 
            (NULL != parameters[0]->supplementary_data) && 
            (NULL != parameters[1]->supplementary_data))
        {
            vx_reference supp_params[2] = {&parameters[0]->supplementary_data->base, &parameters[1]->supplementary_data->base};
            status = call_kernel_func(node->kernel->enumeration, (vx_bool)vx_true_e, supp_params);
        }
        if ((vx_status)VX_SUCCESS == status)
        {
            if ((vx_bool)vx_true_e == tivxIsReferenceVirtual(parameters[VX_KERNEL_COPY_OUTPUT_IDX]))
            {
                status = vxSetMetaFormatFromReference(metas[VX_KERNEL_COPY_OUTPUT_IDX], parameters[VX_KERNEL_COPY_INPUT_IDX]);
            }
        }
        else
        {
            if ((vx_status)VX_ERROR_NOT_SUPPORTED == status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Copy or swap not supported for requested type\n");
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Cannot copy, swap or move objects of differing types or attributes\n");
            }
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxAddKernelCopySwapMoveInitialize(vx_node node,
            const vx_reference parameters[],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if ( (num_params != VX_KERNEL_COPY_MAX_PARAMS)
        || (NULL == parameters[VX_KERNEL_COPY_INPUT_IDX])
        || (NULL == parameters[VX_KERNEL_COPY_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    return status;
}

static vx_status VX_CALLBACK vxKernelCopySwapMoveProcess(vx_node node,
            const vx_reference parameters[],
            vx_uint32 num)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if ((VX_KERNEL_COPY_MAX_PARAMS != num) ||
        ((vx_status)VX_SUCCESS != vxGetStatus(parameters[VX_KERNEL_COPY_INPUT_IDX])) ||
        ((vx_status)VX_SUCCESS != vxGetStatus(parameters[VX_KERNEL_COPY_OUTPUT_IDX])))
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        status = call_kernel_func(node->kernel->enumeration, (vx_bool)vx_false_e, parameters);
        if (((vx_status)VX_SUCCESS == status) && (NULL != parameters[0]->supplementary_data) && (NULL != parameters[1]->supplementary_data))
        {
            vx_reference supp_params[2] = {&parameters[0]->supplementary_data->base, &parameters[1]->supplementary_data->base};
            status = call_kernel_func(node->kernel->enumeration, (vx_bool)vx_false_e, supp_params);
        }
    }
    return status;
}

vx_status tivxAddKernelCopy(vx_context context)
{
    vx_kernel kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.copy",
                (vx_enum)VX_KERNEL_COPY,
                vxKernelCopySwapMoveProcess,
                VX_KERNEL_COPY_MAX_PARAMS,
                vxAddKernelCopySwapMoveValidate,
                vxAddKernelCopySwapMoveInitialize,
                NULL);
    vx_status status = vxGetStatus((vx_reference)kernel);
    if ((vx_status)VX_SUCCESS == status)
     {
        status = vxAddParameterToKernel(kernel,
                        VX_KERNEL_COPY_INPUT_IDX,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_REFERENCE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED);
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        status = vxAddParameterToKernel(kernel,
                        VX_KERNEL_COPY_OUTPUT_IDX,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_REFERENCE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED);
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        status = vxFinalizeKernel(kernel);
    }
    if (status != (vx_status)VX_SUCCESS)
    {
        status = vxReleaseKernel(&kernel);
        vx_copy_kernel = NULL;
    }
    else
    {
        vx_copy_kernel = kernel;
    }
    return status;
}

vx_status tivxRemoveKernelCopy(vx_context context)
{
    vx_status status;
    status = vxRemoveKernel(vx_copy_kernel);
    vx_copy_kernel = NULL;
    return status;
}

vx_status tivxAddKernelSwap(vx_context context)
{
    vx_kernel kernel = vxAddUserKernel(
                context,
                VX_KERNEL_SWAP_NAME,
                (vx_enum)VX_KERNEL_SWAP,
                vxKernelCopySwapMoveProcess,
                VX_KERNEL_SWAP_MAX_PARAMS,
                vxAddKernelCopySwapMoveValidate,
                vxAddKernelCopySwapMoveInitialize,
                NULL);
    vx_status status = vxGetStatus((vx_reference)kernel);
    if ((vx_status)VX_SUCCESS == status)
    {
        status = vxAddParameterToKernel(kernel,
                        VX_KERNEL_SWAP_FIRST_IDX,
                        (vx_enum)VX_BIDIRECTIONAL,
                        (vx_enum)VX_TYPE_REFERENCE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED);
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        status = vxAddParameterToKernel(kernel,
                        VX_KERNEL_SWAP_SECOND_IDX,
                        (vx_enum)VX_BIDIRECTIONAL,
                        (vx_enum)VX_TYPE_REFERENCE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED);
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        status = vxFinalizeKernel(kernel);
    }
    if (status != (vx_status)VX_SUCCESS)
    {
        status = vxReleaseKernel(&kernel);
        vx_swap_kernel = NULL;
    }
    else
    {
        vx_swap_kernel = kernel;
    }
    return status;
}

vx_status tivxRemoveKernelSwap(vx_context context)
{
    vx_status status;

    status = vxRemoveKernel(vx_swap_kernel);
    vx_swap_kernel = NULL;

    return status;
}

vx_status tivxAddKernelMove(vx_context context)
{
    vx_kernel kernel = vxAddUserKernel(
                context,
                VX_KERNEL_MOVE_NAME,
                (vx_enum)VX_KERNEL_MOVE,
                vxKernelCopySwapMoveProcess,
                VX_KERNEL_MOVE_MAX_PARAMS,
                vxAddKernelCopySwapMoveValidate,
                vxAddKernelCopySwapMoveInitialize,
                NULL);
    vx_status status = vxGetStatus((vx_reference)kernel);
    if ((vx_status)VX_SUCCESS == status)
    {
        status = vxAddParameterToKernel(kernel,
                        VX_KERNEL_MOVE_FIRST_IDX,
                        (vx_enum)VX_BIDIRECTIONAL,
                        (vx_enum)VX_TYPE_REFERENCE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED);
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        status = vxAddParameterToKernel(kernel,
                        VX_KERNEL_MOVE_SECOND_IDX,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_REFERENCE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED);
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        status = vxFinalizeKernel(kernel);
    }
    if (status != (vx_status)VX_SUCCESS)
    {
        status = vxReleaseKernel(&kernel);
        vx_move_kernel = NULL;
    }
    else
    {
        vx_move_kernel = kernel;
    }
    return status;
}

vx_status tivxRemoveKernelMove(vx_context context)
{
    vx_status status;

    status = vxRemoveKernel(vx_move_kernel);
    vx_move_kernel = NULL;

    return status;
}

