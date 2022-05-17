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
    vx_status status = (vx_status)VX_SUCCESS;
    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&((*kernel)->base), (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e) )
    {
        ownReleaseReferenceInt((vx_reference *)kernel, (vx_enum)VX_TYPE_KERNEL, (vx_enum)VX_EXTERNAL, NULL);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid kernel reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryKernel(vx_kernel kern, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if ((NULL != kern) &&
        (ownIsValidSpecificReference(&kern->base, (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e))
    {
        vx_kernel kernel = kern;
        switch (attribute)
        {
            case (vx_enum)VX_KERNEL_PARAMETERS:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = kernel->signature.num_parameters;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query kernel parameters failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_KERNEL_NAME:
                if ((ptr != NULL) && ((vx_enum)size >= VX_MAX_KERNEL_NAME))
                {
                    strncpy(ptr, kernel->name, VX_MAX_KERNEL_NAME);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query kernel name failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_KERNEL_ENUM:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = kernel->enumeration;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query kernel enum failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_KERNEL_LOCAL_DATA_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = 0;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query kernel local data size failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_KERNEL_PIPEUP_OUTPUT_DEPTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = kernel->num_pipeup_bufs;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query kernel local data size failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_KERNEL_TIMEOUT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = kernel->timeout_val;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query TIVX_KERNEL_TIMEOUT failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid query attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid kernel reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetKernelAttribute(vx_kernel kernel, vx_enum attribute, const void * ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&kernel->base, (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e))
    {
        switch (attribute)
        {
            case (vx_enum)VX_KERNEL_LOCAL_DATA_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    kernel->local_data_size = *(const vx_size*)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Set local data size failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_KERNEL_PIPEUP_OUTPUT_DEPTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    kernel->num_pipeup_bufs = *(const vx_uint32*)ptr;
                    kernel->pipeup_buf_idx  = *(const vx_uint32*)ptr;
                    if (kernel->num_pipeup_bufs > 1U)
                    {
                        kernel->state = (vx_enum)VX_NODE_STATE_PIPEUP;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Set pipeup buffers failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_KERNEL_TIMEOUT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    vx_uint32   timeout_val = *(vx_uint32*)ptr;

                    /* Validate the timeout. It cannot be zero. */
                    if (timeout_val == 0)
                    {
                        VX_PRINT(VX_ZONE_ERROR,
                                 "Invalid timeout value specified: %d\n",
                                 timeout_val);
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                    else
                    {
                        kernel->timeout_val = timeout_val;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Set TIVX_KERNEL_TIMEOUT failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid set kernel attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid kernel reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxRemoveKernel(vx_kernel kernel)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&kernel->base, (vx_enum)VX_TYPE_KERNEL) ==
            (vx_bool)vx_true_e))
    {
        if((kernel->lock_kernel_remove == (vx_bool)vx_true_e)
            &&
            (ownContextGetKernelRemoveLock(kernel->base.context) == (vx_bool)vx_false_e)
            )
        {
            /* kernel removal is locked, return error */
            VX_PRINT(VX_ZONE_ERROR, "Kernel is locked\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
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
        VX_PRINT(VX_ZONE_ERROR, "Invalid kernel reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAddParameterToKernel(vx_kernel kernel,
                                        vx_uint32 index,
                                        vx_enum dir,
                                        vx_enum data_type,
                                        vx_enum state)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&kernel->base, (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e))
    {
        vx_kernel kern = kernel;
        if (index < kern->signature.num_parameters)
        {
            if (((ownIsValidType(data_type) == (vx_bool)vx_false_e) ||
                 (ownIsValidDirection(dir) == (vx_bool)vx_false_e) ||
                 (ownIsValidState(state) == (vx_bool)vx_false_e)) ||
                 ((data_type == (vx_enum)VX_TYPE_DELAY) && (dir != (vx_enum)VX_INPUT)))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                if (ownIsValidType(data_type) == (vx_bool)vx_false_e)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Invalid data type\n");
                }
                if (ownIsValidDirection(dir) == (vx_bool)vx_false_e)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Invalid direction\n");
                }
                if (ownIsValidState(state) == (vx_bool)vx_false_e)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Invalid state\n");
                }
                if ((data_type == (vx_enum)VX_TYPE_DELAY) && (dir != (vx_enum)VX_INPUT))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Delay type is not input\n");
                }
            }
            else
            {
                kern->signature.directions[index] = dir;
                kern->signature.types[index] = data_type;
                kern->signature.states[index] = state;
                tivxLogSetResourceUsedValue("TIVX_KERNEL_MAX_PARAMS", (uint16_t)kern->signature.num_parameters);
                status = (vx_status)VX_SUCCESS;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Index value is greater than the number of parameters\n");
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_KERNEL_MAX_PARAMS in tiovx/include/tivx_config.h\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid kernel reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
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

    if(ownIsValidContext(context)==(vx_bool)vx_true_e)
    {
        if (NULL != name)
        {
            is_found = (vx_bool)vx_false_e;

            ownIsKernelInContext(context, enumeration, name, &is_found);
            if((numParams <= TIVX_KERNEL_MAX_PARAMS)
                && (
                (is_found == (vx_bool)vx_false_e) /* not a duplicate kernel */
                ))
            {
                kernel = (vx_kernel)ownCreateReference(context, (vx_enum)VX_TYPE_KERNEL, (vx_enum)VX_EXTERNAL, &context->base);
                if ((vxGetStatus((vx_reference)kernel) == (vx_status)VX_SUCCESS) && (kernel->base.type == (vx_enum)VX_TYPE_KERNEL))
                {
                    strncpy(kernel->name, name, VX_MAX_KERNEL_NAME-1);
                    kernel->name[VX_MAX_KERNEL_NAME-1]=(char)0;
                    vxSetReferenceName((vx_reference)kernel, kernel->name);
                    kernel->enumeration = enumeration;
                    kernel->function = func_ptr;
                    kernel->validate = validate;
                    kernel->initialize = initialize;
                    kernel->deinitialize = deinitialize;
                    kernel->num_targets = 0;
                    kernel->num_pipeup_bufs = 1;
                    kernel->pipeup_buf_idx  = 0;
                    kernel->num_sink_bufs = 1;
                    kernel->connected_sink_bufs = 1;
                    kernel->state = (vx_enum)VX_NODE_STATE_STEADY;
                    kernel->signature.num_parameters = numParams;
                    kernel->local_data_size = 0;
                    kernel->lock_kernel_remove = ownContextGetKernelRemoveLock(context);
                    kernel->timeout_val = TIVX_DEFAULT_KERNAL_TIMEOUT;
                    if(kernel->function != NULL)
                    {
                        kernel->is_target_kernel = (vx_bool)vx_false_e;
                    }
                    else
                    {
                        kernel->is_target_kernel = (vx_bool)vx_true_e;
                    }
                    for(idx=0; idx<TIVX_KERNEL_MAX_PARAMS; idx++)
                    {
                        kernel->signature.directions[idx] = (vx_enum)VX_TYPE_INVALID;
                        kernel->signature.types[idx] = (vx_enum)VX_TYPE_INVALID;
                        kernel->signature.states[idx] = (vx_enum)VX_TYPE_INVALID;
                    }
                    kernel->base.release_callback = (tivx_reference_release_callback_f)&vxReleaseKernel;
                    if(kernel->is_target_kernel == (vx_bool)(vx_bool)vx_false_e)
                    {
                        /* for user kernel, add to HOST target by default */
                        tivxAddKernelTarget(kernel, TIVX_TARGET_HOST);
                    }
                }
            }
            else
            {
                kernel = (vx_kernel)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
            }
        }
        else
        {
            kernel = (vx_kernel)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
            VX_PRINT(VX_ZONE_ERROR,"provided kernel name was NULL, please provide non-NULL kernel name\n");
        }
    }

    return kernel;
}

VX_API_ENTRY vx_status VX_API_CALL vxFinalizeKernel(vx_kernel kernel)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&kernel->base, (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e))
    {
        vx_uint32 p = 0;

        if(kernel->signature.num_parameters <= TIVX_KERNEL_MAX_PARAMS )
        {
            for (p = 0; p < kernel->signature.num_parameters; p++)
            {
                if (kernel->signature.directions[p] < (vx_enum)VX_INPUT)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Kernel signature directions less than VX_INPUT\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }

                if (kernel->signature.directions[p] > (vx_enum)VX_BIDIRECTIONAL)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Kernel signature directions greater than VX_BIDIRECTIONAL\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }

                if (ownIsValidType(kernel->signature.types[p]) == (vx_bool)vx_false_e)
                {
                    printf("p = %d\n", p);
                    VX_PRINT(VX_ZONE_ERROR, "Invalid kernel signature type\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                if(status!=(vx_status)VX_SUCCESS)
                {
                    break;
                }
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = ownAddKernelToContext(kernel->base.context, kernel);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Number of parameters greater than maximum allowable\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid kernel reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxAddKernelTarget(vx_kernel kernel, const char *target_name)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&kernel->base, (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e))
    {
        if(kernel->num_targets < TIVX_MAX_TARGETS_PER_KERNEL)
        {
            strncpy(kernel->target_name[kernel->num_targets],
                    target_name,
                    TIVX_TARGET_MAX_NAME-1U
                );
            kernel->target_name[kernel->num_targets][TIVX_TARGET_MAX_NAME-1U] = '\0';
            kernel->num_targets++;
            tivxLogSetResourceUsedValue("TIVX_MAX_TARGETS_PER_KERNEL", (uint16_t)kernel->num_targets);
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
            VX_PRINT(VX_ZONE_ERROR, "Number of targets greater than maximum allowable\n");
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_MAX_TARGETS_PER_KERNEL in tiovx/include/tivx_config.h\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid kernel reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxSetKernelSinkDepth(vx_kernel kernel, uint32_t num_sink_bufs)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&kernel->base, (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e))
    {
        kernel->num_sink_bufs = num_sink_bufs;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid kernel reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

vx_enum ownKernelGetDefaultTarget(vx_kernel kernel)
{
    vx_enum target_id = (vx_enum)TIVX_TARGET_ID_INVALID;

    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(&kernel->base, (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e))
    {
        if(kernel->num_targets==0U)
        {
            target_id = (vx_enum)TIVX_TARGET_ID_INVALID;
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
    vx_enum target_id = (vx_enum)TIVX_TARGET_ID_INVALID;

    if ((NULL != kernel) &&
          (ownIsValidSpecificReference(&kernel->base, (vx_enum)VX_TYPE_KERNEL) ==
                (vx_bool)vx_true_e) &&
          (NULL != target_string))
    {
        if(kernel->num_targets==0U)
        {
            target_id = (vx_enum)TIVX_TARGET_ID_INVALID;
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
