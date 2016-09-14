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

static vx_status ownDestructArray(vx_reference ref);
static vx_status ownAllocArrayBuffer(vx_reference ref);
static void ownInitArrayObject(
    vx_array arr, vx_enum item_type, vx_size capacity, vx_bool is_virtual);

/* Function to get the size of user defined structure */
static vx_size ownGetArrayItemSize(vx_context context, vx_enum item_type)
{
    vx_size res = ownSizeOfEnumType(item_type);
    vx_uint32 i = 0;

    if (res == 0ul)
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
    vx_bool res = vx_false_e;

    if (ownGetArrayItemSize(context, item_type) != 0)
    {
        res = vx_true_e;
    }

    return res;
}

/* TODO: implement virtual array object */
vx_bool ownInitVirtualArray(vx_array arr, vx_enum item_type, vx_size capacity)
{
    vx_bool status = vx_false_e;

    if ((ownIsValidSpecificReference(&arr->base, VX_TYPE_ARRAY) == vx_true_e)
        &&
        (arr->obj_desc != NULL))
    {
        if ((ownIsValidArrayItemType(arr->base.context, item_type) ==
                vx_true_e) &&
            (capacity > 0) &&
            (VX_TYPE_INVALID != item_type) &&  /* It should not be invalid now */
            (vx_true_e == arr->base.is_virtual))
        {
            ownInitArrayObject(arr, item_type, capacity, vx_true_e);

            status = vx_true_e;
        }
    }

    return (status);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseArray(vx_array *arr)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)arr, VX_TYPE_ARRAY, VX_EXTERNAL, NULL));
}

vx_array VX_API_CALL vxCreateArray(
    vx_context context, vx_enum item_type, vx_size capacity)
{
    vx_array arr = NULL;

    if(ownIsValidContext(context) == vx_true_e)
    {
        if ((capacity > 0) &&
            (vx_true_e == ownIsValidArrayItemType(context, item_type)))
        {
            arr = (vx_array)ownCreateReference(context, VX_TYPE_ARRAY,
                VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)arr) == VX_SUCCESS) &&
                (arr->base.type == VX_TYPE_ARRAY))
            {
                /* assign refernce type specific callback's */
                arr->base.destructor_callback = ownDestructArray;
                arr->base.mem_alloc_callback = ownAllocArrayBuffer;
                arr->base.release_callback =
                    (tivx_reference_release_callback_f)vxReleaseArray;

                arr->obj_desc = (tivx_obj_desc_array_t*)tivxObjDescAlloc(
                    TIVX_OBJ_DESC_ARRAY);
                if(arr->obj_desc==NULL)
                {
                    vxReleaseArray(&arr);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                        "Could not allocate arr object descriptor\n");
                    arr = (vx_array)ownGetErrorObject(
                        context, VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    ownInitArrayObject(arr, item_type, capacity, vx_false_e);
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

    if(ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
    {
        context = graph->base.context;

        /* capacity can be zero and item_type can be invalid for
           virtual array */

        arr = (vx_array)ownCreateReference(context, VX_TYPE_ARRAY,
            VX_EXTERNAL, &context->base);

        if ((vxGetStatus((vx_reference)arr) == VX_SUCCESS) &&
            (arr->base.type == VX_TYPE_ARRAY))
        {
            /* assign refernce type specific callback's */
            arr->base.destructor_callback = ownDestructArray;
            arr->base.mem_alloc_callback = ownAllocArrayBuffer;
            arr->base.release_callback =
                (tivx_reference_release_callback_f)vxReleaseArray;

            arr->obj_desc = (tivx_obj_desc_array_t*)tivxObjDescAlloc(
                TIVX_OBJ_DESC_ARRAY);
            if(arr->obj_desc==NULL)
            {
                vxReleaseArray(&arr);

                vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                    "Could not allocate arr object descriptor\n");
                arr = (vx_array)ownGetErrorObject(
                    context, VX_ERROR_NO_RESOURCES);
            }
            else
            {
                ownInitArrayObject(arr, item_type, capacity, vx_true_e);

                arr->base.scope = (vx_reference)graph;
            }
        }

    }

    return (arr);
}

vx_status VX_API_CALL vxQueryArray(
    vx_array arr, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;

    if ((ownIsValidSpecificReference(&arr->base, VX_TYPE_ARRAY) == vx_false_e)
            || (arr->obj_desc == NULL))
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case VX_ARRAY_ITEMTYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = arr->obj_desc->item_type;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_ARRAY_NUMITEMS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = arr->obj_desc->num_items;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_ARRAY_CAPACITY:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = arr->obj_desc->capacity;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_ARRAY_ITEMSIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = arr->obj_desc->item_size;
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

    return status;
}

vx_status VX_API_CALL vxAddArrayItems(
    vx_array arr, vx_size count, const void *ptr, vx_size stride)
{
    vx_status status = VX_SUCCESS;
    vx_size offset, i;
    vx_uint8 *temp_ptr;
    vx_uint8 *user_ptr = (vx_uint8 *)ptr;

    if (ownIsValidSpecificReference(&arr->base, VX_TYPE_ARRAY) == vx_false_e
        ||
        arr->obj_desc == NULL
        ||
        arr->obj_desc->mem_ptr.host_ptr == NULL)
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {

        if ((arr->obj_desc->capacity == 0) ||
            (NULL == ptr))
        {
            /* Array is still not allocated */
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if ((arr->obj_desc->capacity <= (arr->obj_desc->num_items + count)) ||
            (stride < arr->obj_desc->item_size) ||
            (stride == 0))
        {
            /* Array is full */
            status = VX_FAILURE;
        }
    }

    if (status == VX_SUCCESS)
    {
        /* Get the offset to the free memory */
        offset = arr->obj_desc->num_items * arr->obj_desc->item_size;
        temp_ptr = (vx_uint8 *)arr->obj_desc->mem_ptr.host_ptr + offset;

        tivxMemBufferMap(
            arr->obj_desc->mem_ptr.host_ptr, arr->obj_desc->mem_size,
            arr->obj_desc->mem_ptr.mem_type, VX_WRITE_ONLY);

        for (i = 0; i < count; i ++)
        {
            memcpy(temp_ptr, user_ptr, arr->obj_desc->item_size);

            temp_ptr += arr->obj_desc->item_size;
            user_ptr += stride;
        }

        tivxMemBufferUnmap(
            arr->obj_desc->mem_ptr.host_ptr, arr->obj_desc->mem_size,
            arr->obj_desc->mem_ptr.mem_type, VX_WRITE_ONLY);

        arr->obj_desc->num_items += count;
    }

    return (status);
}

vx_status VX_API_CALL vxTruncateArray(vx_array arr, vx_size new_num_items)
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidSpecificReference(&arr->base, VX_TYPE_ARRAY) == vx_false_e
        ||
        arr->obj_desc == NULL
        ||
        arr->obj_desc->mem_ptr.host_ptr == NULL
        )
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if (arr->obj_desc->capacity == 0)
        {
            /* Array is still not allocated */
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if (new_num_items > arr->obj_desc->num_items)
        {
            /* Array is full */
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (status == VX_SUCCESS)
    {
        arr->obj_desc->num_items = new_num_items;
    }

    return (status);
}

vx_status VX_API_CALL vxCopyArrayRange(
    vx_array arr, vx_size range_start, vx_size range_end,
    vx_size stride, void *ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = VX_SUCCESS;
    vx_size i, inst;
    vx_uint8 *temp_ptr, *start_offset;
    vx_uint8 *user_ptr = (vx_uint8 *)ptr;

    if (ownIsValidSpecificReference(&arr->base, VX_TYPE_ARRAY) == vx_false_e
        ||
        arr->obj_desc == NULL
        ||
        arr->obj_desc->mem_ptr.host_ptr == NULL)
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if (user_ptr == NULL)
        {
            /* Array is still not allocated */
            status = VX_ERROR_INVALID_PARAMETERS;
        }
        if (arr->obj_desc->capacity == 0)
        {
            /* Array is still not allocated */
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if ((stride < arr->obj_desc->item_size) ||
            (stride == 0))
        {
            /* Array is full */
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Not of condition */
        if (!((range_end > range_start) &&
              (range_end <= arr->obj_desc->num_items)))
        {
            /* Invalid range */
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if (arr->base.is_virtual == vx_true_e)
        {
            /* cannot be accessed by app */
            status = VX_ERROR_OPTIMIZED_AWAY;
        }

        if (VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (status == VX_SUCCESS)
    {
        /* Get the offset to the free memory */
        start_offset = (vx_uint8 *)arr->obj_desc->mem_ptr.host_ptr +
            (range_start * arr->obj_desc->item_size);
        inst = range_end - range_start;

        /* Copy from arr object to user memory */
        if (VX_READ_ONLY == usage)
        {
            tivxMemBufferMap(start_offset, inst*arr->obj_desc->item_size,
                arr->obj_desc->mem_ptr.mem_type, VX_READ_ONLY);

            temp_ptr = start_offset;
            for (i = 0; i < inst; i ++)
            {
                memcpy(user_ptr, temp_ptr, arr->obj_desc->item_size);

                temp_ptr += arr->obj_desc->item_size;
                user_ptr += stride;
            }

            tivxMemBufferUnmap(start_offset, inst*arr->obj_desc->item_size,
                arr->obj_desc->mem_ptr.mem_type, VX_READ_ONLY);
        }
        else /* Copy from user memory to arr object */
        {
            tivxMemBufferMap(start_offset, inst*arr->obj_desc->item_size,
                arr->obj_desc->mem_ptr.mem_type, VX_WRITE_ONLY);

            temp_ptr = start_offset;
            for (i = 0; i < inst; i ++)
            {
                memcpy(temp_ptr, user_ptr, arr->obj_desc->item_size);

                temp_ptr += arr->obj_desc->item_size;
                user_ptr += stride;
            }

            tivxMemBufferUnmap(start_offset, inst*arr->obj_desc->item_size,
                arr->obj_desc->mem_ptr.mem_type, VX_WRITE_ONLY);
        }

        arr->obj_desc->num_items += inst;
    }

    return (status);
}

vx_status VX_API_CALL vxMapArrayRange(
    vx_array arr, vx_size range_start, vx_size range_end, vx_map_id *map_id,
    vx_size *stride, void **ptr, vx_enum usage, vx_enum mem_type,
    vx_uint32 flags)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i, inst;
    vx_uint8 *start_offset;

    if (ownIsValidSpecificReference(&arr->base, VX_TYPE_LUT) == vx_false_e
        ||
        arr->obj_desc == NULL)
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if (arr->base.is_virtual == vx_true_e)
        {
            /* cannot be accessed by app */
            status = VX_ERROR_OPTIMIZED_AWAY;
        }

        /* Not of condition */
        if (!((range_end > range_start) &&
              (range_end <= arr->obj_desc->num_items)))
        {
            /* Invalid range */
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        for (i = 0; i < TIVX_ARRAY_MAX_MAPS; i ++)
        {
            if (arr->maps[i].map_addr == NULL)
            {
                break;
            }
        }
    }

    if ((VX_SUCCESS == status) && (i < TIVX_ARRAY_MAX_MAPS))
    {
        /* Get the offset to the free memory */
        start_offset = (vx_uint8 *)arr->obj_desc->mem_ptr.host_ptr +
            (range_start * arr->obj_desc->item_size);
        inst = range_end - range_start;

        if ((NULL != ptr) && (NULL != map_id))
        {
            arr->maps[i].map_addr = start_offset;
            arr->maps[i].map_size = inst*arr->obj_desc->item_size;
            arr->maps[i].usage = usage;

            tivxMemBufferMap(start_offset, arr->maps[i].map_size,
                arr->obj_desc->mem_ptr.mem_type, usage);

            *ptr = (vx_uint8 *)start_offset;

            *map_id = i;
        }
    }

    return (status);
}

vx_status VX_API_CALL vxUnmapArrayRange(vx_array arr, vx_map_id map_id)
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidSpecificReference(&arr->base, VX_TYPE_LUT) == vx_false_e
        ||
        arr->obj_desc == NULL)
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if (arr->base.is_virtual == vx_true_e)
        {
            /* cannot be accessed by app */
            status = VX_ERROR_OPTIMIZED_AWAY;
        }
        if ((map_id >= TIVX_IMAGE_MAX_MAPS) ||
            (arr->maps[map_id].map_addr == NULL) ||
            (arr->maps[map_id].map_size == 0))
        {
        status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivxMemBufferUnmap(arr->maps[map_id].map_addr,
            arr->maps[map_id].map_size, arr->obj_desc->mem_ptr.mem_type,
            arr->maps[map_id].usage);

        arr->maps[map_id].map_addr = NULL;
        arr->maps[map_id].map_size = 0;
    }

    return (status);
}

static vx_status ownAllocArrayBuffer(vx_reference ref)
{
    vx_status status = VX_SUCCESS;
    vx_array arr = (vx_array)ref;

    if(arr->base.type == VX_TYPE_ARRAY)
    {
        if(arr->obj_desc != NULL)
        {
            /* memory is not allocated, so allocate it */
            if(arr->obj_desc->mem_ptr.host_ptr == NULL)
            {
                tivxMemBufferAlloc(
                    &arr->obj_desc->mem_ptr, arr->obj_desc->mem_size,
                    TIVX_MEM_EXTERNAL);

                if(arr->obj_desc->mem_ptr.host_ptr==NULL)
                {
                    /* could not allocate memory */
                    status = VX_ERROR_NO_MEMORY ;
                }
            }
        }
        else
        {
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownDestructArray(vx_reference ref)
{
    vx_array arr = (vx_array)ref;

    if(arr->base.type == VX_TYPE_ARRAY)
    {
        if(arr->obj_desc!=NULL)
        {
            if(arr->obj_desc->mem_ptr.host_ptr!=NULL)
            {
                tivxMemBufferFree(
                    &arr->obj_desc->mem_ptr, arr->obj_desc->mem_size);
            }

            tivxObjDescFree((tivx_obj_desc_t**)&arr->obj_desc);
        }
    }
    return VX_SUCCESS;
}


static void ownInitArrayObject(
    vx_array arr, vx_enum item_type, vx_size capacity, vx_bool is_virtual)
{
    vx_uint32 i;

    arr->obj_desc->item_type = item_type;
    arr->obj_desc->item_size =
        ownGetArrayItemSize(arr->base.context, item_type);
    arr->obj_desc->num_items = 0;
    arr->obj_desc->capacity = capacity;

    arr->obj_desc->mem_size =
        arr->obj_desc->item_size * capacity;
    arr->obj_desc->mem_ptr.host_ptr = NULL;
    arr->obj_desc->mem_ptr.shared_ptr = NULL;
    arr->obj_desc->mem_ptr.mem_type = TIVX_MEM_EXTERNAL;

    arr->base.is_virtual = is_virtual;

    for (i = 0; i < TIVX_ARRAY_MAX_MAPS; i ++)
    {
        arr->maps[i].map_addr = NULL;
        arr->maps[i].map_size = 0;
    }
}


