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

static vx_status ownDestructArray(vx_reference ref);
static vx_status ownAllocArrayBuffer(vx_reference ref);
static void ownInitArrayObject(
    vx_array arr, vx_enum item_type, vx_size capacity, vx_bool is_virtual);
static vx_size ownGetArrayItemSize(vx_context context, vx_enum item_type);
static vx_bool ownIsValidArrayItemType(vx_context context, vx_enum item_type);

/* Function to get the size of user defined structure */
static vx_size ownGetArrayItemSize(vx_context context, vx_enum item_type)
{
    vx_size res = ownSizeOfEnumType(item_type);
    vx_uint32 i = 0;

    if (res == 0UL)
    {
        for (i = 0; i < TIVX_CONTEXT_MAX_USER_STRUCTS; ++i)
        {
            if (context->user_structs[i].type == item_type)
            {
                res = context->user_structs[i].size;
                break;
            }
        }
    }
    return res;
}

static vx_bool ownIsValidArrayItemType(vx_context context, vx_enum item_type)
{
    vx_bool res = (vx_bool)vx_false_e;

    if (ownGetArrayItemSize(context, item_type) != 0U)
    {
        res = (vx_bool)vx_true_e;
    }

    return res;
}

vx_status ownInitVirtualArray(vx_array arr, vx_enum item_type, vx_size capacity)
{
    vx_status status = (vx_status)VX_FAILURE;

    if ((ownIsValidSpecificReference((vx_reference)arr, (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_true_e)
        &&
        (arr->base.obj_desc != NULL))
    {
        if ((ownIsValidArrayItemType(arr->base.context, item_type) ==
                (vx_bool)vx_true_e) &&
            (capacity > 0U) &&
            ((vx_enum)(vx_enum)VX_TYPE_INVALID != item_type) &&  /* It should not be invalid now */
            ((vx_bool)vx_true_e == arr->base.is_virtual))
        {
            ownInitArrayObject(arr, item_type, capacity, (vx_bool)vx_true_e);

            status = (vx_status)VX_SUCCESS;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"Own init virtual array failed\n");
            if (ownIsValidArrayItemType(arr->base.context, item_type) !=
                    (vx_bool)vx_true_e)
            {
                VX_PRINT(VX_ZONE_ERROR,"Own is valid array item type failed\n");
            }

            if (!(capacity > 0U))
            {
                VX_PRINT(VX_ZONE_ERROR,"Array capacity is not greater than 0\n");
            }

            if ((vx_enum)(vx_enum)VX_TYPE_INVALID == item_type)
            {
                VX_PRINT(VX_ZONE_ERROR,"array item type is invalid\n");
            }

            if ((vx_bool)vx_true_e != (arr->base.is_virtual))
            {
                VX_PRINT(VX_ZONE_ERROR,"array is not virtual\n");
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Own init virtual array failed\n");
        VX_PRINT(VX_ZONE_ERROR,"Reference is invalid or object descriptor is NULL\n");
    }

    return (status);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseArray(vx_array *arr)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)arr, (vx_enum)VX_TYPE_ARRAY, (vx_enum)VX_EXTERNAL, NULL));
}

vx_array VX_API_CALL vxCreateArray(
    vx_context context, vx_enum item_type, vx_size capacity)
{
    vx_array arr = NULL;

    if(ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        if ((capacity > 0U) &&
            ((vx_bool)vx_true_e == ownIsValidArrayItemType(context, item_type)))
        {
            arr = (vx_array)ownCreateReference(context, (vx_enum)VX_TYPE_ARRAY,
                (vx_enum)VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)arr) == (vx_status)VX_SUCCESS) &&
                (arr->base.type == (vx_enum)VX_TYPE_ARRAY))
            {
                /* assign refernce type specific callback's */
                arr->base.destructor_callback = &ownDestructArray;
                arr->base.mem_alloc_callback = &ownAllocArrayBuffer;
                arr->base.release_callback =
                    (tivx_reference_release_callback_f)&vxReleaseArray;

                arr->base.obj_desc = (tivx_obj_desc_t *)ownObjDescAlloc(
                    (vx_enum)TIVX_OBJ_DESC_ARRAY, (vx_reference)arr);
                if(arr->base.obj_desc==NULL)
                {
                    vxReleaseArray(&arr);

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                        "Could not allocate arr object descriptor\n");
                    arr = (vx_array)ownGetErrorObject(
                        context, (vx_status)VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    ownInitArrayObject(arr, item_type, capacity, (vx_bool)vx_false_e);
                }
            }
        }
    }

    return (arr);
}

vx_array VX_API_CALL vxCreateVirtualArray(
    vx_graph graph, vx_enum item_type, vx_size capacity)
{
    vx_array arr = NULL;
    vx_context context;

    if(ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        context = graph->base.context;

        /* capacity can be zero and item_type can be invalid for
           virtual array */

        arr = (vx_array)ownCreateReference(context, (vx_enum)VX_TYPE_ARRAY,
            (vx_enum)VX_EXTERNAL, &context->base);

        if ((vxGetStatus((vx_reference)arr) == (vx_status)VX_SUCCESS) &&
            (arr->base.type == (vx_enum)VX_TYPE_ARRAY))
        {
            /* assign refernce type specific callback's */
            arr->base.destructor_callback = &ownDestructArray;
            arr->base.mem_alloc_callback = &ownAllocArrayBuffer;
            arr->base.release_callback =
                (tivx_reference_release_callback_f)&vxReleaseArray;

            arr->base.obj_desc = (tivx_obj_desc_t*)ownObjDescAlloc(
                (vx_enum)TIVX_OBJ_DESC_ARRAY, (vx_reference)arr);
            if(arr->base.obj_desc==NULL)
            {
                vxReleaseArray(&arr);

                vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                    "Could not allocate arr object descriptor\n");
                arr = (vx_array)ownGetErrorObject(
                    context, (vx_status)VX_ERROR_NO_RESOURCES);
            }
            else
            {
                ownInitArrayObject(arr, item_type, capacity, (vx_bool)vx_true_e);

                ownReferenceSetScope(&arr->base, &graph->base);
            }
        }
    }

    return (arr);
}

vx_status VX_API_CALL vxQueryArray(
    vx_array arr, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_array_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference((vx_reference)arr, (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_false_e)
            || (arr->base.obj_desc == NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"vxQueryArray failed\n");
        VX_PRINT(VX_ZONE_ERROR,"Reference is invalid or object descriptor is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_array_t *)arr->base.obj_desc;
        switch (attribute)
        {
            case (vx_enum)VX_ARRAY_ITEMTYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = obj_desc->item_type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query array item type failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_ARRAY_NUMITEMS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->num_items;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query array num items failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_ARRAY_CAPACITY:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->capacity;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query array capacity failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_ARRAY_ITEMSIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->item_size;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query array item size failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR,"query array option not supported\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

vx_status VX_API_CALL vxAddArrayItems(
    vx_array arr, vx_size count, const void *ptr, vx_size stride)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_size offset, i;
    vx_uint8 *temp_ptr;
    const vx_uint8 *user_ptr = (const vx_uint8 *)ptr;
    tivx_obj_desc_array_t *obj_desc = NULL;

    if (ownIsValidSpecificReference((vx_reference)arr, (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_array_t *)arr->base.obj_desc;
    }

    if (obj_desc == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,"Array object descriptor is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        status = ownAllocArrayBuffer((vx_reference)arr);

        if (obj_desc->capacity == 0U)
        {
            VX_PRINT(VX_ZONE_ERROR,"Array capacity is 0\n");
            /* Array is still not allocated */
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        if (NULL == ptr)
        {
            VX_PRINT(VX_ZONE_ERROR,"Array pointer is NULL\n");
            /* Array is still not allocated */
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        if (obj_desc->capacity < (obj_desc->num_items + count))
        {
            /* Array is full */
            VX_PRINT(VX_ZONE_ERROR,"Array is full\n");
            status = (vx_status)VX_FAILURE;
        }

        if ((stride < obj_desc->item_size) || (stride == 0U))
        {
            VX_PRINT(VX_ZONE_ERROR,"Stride should be >= item_size\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Get the offset to the free memory */
        offset = (vx_size)obj_desc->num_items * (vx_size)obj_desc->item_size;
        temp_ptr = (vx_uint8 *)(uintptr_t)obj_desc->mem_ptr.host_ptr + offset;

        tivxCheckStatus(&status, tivxMemBufferMap(
            (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, obj_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        for (i = 0; i < count; i ++)
        {
            memcpy(temp_ptr, user_ptr, obj_desc->item_size);

            temp_ptr += obj_desc->item_size;
            user_ptr += stride;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(
            (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, obj_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        obj_desc->num_items += (uint32_t)count;
    }

    return (status);
}

vx_status VX_API_CALL vxTruncateArray(vx_array arr, vx_size new_num_items)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_array_t *obj_desc = NULL;

    if (ownIsValidSpecificReference((vx_reference)arr, (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_array_t *)arr->base.obj_desc;
    }

    if ( (obj_desc == NULL) || (obj_desc->mem_ptr.host_ptr == (uint64_t)(uintptr_t)NULL) )
    {
        VX_PRINT(VX_ZONE_ERROR,"Array object descriptor or host pointer is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if (obj_desc->capacity == 0U)
        {
            /* Array is still not allocated */
            VX_PRINT(VX_ZONE_ERROR,"Array is still not allocated\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        if (new_num_items > obj_desc->num_items)
        {
            /* Array is full */
            VX_PRINT(VX_ZONE_ERROR,"Array is full\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        obj_desc->num_items = (uint32_t)new_num_items;
    }

    return (status);
}

vx_status VX_API_CALL vxCopyArrayRange(
    vx_array arr, vx_size range_start, vx_size range_end,
    vx_size stride, void *ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_size i, inst;
    vx_uint8 *temp_ptr, *start_offset;
    vx_uint8 *user_ptr = (vx_uint8 *)ptr;
    tivx_obj_desc_array_t *obj_desc = NULL;

    if (ownIsValidSpecificReference((vx_reference)arr, (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_array_t *)arr->base.obj_desc;
    }

    if ( (obj_desc == NULL) || (obj_desc->mem_ptr.host_ptr == (uint64_t)(uintptr_t)NULL) )
    {
        VX_PRINT(VX_ZONE_ERROR,"Array object descriptor or host pointer is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if (user_ptr == NULL)
        {
            /* Array is still not allocated */
            VX_PRINT(VX_ZONE_ERROR,"Array is still not allocated; user pointer is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        if (obj_desc->capacity == 0U)
        {
            /* Array is still not allocated */
            VX_PRINT(VX_ZONE_ERROR,"Array is still not allocated; capacity is 0\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        if ((stride < obj_desc->item_size) ||
            (stride == 0U))
        {
            /* Array is full */
            VX_PRINT(VX_ZONE_ERROR,"Array is full\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        /* Not of condition */
        if (!((range_end > range_start) &&
              (range_end <= obj_desc->num_items)))
        {
            /* Invalid range */
            VX_PRINT(VX_ZONE_ERROR,"Invalid array range\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        if ( (arr->base.is_virtual == (vx_bool)vx_true_e)
            &&
            (arr->base.is_accessible == (vx_bool)vx_false_e)
            )
        {
            /* cannot be accessed by app */
            VX_PRINT(VX_ZONE_ERROR,"Array cannot be accessed by application\n");
            status = (vx_status)VX_ERROR_OPTIMIZED_AWAY;
        }

        if ((vx_enum)(vx_enum)VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid user_mem_type; must be VX_MEMORY_TYPE_HOST\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Get the offset to the free memory */
        start_offset = (vx_uint8 *)(uintptr_t)obj_desc->mem_ptr.host_ptr +
            (range_start * obj_desc->item_size);
        inst = range_end - range_start;

        /* Copy from arr object to user memory */
        if ((vx_enum)(vx_enum)VX_READ_ONLY == usage)
        {
            tivxCheckStatus(&status, tivxMemBufferMap(start_offset, (uint32_t)inst*obj_desc->item_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            temp_ptr = start_offset;
            for (i = 0; i < inst; i ++)
            {
                memcpy(user_ptr, temp_ptr, obj_desc->item_size);

                temp_ptr += obj_desc->item_size;
                user_ptr += stride;
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(start_offset, (uint32_t)inst*obj_desc->item_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
        else /* Copy from user memory to arr object */
        {
            tivxCheckStatus(&status, tivxMemBufferMap(start_offset, (uint32_t)inst*obj_desc->item_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            temp_ptr = start_offset;
            for (i = 0; i < inst; i ++)
            {
                memcpy(temp_ptr, user_ptr, obj_desc->item_size);

                temp_ptr += obj_desc->item_size;
                user_ptr += stride;
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(start_offset, (uint32_t)inst*obj_desc->item_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
        }
    }

    return (status);
}

vx_status VX_API_CALL vxMapArrayRange(
    vx_array arr, vx_size range_start, vx_size range_end, vx_map_id *map_id,
    vx_size *stride, void **ptr, vx_enum usage, vx_enum mem_type,
    vx_uint32 flags)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 i, inst;
    vx_uint8 *start_offset;
    tivx_obj_desc_array_t *obj_desc = NULL;

    if (ownIsValidSpecificReference((vx_reference)arr, (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_array_t *)arr->base.obj_desc;
    }

    if ( (obj_desc == NULL) || (obj_desc->mem_ptr.host_ptr == (uint64_t)(uintptr_t)NULL) )
    {
        VX_PRINT(VX_ZONE_ERROR,"Array object descriptor or host pointer is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if ( (arr->base.is_virtual == (vx_bool)vx_true_e)
            &&
            (arr->base.is_accessible == (vx_bool)vx_false_e)
            )
        {
            /* cannot be accessed by app */
            VX_PRINT(VX_ZONE_ERROR,"Array cannot be accessed by application\n");
            status = (vx_status)VX_ERROR_OPTIMIZED_AWAY;
        }

        /* Not of condition */
        if (!((range_end > range_start) &&
              (range_end <= obj_desc->num_items)))
        {
            /* Invalid range */
            VX_PRINT(VX_ZONE_ERROR,"Invalid array range\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        if (NULL == stride)
        {
            /* Invalid range */
            VX_PRINT(VX_ZONE_ERROR,"Invalid array range; stride is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        for (i = 0; i < TIVX_ARRAY_MAX_MAPS; i ++)
        {
            if (arr->maps[i].map_addr == NULL)
            {
                break;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (i < TIVX_ARRAY_MAX_MAPS)
        {
            /* Get the offset to the free memory */
            start_offset = (vx_uint8 *)(uintptr_t)obj_desc->mem_ptr.host_ptr +
                (range_start * obj_desc->item_size);
            inst = (uint32_t)range_end - (uint32_t)range_start;

            if ((NULL != ptr) && (NULL != map_id))
            {
                arr->maps[i].map_addr = start_offset;
                arr->maps[i].map_size = (vx_size)inst * (vx_size)obj_desc->item_size;
                arr->maps[i].mem_type = mem_type;
                arr->maps[i].usage = usage;

                tivxCheckStatus(&status, tivxMemBufferMap(start_offset, (uint32_t)arr->maps[i].map_size,
                    mem_type, usage));

                *ptr = (vx_uint8 *)start_offset;
                *stride = obj_desc->item_size;

                *map_id = i;

                ownLogSetResourceUsedValue("TIVX_ARRAY_MAX_MAPS", (uint16_t)i+1U);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "No available array maps\n");
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_ARRAY_MAX_MAPS in tiovx/include/TI/tivx_config.h\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }

    return (status);
}

vx_status VX_API_CALL vxUnmapArrayRange(vx_array arr, vx_map_id map_id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_array_t *obj_desc = NULL;

    if (ownIsValidSpecificReference((vx_reference)arr, (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_array_t *)arr->base.obj_desc;
    }

    if ( (obj_desc == NULL) || (obj_desc->mem_ptr.host_ptr == (uint64_t)(uintptr_t)NULL) )
    {
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if ( (arr->base.is_virtual == (vx_bool)vx_true_e)
            &&
            (arr->base.is_accessible == (vx_bool)vx_false_e)
            )
        {
            /* cannot be accessed by app */
            VX_PRINT(VX_ZONE_ERROR,"Array cannot be accessed by application\n");
            status = (vx_status)VX_ERROR_OPTIMIZED_AWAY;
        }

        if ((map_id >= TIVX_ARRAY_MAX_MAPS) ||
            (arr->maps[map_id].map_addr == NULL) ||
            (arr->maps[map_id].map_size == 0U))
        {
            VX_PRINT(VX_ZONE_ERROR,"Array map is invalid\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, tivxMemBufferUnmap(arr->maps[map_id].map_addr,
            (uint32_t)arr->maps[map_id].map_size, arr->maps[map_id].mem_type,
            arr->maps[map_id].usage));

        arr->maps[map_id].map_addr = NULL;
        arr->maps[map_id].map_size = 0;
    }

    return (status);
}

static vx_status ownAllocArrayBuffer(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_array_t *obj_desc = NULL;

    if(ref->type == (vx_enum)VX_TYPE_ARRAY)
    {
        obj_desc = (tivx_obj_desc_array_t *)ref->obj_desc;

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
                    VX_PRINT(VX_ZONE_ERROR,"Could not allocate array memory\n");
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
            VX_PRINT(VX_ZONE_ERROR,"Array object descriptor is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Reference is not an array type\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownDestructArray(vx_reference ref)
{
    tivx_obj_desc_array_t *obj_desc = NULL;

    if(ref->type == (vx_enum)VX_TYPE_ARRAY)
    {
        obj_desc = (tivx_obj_desc_array_t *)ref->obj_desc;
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


static void ownInitArrayObject(
    vx_array arr, vx_enum item_type, vx_size capacity, vx_bool is_virtual)
{
    vx_uint32 i;
    tivx_obj_desc_array_t *obj_desc = NULL;

    obj_desc = (tivx_obj_desc_array_t *)arr->base.obj_desc;

    obj_desc->item_type = item_type;
    obj_desc->item_size =
        (uint32_t)ownGetArrayItemSize(arr->base.context, item_type);
    obj_desc->num_items = 0;
    obj_desc->capacity = (uint32_t)capacity;

    obj_desc->mem_size =
        obj_desc->item_size * (uint32_t)capacity;
    obj_desc->mem_ptr.host_ptr = (uint64_t)(uintptr_t)NULL;
    obj_desc->mem_ptr.shared_ptr = (uint64_t)(uintptr_t)NULL;
    obj_desc->mem_ptr.mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;

    arr->base.is_virtual = is_virtual;

    for (i = 0; i < TIVX_ARRAY_MAX_MAPS; i ++)
    {
        arr->maps[i].map_addr = NULL;
        arr->maps[i].map_size = 0;
    }
}


