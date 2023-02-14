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

static vx_status ownDestructDistribution(vx_reference ref);
static vx_status ownAllocDistributionBuffer(vx_reference ref);

vx_distribution VX_API_CALL vxCreateDistribution(
    vx_context context, vx_size num_bins, vx_int32 offset, vx_uint32 range)
{
    vx_distribution dist = NULL;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if(ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        if ((0U != num_bins) && (0U != range))
        {
            dist = (vx_distribution)ownCreateReference(context,
                (vx_enum)VX_TYPE_DISTRIBUTION, (vx_enum)VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)dist) == (vx_status)VX_SUCCESS) &&
                (dist->base.type == (vx_enum)VX_TYPE_DISTRIBUTION))
            {
                /* assign refernce type specific callback's */
                dist->base.destructor_callback = &ownDestructDistribution;
                dist->base.mem_alloc_callback = &ownAllocDistributionBuffer;
                dist->base.release_callback =
                    (tivx_reference_release_callback_f)&vxReleaseDistribution;

                obj_desc = (tivx_obj_desc_distribution_t*)ownObjDescAlloc(
                    (vx_enum)TIVX_OBJ_DESC_DISTRIBUTION, (vx_reference)dist);
                if(obj_desc==NULL)
                {
                    vxReleaseDistribution(&dist);

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                        "Could not allocate dist object descriptor\n");
                    dist = (vx_distribution)ownGetErrorObject(
                        context, (vx_status)VX_ERROR_NO_RESOURCES);
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate dist object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
                }
                else
                {
                    obj_desc->num_bins = (vx_uint32)num_bins;
                    obj_desc->range = range;
                    obj_desc->offset = (vx_uint32)offset;
                    obj_desc->num_win = (vx_uint32)range/(vx_uint32)num_bins;
                    obj_desc->mem_size = (vx_uint32)num_bins * (vx_uint32)sizeof(vx_int32);
                    obj_desc->mem_ptr.host_ptr = (uint64_t)(uintptr_t)NULL;
                    obj_desc->mem_ptr.shared_ptr = (uint64_t)(uintptr_t)NULL;
                    obj_desc->mem_ptr.mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;
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
        (vx_reference*)dist, (vx_enum)VX_TYPE_DISTRIBUTION, (vx_enum)VX_EXTERNAL, NULL));
}

vx_status VX_API_CALL vxQueryDistribution(
    vx_distribution dist, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if (ownIsValidSpecificReference((vx_reference)dist, (vx_enum)VX_TYPE_DISTRIBUTION) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_distribution_t *)dist->base.obj_desc;
    }

    if (obj_desc == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Object Descriptor! \n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case (vx_enum)VX_DISTRIBUTION_DIMENSIONS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    /* Only 1D is supported */
                    *(vx_size *)ptr = 1U;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "distribution dimensions query failed \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_DISTRIBUTION_OFFSET:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U))
                {
                    *(vx_int32 *)ptr = (vx_int32)obj_desc->offset;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "distribution offset query failed \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_DISTRIBUTION_RANGE:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->range;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "distribution range query failed \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_DISTRIBUTION_BINS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->num_bins;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "distribution bins query failed \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_DISTRIBUTION_WINDOW:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->num_win;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "distribution window query failed \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_DISTRIBUTION_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->mem_size;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "distribution size query failed \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "invalid attribute \n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

vx_status VX_API_CALL vxCopyDistribution(
    vx_distribution dist, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 size;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if (ownIsValidSpecificReference((vx_reference)dist, (vx_enum)VX_TYPE_DISTRIBUTION) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_distribution_t *)dist->base.obj_desc;
    }

    if (obj_desc == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Object Descriptor! \n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if ((vx_enum)VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            VX_PRINT(VX_ZONE_ERROR, "user_mem_type is not VX_MEMORY_TYPE_HOST\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        /* Memory still not allocated */
        if (((vx_enum)VX_READ_ONLY == usage) &&
            ((uint64_t)(uintptr_t)NULL == obj_desc->mem_ptr.host_ptr))
        {
            VX_PRINT(VX_ZONE_ERROR, "Memory still allocated\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        if (NULL == user_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "user pointer is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        size = obj_desc->mem_size;

        /* Copy from dist object to user memory */
        if ((vx_enum)VX_READ_ONLY == usage)
        {
            tivxCheckStatus(&status, tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            memcpy(user_ptr, (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size);

            tivxCheckStatus(&status, tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
        else /* Copy from user memory to dist object */
        {
            status = ownAllocDistributionBuffer(&dist->base);

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

vx_status VX_API_CALL vxMapDistribution(
    vx_distribution dist, vx_map_id *map_id, void **ptr, vx_enum usage, vx_enum mem_type,
    vx_bitfield flags)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference((vx_reference)dist, (vx_enum)VX_TYPE_DISTRIBUTION) == (vx_bool)vx_false_e)
        ||
        (dist->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Distribution reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        status = ownAllocDistributionBuffer(&dist->base);
        if ((NULL != ptr) && ((vx_status)VX_SUCCESS == status))
        {
            obj_desc = (tivx_obj_desc_distribution_t *)dist->base.obj_desc;
            tivxCheckStatus(&status, tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr,
                obj_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_AND_WRITE));

            *ptr = (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
        }
    }

    return (status);
}

vx_status VX_API_CALL vxUnmapDistribution(vx_distribution dist, vx_map_id map_id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if (ownIsValidSpecificReference((vx_reference)dist, (vx_enum)VX_TYPE_DISTRIBUTION) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_distribution_t *)dist->base.obj_desc;
    }

    if ((obj_desc == NULL) || (obj_desc->mem_ptr.host_ptr == (uint64_t)(uintptr_t)NULL))
    {
        VX_PRINT(VX_ZONE_ERROR, "object descriptor is NULL or host ptr is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        tivxCheckStatus(&status, tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr,
            obj_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_AND_WRITE));
    }

    return (status);
}

static vx_status ownAllocDistributionBuffer(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if(ref->type == (vx_enum)VX_TYPE_DISTRIBUTION)
    {
        obj_desc = (tivx_obj_desc_distribution_t *)ref->obj_desc;
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
                    status = (vx_status)VX_ERROR_NO_MEMORY;
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
        VX_PRINT(VX_ZONE_ERROR, "reference type is not distribution\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownDestructDistribution(vx_reference ref)
{
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if(ref->type == (vx_enum)VX_TYPE_DISTRIBUTION)
    {
        obj_desc = (tivx_obj_desc_distribution_t *)ref->obj_desc;
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


