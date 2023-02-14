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

#define TIVX_IMG_ALIGN_BYTES    (64U)
#define TIVX_IMG_ALIGN(size)    (((size + (TIVX_IMG_ALIGN_BYTES-1)) / TIVX_IMG_ALIGN_BYTES) * TIVX_IMG_ALIGN_BYTES)

static vx_bool ownIsSupportedFourcc(vx_df_image code);
static vx_bool ownIsValidImage(vx_image image);
static vx_bool ownIsOdd(vx_uint32 a);
static vx_bool ownIsValidDimensions(vx_uint32 width, vx_uint32 height, vx_df_image color);
static vx_uint32 ownComputePatchOffset(vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t* addr);
static vx_size ownSizeOfChannel(vx_df_image color);
static void ownLinkParentSubimage(vx_image parent, vx_image subimage);
static vx_status ownDestructImage(vx_reference ref);
static vx_status ownAllocImageBuffer(vx_reference ref);
static void ownInitPlane(vx_image image,
                 vx_uint32 index,
                 vx_uint32 size_of_ch,
                 vx_uint32 channels,
                 vx_uint32 width,
                 vx_uint32 height,
                 vx_uint32 step_x,
                 vx_uint32 step_y,
                 vx_uint32 bits_per_pixel  /* Valid when (size_of_ch == 0), otherwise don't care */);
static void ownInitImage(vx_image image, vx_uint32 width, vx_uint32 height, vx_df_image format);
static vx_status ownIsFreeSubimageAvailable(vx_image image);
static vx_image ownCreateImageInt(vx_context context,
                                     vx_uint32 width,
                                     vx_uint32 height,
                                     vx_df_image color,
                                     tivx_image_create_type_e create_type);
static vx_status ownCopyAndMapCheckParams(
    vx_image image,
    const vx_rectangle_t* rect,
    vx_uint32 plane_index,
    vx_enum usage);


static vx_bool ownIsSupportedFourcc(vx_df_image code)
{
    vx_bool is_supported_fourcc = (vx_bool)vx_false_e;

    switch (code)
    {
        case (vx_df_image)VX_DF_IMAGE_RGB:
        case (vx_df_image)VX_DF_IMAGE_RGBX:
        case (vx_df_image)VX_DF_IMAGE_NV12:
        case (vx_df_image)VX_DF_IMAGE_NV21:
        case (vx_df_image)VX_DF_IMAGE_UYVY:
        case (vx_df_image)VX_DF_IMAGE_YUYV:
        case (vx_df_image)VX_DF_IMAGE_IYUV:
        case (vx_df_image)VX_DF_IMAGE_YUV4:
        case (vx_df_image)VX_DF_IMAGE_U8:
        case (vx_df_image)VX_DF_IMAGE_U16:
        case (vx_df_image)VX_DF_IMAGE_S16:
        case (vx_df_image)VX_DF_IMAGE_U32:
        case (vx_df_image)VX_DF_IMAGE_S32:
        case (vx_df_image)VX_DF_IMAGE_VIRT:
        case (vx_df_image)TIVX_DF_IMAGE_P12:
        case (vx_df_image)TIVX_DF_IMAGE_NV12_P12:
        case (vx_df_image)TIVX_DF_IMAGE_RGB565:
        case (vx_df_image)TIVX_DF_IMAGE_BGRX:
            is_supported_fourcc = (vx_bool)vx_true_e;
            break;
        default:
            is_supported_fourcc = (vx_bool)vx_false_e;
            break;
    }

    return is_supported_fourcc;
}

static vx_bool ownIsValidImage(vx_image image)
{
    vx_bool is_valid;

    if ((ownIsValidSpecificReference((vx_reference)image, (vx_enum)VX_TYPE_IMAGE) == (vx_bool)vx_true_e) &&
        (image->base.obj_desc != NULL) &&
        (ownIsSupportedFourcc(((tivx_obj_desc_image_t*)image->base.obj_desc)->
            format) == (vx_bool)vx_true_e)
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

static vx_bool ownIsOdd(vx_uint32 a)
{
    vx_bool isOdd;
    if ((a & 0x1U) != 0U)
    {
        isOdd = (vx_bool)vx_true_e;
    }
    else
    {
        isOdd = (vx_bool)vx_false_e;
    }

    return (isOdd);
}

static vx_bool ownIsValidDimensions(vx_uint32 width, vx_uint32 height, vx_df_image color)
{
    vx_bool is_valid = (vx_bool)vx_true_e;

    if ((ownIsOdd(width) != (vx_bool)vx_false_e) && ( (color == (vx_df_image)VX_DF_IMAGE_UYVY) || (color == (vx_df_image)VX_DF_IMAGE_YUYV)))
    {
        is_valid = (vx_bool)vx_false_e;
    }
    else if (((ownIsOdd(width) != (vx_bool)vx_false_e) || (ownIsOdd(height) != (vx_bool)vx_false_e)) &&
              ((color == (vx_df_image)VX_DF_IMAGE_IYUV) || (color == (vx_df_image)VX_DF_IMAGE_NV12) || (color == (vx_df_image)VX_DF_IMAGE_NV21)))
    {
        is_valid = (vx_bool)vx_false_e;
    }
    else
    {
        /* do nothing as is_valid is already initialized */
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
        offset = ((vx_uint32)addr->stride_y * (y / addr->step_y)) +
                 ((12U * (x / addr->step_x))/8U);
    }
    else
    {
        offset = ((vx_uint32)addr->stride_y * (y / addr->step_y)) +
                 ((vx_uint32)addr->stride_x * (x / addr->step_x));
    }

    return offset;
}


static vx_size ownSizeOfChannel(vx_df_image color)
{
    vx_size size = 0U;
    if (ownIsSupportedFourcc(color) != 0)
    {
        switch (color)
        {
            case (vx_df_image)TIVX_DF_IMAGE_RGB565:
            case (vx_df_image)VX_DF_IMAGE_S16:
            case (vx_df_image)VX_DF_IMAGE_U16:
                size = sizeof(vx_uint16);
                break;
            case (vx_df_image)VX_DF_IMAGE_U32:
            case (vx_df_image)VX_DF_IMAGE_S32:
                size = sizeof(vx_uint32);
                break;
            case (vx_df_image)TIVX_DF_IMAGE_P12:
            case (vx_df_image)TIVX_DF_IMAGE_NV12_P12:
                size = 0U; /* Special case for (bits per pixel % 8) != 0 */
                break;
            default:
                size = 1U;
                break;
        }
    }
    return size;
}

static void ownLinkParentSubimage(vx_image parent, vx_image subimage)
{
    uint16_t p;

    /* remember that the scope of the subimage is the parent image */
    ownReferenceSetScope(&subimage->base, &parent->base);

    /* refer to our parent image and internally refcount it */
    subimage->parent = parent;

    /* it will find free space for subimage since this was checked before */
    for (p = 0; p < TIVX_IMAGE_MAX_SUBIMAGES; p++)
    {
        if (parent->subimages[p] == NULL)
        {
            parent->subimages[p] = subimage;
            ownLogSetResourceUsedValue("TIVX_IMAGE_MAX_SUBIMAGES", (uint16_t)p+1U);
            break;
        }
    }

    if (p == TIVX_IMAGE_MAX_SUBIMAGES)
    {
        VX_PRINT(VX_ZONE_WARNING, "May need to increase the value of TIVX_IMAGE_MAX_SUBIMAGES in tiovx/include/TI/tivx_config.h\n");
    }

    ownIncrementReference(&parent->base, (vx_enum)VX_INTERNAL);
}

static vx_status ownDestructImage(vx_reference ref)
{
    tivx_obj_desc_image_t *obj_desc = NULL;
    uint16_t plane_idx;
    vx_image image = (vx_image)ref;
    uint32_t size = 0;

    if(ref->type == (vx_enum)VX_TYPE_IMAGE)
    {
        obj_desc = (tivx_obj_desc_image_t *)ref->obj_desc;

        if(obj_desc!=NULL)
        {
            if ( ((vx_enum)obj_desc->create_type == (vx_enum)TIVX_IMAGE_NORMAL)
                || ((vx_enum)obj_desc->create_type == (vx_enum)TIVX_IMAGE_UNIFORM)
             )
            {
                if(obj_desc->mem_ptr[0].host_ptr!=(uint64_t)(uintptr_t) NULL)
                {
                    for(plane_idx=0; plane_idx<obj_desc->planes-1; plane_idx++)
                    {
                        size += TIVX_IMG_ALIGN(obj_desc->mem_size[plane_idx]);
                    }
                    size += obj_desc->mem_size[plane_idx];

                    tivxMemBufferFree(&obj_desc->mem_ptr[0], size);
                }
            }
            ownObjDescFree((tivx_obj_desc_t**)&obj_desc);
        }
        if (NULL != image->parent)
        {
            ownReleaseReferenceInt((vx_reference *)&image->parent, (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_INTERNAL, NULL);
        }
    }
    return (vx_status)VX_SUCCESS;
}

static vx_status ownAllocImageBuffer(vx_reference ref)
{
    tivx_obj_desc_image_t *obj_desc = NULL;
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t plane_idx;
    uint32_t size = 0;

    if(ref->type == (vx_enum)VX_TYPE_IMAGE)
    {
        obj_desc = (tivx_obj_desc_image_t *)ref->obj_desc;

        if(obj_desc != NULL)
        {
            if( ((vx_enum)obj_desc->create_type == (vx_enum)TIVX_IMAGE_NORMAL)
            || ((vx_enum)obj_desc->create_type == (vx_enum)TIVX_IMAGE_UNIFORM)
             )
            {
                /* We will allocate all planes in a single buffer and multiple plane pointers,
                 * with appropriate alignment between them to account for HWA requirements.
                 * This is required for Video Codec in PSDK 7.3 to have NV12 buffers be contiguous. */
                for(plane_idx=0; plane_idx<obj_desc->planes-1; plane_idx++)
                {
                    size += TIVX_IMG_ALIGN(obj_desc->mem_size[plane_idx]);
                }
                size += obj_desc->mem_size[plane_idx];

                /* memory is not allocated, so allocate it */
                if(obj_desc->mem_ptr[0].host_ptr==(uint64_t)(uintptr_t)NULL)
                {
                    tivxMemBufferAlloc(&obj_desc->mem_ptr[0], size, (vx_enum)TIVX_MEM_EXTERNAL);

                    if(obj_desc->mem_ptr[0].host_ptr==(uint64_t)(uintptr_t)NULL)
                    {
                        /* could not allocate memory */
                        VX_PRINT(VX_ZONE_ERROR, "could not allocate memory\n");
                        status = (vx_status)VX_ERROR_NO_MEMORY;
                    }
                    else
                    {
                        obj_desc->mem_ptr[plane_idx].shared_ptr =
                            tivxMemHost2SharedPtr(
                                obj_desc->mem_ptr[0].host_ptr,
                                (vx_enum)TIVX_MEM_EXTERNAL);
                        for(plane_idx=1; plane_idx<obj_desc->planes; plane_idx++)
                        {
                            obj_desc->mem_ptr[plane_idx].mem_heap_region =
                                obj_desc->mem_ptr[plane_idx-1].mem_heap_region;
                            obj_desc->mem_ptr[plane_idx].host_ptr =
                                obj_desc->mem_ptr[plane_idx-1].host_ptr +
                                obj_desc->mem_size[plane_idx-1];
                            obj_desc->mem_ptr[plane_idx].shared_ptr =
                                tivxMemHost2SharedPtr(
                                    obj_desc->mem_ptr[plane_idx].host_ptr,
                                    (vx_enum)TIVX_MEM_EXTERNAL);
                            obj_desc->mem_ptr[plane_idx].dma_buf_fd =
                                obj_desc->mem_ptr[plane_idx-1].dma_buf_fd;
                            obj_desc->mem_ptr[plane_idx].dma_buf_fd_offset =
                                obj_desc->mem_ptr[plane_idx-1].dma_buf_fd_offset +
                                TIVX_IMG_ALIGN(obj_desc->mem_size[plane_idx-1]);
                        }
                    }
                }
            }
            else
            {
                /* NOT an error since memory allocation not needed for other create types */
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
        VX_PRINT(VX_ZONE_ERROR, "reference type is not an image\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static void ownInitPlane(vx_image image,
                 vx_uint32 index,
                 vx_uint32 size_of_ch,
                 vx_uint32 channels,
                 vx_uint32 width,
                 vx_uint32 height,
                 vx_uint32 step_x,
                 vx_uint32 step_y,
                 vx_uint32 bits_per_pixel  /* Valid when (size_of_ch == 0), otherwise don't care */
                 )
{
    vx_imagepatch_addressing_t *imagepatch_addr;
    uint32_t mem_size;
    tivx_obj_desc_image_t *obj_desc = NULL;

    if (image != NULL)
    {
        obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;

        imagepatch_addr = &obj_desc->imagepatch_addr[index];

        imagepatch_addr->dim_x = width;
        imagepatch_addr->dim_y = height;
        imagepatch_addr->stride_x = (vx_int32)size_of_ch*(vx_int32)channels;
        if ( size_of_ch != 0U )
        {
            vx_uint32 temp_if = TIVX_ALIGN(
                        (imagepatch_addr->dim_x*(vx_uint32)imagepatch_addr->stride_x)/step_x,
                        TIVX_DEFAULT_STRIDE_Y_ALIGN);
            imagepatch_addr->stride_y = (vx_int32)temp_if;
        }
        else /* Only for P12 and NV12_P12 */
        {
            vx_uint32 temp_else = TIVX_ALIGN(
                        (((imagepatch_addr->dim_x*(vx_uint32)bits_per_pixel)+7U)/8U),
                        TIVX_DEFAULT_STRIDE_Y_ALIGN);
            imagepatch_addr->stride_y = (vx_int32)temp_else;
        }
        imagepatch_addr->scale_x = VX_SCALE_UNITY/step_x;
        imagepatch_addr->scale_y = VX_SCALE_UNITY/step_y;
        imagepatch_addr->step_x = step_x;
        imagepatch_addr->step_y = step_y;

        mem_size = ((vx_uint32)imagepatch_addr->stride_y*imagepatch_addr->dim_y)/step_y;

        obj_desc->mem_size[index] = mem_size;

        obj_desc->mem_ptr[index].mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;
        obj_desc->mem_ptr[index].host_ptr = (uint64_t)(uintptr_t)NULL;
        obj_desc->mem_ptr[index].shared_ptr = (uint64_t)(uintptr_t)NULL;

        image->mem_offset[index] = 0;
    }
}

static void ownInitImage(vx_image image, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    vx_uint32 size_of_ch;
    vx_uint16 subimage_idx, map_idx;
    tivx_obj_desc_image_t *obj_desc = NULL;

    obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;
    size_of_ch = (vx_uint32)ownSizeOfChannel(format);

    image->parent = NULL;
    for(subimage_idx=0; subimage_idx<TIVX_IMAGE_MAX_SUBIMAGES; subimage_idx++)
    {
        image->subimages[subimage_idx] = NULL;
    }

    for(map_idx=0; map_idx<TIVX_IMAGE_MAX_MAPS; map_idx++)
    {
        image->maps[map_idx].map_addr = NULL;
        image->maps[map_idx].map_size = 0;
    }
    image->channel_plane = 0;

    obj_desc->uniform_image_pixel_value = 0;
    obj_desc->width = width;
    obj_desc->height = height;
    obj_desc->format = format;
    obj_desc->color_range = (vx_enum)VX_CHANNEL_RANGE_FULL;


    obj_desc->valid_roi.start_x = 0;
    obj_desc->valid_roi.start_y = 0;
    obj_desc->valid_roi.end_x = width;
    obj_desc->valid_roi.end_y = height;

    switch (format)
    {
        case (vx_df_image)VX_DF_IMAGE_U8:
        case (vx_df_image)VX_DF_IMAGE_U16:
        case (vx_df_image)VX_DF_IMAGE_U32:
        case (vx_df_image)VX_DF_IMAGE_S16:
        case (vx_df_image)VX_DF_IMAGE_S32:
        case (vx_df_image)TIVX_DF_IMAGE_RGB565:
        case (vx_df_image)TIVX_DF_IMAGE_P12:
            obj_desc->color_space = (vx_enum)VX_COLOR_SPACE_NONE;
            break;
        default:
            obj_desc->color_space = (vx_enum)VX_COLOR_SPACE_DEFAULT;
            break;
    }

    switch (format)
    {
        case (vx_df_image)VX_DF_IMAGE_VIRT:
            obj_desc->planes = 0;
            break;
        case (vx_df_image)VX_DF_IMAGE_NV12:
        case (vx_df_image)VX_DF_IMAGE_NV21:
            obj_desc->planes = 2;
            ownInitPlane(image, 0, size_of_ch, 1, obj_desc->width, obj_desc->height, 1, 1, 0);
            ownInitPlane(image, 1, size_of_ch, 2, obj_desc->width, obj_desc->height, 2, 2, 0);
            break;
        case (vx_df_image)VX_DF_IMAGE_RGB:
            obj_desc->planes = 1;
            ownInitPlane(image, 0, size_of_ch, 3, obj_desc->width, obj_desc->height, 1, 1, 0);
            break;
        case (vx_df_image)TIVX_DF_IMAGE_BGRX:
        case (vx_df_image)VX_DF_IMAGE_RGBX:
            obj_desc->planes = 1;
            ownInitPlane(image, 0, size_of_ch, 4, obj_desc->width, obj_desc->height, 1, 1, 0);
            break;
        case (vx_df_image)VX_DF_IMAGE_UYVY:
        case (vx_df_image)VX_DF_IMAGE_YUYV:
            obj_desc->planes = 1;
            ownInitPlane(image, 0, size_of_ch, 2, obj_desc->width, obj_desc->height, 1, 1, 0);
            break;
        case (vx_df_image)VX_DF_IMAGE_YUV4:
            obj_desc->planes = 3;
            ownInitPlane(image, 0, size_of_ch, 1, obj_desc->width, obj_desc->height, 1, 1, 0);
            ownInitPlane(image, 1, size_of_ch, 1, obj_desc->width, obj_desc->height, 1, 1, 0);
            ownInitPlane(image, 2, size_of_ch, 1, obj_desc->width, obj_desc->height, 1, 1, 0);
            break;
        case (vx_df_image)VX_DF_IMAGE_IYUV:
            obj_desc->planes = 3;
            ownInitPlane(image, 0, size_of_ch, 1, obj_desc->width, obj_desc->height, 1, 1, 0);
            ownInitPlane(image, 1, size_of_ch, 1, obj_desc->width, obj_desc->height, 2, 2, 0);
            ownInitPlane(image, 2, size_of_ch, 1, obj_desc->width, obj_desc->height, 2, 2, 0);
            break;
        case (vx_df_image)VX_DF_IMAGE_U8:
        case (vx_df_image)VX_DF_IMAGE_U16:
        case (vx_df_image)VX_DF_IMAGE_S16:
        case (vx_df_image)VX_DF_IMAGE_U32:
        case (vx_df_image)VX_DF_IMAGE_S32:
        case (vx_df_image)TIVX_DF_IMAGE_RGB565:
            obj_desc->planes = 1;
            ownInitPlane(image, 0, size_of_ch, 1, obj_desc->width, obj_desc->height, 1, 1, 0);
            break;
        case (vx_df_image)TIVX_DF_IMAGE_P12:
            obj_desc->planes = 1;
            ownInitPlane(image, 0, 0, 1, obj_desc->width, obj_desc->height, 1, 1, 12);
            break;
        case (vx_df_image)TIVX_DF_IMAGE_NV12_P12:
            obj_desc->planes = 2;
            ownInitPlane(image, 0, 0, 1, obj_desc->width, obj_desc->height, 1, 1, 12);
            ownInitPlane(image, 1, 0, 2, obj_desc->width, obj_desc->height, 2, 2, 12);
            break;
        default:
            /*! should not get here unless there's a bug in the
             * ownIsSupportedFourcc call.
             */
            vxAddLogEntry((vx_reference)image, (vx_status)VX_ERROR_INVALID_PARAMETERS, "FourCC format is invalid!\n");
            break;
    }
}

static vx_status ownIsFreeSubimageAvailable(vx_image image)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t p;

    /* check if image can contain subimage */
    for (p = 0; p < TIVX_IMAGE_MAX_SUBIMAGES; p++)
    {
        if (image->subimages[p] == NULL)
        {
            break;
        }
    }
    if(p>=TIVX_IMAGE_MAX_SUBIMAGES)
    {
        VX_PRINT(VX_ZONE_ERROR, "no subimage is available\n");
        status = (vx_status)VX_ERROR_NO_RESOURCES;
    }

    return status;
}

static vx_image ownCreateImageInt(vx_context context,
                                     vx_uint32 width,
                                     vx_uint32 height,
                                     vx_df_image color,
                                     tivx_image_create_type_e create_type)
{
    vx_image image = NULL;
    tivx_obj_desc_image_t *obj_desc = NULL;

    if (ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        if (ownIsSupportedFourcc(color) == (vx_bool)vx_true_e)
        {
            if (ownIsValidDimensions(width, height, color) == (vx_bool)vx_true_e)
            {
                image = (vx_image)ownCreateReference(context, (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_EXTERNAL, &context->base);
                if ( (vxGetStatus((vx_reference)image) == (vx_status)VX_SUCCESS) && (image->base.type == (vx_enum)VX_TYPE_IMAGE) )
                {
                    /* assign refernce type specific callback's */
                    image->base.destructor_callback = &ownDestructImage;
                    image->base.mem_alloc_callback = &ownAllocImageBuffer;
                    image->base.release_callback = (tivx_reference_release_callback_f)&vxReleaseImage;

                    obj_desc = (tivx_obj_desc_image_t*)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_IMAGE, (vx_reference)image);

                    if(obj_desc == NULL)
                    {
                        vxReleaseImage(&image);

                        vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES, "Could not allocate image object descriptor\n");
                        image = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
                        VX_PRINT(VX_ZONE_ERROR, "Could not allocate image object descriptor\n");
                        VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                        VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
                    }
                    else
                    {
                        obj_desc->create_type = (vx_uint32)create_type;

                        image->base.obj_desc = (tivx_obj_desc_t *)obj_desc;
                        ownInitImage(image, width, height, color);
                    }
                }
            }
            else
            {
                vxAddLogEntry((vx_reference)context, (vx_status)VX_ERROR_INVALID_DIMENSION, "Requested Image Dimensions was invalid!\n");
                image = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_DIMENSION);
            }
        }
        else
        {
            vxAddLogEntry((vx_reference)context, (vx_status)VX_ERROR_INVALID_FORMAT, "Requested Image Format was invalid!\n");
            image = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_FORMAT);
        }
    }

    return image;
}

static vx_status ownCopyAndMapCheckParams(
    vx_image image,
    const vx_rectangle_t* rect,
    vx_uint32 plane_index,
    vx_enum usage)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 start_x = rect ? rect->start_x : 0u;
    vx_uint32 start_y = rect ? rect->start_y : 0u;
    vx_uint32 end_x = rect ? rect->end_x : 0u;
    vx_uint32 end_y = rect ? rect->end_y : 0u;
    tivx_obj_desc_image_t *obj_desc = NULL;

    /* bad parameters */
    if ( rect == NULL )
    {
        VX_PRINT(VX_ZONE_ERROR, "rectangle parameter is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        /* bad references */
        if ( ownIsValidImage(image) == (vx_bool)vx_false_e )
        {
            VX_PRINT(VX_ZONE_ERROR, "image is not valid\n");
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }
    }

    obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;
    if(status == (vx_status)VX_SUCCESS)
    {
        if((vx_enum)obj_desc->create_type == (vx_enum)TIVX_IMAGE_VIRTUAL)
        {
            VX_PRINT(VX_ZONE_ERROR, "image is virtual\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        /* more bad parameters */
        if (plane_index >= obj_desc->planes)
        {
            VX_PRINT(VX_ZONE_ERROR, "plane index is greater than the image's number of planes\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        if (start_y >= end_y)
        {
            VX_PRINT(VX_ZONE_ERROR, "image start y is greater than image end y\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        if (start_x >= end_x)
        {
            VX_PRINT(VX_ZONE_ERROR, "image start x is greater than image end x\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        /* allocate if not already allocated */
        status = ownAllocImageBuffer((vx_reference)image);
        if (status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "image allocation failed\n");
        }
    }

    if(status==(vx_status)VX_SUCCESS)
    {
        if ( ((vx_enum)obj_desc->create_type == (vx_enum)TIVX_IMAGE_UNIFORM) && ( ((vx_enum)usage == (vx_enum)VX_WRITE_ONLY) || ((vx_enum)usage == (vx_enum)VX_READ_AND_WRITE) ) )
        {
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
            vxAddLogEntry(&image->base, status, "Can't write to constant data, only read!\n");
            VX_PRINT(VX_ZONE_ERROR, "Can't write to constant data, only read!\n");
        }
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

void ownPrintImageAddressing(const vx_imagepatch_addressing_t *addr)
{
    if (addr != NULL)
    {
        VX_PRINT(VX_ZONE_IMAGE, "dim={%u,%u} stride={%d,%d} scale={%u,%u} step={%u,%u}\n",
                addr->dim_x, addr->dim_y,
                addr->stride_x, addr->stride_y,
                addr->scale_x, addr->scale_y,
                addr->step_x, addr->step_y);
    }
}

void ownPrintImage(vx_image image)
{
    vx_uint32 p = 0;
    volatile vx_char df_image[5];
    tivx_obj_desc_image_t *obj_desc = NULL;

    obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;

    tivx_obj_desc_strncpy(df_image, (volatile char *)&obj_desc->format, 4);
    df_image[4] = '\0';
    ownPrintReference(&image->base);
    VX_PRINT(VX_ZONE_IMAGE,
            "vx_image:%s %ux%u (%s), planes:%d\n",
            df_image,
            obj_desc->width,
            obj_desc->height,
            ((vx_enum)obj_desc->create_type == (vx_enum)TIVX_IMAGE_UNIFORM)?"CONSTANT":"MUTABLE",
            obj_desc->planes
        );
    VX_PRINT(VX_ZONE_IMAGE,"\n");
    for (p = 0; p < obj_desc->planes; p++)
    {
        VX_PRINT(VX_ZONE_IMAGE,"Plane %d: host_ptr:%p, shared_ptr:%p, mem_size:%d B\n",
            p,
            (void*)(uintptr_t)obj_desc->mem_ptr[p].host_ptr,
            (void*)(uintptr_t)obj_desc->mem_ptr[p].shared_ptr,
            obj_desc->mem_size[p]);
        ownPrintImageAddressing(&obj_desc->imagepatch_addr[p]);
        VX_PRINT(VX_ZONE_IMAGE,"\n");
    }
}


VX_API_ENTRY vx_image VX_API_CALL vxCreateImage(vx_context context, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    vx_image image;

    if ((width == 0U) || (height == 0U) ||
        (ownIsSupportedFourcc(format) == (vx_bool)vx_false_e) || (format == (vx_df_image)VX_DF_IMAGE_VIRT))
    {
        image = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
    }
    else
    {
        image = (vx_image)ownCreateImageInt(context, width, height, format, TIVX_IMAGE_NORMAL);
    }

    return image;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateImageFromHandle(vx_context context, vx_df_image color, const vx_imagepatch_addressing_t addrs[], void *const ptrs[], vx_enum memory_type)
{
    vx_image image = 0;
    vx_imagepatch_addressing_t *imagepatch_addr;
    tivx_shared_mem_ptr_t *mem_ptr;
    tivx_obj_desc_image_t *obj_desc = NULL;

    if ((addrs[0].dim_x == 0U) || (addrs[0].dim_y == 0U) ||
        (ownIsSupportedFourcc(color) == (vx_bool)vx_false_e) || (color == (vx_df_image)VX_DF_IMAGE_VIRT))
    {
        image = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
    }
    else
    {
        image = (vx_image)ownCreateImageInt(context, addrs[0].dim_x, addrs[0].dim_y, color, TIVX_IMAGE_FROM_HANDLE);

        if ( (vxGetStatus((vx_reference)image) == (vx_status)VX_SUCCESS) && (image->base.type == (vx_enum)VX_TYPE_IMAGE) )
        {
            vx_uint32 plane_idx = 0;
            vx_status status = (vx_status)VX_SUCCESS;

            obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;

            /* now assign the plane pointers, assume linearity */
            for (plane_idx = 0; (plane_idx < obj_desc->planes) && (status == (vx_status)VX_SUCCESS); plane_idx++)
            {
                /* ensure row-major memory layout */
                if ((color == (vx_df_image)TIVX_DF_IMAGE_P12) || (color == (vx_df_image)TIVX_DF_IMAGE_NV12_P12))
                {
                    if((addrs[plane_idx].stride_x != 0) || (addrs[plane_idx].stride_y < ((((vx_int32)addrs[plane_idx].dim_x * 12)+7)/8)) )
                    {
                        vxReleaseImage(&image);
                        image = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
                        status = (vx_status)VX_FAILURE;
                    }

                }
                else
                {
                    if((addrs[plane_idx].stride_x <= 0) || (addrs[plane_idx].stride_y < (addrs[plane_idx].stride_x * (vx_int32)addrs[plane_idx].dim_x) ) )
                    {
                        vxReleaseImage(&image);
                        image = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
                        status = (vx_status)VX_FAILURE;
                    }
                }

                if(status == (vx_status)VX_SUCCESS)
                {
                    imagepatch_addr = &obj_desc->imagepatch_addr[plane_idx];
                    mem_ptr = &obj_desc->mem_ptr[plane_idx];

                    imagepatch_addr->stride_x = addrs[plane_idx].stride_x;
                    imagepatch_addr->stride_y = addrs[plane_idx].stride_y;

                    obj_desc->mem_size[plane_idx] = ((vx_uint32)imagepatch_addr->stride_y*imagepatch_addr->dim_y)/imagepatch_addr->step_y;

                    mem_ptr->mem_heap_region =  (vx_enum)TIVX_MEM_EXTERNAL;
                    mem_ptr->host_ptr = (uint64_t)(uintptr_t)ptrs[plane_idx];
                    if(mem_ptr->host_ptr!=(uint64_t)(uintptr_t)NULL)
                    {
                        /* ptrs[plane_idx] can be NULL */
                        mem_ptr->shared_ptr = tivxMemHost2SharedPtr(mem_ptr->host_ptr, (vx_enum)TIVX_MEM_EXTERNAL);

                        tivxCheckStatus(&status, tivxMemBufferUnmap((void*)(uintptr_t)mem_ptr->host_ptr,
                            obj_desc->mem_size[plane_idx], (vx_enum)TIVX_MEM_EXTERNAL,
                            (vx_enum)VX_WRITE_ONLY));
                    }
                }
            }
        }
    }

    return image;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateImageFromChannel(vx_image image, vx_enum channel)
{
    vx_image subimage = NULL;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t width, height;
    vx_enum format, subimage_format;
    uint16_t channel_plane;
    vx_context context;
    tivx_obj_desc_image_t *obj_desc = NULL, *si_obj_desc = NULL;

    if (ownIsValidImage(image) == (vx_bool)vx_true_e)
    {
        context = vxGetContext((vx_reference)image);

        obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;
        /* perhaps the parent hasn't been allocated yet? */
        if(ownAllocImageBuffer((vx_reference)image)==(vx_status)VX_SUCCESS)
        {
            format = (vx_enum)obj_desc->format;

            /* check for valid parameters */
            switch (channel)
            {
                case (vx_enum)VX_CHANNEL_Y:
                {
                    if (((vx_enum)VX_DF_IMAGE_YUV4 != format) &&
                        ((vx_enum)VX_DF_IMAGE_IYUV != format) &&
                        ((vx_enum)VX_DF_IMAGE_NV12 != format) &&
                        ((vx_enum)VX_DF_IMAGE_NV21 != format) )
                    {
                        subimage = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
                        VX_PRINT(VX_ZONE_ERROR, "invalid image format for Y channel\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                    break;
                }

                case (vx_enum)VX_CHANNEL_U:
                case (vx_enum)VX_CHANNEL_V:
                {
                    if (((vx_enum)VX_DF_IMAGE_YUV4 != format) &&
                        ((vx_enum)VX_DF_IMAGE_IYUV != format))
                    {
                        subimage = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
                        VX_PRINT(VX_ZONE_ERROR, "invalid image format for U/V channel\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                    break;
                }

                default:
                {
                    subimage = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
                    VX_PRINT(VX_ZONE_ERROR, "invalid image channel\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    break;
                }
            }

            if(status==(vx_status)VX_SUCCESS)
            {
                status = ownIsFreeSubimageAvailable(image);
                if(status!=(vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "no subimage is available\n");
                    subimage = (vx_image)ownGetErrorObject(context, status);
                }
            }

            if(status==(vx_status)VX_SUCCESS)
            {
                vx_imagepatch_addressing_t *imagepatch_addr;
                tivx_shared_mem_ptr_t *mem_ptr;

                /* plane index */
                channel_plane = ((vx_enum)VX_CHANNEL_Y == channel) ? (uint16_t)0U : (((vx_enum)VX_CHANNEL_U == channel) ? (uint16_t)1U : (uint16_t)2U);

                imagepatch_addr = &obj_desc->imagepatch_addr[channel_plane];
                mem_ptr = &obj_desc->mem_ptr[channel_plane];

                subimage_format = (vx_enum)VX_DF_IMAGE_U8;

                width = 0;
                height = 0;

                switch (obj_desc->format)
                {
                    case (vx_df_image)VX_DF_IMAGE_YUV4:
                    {
                        width = imagepatch_addr->dim_x;
                        height = imagepatch_addr->dim_y;
                        break;
                    }

                    case (vx_df_image)VX_DF_IMAGE_IYUV:
                    {
                        if(channel_plane==0U)
                        {
                            width = imagepatch_addr->dim_x;
                            height = imagepatch_addr->dim_y;
                        }
                        else
                        {
                            width = imagepatch_addr->dim_x/2U;
                            height = imagepatch_addr->dim_y/2U;
                        }
                        break;
                    }
                    case (vx_df_image)VX_DF_IMAGE_NV12:
                    case (vx_df_image)VX_DF_IMAGE_NV21:
                    {
                        if(channel_plane==0U)
                        {
                            width = imagepatch_addr->dim_x;
                            height = imagepatch_addr->dim_y;
                        }
                        else
                        {
                            width = imagepatch_addr->dim_x;
                            height = imagepatch_addr->dim_y/2U;
                        }
                        break;
                    }
                    default:
                        break;
                }

                subimage = (vx_image)ownCreateImageInt(context, width, height, (uint32_t)subimage_format, TIVX_IMAGE_FROM_CHANNEL);

                if ((vxGetStatus((vx_reference)subimage) == (vx_status)VX_SUCCESS) && (subimage->base.type == (vx_enum)VX_TYPE_IMAGE))
                {
                    ownLinkParentSubimage(image, subimage);

                    si_obj_desc = (tivx_obj_desc_image_t *)subimage->base.obj_desc;

                    si_obj_desc->imagepatch_addr[0].stride_x = imagepatch_addr->stride_x;
                    si_obj_desc->imagepatch_addr[0].stride_y = imagepatch_addr->stride_y;
                    /* TIOVX-742 */
                    if((format==(vx_enum)VX_DF_IMAGE_NV12)
                        ||
                       (format==(vx_enum)VX_DF_IMAGE_NV21)
                    )
                    {
                        /* if UV plane in YUV420SP format, then stride_x should stride_x/2 */
                        if(channel_plane==1U)
                        {
                            si_obj_desc->imagepatch_addr[0].stride_x
                                = imagepatch_addr->stride_x/2;
                        }
                    }
                    subimage->channel_plane = channel_plane;
                    si_obj_desc->mem_ptr[0] = *mem_ptr;
                    si_obj_desc->mem_size[0] = obj_desc->mem_size[channel_plane];
                }
            }
        }
    }

    return subimage;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateImageFromROI(vx_image image, const vx_rectangle_t* rect)
{
    vx_image subimage = NULL;
    vx_context context;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t width, height, plane_idx;
    vx_enum format;
    vx_imagepatch_addressing_t *subimage_imagepatch_addr;
    vx_imagepatch_addressing_t *image_imagepatch_addr;
    tivx_shared_mem_ptr_t *subimage_mem_ptr;
    tivx_shared_mem_ptr_t *image_mem_ptr;
    uint32_t mem_size;
    tivx_obj_desc_image_t *obj_desc = NULL, *si_obj_desc = NULL;

    if (ownIsValidImage(image) == (vx_bool)vx_true_e)
    {
        context = vxGetContext((vx_reference)image);

        obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;

        if ((NULL == rect) ||
            (rect->start_x > rect->end_x) ||
            (rect->start_y > rect->end_y) ||
            (rect->end_x > obj_desc->width) ||
            (rect->end_y > obj_desc->height))
        {
            subimage = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
        }
        else
        {
            /* perhaps the parent hasn't been allocated yet? */
            if(ownAllocImageBuffer((vx_reference)image)==(vx_status)VX_SUCCESS)
            {
                format = (vx_enum)obj_desc->format;
                width  = rect->end_x - rect->start_x;
                height = rect->end_y - rect->start_y;

                status = ownIsFreeSubimageAvailable(image);
                if(status!=(vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "no subimage is available\n");
                    subimage = (vx_image)ownGetErrorObject(context, status);
                }

                if(status==(vx_status)VX_SUCCESS)
                {
                    subimage = (vx_image)ownCreateImageInt(context, width, height, (uint32_t)format, TIVX_IMAGE_FROM_ROI);

                    if ((vxGetStatus((vx_reference)subimage) == (vx_status)VX_SUCCESS) &&
                        (subimage->base.type == (vx_enum)VX_TYPE_IMAGE))
                    {
                        ownLinkParentSubimage(image, subimage);

                        si_obj_desc = (tivx_obj_desc_image_t *)subimage->base.obj_desc;

                        for(plane_idx=0; plane_idx<si_obj_desc->planes; plane_idx++)
                        {
                            subimage_imagepatch_addr = &si_obj_desc->imagepatch_addr[plane_idx];
                            image_imagepatch_addr = &obj_desc->imagepatch_addr[plane_idx];
                            subimage_mem_ptr = &si_obj_desc->mem_ptr[plane_idx];
                            image_mem_ptr = &obj_desc->mem_ptr[plane_idx];

                            *subimage_mem_ptr = *image_mem_ptr;

                            subimage_imagepatch_addr->stride_x = image_imagepatch_addr->stride_x;
                            subimage_imagepatch_addr->stride_y = image_imagepatch_addr->stride_y;

                            mem_size = ((uint32_t)subimage_imagepatch_addr->stride_y*subimage_imagepatch_addr->dim_y)/subimage_imagepatch_addr->step_y;

                            si_obj_desc->mem_size[plane_idx] = mem_size;

                            subimage->mem_offset[plane_idx] =
                                ownComputePatchOffset(rect->start_x, rect->start_y, subimage_imagepatch_addr);

                            subimage_mem_ptr->shared_ptr =
                                (uint64_t)((uint64_t)image_mem_ptr->shared_ptr + subimage->mem_offset[plane_idx]);

                            subimage_mem_ptr->host_ptr =
                                (uint64_t)((uint64_t)image_mem_ptr->host_ptr + subimage->mem_offset[plane_idx]);
                        }
                    }
                }
            }
        }
    }
    return subimage;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateVirtualImage(vx_graph graph, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    vx_image image = NULL;
    vx_reference gref = (vx_reference)graph;

    if (ownIsValidSpecificReference(gref, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        /* for now virtual image is same as normal image */
        image = (vx_image)ownCreateImageInt(graph->base.context,
            width, height, format, TIVX_IMAGE_NORMAL);
        if ((vxGetStatus((vx_reference)image) == (vx_status)VX_SUCCESS) && (image->base.type == (vx_enum)VX_TYPE_IMAGE))
        {
            ownReferenceSetScope(&image->base, &graph->base);
            image->base.is_virtual = (vx_bool)vx_true_e;
        }
    }

    return image;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateUniformImage(vx_context context, vx_uint32 width, vx_uint32 height, vx_df_image format, const vx_pixel_value_t *value)
{
    vx_image image = 0;
    vx_status status;

    if (value == NULL)
    {
        image = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
    }
    else
    {
        image = vxCreateImage(context, width, height, format);
        if (vxGetStatus((vx_reference)image) == (vx_status)VX_SUCCESS)
        {
            vx_uint32 x, y, p;
            vx_size planes = 0;
            vx_rectangle_t rect = {0, 0, width, height};
            vx_map_id map_id;
            vx_imagepatch_addressing_t addr;
            void *base;

            vxQueryImage(image, (vx_enum)VX_IMAGE_PLANES, &planes, sizeof(planes));

            for (p = 0; p < planes; p++)
            {
                status = vxMapImagePatch(image, &rect, p, &map_id, &addr, &base, (vx_enum)VX_WRITE_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_NOGAP_X);
                if(status==(vx_status)VX_SUCCESS)
                {
                    if ((format == (vx_df_image)TIVX_DF_IMAGE_P12) || (format == (vx_df_image)TIVX_DF_IMAGE_NV12_P12))
                    {
                        vx_uint16 *pixel;
                        vx_uint16 value_p12_0, value_p12_1;
                        vx_uint8 value_b0, value_b1, value_b2;

                        if (format == (vx_df_image)TIVX_DF_IMAGE_P12)
                        {
                            pixel = (vx_uint16 *)&value->U16;
                        }
                        else
                        {
                            pixel = (vx_uint16 *)&value->YUV_12;
                        }

                        if (p == 0u)
                        {
                            value_p12_0 = pixel[0] & 0x0FFFu;
                            value_p12_1 = value_p12_0;
                        }
                        else
                        {
                            value_p12_0 = pixel[1] & 0x0FFFu;
                            value_p12_1 = pixel[2] & 0x0FFFu;
                        }

                        value_b0 = (vx_uint8)(value_p12_0 & 0xFFu);
                        value_b1 = (vx_uint8)(value_p12_0>>8u) | (vx_uint8)((value_p12_1 & 0x0Fu)<<4u);
                        value_b2 = (vx_uint8)(value_p12_1>>4u);

                        for (y = 0; y < addr.dim_y; y+=addr.step_y)
                        {
                            vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, 0, y, &addr);

                            /* Write 2 pixels at a time (3 bytes) */
                            for (x = 0; x < addr.dim_x; x+=addr.step_x*2U)
                            {
                                *ptr = value_b0;
                                ptr++;
                                *ptr = value_b1;
                                ptr++;
                                *ptr = value_b2;
                                ptr++;
                            }
                        }
                    }
                    else
                    {
                        for (y = 0; y < addr.dim_y; y+=addr.step_y)
                        {
                            for (x = 0; x < addr.dim_x; x+=addr.step_x)
                            {
                                if (format == (vx_df_image)VX_DF_IMAGE_U8)
                                {
                                    vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                    *ptr = value->U8;
                                }
                                else if (format == (vx_df_image)TIVX_DF_IMAGE_RGB565)
                                {
                                    vx_uint16 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                    *ptr = value->U16;
                                }
                                else if (format == (vx_df_image)VX_DF_IMAGE_U16)
                                {
                                    vx_uint16 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                    *ptr = value->U16;
                                }
                                else if (format == (vx_df_image)VX_DF_IMAGE_U32)
                                {
                                    vx_uint32 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                    *ptr = value->U32;
                                }
                                else if (format == (vx_df_image)VX_DF_IMAGE_S16)
                                {
                                    vx_int16 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                    *ptr = value->S16;
                                }
                                else if (format == (vx_df_image)VX_DF_IMAGE_S32)
                                {
                                    vx_int32 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                    *ptr = value->S32;
                                }
                                else if ((format == (vx_df_image)VX_DF_IMAGE_RGB)  ||
                                         (format == (vx_df_image)VX_DF_IMAGE_RGBX) ||
                                         (format == (vx_df_image)TIVX_DF_IMAGE_BGRX))
                                {
                                    vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                    ptr[0] = value->RGBX[0];
                                    ptr[1] = value->RGBX[1];
                                    ptr[2] = value->RGBX[2];
                                    if ((format == (vx_df_image)VX_DF_IMAGE_RGBX) ||
                                        (format == (vx_df_image)TIVX_DF_IMAGE_BGRX))
                                    {
                                        ptr[3] = value->RGBX[3];
                                    }
                                }
                                else if ((format == (vx_df_image)VX_DF_IMAGE_YUV4) ||
                                         (format == (vx_df_image)VX_DF_IMAGE_IYUV))
                                {
                                    vx_uint8 *pixel = (vx_uint8 *)&value->YUV;
                                    vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                    *ptr = pixel[p];
                                }
                                else if ((p == 0U) &&
                                         ((format == (vx_df_image)VX_DF_IMAGE_NV12) ||
                                          (format == (vx_df_image)VX_DF_IMAGE_NV21)))
                                {
                                    vx_uint8 *pixel = (vx_uint8 *)&value->YUV;
                                    vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                    *ptr = pixel[0];
                                }
                                else if ((p == 1U) && (format == (vx_df_image)VX_DF_IMAGE_NV12))
                                {
                                    vx_uint8 *pixel = (vx_uint8 *)&value->YUV;
                                    vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                    ptr[0] = pixel[1];
                                    ptr[1] = pixel[2];
                                }
                                else if ((p == 1U) && (format == (vx_df_image)VX_DF_IMAGE_NV21))
                                {
                                    vx_uint8 *pixel = (vx_uint8 *)&value->YUV;
                                    vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                    ptr[0] = pixel[2];
                                    ptr[1] = pixel[1];
                                }
                                else if (format == (vx_df_image)VX_DF_IMAGE_UYVY)
                                {
                                    vx_uint8 *pixel = (vx_uint8 *)&value->YUV;
                                    vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                    if ((x % 2U) == 0U)
                                    {
                                        ptr[0] = pixel[1];
                                        ptr[1] = pixel[0];
                                    }
                                    else
                                    {
                                        ptr[0] = pixel[2];
                                        ptr[1] = pixel[0];
                                    }
                                }
                                else if (format == (vx_df_image)VX_DF_IMAGE_YUYV)
                                {
                                    vx_uint8 *pixel = (vx_uint8 *)&value->YUV;
                                    vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                    if ((x % 2U) == 0U)
                                    {
                                        ptr[0] = pixel[0];
                                        ptr[1] = pixel[1];
                                    }
                                    else
                                    {
                                        ptr[0] = pixel[0];
                                        ptr[1] = pixel[2];
                                    }
                                }
                                else
                                {
                                    /* Do Nothing */
                                }
                            }
                        }
                    }
                    vxUnmapImagePatch(image, map_id);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxMapImagePatch failed\n");
                    vxReleaseImage(&image);
                    image = (vx_image)ownGetErrorObject(context, (vx_status)VX_FAILURE);
                    break;
                }
            }
            if (vxGetStatus((vx_reference)image) == (vx_status)VX_SUCCESS)
            {
                /* lock the image from being modified again! */
                ((tivx_obj_desc_image_t *)image->base.obj_desc)->create_type =
                    (vx_enum)TIVX_IMAGE_UNIFORM;
            }
        }
    }
    return image;
}


VX_API_ENTRY vx_status VX_API_CALL vxReleaseImage(vx_image* image)
{
    if (image != NULL)
    {
        vx_image this_image = image[0];
        if (ownIsValidSpecificReference((vx_reference)this_image, (vx_enum)VX_TYPE_IMAGE) == (vx_bool)vx_true_e)
        {
            vx_image parent = this_image->parent;

            /* clear this image from its parent' subimages list */
            if ((NULL != parent) &&
                (ownIsValidSpecificReference((vx_reference)parent, (vx_enum)VX_TYPE_IMAGE) ==
                    (vx_bool)vx_true_e) )
            {
                vx_uint32 subimage_idx;

                for (subimage_idx = 0; subimage_idx < TIVX_IMAGE_MAX_SUBIMAGES; subimage_idx++)
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

    return ownReleaseReferenceInt((vx_reference *)image, (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_EXTERNAL, NULL);
}


VX_API_ENTRY void* VX_API_CALL vxFormatImagePatchAddress1d(void *ptr, vx_uint32 index, const vx_imagepatch_addressing_t *addr)
{
    vx_uint8 *new_ptr = NULL;
    if ((NULL != ptr) && (index < (addr->dim_x*addr->dim_y) ) )
    {
        vx_uint32 x = index % addr->dim_x;
        vx_uint32 y = index / addr->dim_x;
        vx_uint32 offset = ownComputePatchOffset(x, y, addr);
        new_ptr = (vx_uint8 *)ptr;
        new_ptr = &new_ptr[offset];
    }
    return new_ptr;
}

VX_API_ENTRY void* VX_API_CALL vxFormatImagePatchAddress2d(void *ptr, vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t *addr)
{
    vx_uint8 *new_ptr = NULL;
    if ((NULL != ptr) && (x < addr->dim_x) && (y < addr->dim_y))
    {
        vx_uint32 offset = ownComputePatchOffset(x, y, addr);
        new_ptr = (vx_uint8 *)ptr;
        new_ptr = &new_ptr[offset];
    }
    return new_ptr;
}

VX_API_ENTRY vx_status VX_API_CALL vxGetValidRegionImage(vx_image image, vx_rectangle_t *rect)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    tivx_obj_desc_image_t *obj_desc = NULL;

    if (ownIsValidImage(image) == (vx_bool)vx_true_e)
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

        if (NULL != rect)
        {
            obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;

            if ((obj_desc->valid_roi.start_x <= obj_desc->valid_roi.end_x) && (obj_desc->valid_roi.start_y <= obj_desc->valid_roi.end_y))
            {
                rect->start_x = obj_desc->valid_roi.start_x;
                rect->start_y = obj_desc->valid_roi.start_y;
                rect->end_x = obj_desc->valid_roi.end_x;
                rect->end_y = obj_desc->valid_roi.end_y;
            }
            else
            {
                /* correct the valid ROI since its invalid */
                obj_desc->valid_roi.start_x = 0;
                obj_desc->valid_roi.start_y = 0;
                obj_desc->valid_roi.end_x   = obj_desc->width;
                obj_desc->valid_roi.end_y   = obj_desc->height;

                rect->start_x = 0;
                rect->start_y = 0;
                rect->end_x = obj_desc->width;
                rect->end_y = obj_desc->height;
            }
            status = (vx_status)VX_SUCCESS;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "rectangle is NULL\n");
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetImageValidRectangle(vx_image image, const vx_rectangle_t* rect)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    tivx_obj_desc_image_t *obj_desc = NULL;

    if (ownIsValidImage(image) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;

        if (NULL != obj_desc)
        {
            if (NULL != rect)
            {
                if ((rect->start_x <= rect->end_x) &&
                    (rect->start_y <= rect->end_y) &&
                    (rect->end_x <= obj_desc->width) &&
                    (rect->end_y <= obj_desc->height))
                {
                    obj_desc->valid_roi.start_x = rect->start_x;
                    obj_desc->valid_roi.start_y = rect->start_y;
                    obj_desc->valid_roi.end_x   = rect->end_x;
                    obj_desc->valid_roi.end_y   = rect->end_y;
                    status = (vx_status)VX_SUCCESS;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "invalid rectangle dimensions\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    if (!(rect->start_x <= rect->end_x))
                    {
                        VX_PRINT(VX_ZONE_ERROR, "rectangle start x is greater than end x\n");
                    }
                    if (!(rect->start_y <= rect->end_y))
                    {
                        VX_PRINT(VX_ZONE_ERROR, "rectangle start y is greater than end y\n");
                    }
                    if (!(rect->end_x <= obj_desc->width))
                    {
                        VX_PRINT(VX_ZONE_ERROR, "rectangle end x is greater than image width\n");
                    }
                    if (!(rect->end_y <= obj_desc->height))
                    {
                        VX_PRINT(VX_ZONE_ERROR, "rectangle end y is greater than image height\n");
                    }
                }
            }
            else
            {
                obj_desc->valid_roi.start_x = 0;
                obj_desc->valid_roi.start_y = 0;
                obj_desc->valid_roi.end_x   = obj_desc->width;
                obj_desc->valid_roi.end_y   = obj_desc->height;
                status = (vx_status)VX_SUCCESS;
            }
        }
    }

    return status;
}

VX_API_ENTRY vx_size VX_API_CALL vxComputeImagePatchSize(vx_image image,
                                       const vx_rectangle_t *rect,
                                       vx_uint32 plane_index)
{
    vx_size size = 0U, num_pixels;
    vx_uint32 start_x = 0u, start_y = 0u, end_x = 0u, end_y = 0u;
    vx_imagepatch_addressing_t *imagepatch_addr;
    tivx_obj_desc_image_t *obj_desc = NULL;
    vx_df_image format;

    if ((ownIsValidImage(image) == (vx_bool)vx_true_e) && (NULL != rect))
    {
        obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;
        if ((rect->start_x <= rect->end_x) && (rect->start_y <= rect->end_y) &&
            (rect->end_x <= obj_desc->width) && (rect->end_y <= obj_desc->height))
        {
            start_x = rect->start_x;
            start_y = rect->start_y;
            end_x = rect->end_x;
            end_y = rect->end_y;

            if (plane_index < obj_desc->planes)
            {
                imagepatch_addr = &obj_desc->imagepatch_addr[plane_index];

                num_pixels  =
                            (((vx_size)end_x-(vx_size)start_x)/(vx_size)imagepatch_addr->step_x)
                            *
                            (((vx_size)end_y-(vx_size)start_y)/(vx_size)imagepatch_addr->step_y)
                            ;

                format = obj_desc->format;
                if (((vx_df_image)TIVX_DF_IMAGE_P12 == format) || ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == format))
                {
                    size = (num_pixels * 3U) / 2U;
                }
                else
                {
                    size = num_pixels * (uint32_t)imagepatch_addr->stride_x;
                }
            }
            else
            {
                vxAddLogEntry((vx_reference)image, (vx_status)VX_ERROR_INVALID_PARAMETERS, "Plane index %u is out of bounds!", plane_index);
            }
        }
        else
        {
            vxAddLogEntry((vx_reference)image, (vx_status)VX_ERROR_INVALID_PARAMETERS, "Input rect out of bounds!");
        }
    }
    return size;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryImage(vx_image image, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *obj_desc = NULL;

    if (ownIsValidImage(image) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;
        switch (attribute)
        {
            case (vx_enum)VX_IMAGE_FORMAT:
                if (VX_CHECK_PARAM(ptr, size, vx_df_image, 0x3U))
                {
                    *(vx_df_image *)ptr = obj_desc->format;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query image format failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_IMAGE_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->width;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query image width failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_IMAGE_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->height;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query image height failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_IMAGE_PLANES:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->planes;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query image planes failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_IMAGE_SPACE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = (vx_enum)obj_desc->color_space;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query image space failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_IMAGE_RANGE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = (vx_enum)obj_desc->color_range;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query image range failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_IMAGE_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    vx_size size = 0U;
                    vx_uint32 p;
                    for (p = 0; p < obj_desc->planes; p++)
                    {
                        size += obj_desc->mem_size[p];
                    }
                    *(vx_size *)ptr = size;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query image size failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_IMAGE_MEMORY_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = (vx_enum)(vx_enum)VX_MEMORY_TYPE_NONE;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query image memory type failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_IMAGE_IMAGEPATCH_ADDRESSING:
                if ((NULL != ptr) &&
                    (size >= sizeof(vx_imagepatch_addressing_t)) &&
                    (((vx_size)ptr & 0x3U) == 0U))
                {
                    vx_size num_dims = size / sizeof(vx_imagepatch_addressing_t);

                    if(num_dims > (vx_size)TIVX_IMAGE_MAX_PLANES)
                    {
                        num_dims = (vx_size)TIVX_IMAGE_MAX_PLANES;
                    }

                    tivx_obj_desc_memcpy(ptr, &obj_desc->imagepatch_addr[0], (uint32_t)sizeof(vx_imagepatch_addressing_t)*num_dims);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query image imagepatch addressing failed\n");
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

VX_API_ENTRY vx_status VX_API_CALL vxSetImageAttribute(vx_image image, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if (ownIsValidImage(image) == (vx_bool)vx_true_e)
    {
        switch (attribute)
        {
            case (vx_enum)VX_IMAGE_SPACE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    ((tivx_obj_desc_image_t *)image->base.obj_desc)->
                        color_space = (uint32_t)*(const vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "invalid image space\n");
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


VX_API_ENTRY vx_status VX_API_CALL vxCopyImagePatch(
    vx_image image,
    const vx_rectangle_t* rect,
    vx_uint32 plane_index,
    const vx_imagepatch_addressing_t* user_addr,
    void* user_ptr,
    vx_enum usage,
    vx_enum mem_type)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 start_x = rect ? rect->start_x : 0u;
    vx_uint32 start_y = rect ? rect->start_y : 0u;
    vx_uint32 end_x = rect ? rect->end_x : 0u;
    vx_uint32 end_y = rect ? rect->end_y : 0u;
    tivx_obj_desc_image_t *obj_desc = NULL;

    if (user_ptr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "User pointer is null\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    if (user_addr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "User addr is null\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    if ((usage != (vx_enum)VX_READ_ONLY) && (usage != (vx_enum)VX_WRITE_ONLY))
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid usage parameter\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        status = ownCopyAndMapCheckParams(image, rect, plane_index, usage);
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        vx_uint32 x;
        vx_uint32 y;
        vx_imagepatch_addressing_t *image_addr = NULL;
        vx_uint8* pImagePtr = NULL;
        vx_uint8* pUserPtr = user_ptr;

        vx_uint8 *pImageLine;
        vx_uint8 *map_addr;
        vx_uint8 *pUserLine;
        vx_uint8 *pImageElem;
        vx_uint8 *pUserElem;
        uint32_t len, map_size;

        obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;

        image_addr = &obj_desc->imagepatch_addr[plane_index];
        pImagePtr = (vx_uint8*)(uintptr_t)obj_desc->mem_ptr[plane_index].host_ptr;

        if (user_addr->stride_x < image_addr->stride_x)
        {
            VX_PRINT(VX_ZONE_ERROR, "User value for stride_x is smaller than minimum needed for image type\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
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

        map_addr = pImageLine;
        map_size = ((end_y - start_y)*(vx_uint32)image_addr->stride_y)/image_addr->step_y;

        if(status == (vx_status)VX_SUCCESS)
        {
            tivxCheckStatus(&status, tivxMemBufferMap(map_addr, map_size, (vx_enum)VX_MEMORY_TYPE_HOST, usage));

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

                len = (vx_uint32)image_addr->stride_x;

                if(usage == (vx_enum)VX_READ_ONLY)
                {
                    if(image_addr->stride_x == 0)
                    {
                        /* The destination is not compact, we need to copy per element */
                        for (y = start_y; y < end_y; y += image_addr->step_y)
                        {
                            pImageElem = pImageLine;
                            pUserElem = pUserLine;

                            for (x = start_x; x < end_x; x += image_addr->step_x*2U)
                            {
                                vx_uint32 *pImageElem32 = (vx_uint32*)pImageElem;
                                vx_uint16 *pUserElem16 = (vx_uint16*)pUserElem;
                                vx_uint32 value;

                                value = *pImageElem32;

                                *pUserElem16 = (vx_uint16)(value & 0xFFFU);

                                pUserElem += user_addr->stride_x;
                                pUserElem16 = (vx_uint16*)pUserElem;

                                *pUserElem16 = (vx_uint16)((value >> 12) & 0xFFFU);

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

                            for (x = start_x; x < end_x; x += image_addr->step_x*2U)
                            {
                                vx_uint16 *pUserElem16 = (vx_uint16*)pUserElem;
                                vx_uint32 value;

                                value = (vx_uint32)*pUserElem16 & 0xFFFU;

                                pUserElem += user_addr->stride_x;
                                pUserElem16 = (vx_uint16*)pUserElem;

                                value |= ((vx_uint32)*pUserElem16 & 0xFFFU)<<12;

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

            tivxCheckStatus(&status, tivxMemBufferUnmap(map_addr, map_size, (vx_enum)VX_MEMORY_TYPE_HOST, usage));
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxMapImagePatch(
    vx_image image,
    const vx_rectangle_t* rect,
    vx_uint32 plane_index,
    vx_map_id* map_id,
    vx_imagepatch_addressing_t* user_addr,
    void** user_ptr,
    vx_enum usage,
    vx_enum mem_type,
    vx_uint32 flags)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *obj_desc = NULL;

    if (user_ptr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "User pointer is null\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    if (user_addr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "User addr is null\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    if (map_id == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Map ID is null\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        status = ownCopyAndMapCheckParams(image, rect, plane_index, usage);
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        vx_imagepatch_addressing_t *image_addr = NULL;
        vx_uint8* map_addr = NULL, *end_addr = NULL, *host_addr = NULL;
        uint32_t map_size = 0;
        uint32_t map_idx;

        obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;

        image_addr = &obj_desc->imagepatch_addr[plane_index];
        map_addr = (vx_uint8*)(uintptr_t)obj_desc->mem_ptr[plane_index].host_ptr;
        map_size = obj_desc->mem_size[plane_index];

        if (NULL != map_addr)
        {
            host_addr = map_addr;

            /* Move Map Pointer as per Valid ROI */
            map_addr = vxFormatImagePatchAddress2d(map_addr, rect->start_x,
                rect->start_y, image_addr);

            for(map_idx=0; map_idx<TIVX_IMAGE_MAX_MAPS; map_idx++)
            {
                if(image->maps[map_idx].map_addr==NULL)
                {
                    image->maps[map_idx].map_addr = host_addr;
                    image->maps[map_idx].map_size = map_size;
                    image->maps[map_idx].mem_type = mem_type;
                    image->maps[map_idx].usage = usage;
                    break;
                }
            }
            if(map_idx<TIVX_IMAGE_MAX_MAPS)
            {
                *map_id = map_idx;
                tivx_obj_desc_memcpy(user_addr, image_addr, (vx_uint32)sizeof(vx_imagepatch_addressing_t));
                *user_ptr = map_addr;

                user_addr->dim_x = rect->end_x - rect->start_x;
                user_addr->dim_y = rect->end_y - rect->start_y;

                end_addr = host_addr + map_size;
                map_addr = (vx_uint8*)TIVX_FLOOR((uintptr_t)host_addr, 128U);
                end_addr = (vx_uint8*)TIVX_ALIGN((uintptr_t)end_addr, 128U);
                uintptr_t temp_map_size = (uintptr_t)end_addr - (uintptr_t)host_addr;
                map_size = (vx_uint32)temp_map_size;
                tivxCheckStatus(&status, tivxMemBufferMap(map_addr, map_size, mem_type, usage));

                ownLogSetResourceUsedValue("TIVX_IMAGE_MAX_MAPS", (uint16_t)map_idx+1U);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "No available image maps\n");
                VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_IMAGE_MAX_MAPS in tiovx/include/TI/tivx_config.h\n");
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

VX_API_ENTRY vx_status VX_API_CALL vxUnmapImagePatch(vx_image image, vx_map_id map_id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* bad references */
    if (ownIsValidImage(image) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid image reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        if ((image->base.is_virtual == (vx_bool)vx_true_e)
            &&
            (image->base.is_accessible == (vx_bool)vx_false_e)
            )
        {
            /* cannot be accessed by app */
            VX_PRINT(VX_ZONE_ERROR, "image cannot be accessed by application\n");
            status = (vx_status)VX_ERROR_OPTIMIZED_AWAY;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        if(map_id >= TIVX_IMAGE_MAX_MAPS)
        {
            VX_PRINT(VX_ZONE_ERROR, "map ID is greater than the maximum image maps\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        if( (image->maps[map_id].map_addr!=NULL)
            &&
            (image->maps[map_id].map_size!=0U)
            )
        {
            vx_uint8* map_addr = NULL, *end_addr = NULL;
            uint32_t map_size = 0;

            map_addr = image->maps[map_id].map_addr;
            map_size = (uint32_t)image->maps[map_id].map_size;

            end_addr = map_addr + map_size;
            map_addr = (vx_uint8*)TIVX_FLOOR((uintptr_t)map_addr, 128U);
            end_addr = (vx_uint8*)TIVX_ALIGN((uintptr_t)end_addr, 128U);
            uintptr_t temp_size = (uintptr_t)end_addr - (uintptr_t)map_addr;
            map_size = (uint32_t)temp_size;

            tivxCheckStatus(&status, tivxMemBufferUnmap(
                map_addr, map_size,
                image->maps[map_id].mem_type,
                image->maps[map_id].usage));

            image->maps[map_id].map_addr = NULL;
        }
        else
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            if(image->maps[map_id].map_addr==NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "map address is null\n");
            }
            if(image->maps[map_id].map_size==0U)
            {
                VX_PRINT(VX_ZONE_ERROR, "map size is equal to 0\n");
            }
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSwapImageHandle(vx_image image, void* const new_ptrs[],
    void* prev_ptrs[], vx_size num_planes)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t image_planes;
    vx_uint32 i;
    vx_uint32 p;
    vx_image subimage;
    tivx_obj_desc_image_t *obj_desc = NULL, *si_obj_desc = NULL;

    if (ownIsValidImage(image) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;

        image_planes = (uint16_t)obj_desc->planes;

        if(num_planes != image_planes)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "number of planes is not equal to the number of image planes\n");
        }

        if(status == (vx_status)VX_SUCCESS)
        {
            if (new_ptrs != NULL)
            {
                for (p = 0; p < image_planes; p++)
                {
                    if (new_ptrs[p] == NULL)
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "Plane %d is NULL\n", p);
                    }
                }
            }
        }

        if(status == (vx_status)VX_SUCCESS)
        {
            if ((prev_ptrs != NULL) && (image->parent != NULL))
            {
                /* do not return prev pointers for subimages */
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Previous pointers are not returned for subimages\n");
            }
        }

        if(status == (vx_status)VX_SUCCESS)
        {
            if ((prev_ptrs != NULL) && (image->parent == NULL))
            {
                /* return previous image handles */
                for (p = 0; p < image_planes; p++)
                {
                    prev_ptrs[p] = (void*)(uintptr_t)obj_desc->mem_ptr[p].host_ptr;

                    if (NULL != prev_ptrs[p])
                    {
                        tivxCheckStatus(&status, tivxMemBufferMap(prev_ptrs[p], obj_desc->mem_size[p],
                            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
                    }
                }
            }

            /* visit each subimage of this image and reclaim its pointers */
            for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGES; i++)
            {
                subimage = image->subimages[i];

                if (subimage != NULL)
                {
                    si_obj_desc = (tivx_obj_desc_image_t *)subimage->base.
                        obj_desc;

                    if (new_ptrs == NULL)
                    {
                        status = vxSwapImageHandle(subimage, (void**)NULL, (void**)NULL, si_obj_desc->planes);
                    }
                    else
                    {
                        vx_uint8* ptrs[4];

                        if((vx_enum)si_obj_desc->create_type==(vx_enum)TIVX_IMAGE_FROM_ROI)
                        {
                            for (p = 0; p < si_obj_desc->planes; p++)
                            {
                                ptrs[p] = (vx_uint8*)new_ptrs[p] + subimage->mem_offset[p];
                            }

                            status = vxSwapImageHandle(subimage, (void**)ptrs, (void**)NULL, si_obj_desc->planes);
                        }
                        else
                        if((vx_enum)si_obj_desc->create_type==(vx_enum)TIVX_IMAGE_FROM_CHANNEL)
                        {
                            ptrs[0] = new_ptrs[subimage->channel_plane];

                            status = vxSwapImageHandle(subimage, (void**)ptrs, (void**)NULL, si_obj_desc->planes);
                        }
                        else
                        {
                            /* Should not hit this condition */
                            VX_PRINT(VX_ZONE_ERROR, "Invalid image create type\n");
                            status = (vx_status)VX_FAILURE;
                        }
                    }
                }
            }

            /* reclaim previous and set new handles for this image */
            for (p = 0; p < image_planes; p++)
            {
                if (new_ptrs == NULL)
                {
                    obj_desc->mem_ptr[p].host_ptr = (uint64_t)(uintptr_t)NULL;
                    obj_desc->mem_ptr[p].shared_ptr = (uint64_t)(uintptr_t)NULL;
                }
                else
                {
                    /* set new pointers for subimage */
                    obj_desc->mem_ptr[p].host_ptr = (uint64_t)(uintptr_t)new_ptrs[p];
                    obj_desc->mem_ptr[p].shared_ptr = tivxMemHost2SharedPtr((uint64_t)(uintptr_t)new_ptrs[p], (int32_t)obj_desc->mem_ptr[p].mem_heap_region);

                    if (NULL != new_ptrs[p])
                    {
                        tivxCheckStatus(&status, tivxMemBufferUnmap(new_ptrs[p], obj_desc->mem_size[p],
                            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
                    }
                }
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid image reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status ownInitVirtualImage(
    vx_image img, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    vx_status status = (vx_status)VX_FAILURE;

    if ((ownIsValidSpecificReference((vx_reference)img, (vx_enum)VX_TYPE_IMAGE) == (vx_bool)vx_true_e)
        &&
        (img->base.obj_desc != NULL))
    {
        if ((width > 0U) &&
            (height > 0U) &&
            (img->base.is_virtual == (vx_bool)vx_true_e))
        {
            ownInitImage(img, width, height, format);
            status = (vx_status)VX_SUCCESS;
        }

        if (!(width > 0U))
        {
            VX_PRINT(VX_ZONE_ERROR, "Width is not greater than 0\n");
        }
        if (!(height > 0U))
        {
            VX_PRINT(VX_ZONE_ERROR, "Height is not greater than 0\n");
        }
        if (!(img->base.is_virtual == (vx_bool)vx_true_e))
        {
            VX_PRINT(VX_ZONE_ERROR, "Image is not virtual\n");
        }
    }

    return (status);
}
