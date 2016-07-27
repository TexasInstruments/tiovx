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

static vx_status ownDestructDistribution(vx_reference ref);
static vx_status ownAllocDistributionBuffer(vx_reference ref);

vx_distribution VX_API_CALL vxCreateDistribution(
    vx_context context, vx_size num_bins, vx_int32 offset, vx_uint32 range)
{
    vx_distribution dist = NULL;

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
                dist->base.destructor_callback = ownDestructDistribution;
                dist->base.mem_alloc_callback = ownAllocDistributionBuffer;
                dist->base.release_callback =
                    (tivx_reference_release_callback_f)vxReleaseDistribution;

                dist->obj_desc = (tivx_obj_desc_distribution_t*)tivxObjDescAlloc(
                    TIVX_OBJ_DESC_DISTRIBUTION);
                if(dist->obj_desc==NULL)
                {
                    vxReleaseDistribution(&dist);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                        "Could not allocate dist object descriptor\n");
                    dist = (vx_distribution)ownGetErrorObject(
                        context, VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    dist->obj_desc->num_bins = num_bins;
                    dist->obj_desc->range = range;
                    dist->obj_desc->offset = offset;
                    dist->obj_desc->num_win = (vx_uint32)range/(vx_uint32)num_bins;
                    dist->obj_desc->mem_size = num_bins * sizeof(vx_int32);
                    dist->obj_desc->mem_ptr.host_ptr = NULL;
                    dist->obj_desc->mem_ptr.shared_ptr = NULL;
                    dist->obj_desc->mem_ptr.mem_type = TIVX_MEM_EXTERNAL;
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

    if (ownIsValidSpecificReference(&dist->base, VX_TYPE_DISTRIBUTION) == vx_false_e
        &&
        dist->obj_desc != NULL
        )
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case VX_DISTRIBUTION_DIMENSIONS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    /* Only 1D is supported */
                    *(vx_size *)ptr = 1U;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_DISTRIBUTION_OFFSET:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3))
                {
                    *(vx_int32 *)ptr = dist->obj_desc->offset;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_DISTRIBUTION_RANGE:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = dist->obj_desc->range;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_DISTRIBUTION_BINS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr =
                        dist->obj_desc->num_bins;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_DISTRIBUTION_WINDOW:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr =
                        dist->obj_desc->num_win;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_DISTRIBUTION_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = dist->obj_desc->mem_size;
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

vx_status VX_API_CALL vxCopyDistribution(
    vx_distribution dist, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 size;

    if ((ownIsValidSpecificReference(&dist->base, VX_TYPE_DISTRIBUTION) ==
            vx_false_e) && (dist->obj_desc != NULL))
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if (VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Memory still not allocated */
        if ((VX_READ_ONLY == usage) &&
            (NULL == dist->obj_desc->mem_ptr.host_ptr))
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
        size = dist->obj_desc->mem_size;

        /* Copy from dist object to user memory */
        if (VX_READ_ONLY == usage)
        {
            tivxMemBufferMap(dist->obj_desc->mem_ptr.host_ptr, size,
                dist->obj_desc->mem_ptr.mem_type, VX_READ_ONLY);

            memcpy(user_ptr, dist->obj_desc->mem_ptr.host_ptr, size);

            tivxMemBufferUnmap(dist->obj_desc->mem_ptr.host_ptr, size,
                dist->obj_desc->mem_ptr.mem_type, VX_READ_ONLY);
        }
        else /* Copy from user memory to dist object */
        {
            status = ownAllocDistributionBuffer(&dist->base);

            if (VX_SUCCESS == status)
            {
                tivxMemBufferMap(dist->obj_desc->mem_ptr.host_ptr, size,
                    dist->obj_desc->mem_ptr.mem_type, VX_WRITE_ONLY);

                memcpy(dist->obj_desc->mem_ptr.host_ptr, user_ptr, size);

                tivxMemBufferUnmap(dist->obj_desc->mem_ptr.host_ptr, size,
                    dist->obj_desc->mem_ptr.mem_type, VX_WRITE_ONLY);
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

    if (ownIsValidSpecificReference(&dist->base, VX_TYPE_DISTRIBUTION) == vx_false_e
        &&
        dist->obj_desc != NULL
        )
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if (NULL != ptr)
        {
            tivxMemBufferMap(dist->obj_desc->mem_ptr.host_ptr,
                dist->obj_desc->mem_size, dist->obj_desc->mem_ptr.mem_type,
                VX_READ_AND_WRITE);

            *ptr = dist->obj_desc->mem_ptr.host_ptr;
        }
    }

    return (status);
}

vx_status VX_API_CALL vxUnmapDistribution(vx_distribution dist, vx_map_id map_id)
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidSpecificReference(&dist->base, VX_TYPE_DISTRIBUTION) == vx_false_e
        &&
        dist->obj_desc != NULL
        )
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        tivxMemBufferUnmap(dist->obj_desc->mem_ptr.host_ptr,
            dist->obj_desc->mem_size, dist->obj_desc->mem_ptr.mem_type,
            VX_READ_AND_WRITE);
    }

    return (status);
}

static vx_status ownAllocDistributionBuffer(vx_reference ref)
{
    vx_status status = VX_SUCCESS;
    vx_distribution dist = (vx_distribution)ref;

    if(dist->base.type == VX_TYPE_DISTRIBUTION)
    {
        if(dist->obj_desc != NULL)
        {
            /* memory is not allocated, so allocate it */
            if(dist->obj_desc->mem_ptr.host_ptr == NULL)
            {
                tivxMemBufferAlloc(
                    &dist->obj_desc->mem_ptr, dist->obj_desc->mem_size,
                    TIVX_MEM_EXTERNAL);

                if(dist->obj_desc->mem_ptr.host_ptr==NULL)
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

static vx_status ownDestructDistribution(vx_reference ref)
{
    vx_distribution dist = (vx_distribution)ref;

    if(dist->base.type == VX_TYPE_DISTRIBUTION)
    {
        if(dist->obj_desc!=NULL)
        {
            if(dist->obj_desc->mem_ptr.host_ptr!=NULL)
            {
                tivxMemBufferFree(
                    &dist->obj_desc->mem_ptr, dist->obj_desc->mem_size);
            }

            tivxObjDescFree((tivx_obj_desc_t**)&dist->obj_desc);
        }
    }
    return VX_SUCCESS;
}


