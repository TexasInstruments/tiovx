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
