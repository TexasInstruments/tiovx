/*

 * Copyright (c) 2015-2017 The Khronos Group Inc.
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

#include <VX/vx.h>
#include <VX/vxu.h>

#include "test_engine/test.h"

TESTCASE(ObjectArray, CT_VXContext, ct_setup_vx_context, 0)

typedef struct
{
    const char* testName;
    const char* p;
    vx_enum item_type;
} Obj_Array_Arg;


static vx_reference own_create_exemplar(vx_context context, vx_enum item_type)
{
    vx_reference exemplar = NULL;

    vx_uint8 value = 0;
    vx_enum format = VX_DF_IMAGE_U8;
    vx_uint32 obj_width = 128, obj_height = 128;
    vx_enum obj_item_type = VX_TYPE_UINT8;
    vx_size capacity = 100;
    vx_size levels = 8;
    vx_float32 scale = 0.5f;
    vx_size bins = 36;
    vx_int32 offset = 0;
    vx_uint32 range = 360;
    vx_enum thresh_type = VX_THRESHOLD_TYPE_BINARY;
    vx_size lut_num_items = 100;
    vx_size m = 5, n = 5;

    switch (item_type)
    {
        case VX_TYPE_IMAGE:
            exemplar = (vx_reference)vxCreateImage(context, obj_width, obj_height, format);
            break;
        case VX_TYPE_ARRAY:
            exemplar = (vx_reference)vxCreateArray(context, obj_item_type, capacity);
            break;
        case VX_TYPE_PYRAMID:
            exemplar = (vx_reference)vxCreatePyramid(context, levels, scale, obj_width, obj_height, format);
            break;
        case VX_TYPE_SCALAR:
            exemplar = (vx_reference)vxCreateScalar(context, obj_item_type, &value);
            break;
        case VX_TYPE_MATRIX:
            exemplar = (vx_reference)vxCreateMatrix(context, obj_item_type, m, n);
            break;
        case VX_TYPE_DISTRIBUTION:
            exemplar = (vx_reference)vxCreateDistribution(context, bins, offset, range);
            break;
        case VX_TYPE_REMAP:
            exemplar = (vx_reference)vxCreateRemap(context, obj_width, obj_height, obj_width, obj_height);
            break;
        case VX_TYPE_LUT:
            exemplar = (vx_reference)vxCreateLUT(context, obj_item_type, lut_num_items);
            break;
        case VX_TYPE_THRESHOLD:
            exemplar = (vx_reference)vxCreateThreshold(context, thresh_type, obj_item_type);
            break;
        default:
            break;
    }

    return exemplar;
}

static void own_check_meta(vx_reference item, vx_reference ref)
{
    vx_enum ref_type, item_type;

    VX_CALL(vxQueryReference(ref, VX_REFERENCE_TYPE, &ref_type, sizeof(ref_type)));

    VX_CALL(vxQueryReference(item, VX_REFERENCE_TYPE, &item_type, sizeof(item_type)));

    ASSERT(item_type == ref_type);

    switch (item_type)
    {
        case VX_TYPE_IMAGE:
        {
            vx_uint32 ref_width, item_width;
            vx_uint32 ref_height, item_height;
            vx_df_image ref_format, item_format;

            VX_CALL(vxQueryImage((vx_image)ref, VX_IMAGE_WIDTH, &ref_width, sizeof(ref_width)));
            VX_CALL(vxQueryImage((vx_image)ref, VX_IMAGE_HEIGHT, &ref_height, sizeof(ref_height)));
            VX_CALL(vxQueryImage((vx_image)ref, VX_IMAGE_FORMAT, &ref_format, sizeof(ref_format)));

            VX_CALL(vxQueryImage((vx_image)item, VX_IMAGE_WIDTH, &item_width, sizeof(item_width)));
            VX_CALL(vxQueryImage((vx_image)item, VX_IMAGE_HEIGHT, &item_height, sizeof(item_height)));
            VX_CALL(vxQueryImage((vx_image)item, VX_IMAGE_FORMAT, &item_format, sizeof(item_format)));

            ASSERT(ref_width == item_width);
            ASSERT(ref_height == item_height);
            ASSERT(ref_format == item_format);
        }   break;
        case VX_TYPE_ARRAY:
        {
            vx_size ref_capacity, item_capacity;
            vx_enum ref_itemtype, item_itemtype;

            VX_CALL(vxQueryArray((vx_array)ref, VX_ARRAY_CAPACITY, &ref_capacity, sizeof(ref_capacity)));
            VX_CALL(vxQueryArray((vx_array)ref, VX_ARRAY_ITEMTYPE, &ref_itemtype, sizeof(ref_itemtype)));

            VX_CALL(vxQueryArray((vx_array)item, VX_ARRAY_CAPACITY, &item_capacity, sizeof(item_capacity)));
            VX_CALL(vxQueryArray((vx_array)item, VX_ARRAY_ITEMTYPE, &item_itemtype, sizeof(item_itemtype)));

            ASSERT(ref_capacity == item_capacity);
            ASSERT(ref_itemtype == item_itemtype);
        }   break;
        case VX_TYPE_PYRAMID:
        {
            vx_uint32 ref_width, item_width;
            vx_uint32 ref_height, item_height;
            vx_df_image ref_format, item_format;
            vx_size ref_levels, item_levels;
            vx_float32 ref_scale, item_scale;

            VX_CALL(vxQueryPyramid((vx_pyramid)ref, VX_PYRAMID_WIDTH, &ref_width, sizeof(ref_width)));
            VX_CALL(vxQueryPyramid((vx_pyramid)ref, VX_PYRAMID_HEIGHT, &ref_height, sizeof(ref_height)));
            VX_CALL(vxQueryPyramid((vx_pyramid)ref, VX_PYRAMID_FORMAT, &ref_format, sizeof(ref_format)));
            VX_CALL(vxQueryPyramid((vx_pyramid)ref, VX_PYRAMID_LEVELS, &ref_levels, sizeof(ref_levels)));
            VX_CALL(vxQueryPyramid((vx_pyramid)ref, VX_PYRAMID_SCALE, &ref_scale, sizeof(ref_scale)));

            VX_CALL(vxQueryPyramid((vx_pyramid)item, VX_PYRAMID_WIDTH, &item_width, sizeof(item_width)));
            VX_CALL(vxQueryPyramid((vx_pyramid)item, VX_PYRAMID_HEIGHT, &item_height, sizeof(item_height)));
            VX_CALL(vxQueryPyramid((vx_pyramid)item, VX_PYRAMID_FORMAT, &item_format, sizeof(item_format)));
            VX_CALL(vxQueryPyramid((vx_pyramid)item, VX_PYRAMID_LEVELS, &item_levels, sizeof(item_levels)));
            VX_CALL(vxQueryPyramid((vx_pyramid)item, VX_PYRAMID_SCALE, &item_scale, sizeof(item_scale)));

            ASSERT(ref_width == item_width);
            ASSERT(ref_height == item_height);
            ASSERT(ref_format == item_format);
            ASSERT(ref_levels == item_levels);
            ASSERT(ref_scale == item_scale);
        }   break;
        case VX_TYPE_SCALAR:
        {
            vx_enum ref_type, item_type;

            VX_CALL(vxQueryScalar((vx_scalar)ref, VX_SCALAR_TYPE, &ref_type, sizeof(ref_type)));

            VX_CALL(vxQueryScalar((vx_scalar)item, VX_SCALAR_TYPE, &item_type, sizeof(item_type)));

            ASSERT(ref_type == item_type);
        }   break;
        case VX_TYPE_MATRIX:
        {
            vx_enum ref_type, item_type;
            vx_size ref_rows, item_rows;
            vx_size ref_cols, item_cols;

            VX_CALL(vxQueryMatrix((vx_matrix)ref, VX_MATRIX_TYPE, &ref_type, sizeof(ref_type)));
            VX_CALL(vxQueryMatrix((vx_matrix)ref, VX_MATRIX_ROWS, &ref_rows, sizeof(ref_rows)));
            VX_CALL(vxQueryMatrix((vx_matrix)ref, VX_MATRIX_COLUMNS, &ref_cols, sizeof(ref_cols)));

            VX_CALL(vxQueryMatrix((vx_matrix)item, VX_MATRIX_TYPE, &item_type, sizeof(item_type)));
            VX_CALL(vxQueryMatrix((vx_matrix)item, VX_MATRIX_ROWS, &item_rows, sizeof(item_rows)));
            VX_CALL(vxQueryMatrix((vx_matrix)item, VX_MATRIX_COLUMNS, &item_cols, sizeof(item_cols)));

            ASSERT(ref_type == item_type);
            ASSERT(ref_rows == item_rows);
            ASSERT(ref_cols == item_cols);
        }   break;
        case VX_TYPE_DISTRIBUTION:
        {
            vx_size ref_bins, item_bins;
            vx_int32 ref_offset, item_offset;
            vx_uint32 ref_range, item_range;

            VX_CALL(vxQueryDistribution((vx_distribution)ref, VX_DISTRIBUTION_BINS, &ref_bins, sizeof(ref_bins)));
            VX_CALL(vxQueryDistribution((vx_distribution)ref, VX_DISTRIBUTION_OFFSET, &ref_offset, sizeof(ref_offset)));
            VX_CALL(vxQueryDistribution((vx_distribution)ref, VX_DISTRIBUTION_RANGE, &ref_range, sizeof(ref_range)));

            VX_CALL(vxQueryDistribution((vx_distribution)item, VX_DISTRIBUTION_BINS, &item_bins, sizeof(item_bins)));
            VX_CALL(vxQueryDistribution((vx_distribution)item, VX_DISTRIBUTION_OFFSET, &item_offset, sizeof(item_offset)));
            VX_CALL(vxQueryDistribution((vx_distribution)item, VX_DISTRIBUTION_RANGE, &item_range, sizeof(item_range)));

            ASSERT(ref_bins == item_bins);
            ASSERT(ref_offset == item_offset);
            ASSERT(ref_range == item_range);
        }   break;
        case VX_TYPE_REMAP:
        {
            vx_uint32 ref_srcwidth, item_srcwidth;
            vx_uint32 ref_srcheight, item_srcheight;
            vx_uint32 ref_dstwidth, item_dstwidth;
            vx_uint32 ref_dstheight, item_dstheight;

            VX_CALL(vxQueryRemap((vx_remap)ref, VX_REMAP_SOURCE_WIDTH, &ref_srcwidth, sizeof(ref_srcwidth)));
            VX_CALL(vxQueryRemap((vx_remap)ref, VX_REMAP_SOURCE_HEIGHT, &ref_srcheight, sizeof(ref_srcheight)));
            VX_CALL(vxQueryRemap((vx_remap)ref, VX_REMAP_DESTINATION_WIDTH, &ref_dstwidth, sizeof(ref_dstwidth)));
            VX_CALL(vxQueryRemap((vx_remap)ref, VX_REMAP_DESTINATION_HEIGHT, &ref_dstheight, sizeof(ref_dstheight)));

            VX_CALL(vxQueryRemap((vx_remap)item, VX_REMAP_SOURCE_WIDTH, &item_srcwidth, sizeof(item_srcwidth)));
            VX_CALL(vxQueryRemap((vx_remap)item, VX_REMAP_SOURCE_HEIGHT, &item_srcheight, sizeof(item_srcheight)));
            VX_CALL(vxQueryRemap((vx_remap)item, VX_REMAP_DESTINATION_WIDTH, &item_dstwidth, sizeof(item_dstwidth)));
            VX_CALL(vxQueryRemap((vx_remap)item, VX_REMAP_DESTINATION_HEIGHT, &item_dstheight, sizeof(item_dstheight)));

            ASSERT(ref_srcwidth == item_srcwidth);
            ASSERT(ref_srcheight == item_srcheight);
            ASSERT(ref_dstwidth == item_dstwidth);
            ASSERT(ref_dstheight == item_dstheight);
        }   break;
        case VX_TYPE_LUT:
        {
            vx_enum ref_type, item_type;
            vx_size ref_count, item_count;

            VX_CALL(vxQueryLUT((vx_lut)ref, VX_LUT_TYPE, &ref_type, sizeof(ref_type)));
            VX_CALL(vxQueryLUT((vx_lut)ref, VX_LUT_COUNT, &ref_count, sizeof(ref_count)));

            VX_CALL(vxQueryLUT((vx_lut)item, VX_LUT_TYPE, &item_type, sizeof(item_type)));
            VX_CALL(vxQueryLUT((vx_lut)item, VX_LUT_COUNT, &item_count, sizeof(item_count)));

            ASSERT(ref_type == item_type);
            ASSERT(ref_count == item_count);
        }   break;
        case VX_TYPE_THRESHOLD:
        {
            vx_enum ref_type, item_type;

            VX_CALL(vxQueryThreshold((vx_threshold)ref, VX_THRESHOLD_TYPE, &ref_type, sizeof(ref_type)));

            VX_CALL(vxQueryThreshold((vx_threshold)item, VX_THRESHOLD_TYPE, &item_type, sizeof(item_type)));

            ASSERT(ref_type == item_type);
        }   break;
        default:
            ASSERT(0 == 1);
    }
}

#define OBJECT_ARRAY_NUM_ITEMS 10

#define ADD_VX_OBJECT_ARRAY_TYPES(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_IMAGE", __VA_ARGS__, VX_TYPE_IMAGE)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_ARRAY", __VA_ARGS__, VX_TYPE_ARRAY)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_PYRAMID", __VA_ARGS__, VX_TYPE_PYRAMID)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_SCALAR", __VA_ARGS__, VX_TYPE_SCALAR)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_MATRIX", __VA_ARGS__, VX_TYPE_MATRIX)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_DISTRIBUTION", __VA_ARGS__, VX_TYPE_DISTRIBUTION)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_REMAP", __VA_ARGS__, VX_TYPE_REMAP)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_LUT", __VA_ARGS__, VX_TYPE_LUT)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_THRESHOLD", __VA_ARGS__, VX_TYPE_THRESHOLD ))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("object_array", ADD_VX_OBJECT_ARRAY_TYPES, ARG, NULL)

TEST_WITH_ARG(ObjectArray, test_vxCreateObjectArray, Obj_Array_Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;

    vx_reference exemplar = NULL;
    vx_size num_items = OBJECT_ARRAY_NUM_ITEMS;
    vx_enum item_type = arg_->item_type;

    vx_object_array object_array = 0;

    vx_reference actual_item = NULL;
    vx_enum actual_type = VX_TYPE_INVALID;
    vx_size actual_num_items = 0;

    vx_uint32 i;

    ASSERT_VX_OBJECT(exemplar = own_create_exemplar(context, item_type), (enum vx_type_e)item_type);

    /* 1. check if object array can be created with allowed types*/
    ASSERT_VX_OBJECT(object_array = vxCreateObjectArray(context, exemplar, num_items), VX_TYPE_OBJECT_ARRAY);

    /* 2. check if object array's actual item_type corresponds to requested item_type */
    VX_CALL(vxQueryObjectArray(object_array, VX_OBJECT_ARRAY_ITEMTYPE, &actual_type, sizeof(actual_type)));
    ASSERT_EQ_INT(item_type, actual_type);

    /* 3. check if object array's actual item_size corresponds to requested item_type size */
    VX_CALL(vxQueryObjectArray(object_array, VX_OBJECT_ARRAY_NUMITEMS, &actual_num_items, sizeof(actual_num_items)));
    ASSERT_EQ_INT(num_items, actual_num_items);

    /* 4. check meta formats of objects in object array */
    for (i = 0u; i < num_items; i++)
    {
        ASSERT_VX_OBJECT(actual_item = vxGetObjectArrayItem(object_array, i), (enum vx_type_e)item_type);

        ASSERT_NO_FAILURE(own_check_meta(actual_item, exemplar));

        VX_CALL(vxReleaseReference(&actual_item));
        ASSERT(actual_item == 0);
    }

    /* 5. check that we can't get item out of object array's range */
    actual_item = vxGetObjectArrayItem(object_array, (vx_uint32)num_items);
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)actual_item));

    VX_CALL(vxReleaseReference(&exemplar));
    ASSERT(exemplar == 0);

    VX_CALL(vxReleaseObjectArray(&object_array));
    ASSERT(object_array == 0);
}

TESTCASE_TESTS(
    ObjectArray,
    test_vxCreateObjectArray
)

