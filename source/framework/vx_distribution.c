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

VX_API_ENTRY vx_distribution VX_API_CALL vxCreateDistribution(vx_context context, vx_size num_bins, vx_int32 offset, vx_uint32 range)
{
    vx_distribution dist = NULL;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if(ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        vx_reference ref;
        if ((0U != num_bins) && (0U != range))
        {
            ref = ownCreateReference(context,
                (vx_enum)VX_TYPE_DISTRIBUTION, (vx_enum)VX_EXTERNAL, &context->base);

            if ((vxGetStatus(ref) == (vx_status)VX_SUCCESS) &&
                (ref->type == (vx_enum)VX_TYPE_DISTRIBUTION))
            {
                /*status set to NULL due to preceding type check*/
                dist = vxCastRefAsDistribution(ref, NULL);
                /* assign refernce type specific callback's */
                dist->base.destructor_callback = &ownDestructReferenceGeneric;
                dist->base.mem_alloc_callback = &ownAllocReferenceBufferGeneric;
                dist->base.release_callback =
                    &ownReleaseReferenceBufferGeneric;
                dist->base.kernel_callback = &ownKernelCallbackGeneric;
                obj_desc = (tivx_obj_desc_distribution_t*)ownObjDescAlloc(
                    (vx_enum)TIVX_OBJ_DESC_DISTRIBUTION, vxCastRefFromDistribution(dist));
                if(obj_desc==NULL)
                {
                    (void)vxReleaseDistribution(&dist);

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                        "Could not allocate dist object descriptor\n");
                    dist = (vx_distribution)ownGetErrorObject(
                        context, (vx_status)VX_ERROR_NO_RESOURCES);
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate dist object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in include/TI/soc/tivx_config_<soc>.h\n");
                }
                else
                {
                    obj_desc->num_bins = (vx_uint32)num_bins;
                    obj_desc->range = range;
                    obj_desc->offset = (vx_uint32)offset;
                    obj_desc->num_win = (vx_uint32)range/(vx_uint32)num_bins;
                    obj_desc->mem_size = (vx_uint32)num_bins * (vx_uint32)sizeof(vx_int32);
                    obj_desc->mem_ptr.host_ptr = (uint64_t)0;
                    obj_desc->mem_ptr.shared_ptr = (uint64_t)0;
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
        vxCastRefFromDistributionP(dist), (vx_enum)VX_TYPE_DISTRIBUTION, (vx_enum)VX_EXTERNAL, NULL));
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryDistribution(
    vx_distribution dist, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(vxCastRefFromDistribution(dist), (vx_enum)VX_TYPE_DISTRIBUTION) == (vx_bool)vx_true_e)
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

VX_API_ENTRY vx_status VX_API_CALL vxCopyDistribution(
    vx_distribution dist, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 size;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(vxCastRefFromDistribution(dist), (vx_enum)VX_TYPE_DISTRIBUTION) == (vx_bool)vx_true_e)
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
            ((uint64_t)0 == obj_desc->mem_ptr.host_ptr))
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

            (void)memcpy(user_ptr, (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size);

            tivxCheckStatus(&status, tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
        else /* Copy from user memory to dist object */
        {
            status = ownAllocReferenceBufferGeneric(&dist->base);

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

VX_API_ENTRY vx_status VX_API_CALL vxMapDistribution(
    vx_distribution dist, vx_map_id *map_id, void **ptr, vx_enum usage, vx_enum mem_type,
    vx_bitfield flags)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(vxCastRefFromDistribution(dist), (vx_enum)VX_TYPE_DISTRIBUTION) == (vx_bool)vx_false_e)
        ||
        (dist->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Distribution reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        status = ownAllocReferenceBufferGeneric(&dist->base);
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

VX_API_ENTRY vx_status VX_API_CALL vxUnmapDistribution(vx_distribution dist, vx_map_id map_id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_distribution_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(vxCastRefFromDistribution(dist), (vx_enum)VX_TYPE_DISTRIBUTION) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_distribution_t *)dist->base.obj_desc;
    }

    if ((obj_desc == NULL) || (obj_desc->mem_ptr.host_ptr == (uint64_t)0))
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
