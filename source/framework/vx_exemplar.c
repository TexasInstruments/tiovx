/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
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
static vx_reference ownCreateObjectArrayFromExemplar(
    vx_context context, vx_object_array exemplar);
static vx_reference ownCreateTensorFromExemplar(
    vx_context context, vx_tensor exemplar);
static vx_reference ownCreateUserDataObjectFromExemplar(
    vx_context context, vx_user_data_object exemplar);
static vx_reference ownCreateRawImageFromExemplar(
    vx_context context, tivx_raw_image exemplar);


vx_reference tivxCreateReferenceFromExemplar(
    vx_context context, vx_reference exemplar)
{
    vx_reference ref = NULL;

    switch (exemplar->type)
    {
        case (vx_enum)VX_TYPE_LUT:
            /*status set to NULL due to preceding type check*/
            ref = ownCreateLutFromExemplar(
                context, (vxCastRefAsLUT(exemplar, NULL)));
            break;
        case (vx_enum)VX_TYPE_REMAP:
            /*status set to NULL due to preceding type check*/
            ref = ownCreateRemapFromExemplar(
                context, vxCastRefAsRemap(exemplar, NULL));
            break;
        case (vx_enum)VX_TYPE_MATRIX:
            /*status set to NULL due to preceding type check*/
            ref = ownCreateMatrixFromExemplar(
                context, vxCastRefAsMatrix(exemplar, NULL));
            break;
        case (vx_enum)VX_TYPE_PYRAMID:
            /*status set to NULL due to preceding type check*/
            ref = ownCreatePyramidFromExemplar(
                context, vxCastRefAsPyramid(exemplar, NULL));
            break;
        case (vx_enum)VX_TYPE_IMAGE:
            /*status set to NULL due to preceding type check*/
            ref = ownCreateImageFromExemplar(
                context, vxCastRefAsImage(exemplar, NULL));
            break;
        case (vx_enum)VX_TYPE_ARRAY:
            /*status set to NULL due to preceding type check*/
            ref = ownCreateArrayFromExemplar(
                context, vxCastRefAsArray(exemplar, NULL));
            break;
        case (vx_enum)VX_TYPE_SCALAR:
            /*status set to NULL due to preceding type check*/
            ref = ownCreateScalarFromExemplar(
                context, vxCastRefAsScalar(exemplar, NULL));
            break;
        case (vx_enum)VX_TYPE_DISTRIBUTION:
            /*status set to NULL due to preceding type check*/
            ref = ownCreateDistributionFromExemplar(
                context, vxCastRefAsDistribution(exemplar, NULL));
            break;
        case (vx_enum)VX_TYPE_THRESHOLD:
            /*status set to NULL due to preceding type check*/
            ref = ownCreateThresholdFromExemplar(
                context, vxCastRefAsThreshold(exemplar, NULL));
            break;
        case (vx_enum)VX_TYPE_CONVOLUTION:
            /*status set to NULL due to preceding type check*/
            ref = ownCreateConvolutionFromExemplar(
                context, vxCastRefAsConvolution(exemplar, NULL));
            break;
        case (vx_enum)VX_TYPE_OBJECT_ARRAY:
            /*status set to NULL due to preceding type check*/
            ref = ownCreateObjectArrayFromExemplar(
                context, vxCastRefAsObjectArray(exemplar, NULL));
            break;
        case (vx_enum)VX_TYPE_TENSOR:
            /*status set to NULL due to preceding type check*/
            ref = ownCreateTensorFromExemplar(
                context, vxCastRefAsTensor(exemplar, NULL));
            break;
        case VX_TYPE_USER_DATA_OBJECT:
            /*status set to NULL due to preceding type check*/
            ref = ownCreateUserDataObjectFromExemplar(
                context, vxCastRefAsUserDataObject(exemplar, NULL));
            break;
        case TIVX_TYPE_RAW_IMAGE:
            ref = ownCreateRawImageFromExemplar(
                context, (tivx_raw_image)exemplar);
            break;
        default:
            break;
    }
    if (NULL != exemplar->supplementary_data)
    {
        (void)vxSetSupplementaryUserDataObject(ref, exemplar->supplementary_data);
    }
    return (ref);
}

static vx_reference ownCreateLutFromExemplar(
    vx_context context, vx_lut exemplar)
{
    vx_enum data_type;
    vx_size count;
    vx_lut lut = NULL;

    (void)vxQueryLUT(exemplar, (vx_enum)VX_LUT_TYPE, &data_type,
        sizeof(data_type));
    (void)vxQueryLUT(exemplar, (vx_enum)VX_LUT_COUNT, &count,
        sizeof(count));

    lut = vxCreateLUT(context, data_type, count);

    return vxCastRefFromLUT(lut);
}

static vx_reference ownCreateRemapFromExemplar(
    vx_context context, vx_remap exemplar)
{
    vx_uint32 src_width, src_height, dst_width, dst_height;
    vx_remap rem = NULL;

    (void)vxQueryRemap(exemplar, (vx_enum)VX_REMAP_SOURCE_WIDTH, &src_width,
        sizeof(src_width));
    (void)vxQueryRemap(exemplar, (vx_enum)VX_REMAP_SOURCE_HEIGHT, &src_height,
        sizeof(src_height));
    (void)vxQueryRemap(exemplar, (vx_enum)VX_REMAP_DESTINATION_WIDTH, &dst_width,
        sizeof(dst_width));
    (void)vxQueryRemap(exemplar, (vx_enum)VX_REMAP_DESTINATION_HEIGHT, &dst_height,
        sizeof(dst_height));

    rem = vxCreateRemap(context, src_width, src_height, dst_width,
        dst_height);

    return vxCastRefFromRemap(rem);
}

static vx_reference ownCreateMatrixFromExemplar(
    vx_context context, vx_matrix exemplar)
{
    vx_size rows, columns;
    vx_enum type;
    vx_matrix mat = NULL;

    (void)vxQueryMatrix(exemplar, (vx_enum)VX_MATRIX_TYPE, &type, sizeof(type));
    (void)vxQueryMatrix(exemplar, (vx_enum)(vx_enum)VX_MATRIX_ROWS, &rows, sizeof(rows));
    (void)vxQueryMatrix(exemplar, (vx_enum)VX_MATRIX_COLUMNS, &columns,
        sizeof(columns));

    mat = vxCreateMatrix(context, type, columns, rows);

    return vxCastRefFromMatrix(mat);
}

static vx_reference ownCreatePyramidFromExemplar(
    vx_context context, vx_pyramid exemplar)
{
    vx_size levels;
    vx_float32 scale;
    vx_uint32 width, height;
    vx_df_image format;
    vx_pyramid pmd = NULL;

    (void)vxQueryPyramid(exemplar, (vx_enum)VX_PYRAMID_LEVELS, &levels, sizeof(levels));
    (void)vxQueryPyramid(exemplar, (vx_enum)VX_PYRAMID_SCALE, &scale, sizeof(scale));
    (void)vxQueryPyramid(exemplar, (vx_enum)VX_PYRAMID_WIDTH, &width, sizeof(width));
    (void)vxQueryPyramid(exemplar, (vx_enum)VX_PYRAMID_HEIGHT, &height, sizeof(height));
    (void)vxQueryPyramid(exemplar, (vx_enum)VX_PYRAMID_FORMAT, &format, sizeof(format));

    pmd = vxCreatePyramid(context, levels, scale, width, height,
            format);

    if ((vx_status)VX_SUCCESS == vxGetStatus((vx_reference)pmd) && (NULL != exemplar->base.supplementary_data))
    {
        vx_uint32 i;
        vx_status status = (vx_status)VX_SUCCESS;
        for (i = 0U; (i < levels) && ((vx_status)VX_SUCCESS == status); ++i)
        {
            status = vxSetSupplementaryUserDataObject(&pmd->img[i]->base, exemplar->base.supplementary_data);
        }
    }

    return vxCastRefFromPyramid(pmd);
}

static vx_reference ownCreateImageFromExemplar(
    vx_context context, vx_image exemplar)
{
    vx_uint32 width, height, stride_y_alignment;
    vx_df_image format;
    vx_image img = NULL;

    (void)vxQueryImage(exemplar, (vx_enum)VX_IMAGE_WIDTH, &width, sizeof(width));
    (void)vxQueryImage(exemplar, (vx_enum)VX_IMAGE_HEIGHT, &height, sizeof(height));
    (void)vxQueryImage(exemplar, (vx_enum)VX_IMAGE_FORMAT, &format, sizeof(format));
    (void)vxQueryImage(exemplar, (vx_enum)TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &stride_y_alignment, sizeof(stride_y_alignment));

    img = vxCreateImage(context, width, height, format);

    if ((vx_status)VX_SUCCESS == vxGetStatus(vxCastRefFromImage(img)))
    {
        (void)vxSetImageAttribute(img, (vx_enum)TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &stride_y_alignment, sizeof(stride_y_alignment));
    }

    return vxCastRefFromImage(img);
}

static vx_reference ownCreateArrayFromExemplar(
    vx_context context, vx_array exemplar)
{
    vx_enum type;
    vx_size capacity;
    vx_array arr = NULL;

    (void)vxQueryArray(exemplar, (vx_enum)VX_ARRAY_ITEMTYPE, &type, sizeof(type));
    (void)vxQueryArray(exemplar, (vx_enum)VX_ARRAY_CAPACITY, &capacity, sizeof(capacity));

    arr = vxCreateArray(context, type, capacity);

    return vxCastRefFromArray(arr);
}

static vx_reference ownCreateScalarFromExemplar(
    vx_context context, vx_scalar exemplar)
{
    vx_enum type;
    vx_scalar sc = NULL;

    (void)vxQueryScalar(exemplar, (vx_enum)VX_SCALAR_TYPE, &type, sizeof(type));

    sc = vxCreateScalar(context, type, NULL);

    return vxCastRefFromScalar(sc);
}

static vx_reference ownCreateDistributionFromExemplar(
    vx_context context, vx_distribution exemplar)
{
    vx_size num_bins;
    vx_int32 offset;
    vx_uint32 range;
    vx_distribution dist = NULL;

    (void)vxQueryDistribution(exemplar, (vx_enum)VX_DISTRIBUTION_OFFSET, &offset, sizeof(offset));
    (void)vxQueryDistribution(exemplar, (vx_enum)VX_DISTRIBUTION_RANGE, &range, sizeof(range));
    (void)vxQueryDistribution(exemplar, (vx_enum)VX_DISTRIBUTION_BINS, &num_bins, sizeof(num_bins));

    dist = vxCreateDistribution(context, num_bins, offset, range);

    return vxCastRefFromDistribution(dist);
}

static vx_reference ownCreateThresholdFromExemplar(
    vx_context context, vx_threshold exemplar)
{
    vx_enum thr_type;
    vx_enum data_type;
    vx_threshold thr = NULL;

    (void)vxQueryThreshold(exemplar, (vx_enum)VX_THRESHOLD_DATA_TYPE, &data_type,
        sizeof(data_type));
    (void)vxQueryThreshold(exemplar, (vx_enum)VX_THRESHOLD_TYPE, &thr_type,
        sizeof(thr_type));

    thr = vxCreateThreshold(context, thr_type, data_type);

    return vxCastRefFromThreshold(thr);
}

static vx_reference ownCreateConvolutionFromExemplar(
    vx_context context, vx_convolution exemplar)
{
    vx_size rows, columns;
    vx_convolution conv = NULL;

    (void)vxQueryConvolution(exemplar, (vx_enum)VX_CONVOLUTION_ROWS, &rows, sizeof(rows));
    (void)vxQueryConvolution(exemplar, (vx_enum)VX_CONVOLUTION_COLUMNS, &columns,
        sizeof(columns));

    conv = vxCreateConvolution(context, columns, rows);

    return vxCastRefFromConvolution(conv);
}

static vx_reference ownCreateObjectArrayFromExemplar(
    vx_context context, vx_object_array exemplar)
{
    vx_size count;
    vx_reference objarr_item_exemplar, objarr_item;
    vx_object_array objarr = NULL;

    (void)vxQueryObjectArray(exemplar, (vx_enum)VX_OBJECT_ARRAY_NUMITEMS, &count, sizeof(count));

    objarr_item_exemplar = vxGetObjectArrayItem(exemplar, 0);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1721- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_EXEMPLER_UM011 */
    if(objarr_item_exemplar==NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid object array reference\n");
    }
    else
#endif
    {
        vx_uint32 i;
        vx_status status = (vx_status)VX_SUCCESS;
        objarr = vxCreateObjectArray(context, objarr_item_exemplar, count);
        (void)vxReleaseReference(&objarr_item_exemplar);
        if (NULL != exemplar->base.supplementary_data)
        {
            for (i = 0U; (i < count) && ((vx_status)VX_SUCCESS == status); ++i)
            {
                objarr_item = vxGetObjectArrayItem(objarr, i);
                status = vxSetSupplementaryUserDataObject(objarr_item, exemplar->base.supplementary_data);
                (void)vxReleaseReference(&objarr_item);
            }
        }
    }

    return vxCastRefFromObjectArray(objarr);
}

static vx_reference ownCreateTensorFromExemplar(
    vx_context context, vx_tensor exemplar)
{
    vx_size num_dims;
    vx_size dims[TIVX_CONTEXT_MAX_TENSOR_DIMS];
    vx_enum type;
    vx_int8 fixed;
    vx_tensor tensor = NULL;

    (void)vxQueryTensor(exemplar, (vx_enum)VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(num_dims));
    (void)vxQueryTensor(exemplar, (vx_enum)VX_TENSOR_DIMS, &dims, sizeof(dims));
    (void)vxQueryTensor(exemplar, (vx_enum)VX_TENSOR_DATA_TYPE, &type, sizeof(type));
    (void)vxQueryTensor(exemplar, (vx_enum)VX_TENSOR_FIXED_POINT_POSITION, &fixed, sizeof(fixed));

    tensor = vxCreateTensor(context, num_dims, dims, type, fixed);

    return vxCastRefFromTensor(tensor);
}

static vx_reference ownCreateUserDataObjectFromExemplar(
    vx_context context, vx_user_data_object exemplar)
{
    vx_char type_name[VX_MAX_REFERENCE_NAME];
    vx_size size;
    vx_user_data_object user_data_object = NULL;

    (void)vxQueryUserDataObject(exemplar, (vx_enum)VX_USER_DATA_OBJECT_NAME, &type_name, sizeof(type_name));
    (void)vxQueryUserDataObject(exemplar, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &size, sizeof(size));

    user_data_object = vxCreateUserDataObject(context, type_name, size, NULL);

    return vxCastRefFromUserDataObject(user_data_object);
}

static vx_reference ownCreateRawImageFromExemplar(
    vx_context context, tivx_raw_image exemplar)
{
    tivx_raw_image img = NULL;
    tivx_raw_image_create_params_t params;

    (void)tivxQueryRawImage(exemplar, (vx_enum)TIVX_RAW_IMAGE_WIDTH, &params.width, sizeof(params.width));
    (void)tivxQueryRawImage(exemplar, (vx_enum)TIVX_RAW_IMAGE_HEIGHT, &params.height, sizeof(params.height));
    (void)tivxQueryRawImage(exemplar, (vx_enum)TIVX_RAW_IMAGE_NUM_EXPOSURES, &params.num_exposures, sizeof(params.num_exposures));
    (void)tivxQueryRawImage(exemplar, (vx_enum)TIVX_RAW_IMAGE_LINE_INTERLEAVED, &params.line_interleaved, sizeof(params.line_interleaved));
    (void)tivxQueryRawImage(exemplar, (vx_enum)TIVX_RAW_IMAGE_FORMAT, &params.format, sizeof(params.format));
    (void)tivxQueryRawImage(exemplar, (vx_enum)TIVX_RAW_IMAGE_META_HEIGHT_BEFORE, &params.meta_height_before, sizeof(params.meta_height_before));
    (void)tivxQueryRawImage(exemplar, (vx_enum)TIVX_RAW_IMAGE_META_HEIGHT_AFTER, &params.meta_height_after, sizeof(params.meta_height_after));

    img = tivxCreateRawImage(context, &params);

    return (vx_reference)img;
}

