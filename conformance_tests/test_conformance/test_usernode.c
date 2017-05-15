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

#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>

#define VX_KERNEL_CONFORMANCE_TEST_OWN_BAD (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 0)
#define VX_KERNEL_CONFORMANCE_TEST_OWN_BAD_NAME "org.khronos.openvx.test.own_bad"

#define VX_KERNEL_CONFORMANCE_TEST_OWN (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 1)
#define VX_KERNEL_CONFORMANCE_TEST_OWN_NAME "org.khronos.openvx.test.own"

#define VX_KERNEL_CONFORMANCE_TEST_OWN_USER (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 2)
#define VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME "org.khronos.openvx.test.own_user"

TESTCASE(UserNode, CT_VXContext, ct_setup_vx_context, 0)

typedef enum _own_params_e
{
    OWN_PARAM_INPUT = 0,
    OWN_PARAM_OUTPUT,
} own_params_e;

static enum vx_type_e type = VX_TYPE_INVALID;
static enum vx_type_e objarray_itemtype = VX_TYPE_INVALID;

static vx_size local_size = 0;
static vx_bool is_kernel_alloc = vx_false_e;
static vx_size local_size_auto_alloc = 0;
static vx_size local_size_kernel_alloc = 0;

static vx_status set_local_size_status_init = VX_SUCCESS;
static vx_status set_local_ptr_status_init = VX_SUCCESS;

static vx_status query_local_size_status_deinit = VX_SUCCESS;
static vx_status query_local_ptr_status_deinit = VX_SUCCESS;
static vx_status set_local_size_status_deinit = VX_SUCCESS;
static vx_status set_local_ptr_status_deinit = VX_SUCCESS;


static vx_status VX_CALLBACK own_set_image_valid_rect(
    vx_node node,
    vx_uint32 index,
    const vx_rectangle_t* const input_valid[],
    vx_rectangle_t* const output_valid[])
{
    vx_status status = VX_FAILURE;

    if (index == OWN_PARAM_OUTPUT)
    {
        output_valid[0]->start_x = input_valid[0]->start_x + 2;
        output_valid[0]->start_y = input_valid[0]->start_y + 2;
        output_valid[0]->end_x   = input_valid[0]->end_x - 2;
        output_valid[0]->end_y   = input_valid[0]->end_y - 2;

        status = VX_SUCCESS;
    }

    return status;
}

static vx_bool is_validator_called = vx_false_e;
static vx_status VX_CALLBACK own_ValidatorMetaFromRef(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    is_validator_called = vx_true_e;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);

    vx_reference input = parameters[OWN_PARAM_INPUT];
    ASSERT_VX_OBJECT_(return VX_FAILURE, input, type);
    vx_reference output = parameters[OWN_PARAM_OUTPUT];
    ASSERT_VX_OBJECT_(return VX_FAILURE, output, type);

    vx_meta_format meta = metas[OWN_PARAM_OUTPUT];

    vx_enum in_ref_type = VX_TYPE_INVALID;
    VX_CALL_(return VX_ERROR_INVALID_PARAMETERS, vxQueryReference(input, VX_REFERENCE_TYPE, &in_ref_type, sizeof(vx_enum)));
    vx_enum out_ref_type = VX_TYPE_INVALID;
    VX_CALL_(return VX_ERROR_INVALID_PARAMETERS, vxQueryReference(output, VX_REFERENCE_TYPE, &out_ref_type, sizeof(vx_enum)));

    if (in_ref_type == out_ref_type)
    {
        vx_enum format = VX_DF_IMAGE_U8;
        vx_uint32 src_width = 128, src_height = 128;
        vx_uint32 dst_width = 256, dst_height = 256;
        vx_enum item_type = (type == VX_TYPE_OBJECT_ARRAY) ? objarray_itemtype : VX_TYPE_UINT8;
        vx_size capacity = 20;
        vx_size bins = 36;
        vx_int32 offset = 0;
        vx_uint32 range = 360;
        vx_enum thresh_type = VX_THRESHOLD_TYPE_BINARY;
        vx_size num_items = 100;
        vx_size m = 5, n = 5;

        vx_enum actual_format = VX_TYPE_INVALID;
        vx_uint32 actual_src_width = 128, actual_src_height = 128;
        vx_uint32 actual_dst_width = 256, actual_dst_height = 256;
        vx_enum actual_item_type = VX_TYPE_INVALID;
        vx_size actual_capacity = 0;
        vx_size actual_levels = 0;
        vx_float32 actual_scale = 0;
        vx_size actual_bins = 0;
        vx_int32 actual_offset = -1;
        vx_uint32 actual_range = 0;
        vx_enum actual_thresh_type = VX_TYPE_INVALID;
        vx_size actual_num_items = 0;
        vx_size actual_m = 0, actual_n = 0;
        switch (type)
        {
        case VX_TYPE_IMAGE:
            VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_FORMAT, &actual_format, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_WIDTH, &actual_src_width, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_HEIGHT, &actual_src_height, sizeof(vx_uint32)));

            if (format == actual_format && src_width == actual_src_width && src_height == actual_src_height)
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
                vx_kernel_image_valid_rectangle_f callback = &own_set_image_valid_rect;
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_VALID_RECT_CALLBACK, &callback, sizeof(callback)));
            }
            else
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_TYPE_ARRAY:
            VX_CALL_(return VX_FAILURE, vxQueryArray((vx_array)input, VX_ARRAY_ITEMTYPE, &actual_item_type, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxQueryArray((vx_array)input, VX_ARRAY_CAPACITY, &actual_capacity, sizeof(vx_size)));

            if (actual_item_type == item_type && actual_capacity == capacity)
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
            }
            else
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_TYPE_PYRAMID:
            VX_CALL_(return VX_FAILURE, vxQueryPyramid((vx_pyramid)input, VX_PYRAMID_FORMAT, &actual_format, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxQueryPyramid((vx_pyramid)input, VX_PYRAMID_WIDTH, &actual_src_width, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxQueryPyramid((vx_pyramid)input, VX_PYRAMID_HEIGHT, &actual_src_height, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxQueryPyramid((vx_pyramid)input, VX_PYRAMID_LEVELS, &actual_levels, sizeof(vx_size)));
            VX_CALL_(return VX_FAILURE, vxQueryPyramid((vx_pyramid)input, VX_PYRAMID_SCALE, &actual_scale, sizeof(vx_float32)));

            if (actual_format == format &&
                actual_src_width == src_width && actual_src_height == src_height &&
                actual_dst_width == dst_width && actual_dst_width == dst_width)
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
            }
            else
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_TYPE_SCALAR:
            VX_CALL_(return VX_FAILURE, vxQueryScalar((vx_scalar)input, VX_SCALAR_TYPE, &actual_item_type, sizeof(vx_enum)));

            if (actual_item_type == item_type)
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
            }
            break;
        case VX_TYPE_MATRIX:
            VX_CALL_(return VX_FAILURE, vxQueryMatrix((vx_matrix)input, VX_MATRIX_TYPE, &actual_item_type, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxQueryMatrix((vx_matrix)input, VX_MATRIX_ROWS, &actual_m, sizeof(vx_size)));
            VX_CALL_(return VX_FAILURE, vxQueryMatrix((vx_matrix)input, VX_MATRIX_COLUMNS, &actual_n, sizeof(vx_size)));

            if (actual_item_type == item_type && actual_m == m && actual_n == n)
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
            }
            else
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_TYPE_DISTRIBUTION:
            VX_CALL_(return VX_FAILURE, vxQueryDistribution((vx_distribution)input, VX_DISTRIBUTION_BINS, &actual_bins, sizeof(vx_size)));
            VX_CALL_(return VX_FAILURE, vxQueryDistribution((vx_distribution)input, VX_DISTRIBUTION_OFFSET, &actual_offset, sizeof(vx_int32)));
            VX_CALL_(return VX_FAILURE, vxQueryDistribution((vx_distribution)input, VX_DISTRIBUTION_RANGE, &actual_range, sizeof(vx_uint32)));

            if (actual_bins == bins && actual_offset == offset && actual_range == range)
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
            }
            else
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_TYPE_REMAP:
            VX_CALL_(return VX_FAILURE, vxQueryRemap((vx_remap)input, VX_REMAP_SOURCE_WIDTH, &actual_src_width, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxQueryRemap((vx_remap)input, VX_REMAP_SOURCE_HEIGHT, &actual_src_height, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxQueryRemap((vx_remap)input, VX_REMAP_DESTINATION_WIDTH, &actual_dst_width, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxQueryRemap((vx_remap)input, VX_REMAP_DESTINATION_HEIGHT, &actual_dst_height, sizeof(vx_uint32)));

            if (actual_src_width == src_width && actual_src_height == src_height &&
                actual_dst_width == dst_width && actual_dst_height == dst_height)
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
            }
            else
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_TYPE_LUT:
            VX_CALL_(return VX_FAILURE, vxQueryLUT((vx_lut)input, VX_LUT_TYPE, &actual_item_type, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxQueryLUT((vx_lut)input, VX_LUT_COUNT, &actual_num_items, sizeof(vx_size)));
            if (actual_item_type == item_type && actual_num_items == num_items)
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
            }
            else
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_TYPE_THRESHOLD:
            VX_CALL_(return VX_FAILURE, vxQueryThreshold((vx_threshold)input, VX_THRESHOLD_TYPE, &actual_thresh_type, sizeof(vx_enum)));
            if (actual_thresh_type == thresh_type)
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
            }
            else
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_TYPE_OBJECT_ARRAY:
            VX_CALL_(return VX_FAILURE, vxQueryObjectArray((vx_object_array)input, VX_OBJECT_ARRAY_ITEMTYPE, &actual_item_type, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxQueryObjectArray((vx_object_array)input, VX_OBJECT_ARRAY_NUMITEMS, &actual_capacity, sizeof(vx_size)));

            if (actual_item_type == item_type && actual_capacity == capacity)
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
            }
            else
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        default:
            return VX_ERROR_INVALID_PARAMETERS;
            break;
        }

    }

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK own_ValidatorMetaFromAttr(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    is_validator_called = vx_true_e;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);

    vx_reference input = parameters[OWN_PARAM_INPUT];

    vx_meta_format meta = metas[OWN_PARAM_OUTPUT];

    vx_enum format = VX_DF_IMAGE_U8;
    vx_uint32 src_width = 128, src_height = 128;
    vx_uint32 dst_width = 256, dst_height = 256;
    vx_enum item_type = (type == VX_TYPE_OBJECT_ARRAY) ? objarray_itemtype : VX_TYPE_UINT8;
    vx_size capacity = 20;
    vx_size levels = 8;
    vx_float32 scale = 0.5f;
    vx_size bins = 36;
    vx_int32 offset = 0;
    vx_uint32 range = 360;
    vx_enum thresh_type = VX_THRESHOLD_TYPE_BINARY;
    vx_size num_items = 100;
    vx_size m = 5, n = 5;

    vx_enum actual_format = VX_TYPE_INVALID;
    vx_uint32 actual_src_width = 128, actual_src_height = 128;
    vx_uint32 actual_dst_width = 256, actual_dst_height = 256;
    vx_enum actual_item_type = VX_TYPE_INVALID;
    vx_size actual_capacity = 0;
    vx_size actual_levels = 0;
    vx_float32 actual_scale = 0;
    vx_size actual_bins = 0;
    vx_int32 actual_offset = -1;
    vx_uint32 actual_range = 0;
    vx_enum actual_thresh_type = VX_TYPE_INVALID;
    vx_size actual_num_items = 0;
    vx_size actual_m = 0, actual_n = 0;
    switch (type)
    {
    case VX_TYPE_IMAGE:
        VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_FORMAT, &actual_format, sizeof(vx_enum)));
        VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_WIDTH, &actual_src_width, sizeof(vx_uint32)));
        VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_HEIGHT, &actual_src_height, sizeof(vx_uint32)));

        if (format == actual_format && src_width == actual_src_width && src_height == actual_src_height)
        {
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_IMAGE_FORMAT, &format, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_IMAGE_WIDTH, &src_width, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_IMAGE_HEIGHT, &src_height, sizeof(vx_uint32)));
            vx_kernel_image_valid_rectangle_f callback = &own_set_image_valid_rect;
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_VALID_RECT_CALLBACK, &callback, sizeof(callback)));
        }
        else
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
        break;
    case VX_TYPE_ARRAY:
        VX_CALL_(return VX_FAILURE, vxQueryArray((vx_array)input, VX_ARRAY_ITEMTYPE, &actual_item_type, sizeof(vx_enum)));
        VX_CALL_(return VX_FAILURE, vxQueryArray((vx_array)input, VX_ARRAY_CAPACITY, &actual_capacity, sizeof(vx_size)));

        if (actual_item_type == item_type && actual_capacity == capacity)
        {
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_ARRAY_ITEMTYPE, &item_type, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_ARRAY_CAPACITY, &capacity, sizeof(vx_size)));
        }
        else
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
        break;
    case VX_TYPE_PYRAMID:
        VX_CALL_(return VX_FAILURE, vxQueryPyramid((vx_pyramid)input, VX_PYRAMID_FORMAT, &actual_format, sizeof(vx_enum)));
        VX_CALL_(return VX_FAILURE, vxQueryPyramid((vx_pyramid)input, VX_PYRAMID_WIDTH, &actual_src_width, sizeof(vx_uint32)));
        VX_CALL_(return VX_FAILURE, vxQueryPyramid((vx_pyramid)input, VX_PYRAMID_HEIGHT, &actual_src_height, sizeof(vx_uint32)));
        VX_CALL_(return VX_FAILURE, vxQueryPyramid((vx_pyramid)input, VX_PYRAMID_LEVELS, &actual_levels, sizeof(vx_size)));
        VX_CALL_(return VX_FAILURE, vxQueryPyramid((vx_pyramid)input, VX_PYRAMID_SCALE, &actual_scale, sizeof(vx_float32)));

        if (actual_format == format &&
            actual_src_width == src_width && actual_src_height == src_height &&
            actual_dst_width == dst_width && actual_dst_width == dst_width)
        {
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_PYRAMID_FORMAT, &format, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_PYRAMID_WIDTH, &src_width, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_PYRAMID_HEIGHT, &src_height, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_PYRAMID_LEVELS, &levels, sizeof(vx_size)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_PYRAMID_SCALE, &scale, sizeof(vx_float32)));
        }
        else
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
        break;
    case VX_TYPE_SCALAR:
        VX_CALL_(return VX_FAILURE, vxQueryScalar((vx_scalar)input, VX_SCALAR_TYPE, &actual_item_type, sizeof(vx_enum)));

        if (actual_item_type == item_type)
        {
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_SCALAR_TYPE, &item_type, sizeof(vx_enum)));
        }
        break;
    case VX_TYPE_MATRIX:
        VX_CALL_(return VX_FAILURE, vxQueryMatrix((vx_matrix)input, VX_MATRIX_TYPE, &actual_item_type, sizeof(vx_enum)));
        VX_CALL_(return VX_FAILURE, vxQueryMatrix((vx_matrix)input, VX_MATRIX_ROWS, &actual_m, sizeof(vx_size)));
        VX_CALL_(return VX_FAILURE, vxQueryMatrix((vx_matrix)input, VX_MATRIX_COLUMNS, &actual_n, sizeof(vx_size)));

        if (actual_item_type == item_type && actual_m == m && actual_n == n)
        {
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_MATRIX_TYPE, &item_type, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_MATRIX_ROWS, &m, sizeof(vx_size)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_MATRIX_COLUMNS, &n, sizeof(vx_size)));
        }
        else
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
        break;
    case VX_TYPE_DISTRIBUTION:
        VX_CALL_(return VX_FAILURE, vxQueryDistribution((vx_distribution)input, VX_DISTRIBUTION_BINS, &actual_bins, sizeof(vx_size)));
        VX_CALL_(return VX_FAILURE, vxQueryDistribution((vx_distribution)input, VX_DISTRIBUTION_OFFSET, &actual_offset, sizeof(vx_int32)));
        VX_CALL_(return VX_FAILURE, vxQueryDistribution((vx_distribution)input, VX_DISTRIBUTION_RANGE, &actual_range, sizeof(vx_uint32)));

        if (actual_bins == bins && actual_offset == offset && actual_range == range)
        {
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_DISTRIBUTION_BINS, &bins, sizeof(vx_size)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_DISTRIBUTION_OFFSET, &offset, sizeof(vx_int32)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_DISTRIBUTION_RANGE, &range, sizeof(vx_uint32)));
        }
        else
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
        break;
    case VX_TYPE_REMAP:
        VX_CALL_(return VX_FAILURE, vxQueryRemap((vx_remap)input, VX_REMAP_SOURCE_WIDTH, &actual_src_width, sizeof(vx_uint32)));
        VX_CALL_(return VX_FAILURE, vxQueryRemap((vx_remap)input, VX_REMAP_SOURCE_HEIGHT, &actual_src_height, sizeof(vx_uint32)));
        VX_CALL_(return VX_FAILURE, vxQueryRemap((vx_remap)input, VX_REMAP_DESTINATION_WIDTH, &actual_dst_width, sizeof(vx_uint32)));
        VX_CALL_(return VX_FAILURE, vxQueryRemap((vx_remap)input, VX_REMAP_DESTINATION_HEIGHT, &actual_dst_height, sizeof(vx_uint32)));

        if (actual_src_width == src_width && actual_src_height == src_height &&
            actual_dst_width == dst_width && actual_dst_height == dst_height)
        {
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_REMAP_SOURCE_WIDTH, &src_width, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_REMAP_SOURCE_HEIGHT, &src_height, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_REMAP_DESTINATION_WIDTH, &dst_width, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_REMAP_DESTINATION_HEIGHT, &dst_height, sizeof(vx_uint32)));
        }
        else
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
        break;
    case VX_TYPE_LUT:
        VX_CALL_(return VX_FAILURE, vxQueryLUT((vx_lut)input, VX_LUT_TYPE, &actual_item_type, sizeof(vx_enum)));
        VX_CALL_(return VX_FAILURE, vxQueryLUT((vx_lut)input, VX_LUT_COUNT, &actual_num_items, sizeof(vx_size)));
        if (actual_item_type == item_type && actual_num_items == num_items)
        {
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_LUT_TYPE, &item_type, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_LUT_COUNT, &num_items, sizeof(vx_size)));
        }
        else
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
        break;
    case VX_TYPE_THRESHOLD:
        VX_CALL_(return VX_FAILURE, vxQueryThreshold((vx_threshold)input, VX_THRESHOLD_TYPE, &actual_thresh_type, sizeof(vx_enum)));
        if (actual_thresh_type == thresh_type)
        {
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_THRESHOLD_TYPE, &thresh_type, sizeof(vx_enum)));
        }
        else
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
        break;
    case VX_TYPE_OBJECT_ARRAY:
        VX_CALL_(return VX_FAILURE, vxQueryObjectArray((vx_object_array)input, VX_OBJECT_ARRAY_ITEMTYPE, &actual_item_type, sizeof(vx_enum)));
        VX_CALL_(return VX_FAILURE, vxQueryObjectArray((vx_object_array)input, VX_OBJECT_ARRAY_NUMITEMS, &actual_capacity, sizeof(vx_size)));

        if (actual_item_type == item_type && actual_capacity == capacity)
        {
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_OBJECT_ARRAY_ITEMTYPE, &item_type, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_OBJECT_ARRAY_NUMITEMS, &capacity, sizeof(vx_size)));
        }
        else
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
        break;
    default:
        return VX_ERROR_INVALID_PARAMETERS;
        break;
    }

    return VX_SUCCESS;
}

static vx_bool is_kernel_called = vx_false_e;
static vx_status VX_CALLBACK own_Kernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    is_kernel_called = vx_true_e;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);
    EXPECT(parameters != NULL);
    EXPECT(num == 2);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }

    return VX_SUCCESS;
}

static vx_bool is_initialize_called = vx_false_e;
static vx_status VX_CALLBACK own_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_size size = 0;
    void* ptr = NULL;
    is_initialize_called = vx_true_e;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);
    EXPECT(parameters != NULL);
    EXPECT(num == 2);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }
    if (local_size_kernel_alloc > 0)
    {
        size = local_size_kernel_alloc;
        ptr = ct_calloc(1, local_size_kernel_alloc);
    }
    set_local_size_status_init = vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_SIZE, &size, sizeof(size));
    set_local_ptr_status_init = vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_PTR, &ptr, sizeof(ptr));
    return VX_SUCCESS;
}

static vx_bool is_deinitialize_called = vx_false_e;
static vx_status VX_CALLBACK own_Deinitialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_size size = 0;
    void* ptr = NULL;
    is_deinitialize_called = vx_true_e;
    EXPECT(node != 0);
    EXPECT(parameters != NULL);
    EXPECT(num == 2);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }
    query_local_size_status_deinit = vxQueryNode(node, VX_NODE_LOCAL_DATA_SIZE, &size, sizeof(size));
    query_local_ptr_status_deinit = vxQueryNode(node, VX_NODE_LOCAL_DATA_PTR, &ptr, sizeof(ptr));
    if (local_size_kernel_alloc > 0)
    {
        size = 0;
        if (ptr != NULL)
        {
            ct_free_mem(ptr);
            ptr = NULL;
        }
    }
    set_local_size_status_deinit = vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_SIZE, &size, sizeof(size));
    set_local_ptr_status_deinit = vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_PTR, &ptr, sizeof(ptr));
    return VX_SUCCESS;
}

static void own_register_kernel(vx_context context, vx_bool is_meta_from_ref)
{
    vx_kernel kernel = 0;
    vx_size size = local_size_auto_alloc;

    if (is_meta_from_ref)
    {
        ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
            context,
            VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME,
            VX_KERNEL_CONFORMANCE_TEST_OWN_USER,
            own_Kernel,
            2,
            own_ValidatorMetaFromRef,
            own_Initialize,
            own_Deinitialize), VX_TYPE_KERNEL);
    }
    else
    {
        ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
            context,
            VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME,
            VX_KERNEL_CONFORMANCE_TEST_OWN_USER,
            own_Kernel,
            2,
            own_ValidatorMetaFromAttr,
            own_Initialize,
            own_Deinitialize), VX_TYPE_KERNEL);
    }

    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_INPUT, VX_INPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_INPUT), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_INPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_OUTPUT, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_OUTPUT), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxSetKernelAttribute(kernel, VX_KERNEL_LOCAL_DATA_SIZE, &size, sizeof(size)));
    VX_CALL(vxFinalizeKernel(kernel));
    VX_CALL(vxReleaseKernel(&kernel));
}

typedef struct {
    const char* name;
    vx_enum type;
    vx_bool is_meta_from_ref;
    vx_size local_size;
    vx_bool is_kernel_alloc;
} type_arg;

#define ADD_TYPE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "IMAGE", __VA_ARGS__, VX_TYPE_IMAGE)), \
    CT_EXPAND(nextmacro(testArgName "ARRAY", __VA_ARGS__, VX_TYPE_ARRAY)), \
    CT_EXPAND(nextmacro(testArgName "PYRAMID", __VA_ARGS__, VX_TYPE_PYRAMID)), \
    CT_EXPAND(nextmacro(testArgName "SCALAR", __VA_ARGS__, VX_TYPE_SCALAR)), \
    CT_EXPAND(nextmacro(testArgName "DISTRIBUTION", __VA_ARGS__, VX_TYPE_DISTRIBUTION)), \
    CT_EXPAND(nextmacro(testArgName "MATRIX", __VA_ARGS__, VX_TYPE_MATRIX)), \
    CT_EXPAND(nextmacro(testArgName "THRESHOLD", __VA_ARGS__, VX_TYPE_THRESHOLD)), \
    CT_EXPAND(nextmacro(testArgName "LUT", __VA_ARGS__, VX_TYPE_LUT)), \
    CT_EXPAND(nextmacro(testArgName "REMAP", __VA_ARGS__, VX_TYPE_REMAP))

#define ADD_FROM_FLAG(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "_FROM_REF", __VA_ARGS__, vx_true_e)), \
    CT_EXPAND(nextmacro(testArgName "_FROM_ATTR", __VA_ARGS__, vx_false_e))

#define ADD_LOCAL_SIZE_AND_ALLOC(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/LOCAL_SIZE=0", __VA_ARGS__, 0, vx_false_e)), \
    CT_EXPAND(nextmacro(testArgName "/LOCAL_SIZE=10/ALLOC=AUTO", __VA_ARGS__, 10, vx_false_e)), \
    CT_EXPAND(nextmacro(testArgName "/LOCAL_SIZE=10/ALLOC=KERNEL", __VA_ARGS__, 10, vx_true_e))

#define USERKERNEL_PARAMETERS \
    CT_GENERATE_PARAMETERS("", ADD_TYPE, ADD_FROM_FLAG, ADD_LOCAL_SIZE_AND_ALLOC, ARG)

TEST_WITH_ARG(UserNode, testUserKernel, type_arg, USERKERNEL_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_reference src = 0, dst = 0;
    vx_graph graph = 0;
    vx_kernel user_kernel = 0;
    vx_node node = 0;
    vx_bool is_meta_from_ref = arg_->is_meta_from_ref;

    vx_uint8 value = 0;
    vx_enum format = VX_DF_IMAGE_U8;
    vx_uint32 src_width = 128, src_height = 128;
    vx_uint32 dst_width = 256, dst_height = 256;
    vx_enum item_type = VX_TYPE_UINT8;
    vx_size capacity = 20;
    vx_size levels = 8;
    vx_float32 scale = 0.5f;
    vx_size bins = 36;
    vx_int32 offset = 0;
    vx_uint32 range = 360;
    vx_enum thresh_type = VX_THRESHOLD_TYPE_BINARY;
    vx_size num_items = 100;
    vx_size m = 5, n = 5;

    int phase = 0;

    type = (enum vx_type_e)arg_->type;
    local_size = arg_->local_size;
    is_kernel_alloc = arg_->is_kernel_alloc;

    if (is_kernel_alloc == vx_false_e)
    {
        local_size_auto_alloc = local_size;
        local_size_kernel_alloc = 0;
    }
    else
    {
        local_size_auto_alloc = 0;
        local_size_kernel_alloc = local_size;
    }

    is_validator_called = vx_false_e;
    is_kernel_called = vx_false_e;
    is_initialize_called = vx_false_e;
    is_deinitialize_called = vx_false_e;

    switch (type)
    {
    case VX_TYPE_IMAGE:
        ASSERT_VX_OBJECT(src = (vx_reference)vxCreateImage(context, src_width, src_height, format), type);
        ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateImage(context, src_width, src_height, format), type);
        break;
    case VX_TYPE_ARRAY:
        ASSERT_VX_OBJECT(src = (vx_reference)vxCreateArray(context, item_type, capacity), type);
        ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateArray(context, item_type, capacity), type);
        break;
    case VX_TYPE_PYRAMID:
        ASSERT_VX_OBJECT(src = (vx_reference)vxCreatePyramid(context, levels, scale, src_width, src_height, format), type);
        ASSERT_VX_OBJECT(dst = (vx_reference)vxCreatePyramid(context, levels, scale, src_width, src_height, format), type);
        break;
    case VX_TYPE_SCALAR:
        ASSERT_VX_OBJECT(src = (vx_reference)vxCreateScalar(context, item_type, &value), type);
        ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateScalar(context, item_type, &value), type);
        break;
    case VX_TYPE_MATRIX:
        ASSERT_VX_OBJECT(src = (vx_reference)vxCreateMatrix(context, item_type, m, n), type);
        ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateMatrix(context, item_type, m, n), type);
        break;
    case VX_TYPE_DISTRIBUTION:
        ASSERT_VX_OBJECT(src = (vx_reference)vxCreateDistribution(context, bins, offset, range), type);
        ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateDistribution(context, bins, offset, range), type);
        break;
    case VX_TYPE_REMAP:
        ASSERT_VX_OBJECT(src = (vx_reference)vxCreateRemap(context, src_width, src_height, dst_width, dst_height), type);
        ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateRemap(context, src_width, src_height, dst_width, dst_height), type);
        break;
    case VX_TYPE_LUT:
        ASSERT_VX_OBJECT(src = (vx_reference)vxCreateLUT(context, item_type, num_items), type);
        ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateLUT(context, item_type, num_items), type);
        break;
    case VX_TYPE_THRESHOLD:
        ASSERT_VX_OBJECT(src = (vx_reference)vxCreateThreshold(context, thresh_type, item_type), type);
        ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateThreshold(context, thresh_type, item_type), type);
        break;
    default:
        break;
    }

    ASSERT_NO_FAILURE(own_register_kernel(context, is_meta_from_ref));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, user_kernel), VX_TYPE_NODE);

    VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)src));
    VX_CALL(vxSetParameterByIndex(node, 1, (vx_reference)dst));

    // graph verification, first-time, and re-verify

    for (phase = 0; phase < 2; ++phase)
    {
        vx_size size = 0;
        void* ptr = NULL;

        is_initialize_called = vx_false_e;
        is_deinitialize_called = vx_false_e;
        is_validator_called = vx_false_e;
        set_local_size_status_init = VX_FAILURE;
        set_local_ptr_status_init = VX_FAILURE;
        query_local_size_status_deinit = VX_FAILURE;
        query_local_ptr_status_deinit = VX_FAILURE;
        set_local_size_status_deinit = VX_FAILURE;
        set_local_ptr_status_deinit = VX_FAILURE;

        VX_CALL(vxVerifyGraph(graph));

        ASSERT(is_initialize_called == vx_true_e);
        if (phase == 0)
            ASSERT(is_deinitialize_called == vx_false_e);
        else
            ASSERT(is_deinitialize_called == vx_true_e);
        ASSERT(is_validator_called == vx_true_e);

        VX_CALL(vxQueryNode(node, VX_NODE_LOCAL_DATA_SIZE, &size, sizeof(size)));
        VX_CALL(vxQueryNode(node, VX_NODE_LOCAL_DATA_PTR, &ptr, sizeof(ptr)));

        ASSERT(VX_SUCCESS != vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_SIZE, &size, sizeof(size)));
        ASSERT(VX_SUCCESS != vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_PTR, &ptr, sizeof(ptr)));

        ASSERT(size == local_size);
        if (size > 0)
            ASSERT(ptr != NULL);
        else
            ASSERT(ptr == NULL);

        if (local_size_auto_alloc == 0) // change allowed
        {
            ASSERT(set_local_size_status_init == VX_SUCCESS);
            ASSERT(set_local_ptr_status_init == VX_SUCCESS);
            if (is_deinitialize_called)
            {
                ASSERT(set_local_size_status_deinit == VX_SUCCESS);
                ASSERT(set_local_ptr_status_deinit == VX_SUCCESS);
            }
        }
        else // change does not allowed: error must be generated
        {
            ASSERT(set_local_size_status_init != VX_SUCCESS);
            ASSERT(set_local_ptr_status_init != VX_SUCCESS);
            if (is_deinitialize_called)
            {
                ASSERT(set_local_size_status_deinit != VX_SUCCESS);
                ASSERT(set_local_ptr_status_deinit != VX_SUCCESS);
            }
        }
    }

    // execute graph

    is_initialize_called = vx_false_e;
    is_deinitialize_called = vx_false_e;
    is_validator_called = vx_false_e;
    is_kernel_called = vx_false_e;

    VX_CALL(vxProcessGraph(graph));

    ASSERT(is_initialize_called == vx_false_e);
    ASSERT(is_deinitialize_called == vx_false_e);
    ASSERT(is_validator_called == vx_false_e);
    ASSERT(is_kernel_called == vx_true_e);

    if (VX_TYPE_IMAGE == type)
    {
        vx_rectangle_t in_rect;
        vx_rectangle_t out_rect;

        VX_CALL(vxGetValidRegionImage((vx_image)src, &in_rect));
        VX_CALL(vxGetValidRegionImage((vx_image)dst, &out_rect));

        ASSERT_EQ_INT(in_rect.start_x + 2, out_rect.start_x);
        ASSERT_EQ_INT(in_rect.start_y + 2, out_rect.start_y);
        ASSERT_EQ_INT(in_rect.end_x - 2, out_rect.end_x);
        ASSERT_EQ_INT(in_rect.end_y - 2, out_rect.end_y);
    }

    // finalization

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* user kernel should be removed only after all references to it released */
    /* Note, vxRemoveKernel doesn't zeroing kernel ref */
    VX_CALL(vxRemoveKernel(user_kernel));

    VX_CALL(vxReleaseReference(&dst));
    VX_CALL(vxReleaseReference(&src));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(dst == 0);
    ASSERT(src == 0);
}

TEST_WITH_ARG(UserNode, testUserKernelObjectArray, type_arg,
    ARG("IMAGE_FROM_REF", VX_TYPE_IMAGE, vx_true_e),
    ARG("ARRAY_FROM_REF", VX_TYPE_ARRAY, vx_true_e),
    ARG("PYRAMID_FROM_REF", VX_TYPE_PYRAMID, vx_true_e),
    ARG("SCALAR_FROM_REF", VX_TYPE_SCALAR, vx_true_e),
    ARG("DISTRIBUTION_FROM_REF", VX_TYPE_DISTRIBUTION, vx_true_e),
    ARG("MATRIX_FROM_REF", VX_TYPE_MATRIX, vx_true_e),
    ARG("THRESHOLD_FROM_REF", VX_TYPE_THRESHOLD, vx_true_e),
    ARG("LUT_FROM_REF", VX_TYPE_LUT, vx_true_e),
    ARG("REMAP_FROM_REF", VX_TYPE_REMAP, vx_true_e),
    ARG("IMAGE_FROM_ATTR", VX_TYPE_IMAGE, vx_false_e),
    ARG("ARRAY_FROM_ATTR",VX_TYPE_ARRAY, vx_false_e),
    ARG("PYRAMID_FROM_ATTR",VX_TYPE_PYRAMID, vx_false_e),
    ARG("SCALAR_FROM_ATTR",VX_TYPE_SCALAR, vx_false_e),
    ARG("DISTRIBUTION_FROM_ATTR",VX_TYPE_DISTRIBUTION, vx_false_e),
    ARG("MATRIX_FROM_ATTR",VX_TYPE_MATRIX, vx_false_e),
    ARG("THRESHOLD_FROM_ATTR",VX_TYPE_THRESHOLD, vx_false_e),
    ARG("LUT_FROM_ATTR",VX_TYPE_LUT, vx_false_e),
    ARG("REMAP_FROM_ATTR",VX_TYPE_REMAP, vx_false_e)
)
{
    vx_context context = context_->vx_context_;
    vx_reference exemplar = 0, src = 0, dst = 0;
    vx_graph graph = 0;
    vx_kernel user_kernel = 0;
    vx_node node = 0;
    vx_bool is_meta_from_ref = arg_->is_meta_from_ref;
    objarray_itemtype = (enum vx_type_e)arg_->type;
    type = VX_TYPE_OBJECT_ARRAY;

    is_validator_called = vx_false_e;
    is_kernel_called = vx_false_e;
    is_initialize_called = vx_false_e;
    is_deinitialize_called = vx_false_e;

    vx_uint8 value = 0;
    vx_enum format = VX_DF_IMAGE_U8;
    vx_uint32 src_width = 128, src_height = 128;
    vx_uint32 dst_width = 256, dst_height = 256;
    vx_enum item_type = VX_TYPE_UINT8;
    vx_size capacity = 20;
    vx_size levels = 8;
    vx_float32 scale = 0.5f;
    vx_size bins = 36;
    vx_int32 offset = 0;
    vx_uint32 range = 360;
    vx_enum thresh_type = VX_THRESHOLD_TYPE_BINARY;
    vx_size num_items = 100;
    vx_size m = 5, n = 5;
    switch (objarray_itemtype)
    {
    case VX_TYPE_IMAGE:
        ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateImage(context, src_width, src_height, format), objarray_itemtype);
        break;
    case VX_TYPE_ARRAY:
        ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateArray(context, item_type, capacity), objarray_itemtype);
        break;
    case VX_TYPE_PYRAMID:
        ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreatePyramid(context, levels, scale, src_width, src_height, format), objarray_itemtype);
        break;
    case VX_TYPE_SCALAR:
        ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateScalar(context, item_type, &value), objarray_itemtype);
        break;
    case VX_TYPE_MATRIX:
        ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateMatrix(context, item_type, m, n), objarray_itemtype);
        break;
    case VX_TYPE_DISTRIBUTION:
        ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateDistribution(context, bins, offset, range), objarray_itemtype);
        break;
    case VX_TYPE_REMAP:
        ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateRemap(context, src_width, src_height, dst_width, dst_height), objarray_itemtype);
        break;
    case VX_TYPE_LUT:
        ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateLUT(context, item_type, num_items), objarray_itemtype);
        break;
    case VX_TYPE_THRESHOLD:
        ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateThreshold(context, thresh_type, item_type), objarray_itemtype);
        break;
    default:
        break;
    }

    ASSERT_VX_OBJECT(src = (vx_reference)vxCreateObjectArray(context, exemplar, capacity), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateObjectArray(context, exemplar, capacity), VX_TYPE_OBJECT_ARRAY);

    ASSERT_NO_FAILURE(own_register_kernel(context, is_meta_from_ref));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, user_kernel), VX_TYPE_NODE);

    VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)src));
    VX_CALL(vxSetParameterByIndex(node, 1, (vx_reference)dst));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* user kernel should be removed only after all references to it released */
    /* Note, vxRemoveKernel doesn't zeroing kernel ref */
    VX_CALL(vxRemoveKernel(user_kernel));

    VX_CALL(vxReleaseReference(&dst));
    VX_CALL(vxReleaseReference(&src));
    VX_CALL(vxReleaseReference(&exemplar));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(dst == 0);
    ASSERT(src == 0);
    ASSERT(exemplar == 0);

    ASSERT(is_validator_called == vx_true_e);
    ASSERT(is_kernel_called == vx_true_e);
    ASSERT(is_initialize_called == vx_true_e);
    ASSERT(is_deinitialize_called == vx_true_e);
}

TEST(UserNode, testRemoveKernel)
{
    vx_context context = context_->vx_context_;
    vx_kernel kernel = 0;

    EXPECT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_ADD), VX_TYPE_KERNEL);
    // Only kernels added through vxAddUserKernel can be removed
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxRemoveKernel(kernel));
    VX_CALL(vxReleaseKernel(&kernel));

    ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
            context,
            VX_KERNEL_CONFORMANCE_TEST_OWN_BAD_NAME,
            VX_KERNEL_CONFORMANCE_TEST_OWN_BAD,
            own_Kernel,
            2,
            own_ValidatorMetaFromRef,
            own_Initialize,
            own_Deinitialize), VX_TYPE_KERNEL);

    VX_CALL(vxRemoveKernel(kernel));
}

TEST(UserNode, testOutDelay)
{
    vx_context context = context_->vx_context_;
    vx_kernel kernel = 0;

    ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
        context,
        VX_KERNEL_CONFORMANCE_TEST_OWN_BAD_NAME,
        VX_KERNEL_CONFORMANCE_TEST_OWN_BAD,
        own_Kernel,
        2,
        own_ValidatorMetaFromRef,
        own_Initialize,
        own_Deinitialize), VX_TYPE_KERNEL);

    ASSERT_NE_VX_STATUS(vxAddParameterToKernel(kernel, 0, VX_OUTPUT, VX_TYPE_DELAY, VX_PARAMETER_STATE_REQUIRED), VX_SUCCESS);

    VX_CALL(vxRemoveKernel(kernel));
}

TESTCASE_TESTS(UserNode,
        testUserKernel,
        testUserKernelObjectArray,
        testRemoveKernel,
        testOutDelay
        )
