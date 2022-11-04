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
 * Copyright (c) 2022 Texas Instruments Incorporated
 */

#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx_config.h>

#include "test_engine/test.h"

TESTCASE(tivxMetaFormat, CT_VXContext, ct_setup_vx_context, 0)

static vx_status VX_CALLBACK own_set_image_valid_rect(
    vx_node node,
    vx_uint32 index,
    const vx_rectangle_t* const input_valid[],
    vx_rectangle_t* const output_valid[])
{
    return VX_SUCCESS;
}

TEST(tivxMetaFormat, negativeTestSetMetaFormatAttribute)
{
    vx_context context = context_->vx_context_;

    vx_meta_format meta = NULL;
    vx_enum attribute = VX_VALID_RECT_CALLBACK;
    vx_kernel_image_valid_rectangle_f callback = &own_set_image_valid_rect;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetMetaFormatAttribute(meta, attribute, &callback, sizeof(callback)));
}

TEST(tivxMetaFormat, negativeTestSetMetaFormatFromReference)
{
    vx_context context = context_->vx_context_;

    vx_meta_format meta = NULL;
    vx_reference exemplar = NULL;

    ASSERT_EQ_VX_STATUS(VX_FAILURE, vxSetMetaFormatFromReference(meta, exemplar));
}

TESTCASE_TESTS(
    tivxMetaFormat,
    negativeTestSetMetaFormatAttribute,
    negativeTestSetMetaFormatFromReference
)

