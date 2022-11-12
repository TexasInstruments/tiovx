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

static vx_status ownDestructParameter(vx_reference ref);

static vx_status ownDestructParameter(vx_reference ref)
{
    vx_parameter param = (vx_parameter)ref;

    if(param != NULL)
    {
        if (ownIsValidSpecificReference((vx_reference)param->node, (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
        {
            vx_node node = (vx_node)param->node;
            ownReleaseReferenceInt((vx_reference *)&node, (vx_enum)VX_TYPE_NODE, (vx_enum)VX_INTERNAL, NULL);
        }
        if (ownIsValidSpecificReference((vx_reference)param->kernel, (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e)
        {
            vx_kernel kernel = (vx_kernel)param->kernel;
            ownReleaseReferenceInt((vx_reference *)&kernel, (vx_enum)VX_TYPE_KERNEL, (vx_enum)VX_INTERNAL, NULL);
        }
    }
    return (vx_status)VX_SUCCESS;
}

vx_bool ownIsValidDirection(vx_enum dir)
{
    vx_bool is_valid;

    if ((dir == (vx_enum)VX_INPUT) || (dir == (vx_enum)VX_OUTPUT)) /* Bidirectional is not valid for user kernels */
    {
        is_valid = (vx_bool)vx_true_e;
    }
    else
    {
        is_valid = (vx_bool)vx_false_e;
    }

    return is_valid;
}

vx_bool ownIsValidTypeMatch(vx_enum expected, vx_enum supplied)
{
    vx_bool match = (vx_bool)vx_false_e;
    if (expected == supplied)
    {
        match = (vx_bool)vx_true_e;
    }
    if (match == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "Expected %08x and got %08x!\n", expected, supplied);
    }
    return match;
}

vx_bool ownIsValidState(vx_enum state)
{
    vx_bool is_valid;

    if ((state == (vx_enum)VX_PARAMETER_STATE_REQUIRED) ||
        (state == (vx_enum)VX_PARAMETER_STATE_OPTIONAL))
    {
        is_valid = (vx_bool)vx_true_e;
    }
    else
    {
        is_valid = (vx_bool)vx_false_e;
    }

    return is_valid;
}

/******************************************************************************/
/* PUBLIC API */
/******************************************************************************/

VX_API_ENTRY vx_parameter VX_API_CALL vxGetKernelParameterByIndex(vx_kernel kernel, vx_uint32 index)
{
    vx_parameter parameter = NULL;

    if (ownIsValidSpecificReference((vx_reference)kernel, (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e)
    {
        if ((index < TIVX_KERNEL_MAX_PARAMS) && (index < kernel->signature.num_parameters))
        {
            parameter = (vx_parameter)ownCreateReference(kernel->base.context, (vx_enum)VX_TYPE_PARAMETER, (vx_enum)VX_EXTERNAL, &kernel->base.context->base);
            if ((vxGetStatus((vx_reference)parameter) == (vx_status)VX_SUCCESS) && (parameter->base.type == (vx_enum)VX_TYPE_PARAMETER))
            {
                parameter->base.destructor_callback = &ownDestructParameter;
                parameter->base.release_callback = (tivx_reference_release_callback_f)&vxReleaseParameter;
                parameter->index = index;
                parameter->node = NULL;
                parameter->kernel = kernel;
                ownIncrementReference(&parameter->kernel->base, (vx_enum)VX_INTERNAL);
            }
        }
        else
        {
            vxAddLogEntry(&kernel->base, (vx_status)VX_ERROR_INVALID_PARAMETERS, "Index %u out of range for kernel %s (numparams = %u)!\n",
                    index, kernel->name, kernel->signature.num_parameters);
            parameter = (vx_parameter)ownGetErrorObject(kernel->base.context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
        }
    }

    return parameter;
}

VX_API_ENTRY vx_parameter VX_API_CALL vxGetParameterByIndex(vx_node node, vx_uint32 index)
{
    vx_parameter param = NULL;
    if (ownIsValidSpecificReference((vx_reference)node, (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        if (node->kernel == NULL)
        {
            /* this can probably never happen */
            vxAddLogEntry(&node->base, (vx_status)VX_ERROR_INVALID_NODE, "Node was created without a kernel! Fatal Error!\n");
            param = (vx_parameter)ownGetErrorObject(node->base.context, (vx_status)VX_ERROR_INVALID_NODE);
        }
        else
        {
            if ((index < TIVX_KERNEL_MAX_PARAMS) && (index < node->kernel->signature.num_parameters))
            {
                param = (vx_parameter)ownCreateReference(node->base.context, (vx_enum)VX_TYPE_PARAMETER, (vx_enum)VX_EXTERNAL, &node->base);
                if ((vxGetStatus((vx_reference)param) == (vx_status)VX_SUCCESS) && (param->base.type == (vx_enum)VX_TYPE_PARAMETER))
                {
                    param->base.destructor_callback = &ownDestructParameter;
                    param->base.release_callback = (tivx_reference_release_callback_f)&vxReleaseParameter;
                    param->index = index;
                    param->node = node;
                    ownIncrementReference(&param->node->base, (vx_enum)VX_INTERNAL);
                    param->kernel = node->kernel;
                    ownIncrementReference(&param->kernel->base, (vx_enum)VX_INTERNAL);
                }
            }
            else
            {
                vxAddLogEntry(&node->base, (vx_status)VX_ERROR_INVALID_PARAMETERS, "Index %u out of range for node %s (numparams = %u)!\n",
                        index, node->kernel->name, node->kernel->signature.num_parameters);
                param = (vx_parameter)ownGetErrorObject(node->base.context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
            }
        }
    }
    return param;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseParameter(vx_parameter *param)
{
    return ownReleaseReferenceInt((vx_reference *)param, (vx_enum)VX_TYPE_PARAMETER, (vx_enum)VX_EXTERNAL, NULL);
}


VX_API_ENTRY vx_status VX_API_CALL vxSetParameterByIndex(vx_node node, vx_uint32 index, vx_reference value)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum type = 0;
    vx_enum data_type = 0;

    if (ownIsValidSpecificReference((vx_reference)node, (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "Supplied node was not actually a node\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_PARAMETER, "Attempting to set parameter[%u] on %s (enum:%d) to "VX_FMT_REF"\n",
                    index,
                    node->kernel->name,
                    node->kernel->enumeration,
                    value);
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        /* is the index out of bounds? */
        if ((index >= node->kernel->signature.num_parameters) || (index >= TIVX_KERNEL_MAX_PARAMS))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid index %u\n", index);
            status = (vx_status)VX_ERROR_INVALID_VALUE;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        /* if it's an optional parameter, it's ok to be NULL */
        if ((value == 0) && (node->kernel->signature.states[index] == (vx_enum)VX_PARAMETER_STATE_OPTIONAL))
        {
            status = (vx_status)VX_SUCCESS;
        }
        else
        {
            /* if it's required, it's got to exist */
            if (ownIsValidReference(value) == (vx_bool)vx_false_e)
            {
                VX_PRINT(VX_ZONE_ERROR, "Supplied value was not actually a reference\n");
                status = (vx_status)VX_ERROR_INVALID_REFERENCE;
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* if it was a valid reference then get the type from it */
                vxQueryReference(value, (vx_enum)VX_REFERENCE_TYPE, &type, sizeof(type));
                VX_PRINT(VX_ZONE_PARAMETER, "Query returned type %08x for ref "VX_FMT_REF"\n", type, value);

                /* Check that signature type matches reference type*/
                if (node->kernel->signature.types[index] != type)
                {
                    /* Check special case where signature is a specific scalar type.
                       This can happen if the vxAddParameterToKernel() passes one of the scalar
                       vx_type_e types instead of the more generic (vx_enum)VX_TYPE_SCALAR since the spec
                       doesn't specify that only (vx_enum)VX_TYPE_SCALAR should be used for scalar types in
                       this function. */
                    if((type == (vx_enum)VX_TYPE_SCALAR) &&
                       (vxQueryScalar((vx_scalar)value, (vx_enum)VX_SCALAR_TYPE, &data_type, sizeof(data_type)) == (vx_status)VX_SUCCESS))
                    {
                        if(data_type != node->kernel->signature.types[index])
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Invalid scalar type 0x%08x!\n", data_type);
                            status = (vx_status)VX_ERROR_INVALID_TYPE;
                        }
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Invalid type 0x%08x!\n", type);
                        status = (vx_status)VX_ERROR_INVALID_TYPE;
                    }
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                if (node->parameters[index] != NULL)
                {
                    if (node->parameters[index]->delay!=NULL) {
                        /* we already have a delay element here */
                        vx_bool res = ownRemoveAssociationToDelay(node->parameters[index], node, index);
                        if (res == (vx_bool)vx_false_e) {
                            VX_PRINT(VX_ZONE_ERROR, "Internal error removing delay association\n");
                            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
                        }
                    }
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                if (value->delay!=NULL) {
                    /* the new parameter is a delay element */
                    vx_bool res = ownAddAssociationToDelay(value, node, index);
                    if (res == (vx_bool)vx_false_e) {
                        VX_PRINT(VX_ZONE_ERROR, "Internal error adding delay association\n");
                        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
                    }
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                ownNodeSetParameter(node, index, value);
            }

            /* Note that we don't need to do anything special for parameters to child graphs. */
        }

        if (status == (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_PARAMETER, "Assigned Node[%u] %p type:%08x ref="VX_FMT_REF"\n",
                     index, node, type, value);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Specified: parameter[%u] type:%08x => "VX_FMT_REF"\n",
                            index, type, value);
            VX_PRINT(VX_ZONE_ERROR, "Required: parameter[%u] dir:%d type:%08x\n",
                index,
                node->kernel->signature.directions[index],
                node->kernel->signature.types[index]);
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetParameterByReference(vx_parameter parameter, vx_reference value)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    if (ownIsValidSpecificReference((vx_reference)parameter, (vx_enum)VX_TYPE_PARAMETER) == (vx_bool)vx_true_e)
    {
        if (parameter->node != NULL)
        {
            status = vxSetParameterByIndex(parameter->node, parameter->index, value);
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryParameter(vx_parameter parameter, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if (ownIsValidSpecificReference((vx_reference)parameter, (vx_enum)VX_TYPE_PARAMETER) == (vx_bool)vx_true_e)
    {
        switch (attribute)
        {
            case (vx_enum)VX_PARAMETER_DIRECTION:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = parameter->kernel->signature.directions[parameter->index];
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query parameter direction failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_PARAMETER_INDEX:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = parameter->index;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query parameter index failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_PARAMETER_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = parameter->kernel->signature.types[parameter->index];
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query parameter type failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_PARAMETER_STATE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = (vx_enum)parameter->kernel->signature.states[parameter->index];
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query parameter state failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_PARAMETER_REF:
                if (VX_CHECK_PARAM(ptr, size, vx_reference, 0x3U))
                {
                    if (parameter->node != NULL)
                    {
                        vx_reference ref = parameter->node->parameters[parameter->index];
                        /* does this object have USER access? */
                        if (ref != NULL)
                        {
                            /*! \internal this could potentially allow the user to break
                             * a currently chosen optimization! We need to alert the
                             * system that if a write occurs to this data, put the graph
                             * into an unverified state.
                             */
                            ownIncrementReference(ref, (vx_enum)VX_EXTERNAL);
                        }
                        *(vx_reference *)ptr = (vx_reference)ref;
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Query parameter reference failed\n");
                        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query parameter reference failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}


