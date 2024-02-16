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
 * Copyright (c) 2023 Texas Instruments Incorporated
 */
#include <vx_internal.h>
#include <tivx_objects.h>

#include "test_tiovx.h"
#include "test_engine/test.h"
#include "test_tiovx/test_tiovx.h"
#include <VX/vx.h>
#include <VX/vxu.h>

#include <vx_context.h>

TESTCASE(tivxInternalTensor, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalTensor, negativeTestUnmapTensorPatchMapId)
{
    vx_context context = context_->vx_context_;
    vx_tensor tensor;
    vx_size num_dims = 2;
    vx_size dim_length = 20;
    vx_size strides[2];
    vx_map_id mid[2];
    vx_size *dims = (vx_size*)ct_alloc_mem(num_dims * sizeof(vx_size));
    vx_uint32 *pdata[2];
    int i;
    for (i = 0; i < 2; i++)
    {
        pdata[i] = 0;
    }
    for (i = 0; i < num_dims; i++)
    {
        strides[i] = 0;
        dims[i] = dim_length;
        strides[i] = i ? strides[i - 1] * dims[i - 1] : sizeof(vx_uint8);
    }

    ASSERT_VX_OBJECT(tensor = vxCreateTensor(context, num_dims, dims, VX_TYPE_UINT8, 0), (enum vx_type_e)VX_TYPE_TENSOR);
    for (i = 0; i < 2; i++)
    {
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxMapTensorPatch(tensor, num_dims, NULL, NULL, &mid[i], strides, (void **)&pdata[i], VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST));
    }
    vx_uint8* map_addr1 = tensor->maps[0].map_addr;
    vx_size map_size1 = tensor->maps[1].map_size;
    tensor->maps[0].map_addr = NULL;
    tensor->maps[1].map_size = 0;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxUnmapTensorPatch(tensor, 0));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxUnmapTensorPatch(tensor, 1));
    tensor->maps[1].map_size = map_size1;
    tensor->maps[0].map_addr = map_addr1;
    for (i = 0; i < 2; i++)
    {
        VX_CALL(tivxUnmapTensorPatch(tensor, mid[i]));
    }
    VX_CALL(vxReleaseTensor(&tensor));
}

TESTCASE_TESTS(
    tivxInternalTensor,
    negativeTestUnmapTensorPatchMapId
)