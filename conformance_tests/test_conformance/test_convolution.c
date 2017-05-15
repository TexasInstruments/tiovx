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

TESTCASE(Convolution, CT_VXContext, ct_setup_vx_context, 0)


TEST(Convolution, test_vxCreateConvolution)
{
    vx_context context = context_->vx_context_;
    vx_convolution conv;
    vx_size rows = 3, cols = 3;

    ASSERT_VX_OBJECT(conv = vxCreateConvolution(context, cols, rows), VX_TYPE_CONVOLUTION);

    VX_CALL(vxReleaseConvolution(&conv));

    ASSERT(conv == 0);
}

TEST(Convolution, test_vxCopyConvolution)
{
    vx_context context = context_->vx_context_;
    vx_convolution conv;
    vx_size rows = 3, cols = 3;
    vx_size i, j;
    vx_int16 gx[3][3] = {
        { 3, 0, -3},
        { 10, 0,-10},
        { 3, 0, -3},
    };


    ASSERT_VX_OBJECT(conv = vxCreateConvolution(context, cols, rows), VX_TYPE_CONVOLUTION);

    VX_CALL(vxCopyConvolutionCoefficients(conv, gx, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    vx_int16 *data = (vx_int16 *)ct_alloc_mem(rows*cols*sizeof(vx_int16));
    VX_CALL(vxCopyConvolutionCoefficients(conv, data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            ASSERT(gx[i][j] == data[i * cols + j]);
        }
    }

    VX_CALL(vxReleaseConvolution(&conv));

    ASSERT(conv == 0);

    ct_free_mem(data);
}

TEST(Convolution, test_vxQueryConvolution)
{
    vx_context context = context_->vx_context_;
    vx_convolution conv;
    vx_size rows = 3, cols = 3;
    vx_uint32 scale = 2;

    ASSERT_VX_OBJECT(conv = vxCreateConvolution(context, cols, rows), VX_TYPE_CONVOLUTION);

    VX_CALL(vxSetConvolutionAttribute(conv, VX_CONVOLUTION_SCALE, &scale, sizeof(scale)));
    {
        vx_size actual_n = 0, actual_m = 0, actual_size = 0;
        vx_uint32 actual_scale = 0;
        VX_CALL(vxQueryConvolution(conv, VX_CONVOLUTION_ROWS, &actual_n, sizeof(actual_n)));
        ASSERT(rows == actual_n);

        VX_CALL(vxQueryConvolution(conv, VX_CONVOLUTION_COLUMNS, &actual_m, sizeof(actual_m)));
        ASSERT(cols == actual_m);

        VX_CALL(vxQueryConvolution(conv, VX_CONVOLUTION_SIZE, &actual_size, sizeof(actual_size)));
        ASSERT(rows*cols*sizeof(vx_int16) == actual_size);

        VX_CALL(vxQueryConvolution(conv, VX_CONVOLUTION_SCALE, &actual_scale, sizeof(actual_scale)));
        ASSERT(scale == actual_scale);
    }

    VX_CALL(vxReleaseConvolution(&conv));

    ASSERT(conv == 0);
}


TESTCASE_TESTS(Convolution, test_vxCreateConvolution, test_vxCopyConvolution, test_vxQueryConvolution)
