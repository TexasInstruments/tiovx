/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <vx_internal.h>


static vx_reference ownCreateImageFromExemplar(
    vx_context context, vx_image exemplar);
static vx_reference ownCreateArrayFromExemplar(
    vx_context context, vx_array exemplar);
static vx_reference ownCreateScalarFromExemplar(
    vx_context context, vx_scalar exemplar);
static vx_reference ownCreateDistributionFromExemplar(
    vx_context context, vx_distribution exemplar);
static vx_reference ownCreateThresholdFromExemplar(
    vx_context context, vx_threshold exemplar);
static vx_reference ownCreatePyramidFromExemplar(
    vx_context context, vx_pyramid exemplar);
static vx_reference ownCreateMatrixFromExemplar(
    vx_context context, vx_matrix exemplar);
static vx_reference ownCreateConvolutionFromExemplar(
    vx_context context, vx_convolution exemplar);
static vx_reference ownCreateRemapFromExemplar(
    vx_context context, vx_remap exemplar);
static vx_reference ownCreateLutFromExemplar(
    vx_context context, vx_lut exemplar);


vx_reference ownCreateReferenceFromExemplar(
    vx_context context, vx_reference exemplar)
{
    vx_reference ref = NULL;

    switch (exemplar->type)
    {
        case VX_TYPE_LUT:
            ref = ownCreateLutFromExemplar(
                context, (vx_lut)exemplar);
            break;
        case VX_TYPE_REMAP:
            ref = ownCreateRemapFromExemplar(
                context, (vx_remap)exemplar);
            break;
        case VX_TYPE_MATRIX:
            ref = ownCreateMatrixFromExemplar(
                context, (vx_matrix)exemplar);
            break;
        case VX_TYPE_PYRAMID:
            ref = ownCreatePyramidFromExemplar(
                context, (vx_pyramid)exemplar);
            break;
        case VX_TYPE_IMAGE:
            ref = ownCreateImageFromExemplar(
                context, (vx_image)exemplar);
            break;
        case VX_TYPE_ARRAY:
            ref = ownCreateArrayFromExemplar(
                context, (vx_array)exemplar);
            break;
        case VX_TYPE_SCALAR:
            ref = ownCreateScalarFromExemplar(
                context, (vx_scalar)exemplar);
            break;
        case VX_TYPE_DISTRIBUTION:
            ref = ownCreateDistributionFromExemplar(
                context, (vx_distribution)exemplar);
            break;
        case VX_TYPE_THRESHOLD:
            ref = ownCreateThresholdFromExemplar(
                context, (vx_threshold)exemplar);
            break;
        case VX_TYPE_CONVOLUTION:
            ref = ownCreateConvolutionFromExemplar(
                context, (vx_convolution)exemplar);
            break;
        default:
            break;
    }

    return (ref);
}

static vx_reference ownCreateLutFromExemplar(
    vx_context context, vx_lut exemplar)
{
    vx_status status = VX_SUCCESS;
    vx_enum data_type;
    vx_size count;
    vx_lut lut = NULL;

    status |= vxQueryLUT(exemplar, VX_LUT_TYPE, &data_type,
        sizeof(data_type));
    status |= vxQueryLUT(exemplar, VX_LUT_COUNT, &count,
        sizeof(count));

    if (VX_SUCCESS == status)
    {
        lut = vxCreateLUT(context, data_type, count);
    }

    return (vx_reference)lut;
}

static vx_reference ownCreateRemapFromExemplar(
    vx_context context, vx_remap exemplar)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 src_width, src_height, dst_width, dst_height;
    vx_remap rem = NULL;

    status |= vxQueryRemap(exemplar, VX_REMAP_SOURCE_WIDTH, &src_width,
        sizeof(src_width));
    status |= vxQueryRemap(exemplar, VX_REMAP_SOURCE_HEIGHT, &src_height,
        sizeof(src_height));
    status |= vxQueryRemap(exemplar, VX_REMAP_DESTINATION_WIDTH, &dst_width,
        sizeof(dst_width));
    status |= vxQueryRemap(exemplar, VX_REMAP_DESTINATION_HEIGHT, &dst_height,
        sizeof(dst_height));

    if (VX_SUCCESS == status)
    {
        rem = vxCreateRemap(context, src_width, src_height, dst_width,
            dst_height);
    }

    return (vx_reference)rem;
}

static vx_reference ownCreateMatrixFromExemplar(
    vx_context context, vx_matrix exemplar)
{
    vx_status status = VX_SUCCESS;
    vx_size rows, columns;
    vx_enum type;
    vx_matrix mat = NULL;

    status |= vxQueryMatrix(exemplar, VX_MATRIX_TYPE, &type, sizeof(type));
    status |= vxQueryMatrix(exemplar, VX_MATRIX_ROWS, &rows, sizeof(rows));
    status |= vxQueryMatrix(exemplar, VX_MATRIX_COLUMNS, &columns,
        sizeof(columns));

    if (VX_SUCCESS == status)
    {
        mat = vxCreateMatrix(context, type, columns, rows);
    }

    return (vx_reference)mat;
}

static vx_reference ownCreatePyramidFromExemplar(
    vx_context context, vx_pyramid exemplar)
{
    vx_status status = VX_SUCCESS;
    vx_size levels;
    vx_float32 scale;
    vx_uint32 width, height;
    vx_df_image format;
    vx_pyramid pmd = NULL;

    status |= vxQueryPyramid(exemplar, VX_PYRAMID_LEVELS, &levels, sizeof(levels));
    status |= vxQueryPyramid(exemplar, VX_PYRAMID_SCALE, &scale, sizeof(scale));
    status |= vxQueryPyramid(exemplar, VX_PYRAMID_WIDTH, &width, sizeof(width));
    status |= vxQueryPyramid(exemplar, VX_PYRAMID_HEIGHT, &height, sizeof(height));
    status |= vxQueryPyramid(exemplar, VX_PYRAMID_FORMAT, &format, sizeof(format));

    if (VX_SUCCESS == status)
    {
        pmd = vxCreatePyramid(context, levels, scale, width, height,
            format);
    }

    return (vx_reference)pmd;
}

static vx_reference ownCreateImageFromExemplar(
    vx_context context, vx_image exemplar)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width, height;
    vx_df_image format;
    vx_image img = NULL;

    status |= vxQueryImage(exemplar, VX_IMAGE_WIDTH, &width, sizeof(width));
    status |= vxQueryImage(exemplar, VX_IMAGE_HEIGHT, &height, sizeof(height));
    status |= vxQueryImage(exemplar, VX_IMAGE_FORMAT, &format, sizeof(format));

    if (VX_SUCCESS == status)
    {
        img = vxCreateImage(context, width, height, format);
    }

    return (vx_reference)img;
}

static vx_reference ownCreateArrayFromExemplar(
    vx_context context, vx_array exemplar)
{
    vx_status status = VX_SUCCESS;
    vx_enum type;
    vx_size capacity;
    vx_array arr = NULL;

    status |= vxQueryArray(exemplar, VX_ARRAY_ITEMTYPE, &type, sizeof(type));
    status |= vxQueryArray(exemplar, VX_ARRAY_CAPACITY, &capacity, sizeof(capacity));

    if (VX_SUCCESS == status)
    {
        arr = vxCreateArray(context, type, capacity);
    }

    return (vx_reference)arr;
}

static vx_reference ownCreateScalarFromExemplar(
    vx_context context, vx_scalar exemplar)
{
    vx_status status = VX_SUCCESS;
    vx_enum type;
    vx_scalar sc = NULL;

    status |= vxQueryScalar(exemplar, VX_SCALAR_TYPE, &type, sizeof(type));

    if (VX_SUCCESS == status)
    {
        sc = vxCreateScalar(context, type, NULL);
    }

    return (vx_reference)sc;
}

static vx_reference ownCreateDistributionFromExemplar(
    vx_context context, vx_distribution exemplar)
{
    vx_status status = VX_SUCCESS;
    vx_size num_bins;
    vx_int32 offset;
    vx_uint32 range;
    vx_distribution dist = NULL;

    status |= vxQueryDistribution(exemplar, VX_DISTRIBUTION_OFFSET, &offset, sizeof(offset));
    status |= vxQueryDistribution(exemplar, VX_DISTRIBUTION_RANGE, &range, sizeof(range));
    status |= vxQueryDistribution(exemplar, VX_DISTRIBUTION_BINS, &num_bins, sizeof(num_bins));

    if (VX_SUCCESS == status)
    {
        dist = vxCreateDistribution(context, num_bins, offset, range);
    }

    return (vx_reference)dist;
}

static vx_reference ownCreateThresholdFromExemplar(
    vx_context context, vx_threshold exemplar)
{
    vx_status status = VX_SUCCESS;
    vx_enum thr_type;
    vx_enum data_type;
    vx_threshold thr = NULL;

    status |= vxQueryThreshold(exemplar, VX_THRESHOLD_DATA_TYPE, &data_type,
        sizeof(data_type));
    status |= vxQueryThreshold(exemplar, VX_THRESHOLD_TYPE, &thr_type,
        sizeof(thr_type));

    if (VX_SUCCESS == status)
    {
        thr = vxCreateThreshold(context, thr_type, data_type);
    }

    return (vx_reference)thr;
}

static vx_reference ownCreateConvolutionFromExemplar(
    vx_context context, vx_convolution exemplar)
{
    vx_status status = VX_SUCCESS;
    vx_size rows, columns;
    vx_convolution conv = NULL;

    status |= vxQueryConvolution(exemplar, VX_CONVOLUTION_ROWS, &rows, sizeof(rows));
    status |= vxQueryConvolution(exemplar, VX_CONVOLUTION_COLUMNS, &columns,
        sizeof(columns));

    if (VX_SUCCESS == status)
    {
        conv = vxCreateConvolution(context, columns, rows);
    }

    return (vx_reference)conv;
}
