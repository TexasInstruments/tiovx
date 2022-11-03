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

TESTCASE(tivxMatx, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxMatx, negativeTestCreateMatrix)
{
    vx_context context = context_->vx_context_;

    vx_enum data_type = VX_TYPE_INVALID;
    vx_size columns = 0, rows = 0;

    ASSERT(NULL == vxCreateMatrix(NULL, data_type, columns, rows));
    ASSERT(NULL == vxCreateMatrix(context, data_type, columns, rows));
    ASSERT(NULL == vxCreateMatrix(context, VX_TYPE_INT8, columns, 1));
}

TEST(tivxMatx, negativeTestCreateMatrixFromPattern)
{
    #define VX_PATTERN_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_matrix matrix = NULL;
    vx_enum pattern = VX_PATTERN_DEFAULT;
    vx_size columns = 0, rows = 0;

    ASSERT(NULL == vxCreateMatrixFromPattern(NULL, pattern, columns, rows));
    ASSERT(NULL == vxCreateMatrixFromPattern(context, VX_PATTERN_CROSS, 2, 2));
    ASSERT(NULL == vxCreateMatrixFromPattern(context, VX_PATTERN_DISK, 1, 2));
    ASSERT_VX_OBJECT(matrix = vxCreateMatrixFromPattern(context, VX_PATTERN_OTHER, 1, 2), VX_TYPE_MATRIX);
    VX_CALL(vxReleaseMatrix(&matrix));
}

TEST(tivxMatx, negativeTestQueryMatrix)
{
    #define VX_MATRIX_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_matrix matrix = NULL;
    vx_enum attribute = VX_MATRIX_DEFAULT;
    vx_size size = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryMatrix(matrix, attribute, NULL, size));
    ASSERT_VX_OBJECT(matrix = vxCreateMatrixFromPattern(context, VX_PATTERN_OTHER, 1, 2), VX_TYPE_MATRIX);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryMatrix(matrix, VX_MATRIX_TYPE, NULL, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryMatrix(matrix, VX_MATRIX_ROWS, NULL, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryMatrix(matrix, VX_MATRIX_COLUMNS, NULL, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryMatrix(matrix, VX_MATRIX_SIZE, NULL, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryMatrix(matrix, VX_MATRIX_ORIGIN, NULL, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryMatrix(matrix, VX_MATRIX_PATTERN, NULL, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryMatrix(matrix, attribute, NULL, size));
    VX_CALL(vxReleaseMatrix(&matrix));
}

TEST(tivxMatx, negativeTestCopyMatrix)
{
    vx_context context = context_->vx_context_;

    vx_matrix matrix = NULL;
    vx_enum usage = VX_READ_ONLY, user_mem_type = VX_MEMORY_TYPE_NONE;
    vx_enum data_type = VX_TYPE_INT8;
    vx_size columns = 5, rows = 5;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxCopyMatrix(matrix, NULL, usage, user_mem_type));
    ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, data_type, columns, rows), VX_TYPE_MATRIX);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxCopyMatrix(matrix, NULL, usage, user_mem_type));
    VX_CALL(vxReleaseMatrix(&matrix));
}

TESTCASE_TESTS(
    tivxMatx,
    negativeTestCreateMatrix,
    negativeTestCreateMatrixFromPattern,
    negativeTestQueryMatrix,
    negativeTestCopyMatrix    
)

