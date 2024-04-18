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

static void ownInitArrayObject(
    vx_array arr, vx_enum item_type, vx_size capacity, vx_bool is_virtual);
static vx_size ownGetArrayItemSize(vx_context context, vx_enum item_type);
static vx_bool ownIsValidArrayItemType(vx_context context, vx_enum item_type);
static vx_bool ownIsValidInputAndOutputArrays(vx_array input, vx_array output);
static vx_status isArrayCopyable(vx_array input, vx_array output);
static vx_status VX_CALLBACK arrayKernelCallback(vx_enum kernel_enum, vx_bool validate_only, const vx_reference params[], vx_uint32 num_params);

/*! \brief checks that the input and output references are not the same, and that both are valid arrays
*/
static vx_bool ownIsValidInputAndOutputArrays(vx_array input, vx_array output)
{
    vx_bool res = (vx_bool)vx_false_e;
    if ((input != output) &&
        (ownIsValidSpecificReference(&input->base, (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_true_e) &&
        (input->base.obj_desc != NULL) &&
        (ownIsValidSpecificReference(&output->base, (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_true_e) &&
        (output->base.obj_desc != NULL)
        )
    {
        res = (vx_bool)vx_true_e;
    }
    return res;
}

/*! \brief This function is called to find out if it is OK to copy the input to the output.
 * If the output is virtual, a zero capacity or invalid item type is OK
 * Will also copy metadata from input to output.
 * \returns VX_SUCCESS if it is, otherwise another error code.
 *
 */
static vx_status isArrayCopyable(vx_array input, vx_array output)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_array_t *ip_obj_desc = (tivx_obj_desc_array_t *)input->base.obj_desc;
    tivx_obj_desc_array_t *op_obj_desc = (tivx_obj_desc_array_t *)output->base.obj_desc;
    if ((vx_bool)vx_false_e == ownIsValidInputAndOutputArrays(input, output))
    {
        status = (vx_status)VX_ERROR_NOT_COMPATIBLE;
    }
    else if ((vx_bool)vx_true_e == output->base.is_virtual)
    {
        /* Either output type must be invalid or types must match*/
        if (((vx_enum)VX_TYPE_INVALID != op_obj_desc->item_type) &&
            (ip_obj_desc->item_type != op_obj_desc->item_type))
        {
            status = (vx_status)VX_ERROR_NOT_COMPATIBLE;
        }
        /* Either output capacity must be zero or at least as large as input capacity */
        if ((0U != op_obj_desc->capacity) &&
             (ip_obj_desc->capacity != op_obj_desc->capacity))
        {
            status = (vx_status)VX_ERROR_NOT_COMPATIBLE;
        }
    }
    else if (op_obj_desc->item_type != ip_obj_desc->item_type)
    {
        /* Types must match */
        status = (vx_status)VX_ERROR_NOT_COMPATIBLE;
    }
    else if (op_obj_desc->capacity < ip_obj_desc->capacity)
    {
        /* Output must have sufficient capacity to hold input */
        status = (vx_status)VX_ERROR_NOT_COMPATIBLE;
    }
    else
    {
        op_obj_desc->item_type = ip_obj_desc->item_type;
        op_obj_desc->item_size = ip_obj_desc->item_size;
        op_obj_desc->capacity = ip_obj_desc->capacity;
    }
    return status;
}

/* Call back function that handles the copy, swap and move kernels */
static vx_status VX_CALLBACK arrayKernelCallback(vx_enum kernel_enum, vx_bool validate_only, const vx_reference params[], vx_uint32 num_params)
{
    vx_status res;
    vx_array input  = NULL;
    vx_array output = NULL;

    if (2U != num_params)
    {
        res = (vx_status)VX_ERROR_NOT_SUPPORTED;
    }
    else
    {
        input  = vxCastRefAsArray(params[0], &res);
        output = vxCastRefAsArray(params[1], &res);
        /* do not check the res, as we know they are arrays at that point*/
        switch (kernel_enum)
        {
            case (vx_enum)VX_KERNEL_COPY:
                if ((vx_bool)vx_true_e == validate_only)
                {
                    res =  isArrayCopyable(input, output);
                }
                else
                {
                    res = ownCopyReferenceGeneric(params[0], params[1]);
                }
                break;
            case (vx_enum)VX_KERNEL_SWAP:
            case (vx_enum)VX_KERNEL_MOVE:
                if ((vx_bool)vx_true_e == validate_only)
                {
                    if ((vx_bool)vx_true_e == tivxIsReferenceMetaFormatEqual(params[0], params[1]))
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
                    res = ownSwapReferenceGeneric(params[0], params[1]);
                }
                break;
            default:
                res = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    return(res);
}

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
    if ((ownIsValidSpecificReference(vxCastRefFromArray(arr), (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_true_e)
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
#ifdef LDRA_UNTESTABLE_CODE
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
#endif
    }
#ifdef LDRA_UNTESTABLE_CODE
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Own init virtual array failed\n");
        VX_PRINT(VX_ZONE_ERROR,"Reference is invalid or object descriptor is NULL\n");
    }
#endif

    return (status);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseArray(vx_array *arr)
{
    return (ownReleaseReferenceInt(
        vxCastRefFromArrayP(arr), (vx_enum)VX_TYPE_ARRAY, (vx_enum)VX_EXTERNAL, NULL));
}

VX_API_ENTRY vx_array VX_API_CALL vxCreateArray(
    vx_context context, vx_enum item_type, vx_size capacity)
{
    vx_array arr = NULL;
    vx_reference ref =NULL;
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        if ((capacity > 0U) &&
            ((vx_bool)vx_true_e == ownIsValidArrayItemType(context, item_type)))
        {
            ref = ownCreateReference(context, (vx_enum)VX_TYPE_ARRAY,
                (vx_enum)VX_EXTERNAL, &context->base);

            if ((vxGetStatus(ref) == (vx_status)VX_SUCCESS) &&
                (ref->type == (vx_enum)VX_TYPE_ARRAY))
            {
                /* status set to NULL due to preceding type check */
                arr = vxCastRefAsArray(ref,NULL);
                /* assign refernce type specific callback's */
                arr->base.destructor_callback = &ownDestructReferenceGeneric;
                arr->base.mem_alloc_callback = &ownAllocReferenceBufferGeneric;
                arr->base.release_callback =
                    &ownReleaseReferenceBufferGeneric;
                arr->base.kernel_callback = &arrayKernelCallback;
                arr->base.obj_desc = (tivx_obj_desc_t *)ownObjDescAlloc(
                    (vx_enum)TIVX_OBJ_DESC_ARRAY, vxCastRefFromArray(arr));
                if(arr->base.obj_desc==NULL)
                {
                    status = vxReleaseArray(&arr);
                    if((vx_status)VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Failed to release reference of array object\n");
                    }

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

VX_API_ENTRY vx_array VX_API_CALL vxCreateVirtualArray(
    vx_graph graph, vx_enum item_type, vx_size capacity)
{
    vx_array arr = NULL;
    vx_reference ref = NULL;
    vx_context context;
    vx_status status= (vx_status)VX_SUCCESS;

    if(ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        context = graph->base.context;

        /* capacity can be zero and item_type can be invalid for
           virtual array */

        ref = ownCreateReference(context, (vx_enum)VX_TYPE_ARRAY,
            (vx_enum)VX_EXTERNAL, &context->base);

        if ((vxGetStatus(ref) == (vx_status)VX_SUCCESS) &&
            (ref->type == (vx_enum)VX_TYPE_ARRAY))
        {
            /* status set to NULL due to preceding type check */
            arr = vxCastRefAsArray(ref,NULL);
            /* assign refernce type specific callback's */
            arr->base.destructor_callback = &ownDestructReferenceGeneric;
            arr->base.mem_alloc_callback = &ownAllocReferenceBufferGeneric;
            arr->base.release_callback =
                &ownReleaseReferenceBufferGeneric;
            arr->base.kernel_callback = &arrayKernelCallback;
            arr->base.obj_desc = (tivx_obj_desc_t*)ownObjDescAlloc(
                (vx_enum)TIVX_OBJ_DESC_ARRAY, vxCastRefFromArray(arr));
            if(arr->base.obj_desc==NULL)
            {
                status = vxReleaseArray(&arr);
                if((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Failed to release reference of array object\n");
                }

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

VX_API_ENTRY vx_status VX_API_CALL vxQueryArray(
    vx_array arr, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_array_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(vxCastRefFromArray(arr), (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_false_e)
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

VX_API_ENTRY vx_status VX_API_CALL vxAddArrayItems(
    vx_array arr, vx_size count, const void *ptr, vx_size stride)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_size offset, i;
    vx_uint8 *temp_ptr;
    const vx_uint8 *user_ptr = (const vx_uint8 *)ptr;
    tivx_obj_desc_array_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(vxCastRefFromArray(arr), (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_true_e)
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
        status = ownAllocReferenceBufferGeneric(vxCastRefFromArray(arr));

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
        temp_ptr = &(((vx_uint8 *)(uintptr_t)obj_desc->mem_ptr.host_ptr)[offset]);

        tivxCheckStatus(&status, tivxMemBufferMap(
            (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, obj_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        for (i = 0; i < count; i ++)
        {
            (void)memcpy(temp_ptr, user_ptr, obj_desc->item_size);

            temp_ptr = &(temp_ptr[obj_desc->item_size]);
            user_ptr = &(user_ptr[stride]);
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(
            (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, obj_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        obj_desc->num_items += (uint32_t)count;
    }

    return (status);
}

VX_API_ENTRY vx_status VX_API_CALL vxTruncateArray(vx_array arr, vx_size new_num_items)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_array_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(vxCastRefFromArray(arr), (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_array_t *)arr->base.obj_desc;
    }

    if ( (obj_desc == NULL) || (obj_desc->mem_ptr.host_ptr == (uint64_t)0) )
    {
        VX_PRINT(VX_ZONE_ERROR,"Array object descriptor or host pointer is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
#ifdef LDRA_UNTESTABLE_CODE
        if (obj_desc->capacity == 0U)
        {
            /* Array is still not allocated */
            VX_PRINT(VX_ZONE_ERROR,"Array is still not allocated\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
#endif

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

VX_API_ENTRY vx_status VX_API_CALL vxCopyArrayRange(
    vx_array arr, vx_size range_start, vx_size range_end,
    vx_size stride, void *ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_size i, inst;
    vx_uint8 *temp_ptr, *start_offset;
    vx_uint8 *user_ptr = (vx_uint8 *)ptr;
    tivx_obj_desc_array_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(vxCastRefFromArray(arr), (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_array_t *)arr->base.obj_desc;
    }

    if ( (obj_desc == NULL) || (obj_desc->mem_ptr.host_ptr == (uint64_t)0) )
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

#ifdef LDRA_UNTESTABLE_CODE
        if (obj_desc->capacity == 0U)
        {
            /* Array is still not allocated */
            VX_PRINT(VX_ZONE_ERROR,"Array is still not allocated; capacity is 0\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
#endif

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

#ifdef LDRA_UNTESTABLE_CODE
        if ( (arr->base.is_virtual == (vx_bool)vx_true_e)
            &&
            (arr->base.is_accessible == (vx_bool)vx_false_e)
            )
        {
            /* cannot be accessed by app */
            VX_PRINT(VX_ZONE_ERROR,"Array cannot be accessed by application\n");
            status = (vx_status)VX_ERROR_OPTIMIZED_AWAY;
        }
#endif

        if ((vx_enum)(vx_enum)VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid user_mem_type; must be VX_MEMORY_TYPE_HOST\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Get the offset to the free memory */
        start_offset = &(((vx_uint8 *)(uintptr_t)obj_desc->mem_ptr.host_ptr)[range_start * obj_desc->item_size]);
        inst = range_end - range_start;

        /* Copy from arr object to user memory */
        if ((vx_enum)(vx_enum)VX_READ_ONLY == usage)
        {
            tivxCheckStatus(&status, tivxMemBufferMap(start_offset, (uint32_t)inst*obj_desc->item_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            temp_ptr = start_offset;
            for (i = 0; i < inst; i ++)
            {
                (void)memcpy(user_ptr, temp_ptr, obj_desc->item_size);

                temp_ptr = &(temp_ptr[obj_desc->item_size]);
                user_ptr = &(user_ptr[stride]);
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
                (void)memcpy(temp_ptr, user_ptr, obj_desc->item_size);

                temp_ptr = &(temp_ptr[obj_desc->item_size]);
                user_ptr = &(user_ptr[stride]);
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(start_offset, (uint32_t)inst*obj_desc->item_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
        }
    }

    return (status);
}

VX_API_ENTRY vx_status VX_API_CALL vxMapArrayRange(
    vx_array arr, vx_size range_start, vx_size range_end, vx_map_id *map_id,
    vx_size *stride, void **ptr, vx_enum usage, vx_enum mem_type,
    vx_uint32 flags)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 i, inst;
    vx_uint8 *start_offset;
    tivx_obj_desc_array_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(vxCastRefFromArray(arr), (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_array_t *)arr->base.obj_desc;
    }

    if ( (obj_desc == NULL) || (obj_desc->mem_ptr.host_ptr == (uint64_t)0) )
    {
        VX_PRINT(VX_ZONE_ERROR,"Array object descriptor or host pointer is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
#ifdef LDRA_UNTESTABLE_CODE
        if ( (arr->base.is_virtual == (vx_bool)vx_true_e)
            &&
            (arr->base.is_accessible == (vx_bool)vx_false_e)
            )
        {
            /* cannot be accessed by app */
            VX_PRINT(VX_ZONE_ERROR,"Array cannot be accessed by application\n");
            status = (vx_status)VX_ERROR_OPTIMIZED_AWAY;
        }
#endif

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
            start_offset = &(((vx_uint8 *)(uintptr_t)obj_desc->mem_ptr.host_ptr)[range_start * obj_desc->item_size]);
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

VX_API_ENTRY vx_status VX_API_CALL vxUnmapArrayRange(vx_array arr, vx_map_id map_id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_array_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(vxCastRefFromArray(arr), (vx_enum)VX_TYPE_ARRAY) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_array_t *)arr->base.obj_desc;
    }

    if ( (obj_desc == NULL) || (obj_desc->mem_ptr.host_ptr == (uint64_t)0) )
    {
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
#ifdef LDRA_UNTESTABLE_CODE
        if ( (arr->base.is_virtual == (vx_bool)vx_true_e)
            &&
            (arr->base.is_accessible == (vx_bool)vx_false_e)
            )
        {
            /* cannot be accessed by app */
            VX_PRINT(VX_ZONE_ERROR,"Array cannot be accessed by application\n");
            status = (vx_status)VX_ERROR_OPTIMIZED_AWAY;
        }
#endif

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
    obj_desc->mem_ptr.host_ptr = (uint64_t)0;
    obj_desc->mem_ptr.shared_ptr = (uint64_t)0;
    obj_desc->mem_ptr.mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;

    arr->base.is_virtual = is_virtual;

    for (i = 0; i < TIVX_ARRAY_MAX_MAPS; i ++)
    {
        arr->maps[i].map_addr = NULL;
        arr->maps[i].map_size = 0;
    }
}


