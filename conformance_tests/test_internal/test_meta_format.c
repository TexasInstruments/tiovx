/*

 * Copyright (c) 2012-2017 The Khronos Group Inc.
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
/*
 * Copyright (c) 2024 Texas Instruments Incorporated
 */

#include "test_tiovx.h"

#include <TI/tivx_test_kernels.h>
#include <TI/tivx_config.h>
#include <TI/tivx_capture.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include <string.h>
#include <math.h>
#include "test_engine/test.h"

#include <vx_internal.h>
#include "shared_functions.h"

TESTCASE(tivxInternalMetaFormat, CT_VXContext, ct_setup_vx_context, 0)

typedef struct
{
    const char *name;
    vx_enum type;
} Arg;

TEST_WITH_ARG(tivxInternalMetaFormat, negativeInternalTestSetMetaFormatFromReference, Arg,
              ARG_ENUM(VX_TYPE_IMAGE),
              ARG_ENUM(VX_TYPE_ARRAY),
            //   ARG_ENUM(VX_TYPE_SCALAR), /* TODO: Commented due to a seg fault */
              ARG_ENUM(VX_TYPE_PYRAMID),
              ARG_ENUM(VX_TYPE_MATRIX),
              ARG_ENUM(VX_TYPE_DISTRIBUTION),
              ARG_ENUM(VX_TYPE_CONVOLUTION),
              ARG_ENUM(VX_TYPE_THRESHOLD),
              ARG_ENUM(VX_TYPE_REMAP),
              ARG_ENUM(VX_TYPE_LUT),
              ARG_ENUM(VX_TYPE_OBJECT_ARRAY),
              ARG_ENUM(VX_TYPE_TENSOR),
              ARG_ENUM(VX_TYPE_USER_DATA_OBJECT),
              ARG_ENUM(TIVX_TYPE_RAW_IMAGE),
              ARG_ENUM(VX_TYPE_INVALID))
{
    vx_context context = context_->vx_context_;

    vx_enum type = (enum vx_type_e)arg_->type;
    vx_reference exemplar = NULL;
    vx_meta_format meta = ownCreateMetaFormat(context);
    tivx_obj_desc_t * orig_obj_desc = NULL;
    vx_size capacity = 10;
    vx_uint8  scalar_val = 0;
    vx_size rows = 3, cols = 3;
    vx_uint32 src_width = 16;
    vx_uint32 src_height = 32;
    vx_uint32 dst_width = 128;
    vx_uint32 dst_height = 64;

    vx_pyramid pyr_exemplar;
    vx_uint32 pyr_width = 16;
    vx_uint32 pyr_height = 32;

    vx_size *dims;
    static const vx_char user_data_object_name[] = "wb_t";
    typedef struct
    {
        vx_int32 mode;
        vx_int32 gain[4];
        vx_int32 offset[4];
    } wb_t;

    tivx_raw_image_create_params_t params;
    params.width = 128;
    params.height = 128;
    params.num_exposures = 3;
    params.line_interleaved = vx_false_e;
    params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    params.format[0].msb = 12;
    params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    params.format[1].msb = 7;
    params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    params.format[2].msb = 11;
    params.meta_height_before = 5;
    params.meta_height_after = 0;

    switch ((uint32_t)type)
    {
        case VX_TYPE_IMAGE:
            ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            orig_obj_desc = ((vx_image)exemplar)->base.obj_desc;
            ((vx_image)exemplar)->base.obj_desc = NULL;

            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatFromReference(meta, exemplar));
            ((vx_image)exemplar)->base.obj_desc = orig_obj_desc;

            VX_CALL(vxReleaseImage((vx_image*)&exemplar));
        break;
        case VX_TYPE_ARRAY:
            ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateArray(context, VX_TYPE_ENUM, capacity), VX_TYPE_ARRAY);
            orig_obj_desc = ((vx_array)exemplar)->base.obj_desc;
            ((vx_array)exemplar)->base.obj_desc = NULL;

            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatFromReference(meta, exemplar));
            ((vx_array)exemplar)->base.obj_desc = orig_obj_desc;

            VX_CALL(vxReleaseArray((vx_array*)&exemplar));
        break;
        // TODO: Segfault when this is enabled from Query scalar
        case VX_TYPE_SCALAR:
            ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
            orig_obj_desc = ((vx_scalar)exemplar)->base.obj_desc;
            ((vx_scalar)exemplar)->base.obj_desc = NULL;

            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatFromReference(meta, exemplar));
            ((vx_scalar)exemplar)->base.obj_desc = orig_obj_desc;

            VX_CALL(vxReleaseScalar((vx_scalar*)&exemplar));
        break;
        case VX_TYPE_PYRAMID:
            ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
            orig_obj_desc = ((vx_pyramid)exemplar)->base.obj_desc;
            ((vx_pyramid)exemplar)->base.obj_desc = NULL;

            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatFromReference(meta, exemplar));
            ((vx_pyramid)exemplar)->base.obj_desc = orig_obj_desc;

            VX_CALL(vxReleasePyramid((vx_pyramid*)&exemplar));
        break;
        case VX_TYPE_MATRIX:
            ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateMatrix(context, VX_TYPE_UINT8, 4, 4), VX_TYPE_MATRIX);
            orig_obj_desc = ((vx_matrix)exemplar)->base.obj_desc;
            ((vx_matrix)exemplar)->base.obj_desc = NULL;

            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatFromReference(meta, exemplar));
            ((vx_matrix)exemplar)->base.obj_desc = orig_obj_desc;

            VX_CALL(vxReleaseMatrix((vx_matrix*)&exemplar));
        break;
        case VX_TYPE_DISTRIBUTION:
            ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateDistribution(context, 32, 0, 255), VX_TYPE_DISTRIBUTION);
            orig_obj_desc = ((vx_distribution)exemplar)->base.obj_desc;
            ((vx_distribution)exemplar)->base.obj_desc = NULL;

            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatFromReference(meta, exemplar));
            ((vx_distribution)exemplar)->base.obj_desc = orig_obj_desc;

            VX_CALL(vxReleaseDistribution((vx_distribution*)&exemplar));
        break;
        case VX_TYPE_CONVOLUTION:
            ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateConvolution(context, cols, rows), VX_TYPE_CONVOLUTION);
            orig_obj_desc = ((vx_convolution)exemplar)->base.obj_desc;
            ((vx_convolution)exemplar)->base.obj_desc = NULL;

            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatFromReference(meta, exemplar));
            ((vx_convolution)exemplar)->base.obj_desc = orig_obj_desc;

            VX_CALL(vxReleaseConvolution((vx_convolution*)&exemplar));
        break;
        case VX_TYPE_THRESHOLD:
            ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8), VX_TYPE_THRESHOLD);
            orig_obj_desc = ((vx_threshold)exemplar)->base.obj_desc;
            ((vx_threshold)exemplar)->base.obj_desc = NULL;

            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatFromReference(meta, exemplar));
            ((vx_threshold)exemplar)->base.obj_desc = orig_obj_desc;

            VX_CALL(vxReleaseThreshold((vx_threshold*)&exemplar));
        break;
        case VX_TYPE_REMAP:

            ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateRemap(context, src_width, src_height, dst_width, dst_height), VX_TYPE_REMAP);
            orig_obj_desc = ((vx_remap)exemplar)->base.obj_desc;
            ((vx_remap)exemplar)->base.obj_desc = NULL;

            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatFromReference(meta, exemplar));
            ((vx_remap)exemplar)->base.obj_desc = orig_obj_desc;

            VX_CALL(vxReleaseRemap((vx_remap*)&exemplar));
        break;
        case VX_TYPE_LUT:
            ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateLUT(context, VX_TYPE_UINT8, 256), VX_TYPE_LUT);
            orig_obj_desc = ((vx_lut)exemplar)->base.obj_desc;
            ((vx_lut)exemplar)->base.obj_desc = NULL;

            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatFromReference(meta, exemplar));
            ((vx_lut)exemplar)->base.obj_desc = orig_obj_desc;

            VX_CALL(vxReleaseLUT((vx_lut*)&exemplar));
        break;
        case VX_TYPE_OBJECT_ARRAY:

            ASSERT_VX_OBJECT(pyr_exemplar = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, pyr_width, pyr_height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
            ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateObjectArray(context, (vx_reference)pyr_exemplar, 2), VX_TYPE_OBJECT_ARRAY);
            orig_obj_desc = ((vx_object_array)exemplar)->base.obj_desc;
            ((vx_object_array)exemplar)->base.obj_desc = NULL;

            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatFromReference(meta, exemplar));
            ((vx_object_array)exemplar)->base.obj_desc = orig_obj_desc;

            VX_CALL(vxReleaseObjectArray((vx_object_array*)&exemplar));
            VX_CALL(vxReleasePyramid(&pyr_exemplar));
        break;
        case VX_TYPE_TENSOR:
            dims = (vx_size*)ct_alloc_mem((4) * sizeof(vx_size));
            ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateTensor(context, 4, dims, VX_TYPE_UINT8, 0), VX_TYPE_TENSOR);
            orig_obj_desc = ((vx_tensor)exemplar)->base.obj_desc;
            ((vx_tensor)exemplar)->base.obj_desc = NULL;

            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatFromReference(meta, exemplar));
            ((vx_tensor)exemplar)->base.obj_desc = orig_obj_desc;
            ct_free_mem(dims);

            VX_CALL(vxReleaseTensor((vx_tensor*)&exemplar));
        break;
        case VX_TYPE_USER_DATA_OBJECT:
            ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateUserDataObject(context, (const vx_char*)&user_data_object_name, sizeof(wb_t), NULL), VX_TYPE_USER_DATA_OBJECT);
            orig_obj_desc = ((vx_user_data_object)exemplar)->base.obj_desc;
            ((vx_user_data_object)exemplar)->base.obj_desc = NULL;

            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatFromReference(meta, exemplar));
            ((vx_user_data_object)exemplar)->base.obj_desc = orig_obj_desc;

            VX_CALL(vxReleaseUserDataObject((vx_user_data_object*)&exemplar));
        break;
        case TIVX_TYPE_RAW_IMAGE:
            ASSERT_VX_OBJECT(exemplar = (vx_reference)tivxCreateRawImage(context, &params), TIVX_TYPE_RAW_IMAGE);
            orig_obj_desc = ((tivx_raw_image)exemplar)->base.obj_desc;
            ((tivx_raw_image)exemplar)->base.obj_desc = NULL;

            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatFromReference(meta, exemplar));
            ((tivx_raw_image)exemplar)->base.obj_desc = orig_obj_desc;

            VX_CALL(tivxReleaseRawImage((tivx_raw_image*)&exemplar));
        break;
        default:
            ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateGraph(context), VX_TYPE_GRAPH);

            ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetMetaFormatFromReference(meta, exemplar));

            VX_CALL(vxReleaseGraph((vx_graph*)&exemplar));
        break;
    }
    VX_CALL(ownReleaseMetaFormat(&meta));
}

TESTCASE_TESTS(tivxInternalMetaFormat,
    negativeInternalTestSetMetaFormatFromReference
    )