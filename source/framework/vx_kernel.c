/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */
/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <vx_internal.h>



VX_API_ENTRY vx_status VX_API_CALL vxReleaseKernel(vx_kernel *kernel)
{
    vx_status status = VX_SUCCESS;
    if (kernel && ownIsValidSpecificReference(&((*kernel)->base), VX_TYPE_KERNEL) == vx_true_e)
    {
        ownReleaseReferenceInt((vx_reference *)kernel, VX_TYPE_KERNEL, VX_EXTERNAL, NULL);
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryKernel(vx_kernel kern, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (kern && ownIsValidSpecificReference(&kern->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        vx_kernel kernel = kern;
        switch (attribute)
        {
            case VX_KERNEL_PARAMETERS:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = kernel->signature.num_parameters;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_KERNEL_NAME:
                if (ptr != NULL && size >= VX_MAX_KERNEL_NAME)
                {
                    strncpy(ptr, kernel->name, VX_MAX_KERNEL_NAME);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_KERNEL_ENUM:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = kernel->enumeration;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_KERNEL_LOCAL_DATA_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = 0;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetKernelAttribute(vx_kernel kern, vx_enum attribute, const void * ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (kern && ownIsValidSpecificReference(&kern->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        switch (attribute)
        {
            case VX_KERNEL_LOCAL_DATA_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    /* NOT required and not supported */
                    status = VX_ERROR_NOT_SUPPORTED;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxRemoveKernel(vx_kernel kernel)
{
    vx_status status = VX_SUCCESS;
    if (kernel && ownIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        status = ownRemoveKernelFromContext(kernel->base.context, kernel);
        if(status==VX_SUCCESS)
        {
            status = vxReleaseKernel(&kernel);
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAddParameterToKernel(vx_kernel kernel,
                                        vx_uint32 index,
                                        vx_enum dir,
                                        vx_enum data_type,
                                        vx_enum state)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if (kernel && ownIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        vx_kernel kern = kernel;
        if (index < kern->signature.num_parameters)
        {
            if (((ownIsValidType(data_type) == vx_false_e) ||
                 (ownIsValidDirection(dir) == vx_false_e) ||
                 (ownIsValidState(state) == vx_false_e)) ||
                 (data_type == VX_TYPE_DELAY && dir != VX_INPUT))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            else
            {
                kern->signature.directions[index] = dir;
                kern->signature.types[index] = data_type;
                kern->signature.states[index] = state;
                status = VX_SUCCESS;
            }
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_kernel VX_API_CALL vxAddUserKernel(vx_context context,
                             const vx_char name[VX_MAX_KERNEL_NAME],
                             vx_enum enumeration,
                             vx_kernel_f func_ptr,
                             vx_uint32 numParams,
                             vx_kernel_validate_f validate,
                             vx_kernel_initialize_f initialize,
                             vx_kernel_deinitialize_f deinitialize)
{
    vx_kernel kernel = NULL;
    vx_bool is_found;
    uint32_t idx;

    if(ownIsValidContext(context)==vx_true_e)
    {
        is_found = vx_false_e;

        ownIsKernelInContext(context, enumeration, name, &is_found);

        if(numParams <= TIVX_KERNEL_MAX_PARAMS
            &&
            is_found == vx_false_e /* not a duplicate kernel */
            )
        {
            kernel = (vx_kernel)ownCreateReference(context, VX_TYPE_KERNEL, VX_EXTERNAL, &context->base);
            if (vxGetStatus((vx_reference)kernel) == VX_SUCCESS && kernel->base.type == VX_TYPE_KERNEL)
            {
                strncpy(kernel->name, name, VX_MAX_KERNEL_NAME);
                kernel->enumeration = enumeration;
                kernel->function = func_ptr;
                kernel->validate = validate;
                kernel->initialize = initialize;
                kernel->deinitialize = deinitialize;
                kernel->num_targets = 0;
                kernel->signature.num_parameters = numParams;
                for(idx=0; idx<TIVX_KERNEL_MAX_PARAMS; idx++)
                {
                    kernel->signature.directions[idx] = VX_TYPE_INVALID;
                    kernel->signature.types[idx] = VX_TYPE_INVALID;
                    kernel->signature.states[idx] = VX_TYPE_INVALID;
                }
            }
        }
        else
        {
            kernel = (vx_kernel)ownGetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
        }
    }
    return kernel;
}

VX_API_ENTRY vx_status VX_API_CALL vxFinalizeKernel(vx_kernel kernel)
{
    vx_status status = VX_SUCCESS;
    if (kernel && ownIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        vx_uint32 p = 0;

        if(kernel->signature.num_parameters <= TIVX_KERNEL_MAX_PARAMS )
        {
            for (p = 0; p < kernel->signature.num_parameters; p++)
            {
                if ((kernel->signature.directions[p] < VX_INPUT) ||
                    (kernel->signature.directions[p] > VX_BIDIRECTIONAL))
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                else
                if (ownIsValidType(kernel->signature.types[p]) == vx_false_e)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                if(status!=VX_SUCCESS)
                {
                    break;
                }
            }
            if (status == VX_SUCCESS)
            {
                status = ownAddKernelToContext(kernel->base.context, kernel);
            }
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxAddKernelTarget(vx_kernel kernel, char *target_name)
{
    vx_status status = VX_SUCCESS;
    if (kernel && ownIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        if(kernel->num_targets < TIVX_MAX_TARGETS_PER_KERNEL)
        {
            strncpy(kernel->target_name[kernel->num_targets],
                    target_name,
                    TIVX_MAX_TARGET_NAME
                );
            kernel->num_targets++;
        }
        else
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

vx_enum ownKernelMatchTarget(vx_kernel kernel, const char *target_string)
{
    vx_enum target_id = TIVX_TARGET_ID_INVALID;

    if (kernel && ownIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        uint32_t idx;

        for(idx = 0; idx < kernel->num_targets; idx++)
        {
            if(tivxPlatformTargetMatch(kernel->target_name[idx], target_string) == vx_true_e)
            {
                /* found a compatible target for this kernel */
                target_id = tivxPlatformGetTargetId(target_string);
                break;
            }
        }
    }
    return target_id;
}

vx_enum ownKernelGetDefaultTarget(vx_kernel kernel)
{
    vx_enum target_id = TIVX_TARGET_ID_INVALID;

    if (kernel && ownIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        if(kernel->num_targets==0)
        {
            target_id = TIVX_TARGET_ID_INVALID;
        }
        else
        {
            /* valid target associated with kernel,
               take 0th index as default for this node
             */
            target_id = tivxPlatformGetTargetId(kernel->target_name[0]);
        }
    }
    return target_id;
}
