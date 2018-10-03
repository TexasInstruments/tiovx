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

static vx_status ownDestructLut(vx_reference ref);
static vx_status ownAllocLutBuffer(vx_reference ref);


VX_API_ENTRY vx_status VX_API_CALL vxReleaseLUT(vx_lut *lut)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)lut, VX_TYPE_LUT, VX_EXTERNAL, NULL));
}

vx_lut VX_API_CALL vxCreateLUT(
    vx_context context, vx_enum data_type, vx_size count)
{
    vx_lut lut = NULL;
    vx_size dim = 0;
    tivx_obj_desc_lut_t *obj_desc = NULL;

    if(ownIsValidContext(context) == vx_true_e)
    {
        if ((data_type == VX_TYPE_INT8) || (data_type == VX_TYPE_UINT8) || (data_type == VX_TYPE_CHAR))
        {
            if (count <= 256)
            {
                dim = sizeof(vx_uint8);
            }
        }
        else if ((data_type == VX_TYPE_INT16) || (data_type == VX_TYPE_UINT16))
        {
            if (count <= 65536)
            {
                dim = sizeof(vx_uint16);
            }
        }
        else if ((data_type == VX_TYPE_FLOAT32) || (data_type == VX_TYPE_UINT32) || (data_type == VX_TYPE_INT32))
        {
            dim = 4u; /* 4 bytes per entry for float32, int32, uint32 */
        }
        else if ((data_type == VX_TYPE_FLOAT64) || (data_type == VX_TYPE_UINT64) || (data_type == VX_TYPE_INT64))
        {
            dim = 8u; /* 8 bytes per entry for float64, int64, uint64 */
        }
        else
        {
            dim = 0;
        }

        if (0 != dim)
        {
            lut = (vx_lut)ownCreateReference(context, VX_TYPE_LUT,
                VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)lut) == VX_SUCCESS) &&
                (lut->base.type == VX_TYPE_LUT))
            {
                /* assign refernce type specific callback's */
                lut->base.destructor_callback = &ownDestructLut;
                lut->base.mem_alloc_callback = &ownAllocLutBuffer;
                lut->base.release_callback =
                    (tivx_reference_release_callback_f)&vxReleaseLUT;

                obj_desc = (tivx_obj_desc_lut_t*)tivxObjDescAlloc(
                    TIVX_OBJ_DESC_LUT, (vx_reference)lut);
                if(obj_desc==NULL)
                {
                    vxReleaseLUT(&lut);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                        "Could not allocate lut object descriptor\n");
                    lut = (vx_lut)ownGetErrorObject(
                        context, VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    obj_desc->item_type = data_type;
                    obj_desc->item_size = dim;
                    obj_desc->num_items = count;
                    obj_desc->mem_size = dim * count;
                    obj_desc->mem_ptr.host_ptr = (uint64_t)NULL;
                    obj_desc->mem_ptr.shared_ptr = (uint64_t)NULL;
                    obj_desc->mem_ptr.mem_heap_region = TIVX_MEM_EXTERNAL;
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
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_lut_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&lut->base, VX_TYPE_LUT) == vx_false_e)
        ||
        (lut->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryLUT: Invalid LUT reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_lut_t *)lut->base.obj_desc;
        switch (attribute)
        {
            case VX_LUT_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = obj_desc->item_type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryLUT: Query LUT type failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_LUT_COUNT:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->num_items;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryLUT: Query LUT count failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_LUT_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->mem_size;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryLUT: Query LUT size failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_LUT_OFFSET:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    if ( (VX_TYPE_UINT8 == obj_desc->item_type) ||
                         (VX_TYPE_UINT16 == obj_desc->item_type) ||
                         (VX_TYPE_UINT32 == obj_desc->item_type) ||
                         (VX_TYPE_UINT64 == obj_desc->item_type) )
                    {
                        *(vx_uint32 *)ptr = 0;
                    }
                    else if ( (VX_TYPE_CHAR == obj_desc->item_type) ||
                              (VX_TYPE_INT8 == obj_desc->item_type) ||
                              (VX_TYPE_INT16 == obj_desc->item_type) ||
                              (VX_TYPE_INT32 == obj_desc->item_type) ||
                              (VX_TYPE_INT64 == obj_desc->item_type) ||
                              (VX_TYPE_FLOAT32 == obj_desc->item_type) ||
                              (VX_TYPE_FLOAT64 == obj_desc->item_type) )
                    {
                        *(vx_uint32 *)ptr = (vx_uint32)(obj_desc->num_items/2);
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "vxQueryLUT: Query LUT offset failed. LUT has invalid type\n");
                        status = VX_ERROR_INVALID_PARAMETERS;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryLUT: Query LUT offset failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "vxQueryLUT: Invalid query attribute\n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

vx_status VX_API_CALL vxCopyLUT(
    vx_lut lut, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 size;
    tivx_obj_desc_lut_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&lut->base, VX_TYPE_LUT) == vx_false_e)
        ||
        (lut->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCopyLUT: Invalid LUT reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_lut_t *)lut->base.obj_desc;
        if (VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyLUT: User mem type is not equal to VX_MEMORY_TYPE_HOST\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Memory still not allocated */
        if ((VX_READ_ONLY == usage) &&
            ((uint64_t)NULL == obj_desc->mem_ptr.host_ptr))
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyLUT: Memory is not allocated\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if (NULL == user_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyLUT: User pointer is NULL\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        size = obj_desc->mem_size;

        /* Copy from lut object to user memory */
        if (VX_READ_ONLY == usage)
        {
            tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

            memcpy(user_ptr, (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size);

            tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        else /* Copy from user memory to lut object */
        {
            status = ownAllocLutBuffer(&lut->base);

            if (VX_SUCCESS == status)
            {
                tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                    VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

                memcpy((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, user_ptr, size);

                tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                    VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }
        }
    }

    return (status);
}

vx_status VX_API_CALL vxMapLUT(
    vx_lut lut, vx_map_id *map_id, void **ptr, vx_enum usage, vx_enum mem_type,
    vx_bitfield flags)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_lut_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&lut->base, VX_TYPE_LUT) == vx_false_e)
        ||
        (lut->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "vxMapLUT: Invalid LUT reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        status = ownAllocLutBuffer(&lut->base);
        if ((NULL != ptr) && (VX_SUCCESS == status))
        {
            obj_desc = (tivx_obj_desc_lut_t *)lut->base.obj_desc;
            tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr,
                obj_desc->mem_size, VX_MEMORY_TYPE_HOST,
                VX_READ_AND_WRITE);

            *ptr = (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
        }
    }

    return (status);
}

vx_status VX_API_CALL vxUnmapLUT(vx_lut lut, vx_map_id map_id)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_lut_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&lut->base, VX_TYPE_LUT) == vx_false_e)
        ||
        (lut->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "vxUnmapLUT: Invalid LUT reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_lut_t *)lut->base.obj_desc;
        tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr,
            obj_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_AND_WRITE);
    }

    return (status);
}

static vx_status ownAllocLutBuffer(vx_reference ref)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_lut_t *obj_desc = NULL;

    if(ref->type == VX_TYPE_LUT)
    {
        obj_desc = (tivx_obj_desc_lut_t *)ref->obj_desc;
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
                    VX_PRINT(VX_ZONE_ERROR, "ownAllocLutBuffer: could not allocate memory\n");
                    status = VX_ERROR_NO_MEMORY ;
                }
                else
                {
                    obj_desc->mem_ptr.shared_ptr =
                        tivxMemHost2SharedPtr(
                            obj_desc->mem_ptr.host_ptr,
                            TIVX_MEM_EXTERNAL);
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "ownAllocLutBuffer: object descriptor is NULL\n");
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "ownAllocLutBuffer: Invalid LUT reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownDestructLut(vx_reference ref)
{
    tivx_obj_desc_lut_t *obj_desc = NULL;

    if(ref->type == VX_TYPE_LUT)
    {
        obj_desc = (tivx_obj_desc_lut_t *)ref->obj_desc;
        if(obj_desc!=NULL)
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

