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
#include <TI/tivx_config.h>

#define TIVX_REFERENCE_NOT_PRESENT 0

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

TEST(tivxReference, testSetInvalid)
{
    vx_context context = context_->vx_context_;
    vx_image image;
    vx_uint64 timestamp;
	vx_bool invalid = vx_true_e;
	vx_bool is_invalid = vx_false_e;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    VX_CALL(tivxSetReferenceAttribute((vx_reference)image, TIVX_REFERENCE_INVALID, &invalid, sizeof(invalid)));

    VX_CALL(vxQueryReference((vx_reference)image, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

    ASSERT(invalid==is_invalid);

    VX_CALL(vxReleaseImage(&image));
}

TEST(tivxReference, testSetValid)
{
    vx_context context = context_->vx_context_;
    vx_image image;
    vx_uint64 timestamp;
	vx_bool invalid = vx_false_e;
	vx_bool is_invalid = vx_true_e;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    VX_CALL(tivxSetReferenceAttribute((vx_reference)image, TIVX_REFERENCE_INVALID, &invalid, sizeof(invalid)));

    VX_CALL(vxQueryReference((vx_reference)image, TIVX_REFERENCE_INVALID, &is_invalid, sizeof(is_invalid)));

    ASSERT(invalid==is_invalid);

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

TEST(tivxReference, negativeTestGetReferenceParent)
{
    vx_reference ref = NULL;
    vx_reference ret;
    
    ret = tivxGetReferenceParent(ref);
    ASSERT(ret == NULL);
}

TEST(tivxReference, negativeTestGetReferenceParent1)
{
    vx_context context = context_->vx_context_;
    vx_uint64 is_invalid, set_is_valid = 10;

    vx_image image;
    vx_reference out_objarr;
    out_objarr = (vx_reference)tivxGetReferenceParent((vx_reference)context);
    ASSERT(out_objarr == NULL);
}

TEST(tivxReference, negativeTestSetRefCount)
{
    vx_reference ref = NULL;
    vx_enum attribute = VX_REFERENCE_DEFAULT;
    vx_uint32 udata = 0;
    vx_size size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, tivxSetReferenceAttribute(ref, VX_REFERENCE_COUNT, &udata, size));
}

TEST(tivxReference, negativeTestSetTimestamp)
{
    vx_reference ref = NULL;
    vx_enum attribute = VX_REFERENCE_DEFAULT;
    vx_uint32 udata = 0;
    vx_size size = 0;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, tivxSetReferenceAttribute(ref, VX_REFERENCE_COUNT, &udata, size));
}

TEST(tivxReference, negativetestSetTimestamp1)
{
    vx_context context = context_->vx_context_;
    vx_image image;
    vx_uint32 udata = 0;
    vx_uint64 udata1 = 0;
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxSetReferenceAttribute((vx_reference)image, TIVX_REFERENCE_TIMESTAMP, &udata, sizeof(udata)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, tivxSetReferenceAttribute((vx_reference)image, TIVX_REFERENCE_NOT_PRESENT, &udata, sizeof(udata)));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxSetReferenceAttribute((vx_reference)image, TIVX_REFERENCE_INVALID, &udata1, sizeof(udata1)));
    VX_CALL(vxReleaseImage(&image));
}

TEST(tivxReference, negativeTestGetStatus1)
{
    vx_context context = context_->vx_context_;
    vx_status status;
    vx_image   src_image;

	ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    VX_CALL(vxReleaseContext(&context));
    status = vxGetStatus((vx_reference)src_image);
    ASSERT(status==VX_FAILURE);
    context = vxCreateContext();
}

TEST(tivxReference, negativeTestGetStatus2)
{
    vx_context context = context_->vx_context_;
    vx_status status;
 
    status = vxGetStatus((vx_reference)context);
    ASSERT(status==VX_SUCCESS);
}

TEST(tivxReference, testGetContext)
{
    vx_context context = context_->vx_context_;
    vx_context new_context;

    ASSERT_VX_OBJECT(new_context = vxGetContext((vx_reference)context), VX_TYPE_CONTEXT);
}

TEST(tivxReference, negativeTestvxReferenceExportHandle)
{
    vx_context context = context_->vx_context_;
    vx_image image;
    vx_uint64 is_invalid, set_is_valid = 10;
    vx_reference ref;
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    VX_CALL(tivxSetReferenceAttribute((vx_reference)image, TIVX_REFERENCE_TIMESTAMP, &set_is_valid, sizeof(set_is_valid)));
    ref = (vx_reference)image;
 
    void *addr[64]; 
    void *nonNullValue = (void *)0x12345678;
    for (int i = 0; i < 64; i++) 
    { addr[i] = nonNullValue; }
    uint32_t size[64] = { 10 }; 
    uint32_t max_entries = 64; 
    uint32_t num_entries = 0;
   
    ASSERT_EQ_VX_STATUS(VX_FAILURE,(tivxReferenceExportHandle((vx_reference)context, addr, size, max_entries, &num_entries)));
    VX_CALL(vxReleaseImage(&image));
}

TEST(tivxReference, negativeTestvxIsReferenceMetaFormatEqual1)
{
    vx_context context = context_->vx_context_;
    vx_image image1 = NULL;
    vx_image image2;
    vx_bool is_equal;

    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 128, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    is_equal = tivxIsReferenceMetaFormatEqual((vx_reference)image1, (vx_reference)image2);
    ASSERT(is_equal==vx_false_e);
    VX_CALL(vxReleaseImage(&image2));
}

TEST(tivxReference, negativeTestvxIsReferenceMetaFormatEqual2)
{
    vx_context context = context_->vx_context_;
    vx_image image1;
    vx_image image2 = NULL;
    vx_bool is_equal;

    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    is_equal = tivxIsReferenceMetaFormatEqual((vx_reference)image1, (vx_reference)image2);
    ASSERT(is_equal==vx_false_e);
    VX_CALL(vxReleaseImage(&image1));
}

TEST(tivxReference, negativeTestvxIsReferenceMetaFormatEqual3)
{
    vx_context context = context_->vx_context_;
    vx_image image;
    vx_bool is_equal;
    vx_uint64 is_invalid, set_is_valid = 10;
    vx_context context1, context2;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    context2 = (vxGetContext((vx_reference)image));
    is_equal = tivxIsReferenceMetaFormatEqual((vx_reference)context, (vx_reference)context2);
    ASSERT(is_equal==vx_false_e);
    VX_CALL(vxReleaseImage(&image));
}

#if 0
TEST(tivxReference, negativeTesttivxReferenceImportHandle)
{
    vx_context context = context_->vx_context_;
    vx_graph   graph;
    vx_node node = 0;
    vx_kernel kernel = 0;
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    vx_context context1 = vxGetContext((vx_reference)graph);
    vx_enum take10_enumId = 0u;
    void *nonNullValue = (void *)0x1234567890AB;
    void *addr[64] = {nonNullValue}; 
    uint32_t size[64] = { 10 }; 
    uint32_t max_entries = 64; 
    uint32_t num_entries = 10;
    vx_enum kernel_id = VX_KERNEL_SOBEL_3x3;

    ASSERT_VX_OBJECT(context1, VX_TYPE_CONTEXT);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context1, kernel_id), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_FAILURE,(tivxReferenceImportHandle((vx_reference)node, (const void **)addr, (const uint32_t *)size, num_entries)));    
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));    
}


TEST(tivxReference, negativeTesttivxReferenceImportHandle1)
{
    vx_context context = context_->vx_context_;
    vx_image image;
    vx_pyramid pyr_in;
    vx_uint64 is_invalid, set_is_valid = 10;
    void *nonNullValue = (void *)0x1234567890AB;
    void *addr[64] = {nonNullValue}; 
    uint32_t size[64] = { 10 }; 
    uint32_t max_entries = 64; 
    uint32_t num_entries = 10;

    ASSERT_VX_OBJECT(pyr_in = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    ASSERT_EQ_VX_STATUS(VX_FAILURE,(tivxReferenceImportHandle((vx_reference)pyr_in, addr, size, &num_entries)));    
    VX_CALL(vxReleasePyramid(&pyr_in));    
} 

TEST(tivxReference, negativeTesttivxReferenceImportHandle2)
{
    vx_context context = context_->vx_context_;
    vx_image image;
    vx_uint64 is_invalid, set_is_valid = 10;
    void *nonNullValue = (void *)0x1234567890AB;
    void *addr[64] = {nonNullValue}; 
    
    uint32_t size[64] = { 10 }; 
    uint32_t max_entries = 64; 
    uint32_t num_entries = 10;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_FAILURE,(tivxReferenceImportHandle((vx_reference)image, (const void **)addr, (const uint32_t *)size, num_entries)));    
    VX_CALL(vxReleaseImage(&image));    
} 
#endif

TESTCASE_TESTS(
    tivxReference,
    testQueryTimestamp,
    testSetTimestamp,
    testSetInvalid,
    testSetValid,
    testQueryInvalidFlag,
    testIsReferenceMetaFormatEqualPass,
    testIsReferenceMetaFormatEqualFail1,
    testIsReferenceMetaFormatEqualFail2,
    negativeTestQueryReference,
    negativeTestSetReferenceName,
    negativeTestReleaseReference,
    negativeTestRetainReference,
    negativeTestHint,
    negativeTestGetReferenceParent,
    negativeTestGetReferenceParent1,
	negativeTestSetRefCount,
    negativeTestSetTimestamp,
    negativetestSetTimestamp1,
    negativeTestGetStatus1,
    negativeTestGetStatus2,    
    negativeTestvxReferenceExportHandle,
    testGetContext,
    negativeTestvxIsReferenceMetaFormatEqual1,
    negativeTestvxIsReferenceMetaFormatEqual2,
    negativeTestvxIsReferenceMetaFormatEqual3,
    negativeTesttivxReferenceImportHandle,
    negativeTesttivxReferenceImportHandle1,
    negativeTesttivxReferenceImportHandle2
)

