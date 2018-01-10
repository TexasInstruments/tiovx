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

static vx_status ownDestructParameter(vx_reference ref);

static vx_status ownDestructParameter(vx_reference ref)
{
    vx_parameter param = (vx_parameter)ref;

    if(param != NULL)
    {
        if (param->node)
        {
            if (ownIsValidSpecificReference(&param->node->base, VX_TYPE_NODE) == vx_true_e)
            {
                vx_node node = (vx_node)param->node;
                ownReleaseReferenceInt((vx_reference *)&node, VX_TYPE_NODE, VX_INTERNAL, NULL);
            }
        }
        if (param->kernel)
        {
            if (ownIsValidSpecificReference(&param->kernel->base, VX_TYPE_KERNEL) == vx_true_e)
            {
                vx_kernel kernel = (vx_kernel)param->kernel;
                ownReleaseReferenceInt((vx_reference *)&kernel, VX_TYPE_KERNEL, VX_INTERNAL, NULL);
            }
        }
    }
    return VX_SUCCESS;
}

vx_bool ownIsValidDirection(vx_enum dir)
{
    vx_bool is_valid;

    if ((dir == VX_INPUT) || (dir == VX_OUTPUT)) /* Bidirectional is not valid for user kernels */
    {
        is_valid = vx_true_e;
    }
    else
    {
        is_valid = vx_false_e;
    }

    return is_valid;
}

vx_bool ownIsValidTypeMatch(vx_enum expected, vx_enum supplied)
{
    vx_bool match = vx_false_e;
    if (expected == supplied)
    {
        match = vx_true_e;
    }
    if (match == vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "Expected %08x and got %08x!\n", expected, supplied);
    }
    return match;
}

vx_bool ownIsValidState(vx_enum state)
{
    vx_bool is_valid;

    if ((state == VX_PARAMETER_STATE_REQUIRED) ||
        (state == VX_PARAMETER_STATE_OPTIONAL))
    {
        is_valid = vx_true_e;
    }
    else
    {
        is_valid = vx_false_e;
    }

    return is_valid;
}

/******************************************************************************/
/* PUBLIC API */
/******************************************************************************/

VX_API_ENTRY vx_parameter VX_API_CALL vxGetKernelParameterByIndex(vx_kernel kernel, vx_uint32 index)
{
    vx_parameter parameter = NULL;

    if (ownIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        if ((index < TIVX_KERNEL_MAX_PARAMS) && (index < kernel->signature.num_parameters))
        {
            parameter = (vx_parameter)ownCreateReference(kernel->base.context, VX_TYPE_PARAMETER, VX_EXTERNAL, &kernel->base.context->base);
            if ((vxGetStatus((vx_reference)parameter) == VX_SUCCESS) && (parameter->base.type == VX_TYPE_PARAMETER))
            {
                parameter->base.destructor_callback = &ownDestructParameter;
                parameter->base.release_callback = (tivx_reference_release_callback_f)&vxReleaseParameter;
                parameter->index = index;
                parameter->node = NULL;
                parameter->kernel = kernel;
                ownIncrementReference(&parameter->kernel->base, VX_INTERNAL);
            }
        }
        else
        {
            vxAddLogEntry(&kernel->base, VX_ERROR_INVALID_PARAMETERS, "Index %u out of range for kernel %s (numparams = %u)!\n",
                    index, kernel->name, kernel->signature.num_parameters);
            parameter = (vx_parameter)ownGetErrorObject(kernel->base.context, VX_ERROR_INVALID_PARAMETERS);
        }
    }

    return parameter;
}

VX_API_ENTRY vx_parameter VX_API_CALL vxGetParameterByIndex(vx_node node, vx_uint32 index)
{
    vx_parameter param = NULL;
    if (ownIsValidSpecificReference(&node->base, VX_TYPE_NODE) == vx_true_e)
    {
        if (node->kernel == NULL)
        {
            /* this can probably never happen */
            vxAddLogEntry(&node->base, VX_ERROR_INVALID_NODE, "Node was created without a kernel! Fatal Error!\n");
            param = (vx_parameter)ownGetErrorObject(node->base.context, VX_ERROR_INVALID_NODE);
        }
        else
        {
            if ((index < TIVX_KERNEL_MAX_PARAMS) && (index < node->kernel->signature.num_parameters))
            {
                param = (vx_parameter)ownCreateReference(node->base.context, VX_TYPE_PARAMETER, VX_EXTERNAL, &node->base);
                if ((vxGetStatus((vx_reference)param) == VX_SUCCESS) && (param->base.type == VX_TYPE_PARAMETER))
                {
                    param->base.destructor_callback = &ownDestructParameter;
                    param->base.release_callback = (tivx_reference_release_callback_f)&vxReleaseParameter;
                    param->index = index;
                    param->node = node;
                    ownIncrementReference(&param->node->base, VX_INTERNAL);
                    param->kernel = node->kernel;
                    ownIncrementReference(&param->kernel->base, VX_INTERNAL);
                }
            }
            else
            {
                vxAddLogEntry(&node->base, VX_ERROR_INVALID_PARAMETERS, "Index %u out of range for node %s (numparams = %u)!\n",
                        index, node->kernel->name, node->kernel->signature.num_parameters);
                param = (vx_parameter)ownGetErrorObject(node->base.context, VX_ERROR_INVALID_PARAMETERS);
            }
        }
    }
    return param;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseParameter(vx_parameter *param)
{
    return ownReleaseReferenceInt((vx_reference *)param, VX_TYPE_PARAMETER, VX_EXTERNAL, NULL);
}


VX_API_ENTRY vx_status VX_API_CALL vxSetParameterByIndex(vx_node node, vx_uint32 index, vx_reference value)
{
    vx_status status = VX_SUCCESS;
    vx_enum type = 0;
    vx_enum data_type = 0;

    if (ownIsValidSpecificReference(&node->base, VX_TYPE_NODE) == vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "Supplied node was not actually a node\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    if(status == VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_PARAMETER, "Attempting to set parameter[%u] on %s (enum:%d) to "VX_FMT_REF"\n",
                    index,
                    node->kernel->name,
                    node->kernel->enumeration,
                    value);
    }

    if(status == VX_SUCCESS)
    {
        /* is the index out of bounds? */
        if ((index >= node->kernel->signature.num_parameters) || (index >= TIVX_KERNEL_MAX_PARAMS))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid index %u\n", index);
            status = VX_ERROR_INVALID_VALUE;
        }
    }

    if(status == VX_SUCCESS)
    {
        /* if it's an optional parameter, it's ok to be NULL */
        if ((value == 0) && (node->kernel->signature.states[index] == VX_PARAMETER_STATE_OPTIONAL))
        {
            status = VX_SUCCESS;
        }
        else
        {
            /* if it's required, it's got to exist */
            if (ownIsValidReference(value) == vx_false_e)
            {
                VX_PRINT(VX_ZONE_ERROR, "Supplied value was not actually a reference\n");
                status = VX_ERROR_INVALID_REFERENCE;
            }

            if(status == VX_SUCCESS)
            {
                /* if it was a valid reference then get the type from it */
                vxQueryReference(value, VX_REFERENCE_TYPE, &type, sizeof(type));
                VX_PRINT(VX_ZONE_PARAMETER, "Query returned type %08x for ref "VX_FMT_REF"\n", type, value);

                /* Check that signature type matches reference type*/
                if (node->kernel->signature.types[index] != type)
                {
                    /* Check special case where signature is a specific scalar type.
                       This can happen if the vxAddParameterToKernel() passes one of the scalar
                       vx_type_e types instead of the more generic VX_TYPE_SCALAR since the spec
                       doesn't specify that only VX_TYPE_SCALAR should be used for scalar types in
                       this function. */
                    if((type == VX_TYPE_SCALAR) &&
                       (vxQueryScalar((vx_scalar)value, VX_SCALAR_TYPE, &data_type, sizeof(data_type)) == VX_SUCCESS))
                    {
                        if(data_type != node->kernel->signature.types[index])
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Invalid scalar type 0x%08x!\n", data_type);
                            status = VX_ERROR_INVALID_TYPE;
                        }
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Invalid type 0x%08x!\n", type);
                        status = VX_ERROR_INVALID_TYPE;
                    }
                }
            }

            if(status == VX_SUCCESS)
            {
                if (node->parameters[index])
                {
                    if (node->parameters[index]->delay!=NULL) {
                        /* we already have a delay element here */
                        vx_bool res = ownRemoveAssociationToDelay(node->parameters[index], node, index);
                        if (res == vx_false_e) {
                            VX_PRINT(VX_ZONE_ERROR, "Internal error removing delay association\n");
                            status = VX_ERROR_INVALID_REFERENCE;
                        }
                    }
                }
            }

            if(status == VX_SUCCESS)
            {
                if (value->delay!=NULL) {
                    /* the new parameter is a delay element */
                    vx_bool res = ownAddAssociationToDelay(value, node, index);
                    if (res == vx_false_e) {
                        VX_PRINT(VX_ZONE_ERROR, "Internal error adding delay association\n");
                        status = VX_ERROR_INVALID_REFERENCE;
                    }
                }
            }

            if(status == VX_SUCCESS)
            {
                ownNodeSetParameter(node, index, value);
            }

            /* Note that we don't need to do anything special for parameters to child graphs. */
        }
    }

    if (status == VX_SUCCESS)
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
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetParameterByReference(vx_parameter parameter, vx_reference value)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (ownIsValidSpecificReference((vx_reference)parameter, VX_TYPE_PARAMETER) == vx_true_e)
    {
        if (parameter->node)
        {
            status = vxSetParameterByIndex(parameter->node, parameter->index, value);
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryParameter(vx_parameter parameter, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (ownIsValidSpecificReference(&parameter->base, VX_TYPE_PARAMETER) == vx_true_e)
    {
        switch (attribute)
        {
            case VX_PARAMETER_DIRECTION:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = parameter->kernel->signature.directions[parameter->index];
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryParameter: Query parameter direction failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_PARAMETER_INDEX:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = parameter->index;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryParameter: Query parameter index failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_PARAMETER_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = parameter->kernel->signature.types[parameter->index];
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryParameter: Query parameter type failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_PARAMETER_STATE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = (vx_enum)parameter->kernel->signature.states[parameter->index];
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryParameter: Query parameter state failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_PARAMETER_REF:
                if (VX_CHECK_PARAM(ptr, size, vx_reference, 0x3U))
                {
                    if (parameter->node)
                    {
                        vx_reference ref = parameter->node->parameters[parameter->index];
                        /* does this object have USER access? */
                        if (ref)
                        {
                            /*! \internal this could potentially allow the user to break
                             * a currently chosen optimization! We need to alert the
                             * system that if a write occurs to this data, put the graph
                             * into an unverified state.
                             */
                            ownIncrementReference(ref, VX_EXTERNAL);
                        }
                        *(vx_reference *)ptr = (vx_reference)ref;
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR,"vxQueryParameter: Query parameter reference failed\n");
                        status = VX_ERROR_NOT_SUPPORTED;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryParameter: Query parameter reference failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR,"vxQueryParameter: Invalid attribute\n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"vxQueryParameter: Invalid reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}


