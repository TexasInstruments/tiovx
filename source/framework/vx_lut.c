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
                    TIVX_OBJ_DESC_LUT);
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
                    obj_desc->mem_ptr.host_ptr = NULL;
                    obj_desc->mem_ptr.shared_ptr = NULL;
                    obj_desc->mem_ptr.mem_type = TIVX_MEM_EXTERNAL;
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
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            /* case VX_LUT_OFFSET ?? */
            default:
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
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_lut_t *)lut->base.obj_desc;
        if (VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Memory still not allocated */
        if ((VX_READ_ONLY == usage) &&
            (NULL == obj_desc->mem_ptr.host_ptr))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if (NULL == user_ptr)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        size = obj_desc->mem_size;

        /* Copy from lut object to user memory */
        if (VX_READ_ONLY == usage)
        {
            tivxMemBufferMap(obj_desc->mem_ptr.host_ptr, size,
                obj_desc->mem_ptr.mem_type, VX_READ_ONLY);

            memcpy(user_ptr, obj_desc->mem_ptr.host_ptr, size);

            tivxMemBufferUnmap(obj_desc->mem_ptr.host_ptr, size,
                obj_desc->mem_ptr.mem_type, VX_READ_ONLY);
        }
        else /* Copy from user memory to lut object */
        {
            status = ownAllocLutBuffer(&lut->base);

            if (VX_SUCCESS == status)
            {
                tivxMemBufferMap(obj_desc->mem_ptr.host_ptr, size,
                    obj_desc->mem_ptr.mem_type, VX_WRITE_ONLY);

                memcpy(obj_desc->mem_ptr.host_ptr, user_ptr, size);

                tivxMemBufferUnmap(obj_desc->mem_ptr.host_ptr, size,
                    obj_desc->mem_ptr.mem_type, VX_WRITE_ONLY);
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
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        status = ownAllocLutBuffer(&lut->base);
        if ((NULL != ptr) && (VX_SUCCESS == status))
        {
            obj_desc = (tivx_obj_desc_lut_t *)lut->base.obj_desc;
            /* TODO: Need to properly set map_id and return it */
            tivxMemBufferMap(obj_desc->mem_ptr.host_ptr,
                obj_desc->mem_size, obj_desc->mem_ptr.mem_type,
                VX_READ_AND_WRITE);

            *ptr = obj_desc->mem_ptr.host_ptr;
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
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_lut_t *)lut->base.obj_desc;
        tivxMemBufferUnmap(obj_desc->mem_ptr.host_ptr,
            obj_desc->mem_size, obj_desc->mem_ptr.mem_type,
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
            if(obj_desc->mem_ptr.host_ptr == NULL)
            {
                tivxMemBufferAlloc(
                    &obj_desc->mem_ptr, obj_desc->mem_size,
                    TIVX_MEM_EXTERNAL);

                if(obj_desc->mem_ptr.host_ptr==NULL)
                {
                    /* could not allocate memory */
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
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
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
            if(obj_desc->mem_ptr.host_ptr!=NULL)
            {
                tivxMemBufferFree(
                    &obj_desc->mem_ptr, obj_desc->mem_size);
            }

            tivxObjDescFree((tivx_obj_desc_t**)&obj_desc);
        }
    }
    return VX_SUCCESS;
}

