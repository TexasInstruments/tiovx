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

#include <TI/tivx_test_kernels.h>
#include <TI/tivx_config.h>
#include <TI/tivx_capture.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include <string.h>
#include <math.h>

#include <vx_internal.h>

#include "shared_functions.h"


TESTCASE(tivxInternalError, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalError, negativeTestCreateConstErrors)
{
    vx_context context = context_->vx_context_;
    tivx_mutex lock1;

    lock1 = context->lock;
    context->lock=NULL;
    vx_bool ret;
    ret = ownCreateConstErrors(context);
    ASSERT(ret == 0);
    context->lock = lock1;
}

TEST(tivxInternalError, negativeTestContextReftable)
{
    vx_context context = context_->vx_context_;
    vx_reference ref, ref1;
    vx_image image;
    vx_reference reft = context->reftable[0];
    vx_reference reft1 = context->reftable[1];
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    context->reftable[0] = NULL;
    context->reftable[1] = (vx_reference)image;
    ref = (vx_reference)ownGetErrorObject(
                        context, (vx_status)VX_ERROR_NO_RESOURCES);
    ref1 = (vx_reference)ownGetErrorObject(
                        context, (vx_status)VX_ERROR_REFERENCE_NONZERO);
    context->reftable[0]=reft;
    context->reftable[1]=reft1;
    VX_CALL(vxReleaseImage(&image));
    EXPECT(ref != NULL);
    EXPECT(ref1 == NULL);
}

TESTCASE_TESTS(tivxInternalError,
    negativeTestCreateConstErrors,
    negativeTestContextReftable
    )