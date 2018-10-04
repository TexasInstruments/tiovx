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
#include <string.h>

#include "shared_functions.h"

#define TENSOR_DIMS_NUM 4
#define TENSOR_DIMS_LENGTH 20

TESTCASE(tivxTensor, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    vx_size winSize;
    int useReferencePyramid;
} Arg;


#define PARAMETERS \
    ARG("Tensor/Create", NULL, 5, 1)

TEST_WITH_ARG(tivxTensor, testCreateTensor, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_tensor tensor_uint8 = 0, tensor_int8 = 0, tensor_uint16 = 0, tensor_int16 = 0;
    vx_tensor tensor_uint32 = 0, tensor_int32 = 0, tensor_uint64 = 0, tensor_int64 = 0;
    vx_tensor tensor_float32 = 0, tensor_float64 = 0;
    vx_size i;

    vx_size *dims = (vx_size*)ct_alloc_mem(TENSOR_DIMS_NUM * sizeof(vx_size));

    for(i = 0; i < TENSOR_DIMS_NUM; i++)
    {
        dims[i] = TENSOR_DIMS_LENGTH;
    }

    ASSERT_VX_OBJECT(tensor_uint8 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_UINT8, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    ASSERT_VX_OBJECT(tensor_int8 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_INT8, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    ASSERT_VX_OBJECT(tensor_uint16 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_UINT16, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    ASSERT_VX_OBJECT(tensor_int16 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_INT16, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    ASSERT_VX_OBJECT(tensor_uint32 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_UINT32, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    ASSERT_VX_OBJECT(tensor_int32 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_INT32, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    ASSERT_VX_OBJECT(tensor_float32 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_FLOAT32, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    EXPECT_VX_ERROR(tensor_uint64 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_UINT64, 0), VX_ERROR_INVALID_TYPE);

    EXPECT_VX_ERROR(tensor_int64 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_INT64, 0), VX_ERROR_INVALID_TYPE);

    EXPECT_VX_ERROR(tensor_float64 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_FLOAT64, 0), VX_ERROR_INVALID_TYPE);
    
    ct_free_mem(dims);

    VX_CALL(vxReleaseTensor(&tensor_float32));
    VX_CALL(vxReleaseTensor(&tensor_int32));
    VX_CALL(vxReleaseTensor(&tensor_uint32));
    VX_CALL(vxReleaseTensor(&tensor_int16));
    VX_CALL(vxReleaseTensor(&tensor_uint16));
    VX_CALL(vxReleaseTensor(&tensor_uint8));
    VX_CALL(vxReleaseTensor(&tensor_int8));
}

TEST_WITH_ARG(tivxTensor, testQueryTensor, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_tensor tensor_uint8 = 0;
    vx_size max_dims, num_dims;
    vx_enum data_type;
    vx_int8 fixed;

    VX_CALL(vxQueryContext(context, VX_CONTEXT_MAX_TENSOR_DIMS, &max_dims, sizeof(vx_size)));

    vx_size *dims = (vx_size*)ct_alloc_mem((max_dims+1) * sizeof(vx_size));
    vx_size *dims_ref = (vx_size*)ct_alloc_mem((max_dims) * sizeof(vx_size));
    vx_size i;
    for(i = 0; i < TENSOR_DIMS_NUM; i++)
    {
        dims[i] = TENSOR_DIMS_LENGTH;
    }

    EXPECT_VX_ERROR(tensor_uint8 = vxCreateTensor(context, max_dims+1, dims, VX_TYPE_UINT8, 0), VX_ERROR_INVALID_DIMENSION);

    EXPECT_VX_ERROR(tensor_uint8 = vxCreateTensor(context, 0, dims, VX_TYPE_UINT8, 0), VX_ERROR_INVALID_DIMENSION);

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryTensor(tensor_uint8, VX_TENSOR_DATA_TYPE, &data_type, sizeof(vx_size)));



    ASSERT_VX_OBJECT(tensor_uint8 = vxCreateTensor(context, max_dims, dims, VX_TYPE_UINT8, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    VX_CALL(vxQueryTensor(tensor_uint8, VX_TENSOR_DATA_TYPE, &data_type, sizeof(vx_enum)));

    ASSERT(data_type == VX_TYPE_UINT8);

    VX_CALL(vxQueryTensor(tensor_uint8, VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(vx_size)));

    ASSERT(num_dims == max_dims);

    VX_CALL(vxQueryTensor(tensor_uint8, VX_TENSOR_FIXED_POINT_POSITION, &fixed, sizeof(vx_int8)));

    ASSERT(fixed == 0);

    VX_CALL(vxQueryTensor(tensor_uint8, VX_TENSOR_DIMS, dims_ref, max_dims * sizeof(vx_size)));

    for(i = 0; i < TENSOR_DIMS_NUM; i++)
    {
        ASSERT(dims_ref[i] == dims[i]);
    }

    VX_CALL(vxReleaseTensor(&tensor_uint8));

    ct_free_mem(dims);
    ct_free_mem(dims_ref);
}

TEST_WITH_ARG(tivxTensor, testCopyTensor, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_tensor tensor_uint8 = 0, tensor_uint16 = 0;
    vx_tensor tensor_uint32 = 0, tensor_float32 = 0;

    vx_size start[TENSOR_DIMS_NUM] = { 0 };
    vx_size strides8[TENSOR_DIMS_NUM]= { 0 };
    vx_size strides16[TENSOR_DIMS_NUM]= { 0 };
    vx_size strides32[TENSOR_DIMS_NUM]= { 0 };
    vx_size stridesf32[TENSOR_DIMS_NUM]= { 0 };
    vx_size *dims = (vx_size*)ct_alloc_mem(TENSOR_DIMS_NUM * sizeof(vx_size));
    uint32_t i;

    for(i = 0; i < TENSOR_DIMS_NUM; i++)
    {
        dims[i] = TENSOR_DIMS_LENGTH;
        start[i] = 0;
        strides8[i] = i ? strides8[i - 1] * dims[i - 1] : sizeof(vx_uint8);
        strides16[i] = i ? strides16[i - 1] * dims[i - 1] : sizeof(vx_uint16);
        strides32[i] = i ? strides32[i - 1] * dims[i - 1] : sizeof(vx_uint32);
        stridesf32[i] = i ? stridesf32[i - 1] * dims[i - 1] : sizeof(vx_float32);
    }

    const vx_size bytes8 = dims[TENSOR_DIMS_NUM - 1] * strides8[TENSOR_DIMS_NUM - 1];
    const vx_size bytes16 = dims[TENSOR_DIMS_NUM - 1] * strides16[TENSOR_DIMS_NUM - 1];
    const vx_size bytes32 = dims[TENSOR_DIMS_NUM - 1] * strides32[TENSOR_DIMS_NUM - 1];
    const vx_size bytesf32 = dims[TENSOR_DIMS_NUM - 1] * stridesf32[TENSOR_DIMS_NUM - 1];
    
    vx_uint8  * data8 = ct_alloc_mem(bytes8);
    vx_uint16 * data16 = ct_alloc_mem(bytes16);
    vx_uint32 * data32 = ct_alloc_mem(bytes32);
    vx_float32 * dataf32 = ct_alloc_mem(bytesf32);

    for(i = 0; i < bytes8; i++)
    {
        data8[i] = i%256;
        data16[i] = i%65536;
        data32[i] = i;
        dataf32[i] = i*1.0f;
    }

    ASSERT_VX_OBJECT(tensor_uint8 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_UINT8, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    ASSERT_VX_OBJECT(tensor_uint16 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_UINT16, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    ASSERT_VX_OBJECT(tensor_uint32 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_UINT32, 0), (enum vx_type_e)VX_TYPE_TENSOR);
    
    ASSERT_VX_OBJECT(tensor_float32 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_FLOAT32, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    VX_CALL(vxCopyTensorPatch(tensor_uint8, TENSOR_DIMS_NUM, start, dims, strides8, data8, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyTensorPatch(tensor_uint16, TENSOR_DIMS_NUM, start, dims, strides16, data16, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyTensorPatch(tensor_uint32, TENSOR_DIMS_NUM, start, dims, strides32, data32, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyTensorPatch(tensor_float32, TENSOR_DIMS_NUM, start, dims, stridesf32, dataf32, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    memset(data8, 0, bytes8);
    memset(data16, 0, bytes16);
    memset(data32, 0, bytes32);
    memset(dataf32, 0, bytesf32);

    VX_CALL(vxCopyTensorPatch(tensor_uint8, TENSOR_DIMS_NUM, start, dims, strides8, data8, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyTensorPatch(tensor_uint16, TENSOR_DIMS_NUM, start, dims, strides16, data16, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyTensorPatch(tensor_uint32, TENSOR_DIMS_NUM, start, dims, strides32, data32, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyTensorPatch(tensor_float32, TENSOR_DIMS_NUM, start, dims, stridesf32, dataf32, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    for(i = 0; i < bytes8; i++)
    {
        ASSERT(data8[i] == i%256);
        ASSERT(data16[i] == i%65536);
        ASSERT(data32[i] == i);
        ASSERT(dataf32[i] == i*1.0f);
    }

    ct_free_mem(dims);
    ct_free_mem(data8);
    ct_free_mem(data16);
    ct_free_mem(data32);
    ct_free_mem(dataf32);

    VX_CALL(vxReleaseTensor(&tensor_float32));
    VX_CALL(vxReleaseTensor(&tensor_uint32));
    VX_CALL(vxReleaseTensor(&tensor_uint16));
    VX_CALL(vxReleaseTensor(&tensor_uint8));
}

TEST_WITH_ARG(tivxTensor, testMapTensor, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_tensor tensor_uint8 = 0, tensor_uint16 = 0;
    vx_tensor tensor_uint32 = 0, tensor_float32 = 0;

    vx_size start[TENSOR_DIMS_NUM] = { 0 };
    vx_size strides8[TENSOR_DIMS_NUM]= { 0 };
    vx_size strides16[TENSOR_DIMS_NUM]= { 0 };
    vx_size strides32[TENSOR_DIMS_NUM]= { 0 };
    vx_size stridesf32[TENSOR_DIMS_NUM]= { 0 };
    
    vx_size strides_map8[TENSOR_DIMS_NUM]= { 0 };
    vx_size strides_map16[TENSOR_DIMS_NUM]= { 0 };
    vx_size strides_map32[TENSOR_DIMS_NUM]= { 0 };
    vx_size strides_mapf32[TENSOR_DIMS_NUM]= { 0 };

    vx_size dims_map8[TENSOR_DIMS_NUM]= { 0 };
    vx_size dims_map16[TENSOR_DIMS_NUM]= { 0 };
    vx_size dims_map32[TENSOR_DIMS_NUM]= { 0 };
    vx_size dims_mapf32[TENSOR_DIMS_NUM]= { 0 };
    
    vx_size start_map0[TENSOR_DIMS_NUM]= { 0 };
    vx_size start_map5[TENSOR_DIMS_NUM]= { 5, 6, 7, 8 };
    vx_size end_map0[TENSOR_DIMS_NUM]= { TENSOR_DIMS_LENGTH, TENSOR_DIMS_LENGTH, TENSOR_DIMS_LENGTH, TENSOR_DIMS_LENGTH };
    vx_size end_map5[TENSOR_DIMS_NUM]= { TENSOR_DIMS_LENGTH-5, TENSOR_DIMS_LENGTH-4, TENSOR_DIMS_LENGTH-3, TENSOR_DIMS_LENGTH-2  };
    
    vx_map_id id8, id16, id32, idf32;
    
    vx_uint8 *ptr8 = NULL;
    vx_uint16 *ptr16 = NULL;
    vx_uint32 *ptr32 = NULL;
    vx_float32 *ptrf32 = NULL;
    
    vx_size *dims = (vx_size*)ct_alloc_mem(TENSOR_DIMS_NUM * sizeof(vx_size));
    uint32_t i, j;

    for(i = 0; i < TENSOR_DIMS_NUM; i++)
    {
        dims[i] = TENSOR_DIMS_LENGTH;
        start[i] = 0;
        strides8[i] = i ? strides8[i - 1] * dims[i - 1] : sizeof(vx_uint8);
        strides16[i] = i ? strides16[i - 1] * dims[i - 1] : sizeof(vx_uint16);
        strides32[i] = i ? strides32[i - 1] * dims[i - 1] : sizeof(vx_uint32);
        stridesf32[i] = i ? stridesf32[i - 1] * dims[i - 1] : sizeof(vx_float32);
    }

    const vx_size bytes8 = dims[TENSOR_DIMS_NUM - 1] * strides8[TENSOR_DIMS_NUM - 1];
    const vx_size bytes16 = dims[TENSOR_DIMS_NUM - 1] * strides16[TENSOR_DIMS_NUM - 1];
    const vx_size bytes32 = dims[TENSOR_DIMS_NUM - 1] * strides32[TENSOR_DIMS_NUM - 1];
    const vx_size bytesf32 = dims[TENSOR_DIMS_NUM - 1] * stridesf32[TENSOR_DIMS_NUM - 1];

    vx_uint8  * data8 = ct_alloc_mem(bytes8);
    vx_uint16 * data16 = ct_alloc_mem(bytes16);
    vx_uint32 * data32 = ct_alloc_mem(bytes32);
    vx_float32 * dataf32 = ct_alloc_mem(bytesf32);

    for(i = 0; i < bytes8; i++)
    {
        data8[i] = i%256;
        data16[i] = i%65536;
        data32[i] = i;
        dataf32[i] = i*1.0f;
    }

    ASSERT_VX_OBJECT(tensor_uint8 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_UINT8, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    ASSERT_VX_OBJECT(tensor_uint16 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_UINT16, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    ASSERT_VX_OBJECT(tensor_uint32 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_UINT32, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    ASSERT_VX_OBJECT(tensor_float32 = vxCreateTensor(context, TENSOR_DIMS_NUM, dims, VX_TYPE_FLOAT32, 0), (enum vx_type_e)VX_TYPE_TENSOR);

    VX_CALL(vxCopyTensorPatch(tensor_uint8, TENSOR_DIMS_NUM, start, dims, strides8, data8, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyTensorPatch(tensor_uint16, TENSOR_DIMS_NUM, start, dims, strides16, data16, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyTensorPatch(tensor_uint32, TENSOR_DIMS_NUM, start, dims, strides32, data32, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyTensorPatch(tensor_float32, TENSOR_DIMS_NUM, start, dims, stridesf32, dataf32, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(tivxMapTensorPatch(tensor_uint8, TENSOR_DIMS_NUM, start_map0, end_map0, &id8, dims_map8, strides_map8, (void **)&ptr8, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0));
    VX_CALL(tivxMapTensorPatch(tensor_uint16, TENSOR_DIMS_NUM, start_map0, end_map0, &id16, dims_map16, strides_map16, (void **)&ptr16, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0));
    VX_CALL(tivxMapTensorPatch(tensor_uint32, TENSOR_DIMS_NUM, start_map0, end_map0, &id32, dims_map32, strides_map32, (void **)&ptr32, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0));
    VX_CALL(tivxMapTensorPatch(tensor_float32, TENSOR_DIMS_NUM, start_map0, end_map0, &idf32, dims_mapf32, strides_mapf32, (void **)&ptrf32, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0));

    for(j = 0; j < bytes8; j++)
    {
        vx_size tensor_pos = 0;
        vx_size index_leftover = j;
        int divisor = 1;
        vx_size i;
        for (i = 0; i < TENSOR_DIMS_NUM; i++)
        {
            divisor = dims_map8[i];
            tensor_pos += strides_map8[i] * (index_leftover%divisor);
            index_leftover = index_leftover / divisor;
        }
        ASSERT(ptr8[tensor_pos] == j%256);
        ptr8[tensor_pos] += 1;
    }

    for(j = 0; j < bytes8; j++)
    {
        vx_size tensor_pos = 0;
        vx_size index_leftover = j;
        int divisor = 1;
        vx_size i;
        for (i = 0; i < TENSOR_DIMS_NUM; i++)
        {
            divisor = dims_map16[i];
            tensor_pos += strides_map16[i] * (index_leftover%divisor);
            index_leftover = index_leftover / divisor;
        }
        ASSERT(ptr16[tensor_pos/2] == j%65536);
        ptr16[tensor_pos/2] += 1;
    }

    for(j = 0; j < bytes8; j++)
    {
        vx_size tensor_pos = 0;
        vx_size index_leftover = j;
        int divisor = 1;
        vx_size i;
        for (i = 0; i < TENSOR_DIMS_NUM; i++)
        {
            divisor = dims_map32[i];
            tensor_pos += strides_map32[i] * (index_leftover%divisor);
            index_leftover = index_leftover / divisor;
        }
        ASSERT(ptr32[tensor_pos/4] == j);
        ptr32[tensor_pos/4] += 1;
    }

    for(j = 0; j < bytes8; j++)
    {
        vx_size tensor_pos = 0;
        vx_size index_leftover = j;
        int divisor = 1;
        vx_size i;
        for (i = 0; i < TENSOR_DIMS_NUM; i++)
        {
            divisor = dims_mapf32[i];
            tensor_pos += strides_mapf32[i] * (index_leftover%divisor);
            index_leftover = index_leftover / divisor;
        }
        ASSERT(ptrf32[tensor_pos/4] == j*1.0f);
        ptrf32[tensor_pos/4] += 1.0f;
    }

    VX_CALL(tivxUnmapTensorPatch(tensor_uint8, id8));
    VX_CALL(tivxUnmapTensorPatch(tensor_uint16, id16));
    VX_CALL(tivxUnmapTensorPatch(tensor_uint32, id32));
    VX_CALL(tivxUnmapTensorPatch(tensor_float32, idf32));
    
    memset(data8, 0, bytes8);
    memset(data16, 0, bytes16);
    memset(data32, 0, bytes32);
    memset(dataf32, 0, bytesf32);

    VX_CALL(vxCopyTensorPatch(tensor_uint8, TENSOR_DIMS_NUM, start, dims, strides8, data8, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyTensorPatch(tensor_uint16, TENSOR_DIMS_NUM, start, dims, strides16, data16, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxCopyTensorPatch(tensor_uint32, TENSOR_DIMS_NUM, start, dims, strides32, data32, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    
    VX_CALL(vxCopyTensorPatch(tensor_float32, TENSOR_DIMS_NUM, start, dims, stridesf32, dataf32, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    for(i = 0; i < bytes8; i++)
    {
        ASSERT(data8[i] == (i+1)%256);
        ASSERT(data16[i] == (i+1)%65536);
        ASSERT(data32[i] == (i+1));
        ASSERT(data32[i] == (i+1)*1.0f);
    }

    ct_free_mem(dataf32);    
    ct_free_mem(data32);
    ct_free_mem(data16);    
    ct_free_mem(data8);
    ct_free_mem(dims);
        
    VX_CALL(vxReleaseTensor(&tensor_float32));
    VX_CALL(vxReleaseTensor(&tensor_uint32));
    VX_CALL(vxReleaseTensor(&tensor_uint16));
    VX_CALL(vxReleaseTensor(&tensor_uint8));

}

TESTCASE_TESTS(tivxTensor,
        testCreateTensor,
        testQueryTensor,
        testCopyTensor,
        testMapTensor
        )
