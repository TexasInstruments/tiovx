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

static vx_status ownDestructTensor(vx_reference ref);
static vx_status ownAllocTensorBuffer(vx_reference ref);
static void ownInitTensorObject(
    vx_tensor tensor, const vx_size* dimensions, vx_size number_of_dimensions, vx_enum data_type, vx_int8 fixed_point_position);
static vx_bool ownIsValidTensorFormat(vx_enum data_type, vx_uint8 fixed_point_pos);
static vx_status ownTensorCheckSizes(const volatile uint32_t *dimensions, const vx_size * view_start, const vx_size * view_end, vx_size number_of_dimensions);
static vx_size ownComputePatchSize (const vx_size * view_start, const vx_size * view_end, vx_size number_of_dimensions);
static void ownComputePositionsFromIndex(vx_size index, const vx_size * start, const vx_size * end,
        const volatile uint32_t * tensor_stride, const vx_size * patch_stride,  vx_size number_of_dimensions,
        vx_size * tensor_pos, vx_size * patch_pos);
static vx_uint32 ownComputePatchOffset(vx_size num_dims, const vx_size *dim_coordinate, const volatile uint32_t *strides);

static vx_bool ownIsValidTensorFormat(vx_enum data_type, vx_uint8 fixed_point_pos)
{
    vx_bool res = (vx_bool)vx_false_e;

    if(
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

    return res;
}

static vx_status ownAllocTensorBuffer(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_tensor_t *obj_desc = NULL;

    if(ref->type == (vx_enum)VX_TYPE_TENSOR)
    {
        obj_desc = (tivx_obj_desc_tensor_t *)ref->obj_desc;

        if(obj_desc != NULL)
        {
            /* memory is not allocated, so allocate it */
            if(obj_desc->mem_ptr.host_ptr == (uint64_t)(uintptr_t)NULL)
            {
                tivxMemBufferAlloc(
                    &obj_desc->mem_ptr, obj_desc->mem_size,
                    (vx_enum)TIVX_MEM_EXTERNAL);

                if(obj_desc->mem_ptr.host_ptr==(uint64_t)(uintptr_t)NULL)
                {
                    /* could not allocate memory */
                    VX_PRINT(VX_ZONE_ERROR,"Could not allocate tensor memory\n");
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
                else
                {
                    obj_desc->mem_ptr.shared_ptr = tivxMemHost2SharedPtr(
                        obj_desc->mem_ptr.host_ptr, (vx_enum)TIVX_MEM_EXTERNAL);
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"Tensor object descriptor is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Reference is not an tensor type\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownDestructTensor(vx_reference ref)
{
    tivx_obj_desc_tensor_t *obj_desc = NULL;

    if(ref->type == (vx_enum)VX_TYPE_TENSOR)
    {
        obj_desc = (tivx_obj_desc_tensor_t *)ref->obj_desc;
        if(obj_desc != NULL)
        {
            if(obj_desc->mem_ptr.host_ptr!=(uint64_t)(uintptr_t)NULL)
            {
                tivxMemBufferFree(
                    &obj_desc->mem_ptr, obj_desc->mem_size);
            }

            ownObjDescFree((tivx_obj_desc_t**)&obj_desc);
        }
    }
    return (vx_status)VX_SUCCESS;
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
    obj_desc->stride[0] = (vx_uint32)ownSizeOfEnumType((vx_int32)obj_desc->data_type);

    /* Remaining strides are simple equation */
    for (i = 1; i < number_of_dimensions; i++)
    {
        obj_desc->dimensions[i] = (vx_uint32)dimensions[i];
        obj_desc->stride[i] = obj_desc->stride[i - 1U] * obj_desc->dimensions[i - 1U];
    }

    obj_desc->mem_size =
        obj_desc->stride[number_of_dimensions - 1U] * obj_desc->dimensions[number_of_dimensions - 1U];

    /* Clear the dimensions beyond what the user is creating */
    for (i = (vx_uint32)number_of_dimensions; i < (vx_uint32)TIVX_CONTEXT_MAX_TENSOR_DIMS; i++)
    {
        obj_desc->dimensions[i] = 0;
        obj_desc->stride[i] = 0;
    }

    obj_desc->mem_ptr.host_ptr = (uint64_t)(uintptr_t)NULL;
    obj_desc->mem_ptr.shared_ptr = (uint64_t)(uintptr_t)NULL;
    obj_desc->mem_ptr.mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;

    for (i = 0; i < TIVX_TENSOR_MAX_MAPS; i ++)
    {
        tensor->maps[i].map_addr = NULL;
        tensor->maps[i].map_size = 0;
    }
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


static void ownComputePositionsFromIndex(vx_size index, const vx_size * start, const vx_size * end,
        const volatile uint32_t * tensor_stride, const vx_size * patch_stride,  vx_size number_of_dimensions,
        vx_size * tensor_pos, vx_size * patch_pos)
{
    *tensor_pos = 0;
    *patch_pos = 0;
    vx_size index_leftover = index;
    int32_t divisor = 1;
    vx_size i;
    for (i = 0; i < number_of_dimensions; i++)
    {
        divisor = (int32_t)end[i] - (int32_t)start[i];
        vx_size curr_dim_index = (vx_size)index_leftover%(vx_size)divisor;
        *tensor_pos += tensor_stride[i] * (curr_dim_index + start[i]);
        *patch_pos += patch_stride[i] * curr_dim_index ;
        index_leftover = index_leftover / (vx_uint32)divisor;
    }
}

static vx_uint32 ownComputePatchOffset(vx_size num_dims, const vx_size *dim_coordinate, const volatile uint32_t *strides)
{
    vx_uint32 offset = 0, i;

    for(i=0; i < num_dims; i++)
    {
        offset += (strides[i] * (vx_uint32)dim_coordinate[i]);
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
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        if ((number_of_dims < 1U) || ((vx_int32)number_of_dims > TIVX_CONTEXT_MAX_TENSOR_DIMS))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid dims for the tensor.\n");
            status = (vx_status)VX_ERROR_INVALID_DIMENSION;
        }

        if ((vx_bool)vx_false_e == ownIsValidTensorFormat(data_type, (vx_uint8)fixed_point_position))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid data_type for the tensor.\n");
            status = (vx_status)VX_ERROR_INVALID_TYPE;
        }

        if( (vx_status)VX_SUCCESS == status )
        {
            tensor = (vx_tensor)ownCreateReference(context, (vx_enum)VX_TYPE_TENSOR, (vx_enum)VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)tensor) == (vx_status)VX_SUCCESS) &&
                (tensor->base.type == (vx_enum)VX_TYPE_TENSOR))
            {
                /* assign reference type specific callback's */
                tensor->base.destructor_callback = &ownDestructTensor;
                tensor->base.mem_alloc_callback = &ownAllocTensorBuffer;
                tensor->base.release_callback =
                    (tivx_reference_release_callback_f)&vxReleaseTensor;

                tensor->base.obj_desc = (tivx_obj_desc_t *)ownObjDescAlloc(
                    (vx_enum)TIVX_OBJ_DESC_TENSOR, (vx_reference)tensor);
                if(tensor->base.obj_desc==NULL)
                {
                    vxReleaseTensor(&tensor);

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                        "Could not allocate tensor object descriptor\n");
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate tensor object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
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
    return (ownReleaseReferenceInt(
        (vx_reference*)tensor, (vx_enum)VX_TYPE_TENSOR, (vx_enum)VX_EXTERNAL, NULL));
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryTensor(
    vx_tensor tensor, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_tensor_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference((vx_reference)tensor, (vx_enum)VX_TYPE_TENSOR) == (vx_bool)vx_false_e)
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
                        p[i] = obj_desc->stride[i];
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

    if ((ownIsValidSpecificReference((vx_reference)tensor, (vx_enum)VX_TYPE_TENSOR) == (vx_bool)vx_false_e)
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

    if ((ownIsValidSpecificReference((vx_reference)tensor, (vx_enum)VX_TYPE_TENSOR) == (vx_bool)vx_false_e) ||
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
            ((uint64_t)(uintptr_t)NULL == obj_desc->mem_ptr.host_ptr))
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

        if (user_stride[0] != obj_desc->stride[0])
        {
            VX_PRINT(VX_ZONE_ERROR, "user_stride[0] must be equal to sizeof(data_type).\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = ownAllocTensorBuffer(&tensor->base);
    }

    /* Maps and copies one line at at time */
    if ((vx_status)VX_SUCCESS == status)
    {
        vx_uint8* user_curr_ptr = (vx_uint8*)user_ptr;
        vx_uint8* tensor_ptr = (vx_uint8*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
        vx_size patch_size = ownComputePatchSize (view_start, view_end, number_of_dimensions);
        vx_uint32 elements_per_line = (vx_uint32)view_end[0]-(vx_uint32)view_start[0];
        vx_uint32 bytes_per_line = obj_desc->stride[0] * elements_per_line;

        for (i = 0; i < patch_size; i+=elements_per_line) {
            vx_size patch_pos = 0;
            vx_size tensor_pos = 0;

            ownComputePositionsFromIndex(i,view_start, view_end, obj_desc->stride, user_stride, number_of_dimensions, &tensor_pos, &patch_pos);

            /* Copy from tensor object to user memory */
            if ((vx_enum)VX_READ_ONLY == usage)
            {
                tivxCheckStatus(&status, tivxMemBufferMap(tensor_ptr + tensor_pos, (uint32_t)bytes_per_line,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

                memcpy (user_curr_ptr + patch_pos, tensor_ptr + tensor_pos, bytes_per_line);

                tivxCheckStatus(&status, tivxMemBufferUnmap(tensor_ptr + tensor_pos, (uint32_t)bytes_per_line,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            }
            else /* Copy from user memory to tensor object */
            {
                tivxCheckStatus(&status, tivxMemBufferMap(tensor_ptr + tensor_pos, (uint32_t)bytes_per_line,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

                memcpy (tensor_ptr + tensor_pos, user_curr_ptr + patch_pos, bytes_per_line);

                tivxCheckStatus(&status, tivxMemBufferUnmap(tensor_ptr + tensor_pos, (uint32_t)bytes_per_line,
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

    if ((ownIsValidSpecificReference((vx_reference)tensor, (vx_enum)VX_TYPE_TENSOR) == (vx_bool)vx_false_e) ||
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
                if (NULL != obj_desc)
                {
                    view_end_map[i] = obj_desc->dimensions[i];
                }
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
        status = ownAllocTensorBuffer(&tensor->base);
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        vx_uint8* map_addr = NULL, *end_addr = NULL, *host_addr = NULL;
        uint32_t map_size = 0;
        uint32_t map_idx;

        map_addr = (vx_uint8*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
        map_size = obj_desc->mem_size;

        if (NULL != map_addr)
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
                    stride[i] = obj_desc->stride[i];
                }

                end_addr = host_addr + map_size;
                map_addr = (vx_uint8*)TIVX_FLOOR((uintptr_t)host_addr, 128U);
                end_addr = (vx_uint8*)TIVX_ALIGN((uintptr_t)end_addr, 128U);
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
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "could not allocate memory\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxUnmapTensorPatch(vx_tensor tensor, vx_map_id map_id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((ownIsValidSpecificReference((vx_reference)tensor, (vx_enum)VX_TYPE_TENSOR) == (vx_bool)vx_false_e) ||
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

            end_addr = map_addr + map_size;
            map_addr = (vx_uint8*)TIVX_FLOOR((uintptr_t)map_addr, 128U);
            end_addr = (vx_uint8*)TIVX_ALIGN((uintptr_t)end_addr, 128U);
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
