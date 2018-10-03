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

static vx_status ownDestructDistribution(vx_reference ref);
static vx_status ownAllocDistributionBuffer(vx_reference ref);

vx_distribution VX_API_CALL vxCreateDistribution(
    vx_context context, vx_size num_bins, vx_int32 offset, vx_uint32 range)
{
    vx_distribution dist = NULL;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if(ownIsValidContext(context) == vx_true_e)
    {
        if ((0 != num_bins) && (0 != range))
        {
            dist = (vx_distribution)ownCreateReference(context,
                VX_TYPE_DISTRIBUTION, VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)dist) == VX_SUCCESS) &&
                (dist->base.type == VX_TYPE_DISTRIBUTION))
            {
                /* assign refernce type specific callback's */
                dist->base.destructor_callback = &ownDestructDistribution;
                dist->base.mem_alloc_callback = &ownAllocDistributionBuffer;
                dist->base.release_callback =
                    (tivx_reference_release_callback_f)&vxReleaseDistribution;

                obj_desc = (tivx_obj_desc_distribution_t*)tivxObjDescAlloc(
                    TIVX_OBJ_DESC_DISTRIBUTION, (vx_reference)dist);
                if(obj_desc==NULL)
                {
                    vxReleaseDistribution(&dist);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                        "Could not allocate dist object descriptor\n");
                    dist = (vx_distribution)ownGetErrorObject(
                        context, VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    obj_desc->num_bins = num_bins;
                    obj_desc->range = range;
                    obj_desc->offset = offset;
                    obj_desc->num_win = (vx_uint32)range/(vx_uint32)num_bins;
                    obj_desc->mem_size = num_bins * sizeof(vx_int32);
                    obj_desc->mem_ptr.host_ptr = (uint64_t)NULL;
                    obj_desc->mem_ptr.shared_ptr = (uint64_t)NULL;
                    obj_desc->mem_ptr.mem_heap_region = TIVX_MEM_EXTERNAL;
                    dist->base.obj_desc = (tivx_obj_desc_t*)obj_desc;
                }
            }
        }
    }

    return (dist);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseDistribution(vx_distribution *dist)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)dist, VX_TYPE_DISTRIBUTION, VX_EXTERNAL, NULL));
}

vx_status VX_API_CALL vxQueryDistribution(
    vx_distribution dist, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(&dist->base, VX_TYPE_DISTRIBUTION) == vx_true_e)
    {
        obj_desc = (tivx_obj_desc_distribution_t *)dist->base.obj_desc;
    }

    if (obj_desc == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryDistribution: Invalid Object Descriptor! \n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case VX_DISTRIBUTION_DIMENSIONS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    /* Only 1D is supported */
                    *(vx_size *)ptr = 1U;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryDistribution: distribution dimensions query failed \n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_DISTRIBUTION_OFFSET:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U))
                {
                    *(vx_int32 *)ptr = obj_desc->offset;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryDistribution: distribution offset query failed \n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_DISTRIBUTION_RANGE:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->range;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryDistribution: distribution range query failed \n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_DISTRIBUTION_BINS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->num_bins;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryDistribution: distribution bins query failed \n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_DISTRIBUTION_WINDOW:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->num_win;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryDistribution: distribution window query failed \n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_DISTRIBUTION_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->mem_size;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryDistribution: distribution size query failed \n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "vxQueryDistribution: invalid attribute \n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

vx_status VX_API_CALL vxCopyDistribution(
    vx_distribution dist, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 size;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(&dist->base, VX_TYPE_DISTRIBUTION) == vx_true_e)
    {
        obj_desc = (tivx_obj_desc_distribution_t *)dist->base.obj_desc;
    }

    if (obj_desc == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCopyDistribution: Invalid Object Descriptor! \n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if (VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyDistribution: user_mem_type is not VX_MEMORY_TYPE_HOST\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Memory still not allocated */
        if ((VX_READ_ONLY == usage) &&
            ((uint64_t)NULL == obj_desc->mem_ptr.host_ptr))
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyDistribution: Memory still allocated\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if (NULL == user_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyDistribution: user pointer is NULL\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        size = obj_desc->mem_size;

        /* Copy from dist object to user memory */
        if (VX_READ_ONLY == usage)
        {
            tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

            memcpy(user_ptr, (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size);

            tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        else /* Copy from user memory to dist object */
        {
            status = ownAllocDistributionBuffer(&dist->base);

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

vx_status VX_API_CALL vxMapDistribution(
    vx_distribution dist, vx_map_id *map_id, void **ptr, vx_enum usage, vx_enum mem_type,
    vx_bitfield flags)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(&dist->base, VX_TYPE_DISTRIBUTION) == vx_true_e)
    {
        obj_desc = (tivx_obj_desc_distribution_t *)dist->base.obj_desc;
    }

    if ((obj_desc == NULL) || (obj_desc->mem_ptr.host_ptr == (uint64_t)NULL))
    {
        VX_PRINT(VX_ZONE_ERROR, "vxMapDistribution: object descriptor is NULL or host ptr is NULL\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if (NULL != ptr)
        {
            tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr,
                obj_desc->mem_size, VX_MEMORY_TYPE_HOST,
                VX_READ_AND_WRITE);

            *ptr = (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
        }
    }

    return (status);
}

vx_status VX_API_CALL vxUnmapDistribution(vx_distribution dist, vx_map_id map_id)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(&dist->base, VX_TYPE_DISTRIBUTION) == vx_true_e)
    {
        obj_desc = (tivx_obj_desc_distribution_t *)dist->base.obj_desc;
    }

    if ((obj_desc == NULL) || (obj_desc->mem_ptr.host_ptr == (uint64_t)NULL))
    {
        VX_PRINT(VX_ZONE_ERROR, "vxUnmapDistribution: object descriptor is NULL or host ptr is NULL\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr,
            obj_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_AND_WRITE);
    }

    return (status);
}

static vx_status ownAllocDistributionBuffer(vx_reference ref)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if(ref->type == VX_TYPE_DISTRIBUTION)
    {
        obj_desc = (tivx_obj_desc_distribution_t *)ref->obj_desc;
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
                    VX_PRINT(VX_ZONE_ERROR, "ownAllocDistributionBuffer: could not allocate memory\n");
                    status = VX_ERROR_NO_MEMORY;
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
            VX_PRINT(VX_ZONE_ERROR, "ownAllocDistributionBuffer: object descriptor is NULL\n");
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "ownAllocDistributionBuffer: reference type is not distribution\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownDestructDistribution(vx_reference ref)
{
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if(ref->type == VX_TYPE_DISTRIBUTION)
    {
        obj_desc = (tivx_obj_desc_distribution_t *)ref->obj_desc;
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


