/*

 * Copyright (c) 2012-2017 The Khronos Group Inc.
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
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
 *
 */


#include <vx_internal.h>

/*==============================================================================
Tensor HELPER FUNCTIONS
=============================================================================*/

static vx_status isTensorSwappable(vx_tensor input, vx_tensor output);
static vx_status moveOrSwapTensor(vx_reference input, vx_reference output);
static vx_status VX_CALLBACK tensorKernelCallback(vx_enum kernel_enum, vx_bool validate_only, const vx_reference input, const vx_reference output);
static vx_bool ownIsValidTensorFormat(vx_enum data_type);
static void ownInitTensorObject(
    vx_tensor tensor, const vx_size* dimensions, vx_size number_of_dimensions, vx_enum data_type, vx_int8 fixed_point_position);
static vx_status ownTensorCheckSizes(const volatile uint32_t *dimensions, const vx_size * view_start, const vx_size * view_end, vx_size number_of_dimensions);
static vx_size ownComputePatchSize (const vx_size * view_start, const vx_size * view_end, vx_size number_of_dimensions);
static vx_status ownDestructTensor(vx_reference ref);
static void ownComputePositionsFromIndex(vx_size idx, const vx_size * start, const vx_size * end,
        const volatile int32_t * tensor_stride, const vx_size * patch_stride,  vx_size number_of_dimensions,
        vx_size * tensor_pos, vx_size * patch_pos);
static vx_uint32 ownComputePatchOffset(vx_size num_dims, const vx_size *dim_coordinate, const volatile int32_t *strides);


/*! \brief check to see if the tensors may be swapped.
 * They must be copyable, and not sub-tensors
*/
static vx_status isTensorSwappable(vx_tensor input, vx_tensor output)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_reference input_ref  = vxCastRefFromTensor(input);
    vx_reference output_ref = vxCastRefFromTensor(output);
    if ((NULL != input->parent) ||
        (NULL != output->parent))
    {
        status =(vx_status) VX_ERROR_NOT_COMPATIBLE;
    }    
    else if ((vx_bool) vx_true_e ==  tivxIsReferenceMetaFormatEqual(input_ref, output_ref))
    {
        tivx_obj_desc_tensor_t * ip_obj_desc = (tivx_obj_desc_tensor_t *)input->base.obj_desc;
        tivx_obj_desc_tensor_t * op_obj_desc = (tivx_obj_desc_tensor_t *)output->base.obj_desc;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TENSOR_UM002
<justification end> */
        if (ip_obj_desc->mem_size != op_obj_desc->mem_size)
        {
           status = (vx_status)VX_ERROR_NOT_SUPPORTED;
       }
/* LDRA_JUSTIFY_END */
    }
    else
    {
        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
    }
    return status;
}

/*! \brief Swap orMove the input tensor to the output
 * Note this is the same as swap if the input is not virtual
 */
static vx_status moveOrSwapTensor(vx_reference input, vx_reference output)
{
    vx_status status =  ownReferenceLock(output);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TENSOR_UBR003
<justification end> */
    if ((vx_status)VX_SUCCESS == status)
    {
        /* swap destructors even if they are generic (identical) for the tensor objects.
           we do it for completeness and to ensure that in case 
           there is later a need of unique destructors
        */
        tivx_reference_callback_f destructor = output->destructor_callback;
        output->destructor_callback = input->destructor_callback;
        input->destructor_callback = (tivx_reference_callback_f)destructor;
        /* swap objects */
        tivx_obj_desc_t *op_obj_desc = output->obj_desc;
        output->obj_desc = input->obj_desc;
        input->obj_desc  = op_obj_desc;
        (void)ownReferenceUnlock(output);
    }
/* LDRA_JUSTIFY_END */
    return status;
}

/*! \brief The kernel operations function for vx_tensor
 * Handles both validation and kernel function
 */
static vx_status VX_CALLBACK tensorKernelCallback(vx_enum kernel_enum, vx_bool validate_only, const vx_reference input, const vx_reference output)
{
    vx_status res = (vx_status)VX_ERROR_NOT_SUPPORTED;
    vx_status res1 = (vx_status) VX_SUCCESS;
    
    vx_tensor input_tensor  = NULL;
    vx_tensor output_tensor = NULL;
 
    input_tensor  = vxCastRefAsTensor(input, &res);
    output_tensor = vxCastRefAsTensor(output, &res1);

    switch (kernel_enum)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TENSOR_UBR004
<justification end> */
    {
/* LDRA_JUSTIFY_END */
        case (vx_enum)VX_KERNEL_COPY:
            if ((vx_bool)vx_true_e == validate_only)
            {
                if ((vx_bool) vx_true_e ==  tivxIsReferenceMetaFormatEqual(input, output))
                {
                    res = (vx_status)VX_SUCCESS;
                }
                else
                {
                    res = (vx_status)VX_ERROR_NOT_COMPATIBLE;
                }
            }
            else
            {
                res = ownCopyReferenceGeneric((vx_reference)input, (vx_reference)output);
            }
            break;
        case (vx_enum)VX_KERNEL_SWAP:    /* Swap and move do exactly the same */
        case (vx_enum)VX_KERNEL_MOVE:
            if ((vx_bool)vx_true_e == validate_only)
            {
                res =  isTensorSwappable(input_tensor, output_tensor);
            }
            else
            {
                res = moveOrSwapTensor(input, output);
            }
            break;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TENSOR_UM003
<justification end> */
        default:
            res =  (vx_status)VX_ERROR_NOT_SUPPORTED;
            break;
/* LDRA_JUSTIFY_END */
    }
    return (res);
}

static vx_status ownDestructTensor(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_tensor_t *obj_desc = NULL;
    /* look if the tensor was created from image*/
    if ((vx_enum)VX_TYPE_TENSOR == ref->type)
    {
        vx_tensor tensor = vxCastRefAsTensor(ref, NULL);
        obj_desc = (tivx_obj_desc_tensor_t *)ref->obj_desc;
        if (NULL != obj_desc)
        {
            if ((vx_enum)TIVX_TENSOR_NORMAL == (vx_enum)obj_desc->create_type)
            {
                if (obj_desc->mem_ptr.host_ptr != (uint64_t)(uintptr_t)NULL)
                {
                    (void)tivxMemBufferFree(
                        &obj_desc->mem_ptr, obj_desc->mem_size);
                }
            }
            (void)ownObjDescFree((tivx_obj_desc_t **)&obj_desc);
            if (NULL != tensor->parent)
            {
               /* decrement the parent's internal reference count */
               status = ownReleaseReferenceInt(vxCastRefFromImageP(&tensor->parent), tensor->parent->base.type, (vx_enum)VX_INTERNAL, NULL);
               if ((vx_status)VX_SUCCESS != status)
               {
                   VX_PRINT(VX_ZONE_ERROR, "Image parent object (for tensor) release failed!\n");
               }
            }
        }
    }
    else 
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }    

    return status;    
}

/**
 * \brief Check the format of a tensor's contents  (T / F)
 * \param [in] data_type        data_type of tensor elements
 * \return A <tt>\ref vx_bool_e</tt> enumeration.
 */
static vx_bool ownIsValidTensorFormat(vx_enum data_type)
{
    vx_bool res;
    if (
        (data_type == (vx_enum)VX_TYPE_FLOAT32) ||
        (data_type == (vx_enum)VX_TYPE_INT64)   ||
        (data_type == (vx_enum)VX_TYPE_UINT64)  ||
        (data_type == (vx_enum)VX_TYPE_INT32)   ||
        (data_type == (vx_enum)VX_TYPE_UINT32)  ||
        (data_type == (vx_enum)VX_TYPE_INT16)   ||
        (data_type == (vx_enum)VX_TYPE_UINT16)  ||
        (data_type == (vx_enum)VX_TYPE_INT8)    ||
        (data_type == (vx_enum)VX_TYPE_UINT8) )
    {
        res = (vx_bool)vx_true_e;
    }
    else
    {
        res = (vx_bool)vx_false_e;
    }
    return(res);
}

static void ownInitTensorObject(
    vx_tensor tensor, const vx_size* dimensions, vx_size number_of_dimensions, vx_enum data_type, vx_int8 fixed_point_position)
{
    vx_uint32 i;
    tivx_obj_desc_tensor_t *obj_desc = NULL;

    obj_desc = (tivx_obj_desc_tensor_t *)tensor->base.obj_desc;

    obj_desc->data_type = (vx_uint32)data_type;
    obj_desc->fixed_point_position = (vx_uint32)fixed_point_position;
    obj_desc->number_of_dimensions = (vx_uint32)number_of_dimensions;
    obj_desc->scaling_divisor = 1;
    obj_desc->scaling_divisor_fixed_point_position= 0;

    /* Stride 0 is simply sizeof(data_type) */
    obj_desc->dimensions[0] = (vx_uint32)dimensions[0];
    obj_desc->stride[0] = (vx_int32)ownSizeOfEnumType((vx_int32)obj_desc->data_type);

    /* Remaining strides are simple equation */
    for (i = 1; i < number_of_dimensions; i++)
    {
        obj_desc->dimensions[i] = (vx_uint32)dimensions[i];
        obj_desc->stride[i] = obj_desc->stride[i - 1U] * (vx_int32)obj_desc->dimensions[i - 1U];
    }

    obj_desc->mem_size =
        (vx_uint32)obj_desc->stride[number_of_dimensions - 1U] * obj_desc->dimensions[number_of_dimensions - 1U];

    /* Clear the dimensions beyond what the user is creating */
    for (i = (vx_uint32)number_of_dimensions; i < (vx_uint32)TIVX_CONTEXT_MAX_TENSOR_DIMS; i++)
    {
        obj_desc->dimensions[i] = 0;
        obj_desc->stride[i] = 0;
    }

    obj_desc->mem_ptr.host_ptr = (uint64_t)0;
    obj_desc->mem_ptr.shared_ptr = (uint64_t)0;
    obj_desc->mem_ptr.mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;

    for (i = 0; i < TIVX_TENSOR_MAX_MAPS; i ++)
    {
        tensor->maps[i].map_addr = NULL;
        tensor->maps[i].map_size = 0;
    }
    tensor->parent = NULL;
    ((tivx_obj_desc_tensor_t *)tensor->base.obj_desc)->parent_id = (vx_uint16)TIVX_OBJ_DESC_INVALID;
}

static vx_status ownTensorCheckSizes(const volatile uint32_t *dimensions, const vx_size * view_start, const vx_size * view_end, vx_size number_of_dimensions)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_size i;

    for (i = 0; i < number_of_dimensions; i++)
    {
        if ((view_end[i] <= view_start[i]) ||
            (view_end[i] > dimensions[i]))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Invalid view parameter(s) in dimension: %d\n", i);
            break;
        }
    }
    return status;
}


static vx_size ownComputePatchSize (const vx_size * view_start, const vx_size * view_end, vx_size number_of_dimensions)
{
    vx_size total_size = 1;
    vx_size i;
    for (i = 0; i < number_of_dimensions; i++)
    {
        total_size *= view_end[i] - view_start[i];
    }
    return total_size;
}


static void ownComputePositionsFromIndex(vx_size idx, const vx_size * start, const vx_size * end,
        const volatile int32_t * tensor_stride, const vx_size * patch_stride,  vx_size number_of_dimensions,
        vx_size * tensor_pos, vx_size * patch_pos)
{
    *tensor_pos = 0;
    *patch_pos = 0;
    vx_size index_leftover = idx;
    int32_t divisor = 1;
    vx_size i;
    for (i = 0; i < number_of_dimensions; i++)
    {
        divisor = (int32_t)end[i] - (int32_t)start[i];
        vx_size curr_dim_index = (vx_size)index_leftover%(vx_size)divisor;
        *tensor_pos += (vx_size)tensor_stride[i] * (curr_dim_index + start[i]);
        *patch_pos += patch_stride[i] * curr_dim_index ;
        index_leftover = index_leftover / (vx_uint32)divisor;
    }
}

static vx_uint32 ownComputePatchOffset(vx_size num_dims, const vx_size *dim_coordinate, const volatile int32_t *strides)
{
    vx_uint32 offset = 0, i;

    for(i=0; i < num_dims; i++)
    {
        offset += ((vx_uint32)strides[i] * (vx_uint32)dim_coordinate[i]);
    }
    return offset;
}


/*==============================================================================
   Tensor API FUNCTIONS
=============================================================================*/

VX_API_ENTRY vx_tensor VX_API_CALL vxCreateTensor(
    vx_context context,
    vx_size number_of_dims,
    const vx_size * dims,
    vx_enum data_type,
    vx_int8 fixed_point_position)
{
    vx_tensor tensor = NULL;
    vx_reference ref = NULL;
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        if ((number_of_dims < 1U) || ((vx_int32)number_of_dims > TIVX_CONTEXT_MAX_TENSOR_DIMS))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid dims for the tensor.\n");
            status = (vx_status)VX_ERROR_INVALID_DIMENSION;
        }
        if ((vx_bool)vx_false_e == ownIsValidTensorFormat(data_type))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid data_type for the tensor.\n");
            status = (vx_status)VX_ERROR_INVALID_TYPE;
        }

        if( (vx_status)VX_SUCCESS == status )
        {
            ref = ownCreateReference(context, (vx_enum)VX_TYPE_TENSOR, (vx_enum)VX_EXTERNAL, &context->base);

            if ((vxGetStatus(ref) == (vx_status)VX_SUCCESS) &&
                (ref->type == (vx_enum)VX_TYPE_TENSOR))
            {
                /* status set to NULL due to preceding type check */
                tensor = vxCastRefAsTensor(ref, NULL);
                /* assign reference type specific callback's */
                tensor->base.destructor_callback = &ownDestructTensor;
                tensor->base.mem_alloc_callback = &ownAllocReferenceBufferGeneric;
                tensor->base.release_callback =
                   &ownReleaseReferenceBufferGeneric;
                tensor->base.kernel_callback = &tensorKernelCallback;
                tensor->base.obj_desc = (tivx_obj_desc_t *)ownObjDescAlloc(
                    (vx_enum)TIVX_OBJ_DESC_TENSOR, vxCastRefFromTensor(tensor));
                if(tensor->base.obj_desc==NULL)
                {
                    (void)vxReleaseTensor(&tensor);

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                        "Could not allocate tensor object descriptor\n");
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate tensor object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in include/TI/soc/tivx_config_<soc>.h\n");
                }
                else
                {
                    ownInitTensorObject(tensor, dims, number_of_dims, data_type, fixed_point_position);
                }
            }
        }

        if ((vx_status)VX_SUCCESS != status)
        {
            tensor = (vx_tensor)ownGetErrorObject((vx_context)context, status);
        }
    }

    return (tensor);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseTensor(vx_tensor *tensor)
{
    if (tensor != NULL)
    {
        vx_tensor this_tensor = tensor[0];
        vx_reference this_tensor_ref = vxCastRefFromTensor(this_tensor);
        if (ownIsValidSpecificReference(this_tensor_ref, (vx_enum)VX_TYPE_TENSOR) == (vx_bool)vx_true_e)
        {        
            vx_image parent = this_tensor->parent;
            /* clear this tensor from its parent' subtensor list */
            if ((NULL != parent) &&
            (ownIsValidSpecificReference(vxCastRefFromImage(parent), (vx_enum)VX_TYPE_IMAGE) ==
                (vx_bool)vx_true_e) )
            {        
                vx_uint32 subtensor_idx;
                for (subtensor_idx = 0; subtensor_idx < TIVX_IMAGE_MAX_SUBTENSORS; subtensor_idx++)
                {
                    if (parent->subtensors[subtensor_idx] == this_tensor)
                    {
                        parent->subtensors[subtensor_idx] = NULL;
                        break;
                    }
                }
            }
        }
    }
    
    return (ownReleaseReferenceInt(
        vxCastRefFromTensorP(tensor), (vx_enum)VX_TYPE_TENSOR, (vx_enum)VX_EXTERNAL, NULL));
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryTensor(
    vx_tensor tensor, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_tensor_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(vxCastRefFromTensor(tensor), (vx_enum)VX_TYPE_TENSOR) == (vx_bool)vx_false_e)
            || (tensor->base.obj_desc == NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"vxQueryTensor failed\n");
        VX_PRINT(VX_ZONE_ERROR,"Reference is invalid or object descriptor is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_tensor_t *)tensor->base.obj_desc;
        switch (attribute)
        {
            case (vx_enum)VX_TENSOR_NUMBER_OF_DIMS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->number_of_dimensions;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query VX_TENSOR_NUMBER_OF_DIMS failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_TENSOR_DIMS:
                if ((NULL != ptr) &&
                    (size >= ((sizeof(vx_size)*obj_desc->number_of_dimensions))) &&
                    (((vx_size)ptr & 0x3U) == 0U))
                {
                    int32_t i;
                    vx_size *p = ptr;

                    /* Use 'for' loop instead of memcpy since interface type size is different from obj_desc size */
                    for(i=0; i<(int32_t)obj_desc->number_of_dimensions; i++)
                    {
                        p[i] = obj_desc->dimensions[i];
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query VX_TENSOR_DIMS failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_TENSOR_DATA_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = (vx_enum)obj_desc->data_type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query VX_TENSOR_DATA_TYPE failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_TENSOR_FIXED_POINT_POSITION:
                if (VX_CHECK_PARAM(ptr, size, vx_int8, 0x0U))
                {
                    *(vx_int8 *)ptr = (vx_int8)obj_desc->fixed_point_position;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query VX_TENSOR_FIXED_POINT_POSITION failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_TENSOR_SCALING_DIVISOR:
                if (VX_CHECK_PARAM(ptr, size, vx_uint8, 0x0U))
                {
                    *(vx_uint8 *)ptr = (vx_uint8)obj_desc->scaling_divisor;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query TIVX_TENSOR_SCALING_DIVISOR failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_TENSOR_SCALING_DIVISOR_FIXED_POINT_POSITION:
                if (VX_CHECK_PARAM(ptr, size, vx_uint8, 0x0U))
                {
                    *(vx_uint8 *)ptr = (vx_uint8)obj_desc->scaling_divisor_fixed_point_position;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query TIVX_TENSOR_SCALING_DIVISOR_FIXED_POINT_POSITION failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_TENSOR_STRIDES:
                if ((NULL != ptr) &&
                    (size >= ((sizeof(vx_size)*obj_desc->number_of_dimensions))) &&
                    (((vx_size)ptr & 0x3U) == 0U))
                {
                    int32_t i;
                    vx_size *p = ptr;

                    /* Use 'for' loop instead of memcpy since interface type size is different from obj_desc size */
                    for(i=0; i<(int32_t)obj_desc->number_of_dimensions; i++)
                    {
                        p[i] = (vx_size)obj_desc->stride[i];
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query TIVX_TENSOR_STRIDES failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR,"query tensor option not supported\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetTensorAttribute(
    vx_tensor tensor, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_tensor_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(vxCastRefFromTensor(tensor), (vx_enum)VX_TYPE_TENSOR) == (vx_bool)vx_false_e)
        ||
        (tensor->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_tensor_t *)tensor->base.obj_desc;
        switch (attribute)
        {
            case (vx_enum)VX_TENSOR_FIXED_POINT_POSITION:
                if (VX_CHECK_PARAM(ptr, size, vx_int8, 0x0U))
                {
                    obj_desc->fixed_point_position = (vx_uint32)*(const vx_int8 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Set fixed point position failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_TENSOR_SCALING_DIVISOR:
                if (VX_CHECK_PARAM(ptr, size, vx_uint8, 0x0U))
                {
                    obj_desc->scaling_divisor = *(const vx_uint8 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Set scaling divisor failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_TENSOR_SCALING_DIVISOR_FIXED_POINT_POSITION:
                if (VX_CHECK_PARAM(ptr, size, vx_uint8, 0x0U))
                {
                    obj_desc->scaling_divisor_fixed_point_position = *(const vx_uint8 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Set scaling divisor's fixed point position failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxCopyTensorPatch(vx_tensor tensor,
        vx_size number_of_dimensions, const vx_size * view_start, const vx_size * view_end,
        const vx_size * user_stride, void * user_ptr, vx_enum usage, vx_enum user_memory_type)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_tensor_t *obj_desc = NULL;
    vx_size i = 0;

    if ((ownIsValidSpecificReference(vxCastRefFromTensor(tensor), (vx_enum)VX_TYPE_TENSOR) == (vx_bool)vx_false_e) ||
        (tensor->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid tensor reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_tensor_t *)tensor->base.obj_desc;

        if ((vx_enum)VX_MEMORY_TYPE_HOST != user_memory_type)
        {
            VX_PRINT(VX_ZONE_ERROR, "User mem type is not equal to VX_MEMORY_TYPE_HOST\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        /* Memory still not allocated */
        if (((vx_enum)VX_READ_ONLY == usage) &&
            ((uint64_t)0 == obj_desc->mem_ptr.host_ptr))
        {
            VX_PRINT(VX_ZONE_ERROR, "Memory is not allocated\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        if ((NULL == view_start) || (NULL == view_end) || (NULL == user_stride) || (NULL == user_ptr))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid NULL pointer\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((obj_desc->number_of_dimensions < number_of_dimensions) || (number_of_dimensions < 1U))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid number of patch dimensions\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        if (ownTensorCheckSizes(obj_desc->dimensions, view_start, view_end, number_of_dimensions) != (vx_status)VX_SUCCESS)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        for (i = 1; i < number_of_dimensions; i++)
        {
            if ((user_stride[i] < (view_end[i-1U] - view_start[i-1U])))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Invalid view parameter(s) in dimension: %d\n", i);
                break;
            }
        }

        if (user_stride[0] != (vx_size)obj_desc->stride[0])
        {
            VX_PRINT(VX_ZONE_ERROR, "user_stride[0] must be equal to sizeof(data_type).\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = ownAllocReferenceBufferGeneric(&tensor->base);
    }

    /* Maps and copies one line at at time */
    if ((vx_status)VX_SUCCESS == status)
    {
        vx_uint8* user_curr_ptr = (vx_uint8*)user_ptr;
        vx_uint8* tensor_ptr = (vx_uint8*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
        vx_size patch_size = ownComputePatchSize (view_start, view_end, number_of_dimensions);
        vx_uint32 elements_per_line = (vx_uint32)view_end[0]-(vx_uint32)view_start[0];
        vx_uint32 bytes_per_line = (vx_uint32)obj_desc->stride[0] * elements_per_line;

        for (i = 0; i < patch_size; i+=elements_per_line) {
            vx_size patch_pos = 0;
            vx_size tensor_pos = 0;

            ownComputePositionsFromIndex(i,view_start, view_end, obj_desc->stride, user_stride, number_of_dimensions, &tensor_pos, &patch_pos);

            /* Copy from tensor object to user memory */
            if ((vx_enum)VX_READ_ONLY == usage)
            {

                tivxCheckStatus(&status, tivxMemBufferMap(&(tensor_ptr[tensor_pos]), (uint32_t)bytes_per_line,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

                (void)memcpy (&(user_curr_ptr[patch_pos]), &(tensor_ptr[tensor_pos]), bytes_per_line);

                tivxCheckStatus(&status, tivxMemBufferUnmap(&(tensor_ptr[tensor_pos]), (uint32_t)bytes_per_line,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            }
            else /* Copy from user memory to tensor object */
            {
                tivxCheckStatus(&status, tivxMemBufferMap(&(tensor_ptr[tensor_pos]), (uint32_t)bytes_per_line,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

                (void)memcpy (&(tensor_ptr[tensor_pos]), &(user_curr_ptr[patch_pos]), bytes_per_line);

                tivxCheckStatus(&status, tivxMemBufferUnmap(&(tensor_ptr[tensor_pos]), (uint32_t)bytes_per_line,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
            }
        }
    }

    return (status);
}

VX_API_ENTRY vx_status VX_API_CALL tivxMapTensorPatch(
    vx_tensor tensor,
    vx_size number_of_dims,
    const vx_size * view_start,
    const vx_size * view_end,
    vx_map_id* map_id,
    vx_size * stride,
    void** ptr,
    vx_enum usage,
    vx_enum memory_type)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_tensor_t *obj_desc = NULL;
    vx_size view_start_map[TIVX_CONTEXT_MAX_TENSOR_DIMS] = {0};
    vx_size view_end_map[TIVX_CONTEXT_MAX_TENSOR_DIMS] = {0};

    if ((ownIsValidSpecificReference(vxCastRefFromTensor(tensor), (vx_enum)VX_TYPE_TENSOR) == (vx_bool)vx_false_e) ||
        (tensor->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid tensor reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        uint32_t i;
        obj_desc = (tivx_obj_desc_tensor_t *)tensor->base.obj_desc;

        if (NULL == ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "User pointer is null\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        if (NULL == view_start)
        {
            for (i = 0; i < number_of_dims; i++)
            {
                view_start_map[i] = 0;
            }
        }
        else
        {
            for (i = 0; i < number_of_dims; i++)
            {
                view_start_map[i] = view_start[i];
            }
        }
        if (NULL == view_end)
        {
            for (i = 0; i < number_of_dims; i++)
            { 
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TENSOR_UBR001
<justification end> */
                if (NULL != obj_desc)
                {
                    view_end_map[i] = obj_desc->dimensions[i];
                }
/* LDRA_JUSTIFY_END */
            }
        }
        else
        {
            for (i = 0; i < number_of_dims; i++)
            {
                view_end_map[i] = view_end[i];
            }
        }
        if (NULL == map_id)
        {
            VX_PRINT(VX_ZONE_ERROR, "Map ID is null\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        status =  ownTensorCheckSizes(obj_desc->dimensions, view_start_map, view_end_map, number_of_dims);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = ownAllocReferenceBufferGeneric(&tensor->base);
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        vx_uint8* map_addr = NULL, *end_addr = NULL, *host_addr = NULL;
        uint32_t map_size = 0;
        uint32_t map_idx;

        map_addr = (vx_uint8*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
        map_size = obj_desc->mem_size;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TENSOR_UBR002
<justification end>*/
        if (NULL != map_addr)
/* LDRA_JUSTIFY_END */        
        {
            uint32_t offset;

            host_addr = map_addr;

            offset = ownComputePatchOffset(number_of_dims, view_start_map, obj_desc->stride);

            map_addr = &map_addr[offset];

            for(map_idx=0; map_idx<TIVX_TENSOR_MAX_MAPS; map_idx++)
            {
                if(tensor->maps[map_idx].map_addr==NULL)
                {
                    tensor->maps[map_idx].map_addr = host_addr;
                    tensor->maps[map_idx].map_size = map_size;
                    tensor->maps[map_idx].mem_type = memory_type;
                    tensor->maps[map_idx].usage = usage;
                    break;
                }
            }
            if(map_idx<TIVX_TENSOR_MAX_MAPS)
            {
                uint32_t i;
                *map_id = map_idx;
                *ptr = map_addr;

                for(i=0; i < number_of_dims; i++)
                {
                    stride[i] = (vx_size)obj_desc->stride[i];
                }

                end_addr = &(host_addr[map_size]);
                map_addr = (vx_uint8*)TIVX_FLOOR((uintptr_t)host_addr, TIVX_DATA_BUFFER_ALIGNMENT);
                end_addr = (vx_uint8*)TIVX_ALIGN((uintptr_t)end_addr, TIVX_DATA_BUFFER_ALIGNMENT);
                uintptr_t temp_map_size0 = (uintptr_t)end_addr - (uintptr_t)host_addr;
                map_size = (vx_uint32)temp_map_size0;
                tivxCheckStatus(&status, tivxMemBufferMap(map_addr, map_size,
                    memory_type, usage));

                ownLogSetResourceUsedValue("TIVX_TENSOR_MAX_MAPS", (vx_uint16)map_idx+1U);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "No available tensor maps\n");
                VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_TENSOR_MAX_MAPS in tiovx/include/TI/tivx_config.h\n");
                status = (vx_status)VX_ERROR_NO_RESOURCES;
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TENSOR_UM001
<justification end>*/
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "could not allocate memory\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
/* LDRA_JUSTIFY_END */
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxUnmapTensorPatch(vx_tensor tensor, vx_map_id map_id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((ownIsValidSpecificReference(vxCastRefFromTensor(tensor), (vx_enum)VX_TYPE_TENSOR) == (vx_bool)vx_false_e) ||
        (tensor->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid tensor reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        if(map_id >= TIVX_TENSOR_MAX_MAPS)
        {
            VX_PRINT(VX_ZONE_ERROR, "map ID is greater than the maximum tensor maps\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        if( (tensor->maps[map_id].map_addr!=NULL)
            &&
            (tensor->maps[map_id].map_size!=0U)
            )
        {
            vx_uint8* map_addr = NULL, *end_addr = NULL;
            uint32_t map_size = 0;

            map_addr = tensor->maps[map_id].map_addr;
            map_size = (vx_uint32)tensor->maps[map_id].map_size;

            end_addr = &(map_addr[map_size]);
            map_addr = (vx_uint8*)TIVX_FLOOR((uintptr_t)map_addr, TIVX_DATA_BUFFER_ALIGNMENT);
            end_addr = (vx_uint8*)TIVX_ALIGN((uintptr_t)end_addr, TIVX_DATA_BUFFER_ALIGNMENT);
            uintptr_t temp_map_size1 = (uintptr_t)end_addr - (uintptr_t)map_addr;
            map_size = (vx_uint32)temp_map_size1;

            tivxCheckStatus(&status, tivxMemBufferUnmap(
                map_addr, map_size,
                tensor->maps[map_id].mem_type,
                tensor->maps[map_id].usage));

            tensor->maps[map_id].map_addr = NULL;
        }
        else
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            if(tensor->maps[map_id].map_addr==NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "map address is null\n");
            }
            if(tensor->maps[map_id].map_size==0U)
            {
                VX_PRINT(VX_ZONE_ERROR, "map size is equal to 0\n");
            }
        }
    }

    return status;
}
