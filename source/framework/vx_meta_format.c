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
static vx_bool ownIsMetaFormatArrayEqual(
    vx_meta_format meta1, vx_meta_format meta2);
static vx_bool ownIsMetaFormatDistributionEqual(
    vx_meta_format meta1, vx_meta_format meta2);
static vx_bool ownIsMetaFormatImageEqual(
    vx_meta_format meta1, vx_meta_format meta2);
static vx_bool ownIsMetaFormatLutEqual(
    vx_meta_format meta1, vx_meta_format meta2);
static vx_bool ownIsMetaFormatMatrixEqual(
    vx_meta_format meta1, vx_meta_format meta2);
static vx_bool ownIsMetaFormatObjectArrayEqual(
    vx_meta_format meta1, vx_meta_format meta2);
static vx_bool ownIsMetaFormatPyramidEqual(
    vx_meta_format meta1, vx_meta_format meta2);
static vx_bool ownIsMetaFormatRawImageEqual(
    vx_meta_format meta1, vx_meta_format meta2);
static vx_bool ownIsMetaFormatTensorEqual(
    vx_meta_format meta1, vx_meta_format meta2);
static vx_bool ownIsMetaFormatScalarEqual(
    vx_meta_format meta1, vx_meta_format meta2);
static vx_bool ownIsMetaFormatTensorEqual(
    vx_meta_format meta1, vx_meta_format meta2);
static vx_bool ownIsMetaFormatRemapEqual(
    vx_meta_format meta1, vx_meta_format meta2);
static vx_bool ownIsMetaFormatThresholdEqual(
    vx_meta_format meta1, vx_meta_format meta2);
static vx_bool ownIsMetaFormatUserDataObjectEqual(
    vx_meta_format meta1, vx_meta_format meta2);

vx_meta_format ownCreateMetaFormat(vx_context context)
{
    vx_meta_format meta = NULL;

    if(ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        meta = (vx_meta_format)ownCreateReference(
            context, (vx_enum)VX_TYPE_META_FORMAT, (vx_enum)VX_EXTERNAL, &context->base);

        if ((vxGetStatus((vx_reference)meta) == (vx_status)VX_SUCCESS) &&
            (meta->base.type == (vx_enum)VX_TYPE_META_FORMAT))
        {
            meta->size = sizeof(tivx_meta_format_t);
            meta->type = (vx_enum)VX_TYPE_INVALID;
            meta->valid_rect_callback = NULL;
        }
    }

    return (meta);
}

vx_status ownReleaseMetaFormat(vx_meta_format *meta)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)meta, (vx_enum)VX_TYPE_META_FORMAT, (vx_enum)VX_EXTERNAL, NULL));
}

VX_API_ENTRY vx_status VX_API_CALL vxSetMetaFormatAttribute(
    vx_meta_format meta, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( ownIsValidSpecificReference((vx_reference)meta, (vx_enum)VX_TYPE_META_FORMAT) ==
            (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid meta format reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if(attribute != (vx_enum)VX_VALID_RECT_CALLBACK)
        {
            if (VX_TYPE(attribute) != (vx_uint32)meta->type)
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid meta format type\n");
                status = (vx_status)VX_ERROR_INVALID_TYPE;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if( NULL == ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "ptr is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        switch(attribute)
        {
            case (vx_enum)VX_VALID_RECT_CALLBACK:
                if((meta->type==(vx_enum)VX_TYPE_IMAGE)
                   || (meta->type==(vx_enum)VX_TYPE_PYRAMID)
                  )
                {
                    if (VX_CHECK_PARAM(ptr, size, vx_kernel_image_valid_rectangle_f, 0x0U))
                    {
                        meta->valid_rect_callback = *(const vx_kernel_image_valid_rectangle_f *)ptr;
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Valid rectangle callback not called\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Invalid type for valid rectangle callback\n");
                    status = (vx_status)VX_ERROR_INVALID_TYPE;
                }
                break;
            case (vx_enum)VX_IMAGE_FORMAT:
                if (VX_CHECK_PARAM(ptr, size, vx_df_image, 0x3U))
                {
                    meta->img.format = *(const vx_df_image *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Image format error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_IMAGE_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->img.height = *(const vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Image height error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_IMAGE_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->img.width = *(const vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Image width error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_ARRAY_CAPACITY:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->arr.capacity = *(const vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Array capacity error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_ARRAY_ITEMTYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    meta->arr.item_type = *(const vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Array item type error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_PYRAMID_FORMAT:
                if (VX_CHECK_PARAM(ptr, size, vx_df_image, 0x3U))
                {
                    meta->pmd.format = *(const vx_df_image *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Pyramid format error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_PYRAMID_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->pmd.height = *(const vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Pyramid height error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_PYRAMID_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->pmd.width = *(const vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Pyramid width error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_PYRAMID_LEVELS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->pmd.levels = *(const vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Pyramid levels error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_PYRAMID_SCALE:
                if (VX_CHECK_PARAM(ptr, size, vx_float32, 0x3U))
                {
                    meta->pmd.scale = *(const vx_float32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Pyramid scale error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_SCALAR_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    meta->sc.type = *(const vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Scalar type error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_MATRIX_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    meta->mat.type = *(const vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Matrix type error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)(vx_enum)VX_MATRIX_ROWS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->mat.rows = *(const vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Matrix rows error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_MATRIX_COLUMNS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->mat.cols = *(const vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Matrix columns error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_DISTRIBUTION_BINS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->dist.bins = *(const vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Distribution bins error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_DISTRIBUTION_OFFSET:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U))
                {
                    meta->dist.offset = *(const vx_int32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Distribution offset error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_DISTRIBUTION_RANGE:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->dist.range = *(const vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Distribution range error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_REMAP_SOURCE_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->remap.src_width = *(const vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Remap source width error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_REMAP_SOURCE_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->remap.src_height = *(const vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Remap source height error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_REMAP_DESTINATION_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->remap.dst_width = *(const vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Remap destination width error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_REMAP_DESTINATION_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->remap.dst_height = *(const vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Remap source height error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_LUT_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    meta->lut.type = *(const vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "LUT type error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_LUT_COUNT:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->lut.count = *(const vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "LUT count error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_THRESHOLD_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    meta->thres.type = *(const vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Threshold type error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_OBJECT_ARRAY_ITEMTYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    meta->objarr.item_type = *(const vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Object array item type error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_OBJECT_ARRAY_NUMITEMS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->objarr.num_items = *(const vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Object array numitems error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_TENSOR_NUMBER_OF_DIMS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->tensor.number_of_dimensions = *(const vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "VX_TENSOR_NUMBER_OF_DIMS error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_TENSOR_DIMS:
                if ((size <= (sizeof(vx_size)*(vx_size)TIVX_CONTEXT_MAX_TENSOR_DIMS)) && (((vx_size)ptr & 0x3U) == 0U))
                {
                    int32_t i;
                    const vx_size *p = ptr;
                    vx_size num_dims = size / sizeof(vx_size);

                    /* Use 'for' loop instead of memcpy since interface type size is different from obj_desc size */
                    for(i=0; i<(int32_t)num_dims; i++)
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
                    VX_PRINT(VX_ZONE_ERROR, "VX_TENSOR_DIMS error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_TENSOR_DATA_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    meta->tensor.data_type = *(const vx_enum *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "VX_TENSOR_DATA_TYPE error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_TENSOR_FIXED_POINT_POSITION:
                if (VX_CHECK_PARAM(ptr, size, vx_int8, 0x0U))
                {
                    meta->tensor.fixed_point_position = *(const vx_int8 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "VX_TENSOR_FIXED_POINT_POSITION error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_USER_DATA_OBJECT_NAME:
                if (size <= (vx_size)VX_MAX_REFERENCE_NAME)
                {
                    tivx_obj_desc_strncpy(meta->user_data_object.type_name, (void*)ptr, VX_MAX_REFERENCE_NAME);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "VX_USER_DATA_OBJECT_NAME error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)VX_USER_DATA_OBJECT_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    meta->user_data_object.size = *(const vx_size *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "VX_USER_DATA_OBJECT_SIZE error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)TIVX_RAW_IMAGE_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->raw_image.width = *(const vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "TIVX_RAW_IMAGE_WIDTH error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)TIVX_RAW_IMAGE_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->raw_image.height = *(const vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "TIVX_RAW_IMAGE_HEIGHT error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)TIVX_RAW_IMAGE_NUM_EXPOSURES:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->raw_image.num_exposures = *(const vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "TIVX_RAW_IMAGE_NUM_EXPOSURES error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)TIVX_RAW_IMAGE_LINE_INTERLEAVED:
                if (VX_CHECK_PARAM(ptr, size, vx_bool, 0x3U))
                {
                    meta->raw_image.line_interleaved =  *(const vx_bool *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "TIVX_RAW_IMAGE_LINE_INTERLEAVED error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)TIVX_RAW_IMAGE_FORMAT:
                if ((size <= (sizeof(tivx_raw_image_format_t)*(vx_size)TIVX_RAW_IMAGE_MAX_EXPOSURES)) && (((vx_size)ptr & 0x3U) == 0U))
                {
                    vx_size num_dims = size / sizeof(tivx_raw_image_format_t);

                    memcpy(&meta->raw_image.format, ptr, sizeof(tivx_raw_image_format_t)*num_dims);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "TIVX_RAW_IMAGE_FORMAT error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)TIVX_RAW_IMAGE_META_HEIGHT_BEFORE:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->raw_image.meta_height_before = *(const vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "TIVX_RAW_IMAGE_META_HEIGHT_BEFORE error\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case (vx_enum)TIVX_RAW_IMAGE_META_HEIGHT_AFTER:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    meta->raw_image.meta_height_after = *(const vx_uint32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "TIVX_RAW_IMAGE_META_HEIGHT_AFTER error\n");
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

VX_API_ENTRY vx_status VX_API_CALL vxSetMetaFormatFromReference(
    vx_meta_format meta, vx_reference exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((NULL == meta) || (NULL == exemplar))
    {
        status = (vx_status)VX_FAILURE;
        if (NULL == meta)
        {
            VX_PRINT(VX_ZONE_ERROR, "meta value is NULL\n");
        }
        if (NULL == exemplar)
        {
            VX_PRINT(VX_ZONE_ERROR, "exemplar value is NULL\n");
        }
    }
    else
    {
        switch (exemplar->type)
        {
            case (vx_enum)VX_TYPE_IMAGE:
                status = ownInitMetaFormatWithImage(meta, (vx_image)exemplar);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Image init meta format failure\n");
                }
                break;
            case (vx_enum)VX_TYPE_ARRAY:
                status = ownInitMetaFormatWithArray(meta, (vx_array)exemplar);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Array init meta format failure\n");
                }
                break;
            case (vx_enum)VX_TYPE_SCALAR:
                status = ownInitMetaFormatWithScalar(
                    meta, (vx_scalar)exemplar);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Scalar init meta format failure\n");
                }
                break;
            case (vx_enum)VX_TYPE_PYRAMID:
                status = ownInitMetaFormatWithPyramid(
                    meta, (vx_pyramid)exemplar);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Pyramid init meta format failure\n");
                }
                break;
            case (vx_enum)VX_TYPE_MATRIX:
                status = ownInitMetaFormatWithMatrix(
                    meta, (vx_matrix)exemplar);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Matrix init meta format failure\n");
                }
                break;
            case (vx_enum)VX_TYPE_DISTRIBUTION:
                status = ownInitMetaFormatWithDistribution(
                    meta, (vx_distribution)exemplar);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Distribution init meta format failure\n");
                }
                break;
            case (vx_enum)VX_TYPE_THRESHOLD:
                status = ownInitMetaFormatWithThreshold(
                    meta, (vx_threshold)exemplar);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Threshold init meta format failure\n");
                }
                break;
            case (vx_enum)VX_TYPE_REMAP:
                status = ownInitMetaFormatWithRemap(
                    meta, (vx_remap)exemplar);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Remap init meta format failure\n");
                }
                break;
            case (vx_enum)VX_TYPE_LUT:
                status = ownInitMetaFormatWithLut(
                    meta, (vx_lut)exemplar);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "LUT init meta format failure\n");
                }
                break;
            case (vx_enum)VX_TYPE_OBJECT_ARRAY:
                status = ownInitMetaFormatWithObjectArray(
                    meta, (vx_object_array)exemplar);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Object array init meta format failure\n");
                }
                break;
            case (vx_enum)VX_TYPE_TENSOR:
                status = ownInitMetaFormatWithTensor(meta, (vx_tensor)exemplar);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Tensor init meta format failure\n");
                }
                break;
            case VX_TYPE_USER_DATA_OBJECT:
                status = ownInitMetaFormatWithUserDataObject(meta, (vx_user_data_object)exemplar);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "User Data Object init meta format failure\n");
                }
                break;

            case TIVX_TYPE_RAW_IMAGE:
                status = ownInitMetaFormatWithRawImage(meta, (tivx_raw_image)exemplar);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Raw image init meta format failure\n");
                }
                break;

            default:
                VX_PRINT(VX_ZONE_ERROR, "invalid attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }

        /* Copy the type information. */
        if (status == (vx_status)VX_SUCCESS)
        {
            meta->type = ((vx_reference)exemplar)->type;
        }
    }

    return (status);
}

static vx_status ownInitMetaFormatWithPyramid(
    vx_meta_format meta, vx_pyramid exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, vxQueryPyramid(exemplar, (vx_enum)VX_PYRAMID_WIDTH, &meta->pmd.width,
        sizeof(meta->pmd.width)));
    tivxCheckStatus(&status, vxQueryPyramid(exemplar, (vx_enum)VX_PYRAMID_HEIGHT, &meta->pmd.height,
        sizeof(meta->pmd.height)));
    tivxCheckStatus(&status, vxQueryPyramid(exemplar, (vx_enum)VX_PYRAMID_FORMAT, &meta->pmd.format,
        sizeof(meta->pmd.format)));
    tivxCheckStatus(&status, vxQueryPyramid(exemplar, (vx_enum)VX_PYRAMID_LEVELS, &meta->pmd.levels,
        sizeof(meta->pmd.levels)));
    tivxCheckStatus(&status, vxQueryPyramid(exemplar, (vx_enum)VX_PYRAMID_SCALE, &meta->pmd.scale,
        sizeof(meta->pmd.scale)));

    return (status);
}

static vx_status ownInitMetaFormatWithImage(
    vx_meta_format meta, vx_image exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, vxQueryImage(exemplar, (vx_enum)VX_IMAGE_WIDTH, &meta->img.width,
        sizeof(meta->img.width)));
    tivxCheckStatus(&status, vxQueryImage(exemplar, (vx_enum)VX_IMAGE_HEIGHT, &meta->img.height,
        sizeof(meta->img.height)));
    tivxCheckStatus(&status, vxQueryImage(exemplar, (vx_enum)VX_IMAGE_FORMAT, &meta->img.format,
        sizeof(meta->img.format)));

    return (status);
}

static vx_status ownInitMetaFormatWithArray(
    vx_meta_format meta, vx_array exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, vxQueryArray(exemplar, (vx_enum)VX_ARRAY_ITEMTYPE, &meta->arr.item_type,
        sizeof(meta->arr.item_type)));
    tivxCheckStatus(&status, vxQueryArray(exemplar, (vx_enum)VX_ARRAY_CAPACITY, &meta->arr.capacity,
        sizeof(meta->arr.capacity)));

    return (status);
}

static vx_status ownInitMetaFormatWithScalar(
    vx_meta_format meta, vx_scalar exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, vxQueryScalar(exemplar, (vx_enum)VX_SCALAR_TYPE, &meta->sc.type,
        sizeof(meta->sc.type)));

    return (status);
}

static vx_status ownInitMetaFormatWithMatrix(
    vx_meta_format meta, vx_matrix exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, vxQueryMatrix(exemplar, (vx_enum)VX_MATRIX_TYPE, &meta->mat.type,
        sizeof(meta->mat.type)));
    tivxCheckStatus(&status, vxQueryMatrix(exemplar, (vx_enum)(vx_enum)VX_MATRIX_ROWS, &meta->mat.rows,
        sizeof(meta->mat.rows)));
    tivxCheckStatus(&status, vxQueryMatrix(exemplar, (vx_enum)VX_MATRIX_COLUMNS, &meta->mat.cols,
        sizeof(meta->mat.cols)));

    return status;
}

static vx_status ownInitMetaFormatWithDistribution(
    vx_meta_format meta, vx_distribution exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, vxQueryDistribution(exemplar, (vx_enum)VX_DISTRIBUTION_BINS, &meta->dist.bins,
        sizeof(meta->dist.bins)));
    tivxCheckStatus(&status, vxQueryDistribution(exemplar, (vx_enum)VX_DISTRIBUTION_OFFSET, &meta->dist.offset,
        sizeof(meta->dist.offset)));
    tivxCheckStatus(&status, vxQueryDistribution(exemplar, (vx_enum)VX_DISTRIBUTION_RANGE, &meta->dist.range,
        sizeof(meta->dist.range)));

    return status;
}

static vx_status ownInitMetaFormatWithRemap(
    vx_meta_format meta, vx_remap exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, vxQueryRemap(exemplar, (vx_enum)VX_REMAP_SOURCE_WIDTH, &meta->remap.src_width,
        sizeof(meta->remap.src_width)));
    tivxCheckStatus(&status, vxQueryRemap(exemplar, (vx_enum)VX_REMAP_SOURCE_HEIGHT, &meta->remap.src_height,
        sizeof(meta->remap.src_height)));
    tivxCheckStatus(&status, vxQueryRemap(exemplar, (vx_enum)VX_REMAP_DESTINATION_WIDTH, &meta->remap.dst_width,
        sizeof(meta->remap.dst_width)));
    tivxCheckStatus(&status, vxQueryRemap(exemplar, (vx_enum)VX_REMAP_DESTINATION_HEIGHT, &meta->remap.dst_height,
        sizeof(meta->remap.dst_height)));

    return status;
}

static vx_status ownInitMetaFormatWithLut(
    vx_meta_format meta, vx_lut exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, vxQueryLUT(exemplar, (vx_enum)VX_LUT_TYPE, &meta->lut.type,
        sizeof(meta->lut.type)));
    tivxCheckStatus(&status, vxQueryLUT(exemplar, (vx_enum)VX_LUT_COUNT, &meta->lut.count,
        sizeof(meta->lut.count)));

    return status;
}

static vx_status ownInitMetaFormatWithThreshold(
    vx_meta_format meta, vx_threshold exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, vxQueryThreshold(exemplar, (vx_enum)VX_THRESHOLD_TYPE, &meta->thres.type,
        sizeof(meta->thres.type)));

    return status;
}

static vx_status ownInitMetaFormatWithObjectArray(
    vx_meta_format meta, vx_object_array exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, vxQueryObjectArray(exemplar, (vx_enum)VX_OBJECT_ARRAY_ITEMTYPE, &meta->objarr.item_type,
        sizeof(meta->objarr.item_type)));
    tivxCheckStatus(&status, vxQueryObjectArray(exemplar, (vx_enum)VX_OBJECT_ARRAY_NUMITEMS, &meta->objarr.num_items,
        sizeof(meta->objarr.num_items)));

    return status;
}

static vx_status ownInitMetaFormatWithTensor(
    vx_meta_format meta, vx_tensor exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, vxQueryTensor(exemplar, (vx_enum)VX_TENSOR_NUMBER_OF_DIMS, &meta->tensor.number_of_dimensions,
        sizeof(meta->tensor.number_of_dimensions)));
    tivxCheckStatus(&status, vxQueryTensor(exemplar, (vx_enum)VX_TENSOR_DIMS, &meta->tensor.dimensions,
        sizeof(meta->tensor.dimensions)));
    tivxCheckStatus(&status, vxQueryTensor(exemplar, (vx_enum)VX_TENSOR_DATA_TYPE, &meta->tensor.data_type,
        sizeof(meta->tensor.data_type)));
    tivxCheckStatus(&status, vxQueryTensor(exemplar, (vx_enum)VX_TENSOR_FIXED_POINT_POSITION, &meta->tensor.fixed_point_position,
        sizeof(meta->tensor.fixed_point_position)));

    return (status);
}

static vx_status ownInitMetaFormatWithUserDataObject(
    vx_meta_format meta, vx_user_data_object exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, vxQueryUserDataObject(exemplar, (vx_enum)VX_USER_DATA_OBJECT_NAME, &meta->user_data_object.type_name,
        sizeof(meta->user_data_object.type_name)));
    tivxCheckStatus(&status, vxQueryUserDataObject(exemplar, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &meta->user_data_object.size,
        sizeof(meta->user_data_object.size)));

    return (status);
}

static vx_status ownInitMetaFormatWithRawImage(
    vx_meta_format meta, tivx_raw_image exemplar)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, tivxQueryRawImage(exemplar, (vx_enum)TIVX_RAW_IMAGE_WIDTH, &meta->raw_image.width,
        sizeof(meta->raw_image.width)));
    tivxCheckStatus(&status, tivxQueryRawImage(exemplar, (vx_enum)TIVX_RAW_IMAGE_HEIGHT, &meta->raw_image.height,
        sizeof(meta->raw_image.height)));
    tivxCheckStatus(&status, tivxQueryRawImage(exemplar, (vx_enum)TIVX_RAW_IMAGE_NUM_EXPOSURES, &meta->raw_image.num_exposures,
        sizeof(meta->raw_image.num_exposures)));
    tivxCheckStatus(&status, tivxQueryRawImage(exemplar, (vx_enum)TIVX_RAW_IMAGE_LINE_INTERLEAVED, &meta->raw_image.line_interleaved,
        sizeof(meta->raw_image.line_interleaved)));
    tivxCheckStatus(&status, tivxQueryRawImage(exemplar, (vx_enum)TIVX_RAW_IMAGE_FORMAT, &meta->raw_image.format,
        sizeof(meta->raw_image.format)));
    tivxCheckStatus(&status, tivxQueryRawImage(exemplar, (vx_enum)TIVX_RAW_IMAGE_META_HEIGHT_BEFORE, &meta->raw_image.meta_height_before,
        sizeof(meta->raw_image.meta_height_before)));
    tivxCheckStatus(&status, tivxQueryRawImage(exemplar, (vx_enum)TIVX_RAW_IMAGE_META_HEIGHT_AFTER, &meta->raw_image.meta_height_after,
        sizeof(meta->raw_image.meta_height_after)));

    return (status);
}

static vx_bool ownIsMetaFormatImageEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = (vx_bool)vx_false_e;

    if ( (ownIsValidSpecificReference((vx_reference)meta1, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) &&
         (ownIsValidSpecificReference((vx_reference)meta2, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) )
    {
        if ( (meta1->img.width  == meta2->img.width) &&
             (meta1->img.height == meta2->img.height) &&
             (meta1->img.format == meta2->img.format) )
        {
            is_equal = (vx_bool)vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "Image object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatArrayEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = (vx_bool)vx_false_e;

    if ( (ownIsValidSpecificReference((vx_reference)meta1, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) &&
         (ownIsValidSpecificReference((vx_reference)meta2, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) )
    {
        if ( (meta1->arr.item_type == meta2->arr.item_type) &&
             (meta1->arr.capacity  == meta2->arr.capacity) )
        {
            is_equal = (vx_bool)vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "Array object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatScalarEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = (vx_bool)vx_false_e;

    if ( (ownIsValidSpecificReference((vx_reference)meta1, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) &&
         (ownIsValidSpecificReference((vx_reference)meta2, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) )
    {
        if ( (meta1->sc.type == meta2->sc.type) )
        {
            is_equal = (vx_bool)vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "Scalar object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatPyramidEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = (vx_bool)vx_false_e;

    if ( (ownIsValidSpecificReference((vx_reference)meta1, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) &&
         (ownIsValidSpecificReference((vx_reference)meta2, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) )
    {
        if ( (meta1->pmd.width  == meta2->pmd.width) &&
             (meta1->pmd.height == meta2->pmd.height) &&
             (meta1->pmd.format == meta2->pmd.format) &&
             (meta1->pmd.levels == meta2->pmd.levels) &&
             (meta1->pmd.scale  == meta2->pmd.scale) )
        {
            is_equal = (vx_bool)vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "Pyramid object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatMatrixEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = (vx_bool)vx_false_e;

    if ( (ownIsValidSpecificReference((vx_reference)meta1, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) &&
         (ownIsValidSpecificReference((vx_reference)meta2, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) )
    {
        if ( (meta1->mat.type  == meta2->mat.type) &&
             (meta1->mat.rows  == meta2->mat.rows) &&
             (meta1->mat.cols  == meta2->mat.cols) )
        {
            is_equal = (vx_bool)vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "Matrix object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatDistributionEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = (vx_bool)vx_false_e;

    if ( (ownIsValidSpecificReference((vx_reference)meta1, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) &&
         (ownIsValidSpecificReference((vx_reference)meta2, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) )
    {
        if ( (meta1->dist.bins   == meta2->dist.bins) &&
             (meta1->dist.offset == meta2->dist.offset) &&
             (meta1->dist.range  == meta2->dist.range) )
        {
            is_equal = (vx_bool)vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "Distribution object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatRemapEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = (vx_bool)vx_false_e;

    if ( (ownIsValidSpecificReference((vx_reference)meta1, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) &&
         (ownIsValidSpecificReference((vx_reference)meta2, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) )
    {
        if ( (meta1->remap.src_width  == meta2->remap.src_width) &&
             (meta1->remap.src_height == meta2->remap.src_height) &&
             (meta1->remap.dst_width  == meta2->remap.dst_width) &&
             (meta1->remap.dst_height == meta2->remap.dst_height) )
        {
            is_equal = (vx_bool)vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "Remap object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatThresholdEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = (vx_bool)vx_false_e;

    if ( (ownIsValidSpecificReference((vx_reference)meta1, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) &&
         (ownIsValidSpecificReference((vx_reference)meta2, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) )
    {
        if ( (meta1->thres.type  == meta2->thres.type) )
        {
            is_equal = (vx_bool)vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "Threshold object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatObjectArrayEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = (vx_bool)vx_false_e;

    if ( (ownIsValidSpecificReference((vx_reference)meta1, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) &&
         (ownIsValidSpecificReference((vx_reference)meta2, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) )
    {
        if ( (meta1->objarr.item_type == meta2->objarr.item_type) &&
             (meta1->objarr.num_items == meta2->objarr.num_items) )
        {
            is_equal = (vx_bool)vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "Object Array object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatLutEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = (vx_bool)vx_false_e;

    if ( (ownIsValidSpecificReference((vx_reference)meta1, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) &&
         (ownIsValidSpecificReference((vx_reference)meta2, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) )
    {
        if ( (meta1->lut.type  == meta2->lut.type) &&
             (meta1->lut.count == meta2->lut.count) )
        {
            is_equal = (vx_bool)vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "LUT object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatTensorEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = (vx_bool)vx_false_e;
    vx_uint32 i;

    if ( (ownIsValidSpecificReference((vx_reference)meta1, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) &&
         (ownIsValidSpecificReference((vx_reference)meta2, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) )
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
                is_equal = (vx_bool)vx_true_e;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "Tensor object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatUserDataObjectEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = (vx_bool)vx_false_e;

    if ( (ownIsValidSpecificReference((vx_reference)meta1, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) &&
         (ownIsValidSpecificReference((vx_reference)meta2, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) )
    {
        if ( (meta1->user_data_object.size == meta2->user_data_object.size) &&
             ( 0 == strncmp(meta1->user_data_object.type_name, meta2->user_data_object.type_name, VX_MAX_REFERENCE_NAME) ) )
        {
            is_equal = (vx_bool)vx_true_e;
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "User data object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

static vx_bool ownIsMetaFormatRawImageEqual(
    vx_meta_format meta1, vx_meta_format meta2)
{
    vx_bool is_equal = (vx_bool)vx_false_e;
    vx_uint32 i;

    if ( (ownIsValidSpecificReference((vx_reference)meta1, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) &&
         (ownIsValidSpecificReference((vx_reference)meta2, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) )
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
                is_equal = (vx_bool)vx_true_e;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO, "Raw Image object meta data are not equivalent!\n");
        }
    }

    return is_equal;
}

vx_bool ownIsMetaFormatEqual(
    vx_meta_format meta1, vx_meta_format meta2, vx_enum type)
{
    vx_bool is_equal = (vx_bool)vx_false_e;

    if ( (ownIsValidSpecificReference((vx_reference)meta1, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) &&
         (ownIsValidSpecificReference((vx_reference)meta2, (vx_enum)VX_TYPE_META_FORMAT) == (vx_bool)vx_true_e) )
    {
        switch (type)
        {
            case (vx_enum)VX_TYPE_IMAGE:
                is_equal = ownIsMetaFormatImageEqual(meta1, meta2);
                break;
            case (vx_enum)VX_TYPE_ARRAY:
                is_equal = ownIsMetaFormatArrayEqual(meta1, meta2);
                break;
            case (vx_enum)VX_TYPE_SCALAR:
                is_equal = ownIsMetaFormatScalarEqual(meta1, meta2);
                break;
            case (vx_enum)VX_TYPE_PYRAMID:
                is_equal = ownIsMetaFormatPyramidEqual(meta1, meta2);
                break;
            case (vx_enum)VX_TYPE_MATRIX:
                is_equal = ownIsMetaFormatMatrixEqual(meta1, meta2);
                break;
            case (vx_enum)VX_TYPE_DISTRIBUTION:
                is_equal = ownIsMetaFormatDistributionEqual(meta1, meta2);
                break;
            case (vx_enum)VX_TYPE_THRESHOLD:
                is_equal = ownIsMetaFormatThresholdEqual(meta1, meta2);
                break;
            case (vx_enum)VX_TYPE_REMAP:
                is_equal = ownIsMetaFormatRemapEqual(meta1, meta2);
                break;
            case (vx_enum)VX_TYPE_LUT:
                is_equal = ownIsMetaFormatLutEqual(meta1, meta2);
                break;
            case (vx_enum)VX_TYPE_OBJECT_ARRAY:
                is_equal = ownIsMetaFormatObjectArrayEqual(meta1, meta2);
                break;
            case (vx_enum)VX_TYPE_TENSOR:
                is_equal = ownIsMetaFormatTensorEqual(meta1, meta2);
                break;
            case VX_TYPE_USER_DATA_OBJECT:
                is_equal = ownIsMetaFormatUserDataObjectEqual(meta1, meta2);
                break;
            case TIVX_TYPE_RAW_IMAGE:
                is_equal = ownIsMetaFormatRawImageEqual(meta1, meta2);
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "invalid attribute\n");
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Meta format is invalid!\n");
    }

    return is_equal;
}

