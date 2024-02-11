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
#include <vx_internal.h>
#include <tivx_objects.h>

#include "test_tiovx.h"
#include "test_engine/test.h"
#include "test_tiovx/test_tiovx.h"
#include <VX/vx.h>
#include <VX/vxu.h>

TESTCASE(tivxInternalRawImage, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalRawImage, negativeTestUnmapRawImagePatch)
{
    vx_context context = context_->vx_context_;
    tivx_raw_image raw_image;
    vx_map_id map_id;
    vx_imagepatch_addressing_t user_addr;
    void **user_ptr;
    vx_rectangle_t rect;
    tivx_raw_image_create_params_t params;
    uint8_t *map_addr;
    vx_size  map_size;

    rect.start_x = 16;
    rect.start_y = 19;
    rect.end_x = 16 + 16;
    rect.end_y = 19 + 21;

    params.width = 128;
    params.height = 128;
    params.num_exposures = 3;
    params.line_interleaved = vx_false_e;
    params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    params.format[0].msb = 12;
    params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    params.format[1].msb = 7;
    params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    params.format[2].msb = 11;
    params.meta_height_before = 5;
    params.meta_height_after = 2;

    ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(context, &params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxMapRawImagePatch(raw_image, &rect, 0, &map_id, &user_addr, (void **)&user_ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, TIVX_RAW_IMAGE_ALLOC_BUFFER));

    map_addr = raw_image->maps[0].map_addr;
    map_size = raw_image->maps[0].map_size;
    raw_image->maps[0].map_addr=NULL;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxUnmapRawImagePatch(raw_image, map_id));
    raw_image->maps[0].map_addr = map_addr;
    raw_image->maps[0].map_size=0;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxUnmapRawImagePatch(raw_image, map_id));
    raw_image->maps[0].map_size = map_size;
    raw_image->base.is_virtual = vx_true_e;
    raw_image->base.is_accessible = vx_false_e;
    ASSERT_EQ_VX_STATUS(VX_ERROR_OPTIMIZED_AWAY, tivxUnmapRawImagePatch(raw_image, map_id));
    raw_image->base.is_accessible = vx_true_e;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxUnmapRawImagePatch(raw_image, map_id));
    VX_CALL(tivxReleaseRawImage(&raw_image));
}

TESTCASE_TESTS(
    tivxInternalRawImage,
    negativeTestUnmapRawImagePatch
)

