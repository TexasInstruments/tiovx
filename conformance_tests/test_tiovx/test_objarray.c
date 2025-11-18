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
/*
 * Copyright (c) 2022-2025 Texas Instruments Incorporated
 */

#include "test_engine/test.h"
#include <tivx.h>

TESTCASE(tivxObjArray, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxObjArray, testCreateObjectArrayFromExemplar)
{
    vx_context context = context_->vx_context_;

    vx_reference object_array = NULL, object_array_item = NULL, exemplar_object_array_item = NULL;
    vx_object_array exemplar_object_array = 0;
    vx_image exemplar_image = 0;
    vx_size count = 10, object_array_size, exemplar_object_array_size;
    vx_enum object_array_type, exemplar_object_array_type;
    vx_uint32 i, ref_width, exemplar_width, ref_height, exemplar_height;
    vx_df_image ref_format, exemplar_format;

    /* Create object array from object array exemplar */
    ASSERT_VX_OBJECT(exemplar_image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(exemplar_object_array = vxCreateObjectArray(context, (vx_reference)exemplar_image, count), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(object_array = tivxCreateReferenceFromExemplar(context, (vx_reference)exemplar_object_array), VX_TYPE_OBJECT_ARRAY);

    /* Verify item types match between exemplar and object array created from exemplar */
    VX_CALL(vxQueryObjectArray(exemplar_object_array, VX_OBJECT_ARRAY_ITEMTYPE, &exemplar_object_array_type, sizeof(exemplar_object_array_type)));
    VX_CALL(vxQueryObjectArray((vx_object_array)object_array, VX_OBJECT_ARRAY_ITEMTYPE, &object_array_type, sizeof(object_array_type)));
    ASSERT_EQ_INT(exemplar_object_array_type, object_array_type);

    /* Verify item sizes match between exemplar and object array created from exemplar*/
    VX_CALL(vxQueryObjectArray(exemplar_object_array, VX_OBJECT_ARRAY_NUMITEMS, &exemplar_object_array_size, sizeof(exemplar_object_array_size)));
    VX_CALL(vxQueryObjectArray((vx_object_array)object_array, VX_OBJECT_ARRAY_NUMITEMS, &object_array_size, sizeof(object_array_size)));
    ASSERT_EQ_INT(exemplar_object_array_size, object_array_size);

    /* Verify meta format values match between exemplar and object array created from exemplar */
    for (i = 0U; i < count; i++)
    {
        ASSERT_VX_OBJECT(exemplar_object_array_item = vxGetObjectArrayItem(exemplar_object_array, i), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(object_array_item = vxGetObjectArrayItem((vx_object_array)object_array, i), VX_TYPE_IMAGE);

        VX_CALL(vxQueryImage((vx_image)exemplar_object_array_item, VX_IMAGE_WIDTH, &exemplar_width, sizeof(exemplar_width)));
        VX_CALL(vxQueryImage((vx_image)exemplar_object_array_item, VX_IMAGE_HEIGHT, &exemplar_height, sizeof(exemplar_height)));
        VX_CALL(vxQueryImage((vx_image)exemplar_object_array_item, VX_IMAGE_FORMAT, &exemplar_format, sizeof(exemplar_format)));

        VX_CALL(vxQueryImage((vx_image)object_array_item, VX_IMAGE_WIDTH, &ref_width, sizeof(ref_width)));
        VX_CALL(vxQueryImage((vx_image)object_array_item, VX_IMAGE_HEIGHT, &ref_height, sizeof(ref_height)));
        VX_CALL(vxQueryImage((vx_image)object_array_item, VX_IMAGE_FORMAT, &ref_format, sizeof(ref_format)));

        ASSERT(exemplar_width == ref_width);
        ASSERT(exemplar_height == ref_height);
        ASSERT(exemplar_format == ref_format);

        VX_CALL(vxReleaseReference(&exemplar_object_array_item));
        VX_CALL(vxReleaseReference(&object_array_item));
    }

    VX_CALL(vxReleaseReference(&object_array));
    VX_CALL(vxReleaseObjectArray(&exemplar_object_array));
    VX_CALL(vxReleaseImage(&exemplar_image));
}

// Testing Nested Object Array
// Variable convention:
// (nested_oa) -> (nested_oa_child) -> (child_item)
//   Obj Arr         Obj Arr Item          Image
TEST(tivxObjArray, testCreateNestedObjectArrayFromExemplar)
{
    vx_context context = context_->vx_context_;

    vx_reference object_array = NULL, child_object_array_item = NULL, exemplar_object_array_item = NULL, nested_object_array_child = NULL;
    vx_object_array exemplar_oafl = 0, nested_oa = 0, exemplar_object_array = 0;
    vx_image exemplar_image = 0;
    vx_size count = 10, child_object_array_size, exemplar_object_array_size;
    vx_enum child_object_array_type, exemplar_object_array_type;
    vx_uint32 i, j, ref_width, exemplar_width, ref_height, exemplar_height;
    vx_df_image ref_format, exemplar_format;

    /* Create object array from object array made from exemplar */
    ASSERT_VX_OBJECT(exemplar_image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(exemplar_object_array = vxCreateObjectArray(context, (vx_reference)exemplar_image, count), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(nested_oa = vxCreateObjectArray(context, (vx_reference)exemplar_object_array, count), VX_TYPE_OBJECT_ARRAY);

    /* Verify meta format values match between exemplar and object array created from exemplar */
    for (i = 0U; i < count; i++)
    {
        ASSERT_VX_OBJECT(nested_object_array_child = vxGetObjectArrayItem(nested_oa, i), VX_TYPE_OBJECT_ARRAY);

        /* Verify item types match between exemplar and object array created from exemplar */
        VX_CALL(vxQueryObjectArray(exemplar_object_array, VX_OBJECT_ARRAY_ITEMTYPE, &exemplar_object_array_type, sizeof(exemplar_object_array_type)));
        VX_CALL(vxQueryObjectArray((vx_object_array)nested_object_array_child, VX_OBJECT_ARRAY_ITEMTYPE, &child_object_array_type, sizeof(child_object_array_type)));
        ASSERT_EQ_INT(exemplar_object_array_type, child_object_array_type);

        /* Verify item sizes match between exemplar and object array created from exemplar*/
        VX_CALL(vxQueryObjectArray(exemplar_object_array, VX_OBJECT_ARRAY_NUMITEMS, &exemplar_object_array_size, sizeof(exemplar_object_array_size)));
        VX_CALL(vxQueryObjectArray((vx_object_array)nested_object_array_child, VX_OBJECT_ARRAY_NUMITEMS, &child_object_array_size, sizeof(child_object_array_size)));
        ASSERT_EQ_INT(exemplar_object_array_size, child_object_array_size);

        for (j = 0U; j < count; j++)
        {
            ASSERT_VX_OBJECT(child_object_array_item = vxGetObjectArrayItem((vx_object_array)nested_object_array_child, j), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(exemplar_object_array_item = vxGetObjectArrayItem(exemplar_object_array, j), VX_TYPE_IMAGE);

            VX_CALL(vxQueryImage((vx_image)exemplar_object_array_item, VX_IMAGE_WIDTH, &exemplar_width, sizeof(exemplar_width)));
            VX_CALL(vxQueryImage((vx_image)exemplar_object_array_item, VX_IMAGE_HEIGHT, &exemplar_height, sizeof(exemplar_height)));
            VX_CALL(vxQueryImage((vx_image)exemplar_object_array_item, VX_IMAGE_FORMAT, &exemplar_format, sizeof(exemplar_format)));

            VX_CALL(vxQueryImage((vx_image)child_object_array_item, VX_IMAGE_WIDTH, &ref_width, sizeof(ref_width)));
            VX_CALL(vxQueryImage((vx_image)child_object_array_item, VX_IMAGE_HEIGHT, &ref_height, sizeof(ref_height)));
            VX_CALL(vxQueryImage((vx_image)child_object_array_item, VX_IMAGE_FORMAT, &ref_format, sizeof(ref_format)));

            ASSERT(exemplar_width == ref_width);
            ASSERT(exemplar_height == ref_height);
            ASSERT(exemplar_format == ref_format);

            VX_CALL(vxReleaseReference(&exemplar_object_array_item));
            VX_CALL(vxReleaseReference(&child_object_array_item));
        }
        VX_CALL(vxReleaseReference(&nested_object_array_child));
    }

    VX_CALL(vxReleaseObjectArray(&nested_oa));
    VX_CALL(vxReleaseObjectArray(&exemplar_object_array));
    VX_CALL(vxReleaseImage(&exemplar_image));
}

TEST(tivxObjArray, negativeTestCreateObjectArray)
{
    vx_context context = context_->vx_context_;

    vx_object_array vxoa = NULL, nested_vxoa = NULL, double_nested_vxoa = NULL;
    vx_convolution cnvl = NULL;
    vx_reference exemplar = NULL;
    vx_size count = 5, columns = 5, rows = 5;

    // Test: NULL context
    ASSERT(NULL == (vxoa = vxCreateObjectArray(NULL, exemplar, count)));

    // Test: NULL Exemplar
    EXPECT_VX_ERROR(vxoa = vxCreateObjectArray(context, exemplar, count), VX_ERROR_INVALID_PARAMETERS);

    // Test: Invalid Type
    ASSERT_VX_OBJECT(cnvl = vxCreateConvolution(context, columns, rows), VX_TYPE_CONVOLUTION);
    exemplar = (vx_reference)(cnvl);
    EXPECT_VX_ERROR(vxoa = vxCreateObjectArray(context, exemplar, count), VX_ERROR_INVALID_TYPE);
    // Test: Count = 0 (after Type: Test because it's checked second) 
    EXPECT_VX_ERROR(vxoa = vxCreateObjectArray(context, exemplar, 0), VX_ERROR_INVALID_PARAMETERS);
    VX_CALL(vxReleaseConvolution(&cnvl));

    // Test: creation of two level nested object array (not supported)
    ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(vxoa = vxCreateObjectArray(context, (vx_reference)exemplar, count), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(nested_vxoa = vxCreateObjectArray(context, (vx_reference)vxoa, count), VX_TYPE_OBJECT_ARRAY);
    EXPECT_VX_ERROR(double_nested_vxoa = vxCreateObjectArray(context, (vx_reference)nested_vxoa, count), VX_ERROR_INVALID_TYPE);
    VX_CALL(vxReleaseObjectArray(&vxoa));
    VX_CALL(vxReleaseObjectArray(&nested_vxoa));
    VX_CALL(vxReleaseReference(&exemplar));
}

TEST(tivxObjArray, negativeTestCreateVirtualObjectArray)
{
    vx_context context = context_->vx_context_;

    vx_object_array vxoa = NULL;
    vx_graph graph = NULL;
    vx_reference exemplar = NULL;
    vx_size count = 0;

    ASSERT(NULL == (vxoa = vxCreateVirtualObjectArray(graph, exemplar, count)));
}

TEST(tivxObjArray, negativeTestGetObjectArrayItem)
{
    vx_context context = context_->vx_context_;

    vx_object_array vxoa = NULL;
    vx_reference ref = NULL;
    vx_uint32 index = 0;

    ASSERT(NULL != (ref = vxGetObjectArrayItem(vxoa, index)));
}

TEST(tivxObjArray, negativeTestQueryObjectArray)
{
    #define VX_OBJECT_ARRAY_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_object_array vxoa = NULL;
    vx_image img = NULL;
    vx_enum attribute = VX_OBJECT_ARRAY_DEFAULT;
    vx_uint32 udata = 0;
    vx_size size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryObjectArray(vxoa, VX_OBJECT_ARRAY_ITEMTYPE, &udata, size));
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(vxoa = vxCreateObjectArray(context, (vx_reference)(img), 1), VX_TYPE_OBJECT_ARRAY);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryObjectArray(vxoa, VX_OBJECT_ARRAY_ITEMTYPE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryObjectArray(vxoa, VX_OBJECT_ARRAY_NUMITEMS, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryObjectArray(vxoa, attribute, &udata, size));
    VX_CALL(vxReleaseObjectArray(&vxoa));
    VX_CALL(vxReleaseImage(&img));
}

TESTCASE_TESTS(
    tivxObjArray,
    testCreateObjectArrayFromExemplar,
    testCreateNestedObjectArrayFromExemplar,
    negativeTestCreateObjectArray,
    negativeTestCreateVirtualObjectArray,
    negativeTestGetObjectArrayItem,
    negativeTestQueryObjectArray
)
