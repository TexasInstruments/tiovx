/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
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



#include <vx_internal.h>

static vx_status ownDestructKernel(vx_reference ref);

/* This will only get called if user calls vxRemoveKernel, AND all references
 * are released */
static vx_status ownDestructKernel(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_kernel kernel = NULL;

    if((ref != NULL) && /* TIOVX-1893- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_KERNEL_UBR001 */
    (ref->type == (vx_enum)VX_TYPE_KERNEL)) /* TIOVX-1893- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_KERNEL_UBR002 */
    {
        /* status set to NULL due to preceding type check */
        kernel = vxCastRefAsKernel(ref, NULL);
        vx_bool is_found;

        /* It is possible that the kernel has been created, but not added to the context table.  The case
         * where this is possible is if the vxAddUserKernel API has been called on the kernel, but the
         * vxFinalizeKernel API has not been called on the kernel.  If the vxFinalizeKernel has been called,
         * then it will need to be removed.  If not, then this should be skipped to avoid a misleading status
         * return. */
        status = ownIsKernelInContext(kernel->base.context, kernel->enumeration, kernel->name, &is_found);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_KERNEL_UBR003
<justification end> */
        if ( ((vx_status)VX_SUCCESS == status) &&
        (is_found == (vx_bool)vx_true_e) )
        {
            VX_PRINT(VX_ZONE_INFO, "Kernel %s destructor called (removed from context)\n", kernel->name);
            status = ownRemoveKernelFromContext(kernel->base.context, kernel);
        }
/* LDRA_JUSTIFY_END */
    }
    return status;
}


VX_API_ENTRY vx_status VX_API_CALL vxReleaseKernel(vx_kernel *kernel)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if ((NULL != kernel) &&
        (ownIsValidSpecificReference(vxCastRefFromKernel(*kernel), (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e) )
    {
        status = ownReleaseReferenceInt(vxCastRefFromKernelP(kernel),
                (vx_enum)VX_TYPE_KERNEL, (vx_enum)VX_EXTERNAL, NULL);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_KERNEL_UM001
<justification end> */
        if((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to destroy reference\n");
        }
/* LDRA_JUSTIFY_END */
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
    if (ownIsValidSpecificReference(vxCastRefFromKernel(kern), (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e)
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
                    (void)strncpy(ptr, kernel->name, VX_MAX_KERNEL_NAME);
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
    if (ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e)
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
                    const vx_uint32   timeout_val = *(const vx_uint32*)ptr;

                    /* Validate the timeout. It cannot be zero. */
                    if (timeout_val == 0U)
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
    if (ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL) ==
            (vx_bool)vx_true_e)
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
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_KERNEL_UBR009
<justification end> */
            /* This is the first time vxRemoveKernel is called on this kernel */
            if ( kernel->base.destructor_callback == NULL )
            {
                /* By default, we don't want to destruct user kernel unless user calls vxRemoveKernel,
                 * at which point we set the destructor callback so that it gets removed when all references
                 * are removed */
                kernel->base.destructor_callback = &ownDestructKernel;

                /* Decrementing internal ref count given when it was added to context.  Do this now even though
                 * kernel will be removed in destructor.  This is needed so that the destructor will properly
                 * get called when all other references have been released.
                 * Setting it void as return value 'count' is not used further.
                 */
                (void)ownDecrementReference(&kernel->base, (vx_enum)VX_INTERNAL);
            }
/* LDRA_JUSTIFY_END */

/* The following logic is to throw an error if :
 *   1. vxRemoveKernel is called on a kernel more than once
 *   2. the external reference count is > 1 when vxRemoveKernel is called
 *   3. the internal reference count is > 0 when vxRemoveKernel is called.
 *   The OpenVX specification says that a VX_FAILURE is returned if there are any remaining external or
 *   internal references.
 *   TIOVX deviates from the spec to give more application flexibility, and will allow this function to
 *   be called with outstanding references, and the destructor should ensure that it is finally removed
 *   when other references are released. In order to change to match OpenVX spec, remove #if 0 below */
#if 0
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "vxRemoveKernel has been called on Kernel %s more than once, perhaps you meant to call vxReleaseKernel?\n", kernel->name);
                status = (vx_status)VX_FAILURE;
            }

            if ( kernel->base.external_count > 1U ) /* Calling vxReleaseKernel below should decrement from 1 to 0 */
            {
                VX_PRINT(VX_ZONE_ERROR, "Kernel %s has remaining external references, check if you have released \n", kernel->name);
                VX_PRINT(VX_ZONE_ERROR, "  all associated references to the kernel object \n" );
                status = (vx_status)VX_FAILURE;
            }

            if ( kernel->base.internal_count > 0U ) /* Calling ownDecrementReference above should have decremented this from 1 to 0 */
            {
                VX_PRINT(VX_ZONE_ERROR, "Kernel %s has remaining internal references, check if you have released \n", kernel->name);
                VX_PRINT(VX_ZONE_ERROR, "  all associated graphs (vxReleaseGraph) or removed all associated nodes (vxRemoveNode)\n" );
                status = (vx_status)VX_FAILURE;
            }
#endif

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

    if (ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e)
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
                ownLogSetResourceUsedValue("TIVX_KERNEL_MAX_PARAMS", (uint16_t)kern->signature.num_parameters);
                status = (vx_status)VX_SUCCESS;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Index value is greater than the number of parameters\n");
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_KERNEL_MAX_PARAMS in tiovx/include/TI/tivx_config.h\n");
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
                             const vx_char *name,
                             vx_enum enumeration,
                             vx_kernel_f func_ptr,
                             vx_uint32 numParams,
                             vx_kernel_validate_f validate,
                             vx_kernel_initialize_f init,
                             vx_kernel_deinitialize_f deinit)
{
    vx_kernel kernel = NULL;
    vx_reference ref = NULL;
    vx_bool is_found;
    uint32_t idx;
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownIsValidContext(context)==(vx_bool)vx_true_e)
    {
        if (NULL != name)
        {
            is_found = (vx_bool)vx_false_e;

            status = ownIsKernelInContext(context, enumeration, name, &is_found);
            if(((vx_status)VX_SUCCESS == status) && (numParams <= TIVX_KERNEL_MAX_PARAMS)
                && (
                (is_found == (vx_bool)vx_false_e) /* not a duplicate kernel */
                ))
            {
                ref = ownCreateReference(context, (vx_enum)VX_TYPE_KERNEL, (vx_enum)VX_EXTERNAL, &context->base);
                if ((vxGetStatus(ref) == (vx_status)VX_SUCCESS) && /* TIOVX-1893- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_KERNEL_UBR005 */
                (ref->type == (vx_enum)VX_TYPE_KERNEL)) /* TIOVX-1893- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_KERNEL_UBR006 */
                {
                    /* status set to NULL due to preceding type check */
                    kernel = vxCastRefAsKernel(ref, NULL);
                    (void)strncpy(kernel->name, name, VX_MAX_KERNEL_NAME-1);
                    kernel->name[VX_MAX_KERNEL_NAME-1]=(char)0;
                    /* Error status check is not done due to the previous condition check */
                    (void)vxSetReferenceName(vxCastRefFromKernel(kernel), kernel->name);
                    kernel->enumeration = enumeration;
                    kernel->function = func_ptr;
                    kernel->validate = validate;
                    kernel->initialize = init;
                    kernel->deinitialize = deinit;
                    kernel->num_targets = 0;
                    kernel->num_pipeup_bufs = 1;
                    kernel->pipeup_buf_idx  = 0;
                    kernel->num_sink_bufs = 1;
                    kernel->connected_sink_bufs = 1;
                    kernel->state = (vx_enum)VX_NODE_STATE_STEADY;
                    kernel->signature.num_parameters = numParams;
                    kernel->local_data_size = 0;
                    kernel->lock_kernel_remove = ownContextGetKernelRemoveLock(context);
                    kernel->timeout_val = TIVX_DEFAULT_KERNEL_TIMEOUT;
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
                    /* By default, we don't want to destruct user kernel unless user calls vxRemoveKernel,
                     * at which point we set the destructor callback so that it gets removed when all references
                     * are removed */
                    kernel->base.destructor_callback = NULL;
                    kernel->base.release_callback = &ownReleaseReferenceBufferGeneric;
                    if(kernel->is_target_kernel == (vx_bool)vx_false_e)
                    {
                        /* for user kernel, add to HOST target by default */
                        status = tivxAddKernelTarget(kernel, TIVX_TARGET_HOST);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_KERNEL_UM002
<justification end> */
                        if((vx_status)VX_SUCCESS != status)
                        {
                            VX_PRINT(VX_ZONE_ERROR,"Failed to add kernel to host target\n");
                            kernel = (vx_kernel)ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
                        }
/* LDRA_JUSTIFY_END */
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
    if (ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e)
    {
        vx_uint32 p = 0;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_KERNEL_UBR007
<justification end>*/
        if(kernel->signature.num_parameters <= TIVX_KERNEL_MAX_PARAMS )
/* LDRA_JUSTIFY_END */
        {
            for (p = 0; p < kernel->signature.num_parameters; p++)
            {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_KERNEL_UTJT001
<justification end> */
                if (kernel->signature.directions[p] < (vx_enum)VX_INPUT)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Kernel signature directions less than VX_INPUT\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_KERNEL_UTJT002
<justification end> */
                if (kernel->signature.directions[p] > (vx_enum)VX_BIDIRECTIONAL)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Kernel signature directions greater than VX_BIDIRECTIONAL\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_KERNEL_UTJT003
<justification end> */
                if (ownIsValidType(kernel->signature.types[p]) == (vx_bool)vx_false_e)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Invalid kernel signature type\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_KERNEL_UTJT006
<justification end> */
                if(status!=(vx_status)VX_SUCCESS)
                {
                    break;
                }
/* LDRA_JUSTIFY_END */
            }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_KERNEL_UBR008
<justification end> */
            if (status == (vx_status)VX_SUCCESS)
            {
                status = ownAddKernelToContext(kernel->base.context, kernel);
            }
/* LDRA_JUSTIFY_END */
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_KERNEL_UTJT004
<justification end>*/
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Number of parameters greater than maximum allowable\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
/* LDRA_JUSTIFY_END */
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
    if ((ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL)) == (vx_bool)vx_true_e)
    {
        if(kernel->num_targets < TIVX_MAX_TARGETS_PER_KERNEL)
        {
            (void)strncpy(kernel->target_name[kernel->num_targets],
                    target_name,
                    TIVX_TARGET_MAX_NAME-1U
                );
            kernel->target_name[kernel->num_targets][TIVX_TARGET_MAX_NAME-1U] = '\0';
            kernel->num_targets++;
            ownLogSetResourceUsedValue("TIVX_MAX_TARGETS_PER_KERNEL", (uint16_t)kernel->num_targets);
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
            VX_PRINT(VX_ZONE_ERROR, "Number of targets greater than maximum allowable\n");
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_MAX_TARGETS_PER_KERNEL in tiovx/include/TI/tivx_config.h\n");
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

    if (ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e)
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

    if ((ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL)) == (vx_bool)vx_true_e)
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
            target_id = ownPlatformGetTargetId(kernel->target_name[0]);
        }
    }
    return target_id;
}

vx_enum ownKernelGetTarget(vx_kernel kernel, const char *target_string)
{
    uint32_t i;
    vx_enum target_id = (vx_enum)TIVX_TARGET_ID_INVALID;

    if (((ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL)) == (vx_bool)vx_true_e) && (NULL != target_string))
    {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_KERNEL_UTJT005
<justification end>*/
        if(kernel->num_targets==0U)
        {
            target_id = (vx_enum)TIVX_TARGET_ID_INVALID;
        }
        else
/* LDRA_JUSTIFY_END */
        {
            for (i = 0; i < kernel->num_targets; i ++)
            {
                if (0 == strncmp(kernel->target_name[i], target_string,
                        TIVX_TARGET_MAX_NAME))
                {
                    target_id = ownPlatformGetTargetId(target_string);
                    break;
                }
            }
        }
    }

    return target_id;
}
