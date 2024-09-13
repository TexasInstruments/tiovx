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

static tivx_remap_point_t *ownGetRemapPoint(const tivx_obj_desc_remap_t *obj_desc, vx_uint32 dst_x, vx_uint32 dst_y);

static tivx_remap_point_t *ownGetRemapPoint(const tivx_obj_desc_remap_t *obj_desc, vx_uint32 dst_x, vx_uint32 dst_y)
{
    return &(((tivx_remap_point_t*)(uintptr_t)obj_desc->mem_ptr.host_ptr)[(dst_y*obj_desc->dst_width)+ dst_x]);
}

VX_API_ENTRY vx_remap VX_API_CALL vxCreateRemap(vx_context context,
                              vx_uint32 src_width,
                              vx_uint32 src_height,
                              vx_uint32 dst_width,
                              vx_uint32 dst_height)
{
    vx_remap remap = NULL;
    vx_reference ref = NULL;
    tivx_obj_desc_remap_t *obj_desc = NULL;

    if(ownIsValidContext(context)==(vx_bool)vx_true_e)
    {
        if ((src_width != 0U) && (src_height != 0U) && (dst_width != 0U) && (dst_height != 0U))
        {
            ref = ownCreateReference(context, (vx_enum)VX_TYPE_REMAP, (vx_enum)VX_EXTERNAL, &context->base);
            if ((vxGetStatus(ref) == (vx_status)VX_SUCCESS) && (ref->type == (vx_enum)VX_TYPE_REMAP))
            {
                /* status set to NULL due to preceding type check */
                remap = vxCastRefAsRemap(ref, NULL);
                /* assign refernce type specific callback's */
                remap->base.destructor_callback = &ownDestructReferenceGeneric;
                remap->base.mem_alloc_callback = &ownAllocReferenceBufferGeneric;
                remap->base.release_callback = &ownReleaseReferenceBufferGeneric;

                obj_desc = (tivx_obj_desc_remap_t*)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_REMAP, vxCastRefFromRemap(remap));
                if(obj_desc==NULL)
                {
                    (void)vxReleaseRemap(&remap);

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES, "Could not allocate remap object descriptor\n");
                    remap = (vx_remap)ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate remap object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
                }
                else
                {
                    obj_desc->src_width = src_width;
                    obj_desc->src_height = src_height;
                    obj_desc->dst_width = dst_width;
                    obj_desc->dst_height = dst_height;
                    obj_desc->mem_size = dst_width*dst_height*(vx_uint32)sizeof(tivx_remap_point_t);
                    obj_desc->mem_ptr.host_ptr = (uint64_t)0;
                    obj_desc->mem_ptr.shared_ptr = (uint64_t)0;
                    obj_desc->mem_ptr.mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;
                    remap->base.obj_desc = (tivx_obj_desc_t *)obj_desc;
                }
            }
        }
        else
        {
            vxAddLogEntry(&context->base, (vx_status)VX_ERROR_INVALID_PARAMETERS, "Invalid parameters to remap\n");
            remap = (vx_remap)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
        }
    }

    return remap;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseRemap(vx_remap *table)
{
    return ownReleaseReferenceInt(vxCastRefFromRemapP(table), (vx_enum)VX_TYPE_REMAP, (vx_enum)VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryRemap(vx_remap remap, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_remap_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(vxCastRefFromRemap(remap), (vx_enum)VX_TYPE_REMAP) == (vx_bool)vx_false_e)
        ||
        (remap->base.obj_desc == NULL))
    {
        VX_PRINT(VX_ZONE_ERROR, "Reference is not valid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_remap_t *)remap->base.obj_desc;
        switch (attribute)
        {
            case (vx_enum)VX_REMAP_SOURCE_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->src_width;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query remap source width failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_REMAP_SOURCE_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->src_height;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query remap source height failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_REMAP_DESTINATION_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->dst_width;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query remap destination width failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_REMAP_DESTINATION_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->dst_height;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query remap destination height failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetRemapPoint(vx_remap remap, vx_uint32 dst_x, vx_uint32 dst_y,
                                 vx_float32 src_x, vx_float32 src_y)
{
    vx_status status = (vx_status)VX_FAILURE;
    tivx_obj_desc_remap_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(vxCastRefFromRemap(remap), (vx_enum)VX_TYPE_REMAP) == (vx_bool)vx_true_e)
         &&
        (remap->base.obj_desc != NULL)
         )
    {
        status = ownAllocReferenceBufferGeneric(&remap->base);

        if ((vx_status)VX_SUCCESS == status)
        {
            obj_desc = (tivx_obj_desc_remap_t *)remap->base.obj_desc;

            if(obj_desc->mem_ptr.host_ptr != (uint64_t)0) /* TIOVX-1882- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_REMAP_UBR001 */
            {
                if ((dst_x < obj_desc->dst_width) &&
                    (dst_y < obj_desc->dst_height))
                {
                    tivx_remap_point_t *remap_point;

                    remap_point = ownGetRemapPoint(obj_desc, dst_x, dst_y);

                    if ( (dst_x==0U) && (dst_y==0U))
                    {
                        tivxCheckStatus(&status, tivxMemBufferMap(remap_point, (uint32_t)sizeof(tivx_remap_point_t),
                            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
                    }

                    remap_point->src_x = src_x;
                    remap_point->src_y = src_y;

                    if ( (dst_x == (obj_desc->dst_width-1U)) &&
                         (dst_y == (obj_desc->dst_height-1U)))
                    {
                        tivxCheckStatus(&status, tivxMemBufferUnmap(remap_point, (uint32_t)sizeof(tivx_remap_point_t),
                            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
                    }

                    status = (vx_status)VX_SUCCESS;
                }
                else
                {
                    status = (vx_status)VX_ERROR_INVALID_VALUE;
                    if (!(dst_x < obj_desc->dst_width))
                    {
                        VX_PRINT(VX_ZONE_ERROR, "dst_x is greater than object descriptor destination width\n");
                    }
                    if (!(dst_y < obj_desc->dst_height))
                    {
                        VX_PRINT(VX_ZONE_ERROR, "dst_x is greater than object descriptor destination width\n");
                    }
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Could not allocate memory\n");
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxGetRemapPoint(vx_remap remap, vx_uint32 dst_x, vx_uint32 dst_y,
                                 vx_float32 *src_x, vx_float32 *src_y)
{
    vx_status status = (vx_status)VX_FAILURE;
    tivx_obj_desc_remap_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(vxCastRefFromRemap(remap), (vx_enum)VX_TYPE_REMAP) == (vx_bool)vx_true_e)
         &&
        (remap->base.obj_desc != NULL)
         )
    {
        obj_desc = (tivx_obj_desc_remap_t *)remap->base.obj_desc;

        if(obj_desc->mem_ptr.host_ptr != (uint64_t)0)
        {
            if ((dst_x < obj_desc->dst_width) &&
                (dst_y < obj_desc->dst_height))
            {
                tivx_remap_point_t *remap_point;

                remap_point = ownGetRemapPoint(obj_desc, dst_x, dst_y);

                if ( (dst_x==0U) && (dst_y==0U))
                {
                    tivxCheckStatus(&status, tivxMemBufferMap(remap_point, (uint32_t)sizeof(tivx_remap_point_t),
                        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
                }

                *src_x = remap_point->src_x;
                *src_y = remap_point->src_y;

                if ( (dst_x == (obj_desc->dst_width-1U)) &&
                     (dst_y == (obj_desc->dst_height-1U)))
                {
                    tivxCheckStatus(&status, tivxMemBufferUnmap(remap_point, (uint32_t)sizeof(tivx_remap_point_t),
                        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
                }

                status = (vx_status)VX_SUCCESS;
            }
            else
            {
                status = (vx_status)VX_ERROR_INVALID_VALUE;
                if (!(dst_x < obj_desc->dst_width))
                {
                    VX_PRINT(VX_ZONE_ERROR, "dst_x is greater than object descriptor destination width\n");
                }
                if (!(dst_y < obj_desc->dst_height))
                {
                    VX_PRINT(VX_ZONE_ERROR, "dst_x is greater than object descriptor destination width\n");
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Could not allocate memory\n");
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}
