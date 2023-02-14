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

static vx_status ownDestructLut(vx_reference ref);
static vx_status ownAllocLutBuffer(vx_reference ref);


VX_API_ENTRY vx_status VX_API_CALL vxReleaseLUT(vx_lut *lut)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)lut, (vx_enum)VX_TYPE_LUT, (vx_enum)VX_EXTERNAL, NULL));
}

vx_lut VX_API_CALL vxCreateLUT(
    vx_context context, vx_enum data_type, vx_size count)
{
    vx_lut lut = NULL;
    vx_size dim = 0;
    tivx_obj_desc_lut_t *obj_desc = NULL;

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
            lut = (vx_lut)ownCreateReference(context, (vx_enum)VX_TYPE_LUT,
                (vx_enum)VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)lut) == (vx_status)VX_SUCCESS) &&
                (lut->base.type == (vx_enum)VX_TYPE_LUT))
            {
                /* assign refernce type specific callback's */
                lut->base.destructor_callback = &ownDestructLut;
                lut->base.mem_alloc_callback = &ownAllocLutBuffer;
                lut->base.release_callback =
                    (tivx_reference_release_callback_f)&vxReleaseLUT;

                obj_desc = (tivx_obj_desc_lut_t*)ownObjDescAlloc(
                    (vx_enum)TIVX_OBJ_DESC_LUT, (vx_reference)lut);
                if(obj_desc==NULL)
                {
                    vxReleaseLUT(&lut);

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
                    obj_desc->mem_ptr.host_ptr = (uint64_t)(uintptr_t)NULL;
                    obj_desc->mem_ptr.shared_ptr = (uint64_t)(uintptr_t)NULL;
                    obj_desc->mem_ptr.mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;
                    lut->base.obj_desc = (tivx_obj_desc_t *)obj_desc;
                }
            }
        }
    }

    return (lut);
}

vx_status VX_API_CALL vxQueryLUT(
    vx_lut lut, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_lut_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference((vx_reference)lut, (vx_enum)VX_TYPE_LUT) == (vx_bool)vx_false_e)
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

    if ((ownIsValidSpecificReference((vx_reference)lut, (vx_enum)VX_TYPE_LUT) == (vx_bool)vx_false_e)
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
            ((uint64_t)(uintptr_t)NULL == obj_desc->mem_ptr.host_ptr))
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

            memcpy(user_ptr, (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size);

            tivxCheckStatus(&status, tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
        else /* Copy from user memory to lut object */
        {
            status = ownAllocLutBuffer(&lut->base);

            if ((vx_status)VX_SUCCESS == status)
            {
                tivxCheckStatus(&status, tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

                memcpy((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, user_ptr, size);

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

    if ((ownIsValidSpecificReference((vx_reference)lut, (vx_enum)VX_TYPE_LUT) == (vx_bool)vx_false_e)
        ||
        (lut->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid LUT reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        status = ownAllocLutBuffer(&lut->base);
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

    if ((ownIsValidSpecificReference((vx_reference)lut, (vx_enum)VX_TYPE_LUT) == (vx_bool)vx_false_e)
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

static vx_status ownAllocLutBuffer(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_lut_t *obj_desc = NULL;

    if(ref->type == (vx_enum)VX_TYPE_LUT)
    {
        obj_desc = (tivx_obj_desc_lut_t *)ref->obj_desc;
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
                    VX_PRINT(VX_ZONE_ERROR, "could not allocate memory\n");
                    status = (vx_status)VX_ERROR_NO_MEMORY ;
                }
                else
                {
                    obj_desc->mem_ptr.shared_ptr =
                        tivxMemHost2SharedPtr(
                            obj_desc->mem_ptr.host_ptr,
                            (vx_enum)TIVX_MEM_EXTERNAL);
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "object descriptor is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid LUT reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownDestructLut(vx_reference ref)
{
    tivx_obj_desc_lut_t *obj_desc = NULL;

    if(ref->type == (vx_enum)VX_TYPE_LUT)
    {
        obj_desc = (tivx_obj_desc_lut_t *)ref->obj_desc;
        if(obj_desc!=NULL)
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

