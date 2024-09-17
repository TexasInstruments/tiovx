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

#include <vx_internal.h>

#include "shared_functions.h"
#include "test_utils_mem_operations.h"


TESTCASE(tivxInternalReference, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalReference, negativeTestOwnDestructReferenceGeneric)
{
    vx_context context = context_->vx_context_;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownDestructReferenceGeneric((vx_reference)context));
}

TEST(tivxInternalReference, negativeTestOwnIsValidReference)
{
    vx_context context = context_->vx_context_;
    vx_image image = NULL;
    vx_uint32 width = 16, height = 9;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_enum ref_type = ((vx_reference)image)->type;
    ((vx_reference)image)->type = VX_TYPE_INVALID;
    ASSERT((vx_bool)vx_false_e == ownIsValidReference((vx_reference)image));
    ((vx_reference)image)->type = ref_type;

    vx_context base_context = context->base.context;
    context->base.context = (vx_context)(vx_reference)image;
    ASSERT((vx_bool)vx_false_e == ownIsValidReference((vx_reference)context));
    context->base.context = base_context;

    VX_CALL(vxReleaseImage(&image));
}
TEST(tivxInternalReference, negativeTestNullReference)
{
    vx_context context = context_->vx_context_;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownCreateReferenceLock(NULL));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownDeleteReferenceLock(NULL));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownInitReference(NULL, NULL, 0, NULL));
    ASSERT((vx_enum)TIVX_OBJ_DESC_INVALID == ownReferenceGetObjDescId(NULL));
}

TEST(tivxInternalReference, negativeTestOwnReleaseReferenceInt)
{
    vx_context context = context_->vx_context_;
    vx_convolution conv;
    vx_size rows = 3, cols = 3;

    ASSERT_VX_OBJECT(conv = vxCreateConvolution(context, cols, rows), VX_TYPE_CONVOLUTION);

    /* The Convolution is release without calling vxReleaseConvolution() by using the ownReleaseReferenceInt() function directly.
    vxReleaseConvolution() need not be called again */
    ownReleaseReferenceInt((vx_reference*)&conv, (vx_enum)VX_TYPE_CONVOLUTION, (vx_enum)VX_EXTERNAL, ((vx_reference)conv)->destructor_callback);
}

TEST(tivxInternalReference, negativeTestOwnReferenceLock)
{
    vx_context context = context_->vx_context_;
    vx_image image = NULL;
    vx_uint32 width = 16, height = 9;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    vx_context ref_context = ((vx_reference)image)->context;
    ((vx_reference)image)->context = NULL;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownReferenceLock((vx_reference)image));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownReferenceUnlock((vx_reference)image));

    ((vx_reference)image)->context = ref_context;

    VX_CALL(vxReleaseImage(&image));

}

TEST(tivxInternalReference, negativeTestReleaseReference)
{
    vx_context context = context_->vx_context_;
    vx_image image = NULL;
    vx_uint32 width = 16, height = 9;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    tivx_reference_release_callback_f ref_callback = ((vx_reference)image)->release_callback;

    ((vx_reference)image)->release_callback = NULL;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxReleaseReference((vx_reference*)&image));
    ((vx_reference)image)->release_callback = ref_callback;
    VX_CALL(vxReleaseImage(&image));
}

TEST(tivxInternalReference, negativeTestGetReferenceParent)
{
    vx_context context = context_->vx_context_;
    vx_image image = NULL;
    vx_uint32 width = 16, height = 9;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT(NULL == tivxGetReferenceParent((vx_reference) image));

    VX_CALL(vxReleaseImage(&image));
}

TESTCASE_TESTS(tivxInternalReference,
               negativeTestOwnDestructReferenceGeneric,
               negativeTestOwnIsValidReference,
               negativeTestNullReference,
               negativeTestOwnReleaseReferenceInt,
               negativeTestOwnReferenceLock,
               negativeTestReleaseReference,
               negativeTestGetReferenceParent
)