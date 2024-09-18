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

/* The below header is used for getting posix enums: TIVX_MUTEX_MAX_OBJECTS.*/
#if !defined(R5F)
#include <os/posix/tivx_platform_posix.h>
#endif /* Not R5F */

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

TEST(tivxInternalReference, negativeTestOwnDeleteReferenceLock)
{
    vx_context context = context_->vx_context_;
    vx_reference ref;
    tivx_mutex mutex;

    ref = (vx_reference)context;
    mutex = ref->lock;
    ref->lock = NULL;

    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownDeleteReferenceLock(ref));
    ref->lock = mutex;
}

TEST(tivxInternalReference, negativeTestOwnAllocReferenceBufferGeneric)
{
    vx_context context = context_->vx_context_;
    tivx_obj_desc_t *base_obj_desc = NULL;
    vx_reference ref;

    ref = ownCreateReference(context, (vx_enum)VX_TYPE_ARRAY, (vx_enum)VX_EXTERNAL, &context->base);
    base_obj_desc = ref->obj_desc;
    ref->obj_desc = NULL;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownAllocReferenceBufferGeneric(ref));
    ref->obj_desc = base_obj_desc;

    VX_CALL(ownReleaseReferenceInt(&ref, (vx_enum)VX_TYPE_ARRAY, (vx_enum)VX_EXTERNAL, NULL));
}
typedef struct
{
    const char *name;
    vx_enum type;
} Arg;

/* To hit 'obj_desc == NULL' condition for all ref types inside tivxReferenceImportHandle() & tivxReferenceExportHandle() */
TEST_WITH_ARG(tivxInternalReference, negativetestReferenceImportExportHandle, Arg,
              ARG_ENUM(VX_TYPE_IMAGE),
              ARG_ENUM(VX_TYPE_TENSOR),
              ARG_ENUM(VX_TYPE_USER_DATA_OBJECT),
              ARG_ENUM(VX_TYPE_ARRAY),
              ARG_ENUM(VX_TYPE_CONVOLUTION),
              ARG_ENUM(VX_TYPE_MATRIX),
              ARG_ENUM(VX_TYPE_DISTRIBUTION),
              ARG_ENUM(TIVX_TYPE_RAW_IMAGE),
              ARG_ENUM(VX_TYPE_PYRAMID))
{

    vx_context context = context_->vx_context_;
    tivx_obj_desc_t *base_obj_desc = NULL;
    vx_reference ref;
    void *addr[1] = {NULL};
    uint32_t size[1];
    uint32_t num_entries;

    ref = ownCreateReference(context, (vx_enum)arg_->type, (vx_enum)VX_EXTERNAL, &context->base);
    base_obj_desc = ref->obj_desc;
    ref->obj_desc = NULL;

    ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxReferenceImportHandle(ref,(const void **)addr, (const uint32_t *)size,1));
    ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxReferenceExportHandle(ref,(void **)addr, (uint32_t *)size, 1, &num_entries));
    ref->obj_desc = base_obj_desc;

    VX_CALL(ownReleaseReferenceInt(&ref, (vx_enum)arg_->type, (vx_enum)VX_EXTERNAL, NULL));
}

/* The Below test is defined for POSIX only, as it is getting the
max value and/or alloc/free functions from POSIX headers */
#if !defined(R5F) /* Not R5F */
TEST(tivxInternalReference, negativeTestOwnCreateReferenceLock)
{
    vx_context context = context_->vx_context_;
    vx_reference ref = (vx_reference)context;
    tivx_mutex mutex[TIVX_MUTEX_MAX_OBJECTS];
    int i,j = 0;
    tivx_mutex mutex1 = ref->lock;

    for (i = 0; i < TIVX_MUTEX_MAX_OBJECTS; i++)
    {
        if((vx_status)VX_SUCCESS != tivxMutexCreate(&mutex[i]))
        {
            break;
        }
    }

/* To fail tivxMutexCreate() inside ownCreateReferenceLock API by maxing out */
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_MEMORY, ownCreateReferenceLock(ref));
    j = i;

    for (i = 0; i < j; i++)
    {
        VX_CALL(tivxMutexDelete(&mutex[i]));
    }
    ref->lock = mutex1;
}
#endif

/* To hit 'img_obj_desc == NULL' condition for pyramid ref->type inside tivxReferenceImportHandle() & tivxReferenceExportHandle() */
TEST(tivxInternalReference, negativetestReferenceImportExportHandlePyramid)
{
    vx_context context = context_->vx_context_;

    tivx_obj_desc_t *img_obj_desc = NULL;
    vx_reference ref;
    void *addr[1] = {NULL};
    uint32_t size[1];
    uint32_t num_entries;

    vx_pyramid pymd = NULL;
    vx_image img = NULL;
    vx_size levels = 1;
    vx_float32 scale = 0.9f;
    vx_uint32 width = 3, height = 3;
    vx_df_image format = VX_DF_IMAGE_U8;

    ASSERT_VX_OBJECT(pymd = vxCreatePyramid(context, levels, scale, width, height, format), VX_TYPE_PYRAMID);

    img = pymd->img[0];
    img_obj_desc = img->base.obj_desc;

    img->base.obj_desc = NULL;

    ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxReferenceImportHandle((vx_reference)pymd,(const void **)addr, (const uint32_t *)size,1));
    ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxReferenceExportHandle((vx_reference)pymd,(void **)addr, (uint32_t *)size, 1, &num_entries));
    img->base.obj_desc = img_obj_desc;

    VX_CALL(vxReleasePyramid(&pymd));
}

TESTCASE_TESTS(tivxInternalReference,
               negativeTestOwnDestructReferenceGeneric,
               negativeTestOwnIsValidReference,
               negativeTestNullReference,
               negativeTestOwnReleaseReferenceInt,
               negativeTestOwnReferenceLock,
               negativeTestReleaseReference,
               negativeTestGetReferenceParent,
               negativeTestOwnDeleteReferenceLock,
               negativeTestOwnAllocReferenceBufferGeneric,
               negativetestReferenceImportExportHandle,
#if !defined(R5F)
               negativeTestOwnCreateReferenceLock,
#endif
               negativetestReferenceImportExportHandlePyramid
)