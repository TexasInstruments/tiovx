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
#define TIVX_IMG_ALIGN(size)    (((size + (TIVX_IMG_ALIGN_BYTES-1U)) / TIVX_IMG_ALIGN_BYTES) * TIVX_IMG_ALIGN_BYTES)
/* Max bound of iterating through sub-image for swapping; the size is directly proportional to the task stack usage */
/* the stack can hold two time the max number of subimages */
#define TIVX_SUBIMAGE_STACK_SIZE (TIVX_IMAGE_MAX_SUBIMAGES * TIVX_IMAGE_MAX_SUBIMAGE_DEPTH)
static vx_status isImageCopyable(vx_image input, vx_image output);
static vx_status isImageSwapable(vx_image input, vx_image output);
static vx_status copyImage(vx_image input, vx_image output);
static vx_status swapImage(vx_image input, vx_image output);
static vx_status VX_CALLBACK imageKernelCallback(vx_enum kernel_enum, vx_bool validate_only, const vx_reference input, const vx_reference output);

static vx_bool ownIsSupportedFourcc(vx_df_image code);
static vx_enum ownGetTensorTypeFromImage(vx_enum image_format);
static vx_bool ownIsOdd(vx_uint32 a);
static vx_bool ownIsValidDimensions(vx_uint32 width, vx_uint32 height, vx_df_image color);
static vx_uint32 ownComputePatchOffset(vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t* addr);
static vx_size ownSizeOfChannel(vx_df_image color);
static void ownLinkParentSubimage(vx_image parent, vx_image subimage);
static vx_uint16 ownGetNumParentSubimages(const vx_image image);
static vx_status ownDestructImage(vx_reference ref);
static vx_status ownAllocImageBuffer(vx_reference ref);
static void ownInitPlane(vx_image image,
                 vx_uint32 idx,
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
static vx_status ownSwapImageCheck(tivx_obj_desc_image_t *obj_desc, vx_image image, void* const new_ptrs[], void* prev_ptrs[], vx_size num_planes);
static void ownSwapImageMap(tivx_obj_desc_image_t *obj_desc, void* prev_ptrs[], vx_size image_planes);
static void ownSwapImageUnmap(tivx_obj_desc_image_t *obj_desc, void* const new_ptrs[], vx_size image_planes);
static vx_status ownSwapSubImage(vx_image image, void* const new_ptrs[]);
static vx_status ownSwapSubImageCheckRemap(tivx_obj_desc_image_t *obj_desc, vx_image image, void* const new_ptrs[]);

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

vx_bool ownIsValidImage(vx_image image)
{
    vx_bool is_valid;

    if ((ownIsValidSpecificReference(vxCastRefFromImage(image), (vx_enum)VX_TYPE_IMAGE) == (vx_bool)vx_true_e) &&
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
    if ((ownIsOdd(width) != (vx_bool)vx_false_e)
    && ( (color == (vx_df_image)VX_DF_IMAGE_UYVY)
    || (color == (vx_df_image)VX_DF_IMAGE_YUYV)))
    {
        is_valid = (vx_bool)vx_false_e;
    }
    else if (((ownIsOdd(width) != (vx_bool)vx_false_e) || (ownIsOdd(height) != (vx_bool)vx_false_e)) &&
              ((color == (vx_df_image)VX_DF_IMAGE_IYUV)
              || (color == (vx_df_image)VX_DF_IMAGE_NV12)
              || (color == (vx_df_image)VX_DF_IMAGE_NV21)))
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
    subimage->base.is_virtual = parent->base.is_virtual;
    ((tivx_obj_desc_image_t *)subimage->base.obj_desc)->parent_id = parent->base.obj_desc->obj_desc_id;
    /* it will find free space for subimage since this was checked before */
    for (p = 0;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR004
<justification end> */
    p < TIVX_IMAGE_MAX_SUBIMAGES;
/* LDRA_JUSTIFY_END */
    p++)
    {
        if (parent->subimages[p] == NULL)
        {
            parent->subimages[p] = subimage;
            ownLogSetResourceUsedValue("TIVX_IMAGE_MAX_SUBIMAGES", (uint16_t)p+1U);
            break;
        }
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM001
<justification end> */
    if (p == TIVX_IMAGE_MAX_SUBIMAGES)
    {
        VX_PRINT(VX_ZONE_WARNING, "May need to increase the value of TIVX_IMAGE_MAX_SUBIMAGES in tiovx/include/TI/tivx_config.h\n");
    }
/* LDRA_JUSTIFY_END */
    (void)ownIncrementReference(&parent->base, (vx_enum)VX_INTERNAL);
}

static uint16_t ownGetNumParentSubimages(const vx_image image)
{
    vx_image p = image;
    uint16_t num_parents = 0U;

    while (p->parent != NULL)
    {
        num_parents++;
        p = p->parent;
    }

    return num_parents;
}


uint32_t ownImageGetBufferSize(tivx_obj_desc_image_t *obj_desc)
{
    uint16_t plane_idx;
    uint32_t size = 0;

    /* We allocate all planes in a single buffer and multiple plane pointers,
     * with appropriate alignment between them to account for HWA requirements.
     * This is required for Video Codec in PSDK 7.3 to have NV12 buffers be contiguous. */
    for(plane_idx=0; plane_idx<(obj_desc->planes-1U); plane_idx++)
    {
        size += TIVX_IMG_ALIGN(obj_desc->mem_size[plane_idx]);
    }
    size += obj_desc->mem_size[plane_idx];

    return size;
}

static vx_status ownDestructImage(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *obj_desc = NULL;
    vx_image image = NULL;
    uint32_t size = 0;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR005
<justification end> */
    if(ref->type == (vx_enum)VX_TYPE_IMAGE)
/* LDRA_JUSTIFY_END */
    {
        /* status set to NULL due to preceding type check */
        image = vxCastRefAsImage(ref,NULL);
        obj_desc = (tivx_obj_desc_image_t *)ref->obj_desc;

        if(obj_desc!=NULL)
        {
            if ( ((vx_enum)obj_desc->create_type == (vx_enum)TIVX_IMAGE_NORMAL)
                || ((vx_enum)obj_desc->create_type == (vx_enum)TIVX_IMAGE_UNIFORM)
             )
            {
                if(obj_desc->mem_ptr[0].host_ptr!=(uint64_t)0)
                {
                    size = ownImageGetBufferSize(obj_desc);

                    (void)tivxMemBufferFree(&obj_desc->mem_ptr[0], size);

                }
            }
            (void)ownObjDescFree((tivx_obj_desc_t**)&obj_desc);
        }

        if (NULL != image->parent)
        {
            status = ownReleaseReferenceInt(vxCastRefFromImageP(&image->parent), (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_INTERNAL, NULL);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM009
<justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Image parent object release failed!\n");
            }
/* LDRA_JUSTIFY_END */
        }
    }
    return status;
}

static vx_status ownAllocImageBuffer(vx_reference ref)
{
    tivx_obj_desc_image_t *obj_desc = NULL;
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t plane_idx;
    uint32_t size = 0;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM002
<justification end> */
    if(ref->type == (vx_enum)VX_TYPE_IMAGE)
/* LDRA_JUSTIFY_END */
    {
        obj_desc = (tivx_obj_desc_image_t *)ref->obj_desc;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM003
<justification end> */
        if(obj_desc != NULL)
/* LDRA_JUSTIFY_END */
        {
            if( ((vx_enum)obj_desc->create_type == (vx_enum)TIVX_IMAGE_NORMAL)
            || ((vx_enum)obj_desc->create_type == (vx_enum)TIVX_IMAGE_UNIFORM)
             )
            {
                size = ownImageGetBufferSize(obj_desc);
                /* memory is not allocated, so allocate it */
                if(obj_desc->mem_ptr[0].host_ptr==(uint64_t)0)
                {
                    /* Two conditions may fail inside function mem pointer null and size zero
                     * to handle that put a status check here
                     */
                    status = tivxMemBufferAlloc(&obj_desc->mem_ptr[0], size, (vx_enum)TIVX_MEM_EXTERNAL);
                    if((vx_status)VX_SUCCESS == status)
                    {
                        obj_desc->mem_ptr[0].shared_ptr =
                            tivxMemHost2SharedPtr(
                                obj_desc->mem_ptr[0].host_ptr,
                                (vx_enum)TIVX_MEM_EXTERNAL);
                        tivxCheckStatus(&status, tivxMemBufferMap((void *)(uintptr_t)obj_desc->mem_ptr[0].host_ptr, (uint32_t)obj_desc->mem_size[0],
                            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));

                        for(plane_idx=1; plane_idx<obj_desc->planes; plane_idx++)
                        {
                            obj_desc->mem_ptr[plane_idx].mem_heap_region =
                                obj_desc->mem_ptr[plane_idx-(uint16_t)1].mem_heap_region;
                            obj_desc->mem_ptr[plane_idx].host_ptr =
                                obj_desc->mem_ptr[plane_idx-(uint16_t)1].host_ptr +
                                obj_desc->mem_size[plane_idx-(uint16_t)1];
                            obj_desc->mem_ptr[plane_idx].shared_ptr =
                                tivxMemHost2SharedPtr(
                                    obj_desc->mem_ptr[plane_idx].host_ptr,
                                    (vx_enum)TIVX_MEM_EXTERNAL);
                            obj_desc->mem_ptr[plane_idx].dma_buf_fd =
                                obj_desc->mem_ptr[plane_idx-(uint16_t)1].dma_buf_fd;
                            obj_desc->mem_ptr[plane_idx].dma_buf_fd_offset =
                                obj_desc->mem_ptr[plane_idx-(uint16_t)1].dma_buf_fd_offset +
                                TIVX_IMG_ALIGN(obj_desc->mem_size[plane_idx-(uint16_t)1]);
                            tivxCheckStatus(&status, tivxMemBufferMap((void *)(uintptr_t)obj_desc->mem_ptr[plane_idx].host_ptr, (uint32_t)obj_desc->mem_size[plane_idx],
                                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));
                        }
                        ref->is_allocated = (vx_bool)vx_true_e;
                    }
                    else
                    {
                        /* tivxMemBufferAlloc failed! */
                        VX_PRINT(VX_ZONE_ERROR, "Memory allocation failed!\n");
                    }
                }
                else
                {
                    /* NOT an error since memory allocation not needed */
                }
            }
            else
            {
                /* NOT an error since memory allocation not needed for other create types */
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM003
<justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "object descriptor is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_VALUE;
        }
/* LDRA_JUSTIFY_END */

    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM002
<justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "reference type is not an image\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
/* LDRA_JUSTIFY_END */

    return status;
}

static vx_status isImageCopyable(vx_image input, vx_image output)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *in_objd = (tivx_obj_desc_image_t *)input->base.obj_desc;
    tivx_obj_desc_image_t *out_objd = (tivx_obj_desc_image_t *)output->base.obj_desc;
    if ((vx_enum)TIVX_IMAGE_UNIFORM == (vx_enum)out_objd->create_type)
    {
        /* If the output image cannot be uniform, as these are defined to be read-only */
        status = (vx_status)VX_ERROR_NOT_COMPATIBLE;
    }
    else if ((in_objd->width != out_objd->width) &&
        ((0U != out_objd->width) ||
         ((vx_bool)vx_false_e == output->base.is_virtual)))
    {
        /* output width must be the same as input width unless output is virtual with width zero */
        status = (vx_status)VX_ERROR_NOT_COMPATIBLE;
    }
    else if ((in_objd->height != out_objd->height) &&
             ((0U != out_objd->height) ||
              ((vx_bool)vx_false_e == output->base.is_virtual)))
    {
        /* output height must be the same as input height unless output is virtual with height zero */
        status = (vx_status)VX_ERROR_NOT_COMPATIBLE;
    }
    else if ((in_objd->format != out_objd->format) &&
             (((uint32_t)VX_DF_IMAGE_VIRT != out_objd->format) ||
              ((vx_bool)vx_false_e == output->base.is_virtual)))
    {
        /* Output format must be the same as input format unless output is virtual with virtual format */
        status = (vx_status)VX_ERROR_NOT_COMPATIBLE;
    }
    else
    {
        /* Do nothing */
    }
    
    return status;
}

static vx_status isImageSwapable(vx_image input, vx_image output)
{
    tivx_obj_desc_image_t *in_objd = (tivx_obj_desc_image_t *)input->base.obj_desc;
    vx_status status = isImageCopyable(input, output);
    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_enum)TIVX_IMAGE_UNIFORM == (vx_enum)in_objd->create_type)
        {
            /* Neither image can be uniform */
            status = (vx_status)VX_ERROR_NOT_COMPATIBLE;
        }
        else if ((NULL != input->parent) ||
                 (NULL != output->parent))
        {
            /* Neither image may be a sub-image */
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        else
        {
            tivx_obj_desc_image_t *out_objd = (tivx_obj_desc_image_t *)output->base.obj_desc;
            vx_uint32 i;
            vx_bool has_sub_objects = (vx_bool)vx_false_e;
            for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGES; ++i)
            {
                if ((NULL != input->subimages[i]) ||
                    (NULL != output->subimages[i]))
                {
                    has_sub_objects = (vx_bool)vx_true_e;
                    break;
                }
            }
            if ((vx_bool)vx_true_e == has_sub_objects)
            {
                for (i = 0; i < in_objd->planes; ++i)
                {
                    if ((in_objd->mem_size[i] != out_objd->mem_size[i]))
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Swapping images with sub-objects and differing allocated memory size is not supported (size 1 = %zu, size 2 = %zu)\n", in_objd->mem_size[i], out_objd->mem_size[i]);
                        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                        break;
                    }
                }
            }
        }
    }
    return status;
}

static vx_status copyImage(vx_image input, vx_image output)
{
    /* Copy entire image by using memcpy only if memory sizes are equal,
       otherwise use the high-level functions
       NOTE that this will need updating if uniform images are
       implemented in a more efficient manner!
       The images must be copyable!
     */
    vx_uint32 i;
    tivx_obj_desc_image_t *ip_objd = (tivx_obj_desc_image_t *)input->base.obj_desc;
    tivx_obj_desc_image_t *op_objd = (tivx_obj_desc_image_t *)output->base.obj_desc;
    vx_status status = (vx_status)VX_SUCCESS;
    vx_rectangle_t rect = {.start_x = 0, .start_y = 0, .end_x = ip_objd->width, .end_y = ip_objd->height};
    vx_imagepatch_addressing_t addr;
    vx_map_id map_id;
    void *ptr;

    /* All OK, so we propagate metadata and valid region */
    op_objd->color_range = ip_objd->color_range;
    op_objd->color_space = ip_objd->color_space;
    /* struct assigment, use the special copy function */
    tivx_obj_desc_memcpy(&op_objd->valid_roi, &ip_objd->valid_roi, (uint32_t)sizeof(op_objd->valid_roi));
    op_objd->width  = ip_objd->width;
    op_objd->height = ip_objd->height;
    op_objd->format = ip_objd->format;

    for (i = 0; i < ip_objd->planes; ++i)
    {
        /* if the size of both objects is the same, we can use memcpy for faster processing */
        if (ip_objd->mem_size[i] == op_objd->mem_size[i])
        {
            tivxCheckStatus(&status, tivxMemBufferMap((void *)(uintptr_t)ip_objd->mem_ptr[i].host_ptr, ip_objd->mem_size[i], 
                                                      (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR025
<justification end> */
            if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
            {
                tivxCheckStatus(&status, tivxMemBufferMap((void *)(uintptr_t)op_objd->mem_ptr[i].host_ptr, ip_objd->mem_size[i], 
                                                          (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR026
<justification end> */
                if ((vx_status)VX_SUCCESS == status)
                {
                    (void)memcpy((void *)(uintptr_t)op_objd->mem_ptr[i].host_ptr, (void *)(uintptr_t)ip_objd->mem_ptr[i].host_ptr, ip_objd->mem_size[i]);
                }
/* LDRA_JUSTIFY_END */
                tivxCheckStatus(&status, tivxMemBufferUnmap((void *)(uintptr_t)op_objd->mem_ptr[i].host_ptr, op_objd->mem_size[i],
                                                            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
            }
            tivxCheckStatus(&status, tivxMemBufferUnmap((void *)(uintptr_t)ip_objd->mem_ptr[i].host_ptr, ip_objd->mem_size[i],
                                                      (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM011
<justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                break;
            }
/* LDRA_JUSTIFY_END */
        }
        else
        {
            tivxCheckStatus(&status, vxMapImagePatch(input, &rect, i, &map_id, &addr, &ptr, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0));
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR027
<justification end> */
            if ((vx_status)VX_SUCCESS == status)
            {
                status = vxCopyImagePatch(output, &rect, i, &addr, ptr, (vx_enum)VX_WRITE_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);
            }
/* LDRA_JUSTIFY_END */
            tivxCheckStatus(&status, vxUnmapImagePatch(input, map_id));
        }
    }
    return status;
}

/*! \brief Adjust the memory pointer of the image by the given offset
 * This involves non-recursively visiting every sub-image
 * and adjusting the memory pointer for that, as well.
 */
static vx_status adjustMemoryPointer(const vx_reference ref, uint64_t offset[TIVX_IMAGE_MAX_PLANES])
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_reference stack[TIVX_SUBIMAGE_STACK_SIZE];
    vx_image local_img;
    vx_reference local_ref;
    vx_image *subimages = NULL;
    vx_tensor *subtensors = NULL;
    vx_uint32 stack_pointer = 0;
    vx_uint32 i;

    for (i = 0; i < TIVX_SUBIMAGE_STACK_SIZE; ++i)
    {
        stack[i] = NULL;
    }
    stack[stack_pointer] = ref;
    stack_pointer++;
    while (0U < stack_pointer)
    {
        stack_pointer--;
        local_ref = stack[stack_pointer];
        if ((vx_enum)VX_TYPE_IMAGE == local_ref->type)
        {     
            local_img  = vxCastRefAsImage(local_ref, NULL);
            subimages  = local_img->subimages;
            subtensors = local_img->subtensors;
            tivx_obj_desc_image_t *obj_desc = (tivx_obj_desc_image_t *)local_img->base.obj_desc;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR028
<justification end>*/
            if (obj_desc->planes > 0U)
/* LDRA_JUSTIFY_END */
            {
                for (i = 0; i < obj_desc->planes; ++i)
                {
                    obj_desc->mem_ptr[i].host_ptr = obj_desc->mem_ptr[i].host_ptr + offset[i];
                    obj_desc->mem_ptr[i].shared_ptr = tivxMemHost2SharedPtr(obj_desc->mem_ptr[i].host_ptr, (vx_enum)TIVX_MEM_EXTERNAL);
                }
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM012
<justification end>*/
            else
            {
                obj_desc->mem_ptr[0U].host_ptr = obj_desc->mem_ptr[0].host_ptr + offset[local_img->channel_plane];
                obj_desc->mem_ptr[0U].shared_ptr = tivxMemHost2SharedPtr(obj_desc->mem_ptr[0U].host_ptr, (vx_enum)TIVX_MEM_EXTERNAL);
            }     
        }
        else if ((vx_enum)VX_TYPE_TENSOR == local_ref->type)
        {
            tivx_obj_desc_tensor_t *obj_desc = (tivx_obj_desc_tensor_t *)local_ref->obj_desc;
            obj_desc->mem_ptr.host_ptr = obj_desc->mem_ptr.host_ptr + offset[((vx_tensor)local_ref)->channel_plane];
            obj_desc->mem_ptr.shared_ptr = tivxMemHost2SharedPtr(obj_desc->mem_ptr.host_ptr, (vx_enum)TIVX_MEM_EXTERNAL);
            /* there are no subtensors or subimage for the tensor for now 
               stop the loop below by setting the null pointer for the subtensor and subimage*/
            subtensors = NULL;
            subimages  = NULL;
        }
        else
        {
            /* do nothing this should not happen as we only use the function with vximage */
        }
/* LDRA_JUSTIFY_END */
       if ((subimages!=NULL) && (subtensors!=NULL))
       {
        for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGES; ++i)
            {
                if (NULL != subimages[i])
                {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM013
<justification end>*/
                    if (TIVX_SUBIMAGE_STACK_SIZE <= stack_pointer)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Too many sub-images, may need to increase the value of TIVX_SUBIMAGE_STACK_SIZE\n");
                        status = (vx_status)VX_ERROR_NO_RESOURCES;
                        break;
                    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM013
<justification end>*/
                    else
/* LDRA_JUSTIFY_END */
                    {
                        stack[stack_pointer] = vxCastRefFromImage(subimages[i]);
                        stack_pointer++;
                    }
                }
            }
            for (i = 0; i < TIVX_IMAGE_MAX_SUBTENSORS; ++i)
            {
                if (NULL != subtensors[i])
                {
                    if (TIVX_SUBIMAGE_STACK_SIZE <= stack_pointer)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Too many sub-tensors, may need to increase the value of TIVX_SUBOBJECT_STACK_SIZE in include/TI/tivx_config.h\n");
                        status = (vx_status)VX_ERROR_NO_RESOURCES;
                        break;
                    }
                    else
                    {
                        stack[stack_pointer] = vxCastRefFromTensor(subtensors[i]);
                        stack_pointer++;
                    }
                }
            }
        }
    }
    return status;
}

static vx_status swapImage(const vx_image input, const vx_image output)
{
    /* Swap image handles. Need to recalculate for all the sub-images
        NOTE that this will need updating if uniform images are
        implemented in a more efficient manner!
        lock only one reference as this is locking the global vx context
    */
    vx_status status = ownReferenceLock(&output->base);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR029
<justification end> */
    if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
    {
        tivx_obj_desc_image_t *ip_obj_desc = (tivx_obj_desc_image_t *)input->base.obj_desc;
        tivx_obj_desc_image_t *op_obj_desc = (tivx_obj_desc_image_t *)output->base.obj_desc;
        vx_uint32 i;
        vx_uint64 offsets[TIVX_IMAGE_MAX_PLANES];
        vx_imagepatch_addressing_t addrs;
        tivx_reference_callback_f destructor;
        uint32_t creation_type;
        uint32_t mem_size;
        for (i = 0; i < TIVX_IMAGE_MAX_PLANES; ++i)
        {
            offsets[i] = op_obj_desc->mem_ptr[i].host_ptr - ip_obj_desc->mem_ptr[i].host_ptr;
            tivx_obj_desc_memcpy(&addrs, &op_obj_desc->imagepatch_addr[i], (uint32_t)sizeof(addrs));
            tivx_obj_desc_memcpy(&op_obj_desc->imagepatch_addr[i], &ip_obj_desc->imagepatch_addr[i], (uint32_t)sizeof(op_obj_desc->imagepatch_addr[i]));
            tivx_obj_desc_memcpy(&ip_obj_desc->imagepatch_addr[i], &addrs, (uint32_t)sizeof(ip_obj_desc->imagepatch_addr[i]));
            /* swap destructors even if they are generic (identical).
               we do it for completeness and to ensure that in case 
               there is later a need of unique destructors */
            destructor = output->base.destructor_callback;
            output->base.destructor_callback = input->base.destructor_callback;
            input->base.destructor_callback = destructor;
            creation_type = op_obj_desc->create_type;
            op_obj_desc->create_type = ip_obj_desc->create_type;
            ip_obj_desc->create_type = creation_type;
            mem_size = op_obj_desc->mem_size[i];
            op_obj_desc->mem_size[i] = ip_obj_desc->mem_size[i];
            ip_obj_desc->mem_size[i] = mem_size;
        }
        status = adjustMemoryPointer(vxCastRefFromImage(input), offsets);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR030
<justification end>*/
        if ((vx_status)VX_SUCCESS == status)
        {
            /* One's complement to swap the addresses offsets between input and output images */
            for (i = 0; i < TIVX_IMAGE_MAX_PLANES; ++i)
            {
                offsets[i] = ~(offsets[i] - 1UL);
            }
            status = adjustMemoryPointer(vxCastRefFromImage(output), offsets);
        }
/* LDRA_JUSTIFY_END */
        (void)ownReferenceUnlock(&output->base);     
    }
    return (status);
}

static vx_status VX_CALLBACK imageKernelCallback(vx_enum kernel_enum, vx_bool validate_only, const vx_reference input, const vx_reference output)
{
    vx_status res  = (vx_status) VX_SUCCESS;
    vx_status res1 = (vx_status) VX_SUCCESS;

    vx_image input_img  = NULL;
    vx_image output_img = NULL;
 
    input_img  = vxCastRefAsImage(input, &res);
    output_img = vxCastRefAsImage(output, &res1);
    /* check the result only on the output, as we know that the input must be an image at that point
       otherwise the imageKernelCallback would not be called */
    if (((vx_status) VX_SUCCESS == res1))
    {
        switch (kernel_enum)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR031
<justification end>*/
        {
/* LDRA_JUSTIFY_END */
            case (vx_enum)VX_KERNEL_COPY:
                if ((vx_bool)vx_true_e == validate_only)
                {
                    res =  isImageCopyable(input_img, output_img);
                }
                else
                {
                    res = copyImage(input_img, output_img);
                }
                break;
            case (vx_enum)VX_KERNEL_SWAP:
            case (vx_enum)VX_KERNEL_MOVE:
                if ((vx_bool)vx_true_e == validate_only)
                {
                    res =  isImageSwapable(input_img, output_img);
                }
                else
                {
                    res = swapImage(input_img, output_img);
                }
                break;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM014
<justification end>*/
/* the interface for copy, move and swap is done via the direct adressing mode (vxu_...-) or when creating the corresponding specific node
   so this is not possible to reach this code because the kernel type is specified by the private functions */      
            default:
                res = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
/* LDRA_JUSTIFY_END */
        }
    }
    else
    {
        res = (vx_status)VX_ERROR_NOT_COMPATIBLE;
    }
    return (res);
}
static void ownInitPlane(vx_image image,
                 vx_uint32 idx,
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

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR006
<justification end> */
    if (image != NULL)
    {
        obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;

        imagepatch_addr = &obj_desc->imagepatch_addr[idx];

        imagepatch_addr->dim_x = width;
        imagepatch_addr->dim_y = height;
        imagepatch_addr->stride_x = (vx_int32)size_of_ch*(vx_int32)channels;
        if ( size_of_ch != 0U )
        {
            vx_uint32 temp_if = TIVX_ALIGN(
                        (imagepatch_addr->dim_x*(vx_uint32)imagepatch_addr->stride_x)/step_x,
                        image->stride_y_alignment);
            imagepatch_addr->stride_y = (vx_int32)temp_if;
        }
        else /* Only for P12 and NV12_P12 */
        {
            vx_uint32 temp_else = TIVX_ALIGN(
                        (((imagepatch_addr->dim_x*(vx_uint32)bits_per_pixel)+7U)/8U),
                        image->stride_y_alignment);
            imagepatch_addr->stride_y = (vx_int32)temp_else;
        }
        imagepatch_addr->scale_x = VX_SCALE_UNITY/step_x;
        imagepatch_addr->scale_y = VX_SCALE_UNITY/step_y;
        imagepatch_addr->step_x = step_x;
        imagepatch_addr->step_y = step_y;

        mem_size = ((vx_uint32)imagepatch_addr->stride_y*imagepatch_addr->dim_y)/step_y;

        obj_desc->mem_size[idx] = mem_size;

        obj_desc->mem_ptr[idx].mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;
        obj_desc->mem_ptr[idx].host_ptr = (uint64_t)0;
        obj_desc->mem_ptr[idx].shared_ptr = (uint64_t)0;

        image->mem_offset[idx] = 0;
    }
/* LDRA_JUSTIFY_END */
}

static void ownInitImage(vx_image image, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    vx_uint32 size_of_ch;
    vx_uint16 subimage_idx, subtensor_idx, map_idx;
    tivx_obj_desc_image_t *obj_desc = NULL;

    obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;
    size_of_ch = (vx_uint32)ownSizeOfChannel(format);

    image->parent = NULL;
    for(subimage_idx=0; subimage_idx<TIVX_IMAGE_MAX_SUBIMAGES; subimage_idx++)
    {
        image->subimages[subimage_idx] = NULL;
    }
    for(subtensor_idx=0; subtensor_idx<TIVX_IMAGE_MAX_SUBTENSORS; subtensor_idx++)
    {
        image->subtensors[subtensor_idx] = NULL;
    }    

    for(map_idx=0; map_idx<TIVX_IMAGE_MAX_MAPS; map_idx++)
    {
        image->maps[map_idx].map_addr = NULL;
        image->maps[map_idx].map_size = 0;
    }
    image->channel_plane = 0;
    image->stride_y_alignment = TIVX_DEFAULT_STRIDE_Y_ALIGN;

    obj_desc->uniform_image_pixel_value = 0;
    obj_desc->width = width;
    obj_desc->height = height;
    obj_desc->format = format;
    obj_desc->color_range = (vx_enum)VX_CHANNEL_RANGE_FULL;

    obj_desc->valid_roi.start_x = 0;
    obj_desc->valid_roi.start_y = 0;
    obj_desc->valid_roi.end_x = width;
    obj_desc->valid_roi.end_y = height;
    obj_desc->parent_id = (vx_uint16)TIVX_OBJ_DESC_INVALID;

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
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM004
<justification end>*/
    {
/* LDRA_JUSTIFY_END */
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
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM004
<justification end>*/
        default:
            /*! should not get here unless there's a bug in the
             * ownIsSupportedFourcc call.
             */
            vxAddLogEntry(vxCastRefFromImage(image), (vx_status)VX_ERROR_INVALID_PARAMETERS, "FourCC format is invalid!\n");
            break;
/* LDRA_JUSTIFY_END */
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
    vx_reference ref = NULL;

    if (ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        if (ownIsSupportedFourcc(color) == (vx_bool)vx_true_e)
        {
            if (ownIsValidDimensions(width, height, color) == (vx_bool)vx_true_e)
            {
                ref = ownCreateReference(context, (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_EXTERNAL, &context->base);
                if ( (vxGetStatus(ref) == (vx_status)VX_SUCCESS) && (ref->type == (vx_enum)VX_TYPE_IMAGE) )
                {
                    /* status set to NULL due to preceding type check */
                    image = vxCastRefAsImage(ref, NULL);
                    /* assign refernce type specific callback's */
                    image->base.destructor_callback = &ownDestructImage;
                    image->base.mem_alloc_callback = &ownAllocImageBuffer;
                    image->base.release_callback = &ownReleaseReferenceBufferGeneric;
                    image->base.kernel_callback = &imageKernelCallback;
                    obj_desc = (tivx_obj_desc_image_t*)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_IMAGE, vxCastRefFromImage(image));

                    if(obj_desc == NULL)
                    {
                        (void)vxReleaseImage(&image);

                        vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES, "Could not allocate image object descriptor\n");
                        image = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
                        VX_PRINT(VX_ZONE_ERROR, "Could not allocate image object descriptor\n");
                        VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                        VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in include/TI/soc/tivx_config_<soc>.h\n");
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
                vxAddLogEntry(vxCastRefFromContext(context), (vx_status)VX_ERROR_INVALID_DIMENSION, "Requested Image Dimensions was invalid!\n");
                image = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_DIMENSION);
            }
        }
        else
        {
            vxAddLogEntry(vxCastRefFromContext(context), (vx_status)VX_ERROR_INVALID_FORMAT, "Requested Image Format was invalid!\n");
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

    if(status == (vx_status)VX_SUCCESS)
    {
        obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;
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
        status = ownAllocImageBuffer(vxCastRefFromImage(image));
        if (status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "image allocation failed\n");
        }
    }

    if(status==(vx_status)VX_SUCCESS)
    {
        if ( ((vx_enum)obj_desc->create_type == (vx_enum)TIVX_IMAGE_UNIFORM)
        && ( ((vx_enum)usage == (vx_enum)VX_WRITE_ONLY)
        || ((vx_enum)usage == (vx_enum)VX_READ_AND_WRITE) ) )
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
    vx_image image = NULL;
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

        if ( (vxGetStatus(vxCastRefFromImage(image)) == (vx_status)VX_SUCCESS)
         && (image->base.type == (vx_enum)VX_TYPE_IMAGE) )
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
                        (void)vxReleaseImage(&image);

                        image = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
                        status = (vx_status)VX_FAILURE;
                    }

                }
                else
                {
                    if((addrs[plane_idx].stride_x <= 0) || (addrs[plane_idx].stride_y < (addrs[plane_idx].stride_x * (vx_int32)addrs[plane_idx].dim_x) ) )
                    {
                        (void)vxReleaseImage(&image);

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
                    if(mem_ptr->host_ptr!=(uint64_t)0)
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
        context = vxGetContext(vxCastRefFromImage(image));

        obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;
        /* perhaps the parent hasn't been allocated yet? */
        if(ownAllocImageBuffer(vxCastRefFromImage(image))==(vx_status)VX_SUCCESS)
        {
            /* check the number of parent if this is already a subimage
             * if the number of parents is bigger than TIVX_IMAGE_MAX_SUBIMAGE_DEPTH
             * return a VX_ERROR */
            if (ownGetNumParentSubimages(image) >= TIVX_IMAGE_MAX_SUBIMAGE_DEPTH)
            {
                VX_PRINT(VX_ZONE_ERROR, "number of parent subimages is greater than TIVX_IMAGE_MAX_SUBIMAGE_DEPTH\n");
                subimage = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
            }
            else
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
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR010
<justification end>*/
                    {
/* LDRA_JUSTIFY_END */
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
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR011
<justification end> */
                            if(channel_plane==0U)
/* LDRA_JUSTIFY_END */
                            {
                                width = imagepatch_addr->dim_x;
                                height = imagepatch_addr->dim_y;
                            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM006
<justification end> */
                            else
                            {
                                width = imagepatch_addr->dim_x;
                                height = imagepatch_addr->dim_y/2U;
                            }
/* LDRA_JUSTIFY_END */
                            break;
                        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM007
<justification end>*/
                        default:
                            break;
/* LDRA_JUSTIFY_END */
                    }

                    subimage = (vx_image)ownCreateImageInt(context, width, height, (uint32_t)subimage_format, TIVX_IMAGE_FROM_CHANNEL);

                    if ((vxGetStatus(vxCastRefFromImage(subimage)) == (vx_status)VX_SUCCESS) /* TIOVX-1943- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR012 */
                    && (subimage->base.type == (vx_enum)VX_TYPE_IMAGE)) /* TIOVX-1943- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR013 */
                    {
                        ownLinkParentSubimage(image, subimage);

                        si_obj_desc = (tivx_obj_desc_image_t *)subimage->base.obj_desc;

                        si_obj_desc->imagepatch_addr[0].stride_x = imagepatch_addr->stride_x;
                        si_obj_desc->imagepatch_addr[0].stride_y = imagepatch_addr->stride_y;
                        /* TIOVX-742 */
                        if((format==(vx_enum)VX_DF_IMAGE_NV12) ||
                           (format==(vx_enum)VX_DF_IMAGE_NV21)
                        )
                        {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM008
<justification end> */
                            /* if UV plane in YUV420SP format, then stride_x should stride_x/2 */
                            if(channel_plane==1U)
                            {
                                si_obj_desc->imagepatch_addr[0].stride_x = imagepatch_addr->stride_x/2;
                            }
/* LDRA_JUSTIFY_END */
                        }
                        subimage->channel_plane = channel_plane;

                        si_obj_desc->mem_ptr[0].shared_ptr =
                                    (uint64_t)((uint64_t)mem_ptr->shared_ptr);
                        si_obj_desc->mem_ptr[0].host_ptr =
                                    (uint64_t)((uint64_t)mem_ptr->host_ptr);
                        si_obj_desc->mem_ptr[0].mem_heap_region = mem_ptr->mem_heap_region;
                        si_obj_desc->mem_ptr[0].dma_buf_fd = mem_ptr->dma_buf_fd;
                        si_obj_desc->mem_ptr[0].dma_buf_fd_offset = mem_ptr->dma_buf_fd_offset;

                        si_obj_desc->mem_size[0] = obj_desc->mem_size[channel_plane];
                    }
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
        context = vxGetContext(vxCastRefFromImage(image));

        obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;

        /* check the number of parent if this is already a subimage
         * if the number of parents is bigger than TIVX_IMAGE_MAX_SUBIMAGE_DEPTH
         * return a VX_ERROR */
        if (ownGetNumParentSubimages(image) >= TIVX_IMAGE_MAX_SUBIMAGE_DEPTH)
        {
            VX_PRINT(VX_ZONE_ERROR, "number of parent subimages is greater than TIVX_IMAGE_MAX_SUBIMAGE_DEPTH\n");
            subimage = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
        }
        else if ((NULL == rect) ||
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
            if(ownAllocImageBuffer(vxCastRefFromImage(image))==(vx_status)VX_SUCCESS)
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

                    if ((vxGetStatus(vxCastRefFromImage(subimage)) == (vx_status)VX_SUCCESS) && /* TIOVX-1943- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR014 */
                        (subimage->base.type == (vx_enum)VX_TYPE_IMAGE)) /* TIOVX-1943- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR015 */
                    {
                        ownLinkParentSubimage(image, subimage);

                        si_obj_desc = (tivx_obj_desc_image_t *)subimage->base.obj_desc;

                        for(plane_idx=0; plane_idx<si_obj_desc->planes; plane_idx++)
                        {
                            subimage_imagepatch_addr = &si_obj_desc->imagepatch_addr[plane_idx];
                            image_imagepatch_addr = &obj_desc->imagepatch_addr[plane_idx];
                            subimage_mem_ptr = &si_obj_desc->mem_ptr[plane_idx];
                            image_mem_ptr = &obj_desc->mem_ptr[plane_idx];

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
                            subimage_mem_ptr->mem_heap_region = image_mem_ptr->mem_heap_region;
                            subimage_mem_ptr->dma_buf_fd = image_mem_ptr->dma_buf_fd;
                            subimage_mem_ptr->dma_buf_fd_offset = image_mem_ptr->dma_buf_fd_offset;
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
    vx_reference gref = vxCastRefFromGraph(graph);

    if (ownIsValidSpecificReference(gref, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        /* for now virtual image is same as normal image */
        image = (vx_image)ownCreateImageInt(graph->base.context,
            width, height, format, TIVX_IMAGE_NORMAL);
        if ((vxGetStatus(vxCastRefFromImage(image)) == (vx_status)VX_SUCCESS) && (image->base.type == (vx_enum)VX_TYPE_IMAGE))
        {
            ownReferenceSetScope(&image->base, &graph->base);
            image->base.is_virtual = (vx_bool)vx_true_e;
        }
    }

    return image;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateUniformImage(vx_context context, vx_uint32 width, vx_uint32 height, vx_df_image format, const vx_pixel_value_t *value)
{
    vx_image image = NULL;
    vx_status status;

    if (value == NULL)
    {
        image = (vx_image)ownGetErrorObject(context, (vx_status)VX_ERROR_INVALID_PARAMETERS);
    }
    else
    {
        image = vxCreateImage(context, width, height, format);
        if (vxGetStatus(vxCastRefFromImage(image)) == (vx_status)VX_SUCCESS)
        {
            vx_uint32 x, y, p;
            vx_size planes = 0;
            vx_rectangle_t rect = {0, 0, width, height};
            vx_map_id map_id;
            vx_imagepatch_addressing_t addr;
            void *base;

            (void)vxQueryImage(image, (vx_enum)VX_IMAGE_PLANES, &planes, sizeof(planes));

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
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR016
<justification end>*/
                                else if (format == (vx_df_image)VX_DF_IMAGE_YUYV)
/* LDRA_JUSTIFY_END */
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
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM005
<justification end>*/
                                else
                                {
                                    /* Do Nothing */
                                }
/* LDRA_JUSTIFY_END */
                            }
                        }
                    }
                    (void)vxUnmapImagePatch(image, map_id);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxMapImagePatch failed\n");
                    (void)vxReleaseImage(&image);
                    image = (vx_image)ownGetErrorObject(context, (vx_status)VX_FAILURE);
                    break;
                }
            }
            if (vxGetStatus(vxCastRefFromImage(image)) == (vx_status)VX_SUCCESS)
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
        if (ownIsValidSpecificReference(vxCastRefFromImage(this_image), (vx_enum)VX_TYPE_IMAGE) == (vx_bool)vx_true_e)
        {
            vx_image parent = this_image->parent;

            /* clear this image from its parent' subimages list */
            if ((NULL != parent) &&
                (ownIsValidSpecificReference(vxCastRefFromImage(parent), (vx_enum)VX_TYPE_IMAGE) ==
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

    return ownReleaseReferenceInt(vxCastRefFromImageP(image), (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_EXTERNAL, NULL);
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
        /* removed the null check of obj_desc due to previous check in ownIsValidImage */
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
                VX_PRINT(VX_ZONE_ERROR, "Plane index %u is out of bounds!\n", plane_index);
                vxAddLogEntry(vxCastRefFromImage(image), (vx_status)VX_ERROR_INVALID_PARAMETERS, "Plane index %u is out of bounds!", plane_index);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Input rect out of bounds!\n");
            vxAddLogEntry(vxCastRefFromImage(image), (vx_status)VX_ERROR_INVALID_PARAMETERS, "Input rect out of bounds!");
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
                    vx_size img_size = 0U;
                    vx_uint32 p;
                    for (p = 0; p < obj_desc->planes; p++)
                    {
                        img_size += obj_desc->mem_size[p];
                    }
                    *(vx_size *)ptr = img_size;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query image size failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_IMAGE_STRIDE_Y_ALIGNMENT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = image->stride_y_alignment;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query image stride y alignment failed\n");
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

                    tivx_obj_desc_memcpy(ptr, &obj_desc->imagepatch_addr[0], (uint32_t)(sizeof(vx_imagepatch_addressing_t)*num_dims));
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
                        color_space = (vx_uint32)*(const vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "invalid image space\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_IMAGE_STRIDE_Y_ALIGNMENT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    vx_reference ref;

                    ref = vxCastRefFromImage(image);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR032
<justification end> */
                    if ( ((vx_bool)vx_false_e == ref->is_allocated) &&
                         (0U == ownGetNumParentSubimages(image)) &&
                         (NULL == image->parent) &&
                         ((vx_bool)vx_false_e == image->base.is_array_element) )
                    {
                        vx_uint32 tmp_stride_y_alignment = (vx_uint32)*(const vx_uint32 *)ptr;

                        if ( (tmp_stride_y_alignment % TIVX_DEFAULT_STRIDE_Y_MULTIPLE) == 0U)
                        {
                            vx_uint32 idx;
                            tivx_obj_desc_image_t *obj_desc = NULL;
                            vx_imagepatch_addressing_t *imagepatch_addr;

                            /* Updating the stride alignment property of the image */
                            image->stride_y_alignment = tmp_stride_y_alignment;

                            obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;

                            for (idx = 0U; idx < obj_desc->planes; idx++)
                            {
                                uint32_t mem_size;
                                imagepatch_addr = &obj_desc->imagepatch_addr[idx];

                                if ( imagepatch_addr->stride_x != 0 )
                                {
                                    vx_uint32 temp_if = TIVX_ALIGN(
                                                (imagepatch_addr->dim_x*(vx_uint32)imagepatch_addr->stride_x)/imagepatch_addr->step_x,
                                                image->stride_y_alignment);
                                    imagepatch_addr->stride_y = (vx_int32)temp_if;
                                }
                                else /* Only for P12 and NV12_P12 */
                                {
                                    vx_uint32 temp_else = TIVX_ALIGN(
                                                (((imagepatch_addr->dim_x*(vx_uint32)12)+7U)/8U),
                                                image->stride_y_alignment);
                                    imagepatch_addr->stride_y = (vx_int32)temp_else;
                                }

                                mem_size = ((vx_uint32)imagepatch_addr->stride_y*imagepatch_addr->dim_y)/imagepatch_addr->step_y;

                                obj_desc->mem_size[idx] = mem_size;
                            }
                        }
                        else
                        {
                            VX_PRINT(VX_ZONE_ERROR, "image stride y alignment must be a multiple of %d\n", TIVX_DEFAULT_STRIDE_Y_MULTIPLE);
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        }
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "image cannot be allocated and cannot be a subimage, contain subimages, or exist as an element of pyramid or object array\n");
                        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                    }
/* LDRA_JUSTIFY_END */
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "invalid stride y alignment\n");
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
            pImageLine = &(pImagePtr[(((unsigned long)start_y*(vx_uint32)image_addr->stride_y)/image_addr->step_y) + (((start_x*12UL)/8UL)/image_addr->step_x)]);

            if (user_addr->stride_x == 1)
            {
                VX_PRINT(VX_ZONE_ERROR, "User value for stride_x should be 0 for packed format, or >1 for unpacked format\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }
        else
        {
            pImageLine = &(pImagePtr[((start_y*(vx_uint32)image_addr->stride_y)/image_addr->step_y) + ((start_x*(vx_uint32)image_addr->stride_x)/image_addr->step_x)]);
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
                        (void)memcpy(pUserLine, pImageLine, len);
                        pImageLine = &(pImageLine[image_addr->stride_y]);
                        pUserLine = &(pUserLine[user_addr->stride_y]);
                    }
                }
                else
                {
                    /* Both have compact lines */
                    for (y = start_y; y < end_y; y += image_addr->step_y)
                    {
                        (void)memcpy(pImageLine, pUserLine, len);
                        pImageLine = &(pImageLine[image_addr->stride_y]);
                        pUserLine = &(pUserLine[user_addr->stride_y]);
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

                                pUserElem = &(pUserElem[user_addr->stride_x]);
                                pUserElem16 = (vx_uint16*)pUserElem;

                                *pUserElem16 = (vx_uint16)((value >> 12) & 0xFFFU);

                                pUserElem = &(pUserElem[user_addr->stride_x]);
                                pImageElem =&(pImageElem[3]);
                            }
                            pImageLine = &(pImageLine[image_addr->stride_y]);
                            pUserLine  = &(pUserLine[user_addr->stride_y]);
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
                                (void)memcpy(pUserElem, pImageElem, len);

                                pImageElem = &(pImageElem[len]);
                                pUserElem = &(pUserElem[user_addr->stride_x]);
                            }
                            pImageLine = &(pImageLine[image_addr->stride_y]);
                            pUserLine = &(pUserLine[user_addr->stride_y]);
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

                                pUserElem = &(pUserElem[user_addr->stride_x]);
                                pUserElem16 = (vx_uint16*)pUserElem;

                                value |= ((vx_uint32)*pUserElem16 & 0xFFFU)<<12;

                                pUserElem = &(pUserElem[user_addr->stride_x]);

                                *pImageElem = (vx_uint8)(value & 0xFFU);
                                pImageElem++;
                                *pImageElem = (vx_uint8)(value>>8u) | (vx_uint8)((value & 0x0FU)<<4u);
                                pImageElem++;
                                *pImageElem = (vx_uint8)(value>>4u);
                                pImageElem++;
                            }
                            pImageLine = &(pImageLine[image_addr->stride_y]);
                            pUserLine = &(pUserLine[user_addr->stride_y]);
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
                                (void)memcpy(pImageElem, pUserElem, len);

                                pImageElem = &(pImageElem[len]);
                                pUserElem = &(pUserElem[user_addr->stride_x]);
                            }
                            pImageLine = &(pImageLine[image_addr->stride_y]);
                            pUserLine = &(pUserLine[user_addr->stride_y]);
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

                end_addr = &(host_addr[map_size]);
                map_addr = (vx_uint8*)TIVX_FLOOR((uintptr_t)host_addr, TIVX_DATA_BUFFER_ALIGNMENT);
                end_addr = (vx_uint8*)TIVX_ALIGN((uintptr_t)end_addr, TIVX_DATA_BUFFER_ALIGNMENT);
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
        if ((image->base.is_virtual == (vx_bool)vx_true_e) &&
            (image->base.is_accessible == (vx_bool)vx_false_e))
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

            end_addr = &(map_addr[map_size]);
            map_addr = (vx_uint8*)TIVX_FLOOR((uintptr_t)map_addr, TIVX_DATA_BUFFER_ALIGNMENT);
            end_addr = (vx_uint8*)TIVX_ALIGN((uintptr_t)end_addr, TIVX_DATA_BUFFER_ALIGNMENT);
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
    tivx_obj_desc_image_t *obj_desc = NULL;

    if (ownIsValidImage(image) == (vx_bool)vx_true_e)
    {
        obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;

        status = ownSwapImageCheck(obj_desc, image, new_ptrs, prev_ptrs, num_planes);

        if(status == (vx_status)VX_SUCCESS)
        {
            if ((prev_ptrs != NULL) && (image->parent == NULL))
            {
                ownSwapImageMap(obj_desc, prev_ptrs, num_planes);
            }

            status = ownSwapSubImage(image, new_ptrs);

            ownSwapImageUnmap(obj_desc, new_ptrs, num_planes);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid image reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownSwapImageCheck(tivx_obj_desc_image_t *obj_desc, vx_image image, void* const new_ptrs[],void* prev_ptrs[], vx_size num_planes)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t image_planes;
    vx_uint32 p;

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

    return status;
}

static vx_status ownSwapSubImageCheckRemap(tivx_obj_desc_image_t *obj_desc, vx_image image, void* const new_ptrs[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = ownSwapImageCheck(obj_desc, image, new_ptrs, (void **)NULL, obj_desc->planes);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR019
<justification end> */
    if(status == (vx_status)VX_SUCCESS)
    {
        ownSwapImageUnmap(obj_desc, new_ptrs, obj_desc->planes);
    }

/* LDRA_JUSTIFY_END */
    return status;
}

static void ownSwapImageMap(tivx_obj_desc_image_t *obj_desc, void* prev_ptrs[], vx_size image_planes)
{
    vx_uint32 p;

    /* return previous image handles */
    for (p = 0; p < image_planes; p++)
    {
        prev_ptrs[p] = (void*)(uintptr_t)obj_desc->mem_ptr[p].host_ptr;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR020
<justification end> */
        if (NULL != prev_ptrs[p])
        {
            (void)tivxMemBufferMap(prev_ptrs[p], obj_desc->mem_size[p],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);
        }
/* LDRA_JUSTIFY_END */
    }
}

static vx_status ownSwapSubImage(vx_image image, void* const new_ptrs[])
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 i, p;
    vx_image subimage;
    tivx_obj_desc_image_t *si_obj_desc = NULL;
    vx_image next_image;
    vx_image image_arr[TIVX_SUBIMAGE_STACK_SIZE] = {NULL};
    void* new_ptrs_arr[TIVX_SUBIMAGE_STACK_SIZE][TIVX_IMAGE_MAX_PLANES] = {NULL};
    void* next_new_ptrs[TIVX_IMAGE_MAX_PLANES] = {NULL};
    vx_uint32 stack_pointer = 0;
    vx_bool is_first_time = (vx_bool)vx_true_e;

    image_arr[stack_pointer] = image;

    if (NULL != new_ptrs)
    {
        for (p = 0; p < TIVX_IMAGE_MAX_PLANES; p++)
        {
            next_new_ptrs[p] = new_ptrs[p];
        }
    }

    stack_pointer++;

    while (0U < stack_pointer)
    {
        stack_pointer--;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UTJT001
<justification end>*/
        /* this should not happen as the max depth for subimages is fixed */
        if (TIVX_SUBIMAGE_STACK_SIZE <= stack_pointer)
        {
            VX_PRINT(VX_ZONE_ERROR, "Too many sub-images, may need to increase the value of TIVX_SUBIMAGE_STACK_SIZE\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
            break;
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UTJT001
<justification end>*/
        else
/* LDRA_JUSTIFY_END */
        {
            next_image = image_arr[stack_pointer];
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR021
<justification end>*/
            if (ownIsValidImage(next_image) == (vx_bool)vx_true_e)
/* LDRA_JUSTIFY_END */
            {
                if ((vx_bool)vx_false_e == is_first_time)
                {
                    if (NULL != new_ptrs)
                    {
                        for (p = 0; p < TIVX_IMAGE_MAX_PLANES; p++)
                        {
                            next_new_ptrs[p] = new_ptrs_arr[stack_pointer][p];
                        }
                    }
                }
                else
                {
                    is_first_time = (vx_bool)vx_false_e;
                }

                /* visit each subimage of this image and reclaim its pointers */
                for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGES; i++)
                {
                    subimage = next_image->subimages[i];

                    if (subimage != NULL)
                    {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UTJT002
<justification end>*/
                        /* this should not happen as the max depth for subimages is fixed */
                        if (TIVX_SUBIMAGE_STACK_SIZE <= stack_pointer)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Too many sub-images, may need to increase the value of TIVX_SUBIMAGE_STACK_SIZE\n");
                            status = (vx_status)VX_ERROR_NO_RESOURCES;
                            break;
                        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UTJT002
<justification end>*/
                        else
/* LDRA_JUSTIFY_END */
                        {
                            vx_uint8* ptrs[TIVX_IMAGE_MAX_PLANES] = {NULL};

                            si_obj_desc = (tivx_obj_desc_image_t *)subimage->base.
                                obj_desc;

                            if (new_ptrs == NULL)
                            {
                                status = ownSwapSubImageCheckRemap(si_obj_desc, subimage, (void**)NULL);
                            }
                            else
                            {
                                if((vx_enum)si_obj_desc->create_type==(vx_enum)TIVX_IMAGE_FROM_ROI)
                                {
                                    for (p = 0; p < si_obj_desc->planes; p++)
                                    {
                                        ptrs[p] = &(((vx_uint8*)next_new_ptrs[p])[subimage->mem_offset[p]]);
                                    }

                                    status = ownSwapSubImageCheckRemap(si_obj_desc, subimage, (void**)ptrs);
                                }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR022
<justification end>*/
                                else
                                if((vx_enum)si_obj_desc->create_type==(vx_enum)TIVX_IMAGE_FROM_CHANNEL)
/* LDRA_JUSTIFY_END */
                                {
                                    ptrs[0] = next_new_ptrs[subimage->channel_plane];

                                    status = ownSwapSubImageCheckRemap(si_obj_desc, subimage, (void**)ptrs);
                                }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UM010
<justification end>*/
                                else
                                {
                                    /* Should not hit this condition */
                                    VX_PRINT(VX_ZONE_ERROR, "Invalid image create type\n");
                                    status = (vx_status)VX_FAILURE;
                                }
/* LDRA_JUSTIFY_END */
                            }

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR023
<justification end>*/
                            /* This is a valid image and thus adding to the list and incrementing k */
                            if ((vx_status)VX_SUCCESS == status)
                            {
                                image_arr[stack_pointer] = subimage;
                                for (p = 0; p < si_obj_desc->planes; p++)
                                {
                                    new_ptrs_arr[stack_pointer][p]  = ptrs[p];
                                }
                                stack_pointer++;
                            }
/* LDRA_JUSTIFY_END */
                        }
                    }
                }
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IMAGE_UTJT003
<justification end>*/
            else
            {
                break;
            }
/* LDRA_JUSTIFY_END */
        }
    }

    return status;
}

static void ownSwapImageUnmap(tivx_obj_desc_image_t *obj_desc, void* const new_ptrs[], vx_size image_planes)
{
    vx_uint32 p;

    for (p = 0; p < image_planes; p++)
    {
        if (new_ptrs == NULL)
        {
            obj_desc->mem_ptr[p].host_ptr = (uint64_t)(uintptr_t)0;
            obj_desc->mem_ptr[p].shared_ptr = (uint64_t)(uintptr_t)0;
        }
        else
        {
            /* set new pointers for subimage */
            obj_desc->mem_ptr[p].host_ptr = (uint64_t)(uintptr_t)new_ptrs[p];
            obj_desc->mem_ptr[p].shared_ptr = tivxMemHost2SharedPtr((uint64_t)(uintptr_t)new_ptrs[p], (int32_t)obj_desc->mem_ptr[p].mem_heap_region);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IMAGE_UBR024
<justification end> */
            if (NULL != new_ptrs[p])
            {
                (void)tivxMemBufferUnmap(new_ptrs[p], obj_desc->mem_size[p],
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);
            }
/* LDRA_JUSTIFY_END */
        }
    }
}

vx_status ownInitVirtualImage(
    vx_image img, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    vx_status status = (vx_status)VX_FAILURE;

    if ((ownIsValidSpecificReference(vxCastRefFromImage(img), (vx_enum)VX_TYPE_IMAGE) == (vx_bool)vx_true_e)
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

#define DEFAULT_MULTIPLIER 1

static vx_enum ownGetTensorTypeFromImage(vx_enum image_format)
{
    vx_enum tensor_type = (vx_enum)VX_TYPE_INVALID;
    switch (image_format)
    {
    case (vx_enum)VX_DF_IMAGE_U8:
        tensor_type = (vx_enum)VX_TYPE_UINT8;
        break;
    case (vx_enum)VX_DF_IMAGE_U16:
        tensor_type = (vx_enum)VX_TYPE_UINT16;
        break;
    case (vx_enum)VX_DF_IMAGE_S16:
        tensor_type = (vx_enum)VX_TYPE_INT16;
        break;
    case (vx_enum)VX_DF_IMAGE_U32:
        tensor_type = (vx_enum)VX_TYPE_UINT32;
        break;
    case (vx_enum)VX_DF_IMAGE_S32:
        tensor_type = (vx_enum)VX_TYPE_INT32;
        break;
    case (vx_enum)VX_DF_IMAGE_RGB:
        tensor_type = (vx_enum)VX_TYPE_UINT32;
        break;
    case (vx_enum)VX_DF_IMAGE_RGBX:
        tensor_type = (vx_enum)VX_TYPE_UINT32;
        break;
    case (vx_enum)VX_DF_IMAGE_UYVY:
        tensor_type = (vx_enum)VX_TYPE_UINT16;
        break;
    case (vx_enum)VX_DF_IMAGE_YUYV:
        tensor_type = (vx_enum)VX_TYPE_UINT16;
        break;
    case (vx_enum)TIVX_DF_IMAGE_RGB565:
        tensor_type = (vx_enum)VX_TYPE_UINT16;
        break;
    case (vx_enum)TIVX_DF_IMAGE_BGRX:
        tensor_type = (vx_enum)VX_TYPE_UINT32;
        break;
    case (vx_enum)TIVX_DF_IMAGE_P12:
        tensor_type = (vx_enum)VX_TYPE_UINT16;
        break;
    case (vx_enum)VX_DF_IMAGE_VIRT:
    case (vx_enum)VX_DF_IMAGE_NV12:
    case (vx_enum)VX_DF_IMAGE_NV21:
    case (vx_enum)VX_DF_IMAGE_IYUV:
    case (vx_enum)VX_DF_IMAGE_YUV4:
    case (vx_enum)TIVX_DF_IMAGE_NV12_P12:
        /* Investigate whether these 5 are valid and if so, what tensor data format they need */
    default:
        tensor_type = (vx_enum)VX_TYPE_INVALID;
        break;
    }

    return tensor_type;
}

VX_API_ENTRY vx_tensor VX_API_CALL vxCreateTensorFromROI(vx_image image, const vx_rectangle_t *rect, vx_int8 fixed_point_position)
{
    tivx_obj_desc_image_t *p_obj_desc;
    tivx_obj_desc_tensor_t *c_obj_desc;
    vx_status status = (vx_status)VX_SUCCESS;
    vx_tensor tensor = NULL;
    vx_uint32 i;

    /* parent image must be a valid image */
    if (((vx_bool)vx_false_e == ownIsValidSpecificReference(&image->base, (vx_enum)VX_TYPE_IMAGE)) || (NULL == image->base.obj_desc))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        /* image's width and height must be defined */
        p_obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;
        if ((0U == p_obj_desc->width) || (0U == p_obj_desc->height))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid image dimension\n");
            status = (vx_status)VX_ERROR_INVALID_DIMENSION;
        }
        else
        {
            /* check image format and prepare format-appropriate modifiers */
            uint32_t x_multiplier = DEFAULT_MULTIPLIER;
            uint32_t y_multiplier = DEFAULT_MULTIPLIER;
            vx_enum tensor_type = ownGetTensorTypeFromImage((vx_enum)p_obj_desc->format);
            if ((vx_enum)VX_TYPE_INVALID != tensor_type)
            {
                vx_rectangle_t roi;

                /* a NULL rect pointer indicates that the ROI is the whole image */
                if (NULL == rect)
                {
                    roi.start_x = 0;
                    roi.start_y = 0;
                    roi.end_x = p_obj_desc->width;
                    roi.end_y = p_obj_desc->height;
                }
                else
                {
                    roi.start_x = rect->start_x;
                    roi.start_y = rect->start_y;
                    roi.end_x = rect->end_x;
                    roi.end_y = rect->end_y;
                }
                /* rectangle sanity check */
                if ((roi.start_x > roi.end_x) || (roi.start_y > roi.end_y))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Invalid rectangle\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                /* check ROI fits in the image */
                else if ((roi.end_x > p_obj_desc->width) || (roi.end_y > p_obj_desc->height))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Invalid rectangle\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                else
                {
                    /* In case the parent image hasn't been allocated yet */
                    status = ownAllocImageBuffer(vxCastRefFromImage(image));
                    if ((vx_status)VX_SUCCESS == status)
                    {
                        /* create tensor with the context of the parent image and correct dimensions and data type */
                        vx_size dimensions[2];
                        dimensions[0] = ((vx_size)roi.end_x - (vx_size)roi.start_x + x_multiplier - 1U) / x_multiplier;
                        dimensions[1] = ((vx_size)roi.end_y - (vx_size)roi.start_y + y_multiplier - 1U) / y_multiplier;
                        tensor = vxCreateTensor(image->base.context, 2U, dimensions, tensor_type, fixed_point_position);

                        status = vxGetStatus(vxCastRefFromTensor(tensor));
                        if (status == (vx_status)VX_SUCCESS)
                        {
                            /* add the child as a subtensor of the parent */
                            for (i = 0; i < TIVX_IMAGE_MAX_SUBTENSORS; i++)
                            {
                                if (NULL == image->subtensors[i])
                                {
                                    image->subtensors[i] = tensor;
                                    ownLogSetResourceUsedValue("TIVX_IMAGE_MAX_SUBTENSORS", (uint16_t)i + 1U);
                                    break;
                                }
                            }
                            if (TIVX_IMAGE_MAX_SUBTENSORS == i)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "No available subtensor slots\n");
                                VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_IMAGE_MAX_SUBTENSORS in include/TI/tivx_config.h\n");
                                status = (vx_status)VX_ERROR_NO_RESOURCES;
                            }
                            else
                            {
                                /* register the image as the tensor's parent */
                                tensor->base.scope = &image->base;
                                tensor->parent = image;
                                ((tivx_obj_desc_tensor_t *)tensor->base.obj_desc)->parent_id = image->base.obj_desc->obj_desc_id;
                                tensor->base.is_virtual = image->base.is_virtual;
                                tensor->channel_plane = image->channel_plane;
                                /* child is useless without the parent's data, so add an internal reference */
                                (void)ownIncrementReference(&tensor->parent->base, (vx_enum)VX_INTERNAL);

                                /* stride[1] must match that of the image, not be calculated from the dimensions of the ROI */
                                c_obj_desc = (tivx_obj_desc_tensor_t *)tensor->base.obj_desc;
                                c_obj_desc->create_type = (vx_uint32)TIVX_TENSOR_FROM_ROI;
                                c_obj_desc->stride[1] = p_obj_desc->imagepatch_addr[0U].stride_y;

                                /* calculate tensor host_ptr */
                                c_obj_desc->mem_ptr.host_ptr = p_obj_desc->mem_ptr[0].host_ptr + 
                                                            (uint64_t)(((uint64_t)roi.start_x * (uint64_t)c_obj_desc->stride[0]) + ((uint64_t)roi.start_y * (uint64_t)c_obj_desc->stride[1]));
                                /* calculate shared_ptr */
                                c_obj_desc->mem_ptr.shared_ptr = tivxMemHost2SharedPtr(c_obj_desc->mem_ptr.host_ptr, (vx_enum)TIVX_MEM_EXTERNAL);

                                /* use parent supplementary data ID */
                                if ((vx_uint16)TIVX_OBJ_DESC_INVALID != image->base.obj_desc->supp_data_id)
                                {
                                    tensor->base.obj_desc->supp_data_id = image->base.obj_desc->supp_data_id; 
                                    tensor->base.supplementary_data = image->base.supplementary_data;
                                    VX_PRINT(VX_ZONE_INFO, "Found parent supplementary data ID %u, setting to child%s", tensor->base.obj_desc->supp_data_id, "\n");
                                }
                                else
                                {
                                    VX_PRINT(VX_ZONE_INFO, "Parent has no supplementary data ID%s", "\n");
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid image format\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (((vx_status)VX_SUCCESS != status))
    {
        if (tensor != NULL)
        {
            /* release the tensor if it was created */
            if (tensor->base.obj_desc != NULL)
            {
                status = vxReleaseTensor(&tensor);
                if (status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to release tensor\n");
                }
            }
        }
    }
    return tensor;
}
