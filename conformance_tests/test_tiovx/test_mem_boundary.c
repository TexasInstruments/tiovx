/*
 *
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

#include <math.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include "test_utils_mem_operations.h"

#include "test_engine/test.h"

TESTCASE(tivxMemBoundary, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxMemBoundary, negativeTestMemBufferAllocBoundary)
{
    vx_context context = context_->vx_context_;
    tivx_shared_mem_info_t *tivx_shared_mem_info_array;
    uint32_t num_chunks;
    vx_enum mheap_region = TIVX_MEM_EXTERNAL;
    vx_status status = VX_SUCCESS;

    vx_distribution dist = NULL;
    int32_t udata[256];
    vx_enum usage = VX_WRITE_ONLY, user_mem_type = VX_MEMORY_TYPE_HOST;
    vx_size num_bins = 1;
    vx_int32 offset = 1;
    vx_uint32 range = 5;

    vx_lut lut = NULL;
    int32_t user_data[256];

    vx_matrix matrix,matrix1 = NULL;
    vx_enum data_type = VX_TYPE_INT8;
    vx_size columns = 5, rows = 5;
    vx_size max_size = rows*columns*sizeof(vx_float32);
    vx_uint8* data = ct_alloc_mem(max_size);

    vx_remap remap = NULL;
    vx_uint32 src_width = 16;
    vx_uint32 src_height = 32;
    vx_uint32 dst_width = 128;
    vx_uint32 dst_height = 64;

    vx_image img,image = NULL;
    vx_rectangle_t rect = {0,0,10,10};

    tivx_raw_image raw_image =NULL;
    tivx_raw_image_create_params_t params;
    vx_rectangle_t rect1;
    vx_imagepatch_addressing_t addr;

    params.width = 128;
    params.height = 128;
    params.num_exposures = 3;
    params.line_interleaved = vx_true_e;
    params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    params.format[0].msb = 12;
    params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    params.format[1].msb = 7;
    params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    params.format[2].msb = 11;
    params.meta_height_before = 5;
    params.meta_height_after = 0;

    rect1.start_x = 16;
    rect1.start_y = 19;
    rect1.end_x = 16 + 16;
    rect1.end_y = 19 + 21;

    addr.dim_x = 16;
    addr.dim_y = 21;
    addr.stride_x = 1;
    addr.stride_y = 16;
    addr.step_x = 1;
    addr.step_y = 1;
    uint8_t ptr[16 * 21];

    ASSERT_VX_OBJECT(dist = vxCreateDistribution(context, num_bins, offset, range), VX_TYPE_DISTRIBUTION);
    ASSERT_VX_OBJECT(lut = vxCreateLUT(context, VX_TYPE_UINT8, 256), VX_TYPE_LUT);
    ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, data_type, columns, rows), VX_TYPE_MATRIX);
    ASSERT_VX_OBJECT(remap = vxCreateRemap(context, src_width, src_height, dst_width, dst_height),VX_TYPE_REMAP);
    ASSERT_VX_OBJECT(img = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(context, &params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

    /* Allocating all the memory under heap region TIVX_MEM_EXTERNAL using test-utils mem api*/
    VX_CALL(test_utils_max_out_heap_mem(&tivx_shared_mem_info_array, &num_chunks, mheap_region));

    /* Below APIs should fail due to tivxMemBufferAlloc failure */
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_MEMORY, vxCopyDistribution(dist, udata, usage, user_mem_type));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_MEMORY, vxCopyLUT(lut, user_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_MEMORY, vxCopyMatrix(matrix, data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    EXPECT_VX_ERROR(matrix1 = vxCreateMatrixFromPattern(context, VX_PATTERN_OTHER, columns, rows), VX_ERROR_NO_RESOURCES);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetRemapPoint(remap, 0, 0, 0, 0));
    EXPECT_VX_ERROR(image = vxCreateImageFromChannel(img,VX_CHANNEL_Y), VX_ERROR_NO_RESOURCES);
    EXPECT_VX_ERROR(image = vxCreateImageFromROI(img,&rect), VX_ERROR_NO_RESOURCES);
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_MEMORY, tivxCopyRawImagePatch(raw_image, &rect1, 1, &addr, (void *)&ptr,
                                  VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, TIVX_RAW_IMAGE_PIXEL_BUFFER));

    /* Freeing all the previously allocated memory */
    VX_CALL(test_utils_release_maxed_out_heap_mem(tivx_shared_mem_info_array, num_chunks));

    VX_CALL(vxReleaseDistribution(&dist));
    VX_CALL(vxReleaseLUT(&lut));
    VX_CALL(vxReleaseMatrix(&matrix));
    VX_CALL(vxReleaseRemap(&remap));
    VX_CALL(vxReleaseImage(&img));
    VX_CALL(tivxReleaseRawImage(&raw_image));

    ct_free_mem(data);
}

TESTCASE_TESTS(
    tivxMemBoundary,
    negativeTestMemBufferAllocBoundary
)

