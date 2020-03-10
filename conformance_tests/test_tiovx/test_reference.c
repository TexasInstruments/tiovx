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

TESTCASE_TESTS(tivxReference,
        testQueryTimestamp
)
