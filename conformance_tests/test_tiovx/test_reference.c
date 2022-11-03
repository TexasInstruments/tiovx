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


#include "test_tiovx.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include "shared_functions.h"

TESTCASE(tivxReference, CT_VXContext, ct_setup_vx_context, 0)


TEST(tivxReference, testQueryTimestamp)
{
    vx_context context = context_->vx_context_;
    vx_image image;
    vx_uint64 timestamp;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    VX_CALL(vxQueryReference((vx_reference)image, TIVX_REFERENCE_TIMESTAMP, &timestamp, sizeof(timestamp)));

    ASSERT(timestamp==0);

    VX_CALL(vxReleaseImage(&image));
}

TEST(tivxReference, testSetTimestamp)
{
    vx_context context = context_->vx_context_;
    vx_image image;
    vx_uint64 timestamp, set_timestamp = 10;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    VX_CALL(tivxSetReferenceAttribute((vx_reference)image, TIVX_REFERENCE_TIMESTAMP, &set_timestamp, sizeof(set_timestamp)));

    VX_CALL(vxQueryReference((vx_reference)image, TIVX_REFERENCE_TIMESTAMP, &timestamp, sizeof(timestamp)));

    ASSERT(timestamp==set_timestamp);

    VX_CALL(vxReleaseImage(&image));
}

TEST(tivxReference, testQueryInvalidFlag)
{
    vx_context context = context_->vx_context_;
    vx_image image;
    vx_bool is_invalid;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    VX_CALL(vxQueryReference((vx_reference)image, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

    ASSERT(is_invalid==vx_false_e);

    VX_CALL(vxReleaseImage(&image));
}

TEST(tivxReference, testIsReferenceMetaFormatEqualPass)
{
    vx_context context = context_->vx_context_;
    vx_image image1;
    vx_image image2;
    vx_bool is_equal;

    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    is_equal = tivxIsReferenceMetaFormatEqual((vx_reference)image1, (vx_reference)image2);

    ASSERT(is_equal==vx_true_e);

    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
}

TEST(tivxReference, testIsReferenceMetaFormatEqualFail1)
{
    vx_context context = context_->vx_context_;
    vx_image image1;
    vx_image image2;
    vx_bool is_equal;

    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 128, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    is_equal = tivxIsReferenceMetaFormatEqual((vx_reference)image1, (vx_reference)image2);

    ASSERT(is_equal==vx_false_e);

    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
}

TEST(tivxReference, testIsReferenceMetaFormatEqualFail2)
{
    vx_context context = context_->vx_context_;
    vx_image image;
    vx_user_data_object user_data_object = 0;
    vx_bool is_equal;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(user_data_object = vxCreateUserDataObject(context, NULL, 100, NULL),
                     (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    is_equal = tivxIsReferenceMetaFormatEqual((vx_reference)image, (vx_reference)user_data_object);

    ASSERT(is_equal==vx_false_e);

    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseUserDataObject(&user_data_object));
}

TEST(tivxReference, negativeTestQueryReference)
{
    #define VX_REFERENCE_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_reference ref = NULL;
    vx_enum attribute = VX_REFERENCE_DEFAULT;
    vx_uint32 udata = 0;
    vx_size size = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ref = (vx_reference)(graph);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryReference(ref, VX_REFERENCE_COUNT, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryReference(ref, VX_REFERENCE_TYPE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryReference(ref, VX_REFERENCE_NAME, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryReference(ref, TIVX_REFERENCE_TIMESTAMP, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryReference(ref, TIVX_REFERENCE_INVALID, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryReference(ref, attribute, &udata, size));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxReference, negativeTestSetReferenceName)
{
    vx_context context = context_->vx_context_;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetReferenceName(NULL, NULL));
}

TEST(tivxReference, negativeTestReleaseReference)
{
    vx_context context = context_->vx_context_;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxReleaseReference(NULL));
}

TEST(tivxReference, negativeTestRetainReference)
{
    vx_context context = context_->vx_context_;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxRetainReference(NULL));
}

TEST(tivxReference, negativeTestHint)
{
    vx_context context = context_->vx_context_;

    vx_reference ref = NULL;
    vx_enum hint = 0;
    vx_uint32 udata = 0;
    vx_size size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxHint(ref, hint, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxHint((vx_reference)(context), hint, &udata, size));
}

TESTCASE_TESTS(
    tivxReference,
    testQueryTimestamp,
    testSetTimestamp,
    testQueryInvalidFlag,
    testIsReferenceMetaFormatEqualPass,
    testIsReferenceMetaFormatEqualFail1,
    testIsReferenceMetaFormatEqualFail2,
    negativeTestQueryReference,
    negativeTestSetReferenceName,
    negativeTestReleaseReference,
    negativeTestRetainReference,
    negativeTestHint
)

