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

#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include <math.h>

typedef struct
{
    const char* name;
    vx_enum type;
} data_type_arg;

TESTCASE(Matrix, CT_VXContext, ct_setup_vx_context, 0)

TEST_WITH_ARG(Matrix, test_vxCreateMatrix, data_type_arg,
              ARG_ENUM(VX_TYPE_UINT8),
              ARG_ENUM(VX_TYPE_INT32),
              ARG_ENUM(VX_TYPE_FLOAT32))
{
    vx_context context = context_->vx_context_;
    vx_matrix matrix;
    vx_size rows = 5, cols = 3;
    vx_enum data_type = arg_->type;

    ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, data_type, cols, rows), VX_TYPE_MATRIX);

    VX_CALL(vxReleaseMatrix(&matrix));

    ASSERT(matrix == 0);
}

TEST_WITH_ARG(Matrix, test_vxCopyMatrix, data_type_arg,
              ARG_ENUM(VX_TYPE_UINT8),
              ARG_ENUM(VX_TYPE_INT32),
              ARG_ENUM(VX_TYPE_FLOAT32))
{
    vx_context context = context_->vx_context_;
    vx_matrix matrix;
    vx_size rows = 5, cols = 3;
    vx_enum data_type = arg_->type;
    uint64_t* seed = &CT()->seed_;
    vx_size max_size = rows*cols*sizeof(vx_float32);
    vx_uint8* data = ct_alloc_mem(max_size);
    vx_size i;
    for (i = 0; i < max_size; i++)
    {
        data[i] = (vx_uint8)CT_RNG_NEXT_INT(*seed, 0, 256);
    }

    ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, data_type, cols, rows), VX_TYPE_MATRIX);

    VX_CALL(vxCopyMatrix(matrix, data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    void* actual_data = ct_alloc_mem(max_size);
    VX_CALL(vxCopyMatrix(matrix, actual_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    {
        vx_size i;
        switch (data_type)
        {
        case VX_TYPE_UINT8:
            for (i = 0; i < rows*cols; i++)
            {
                ASSERT(((vx_uint8*)data)[i] == ((vx_uint8*)actual_data)[i]);
            }
            break;
        case VX_TYPE_INT32:
            for (i = 0; i < rows*cols; i++)
            {
                ASSERT(((vx_int32*)data)[i] == ((vx_int32*)actual_data)[i]);
            }
            break;
        case VX_TYPE_FLOAT32:
            for (i = 0; i < rows*cols; i++)
            {
                ASSERT(fabs(((vx_float32*)data)[i] - ((vx_float32*)actual_data)[i]) < 0.0000001f);
            }
            break;
        }
    }

    VX_CALL(vxReleaseMatrix(&matrix));

    ASSERT(matrix == 0);

    ct_free_mem(actual_data);
    ct_free_mem(data);
}

TEST_WITH_ARG(Matrix, test_vxQueryMatrix, data_type_arg,
              ARG_ENUM(VX_TYPE_UINT8),
              ARG_ENUM(VX_TYPE_INT32),
              ARG_ENUM(VX_TYPE_FLOAT32))
{
    vx_context context = context_->vx_context_;
    vx_matrix matrix;
    vx_size rows = 5, cols = 3;
    vx_enum data_type = arg_->type;

    ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, data_type, cols, rows), VX_TYPE_MATRIX);

    {
        vx_enum actual_type = VX_TYPE_INVALID;
        vx_size actual_rows = 0, actual_cols = 0;
        vx_coordinates2d_t actual_origin = {0, 0};
        vx_size actual_size = 0;

        VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_TYPE, &actual_type, sizeof(actual_type)));
        ASSERT(data_type == actual_type);

        VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_ROWS, &actual_rows, sizeof(actual_rows)));
        ASSERT(rows == actual_rows);

        VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_COLUMNS, &actual_cols, sizeof(actual_cols)));
        ASSERT(cols == actual_cols);

        VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_ORIGIN, &actual_origin, sizeof(actual_origin)));
        ASSERT(rows / 2 == actual_origin.y);
        ASSERT(cols / 2 == actual_origin.x);

        VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_SIZE, &actual_size, sizeof(actual_size)));
        switch(data_type)
        {
        case VX_TYPE_UINT8:
            ASSERT(rows*cols*sizeof(vx_uint8) == actual_size);
            break;
        case VX_TYPE_INT32:
            ASSERT(rows*cols*sizeof(vx_int32) == actual_size);
            break;
        case VX_TYPE_FLOAT32:
            ASSERT(rows*cols*sizeof(vx_float32) == actual_size);
            break;
        }
    }

    VX_CALL(vxReleaseMatrix(&matrix));

    ASSERT(matrix == 0);
}


TESTCASE_TESTS(Matrix, test_vxCreateMatrix, test_vxCopyMatrix, test_vxQueryMatrix)
