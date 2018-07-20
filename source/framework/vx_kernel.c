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



#include <vx_internal.h>

VX_API_ENTRY vx_status VX_API_CALL vxReleaseKernel(vx_kernel *kernel)
{
    vx_status status = VX_SUCCESS;
    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&((*kernel)->base), VX_TYPE_KERNEL) == vx_true_e) )
    {
        ownReleaseReferenceInt((vx_reference *)kernel, VX_TYPE_KERNEL, VX_EXTERNAL, NULL);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxReleaseKernel: Invalid kernel reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryKernel(vx_kernel kern, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if ((NULL != kern) &&
        (ownIsValidSpecificReference(&kern->base, VX_TYPE_KERNEL) == vx_true_e))
    {
        vx_kernel kernel = kern;
        switch (attribute)
        {
            case VX_KERNEL_PARAMETERS:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = kernel->signature.num_parameters;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryKernel: Query kernel parameters failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_KERNEL_NAME:
                if ((ptr != NULL) && (size >= VX_MAX_KERNEL_NAME))
                {
                    strncpy(ptr, kernel->name, VX_MAX_KERNEL_NAME);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryKernel: Query kernel name failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_KERNEL_ENUM:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = kernel->enumeration;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryKernel: Query kernel enum failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_KERNEL_LOCAL_DATA_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = 0;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryKernel: Query kernel local data size failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "vxQueryKernel: Invalid query attribute\n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryKernel: Invalid kernel reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetKernelAttribute(vx_kernel kernel, vx_enum attribute, const void * ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e))
    {
        switch (attribute)
        {
            case VX_KERNEL_LOCAL_DATA_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    kernel->local_data_size = *(vx_size*)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetKernelAttribute: Set local data size failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "vxSetKernelAttribute: Invalid set kernel attribute\n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetKernelAttribute: Invalid kernel reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxRemoveKernel(vx_kernel kernel)
{
    vx_status status = VX_SUCCESS;
    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) ==
            vx_true_e))
    {
        if((kernel->lock_kernel_remove == vx_true_e)
            &&
            (ownContextGetKernelRemoveLock(kernel->base.context) == vx_false_e)
            )
        {
            /* kernel removal is locked, return error */
            VX_PRINT(VX_ZONE_ERROR, "vxRemoveKernel: Kernel is locked\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
        else
        {
            /* remove from context if it exists in context */
            ownRemoveKernelFromContext(kernel->base.context, kernel);

            status = vxReleaseKernel(&kernel);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxRemoveKernel: Invalid kernel reference\n");
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

    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e))
    {
        vx_kernel kern = kernel;
        if (index < kern->signature.num_parameters)
        {
            if (((ownIsValidType(data_type) == vx_false_e) ||
                 (ownIsValidDirection(dir) == vx_false_e) ||
                 (ownIsValidState(state) == vx_false_e)) ||
                 ((data_type == VX_TYPE_DELAY) && (dir != VX_INPUT)))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                if (ownIsValidType(data_type) == vx_false_e)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxAddParameterToKernel: Invalid data type\n");
                }
                if (ownIsValidDirection(dir) == vx_false_e)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxAddParameterToKernel: Invalid direction\n");
                }
                if (ownIsValidState(state) == vx_false_e)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxAddParameterToKernel: Invalid state\n");
                }
                if ((data_type == VX_TYPE_DELAY) && (dir != VX_INPUT))
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxAddParameterToKernel: Delay type is not input\n");
                }
            }
            else
            {
                kern->signature.directions[index] = dir;
                kern->signature.types[index] = data_type;
                kern->signature.states[index] = state;
                tivxLogSetResourceUsedValue("TIVX_KERNEL_MAX_PARAMS", kern->signature.num_parameters);
                status = VX_SUCCESS;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "vxAddParameterToKernel: Index value is greater than the number of parameters\n");
            VX_PRINT(VX_ZONE_ERROR, "vxAddParameterToKernel: May need to increase the value of TIVX_KERNEL_MAX_PARAMS in tiovx/include/tivx_config.h\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxAddParameterToKernel: Invalid kernel reference\n");
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

        if((numParams <= TIVX_KERNEL_MAX_PARAMS)
            &&
            (is_found == vx_false_e) /* not a duplicate kernel */
            )
        {
            kernel = (vx_kernel)ownCreateReference(context, VX_TYPE_KERNEL, VX_EXTERNAL, &context->base);
            if ((vxGetStatus((vx_reference)kernel) == VX_SUCCESS) && (kernel->base.type == VX_TYPE_KERNEL))
            {
                strncpy(kernel->name, name, VX_MAX_KERNEL_NAME);
                kernel->enumeration = enumeration;
                kernel->function = func_ptr;
                kernel->validate = validate;
                kernel->initialize = initialize;
                kernel->deinitialize = deinitialize;
                kernel->num_targets = 0;
                kernel->signature.num_parameters = numParams;
                kernel->local_data_size = 0;
                kernel->lock_kernel_remove = ownContextGetKernelRemoveLock(context);
                if(kernel->function)
                {
                    kernel->is_target_kernel = vx_false_e;
                }
                else
                {
                    kernel->is_target_kernel = vx_true_e;
                }
                for(idx=0; idx<TIVX_KERNEL_MAX_PARAMS; idx++)
                {
                    kernel->signature.directions[idx] = VX_TYPE_INVALID;
                    kernel->signature.types[idx] = VX_TYPE_INVALID;
                    kernel->signature.states[idx] = VX_TYPE_INVALID;
                }
                kernel->base.release_callback = (tivx_reference_release_callback_f)&vxReleaseKernel;
                if( !kernel->is_target_kernel )
                {
                    /* for user kernel, add to HOST target by default */
                    tivxAddKernelTarget(kernel, TIVX_TARGET_HOST);
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
    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e))
    {
        vx_uint32 p = 0;

        if(kernel->signature.num_parameters <= TIVX_KERNEL_MAX_PARAMS )
        {
            for (p = 0; p < kernel->signature.num_parameters; p++)
            {
                if (kernel->signature.directions[p] < VX_INPUT)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxFinalizeKernel: Kernel signature directions less than VX_INPUT\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }

                if (kernel->signature.directions[p] > VX_BIDIRECTIONAL)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxFinalizeKernel: Kernel signature directions greater than VX_BIDIRECTIONAL\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }

                if (ownIsValidType(kernel->signature.types[p]) == vx_false_e)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxFinalizeKernel: Invalid kernel signature type\n");
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
            VX_PRINT(VX_ZONE_ERROR, "vxFinalizeKernel: Number of parameters greater than maximum allowable\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxFinalizeKernel: Invalid kernel reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxAddKernelTarget(vx_kernel kernel, char *target_name)
{
    vx_status status = VX_SUCCESS;
    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e))
    {
        if(kernel->num_targets < TIVX_MAX_TARGETS_PER_KERNEL)
        {
            strncpy(kernel->target_name[kernel->num_targets],
                    target_name,
                    TIVX_TARGET_MAX_NAME
                );
            kernel->num_targets++;
            tivxLogSetResourceUsedValue("TIVX_MAX_TARGETS_PER_KERNEL", kernel->num_targets);
        }
        else
        {
            status = VX_ERROR_NO_RESOURCES;
            VX_PRINT(VX_ZONE_ERROR, "tivxAddKernelTarget: Number of targets greater than maximum allowable\n");
            VX_PRINT(VX_ZONE_ERROR, "tivxAddKernelTarget: May need to increase the value of TIVX_MAX_TARGETS_PER_KERNEL in tiovx/include/tivx_config.h\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxAddKernelTarget: Invalid kernel reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

vx_enum ownKernelGetDefaultTarget(vx_kernel kernel)
{
    vx_enum target_id = TIVX_TARGET_ID_INVALID;

    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e))
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

vx_enum ownKernelGetTarget(vx_kernel kernel, const char *target_string)
{
    uint32_t i;
    vx_enum target_id = TIVX_TARGET_ID_INVALID;

    if ((NULL != kernel) &&
          (ownIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) ==
                vx_true_e) &&
          (NULL != target_string))
    {
        if(kernel->num_targets==0)
        {
            target_id = TIVX_TARGET_ID_INVALID;
        }
        else
        {
            for (i = 0; i < kernel->num_targets; i ++)
            {
                if (0 == strncmp(kernel->target_name[i], target_string,
                        TIVX_TARGET_MAX_NAME))
                {
                    target_id = tivxPlatformGetTargetId(target_string);
                    break;
                }
            }
        }
    }

    return target_id;
}
