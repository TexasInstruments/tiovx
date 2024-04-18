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

static vx_lut ownCreateLUT(vx_reference scope, vx_enum data_type, vx_size count, vx_bool is_virtual);
static vx_status VX_CALLBACK lutKernelCallback(vx_enum kernel_enum, vx_bool validate_only, vx_enum optimization, const vx_reference params[], vx_uint32 num_params);

/* Call back function that handles the copy, swap and move kernels */
static vx_status VX_CALLBACK lutKernelCallback(vx_enum kernel_enum, vx_bool validate_only, vx_enum optimization, const vx_reference params[], vx_uint32 num_params)
{
    vx_status res;
    vx_reference input = (vx_reference)params[0];
    vx_reference output = (vx_reference)params[1];
    if ((vx_bool)vx_true_e == validate_only)
    {
        if ((vx_bool)vx_true_e == tivxIsReferenceMetaFormatEqual(input, output))
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
        switch (kernel_enum)
        {
            case VX_KERNEL_COPY:
                res = ownCopyReferenceGeneric(input, output);
                break;
            case VX_KERNEL_SWAP:    /* Swap and move do exactly the same */
            case VX_KERNEL_MOVE:
                res = ownSwapReferenceGeneric(input, output);
                break;
            default:
                res = (vx_status)VX_ERROR_NOT_SUPPORTED;
        }
    }
    return (res);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseLUT(vx_lut *lut)
{
    return (ownReleaseReferenceInt(
        vxCastRefFromLUTP(lut), (vx_enum)VX_TYPE_LUT, (vx_enum)VX_EXTERNAL, NULL));
}

static vx_lut ownCreateLUT(vx_reference scope, vx_enum data_type, vx_size count, vx_bool is_virtual)
{
    vx_lut lut = NULL;
    vx_reference ref = NULL;
    vx_size dim = 0;
    tivx_obj_desc_lut_t *obj_desc = NULL;
    vx_context context;
	vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference(scope, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        context = vxGetContext(scope);
    }
    else
    {
        context = (vx_context)scope;
    }
    if(ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        if ((data_type == (vx_enum)VX_TYPE_INT8) || (data_type == (vx_enum)VX_TYPE_UINT8) || (data_type == (vx_enum)VX_TYPE_CHAR))
        {
            if (count <= 256U)
            {
                dim = sizeof(vx_uint8);
            }
        }
        else if ((data_type == (vx_enum)VX_TYPE_INT16) || (data_type == (vx_enum)VX_TYPE_UINT16))
        {
            if (count <= 65536U)
            {
                dim = sizeof(vx_uint16);
            }
        }
        else if ((data_type == (vx_enum)VX_TYPE_FLOAT32) || (data_type == (vx_enum)VX_TYPE_UINT32) || (data_type == (vx_enum)VX_TYPE_INT32))
        {
            dim = 4u; /* 4 bytes per entry for float32, int32, uint32 */
        }
        else if ((data_type == (vx_enum)VX_TYPE_FLOAT64) || (data_type == (vx_enum)VX_TYPE_UINT64) || (data_type == (vx_enum)VX_TYPE_INT64))
        {
            dim = 8u; /* 8 bytes per entry for float64, int64, uint64 */
        }
        else
        {
            dim = 0;
        }

        if (0U != dim)
        {
            ref = ownCreateReference(context, (vx_enum)VX_TYPE_LUT,
                (vx_enum)VX_EXTERNAL, &context->base);

            if ((vxGetStatus(ref) == (vx_status)VX_SUCCESS) &&
                (ref->type == (vx_enum)VX_TYPE_LUT))
            {
                /* status set to NULL due to preceding type check */
                lut = vxCastRefAsLUT(ref,NULL);
                /* assign refernce type specific callback's */
                lut->base.destructor_callback = &ownDestructReferenceGeneric;
                lut->base.mem_alloc_callback = &ownAllocReferenceBufferGeneric;
                lut->base.release_callback =
                    &ownReleaseReferenceBufferGeneric;
                lut->base.kernel_callback = &lutKernelCallback;
                obj_desc = (tivx_obj_desc_lut_t*)ownObjDescAlloc(
                    (vx_enum)TIVX_OBJ_DESC_LUT, vxCastRefFromLUT(lut));
                if(obj_desc==NULL)
                {
                    status = vxReleaseLUT(&lut);
                    if((vx_status)VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Failed to release reference to LUT object\n");
                    }

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                        "Could not allocate lut object descriptor\n");
                    lut = (vx_lut)ownGetErrorObject(
                        context, (vx_status)VX_ERROR_NO_RESOURCES);
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate matrix object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
                }
                else
                {
                    obj_desc->item_type = data_type;
                    obj_desc->item_size = (uint32_t)dim;
                    obj_desc->num_items = (uint32_t)count;
                    obj_desc->mem_size = (uint32_t)dim * (uint32_t)count;
                    obj_desc->mem_ptr.host_ptr = (uint64_t)0;
                    obj_desc->mem_ptr.shared_ptr = (uint64_t)0;
                    obj_desc->mem_ptr.mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;
                    lut->base.obj_desc = (tivx_obj_desc_t *)obj_desc;
                }
            }
        }
    }

    return (lut);
}

vx_lut VX_API_CALL vxCreateLUT(vx_context context, vx_enum data_type, vx_size count)
{
    return ownCreateLUT((vx_reference)context, data_type, count, vx_false_e);
}

vx_lut VX_API_CALL vxCreateVirtualLUT(vx_graph graph, vx_enum data_type, vx_size count)
{
    return ownCreateLUT((vx_reference)graph, data_type, count, vx_true_e);
}

vx_status VX_API_CALL vxQueryLUT(
    vx_lut lut, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_lut_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(vxCastRefFromLUT(lut), (vx_enum)VX_TYPE_LUT) == (vx_bool)vx_false_e)
        ||
        (lut->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid LUT reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_lut_t *)lut->base.obj_desc;
        switch (attribute)
        {
            case (vx_enum)VX_LUT_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = obj_desc->item_type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query LUT type failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_LUT_COUNT:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->num_items;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query LUT count failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_LUT_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->mem_size;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query LUT size failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_LUT_OFFSET:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    if ( ((vx_enum)VX_TYPE_UINT8 == obj_desc->item_type) ||
                         ((vx_enum)VX_TYPE_UINT16 == obj_desc->item_type) ||
                         ((vx_enum)VX_TYPE_UINT32 == obj_desc->item_type) ||
                         ((vx_enum)VX_TYPE_UINT64 == obj_desc->item_type) )
                    {
                        *(vx_uint32 *)ptr = 0;
                    }
                    else if ( ((vx_enum)VX_TYPE_CHAR == obj_desc->item_type) ||
                              ((vx_enum)VX_TYPE_INT8 == obj_desc->item_type) ||
                              ((vx_enum)VX_TYPE_INT16 == obj_desc->item_type) ||
                              ((vx_enum)VX_TYPE_INT32 == obj_desc->item_type) ||
                              ((vx_enum)VX_TYPE_INT64 == obj_desc->item_type) ||
                              ((vx_enum)VX_TYPE_FLOAT32 == obj_desc->item_type) ||
                              ((vx_enum)VX_TYPE_FLOAT64 == obj_desc->item_type) )
                    {
                        *(vx_uint32 *)ptr = (vx_uint32)(obj_desc->num_items/2U);
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Query LUT offset failed. LUT has invalid type\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query LUT offset failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid query attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

vx_status VX_API_CALL vxCopyLUT(
    vx_lut lut, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 size;
    tivx_obj_desc_lut_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(vxCastRefFromLUT(lut), (vx_enum)VX_TYPE_LUT) == (vx_bool)vx_false_e)
        ||
        (lut->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid LUT reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_lut_t *)lut->base.obj_desc;
        if ((vx_enum)VX_MEMORY_TYPE_HOST != user_mem_type)
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

        if (NULL == user_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "User pointer is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        size = obj_desc->mem_size;

        /* Copy from lut object to user memory */
        if ((vx_enum)VX_READ_ONLY == usage)
        {
            tivxCheckStatus(&status, tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            (void)memcpy(user_ptr, (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size);

            tivxCheckStatus(&status, tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
        else /* Copy from user memory to lut object */
        {
            status = ownAllocReferenceBufferGeneric(&lut->base);

            if ((vx_status)VX_SUCCESS == status)
            {
                tivxCheckStatus(&status, tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

                (void)memcpy((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, user_ptr, size);

                tivxCheckStatus(&status, tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
            }
        }
    }

    return (status);
}

vx_status VX_API_CALL vxMapLUT(
    vx_lut lut, vx_map_id *map_id, void **ptr, vx_enum usage, vx_enum mem_type,
    vx_bitfield flags)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_lut_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(vxCastRefFromLUT(lut), (vx_enum)VX_TYPE_LUT) == (vx_bool)vx_false_e)
        ||
        (lut->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid LUT reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        status = ownAllocReferenceBufferGeneric(&lut->base);
        if ((NULL != ptr) && ((vx_status)VX_SUCCESS == status))
        {
            obj_desc = (tivx_obj_desc_lut_t *)lut->base.obj_desc;
            tivxCheckStatus(&status, tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr,
                obj_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_AND_WRITE));

            *ptr = (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
        }
    }

    return (status);
}

vx_status VX_API_CALL vxUnmapLUT(vx_lut lut, vx_map_id map_id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_lut_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(vxCastRefFromLUT(lut), (vx_enum)VX_TYPE_LUT) == (vx_bool)vx_false_e)
        ||
        (lut->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid LUT reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_lut_t *)lut->base.obj_desc;
        tivxCheckStatus(&status, tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr,
            obj_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_AND_WRITE));
    }

    return (status);
}
