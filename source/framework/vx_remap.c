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

static vx_status ownDestructRemap(vx_reference ref);
static tivx_remap_point_t *ownGetRemapPoint(tivx_obj_desc_remap_t *obj_desc, vx_uint32 dst_x, vx_uint32 dst_y);
static vx_status ownAllocRemapBuffer(vx_reference ref);

static vx_status ownDestructRemap(vx_reference ref)
{
    tivx_obj_desc_remap_t *obj_desc = NULL;

    if(ref->type == VX_TYPE_REMAP)
    {
        obj_desc = (tivx_obj_desc_remap_t *)ref->obj_desc;

        if(obj_desc!=NULL)
        {
            if(obj_desc->mem_ptr.host_ptr!=(uint64_t)NULL)
            {
                tivxMemBufferFree(&obj_desc->mem_ptr, obj_desc->mem_size);
            }

            tivxObjDescFree((tivx_obj_desc_t**)&obj_desc);
        }
    }
    return VX_SUCCESS;
}

static tivx_remap_point_t *ownGetRemapPoint(tivx_obj_desc_remap_t *obj_desc, vx_uint32 dst_x, vx_uint32 dst_y)
{
    return (tivx_remap_point_t*)(uintptr_t)obj_desc->mem_ptr.host_ptr
                 + (dst_y*obj_desc->dst_width)
                 + dst_x
           ;
}

static vx_status ownAllocRemapBuffer(vx_reference ref)
{
    tivx_obj_desc_remap_t *obj_desc = NULL;
    vx_status status = VX_SUCCESS;

    if(ref->type == VX_TYPE_REMAP)
    {
        obj_desc = (tivx_obj_desc_remap_t *)ref->obj_desc;

        if(obj_desc!=NULL)
        {
            /* memory is not allocated, so allocate it */
            if(obj_desc->mem_ptr.host_ptr==(uint64_t)NULL)
            {
                tivxMemBufferAlloc(&obj_desc->mem_ptr, obj_desc->mem_size,
                    TIVX_MEM_EXTERNAL);

                if(obj_desc->mem_ptr.host_ptr==(uint64_t)NULL)
                {
                    /* could not allocate memory */
                    VX_PRINT(VX_ZONE_ERROR, "ownAllocRemapBuffer: Could not allocate memory\n");
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
            VX_PRINT(VX_ZONE_ERROR, "ownAllocRemapBuffer: Reference object descriptor is NULL\n");
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "ownAllocRemapBuffer: Reference type is not remap\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_remap VX_API_CALL vxCreateRemap(vx_context context,
                              vx_uint32 src_width,
                              vx_uint32 src_height,
                              vx_uint32 dst_width,
                              vx_uint32 dst_height)
{
    vx_remap remap = NULL;
    tivx_obj_desc_remap_t *obj_desc = NULL;

    if(ownIsValidContext(context)==vx_true_e)
    {
        if ((src_width != 0) && (src_height != 0) && (dst_width != 0) && (dst_height != 0))
        {
            remap = (vx_remap)ownCreateReference(context, VX_TYPE_REMAP, VX_EXTERNAL, &context->base);
            if ((vxGetStatus((vx_reference)remap) == VX_SUCCESS) && (remap->base.type == VX_TYPE_REMAP))
            {
                /* assign refernce type specific callback's */
                remap->base.destructor_callback = &ownDestructRemap;
                remap->base.mem_alloc_callback = &ownAllocRemapBuffer;
                remap->base.release_callback = (tivx_reference_release_callback_f)&vxReleaseRemap;

                obj_desc = (tivx_obj_desc_remap_t*)tivxObjDescAlloc(TIVX_OBJ_DESC_REMAP, (vx_reference)remap);
                if(obj_desc==NULL)
                {
                    vxReleaseRemap(&remap);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES, "Could not allocate remap object descriptor\n");
                    remap = (vx_remap)ownGetErrorObject(context, VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    obj_desc->src_width = src_width;
                    obj_desc->src_height = src_height;
                    obj_desc->dst_width = dst_width;
                    obj_desc->dst_height = dst_height;
                    obj_desc->mem_size = dst_width*dst_height*sizeof(tivx_remap_point_t);
                    obj_desc->mem_ptr.host_ptr = (uint64_t)NULL;
                    obj_desc->mem_ptr.shared_ptr = (uint64_t)NULL;
                    obj_desc->mem_ptr.mem_heap_region = TIVX_MEM_EXTERNAL;
                    remap->base.obj_desc = (tivx_obj_desc_t *)obj_desc;
                }
            }
        }
        else
        {
            vxAddLogEntry(&context->base, VX_ERROR_INVALID_PARAMETERS, "Invalid parameters to remap\n");
            remap = (vx_remap)ownGetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
        }
    }

    return remap;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseRemap(vx_remap *table)
{
    return ownReleaseReferenceInt( (vx_reference*)table, VX_TYPE_REMAP, VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryRemap(vx_remap remap, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_remap_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&remap->base, VX_TYPE_REMAP) == vx_false_e)
        &&
        (remap->base.obj_desc == NULL))
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryRemap: Reference is not valid\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_remap_t *)remap->base.obj_desc;
        switch (attribute)
        {
            case VX_REMAP_SOURCE_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->src_width;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryRemap: Query remap source width failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_REMAP_SOURCE_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->src_height;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryRemap: Query remap source height failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_REMAP_DESTINATION_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->dst_width;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryRemap: Query remap destination width failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_REMAP_DESTINATION_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->dst_height;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryRemap: Query remap destination height failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "vxQueryRemap: Invalid attribute\n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetRemapPoint(vx_remap remap, vx_uint32 dst_x, vx_uint32 dst_y,
                                 vx_float32 src_x, vx_float32 src_y)
{
    vx_status status = VX_FAILURE;
    tivx_obj_desc_remap_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&remap->base, VX_TYPE_REMAP) == vx_true_e)
         &&
        (remap->base.obj_desc != NULL)
         )
    {
        ownAllocRemapBuffer(&remap->base);

        obj_desc = (tivx_obj_desc_remap_t *)remap->base.obj_desc;

        if(obj_desc->mem_ptr.host_ptr != (uint64_t)NULL)
        {
            if ((dst_x < obj_desc->dst_width) &&
                (dst_y < obj_desc->dst_height))
            {
                tivx_remap_point_t *remap_point;

                remap_point = ownGetRemapPoint(obj_desc, dst_x, dst_y);

                tivxMemBufferMap(remap_point, sizeof(tivx_remap_point_t),
                    VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

                remap_point->src_x = src_x;
                remap_point->src_y = src_y;

                tivxMemBufferUnmap(remap_point, sizeof(tivx_remap_point_t),
                    VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

                status = VX_SUCCESS;
            }
            else
            {
                status = VX_ERROR_INVALID_VALUE;
                if (!(dst_x < obj_desc->dst_width))
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetRemapPoint: dst_x is greater than object descriptor destination width\n");
                }
                if (!(dst_y < obj_desc->dst_height))
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetRemapPoint: dst_x is greater than object descriptor destination width\n");
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "vxSetRemapPoint: Could not allocate memory\n");
            status = VX_ERROR_INVALID_REFERENCE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetRemapPoint: Invalid reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxGetRemapPoint(vx_remap remap, vx_uint32 dst_x, vx_uint32 dst_y,
                                 vx_float32 *src_x, vx_float32 *src_y)
{
    vx_status status = VX_FAILURE;
    tivx_obj_desc_remap_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&remap->base, VX_TYPE_REMAP) == vx_true_e)
         &&
        (remap->base.obj_desc != NULL)
         )
    {
        obj_desc = (tivx_obj_desc_remap_t *)remap->base.obj_desc;

        if(obj_desc->mem_ptr.host_ptr != (uint64_t)NULL)
        {
            if ((dst_x < obj_desc->dst_width) &&
                (dst_y < obj_desc->dst_height))
            {
                tivx_remap_point_t *remap_point;

                remap_point = ownGetRemapPoint(obj_desc, dst_x, dst_y);

                tivxMemBufferMap(remap_point, sizeof(tivx_remap_point_t),
                    VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

                *src_x = remap_point->src_x;
                *src_y = remap_point->src_y;

                tivxMemBufferUnmap(remap_point, sizeof(tivx_remap_point_t),
                    VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

                status = VX_SUCCESS;
            }
            else
            {
                status = VX_ERROR_INVALID_VALUE;
                if (!(dst_x < obj_desc->dst_width))
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxGetRemapPoint: dst_x is greater than object descriptor destination width\n");
                }
                if (!(dst_y < obj_desc->dst_height))
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxGetRemapPoint: dst_x is greater than object descriptor destination width\n");
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "vxGetRemapPoint: Could not allocate memory\n");
            status = VX_ERROR_INVALID_REFERENCE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxGetRemapPoint: Invalid reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}
