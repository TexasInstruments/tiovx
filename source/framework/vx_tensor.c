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
static vx_status ownTensorCheckSizes(uint32_t *dimensions, const vx_size * view_start, const vx_size * view_end, vx_size number_of_dimensions);
static vx_size ownComputePatchSize (const vx_size * view_start, const vx_size * view_end, vx_size number_of_dimensions);
static void ownComputePositionsFromIndex(vx_size index, const vx_size * start, const vx_size * end,
        const uint32_t * tensor_stride, const vx_size * patch_stride,  vx_size number_of_dimensions,
        vx_size * tensor_pos, vx_size * patch_pos);
static vx_uint32 ownComputePatchOffset(vx_size num_dims, const vx_size *dim_coordinate, uint32_t *strides);

static vx_bool ownIsValidTensorFormat(vx_enum data_type, vx_uint8 fixed_point_pos)
{
    vx_bool res = vx_false_e;

    if(
        (data_type == VX_TYPE_FLOAT32) ||
        (data_type == VX_TYPE_INT32)   ||
        (data_type == VX_TYPE_UINT32)  ||
        (data_type == VX_TYPE_INT16)   ||
        (data_type == VX_TYPE_UINT16)  ||
        (data_type == VX_TYPE_INT8)    ||
        (data_type == VX_TYPE_UINT8) )
    {
        res = vx_true_e;
    }

    return res;
}

static vx_status ownAllocTensorBuffer(vx_reference ref)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_tensor_t *obj_desc = NULL;

    if(ref->type == VX_TYPE_TENSOR)
    {
        obj_desc = (tivx_obj_desc_tensor_t *)ref->obj_desc;

        if(obj_desc != NULL)
        {
            /* memory is not allocated, so allocate it */
            if(obj_desc->mem_ptr.host_ptr == (uint64_t)NULL)
            {
                tivxMemBufferAlloc(
                    &obj_desc->mem_ptr, obj_desc->mem_size,
                    TIVX_MEM_EXTERNAL);

                if(obj_desc->mem_ptr.host_ptr==(uint64_t)NULL)
                {
                    /* could not allocate memory */
                    VX_PRINT(VX_ZONE_ERROR,"Could not allocate tensor memory\n");
                    status = VX_ERROR_NO_MEMORY;
                }
                else
                {
                    obj_desc->mem_ptr.shared_ptr = tivxMemHost2SharedPtr(
                        obj_desc->mem_ptr.host_ptr, TIVX_MEM_EXTERNAL);
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"Tensor object descriptor is NULL\n");
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Reference is not an tensor type\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownDestructTensor(vx_reference ref)
{
    tivx_obj_desc_tensor_t *obj_desc = NULL;

    if(ref->type == VX_TYPE_TENSOR)
    {
        obj_desc = (tivx_obj_desc_tensor_t *)ref->obj_desc;
        if(obj_desc != NULL)
        {
            if(obj_desc->mem_ptr.host_ptr!=(uint64_t)NULL)
            {
                tivxMemBufferFree(
                    &obj_desc->mem_ptr, obj_desc->mem_size);
            }

            tivxObjDescFree((tivx_obj_desc_t**)&obj_desc);
        }
    }
    return VX_SUCCESS;
}


static void ownInitTensorObject(
    vx_tensor tensor, const vx_size* dimensions, vx_size number_of_dimensions, vx_enum data_type, vx_int8 fixed_point_position)
{
    vx_uint32 i;
    tivx_obj_desc_tensor_t *obj_desc = NULL;

    obj_desc = (tivx_obj_desc_tensor_t *)tensor->base.obj_desc;

    obj_desc->data_type = data_type;
    obj_desc->fixed_point_position = fixed_point_position;
    obj_desc->number_of_dimensions = number_of_dimensions;

    /* Stride 0 is simply sizeof(data_type) */
    obj_desc->dimensions[0] = dimensions[0];
    obj_desc->stride[0] = ownSizeOfEnumType(obj_desc->data_type);

    /* Remaining strides are simple equation */
    for (i = 1; i < number_of_dimensions; i++)
    {
        obj_desc->dimensions[i] = dimensions[i];
        obj_desc->stride[i] = obj_desc->stride[i - 1] * obj_desc->dimensions[i - 1];
    }

    obj_desc->mem_size =
        obj_desc->stride[number_of_dimensions - 1] * obj_desc->dimensions[number_of_dimensions - 1];

    /* Clear the dimensions beyond what the user is creating */
    for (i = number_of_dimensions; i < TIVX_CONTEXT_MAX_TENSOR_DIMS; i++)
    {
        obj_desc->dimensions[i] = 0;
        obj_desc->stride[i] = 0;
    }

    obj_desc->mem_ptr.host_ptr = (uint64_t)NULL;
    obj_desc->mem_ptr.shared_ptr = (uint64_t)NULL;
    obj_desc->mem_ptr.mem_heap_region = TIVX_MEM_EXTERNAL;

    for (i = 0; i < TIVX_TENSOR_MAX_MAPS; i ++)
    {
        tensor->maps[i].map_addr = NULL;
        tensor->maps[i].map_size = 0;
    }
}

static vx_status ownTensorCheckSizes(uint32_t *dimensions, const vx_size * view_start, const vx_size * view_end, vx_size number_of_dimensions)
{
    vx_status status = VX_SUCCESS;
    vx_size i;

    for (i = 0; i < number_of_dimensions; i++)
    {
        if ((view_end[i] <= view_start[i]) ||
            (view_end[i] > dimensions[i]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
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
        const uint32_t * tensor_stride, const vx_size * patch_stride,  vx_size number_of_dimensions,
        vx_size * tensor_pos, vx_size * patch_pos)
{
    *tensor_pos = 0;
    *patch_pos = 0;
    vx_size index_leftover = index;
    int divisor = 1;
    vx_size i;
    for (i = 0; i < number_of_dimensions; i++)
    {
        divisor = end[i] - start[i];
        vx_size curr_dim_index = index_leftover%divisor;
        *tensor_pos += tensor_stride[i] * (curr_dim_index + start[i]);
        *patch_pos += patch_stride[i] * curr_dim_index ;
        index_leftover = index_leftover /divisor;
    }
}

static vx_uint32 ownComputePatchOffset(vx_size num_dims, const vx_size *dim_coordinate, uint32_t *strides)
{
    vx_uint32 offset = 0, i;

    for(i=0; i < num_dims; i++)
    {
        offset += (strides[i] * dim_coordinate[i]);
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

    if(ownIsValidContext(context) == vx_true_e)
    {
        if ((number_of_dims < 1) || (number_of_dims > TIVX_CONTEXT_MAX_TENSOR_DIMS))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid dims for the tensor.\n");
            tensor = (vx_tensor)ownGetErrorObject((vx_context)context, VX_ERROR_INVALID_DIMENSION);
        }

        if (vx_false_e == ownIsValidTensorFormat(data_type, fixed_point_position))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid data_type for the tensor.\n");
            tensor =  (vx_tensor)ownGetErrorObject((vx_context)context, VX_ERROR_INVALID_TYPE);
        }

        if( NULL == tensor )
        {
            tensor = (vx_tensor)ownCreateReference(context, VX_TYPE_TENSOR, VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)tensor) == VX_SUCCESS) &&
                (tensor->base.type == VX_TYPE_TENSOR))
            {
                /* assign reference type specific callback's */
                tensor->base.destructor_callback = &ownDestructTensor;
                tensor->base.mem_alloc_callback = &ownAllocTensorBuffer;
                tensor->base.release_callback =
                    (tivx_reference_release_callback_f)&vxReleaseTensor;

                tensor->base.obj_desc = (tivx_obj_desc_t *)tivxObjDescAlloc(
                    TIVX_OBJ_DESC_TENSOR, (vx_reference)tensor);
                if(tensor->base.obj_desc==NULL)
                {
                    vxReleaseTensor(&tensor);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                        "Could not allocate tensor object descriptor\n");
                    tensor = (vx_tensor)ownGetErrorObject(
                        context, VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    ownInitTensorObject(tensor, dims, number_of_dims, data_type, fixed_point_position);
                }
            }
        }
    }

    return (tensor);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseTensor(vx_tensor *tensor)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)tensor, VX_TYPE_TENSOR, VX_EXTERNAL, NULL));
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryTensor(
    vx_tensor tensor, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_tensor_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&tensor->base, VX_TYPE_TENSOR) == vx_false_e)
            || (tensor->base.obj_desc == NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"vxQueryTensor failed\n");
        VX_PRINT(VX_ZONE_ERROR,"Reference is invalid or object descriptor is NULL\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_tensor_t *)tensor->base.obj_desc;
        switch (attribute)
        {
            case VX_TENSOR_NUMBER_OF_DIMS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->number_of_dimensions;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query VX_TENSOR_NUMBER_OF_DIMS failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_TENSOR_DIMS:
                if (size >= (sizeof(vx_size)*obj_desc->number_of_dimensions) && ((vx_size)ptr & 0x3) == 0)
                {
                    int i;
                    vx_size *p = ptr;

                    /* Use 'for' loop instead of memcpy since interface type size is different from obj_desc size */
                    for(i=0; i<obj_desc->number_of_dimensions; i++)
                    {
                        p[i] = obj_desc->dimensions[i];
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query VX_TENSOR_DIMS failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_TENSOR_DATA_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = obj_desc->data_type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query VX_TENSOR_DATA_TYPE failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_TENSOR_FIXED_POINT_POSITION:
                if (VX_CHECK_PARAM(ptr, size, vx_int8, 0x0U))
                {
                    *(vx_int8 *)ptr = obj_desc->fixed_point_position;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query VX_TENSOR_FIXED_POINT_POSITION failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR,"query tensor option not supported\n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxCopyTensorPatch(vx_tensor tensor, 
        vx_size number_of_dimensions, const vx_size * view_start, const vx_size * view_end,
        const vx_size * user_stride, void * user_ptr, vx_enum usage, vx_enum user_memory_type)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_tensor_t *obj_desc = NULL;
    vx_size i = 0;

    if ((ownIsValidSpecificReference(&tensor->base, VX_TYPE_TENSOR) == vx_false_e) ||
        (tensor->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCopyTensorPatch: Invalid tensor reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_tensor_t *)tensor->base.obj_desc;

        if (VX_MEMORY_TYPE_HOST != user_memory_type)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyTensorPatch: User mem type is not equal to VX_MEMORY_TYPE_HOST\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Memory still not allocated */
        if ((VX_READ_ONLY == usage) &&
            ((uint64_t)NULL == obj_desc->mem_ptr.host_ptr))
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyTensorPatch: Memory is not allocated\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if ((NULL == view_start) || (NULL == view_end) || (NULL == user_stride) || (NULL == user_ptr))
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyTensorPatch: Invalid NULL pointer\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
        
    if (VX_SUCCESS == status)
    {
        if ((obj_desc->number_of_dimensions < number_of_dimensions) || (number_of_dimensions < 1U))
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyTensorPatch: Invalid number of patch dimensions\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if (ownTensorCheckSizes(obj_desc->dimensions, view_start, view_end, number_of_dimensions) != VX_SUCCESS) 
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
        
        for (i = 1; i < number_of_dimensions; i++)
        {
            if ((user_stride[i] < (view_end[i-1] - view_start[i-1])))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Invalid view parameter(s) in dimension: %d\n", i);
                break;
            }
        }

        if (user_stride[0] != obj_desc->stride[0]) 
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyTensorPatch: user_stride[0] must be equal to sizeof(data_type).\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        status = ownAllocTensorBuffer(&tensor->base);
    }

    /* Maps and copies one line at at time */
    if (VX_SUCCESS == status)
    {
        vx_uint8* user_curr_ptr = (vx_uint8*)user_ptr;
        vx_uint8* tensor_ptr = (vx_uint8*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
        vx_size patch_size = ownComputePatchSize (view_start, view_end, number_of_dimensions);
        vx_uint32 elements_per_line = view_end[0]-view_start[0];
        vx_uint32 bytes_per_line = obj_desc->stride[0] * elements_per_line;

        for (i = 0; i < patch_size; i+=elements_per_line) {
            vx_size patch_pos = 0;
            vx_size tensor_pos = 0;
            
            ownComputePositionsFromIndex(i,view_start, view_end, obj_desc->stride, user_stride, number_of_dimensions, &tensor_pos, &patch_pos);

            /* Copy from tensor object to user memory */
            if (VX_READ_ONLY == usage)
            {
                tivxMemBufferMap(tensor_ptr + tensor_pos, (uint32_t)bytes_per_line,
                    VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

                memcpy (user_curr_ptr + patch_pos, tensor_ptr + tensor_pos, bytes_per_line);

                tivxMemBufferUnmap(tensor_ptr + tensor_pos, (uint32_t)bytes_per_line,
                    VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            }
            else /* Copy from user memory to tensor object */
            {
                tivxMemBufferMap(tensor_ptr + tensor_pos, (uint32_t)bytes_per_line,
                    VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

                memcpy (tensor_ptr + tensor_pos, user_curr_ptr + patch_pos, bytes_per_line);

                tivxMemBufferUnmap(tensor_ptr + tensor_pos, (uint32_t)bytes_per_line,
                    VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
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
    vx_size * dims,
    vx_size * strides,
    void** user_ptr,
    vx_enum usage,
    vx_enum user_memory_type,
    vx_uint32 flags)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_tensor_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&tensor->base, VX_TYPE_TENSOR) == vx_false_e) ||
        (tensor->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCopyTensorPatch: Invalid tensor reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    if(status == VX_SUCCESS)
    {
        if (user_ptr == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMapTensorPatch: User pointer is null\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
        if ((view_start == NULL) || (view_end == NULL))
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMapTensorPatch: View pointer is null\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
        if (map_id == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMapTensorPatch: Map ID is null\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if(status == VX_SUCCESS)
    {
        obj_desc = (tivx_obj_desc_tensor_t *)tensor->base.obj_desc;

        status =  ownTensorCheckSizes(obj_desc->dimensions, view_start, view_end, number_of_dims); 
    }

    if (VX_SUCCESS == status)
    {
        status = ownAllocTensorBuffer(&tensor->base);
    }

    if(status == VX_SUCCESS)
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
            
            offset = ownComputePatchOffset(number_of_dims, view_start, obj_desc->stride);

            map_addr = &map_addr[offset];

            for(map_idx=0; map_idx<TIVX_TENSOR_MAX_MAPS; map_idx++)
            {
                if(tensor->maps[map_idx].map_addr==NULL)
                {
                    tensor->maps[map_idx].map_addr = host_addr;
                    tensor->maps[map_idx].map_size = map_size;
                    tensor->maps[map_idx].mem_type = user_memory_type;
                    tensor->maps[map_idx].usage = usage;
                    break;
                }
            }
            if(map_idx<TIVX_TENSOR_MAX_MAPS)
            {
                uint32_t i;
                *map_id = map_idx;
                *user_ptr = map_addr;
                
                for(i=0; i < number_of_dims; i++)
                {
                    dims[i] = view_end[i] - view_start[i];
                    strides[i] = obj_desc->stride[i];
                }

                end_addr = host_addr + map_size;
                map_addr = (vx_uint8*)TIVX_FLOOR((uintptr_t)host_addr, 128U);
                end_addr = (vx_uint8*)TIVX_ALIGN((uintptr_t)end_addr, 128U);
                map_size = end_addr - host_addr;
                tivxMemBufferMap(map_addr, map_size,
                    user_memory_type, usage);

                tivxLogSetResourceUsedValue("TIVX_TENSOR_MAX_MAPS", map_idx+1);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxMapTensorPatch: No available tensor maps\n");
                VX_PRINT(VX_ZONE_ERROR, "tivxMapTensorPatch: May need to increase the value of TIVX_TENSOR_MAX_MAPS in tiovx/include/tivx_config.h\n");
                status = VX_ERROR_NO_RESOURCES;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMapTensorPatch: could not allocate memory\n");
            status = VX_ERROR_NO_MEMORY;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxUnmapTensorPatch(vx_tensor tensor, vx_map_id map_id)
{
    vx_status status = VX_SUCCESS;

    if ((ownIsValidSpecificReference(&tensor->base, VX_TYPE_TENSOR) == vx_false_e) ||
        (tensor->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxUnmapTensorPatch: Invalid tensor reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    if(status == VX_SUCCESS)
    {
        if(map_id >= TIVX_TENSOR_MAX_MAPS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxUnmapTensorPatch: map ID is greater than the maximum tensor maps\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if(status == VX_SUCCESS)
    {
        if( (tensor->maps[map_id].map_addr!=NULL)
            &&
            (tensor->maps[map_id].map_size!=0)
            )
        {
            vx_uint8* map_addr = NULL, *end_addr = NULL;
            uint32_t map_size = 0;

            map_addr = tensor->maps[map_id].map_addr;
            map_size = tensor->maps[map_id].map_size;

            end_addr = map_addr + map_size;
            map_addr = (vx_uint8*)TIVX_FLOOR((uintptr_t)map_addr, 128);
            end_addr = (vx_uint8*)TIVX_ALIGN((uintptr_t)end_addr, 128);
            map_size = end_addr - map_addr;

            tivxMemBufferUnmap(
                map_addr, map_size,
                tensor->maps[map_id].mem_type,
                tensor->maps[map_id].usage);

            tensor->maps[map_id].map_addr = NULL;
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            if(tensor->maps[map_id].map_addr==NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxUnmapTensorPatch: map address is null\n");
            }
            if(tensor->maps[map_id].map_size==0)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxUnmapTensorPatch: map size is equal to 0\n");
            }
        }
    }

    return status;
}
