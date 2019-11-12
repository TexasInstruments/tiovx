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

static vx_status ownInitMetaFormatWithImage(
    vx_meta_format meta, vx_image exemplar);
static vx_status ownInitMetaFormatWithArray(
    vx_meta_format meta, vx_array exemplar);
static vx_status ownInitMetaFormatWithScalar(
    vx_meta_format meta, vx_scalar exemplar);
static vx_status ownInitMetaFormatWithPyramid(
    vx_meta_format meta, vx_pyramid exemplar);
static vx_status ownInitMetaFormatWithMatrix(
    vx_meta_format meta, vx_matrix exemplar);
static vx_status ownInitMetaFormatWithDistribution(
    vx_meta_format meta, vx_distribution exemplar);
static vx_status ownInitMetaFormatWithRemap(
    vx_meta_format meta, vx_remap exemplar);
static vx_status ownInitMetaFormatWithLut(
    vx_meta_format meta, vx_lut exemplar);
static vx_status ownInitMetaFormatWithThreshold(
    vx_meta_format meta, vx_threshold exemplar);
static vx_status ownInitMetaFormatWithObjectArray(
    vx_meta_format meta, vx_object_array exemplar);
static vx_status ownInitMetaFormatWithTensor(
    vx_meta_format meta, vx_tensor exemplar);
static vx_status ownInitMetaFormatWithUserDataObject(
    vx_meta_format meta, vx_user_data_object exemplar);
static vx_status ownInitMetaFormatWithRawImage(
    vx_meta_format meta, tivx_raw_image exemplar);


vx_meta_format vxCreateMetaFormat(vx_context context)
{
    vx_meta_format meta = NULL;

    if(ownIsValidContext(context) == vx_true_e)
    {
        meta = (vx_meta_format)ownCreateReference(
            context, VX_TYPE_META_FORMAT, VX_EXTERNAL, &context->base);

        if ((vxGetStatus((vx_reference)meta) == VX_SUCCESS) &&
            (meta->base.type == VX_TYPE_META_FORMAT))
        {
            meta->size = sizeof(tivx_meta_format_t);
            meta->type = VX_TYPE_INVALID;
            meta->valid_rect_callback = NULL;
        }
    }

    return (meta);
}

vx_status vxReleaseMetaFormat(vx_meta_format *meta)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)meta, VX_TYPE_META_FORMAT, VX_EXTERNAL, NULL));
}

VX_API_ENTRY vx_status VX_API_CALL vxSetMetaFormatAttribute(
    vx_meta_format meta, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidSpecificReference(&meta->base, VX_TYPE_META_FORMAT) ==
            vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Invalid meta format reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    if(attribute != VX_VALID_RECT_CALLBACK)
    {
        if (VX_TYPE(attribute) != meta->type)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Invalid meta format type\n");
            status = VX_ERROR_INVALID_TYPE;
        }
    }

    if( NULL == ptr)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: ptr is NULL\n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    if (VX_SUCCESS == status)
    {
        switch(attribute)
        {
            case VX_VALID_RECT_CALLBACK:
                if((meta->type==VX_TYPE_IMAGE)
                   || (meta->type==VX_TYPE_PYRAMID)
                  )
                {
                    if (VX_CHECK_PARAM(ptr, size, vx_kernel_image_valid_rectangle_f, 0x0U))
                    {
                        meta->valid_rect_callback = *(vx_kernel_image_valid_rectangle_f *)ptr;
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Valid rectangle callback not called\n");
                        status = VX_ERROR_INVALID_PARAMETERS;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Invalid type for valid rectangle callback\n");
                    status = VX_ERROR_INVALID_TYPE;
                }
                break;
            case VX_IMAGE_FORMAT:
                if (VX_CHECK_PARAM(ptr, size, vx_df_image, 0x3U))
                {
                    meta->img.format = *(vx_df_image *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Image format error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_IMAGE_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->img.height = *(vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Image height error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_IMAGE_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->img.width = *(vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Image width error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_ARRAY_CAPACITY:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->arr.capacity = *(vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Array capacity error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_ARRAY_ITEMTYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    meta->arr.item_type = *(vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Array item type error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_PYRAMID_FORMAT:
                if (VX_CHECK_PARAM(ptr, size, vx_df_image, 0x3U))
                {
                    meta->pmd.format = *(vx_df_image *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Pyramid format error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_PYRAMID_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->pmd.height = *(vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Pyramid height error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_PYRAMID_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->pmd.width = *(vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Pyramid width error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_PYRAMID_LEVELS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->pmd.levels = *(vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Pyramid levels error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_PYRAMID_SCALE:
                if (VX_CHECK_PARAM(ptr, size, vx_float32, 0x3U))
                {
                    meta->pmd.scale = *(vx_float32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Pyramid scale error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_SCALAR_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    meta->sc.type = *(vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Scalar type error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_MATRIX_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    meta->mat.type = *(vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Matrix type error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_MATRIX_ROWS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->mat.rows = *(vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Matrix rows error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_MATRIX_COLUMNS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->mat.cols = *(vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Matrix columns error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_DISTRIBUTION_BINS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->dist.bins = *(vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Distribution bins error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_DISTRIBUTION_OFFSET:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U))
                {
                    meta->dist.offset = *(vx_int32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Distribution offset error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_DISTRIBUTION_RANGE:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->dist.range = *(vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Distribution range error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_REMAP_SOURCE_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->remap.src_width = *(vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Remap source width error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_REMAP_SOURCE_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->remap.src_height = *(vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Remap source height error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_REMAP_DESTINATION_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->remap.dst_width = *(vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Remap destination width error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_REMAP_DESTINATION_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->remap.dst_height = *(vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Remap source height error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_LUT_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    meta->lut.type = *(vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: LUT type error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_LUT_COUNT:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->lut.count = *(vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: LUT count error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_THRESHOLD_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    meta->thres.type = *(vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Threshold type error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_OBJECT_ARRAY_ITEMTYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    meta->objarr.item_type = *(vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Object array item type error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_OBJECT_ARRAY_NUMITEMS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->objarr.num_items = *(vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Object array numitems error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_TENSOR_NUMBER_OF_DIMS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->tensor.number_of_dimensions = *(vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxSetMetaFormatAttribute: VX_TENSOR_NUMBER_OF_DIMS error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_TENSOR_DIMS:
                if ((size <= (sizeof(vx_size)*TIVX_CONTEXT_MAX_TENSOR_DIMS)) && (((vx_size)ptr & 0x3) == 0))
                {
                    int i;
                    const vx_size *p = ptr;
                    vx_size num_dims = size / sizeof(vx_size);

                    /* Use 'for' loop instead of memcpy since interface type size is different from obj_desc size */
                    for(i=0; i<num_dims; i++)
                    {
                        meta->tensor.dimensions[i] = p[i];
                    }
                    for(;i<TIVX_CONTEXT_MAX_TENSOR_DIMS; i++)
                    {
                        meta->tensor.dimensions[i] = 0;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxSetMetaFormatAttribute: VX_TENSOR_DIMS error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_TENSOR_DATA_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    meta->tensor.data_type = *(vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxSetMetaFormatAttribute: VX_TENSOR_DATA_TYPE error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_TENSOR_FIXED_POINT_POSITION:
                if (VX_CHECK_PARAM(ptr, size, vx_int8, 0x0U))
                {
                    meta->tensor.fixed_point_position = *(vx_int8 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxSetMetaFormatAttribute: VX_TENSOR_FIXED_POINT_POSITION error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_USER_DATA_OBJECT_NAME:
                if (size <= VX_MAX_REFERENCE_NAME)
                {
                    tivx_obj_desc_strncpy(meta->user_data_object.type_name, (void*)ptr, VX_MAX_REFERENCE_NAME);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: VX_USER_DATA_OBJECT_NAME error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_USER_DATA_OBJECT_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->user_data_object.size = *(vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxSetMetaFormatAttribute: VX_USER_DATA_OBJECT_SIZE error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case TIVX_RAW_IMAGE_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->raw_image.width = *(vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxSetMetaFormatAttribute: TIVX_RAW_IMAGE_WIDTH error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case TIVX_RAW_IMAGE_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->raw_image.height = *(vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxSetMetaFormatAttribute: TIVX_RAW_IMAGE_HEIGHT error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case TIVX_RAW_IMAGE_NUM_EXPOSURES:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->raw_image.num_exposures = *(vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxSetMetaFormatAttribute: TIVX_RAW_IMAGE_NUM_EXPOSURES error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case TIVX_RAW_IMAGE_LINE_INTERLEAVED:
                if (VX_CHECK_PARAM(ptr, size, vx_bool, 0x3U))
                {
                    meta->raw_image.line_interleaved =  *(vx_bool *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: TIVX_RAW_IMAGE_LINE_INTERLEAVED error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case TIVX_RAW_IMAGE_FORMAT:
                if ((size <= (sizeof(tivx_raw_image_format_t)*TIVX_RAW_IMAGE_MAX_EXPOSURES)) && (((vx_size)ptr & 0x3) == 0))
                {
                    vx_size num_dims = size / sizeof(tivx_raw_image_format_t);

                    memcpy(&meta->raw_image.format, ptr, sizeof(tivx_raw_image_format_t)*num_dims);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: TIVX_RAW_IMAGE_FORMAT error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case TIVX_RAW_IMAGE_META_HEIGHT_BEFORE:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->raw_image.meta_height_before = *(vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: TIVX_RAW_IMAGE_META_HEIGHT_BEFORE error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case TIVX_RAW_IMAGE_META_HEIGHT_AFTER:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->raw_image.meta_height_after = *(vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: TIVX_RAW_IMAGE_META_HEIGHT_AFTER error\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            default:
                VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatAttribute: Invalid attribute\n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetMetaFormatFromReference(
    vx_meta_format meta, vx_reference exemplar)
{
    vx_status status = VX_SUCCESS;

    if ((NULL == meta) || (NULL == exemplar))
    {
        status = VX_FAILURE;
        if (NULL == meta)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: meta value is NULL\n");
        }
        if (NULL == exemplar)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: exemplar value is NULL\n");
        }
    }
    else
    {
        switch (exemplar->type)
        {
            case VX_TYPE_IMAGE:
                status = ownInitMetaFormatWithImage(meta, (vx_image)exemplar);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: Image init meta format failure\n");
                }
                break;
            case VX_TYPE_ARRAY:
                status = ownInitMetaFormatWithArray(meta, (vx_array)exemplar);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: Array init meta format failure\n");
                }
                break;
            case VX_TYPE_SCALAR:
                status = ownInitMetaFormatWithScalar(
                    meta, (vx_scalar)exemplar);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: Scalar init meta format failure\n");
                }
                break;
            case VX_TYPE_PYRAMID:
                status = ownInitMetaFormatWithPyramid(
                    meta, (vx_pyramid)exemplar);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: Pyramid init meta format failure\n");
                }
                break;
            case VX_TYPE_MATRIX:
                status = ownInitMetaFormatWithMatrix(
                    meta, (vx_matrix)exemplar);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: Matrix init meta format failure\n");
                }
                break;
            case VX_TYPE_DISTRIBUTION:
                status = ownInitMetaFormatWithDistribution(
                    meta, (vx_distribution)exemplar);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: Distribution init meta format failure\n");
                }
                break;
            case VX_TYPE_THRESHOLD:
                status = ownInitMetaFormatWithThreshold(
                    meta, (vx_threshold)exemplar);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: Threshold init meta format failure\n");
                }
                break;
            case VX_TYPE_REMAP:
                status = ownInitMetaFormatWithRemap(
                    meta, (vx_remap)exemplar);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: Remap init meta format failure\n");
                }
                break;
            case VX_TYPE_LUT:
                status = ownInitMetaFormatWithLut(
                    meta, (vx_lut)exemplar);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: LUT init meta format failure\n");
                }
                break;
            case VX_TYPE_OBJECT_ARRAY:
                status = ownInitMetaFormatWithObjectArray(
                    meta, (vx_object_array)exemplar);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: Object array init meta format failure\n");
                }
                break;
            case VX_TYPE_TENSOR:
                status = ownInitMetaFormatWithTensor(meta, (vx_tensor)exemplar);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: Tensor init meta format failure\n");
                }
                break;
            case VX_TYPE_USER_DATA_OBJECT:
                status = ownInitMetaFormatWithUserDataObject(meta, (vx_user_data_object)exemplar);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: User Data Object init meta format failure\n");
                }
                break;

            case TIVX_TYPE_RAW_IMAGE:
                status = ownInitMetaFormatWithRawImage(meta, (tivx_raw_image)exemplar);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: Raw image init meta format failure\n");
                }
                break;

            default:
                VX_PRINT(VX_ZONE_ERROR, "vxSetMetaFormatFromReference: invalid attribute\n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return (status);
}

static vx_status ownInitMetaFormatWithPyramid(
    vx_meta_format meta, vx_pyramid exemplar)
{
    vx_status status = VX_SUCCESS;

    status |= vxQueryPyramid(exemplar, VX_PYRAMID_WIDTH, &meta->pmd.width,
        sizeof(meta->pmd.width));
    status |= vxQueryPyramid(exemplar, VX_PYRAMID_HEIGHT, &meta->pmd.height,
        sizeof(meta->pmd.height));
    status |= vxQueryPyramid(exemplar, VX_PYRAMID_FORMAT, &meta->pmd.format,
        sizeof(meta->pmd.format));
    status |= vxQueryPyramid(exemplar, VX_PYRAMID_LEVELS, &meta->pmd.levels,
        sizeof(meta->pmd.levels));
    status |= vxQueryPyramid(exemplar, VX_PYRAMID_SCALE, &meta->pmd.scale,
        sizeof(meta->pmd.scale));

    return (status);
}

static vx_status ownInitMetaFormatWithImage(
    vx_meta_format meta, vx_image exemplar)
{
    vx_status status = VX_SUCCESS;

    status |= vxQueryImage(exemplar, VX_IMAGE_WIDTH, &meta->img.width,
        sizeof(meta->img.width));
    status |= vxQueryImage(exemplar, VX_IMAGE_HEIGHT, &meta->img.height,
        sizeof(meta->img.height));
    status |= vxQueryImage(exemplar, VX_IMAGE_FORMAT, &meta->img.format,
        sizeof(meta->img.format));

    return (status);
}

static vx_status ownInitMetaFormatWithArray(
    vx_meta_format meta, vx_array exemplar)
{
    vx_status status = VX_SUCCESS;

    status |= vxQueryArray(exemplar, VX_ARRAY_ITEMTYPE, &meta->arr.item_type,
        sizeof(meta->arr.item_type));
    status |= vxQueryArray(exemplar, VX_ARRAY_CAPACITY, &meta->arr.capacity,
        sizeof(meta->arr.capacity));

    return (status);
}

static vx_status ownInitMetaFormatWithScalar(
    vx_meta_format meta, vx_scalar exemplar)
{
    vx_status status = VX_SUCCESS;

    status |= vxQueryScalar(exemplar, VX_SCALAR_TYPE, &meta->sc.type,
        sizeof(meta->sc.type));

    return (status);
}

static vx_status ownInitMetaFormatWithMatrix(
    vx_meta_format meta, vx_matrix exemplar)
{
    vx_status status = VX_SUCCESS;

    status |= vxQueryMatrix(exemplar, VX_MATRIX_TYPE, &meta->mat.type,
        sizeof(meta->mat.type));
    status |= vxQueryMatrix(exemplar, VX_MATRIX_ROWS, &meta->mat.rows,
        sizeof(meta->mat.rows));
    status |= vxQueryMatrix(exemplar, VX_MATRIX_COLUMNS, &meta->mat.cols,
        sizeof(meta->mat.cols));

    return status;
}

static vx_status ownInitMetaFormatWithDistribution(
    vx_meta_format meta, vx_distribution exemplar)
{
    vx_status status = VX_SUCCESS;

    status |= vxQueryDistribution(exemplar, VX_DISTRIBUTION_BINS, &meta->dist.bins,
        sizeof(meta->dist.bins));
    status |= vxQueryDistribution(exemplar, VX_DISTRIBUTION_OFFSET, &meta->dist.offset,
        sizeof(meta->dist.offset));
    status |= vxQueryDistribution(exemplar, VX_DISTRIBUTION_RANGE, &meta->dist.range,
        sizeof(meta->dist.range));

    return status;
}

static vx_status ownInitMetaFormatWithRemap(
    vx_meta_format meta, vx_remap exemplar)
{
    vx_status status = VX_SUCCESS;

    status |= vxQueryRemap(exemplar, VX_REMAP_SOURCE_WIDTH, &meta->remap.src_width,
        sizeof(meta->remap.src_width));
    status |= vxQueryRemap(exemplar, VX_REMAP_SOURCE_HEIGHT, &meta->remap.src_height,
        sizeof(meta->remap.src_height));
    status |= vxQueryRemap(exemplar, VX_REMAP_DESTINATION_WIDTH, &meta->remap.dst_width,
        sizeof(meta->remap.dst_width));
    status |= vxQueryRemap(exemplar, VX_REMAP_DESTINATION_HEIGHT, &meta->remap.dst_height,
        sizeof(meta->remap.dst_height));

    return status;
}

static vx_status ownInitMetaFormatWithLut(
    vx_meta_format meta, vx_lut exemplar)
{
    vx_status status = VX_SUCCESS;

    status |= vxQueryLUT(exemplar, VX_LUT_TYPE, &meta->lut.type,
        sizeof(meta->lut.type));
    status |= vxQueryLUT(exemplar, VX_LUT_COUNT, &meta->lut.count,
        sizeof(meta->lut.count));

    return status;
}

static vx_status ownInitMetaFormatWithThreshold(
    vx_meta_format meta, vx_threshold exemplar)
{
    vx_status status = VX_SUCCESS;

    status |= vxQueryThreshold(exemplar, VX_THRESHOLD_TYPE, &meta->thres.type,
        sizeof(meta->thres.type));

    return status;
}

static vx_status ownInitMetaFormatWithObjectArray(
    vx_meta_format meta, vx_object_array exemplar)
{
    vx_status status = VX_SUCCESS;

    status |= vxQueryObjectArray(exemplar, VX_OBJECT_ARRAY_ITEMTYPE, &meta->objarr.item_type,
        sizeof(meta->objarr.item_type));
    status |= vxQueryObjectArray(exemplar, VX_OBJECT_ARRAY_NUMITEMS, &meta->objarr.num_items,
        sizeof(meta->objarr.num_items));

    return status;
}

static vx_status ownInitMetaFormatWithTensor(
    vx_meta_format meta, vx_tensor exemplar)
{
    vx_status status = VX_SUCCESS;

    status |= vxQueryTensor(exemplar, VX_TENSOR_NUMBER_OF_DIMS, &meta->tensor.number_of_dimensions,
        sizeof(meta->tensor.number_of_dimensions));
    status |= vxQueryTensor(exemplar, VX_TENSOR_DIMS, &meta->tensor.dimensions,
        sizeof(meta->tensor.dimensions));
    status |= vxQueryTensor(exemplar, VX_TENSOR_DATA_TYPE, &meta->tensor.data_type,
        sizeof(meta->tensor.data_type));
    status |= vxQueryTensor(exemplar, VX_TENSOR_FIXED_POINT_POSITION, &meta->tensor.fixed_point_position,
        sizeof(meta->tensor.fixed_point_position));

    return (status);
}

static vx_status ownInitMetaFormatWithUserDataObject(
    vx_meta_format meta, vx_user_data_object exemplar)
{
    vx_status status = VX_SUCCESS;

    status |= vxQueryUserDataObject(exemplar, VX_USER_DATA_OBJECT_NAME, &meta->user_data_object.type_name,
        sizeof(meta->user_data_object.type_name));
    status |= vxQueryUserDataObject(exemplar, VX_USER_DATA_OBJECT_SIZE, &meta->user_data_object.size,
        sizeof(meta->user_data_object.size));

    return (status);
}

static vx_status ownInitMetaFormatWithRawImage(
    vx_meta_format meta, tivx_raw_image exemplar)
{
    vx_status status = VX_SUCCESS;

    status |= tivxQueryRawImage(exemplar, TIVX_RAW_IMAGE_WIDTH, &meta->raw_image.width,
        sizeof(meta->raw_image.width));
    status |= tivxQueryRawImage(exemplar, TIVX_RAW_IMAGE_HEIGHT, &meta->raw_image.height,
        sizeof(meta->raw_image.height));
    status |= tivxQueryRawImage(exemplar, TIVX_RAW_IMAGE_NUM_EXPOSURES, &meta->raw_image.num_exposures,
        sizeof(meta->raw_image.num_exposures));
    status |= tivxQueryRawImage(exemplar, TIVX_RAW_IMAGE_LINE_INTERLEAVED, &meta->raw_image.line_interleaved,
        sizeof(meta->raw_image.line_interleaved));
    status |= tivxQueryRawImage(exemplar, TIVX_RAW_IMAGE_FORMAT, &meta->raw_image.format,
        sizeof(meta->raw_image.format));
    status |= tivxQueryRawImage(exemplar, TIVX_RAW_IMAGE_META_HEIGHT_BEFORE, &meta->raw_image.meta_height_before,
        sizeof(meta->raw_image.meta_height_before));
    status |= tivxQueryRawImage(exemplar, TIVX_RAW_IMAGE_META_HEIGHT_AFTER, &meta->raw_image.meta_height_after,
        sizeof(meta->raw_image.meta_height_after));

    return (status);
}

static vx_bool ownIsMetaFormatImageEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = vx_false_e;

    if ( (ownIsValidSpecificReference(&meta1->base, VX_TYPE_META_FORMAT) == vx_true_e) &&
         (ownIsValidSpecificReference(&meta2->base, VX_TYPE_META_FORMAT) == vx_true_e) )
    {
        if ( (meta1->img.width  == meta2->img.width) &&
             (meta1->img.height == meta2->img.height) &&
             (meta1->img.format == meta2->img.format) )
        {
            is_equal = vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "ownIsMetaFormatImageEqual: Image object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatArrayEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = vx_false_e;

    if ( (ownIsValidSpecificReference(&meta1->base, VX_TYPE_META_FORMAT) == vx_true_e) &&
         (ownIsValidSpecificReference(&meta2->base, VX_TYPE_META_FORMAT) == vx_true_e) )
    {
        if ( (meta1->arr.item_type == meta2->arr.item_type) &&
             (meta1->arr.capacity  == meta2->arr.capacity) )
        {
            is_equal = vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "ownIsMetaFormatArrayEqual: Array object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatScalarEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = vx_false_e;

    if ( (ownIsValidSpecificReference(&meta1->base, VX_TYPE_META_FORMAT) == vx_true_e) &&
         (ownIsValidSpecificReference(&meta2->base, VX_TYPE_META_FORMAT) == vx_true_e) )
    {
        if ( (meta1->sc.type == meta2->sc.type) )
        {
            is_equal = vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "ownIsMetaFormatScalarEqual: Scalar object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatPyramidEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = vx_false_e;

    if ( (ownIsValidSpecificReference(&meta1->base, VX_TYPE_META_FORMAT) == vx_true_e) &&
         (ownIsValidSpecificReference(&meta2->base, VX_TYPE_META_FORMAT) == vx_true_e) )
    {
        if ( (meta1->pmd.width  == meta2->pmd.width) &&
             (meta1->pmd.height == meta2->pmd.height) &&
             (meta1->pmd.format == meta2->pmd.format) &&
             (meta1->pmd.levels == meta2->pmd.levels) &&
             (meta1->pmd.scale  == meta2->pmd.scale) )
        {
            is_equal = vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "ownIsMetaFormatPyramidEqual: Pyramid object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatMatrixEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = vx_false_e;

    if ( (ownIsValidSpecificReference(&meta1->base, VX_TYPE_META_FORMAT) == vx_true_e) &&
         (ownIsValidSpecificReference(&meta2->base, VX_TYPE_META_FORMAT) == vx_true_e) )
    {
        if ( (meta1->mat.type  == meta2->mat.type) &&
             (meta1->mat.rows  == meta2->mat.rows) &&
             (meta1->mat.cols  == meta2->mat.cols) )
        {
            is_equal = vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "ownIsMetaFormatMatrixEqual: Matrix object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatDistributionEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = vx_false_e;

    if ( (ownIsValidSpecificReference(&meta1->base, VX_TYPE_META_FORMAT) == vx_true_e) &&
         (ownIsValidSpecificReference(&meta2->base, VX_TYPE_META_FORMAT) == vx_true_e) )
    {
        if ( (meta1->dist.bins   == meta2->dist.bins) &&
             (meta1->dist.offset == meta2->dist.offset) &&
             (meta1->dist.range  == meta2->dist.range) )
        {
            is_equal = vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "ownIsMetaFormatDistributionEqual: Distribution object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatRemapEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = vx_false_e;

    if ( (ownIsValidSpecificReference(&meta1->base, VX_TYPE_META_FORMAT) == vx_true_e) &&
         (ownIsValidSpecificReference(&meta2->base, VX_TYPE_META_FORMAT) == vx_true_e) )
    {
        if ( (meta1->remap.src_width  == meta2->remap.src_width) &&
             (meta1->remap.src_height == meta2->remap.src_height) &&
             (meta1->remap.dst_width  == meta2->remap.dst_width) &&
             (meta1->remap.dst_height == meta2->remap.dst_height) )
        {
            is_equal = vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "ownIsMetaFormatRemapEqual: Remap object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatThresholdEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = vx_false_e;

    if ( (ownIsValidSpecificReference(&meta1->base, VX_TYPE_META_FORMAT) == vx_true_e) &&
         (ownIsValidSpecificReference(&meta2->base, VX_TYPE_META_FORMAT) == vx_true_e) )
    {
        if ( (meta1->thres.type  == meta2->thres.type) )
        {
            is_equal = vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "ownIsMetaFormatThresholdEqual: Threshold object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatObjectArrayEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = vx_false_e;

    if ( (ownIsValidSpecificReference(&meta1->base, VX_TYPE_META_FORMAT) == vx_true_e) &&
         (ownIsValidSpecificReference(&meta2->base, VX_TYPE_META_FORMAT) == vx_true_e) )
    {
        if ( (meta1->objarr.item_type == meta2->objarr.item_type) &&
             (meta1->objarr.num_items == meta2->objarr.num_items) )
        {
            is_equal = vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "ownIsMetaFormatObjectArrayEqual: Object Array object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatLutEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = vx_false_e;

    if ( (ownIsValidSpecificReference(&meta1->base, VX_TYPE_META_FORMAT) == vx_true_e) &&
         (ownIsValidSpecificReference(&meta2->base, VX_TYPE_META_FORMAT) == vx_true_e) )
    {
        if ( (meta1->lut.type  == meta2->lut.type) &&
             (meta1->lut.count == meta2->lut.count) )
        {
            is_equal = vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "ownIsMetaFormatLutEqual: LUT object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatTensorEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = vx_false_e;
    vx_uint32 i;

    if ( (ownIsValidSpecificReference(&meta1->base, VX_TYPE_META_FORMAT) == vx_true_e) &&
         (ownIsValidSpecificReference(&meta2->base, VX_TYPE_META_FORMAT) == vx_true_e) )
    {
        if ( (meta1->tensor.number_of_dimensions == meta2->tensor.number_of_dimensions) &&
             (meta1->tensor.data_type == meta2->tensor.data_type) &&
             (meta1->tensor.fixed_point_position == meta2->tensor.fixed_point_position) )
        {
            for (i = 0; i < meta1->tensor.number_of_dimensions; i++)
            {
                if (meta1->tensor.dimensions[i] != meta2->tensor.dimensions[i])
                {
                    break;
                }
            }
            if (i == meta1->tensor.number_of_dimensions)
            {
                is_equal = vx_true_e;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "ownIsMetaFormatTensorEqual: Tensor object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatUserDataObjectEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = vx_false_e;

    if ( (ownIsValidSpecificReference(&meta1->base, VX_TYPE_META_FORMAT) == vx_true_e) &&
         (ownIsValidSpecificReference(&meta2->base, VX_TYPE_META_FORMAT) == vx_true_e) )
    {
        if ( (meta1->user_data_object.size == meta2->user_data_object.size) &&
             ( 0 == strncmp(meta1->user_data_object.type_name, meta2->user_data_object.type_name, VX_MAX_REFERENCE_NAME) ) )
        {
            is_equal = vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "ownIsMetaFormatUserDataObjectEqual: User data object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatRawImageEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = vx_false_e;
    vx_uint32 i;

    if ( (ownIsValidSpecificReference(&meta1->base, VX_TYPE_META_FORMAT) == vx_true_e) &&
         (ownIsValidSpecificReference(&meta2->base, VX_TYPE_META_FORMAT) == vx_true_e) )
    {
        if ( (meta1->raw_image.width              == meta2->raw_image.width) &&
             (meta1->raw_image.height             == meta2->raw_image.height) &&
             (meta1->raw_image.num_exposures      == meta2->raw_image.num_exposures) &&
             (meta1->raw_image.line_interleaved   == meta2->raw_image.line_interleaved) &&
             (meta1->raw_image.meta_height_before == meta2->raw_image.meta_height_before) &&
             (meta1->raw_image.meta_height_after  == meta2->raw_image.meta_height_after) )
        {
            for (i = 0; i < meta1->raw_image.num_exposures; i++)
            {
                if ( (meta1->raw_image.format[i].pixel_container != meta2->raw_image.format[i].pixel_container) ||
                     (meta1->raw_image.format[i].msb != meta2->raw_image.format[i].msb))
                {
                    break;
                }
            }
            if (i == meta1->raw_image.num_exposures)
            {
                is_equal = vx_true_e;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "ownIsMetaFormatRawImageEqual: Raw Image object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

vx_bool ownIsMetaFormatEqual(
    vx_meta_format meta1, vx_meta_format meta2, vx_enum type)
{
    vx_bool is_equal = vx_false_e;

    if ( (ownIsValidSpecificReference(&meta1->base, VX_TYPE_META_FORMAT) == vx_true_e) &&
         (ownIsValidSpecificReference(&meta2->base, VX_TYPE_META_FORMAT) == vx_true_e) )
    {
        switch (type)
        {
            case VX_TYPE_IMAGE:
                is_equal = ownIsMetaFormatImageEqual(meta1, meta2);
                break;
            case VX_TYPE_ARRAY:
                is_equal = ownIsMetaFormatArrayEqual(meta1, meta2);
                break;
            case VX_TYPE_SCALAR:
                is_equal = ownIsMetaFormatScalarEqual(meta1, meta2);
                break;
            case VX_TYPE_PYRAMID:
                is_equal = ownIsMetaFormatPyramidEqual(meta1, meta2);
                break;
            case VX_TYPE_MATRIX:
                is_equal = ownIsMetaFormatMatrixEqual(meta1, meta2);
                break;
            case VX_TYPE_DISTRIBUTION:
                is_equal = ownIsMetaFormatDistributionEqual(meta1, meta2);
                break;
            case VX_TYPE_THRESHOLD:
                is_equal = ownIsMetaFormatThresholdEqual(meta1, meta2);
                break;
            case VX_TYPE_REMAP:
                is_equal = ownIsMetaFormatRemapEqual(meta1, meta2);
                break;
            case VX_TYPE_LUT:
                is_equal = ownIsMetaFormatLutEqual(meta1, meta2);
                break;
            case VX_TYPE_OBJECT_ARRAY:
                is_equal = ownIsMetaFormatObjectArrayEqual(meta1, meta2);
                break;
            case VX_TYPE_TENSOR:
                is_equal = ownIsMetaFormatTensorEqual(meta1, meta2);
                break;
            case VX_TYPE_USER_DATA_OBJECT:
                is_equal = ownIsMetaFormatUserDataObjectEqual(meta1, meta2);
                break;
            case TIVX_TYPE_RAW_IMAGE:
                is_equal = ownIsMetaFormatRawImageEqual(meta1, meta2);
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "ownIsMetaFormatEqual: invalid attribute\n");
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "ownIsMetaFormatEqual: Meta format is invalid!\n");
    }

    return is_equal;
}
