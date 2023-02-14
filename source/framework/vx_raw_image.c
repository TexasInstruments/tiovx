/*
 * Copyright (c) 2012-2018 The Khronos Group Inc.
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

static vx_bool ownIsValidCreateParams(const tivx_raw_image_create_params_t *params);
static vx_bool ownIsValidRawImage(tivx_raw_image image);
static vx_uint32 ownComputePatchOffset(vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t* addr);
static vx_status ownDestructRawImage(vx_reference ref);
static vx_status ownAllocRawImageBuffer(vx_reference ref);
static void ownInitRawImage(tivx_raw_image image, tivx_raw_image_create_params_t *params);
static tivx_raw_image ownCreateRawImageInt(vx_context context,
                                           tivx_raw_image_create_params_t *params,
                                           tivx_image_create_type_e create_type);
static vx_status ownCopyAndMapCheckParams(
    tivx_raw_image image,
    const vx_rectangle_t* rect,
    const vx_imagepatch_addressing_t *addr,
    vx_uint32 exposure_index,
    vx_enum usage,
    vx_uint32 buffer_select);

static vx_bool ownIsValidCreateParams(const tivx_raw_image_create_params_t *params)
{
    vx_bool is_valid = (vx_bool)vx_true_e;

    if( (params->width < 2u ) || (params->height < 1u ) || ((params->width & 1u) == 1u) )
    {
        is_valid = (vx_bool)vx_false_e;
        VX_PRINT(VX_ZONE_ERROR, "invalid width and/or height\n");
    }

    if( (params->num_exposures < 1u) || ((int32_t)params->num_exposures > TIVX_RAW_IMAGE_MAX_EXPOSURES) )
    {
        is_valid = (vx_bool)vx_false_e;
        VX_PRINT(VX_ZONE_ERROR, "invalid num_exposures\n");
    }
    else
    {
        uint32_t i;

        for(i=0; i < params->num_exposures; i++)
        {
            if( (int32_t)params->format[i].pixel_container > (int32_t)TIVX_RAW_IMAGE_P12_BIT )
            {
                is_valid = (vx_bool)vx_false_e;
                VX_PRINT(VX_ZONE_ERROR, "invalid pixel_container for exposure index %d\n", i);
            }

            if( params->format[i].msb > 15u )
            {
                is_valid = (vx_bool)vx_false_e;
                VX_PRINT(VX_ZONE_ERROR, "invalid msb for exposure index %d\n", i);
            }
        }
    }

    return is_valid;
}

static vx_bool ownIsValidRawImage(tivx_raw_image image)
{
    vx_bool is_valid;

    if ((ownIsValidSpecificReference((vx_reference)image, TIVX_TYPE_RAW_IMAGE) == (vx_bool)vx_true_e) &&
        (image->base.obj_desc != NULL)
       )
    {
        is_valid = (vx_bool)vx_true_e;
    }
    else
    {
        is_valid = (vx_bool)vx_false_e;
    }

    return is_valid;
}

static vx_uint32 ownComputePatchOffset(vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t* addr)
{
    vx_uint32 offset;

    if(addr->stride_x == 0)
    {
        /* TIVX_DF_IMAGE_P12 case */
        /* If x is even, proper offset
         * if x is odd, then offset is on byte alignment 4 bits before start of pixel */
        offset = ((vx_uint32)addr->stride_y * y) +
                 ((12U * x)/8U);
    }
    else
    {
        offset = ((vx_uint32)addr->stride_y * y) +
                 ((vx_uint32)addr->stride_x * x);
    }

    return offset;
}

#if 0
static void ownLinkParentSubimage(tivx_raw_image parent, tivx_raw_image subimage)
{
    uint16_t p;

    /* remember that the scope of the subimage is the parent image */
    ownReferenceSetScope(&subimage->base, &parent->base);

    /* refer to our parent image and internally refcount it */
    subimage->parent = parent;

    /* it will find free space for subimage since this was checked before */
    for (p = 0; p < TIVX_RAW_IMAGE_MAX_SUBIMAGES; p++)
    {
        if (parent->subimages[p] == NULL)
        {
            parent->subimages[p] = subimage;
            ownLogSetResourceUsedValue("TIVX_RAW_IMAGE_MAX_SUBIMAGES", p+1);
            break;
        }
    }

    if (parent->subimages[p] == NULL)
    {
        VX_PRINT(VX_ZONE_WARNING, "May need to increase the value of TIVX_RAW_IMAGE_MAX_SUBIMAGES in tiovx/include/TI/tivx_config.h\n");
    }

    ownIncrementReference(&parent->base, (vx_enum)VX_INTERNAL);
}

static vx_status ownIsFreeSubimageAvailable(tivx_raw_image image)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t p;

    /* check if image can contain subimage */
    for (p = 0; p < TIVX_RAW_IMAGE_MAX_SUBIMAGES; p++)
    {
        if (image->subimages[p] == NULL)
        {
            break;
        }
    }
    if(p>=TIVX_RAW_IMAGE_MAX_SUBIMAGES)
    {
        VX_PRINT(VX_ZONE_ERROR, "no subimage is available\n");
        status = (vx_status)VX_ERROR_NO_RESOURCES;
    }

    return status;
}
#endif

static vx_status ownDestructRawImage(vx_reference ref)
{
    tivx_obj_desc_raw_image_t *obj_desc = NULL;
    uint16_t exp_idx;
    tivx_raw_image raw_image = (tivx_raw_image)ref;

    if(ref->type == TIVX_TYPE_RAW_IMAGE)
    {
        obj_desc = (tivx_obj_desc_raw_image_t *)ref->obj_desc;

        if(obj_desc!=NULL)
        {
            if ( (vx_enum)obj_desc->create_type == (vx_enum)TIVX_IMAGE_NORMAL )
            {
                for(exp_idx=0; exp_idx < obj_desc->params.num_exposures; exp_idx++)
                {
                    if(obj_desc->mem_ptr[exp_idx].host_ptr != (uint64_t)(uintptr_t)NULL)
                    {
                        tivxMemBufferFree(&obj_desc->mem_ptr[exp_idx], obj_desc->mem_size[exp_idx]);
                    }
                }
            }
            ownObjDescFree((tivx_obj_desc_t**)&obj_desc);
        }
        if (NULL != raw_image->parent)
        {
            ownReleaseReferenceInt((vx_reference *)&raw_image->parent, TIVX_TYPE_RAW_IMAGE, (vx_enum)VX_INTERNAL, NULL);
        }
    }
    return (vx_status)VX_SUCCESS;
}

vx_status ownDeriveRawImageBufferPointers(vx_reference ref)
{
    tivx_obj_desc_raw_image_t *obj_desc = NULL;
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t exp_idx;

    if(ref != NULL)
    {
        if(ref->type == TIVX_TYPE_RAW_IMAGE)
        {
            obj_desc = (tivx_obj_desc_raw_image_t *)ref->obj_desc;

            if(obj_desc != NULL)
            {
                uint32_t img_line_offset = obj_desc->params.meta_height_before;
                uint32_t meta_line_offset = 0;

                for(exp_idx=0; exp_idx < obj_desc->params.num_exposures; exp_idx++)
                {
                    uint32_t alloc_idx = exp_idx;
                    uint32_t img_byte_offset, meta_byte_offset;
                    uint32_t exposure_offset = 0;

                    if( obj_desc->params.line_interleaved == (vx_bool)vx_true_e )
                    {
                        alloc_idx = 0;
                        exposure_offset = exp_idx;
                    }

                    if(obj_desc->mem_ptr[alloc_idx].host_ptr != (uint64_t)(uintptr_t)NULL)
                    {
                        img_byte_offset = ownComputePatchOffset(0, img_line_offset+exposure_offset,
                                                                &obj_desc->imagepatch_addr[exp_idx]);

                        obj_desc->img_ptr[exp_idx].mem_heap_region = obj_desc->mem_ptr[alloc_idx].mem_heap_region;
                        obj_desc->img_ptr[exp_idx].host_ptr = obj_desc->mem_ptr[alloc_idx].host_ptr + img_byte_offset;
                        obj_desc->img_ptr[exp_idx].shared_ptr = obj_desc->mem_ptr[alloc_idx].shared_ptr + img_byte_offset;

                        if( obj_desc->params.meta_height_before > 0U )
                        {
                            meta_line_offset = 0;
                            meta_byte_offset = ownComputePatchOffset(0, meta_line_offset+exposure_offset,
                                                                    &obj_desc->imagepatch_addr[exp_idx]);

                            obj_desc->meta_before_ptr[exp_idx].mem_heap_region = obj_desc->mem_ptr[alloc_idx].mem_heap_region;
                            obj_desc->meta_before_ptr[exp_idx].host_ptr = obj_desc->mem_ptr[alloc_idx].host_ptr + meta_byte_offset;
                            obj_desc->meta_before_ptr[exp_idx].shared_ptr = obj_desc->mem_ptr[alloc_idx].shared_ptr + meta_byte_offset;
                        }

                        if( obj_desc->params.meta_height_after > 0U )
                        {
                            meta_line_offset = obj_desc->params.meta_height_before + obj_desc->params.height;
                            meta_byte_offset = ownComputePatchOffset(0, meta_line_offset+exposure_offset,
                                                                    &obj_desc->imagepatch_addr[exp_idx]);

                            obj_desc->meta_after_ptr[exp_idx].mem_heap_region = obj_desc->mem_ptr[alloc_idx].mem_heap_region;
                            obj_desc->meta_after_ptr[exp_idx].host_ptr = obj_desc->mem_ptr[alloc_idx].host_ptr + meta_byte_offset;
                            obj_desc->meta_after_ptr[exp_idx].shared_ptr = obj_desc->mem_ptr[alloc_idx].shared_ptr + meta_byte_offset;
                        }
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_INFO, "host_ptr is NULL so setting derived pointers also to NULL!\n");

                        obj_desc->img_ptr[exp_idx].host_ptr = (uint64_t)(uintptr_t)NULL;
                        obj_desc->img_ptr[exp_idx].shared_ptr = (uint64_t)(uintptr_t)NULL;

                        obj_desc->meta_before_ptr[exp_idx].host_ptr = (uint64_t)(uintptr_t)NULL;
                        obj_desc->meta_before_ptr[exp_idx].shared_ptr = (uint64_t)(uintptr_t)NULL;

                        obj_desc->meta_after_ptr[exp_idx].host_ptr = (uint64_t)(uintptr_t)NULL;
                        obj_desc->meta_after_ptr[exp_idx].shared_ptr = (uint64_t)(uintptr_t)NULL;
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
            VX_PRINT(VX_ZONE_ERROR, "reference type is not a raw image\n");
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "reference handle is NULL!\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownAllocRawImageBuffer(vx_reference ref)
{
    tivx_obj_desc_raw_image_t *obj_desc = NULL;
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t exp_idx;

    if(ref->type == TIVX_TYPE_RAW_IMAGE)
    {
        obj_desc = (tivx_obj_desc_raw_image_t *)ref->obj_desc;

        if(obj_desc != NULL)
        {
            if ( (vx_enum)obj_desc->create_type == (vx_enum)TIVX_IMAGE_NORMAL )
            {
                for(exp_idx=0; exp_idx < obj_desc->params.num_exposures; exp_idx++)
                {
                    /* memory is not allocated, so allocate it */
                    if( (obj_desc->mem_ptr[exp_idx].host_ptr == (uint64_t)(uintptr_t)NULL) &&
                        ((obj_desc->params.line_interleaved == (vx_bool)vx_false_e) || (exp_idx == 0U))
                      )
                    {
                        tivxMemBufferAlloc(&obj_desc->mem_ptr[exp_idx], obj_desc->mem_size[exp_idx], (vx_enum)TIVX_MEM_EXTERNAL);

                        if(obj_desc->mem_ptr[exp_idx].host_ptr == (uint64_t)(uintptr_t)NULL)
                        {
                            /* could not allocate memory */
                            VX_PRINT(VX_ZONE_ERROR, "could not allocate memory\n");
                            status = (vx_status)VX_ERROR_NO_MEMORY;
                            break;
                        }
                        else
                        {
                            obj_desc->mem_ptr[exp_idx].shared_ptr =
                                tivxMemHost2SharedPtr(
                                    obj_desc->mem_ptr[exp_idx].
                                    host_ptr,
                                    (vx_enum)TIVX_MEM_EXTERNAL);
                        }
                    }
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
        VX_PRINT(VX_ZONE_ERROR, "reference type is not a raw image\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = ownDeriveRawImageBufferPointers(ref);
    }
    return status;
}

static void ownInitRawImage(tivx_raw_image image, tivx_raw_image_create_params_t *params)
{
    vx_uint16 subimage_idx, map_idx, exp_idx;
    tivx_obj_desc_raw_image_t *obj_desc = NULL;
    vx_imagepatch_addressing_t imagepatch_addr;
    vx_uint32 mem_size;
    vx_uint32 stride_y_multiplier = 1U;

    obj_desc = (tivx_obj_desc_raw_image_t *)image->base.obj_desc;

    image->parent = NULL;
    for(subimage_idx=0; subimage_idx<TIVX_RAW_IMAGE_MAX_SUBIMAGES; subimage_idx++)
    {
        image->subimages[subimage_idx] = NULL;
    }

    for(map_idx=0; map_idx<TIVX_RAW_IMAGE_MAX_MAPS; map_idx++)
    {
        image->maps[map_idx].map_addr = NULL;
        image->maps[map_idx].map_size = 0;
    }

    tivx_obj_desc_memcpy(&obj_desc->params, params, (uint32_t)sizeof(tivx_raw_image_create_params_t));

    obj_desc->valid_roi.start_x = 0;
    obj_desc->valid_roi.start_y = 0;
    obj_desc->valid_roi.end_x = params->width;
    obj_desc->valid_roi.end_y = params->height;

    /* Initialize parameters that don't change between exposure once */
    /* These will be copied from stack variable to each exposure at end of for loop below */
    imagepatch_addr.dim_x = obj_desc->params.width;
    imagepatch_addr.dim_y = obj_desc->params.height;
    imagepatch_addr.scale_x = VX_SCALE_UNITY;
    imagepatch_addr.scale_y = VX_SCALE_UNITY;
    imagepatch_addr.step_x = 1;
    imagepatch_addr.step_y = 1;

    if((vx_bool)vx_true_e == params->line_interleaved)
    {
        stride_y_multiplier = (vx_uint32)params->num_exposures;
    }

    /* Initialize per exposure settings */
    for( exp_idx = 0; exp_idx < params->num_exposures; exp_idx++)
    {
        uint32_t pixel_container = obj_desc->params.format[exp_idx].pixel_container;

        if ( (uint32_t)TIVX_RAW_IMAGE_P12_BIT != pixel_container )
        {
            imagepatch_addr.stride_x = ((vx_enum)TIVX_RAW_IMAGE_8_BIT == (vx_enum)pixel_container) ? 1 : 2;
            vx_uint32 temp_if = TIVX_ALIGN(
                        (imagepatch_addr.dim_x*(vx_uint32)imagepatch_addr.stride_x),
                        TIVX_DEFAULT_STRIDE_Y_ALIGN) * stride_y_multiplier;
            imagepatch_addr.stride_y = (vx_int32) temp_if;
        }
        else
        {
            imagepatch_addr.stride_x = 0;
            vx_uint32 temp_else = TIVX_ALIGN(
                        (((imagepatch_addr.dim_x*12U)+7U)/8U),
                        TIVX_DEFAULT_STRIDE_Y_ALIGN) * stride_y_multiplier;
            imagepatch_addr.stride_y = (vx_int32)temp_else;
        }

        mem_size = ((vx_uint32)imagepatch_addr.stride_y*(imagepatch_addr.dim_y + (params->meta_height_before + params->meta_height_after)));

        if( ((vx_bool)vx_true_e == params->line_interleaved) && (exp_idx > 0U) )
        {
            obj_desc->mem_size[exp_idx] = 0;
        }
        else
        {
            obj_desc->mem_size[exp_idx] = mem_size;
        }

        obj_desc->mem_ptr[exp_idx].mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;
        obj_desc->mem_ptr[exp_idx].host_ptr = (uint64_t)(uintptr_t)NULL;
        obj_desc->mem_ptr[exp_idx].shared_ptr = (uint64_t)(uintptr_t)NULL;

        tivx_obj_desc_memcpy(&obj_desc->imagepatch_addr[exp_idx], &imagepatch_addr, (uint32_t)sizeof(vx_imagepatch_addressing_t));
        tivx_obj_desc_memcpy(&obj_desc->img_ptr[exp_idx], &obj_desc->mem_ptr[exp_idx], (uint32_t)sizeof(tivx_shared_mem_ptr_t));
        tivx_obj_desc_memcpy(&obj_desc->meta_before_ptr[exp_idx], &obj_desc->mem_ptr[exp_idx], (uint32_t)sizeof(tivx_shared_mem_ptr_t));
        tivx_obj_desc_memcpy(&obj_desc->meta_after_ptr[exp_idx], &obj_desc->mem_ptr[exp_idx], (uint32_t)sizeof(tivx_shared_mem_ptr_t));

        image->mem_offset[exp_idx] = 0;
    }
}

static tivx_raw_image ownCreateRawImageInt(vx_context context,
                                           tivx_raw_image_create_params_t *params,
                                           tivx_image_create_type_e create_type)
{
    tivx_raw_image raw_image = NULL;
    tivx_obj_desc_raw_image_t *obj_desc = NULL;

    if (ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        if (ownIsValidCreateParams(params) == (vx_bool)vx_true_e)
        {
            raw_image = (tivx_raw_image)ownCreateReference(context, TIVX_TYPE_RAW_IMAGE, (vx_enum)VX_EXTERNAL, &context->base);

            if ( (vxGetStatus((vx_reference)raw_image) == (vx_status)VX_SUCCESS) && (raw_image->base.type == TIVX_TYPE_RAW_IMAGE) )
            {
                /* assign refernce type specific callback's */
                raw_image->base.destructor_callback = &ownDestructRawImage;
                raw_image->base.mem_alloc_callback = &ownAllocRawImageBuffer;
                raw_image->base.release_callback = (tivx_reference_release_callback_f)&tivxReleaseRawImage;

                obj_desc = (tivx_obj_desc_raw_image_t*)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_RAW_IMAGE, (vx_reference)raw_image);

                if(obj_desc == NULL)
                {
                    tivxReleaseRawImage(&raw_image);

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES, "Could not allocate raw image object descriptor\n");
                    raw_image = (tivx_raw_image)ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate raw image object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
                }
                else
                {
                    obj_desc->create_type = (uint32_t)create_type;

                    raw_image->base.obj_desc = (tivx_obj_desc_t *)obj_desc;
                    ownInitRawImage(raw_image, params);
                }
            }
        }
        else
        {
            vxAddLogEntry((vx_reference)context, (vx_status)VX_ERROR_INVALID_PARAMETERS, "Requested create parameters was invalid!\n");
            raw_image = (tivx_raw_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
        }
    }

    return raw_image;
}

static vx_status ownCopyAndMapCheckParams(
    tivx_raw_image image,
    const vx_rectangle_t* rect,
    const vx_imagepatch_addressing_t *addr,
    vx_uint32 exposure_index,
    vx_enum usage,
    vx_uint32 buffer_select)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_raw_image_t *obj_desc = NULL;

    /* bad parameters */
    if (addr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "addr is null\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    if ( (rect == NULL) && ((vx_int32)buffer_select == (vx_enum)TIVX_RAW_IMAGE_PIXEL_BUFFER) )
    {
        VX_PRINT(VX_ZONE_ERROR, "rectangle parameter is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        /* bad references */
        if ( ownIsValidRawImage(image) == (vx_bool)vx_false_e )
        {
            VX_PRINT(VX_ZONE_ERROR, "image is not valid\n");
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }
    }

    obj_desc = (tivx_obj_desc_raw_image_t *)image->base.obj_desc;
    if(status == (vx_status)VX_SUCCESS)
    {
        if((vx_int32)obj_desc->create_type == (vx_enum)TIVX_IMAGE_VIRTUAL)
        {
            VX_PRINT(VX_ZONE_ERROR, "image is virtual\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        /* more bad parameters */
        if (exposure_index >= obj_desc->params.num_exposures)
        {
            VX_PRINT(VX_ZONE_ERROR, "exposure index is greater than the image's number of exposures\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        if ((vx_int32)buffer_select == (vx_enum)TIVX_RAW_IMAGE_PIXEL_BUFFER)
        {
            if (rect->start_y >= rect->end_y)
            {
                VX_PRINT(VX_ZONE_ERROR, "image start y is greater than image end y\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            if (rect->start_x >= rect->end_x)
            {
                VX_PRINT(VX_ZONE_ERROR, "image start x is greater than image end x\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            if (rect->end_y > obj_desc->params.height)
            {
                VX_PRINT(VX_ZONE_ERROR, "image end y is greater than image height\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            if (rect->end_x > obj_desc->params.width)
            {
                VX_PRINT(VX_ZONE_ERROR, "image end x is greater than image width\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }
        if ( ((vx_int32)buffer_select > (vx_enum)TIVX_RAW_IMAGE_META_AFTER_BUFFER ) ||
            (((vx_int32)buffer_select == (vx_enum)TIVX_RAW_IMAGE_META_BEFORE_BUFFER) && (obj_desc->params.meta_height_before < 1U)) ||
            (((vx_int32)buffer_select == (vx_enum)TIVX_RAW_IMAGE_META_AFTER_BUFFER) && (obj_desc->params.meta_height_after < 1U))
           )
        {
            VX_PRINT(VX_ZONE_ERROR, "buffer_select is invalid \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        /* allocate if not already allocated */
        status = ownAllocRawImageBuffer((vx_reference)image);
        if (status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "image allocation failed\n");
        }
    }

    if(status==(vx_status)VX_SUCCESS)
    {
        if ( (image->base.is_virtual == (vx_bool)vx_true_e)
            &&
            (image->base.is_accessible == (vx_bool)vx_false_e)
            )
        {
            /* cannot be accessed by app */
            VX_PRINT(VX_ZONE_ERROR, "image cannot be accessed by application\n");
            status = (vx_status)VX_ERROR_OPTIMIZED_AWAY;
        }
    }

    return status;
}

VX_API_ENTRY tivx_raw_image VX_API_CALL tivxCreateRawImage(vx_context context, tivx_raw_image_create_params_t *params)
{
    tivx_raw_image raw_image;

    raw_image = (tivx_raw_image)ownCreateRawImageInt(context, params, TIVX_IMAGE_NORMAL);

    return raw_image;
}

VX_API_ENTRY vx_status VX_API_CALL tivxReleaseRawImage(tivx_raw_image* image)
{
    if (image != NULL)
    {
        tivx_raw_image this_image = image[0];
        if (ownIsValidSpecificReference((vx_reference)this_image, TIVX_TYPE_RAW_IMAGE) == (vx_bool)vx_true_e)
        {
            tivx_raw_image parent = this_image->parent;

            /* clear this image from its parent' subimages list */
            if ((NULL != parent) &&
                (ownIsValidSpecificReference((vx_reference)parent, TIVX_TYPE_RAW_IMAGE) ==
                    (vx_bool)vx_true_e) )
            {
                vx_uint32 subimage_idx;

                for (subimage_idx = 0; subimage_idx < TIVX_RAW_IMAGE_MAX_SUBIMAGES; subimage_idx++)
                {
                    if (parent->subimages[subimage_idx] == this_image)
                    {
                        parent->subimages[subimage_idx] = NULL;
                        break;
                    }
                }
            }
        }
    }

    return ownReleaseReferenceInt((vx_reference *)image, TIVX_TYPE_RAW_IMAGE, (vx_enum)VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_status VX_API_CALL tivxQueryRawImage(tivx_raw_image raw_image, vx_enum attribute, volatile void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_raw_image_t *obj_desc = NULL;

    if (ownIsValidRawImage(raw_image) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_raw_image_t *)raw_image->base.obj_desc;
        switch (attribute)
        {
            case (vx_enum)TIVX_RAW_IMAGE_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(volatile vx_uint32 *)ptr = obj_desc->params.width;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query raw image width failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_RAW_IMAGE_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(volatile vx_uint32 *)ptr = obj_desc->params.height;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query raw image height failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_RAW_IMAGE_NUM_EXPOSURES:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(volatile vx_uint32 *)ptr = obj_desc->params.num_exposures;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query raw image num_exposures failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_RAW_IMAGE_LINE_INTERLEAVED:
                if (VX_CHECK_PARAM(ptr, size, vx_bool, 0x3U))
                {
                    *(volatile vx_bool *)ptr = obj_desc->params.line_interleaved;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query raw image line_interleaved failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_RAW_IMAGE_FORMAT:
                if ((NULL != ptr) &&
                    (size >= sizeof(tivx_raw_image_format_t)) &&
                    (((vx_size)ptr & 0x3U) == 0U))
                {
                    vx_size num_dims = size / sizeof(tivx_raw_image_format_t);

                    if(num_dims > (vx_size)TIVX_RAW_IMAGE_MAX_EXPOSURES)
                    {
                        num_dims = (vx_size)TIVX_RAW_IMAGE_MAX_EXPOSURES;
                    }

                    tivx_obj_desc_memcpy(ptr, &obj_desc->params.format, (uint32_t)sizeof(tivx_raw_image_format_t)*(uint32_t)num_dims);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query raw image format failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_RAW_IMAGE_META_HEIGHT_BEFORE :
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(volatile vx_uint32 *)ptr = obj_desc->params.meta_height_before;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query raw image meta height before failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_RAW_IMAGE_META_HEIGHT_AFTER :
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(volatile vx_uint32 *)ptr = obj_desc->params.meta_height_after;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query raw image meta height after failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_RAW_IMAGE_IMAGEPATCH_ADDRESSING:
                if ((NULL != ptr) &&
                    (size >= sizeof(vx_imagepatch_addressing_t)) &&
                    (((vx_size)ptr & 0x3U) == 0U))
                {
                    vx_size num_dims = size / sizeof(vx_imagepatch_addressing_t);

                    if(num_dims > (vx_size)TIVX_RAW_IMAGE_MAX_EXPOSURES)
                    {
                        num_dims = (vx_size)TIVX_RAW_IMAGE_MAX_EXPOSURES;
                    }

                    tivx_obj_desc_memcpy(ptr, &obj_desc->imagepatch_addr[0], (uint32_t)sizeof(vx_imagepatch_addressing_t)*num_dims);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query raw image imagepatch addressing failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "invalid attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid image reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxCopyRawImagePatch(
    tivx_raw_image raw_image,
    const vx_rectangle_t *rect,
    vx_uint32 exposure_index,
    const vx_imagepatch_addressing_t *user_addr,
    void * user_ptr,
    vx_enum usage,
    vx_enum mem_type,
    vx_enum buffer_select)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_raw_image_t *obj_desc = NULL;
    vx_imagepatch_addressing_t *image_addr = NULL;

    if (user_ptr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "User pointer is null\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    if ((usage != (vx_enum)VX_READ_ONLY) && (usage != (vx_enum)VX_WRITE_ONLY))
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid usage parameter\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        status = ownCopyAndMapCheckParams(raw_image, rect, user_addr, exposure_index, usage, (uint32_t)buffer_select);
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        obj_desc = (tivx_obj_desc_raw_image_t *)raw_image->base.obj_desc;

        image_addr = &obj_desc->imagepatch_addr[exposure_index];

        if(buffer_select == (vx_enum)TIVX_RAW_IMAGE_PIXEL_BUFFER)
        {
            if ((rect->end_x - rect->start_x) > user_addr->dim_x)
            {
                VX_PRINT(VX_ZONE_ERROR, "rect width is greater than image dim_x\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            if ((rect->end_y - rect->start_y) > user_addr->dim_y)
            {
                VX_PRINT(VX_ZONE_ERROR, "rect height is greater than image dim_y\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            if (user_addr->dim_x > image_addr->dim_x)
            {
                VX_PRINT(VX_ZONE_ERROR, "user dim_x is greater than image dim_x\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            if (user_addr->dim_y > image_addr->dim_y)
            {
                VX_PRINT(VX_ZONE_ERROR, "user dim_y is greater than image dim_y\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            if (user_addr->stride_x < image_addr->stride_x)
            {
                VX_PRINT(VX_ZONE_ERROR, "User value for stride_x is smaller than minimum needed for image type\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        vx_uint32 x;
        vx_uint32 y;
        vx_uint8* pImagePtr = NULL;
        vx_uint8* pUserPtr = user_ptr;

        vx_uint8 *pImageLine;
        vx_uint8 *map_addr;
        vx_uint8 *pUserLine;
        vx_uint8 *pImageElem;
        vx_uint8 *pUserElem;
        uint32_t len, map_size;
        vx_uint32 start_x = 0, start_y = 0;
        vx_uint32 end_x = 0, end_y = 0;
        vx_uint32 alloc_index = exposure_index;

        switch(buffer_select)
        {
            case (vx_enum)TIVX_RAW_IMAGE_ALLOC_BUFFER:
                pImagePtr = (vx_uint8*)(uintptr_t)obj_desc->mem_ptr[exposure_index].host_ptr;
                if (pImagePtr == NULL)
                {
                    VX_PRINT(VX_ZONE_ERROR, "User mem_ptr is null at exposure_index = %d\n", exposure_index);
                    status = (vx_status)VX_FAILURE;
                }
                break;
            case (vx_enum)TIVX_RAW_IMAGE_PIXEL_BUFFER:
                pImagePtr = (vx_uint8*)(uintptr_t)obj_desc->img_ptr[exposure_index].host_ptr;
                if (pImagePtr == NULL)
                {
                    VX_PRINT(VX_ZONE_ERROR, "User img_ptr is null at exposure_index = %d\n", exposure_index);
                    status = (vx_status)VX_FAILURE;
                }
                break;
            case (vx_enum)TIVX_RAW_IMAGE_META_BEFORE_BUFFER:
                pImagePtr = (vx_uint8*)(uintptr_t)obj_desc->meta_before_ptr[exposure_index].host_ptr;
                if (pImagePtr == NULL)
                {
                    VX_PRINT(VX_ZONE_ERROR, "User meta_before_ptr is null at exposure_index = %d\n", exposure_index);
                    status = (vx_status)VX_FAILURE;
                }
                break;
            case (vx_enum)TIVX_RAW_IMAGE_META_AFTER_BUFFER:
                pImagePtr = (vx_uint8*)(uintptr_t)obj_desc->meta_after_ptr[exposure_index].host_ptr;
                if (pImagePtr == NULL)
                {
                    VX_PRINT(VX_ZONE_ERROR, "User meta_after_ptr is null at exposure_index = %d\n", exposure_index);
                    status = (vx_status)VX_FAILURE;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "invalid buffer_select\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                break;
        }

        if(status == (vx_status)VX_SUCCESS)
        {
            if (buffer_select == (vx_enum)TIVX_RAW_IMAGE_PIXEL_BUFFER)
            {
                start_x = rect->start_x;
                start_y = rect->start_y;
                end_x = rect->end_x;
                end_y = rect->end_y;
            }

            if (image_addr->stride_x == 0)
            {
                pImageLine = pImagePtr + ((start_y*(vx_uint32)image_addr->stride_y)/image_addr->step_y) + (((start_x*12UL)/8UL)/image_addr->step_x);

                if (user_addr->stride_x == 1)
                {
                    VX_PRINT(VX_ZONE_ERROR, "User value for stride_x should be 0 for packed format, or >1 for unpacked format\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
            }
            else
            {
                pImageLine = pImagePtr + ((start_y*(vx_uint32)image_addr->stride_y)/image_addr->step_y) + ((start_x*(vx_uint32)image_addr->stride_x)/image_addr->step_x);
            }
            pUserLine = pUserPtr;

            if(obj_desc->params.line_interleaved == (vx_bool)vx_true_e)
            {
                alloc_index = 0;
            }

            map_addr = (vx_uint8*)(uintptr_t)obj_desc->mem_ptr[alloc_index].host_ptr;;
            map_size = obj_desc->mem_size[alloc_index];

            if(buffer_select == (vx_enum)TIVX_RAW_IMAGE_ALLOC_BUFFER)
            {
                if (user_addr->dim_x > map_size)
                {
                    VX_PRINT(VX_ZONE_ERROR, "dim_x is greater than alloc size of buffer\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
            }
            else if (buffer_select == (vx_enum)TIVX_RAW_IMAGE_META_BEFORE_BUFFER)
            {
                if (user_addr->dim_x > (obj_desc->params.meta_height_before * (vx_uint32)image_addr->stride_y))
                {
                    VX_PRINT(VX_ZONE_ERROR, "dim_x is greater than meta buffer\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
            }
            else if (buffer_select == (vx_enum)TIVX_RAW_IMAGE_META_AFTER_BUFFER)
            {
                if (user_addr->dim_x > (obj_desc->params.meta_height_after * (vx_uint32)image_addr->stride_y))
                {
                    VX_PRINT(VX_ZONE_ERROR, "dim_x is greater than meta buffer\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
            }
            else
            {
                /* Do nothing */
            }
        }

        if(status == (vx_status)VX_SUCCESS)
        {
            tivxCheckStatus(&status, tivxMemBufferMap(map_addr, map_size, (vx_enum)VX_MEMORY_TYPE_HOST, usage));

            if (buffer_select == (vx_enum)TIVX_RAW_IMAGE_PIXEL_BUFFER)
            {
                /* copy the patch from the image */
                if (user_addr->stride_x == image_addr->stride_x)
                {
                    if(image_addr->stride_x == 0)
                    {
                        len = ((((end_x - start_x)*12U)+7U)/8U)/image_addr->step_x;
                    }
                    else
                    {
                        len = ((end_x - start_x)*(vx_uint32)image_addr->stride_x)/image_addr->step_x;
                    }

                    if(usage == (vx_enum)VX_READ_ONLY)
                    {
                        /* Both have compact lines */
                        for (y = start_y; y < end_y; y += image_addr->step_y)
                        {
                            memcpy(pUserLine, pImageLine, len);
                            pImageLine += image_addr->stride_y;
                            pUserLine += user_addr->stride_y;
                        }
                    }
                    else
                    {
                        /* Both have compact lines */
                        for (y = start_y; y < end_y; y += image_addr->step_y)
                        {
                            memcpy(pImageLine, pUserLine, len);
                            pImageLine += image_addr->stride_y;
                            pUserLine += user_addr->stride_y;
                        }
                    }
                }
                else
                {

                    len = (uint32_t)image_addr->stride_x;

                    if(usage == (vx_enum)VX_READ_ONLY)
                    {
                        if(image_addr->stride_x == 0)
                        {
                            /* The destination is not compact, we need to copy per element */
                            for (y = start_y; y < end_y; y += image_addr->step_y)
                            {
                                pImageElem = pImageLine;
                                pUserElem = pUserLine;

                                for (x = start_x; x < end_x; x += 2U)
                                {
                                    vx_uint32 *pImageElem32 = (vx_uint32*)pImageElem;
                                    vx_uint16 *pUserElem16 = (vx_uint16*)pUserElem;
                                    vx_uint32 value;

                                    value = *pImageElem32;

                                    *pUserElem16 = (uint16_t)value & (uint16_t)0xFFFU;

                                    pUserElem += user_addr->stride_x;
                                    pUserElem16 = (vx_uint16*)pUserElem;

                                    *pUserElem16 = (uint16_t)(value >> 12U) & (uint16_t)0xFFFU;

                                    pUserElem += user_addr->stride_x;
                                    pImageElem += 3;
                                }
                                pImageLine += image_addr->stride_y;
                                pUserLine += user_addr->stride_y;
                            }
                        }
                        else
                        {
                            /* The destination is not compact, we need to copy per element */
                            for (y = start_y; y < end_y; y += image_addr->step_y)
                            {
                                pImageElem = pImageLine;
                                pUserElem = pUserLine;

                                for (x = start_x; x < end_x; x += image_addr->step_x)
                                {
                                    /* One element */
                                    memcpy(pUserElem, pImageElem, len);

                                    pImageElem += len;
                                    pUserElem += user_addr->stride_x;
                                }
                                pImageLine += image_addr->stride_y;
                                pUserLine += user_addr->stride_y;
                            }
                        }
                    }
                    else
                    {
                        if(image_addr->stride_x == 0)
                        {
                            /* The destination is not compact, we need to copy per element */
                            for (y = start_y; y < end_y; y += image_addr->step_y)
                            {
                                pImageElem = pImageLine;
                                pUserElem = pUserLine;

                                for (x = start_x; x < end_x; x += 2U)
                                {
                                    vx_uint16 *pUserElem16 = (vx_uint16*)pUserElem;
                                    vx_uint32 value;

                                    value = (uint32_t)*pUserElem16 & 0xFFFU;

                                    pUserElem += user_addr->stride_x;
                                    pUserElem16 = (vx_uint16*)pUserElem;

                                    value |= ((uint32_t)*pUserElem16 & 0xFFFU)<<12;

                                    pUserElem += user_addr->stride_x;

                                    *pImageElem = (vx_uint8)(value & 0xFFU);
                                    pImageElem++;
                                    *pImageElem = (vx_uint8)(value>>8u) | (vx_uint8)((value & 0x0FU)<<4u);
                                    pImageElem++;
                                    *pImageElem = (vx_uint8)(value>>4u);
                                    pImageElem++;
                                }
                                pImageLine += image_addr->stride_y;
                                pUserLine += user_addr->stride_y;
                            }
                        }
                        else
                        {
                            /* The destination is not compact, we need to copy per element */
                            for (y = start_y; y < end_y; y += image_addr->step_y)
                            {
                                pImageElem = pImageLine;
                                pUserElem = pUserLine;

                                for (x = start_x; x < end_x; x += image_addr->step_x)
                                {
                                    /* One element */
                                    memcpy(pImageElem, pUserElem, len);

                                    pImageElem += len;
                                    pUserElem += user_addr->stride_x;
                                }
                                pImageLine += image_addr->stride_y;
                                pUserLine += user_addr->stride_y;
                            }
                        }
                    }
                }
            }
            else
            {
                if(usage == (vx_enum)VX_READ_ONLY)
                {
                    memcpy(pUserLine, pImageLine, user_addr->dim_x);
                }
                else
                {
                    memcpy(pImageLine, pUserLine, user_addr->dim_x);
                }
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(map_addr, map_size, (vx_enum)VX_MEMORY_TYPE_HOST, usage));
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxMapRawImagePatch(
    tivx_raw_image raw_image,
    const vx_rectangle_t *rect,
    vx_uint32 exposure_index,
    vx_map_id *map_id,
    vx_imagepatch_addressing_t *user_addr,
    void **user_ptr,
    vx_enum usage,
    vx_enum mem_type,
    vx_enum buffer_select)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_raw_image_t *obj_desc = NULL;

    if (map_id == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Map ID is null\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    if (user_ptr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "ptr is null\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        status = ownCopyAndMapCheckParams(raw_image, rect, (vx_imagepatch_addressing_t *)user_addr, exposure_index, usage, (uint32_t)buffer_select);
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        vx_imagepatch_addressing_t *image_addr = NULL;
        vx_uint8* map_addr = NULL, *end_addr = NULL, *host_addr = NULL;
        uint32_t map_size = 0;
        uint32_t map_idx;
        uint32_t alloc_index = exposure_index;

        obj_desc = (tivx_obj_desc_raw_image_t *)raw_image->base.obj_desc;

        image_addr = &obj_desc->imagepatch_addr[exposure_index];

        switch(buffer_select)
        {
            case (vx_enum)TIVX_RAW_IMAGE_ALLOC_BUFFER:
                map_addr = (vx_uint8*)(uintptr_t)obj_desc->mem_ptr[exposure_index].host_ptr;
                break;
            case (vx_enum)TIVX_RAW_IMAGE_PIXEL_BUFFER:
                map_addr = (vx_uint8*)(uintptr_t)obj_desc->img_ptr[exposure_index].host_ptr;
                break;
            case (vx_enum)TIVX_RAW_IMAGE_META_BEFORE_BUFFER:
                map_addr = (vx_uint8*)(uintptr_t)obj_desc->meta_before_ptr[exposure_index].host_ptr;
                break;
            case (vx_enum)TIVX_RAW_IMAGE_META_AFTER_BUFFER:
                map_addr = (vx_uint8*)(uintptr_t)obj_desc->meta_after_ptr[exposure_index].host_ptr;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "invalid buffer_select\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                break;
        }

        if(obj_desc->params.line_interleaved == (vx_bool)vx_true_e)
        {
            alloc_index = 0;
        }
        map_size = obj_desc->mem_size[alloc_index];

        if (NULL != map_addr)
        {
            host_addr = (vx_uint8*)(uintptr_t)obj_desc->mem_ptr[alloc_index].host_ptr;

            /* Move Map Pointer as per Valid ROI */
            if( buffer_select == (vx_enum)TIVX_RAW_IMAGE_PIXEL_BUFFER)
            {
                map_addr = vxFormatImagePatchAddress2d(map_addr, rect->start_x,
                    rect->start_y, image_addr);
            }

            for(map_idx=0; map_idx<TIVX_RAW_IMAGE_MAX_MAPS; map_idx++)
            {
                if(raw_image->maps[map_idx].map_addr==NULL)
                {
                    raw_image->maps[map_idx].map_addr = host_addr;
                    raw_image->maps[map_idx].map_size = map_size;
                    raw_image->maps[map_idx].mem_type = mem_type;
                    raw_image->maps[map_idx].usage = usage;
                    break;
                }
            }
            if(map_idx<TIVX_RAW_IMAGE_MAX_MAPS)
            {
                *map_id = map_idx;
                tivx_obj_desc_memcpy(user_addr, image_addr, (uint32_t)sizeof(vx_imagepatch_addressing_t));
                *user_ptr = map_addr;

                if( buffer_select == (vx_enum)TIVX_RAW_IMAGE_PIXEL_BUFFER)
                {
                    user_addr->dim_x = rect->end_x - rect->start_x;
                    user_addr->dim_y = rect->end_y - rect->start_y;
                }
                else if (buffer_select == (vx_enum)TIVX_RAW_IMAGE_META_BEFORE_BUFFER)
                {
                    user_addr->dim_x = obj_desc->params.meta_height_before * (vx_uint32)image_addr->stride_y;
                }
                else if (buffer_select == (vx_enum)TIVX_RAW_IMAGE_META_AFTER_BUFFER)
                {
                    user_addr->dim_x = obj_desc->params.meta_height_after * (vx_uint32)image_addr->stride_y;
                }
                else
                {
                    user_addr->dim_x = map_size;
                }

                end_addr = host_addr + map_size;
                map_addr = (vx_uint8*)TIVX_FLOOR((uintptr_t)host_addr, 128U);
                end_addr = (vx_uint8*)TIVX_ALIGN((uintptr_t)end_addr, 128U);
                uintptr_t temp_map_size0 = ((uintptr_t)end_addr - (uintptr_t)host_addr);
                map_size = (vx_uint32)temp_map_size0;
                tivxCheckStatus(&status, tivxMemBufferMap(map_addr, map_size, mem_type, usage));

                ownLogSetResourceUsedValue("TIVX_RAW_IMAGE_MAX_MAPS", (uint16_t)map_idx+1U);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "No available image maps\n");
                VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_RAW_IMAGE_MAX_MAPS in tiovx/include/TI/tivx_config.h\n");
                status = (vx_status)VX_ERROR_NO_RESOURCES;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "could not allocate memory\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxUnmapRawImagePatch(tivx_raw_image raw_image, vx_map_id map_id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* bad references */
    if (ownIsValidRawImage(raw_image) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid image reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        if ((raw_image->base.is_virtual == (vx_bool)vx_true_e)
            &&
            (raw_image->base.is_accessible == (vx_bool)vx_false_e)
            )
        {
            /* cannot be accessed by app */
            VX_PRINT(VX_ZONE_ERROR, "image cannot be accessed by application\n");
            status = (vx_status)VX_ERROR_OPTIMIZED_AWAY;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        if(map_id >= TIVX_RAW_IMAGE_MAX_MAPS)
        {
            VX_PRINT(VX_ZONE_ERROR, "map ID is greater than the maximum image maps\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        if( (raw_image->maps[map_id].map_addr!=NULL)
            &&
            (raw_image->maps[map_id].map_size!=0U)
            )
        {
            vx_uint8* map_addr = NULL, *end_addr = NULL;
            uint32_t map_size = 0;

            map_addr = raw_image->maps[map_id].map_addr;
            map_size = (uint32_t)raw_image->maps[map_id].map_size;

            end_addr = map_addr + map_size;
            map_addr = (vx_uint8*)TIVX_FLOOR((uintptr_t)map_addr, 128U);
            end_addr = (vx_uint8*)TIVX_ALIGN((uintptr_t)end_addr, 128U);
            uintptr_t temp_map_size1 = ((uintptr_t)end_addr - (uintptr_t)map_addr);
            map_size = (vx_uint32)temp_map_size1;

            tivxCheckStatus(&status, tivxMemBufferUnmap(
                map_addr, map_size,
                raw_image->maps[map_id].mem_type,
                raw_image->maps[map_id].usage));

            raw_image->maps[map_id].map_addr = NULL;
        }
        else
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            if(raw_image->maps[map_id].map_addr==NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "map address is null\n");
            }
            if(raw_image->maps[map_id].map_size==0U)
            {
                VX_PRINT(VX_ZONE_ERROR, "map size is equal to 0\n");
            }
        }
    }

    return status;
}
