/*

 * Copyright (c) 2023 The Khronos Group Inc.
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

/* copy_swap.c
 * Tests for copy/swap/move functionality.
 */

#include "test_engine/test.h"
#include "VX/vx.h"
#include "VX/vxu.h"
#include "TI/tivx.h"
#include <TI/tivx_config.h>
#include <VX/vx_khr_safe_casts.h>
#include <VX/vx_khr_swap_move.h>


/**********************************************************/
/* test : 'Copy Swap' for RB copy and swap implementation */
/**********************************************************/

TESTCASE(copySwap, CT_VXContext, ct_setup_vx_context, 0)

/* Function to get a parameter, add it to a graph and then release it */
static void addParameterToGraph(vx_graph graph, vx_node node, vx_uint32 num)
{
    vx_parameter p = vxGetParameterByIndex(node, num);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS,vxAddParameterToGraph(graph, p));
    vxReleaseParameter(&p);
}
/* Write some data on an image */
static void writeImage(vx_image image, vx_uint8 a, vx_uint8 b)
{
    vx_map_id id;
    vx_rectangle_t rect = {.start_x = 0, .end_x = 2, .start_y = 0, .end_y = 2};
    vx_imagepatch_addressing_t ipa;
    void *ptr;
    vxMapImagePatch(image, &rect, 0, &id, &ipa, &ptr, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);
    *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr, 0, 0, &ipa) = a;
    *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr, 1, 0, &ipa) = b;
    vxUnmapImagePatch(image, id);
}

/* Check data in an image */
static vx_status checkImage(vx_image image, vx_uint8 a, vx_uint8 b)
{
    vx_map_id id;
    vx_status stat;
    vx_rectangle_t rect = {.start_x = 0, .end_x = 2, .start_y = 0, .end_y = 2};
    vx_imagepatch_addressing_t ipa;
    void *ptr;
    vxMapImagePatch(image, &rect, 0, &id, &ipa, &ptr, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);
    vx_uint8 aa = *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr, 0, 0, &ipa);
    vx_uint8 bb = *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr, 1, 0, &ipa);
    vxUnmapImagePatch(image, id);
    if (aa != a || bb != b)
        printf("expected (%x, %x) but got (%x, %x)\n", (int)a, (int)b, (int)aa, (int)bb);

    /* On failure return a non-zero value encoding the pixels, easily readable in decimal */
    return (aa == a) && (bb  == b) ? VX_SUCCESS : aa * 1000 + bb + 1000000;
}

static const vx_enum type_ids[] =
{
    VX_TYPE_ARRAY,
    VX_TYPE_CONVOLUTION,
    VX_TYPE_DISTRIBUTION,
    VX_TYPE_IMAGE,
    VX_TYPE_LUT,
    VX_TYPE_MATRIX,
    VX_TYPE_OBJECT_ARRAY,
    VX_TYPE_PYRAMID,
    VX_TYPE_REMAP,
    VX_TYPE_SCALAR,
    VX_TYPE_TENSOR,
    VX_TYPE_THRESHOLD,
    VX_TYPE_USER_DATA_OBJECT
    };

#define NUM_TYPES (sizeof(type_ids) / sizeof(vx_enum))

typedef struct _user_data
{
    vx_uint32 numbers[4];
} user_data_t;

/* Create a non-virtual example reference of the given type */
vx_reference createReference(vx_context context, vx_enum type)
{
    vx_reference ref = NULL;
    switch (type)
    {
        case VX_TYPE_ARRAY:
            ref = (vx_reference)vxCreateArray(context, VX_TYPE_UINT8, 4);
            break;
        case VX_TYPE_CONVOLUTION:
            ref = (vx_reference)vxCreateConvolution(context, 3, 3);
            break;;
        case VX_TYPE_DISTRIBUTION:
            ref = (vx_reference)vxCreateDistribution(context, 4, 0, 256);
            break;
        case VX_TYPE_IMAGE:
            ref = (vx_reference)vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
            break;
        case VX_TYPE_LUT:
            ref = (vx_reference)vxCreateLUT(context, VX_TYPE_UINT8, 4);
            break;
        case VX_TYPE_MATRIX:
            ref = (vx_reference)vxCreateMatrix(context, VX_TYPE_INT32, 3, 3);
            break;
        case VX_TYPE_OBJECT_ARRAY:
        {
            vx_image image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
            ref = (vx_reference)vxCreateObjectArray(context, (vx_reference)image, 4);
            vxReleaseImage(&image);
            break;
        }
        case VX_TYPE_PYRAMID:
            ref = (vx_reference)vxCreatePyramid(context, 2, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8);
            break;
        case VX_TYPE_SCALAR:
        {
            vx_uint32 value = 0;
            ref = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &value);
            break;
        }
        case VX_TYPE_TENSOR:
        {
            vx_size dims[3] = {4, 4, 4};
            ref = (vx_reference)vxCreateTensor(context, 3, dims, VX_TYPE_INT32, 0);
            break;
        }
        case VX_TYPE_USER_DATA_OBJECT:
        {
            user_data_t user_data = {0};
            ref = (vx_reference)vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data);
            break;
        }
        case VX_TYPE_REMAP:
            ref = (vx_reference)vxCreateRemap(context, 16, 16, 16, 16);
            break;
        case VX_TYPE_THRESHOLD:
            ref = (vx_reference)vxCreateThreshold(context, VX_THRESHOLD_TYPE_BINARY, VX_TYPE_UINT8); /* This will need updating!!)*/
            break;
        default:
            /* Should not reach here. ERROR in TEST! Found unknown type */
            break;
    }
    return ref;
}

/* Write values into appropriate places of the given reference & type */
static vx_status writeValues(vx_reference ref, vx_enum type, vx_uint8 a, vx_uint8 b)
{
    vx_status status = VX_SUCCESS;
    switch (type)
    {
        case VX_TYPE_ARRAY:
        {
            vx_uint8 items[4] = {a, b, b, a};
            status = vxAddArrayItems((vx_array)ref, 4, items, sizeof(vx_uint8));
            break;
        }
        case VX_TYPE_CONVOLUTION:
        {
            vx_int16 coeffs[9] = { a, b, (a << 8) + b, b, a, (b << 8) + a, a + b, a - b, a * b};
            status = vxCopyConvolutionCoefficients((vx_convolution)ref, coeffs, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            break;
        }
        case VX_TYPE_DISTRIBUTION:
        {
            vx_uint8 dist[4] = {a, b, b, a};
            status = vxCopyDistribution((vx_distribution)ref, dist, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            break;
        }
        case VX_TYPE_IMAGE:
            writeImage((vx_image)ref, a, b);
            break;
        case VX_TYPE_LUT:
        {
            vx_uint8 lut[4] = {a, b, b, a};
            status = vxCopyLUT((vx_lut)ref, lut, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            break;
        }
        case VX_TYPE_MATRIX:
        {
            vx_int32 coeffs[9] = { a, b, (a << 8) + b, b, a, (b << 8) + a, a + b, a - b, a * b};
            status = vxCopyMatrix((vx_matrix)ref, coeffs, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            break;
        }
        case VX_TYPE_OBJECT_ARRAY:
        {
            vx_image image = (vx_image)vxGetObjectArrayItem((vx_object_array)ref, 0);
            writeImage(image, a, b);
            vxReleaseImage(&image);
            image = (vx_image)vxGetObjectArrayItem((vx_object_array)ref, 1);
            writeImage(image, b, a);
            vxReleaseImage(&image);
            image = (vx_image)vxGetObjectArrayItem((vx_object_array)ref, 2);
            writeImage(image, b, a);
            vxReleaseImage(&image);
            image = (vx_image)vxGetObjectArrayItem((vx_object_array)ref, 3);
            writeImage(image, b, a);
            vxReleaseImage(&image);
            break;
        }
        case VX_TYPE_PYRAMID:
        {
            vx_image image = vxGetPyramidLevel((vx_pyramid)ref, 0);
            writeImage(image, a, b);
            vxReleaseImage(&image);
            image = (vx_image)vxGetPyramidLevel((vx_pyramid)ref, 1);
            writeImage(image, b, a);
            vxReleaseImage(&image);
            break;
        }
        case VX_TYPE_REMAP:
        {
            status = VX_ERROR_NOT_SUPPORTED;
            break;
        }
        case VX_TYPE_SCALAR:
        {
            vx_uint32 value = a << (16 + b);
            status = vxCopyScalar((vx_scalar)ref, &value, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            break;
        }
        case VX_TYPE_TENSOR:
        {
            vx_size num_dims;
            vx_enum data_type;
            vx_uint16 size;     /* dimension vx_size triggers warning with flag -Wvla-larger-than= */

            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryTensor((vx_tensor)ref, VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(num_dims)));
            vx_size end[num_dims];
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryTensor((vx_tensor)ref, VX_TENSOR_DIMS, &end, sizeof(end)));
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryTensor((vx_tensor)ref, VX_TENSOR_DATA_TYPE, &data_type, sizeof(data_type)));
            vx_size start[num_dims];
            vx_size strides[num_dims];
            switch (data_type)
            {
                case VX_TYPE_UINT8:
                case VX_TYPE_INT8:
                    strides[0] = sizeof(vx_uint8);
                    break;
                case VX_TYPE_UINT16:
                case VX_TYPE_INT16:
                    strides[0] = sizeof(vx_uint16);
                    break;
                case VX_TYPE_UINT32:
                case VX_TYPE_INT32:
                case VX_TYPE_FLOAT32:
                    strides[0] = sizeof(vx_uint32);
                    break;
                case VX_TYPE_UINT64:
                case VX_TYPE_INT64:
                case VX_TYPE_FLOAT64:
                    strides[0] = sizeof(vx_uint64);
                    break;
                default:
                    printf("ERROR: Unknown type in tensor!\n");
                    return VX_FAILURE;
            }
            vx_uint32 i;

            size = strides[0] * end[0];

            start[0] = 0;
            for (i = 1; i < num_dims; ++i)
            {
                strides[i] = strides[i-1] * end[i-1];
                start[i] = 0;
                size *= end[i];
            }
            if (size > 1024)
            {
                printf("Tensor too large, size is %d!\n", (int)size);
                status = VX_ERROR_NO_RESOURCES;
            }

            vx_uint8 data[size];
            for (i = 0; i < size; ++i)
            {
                if (i % strides[0])
                {
                    data[i] = 0;
                }
                else
                {
                    data[i] = ((i / strides[0]) & 1)? b: a;
                }
            }
            if (VX_SUCCESS == status)
            {
                status = vxCopyTensorPatch((vx_tensor)ref, num_dims, start, end, strides, data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            }
            break;
        }
        case VX_TYPE_THRESHOLD:
            status = VX_ERROR_NOT_SUPPORTED;
            break;
        case VX_TYPE_USER_DATA_OBJECT:
        {
            user_data_t user_data = {.numbers = {a, b, b, a}};
            status = vxCopyUserDataObject((vx_user_data_object)ref, 0, sizeof(user_data_t), &user_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            break;
        }
        default:
        /* Should not reach here. ERROR in TEST! Found unspecified type */
           break;
    }
    return status;
}

/* check values in a reference of the given type */
static vx_status checkValues(vx_reference ref, vx_enum type, vx_uint8 a, vx_uint8 b)
{
    vx_status status = VX_SUCCESS;
    switch (type)
    {
        case VX_TYPE_ARRAY:
        {
            vx_uint8 items[4];
            status = vxCopyArrayRange((vx_array)ref, 0, 4, sizeof(vx_uint8), &items, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            if (VX_SUCCESS == status)
            {
                if ((items[0] != a) ||
                    (items[1] != b) ||
                    (items[2] != b) ||
                    (items[3] != a))
                {
                    status = VX_FAILURE;
                }
            }
            break;
        }
        case VX_TYPE_CONVOLUTION:
        {
            vx_int16 ref_coeffs[9] = { a, b, (a << 8) + b, b, a, (b << 8) + a, a + b, a - b, a * b};
            vx_int16 coeffs[9];
            int i;
            status = vxCopyConvolutionCoefficients((vx_convolution)ref, coeffs, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            for (i = 0; i < 9 && VX_SUCCESS == status; ++i)
            {
                if (ref_coeffs[i] != coeffs[i])
                {
                    status = VX_FAILURE;
                }
            }
            break;
        }
        case VX_TYPE_DISTRIBUTION:
        {
            vx_uint8 dist[4];
            status = vxCopyDistribution((vx_distribution)ref, dist, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            if (VX_SUCCESS == status)
            {
                if ((dist[0] != a) ||
                    (dist[1] != b) ||
                    (dist[2] != b) ||
                    (dist[3] != a))
                {
                    status = VX_FAILURE;
                }
            }
            break;
        }
        case VX_TYPE_IMAGE:
            status = checkImage((vx_image)ref, a, b);
            break;
        case VX_TYPE_LUT:
        {
            vx_uint8 lut[4];
            status = vxCopyLUT((vx_lut)ref, lut, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            if (VX_SUCCESS == status)
            {
                if ((lut[0] != a) ||
                    (lut[1] != b) ||
                    (lut[2] != b) ||
                    (lut[3] != a))
                {
                    status = VX_FAILURE;
                }
            }
            break;
        }
        case VX_TYPE_MATRIX:
        {
            vx_int32 ref_coeffs[9] = { a, b, (a << 8) + b, b, a, (b << 8) + a, a + b, a - b, a * b};
            vx_int32 coeffs[9];
            int i;
            status = vxCopyMatrix((vx_matrix)ref, coeffs, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            for (i = 0; i < 9 && VX_SUCCESS == status; ++i)
            {
                if (ref_coeffs[i] != coeffs[i])
                {
                    status = VX_FAILURE;
                }
            }
            break;
        }
        case VX_TYPE_OBJECT_ARRAY:
        {
            vx_image image = (vx_image)vxGetObjectArrayItem((vx_object_array)ref, 0);
            status = checkImage(image, a, b);
            vxReleaseImage(&image);
            image = (vx_image)vxGetObjectArrayItem((vx_object_array)ref, 1);
            status |= checkImage(image, b, a);
            vxReleaseImage(&image);
            break;
        }
        case VX_TYPE_PYRAMID:
        {
            vx_image image = vxGetPyramidLevel((vx_pyramid)ref, 0);
            status = checkImage(image, a, b);
            vxReleaseImage(&image);
            image = (vx_image)vxGetPyramidLevel((vx_pyramid)ref, 1);
            status |= checkImage(image, b, a);
            vxReleaseImage(&image);
            break;
        }
        case VX_TYPE_REMAP:
            status = VX_ERROR_NOT_SUPPORTED;
            break;
        case VX_TYPE_SCALAR:
        {
            vx_uint32 value = 0;
            status = vxCopyScalar((vx_scalar)ref, &value, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            if (VX_SUCCESS == status && a << (16 + b) != value)
            {
                status = VX_FAILURE;
            }
            break;
        }
        case VX_TYPE_TENSOR:
        {
            vx_size num_dims;
            vx_enum data_type;
            vx_size size;
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryTensor((vx_tensor)ref, VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(num_dims)));
            vx_size end[num_dims];
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryTensor((vx_tensor)ref, VX_TENSOR_DIMS, &end, sizeof(end)));
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryTensor((vx_tensor)ref, VX_TENSOR_DATA_TYPE, &data_type, sizeof(data_type)));
            vx_size start[num_dims];
            vx_size strides[num_dims];
            switch (data_type)
            {
                case VX_TYPE_UINT8:
                case VX_TYPE_INT8:
                    strides[0] = sizeof(vx_uint8);
                    break;
                case VX_TYPE_UINT16:
                case VX_TYPE_INT16:
                    strides[0] = sizeof(vx_uint16);
                    break;
                case VX_TYPE_UINT32:
                case VX_TYPE_INT32:
                case VX_TYPE_FLOAT32:
                    strides[0] = sizeof(vx_uint32);
                    break;
                case VX_TYPE_UINT64:
                case VX_TYPE_INT64:
                case VX_TYPE_FLOAT64:
                    strides[0] = sizeof(vx_uint64);
                    break;
                default:
                    printf("ERROR: Unknown type in tensor!\n");
                    return VX_FAILURE;
            }
            vx_uint32 i;
            size = strides[0] * end[0];
            start[0] = 0;
            for (i = 1; i < num_dims; ++i)
            {
                strides[i] = strides[i-1] * end[i-1];
                start[i] = 0;
                size *= end[i];
            }
            if (size > 1024)
            {
                printf("Tensor too large! size is %d, #dims is %d!\n", (int)size, (int)num_dims);
                for (i = 0; i < num_dims; ++i)
                {
                    printf(" Stride %d: %d, dim %d: %d\n", (int)i, (int)strides[i], (int)i, (int)end[i]);
                }
                status = VX_ERROR_NO_RESOURCES;
            }
            vx_uint8 data[size];
            if (VX_SUCCESS == status)
            {
                status = vxCopyTensorPatch((vx_tensor)ref, num_dims, start, end, strides, data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
                for (i = 0; i < 2; i+=strides[0])
                {
                    if (data[i] != (((i / strides[0]) & 1)? b: a) )
                    {
                        status = VX_FAILURE;
                         printf("Check tensor failure at index %d, got 0x%x expected 0x%x\n", i, (int)data[i], (int)(((i / strides[0]) & 1)? b: a));
                        break;
                    }
                }
            }
            break;
        }
        case VX_TYPE_THRESHOLD:
            status = VX_ERROR_NOT_SUPPORTED;
            break;
        case VX_TYPE_USER_DATA_OBJECT:
        {
            user_data_t user_data;
            status = vxCopyUserDataObject((vx_user_data_object)ref, 0, sizeof(user_data_t), &user_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            if (VX_SUCCESS == status)
            {
                if ((user_data.numbers[0] != a) ||
                    (user_data.numbers[1] != b) ||
                    (user_data.numbers[2] != b) ||
                    (user_data.numbers[3] != a))
                {
                    status = VX_FAILURE;
                }
            }
            break;
        }
        default:
            /* Should not reach here. ERROR in TEST! Found unknown type */
        break;
    }
    return status;
}

/* Check to see if the type is currently supported */
static vx_bool is_supported(vx_enum type)
{
    switch (type)
    {
        case VX_TYPE_ARRAY:
        case VX_TYPE_CONVOLUTION:
        case VX_TYPE_DISTRIBUTION:
        case VX_TYPE_IMAGE:
        case VX_TYPE_LUT:
        case VX_TYPE_MATRIX:
        case VX_TYPE_OBJECT_ARRAY:
        case VX_TYPE_PYRAMID:
        case VX_TYPE_SCALAR:
        case VX_TYPE_TENSOR:
        case VX_TYPE_USER_DATA_OBJECT:
            return vx_true_e;
        case VX_TYPE_REMAP:
        case VX_TYPE_THRESHOLD:
        default:
        {
            return vx_false_e;
        }
   }
}


/* Test basic performance of Copy kernel for all supported types */
TEST (copySwap, testCopy)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    int i;
    for (i = 0; i < ( NUM_TYPES); ++i)
    {
        vx_enum type = type_ids[i];

        if (is_supported(type))
        {
            graph = vxCreateGraph(context);
            vx_node node;
            vx_reference example1 = createReference(context, type);
            vx_reference example2 = createReference(context, type);

            EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(example1, type, 4, 3));
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(example2, type, 2, 5));

            node = vxCopyNode(graph, example1, example2);
            /* Add copy node to graph */
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
            /* Copy node verified in graph */
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
            /* Copy node graph executed */
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
            /* Copy executed correctly */
            EXPECT_EQ_VX_STATUS(VX_SUCCESS,checkValues(example1, type, 4, 3) | checkValues(example2, type, 4, 3));

            VX_CALL(vxReleaseReference(&example1));
            VX_CALL(vxReleaseReference(&example2));
            VX_CALL(vxReleaseNode(&node));
            VX_CALL(vxReleaseGraph(&graph));
        }
    }

    /* improve test coverage */
    vx_node node;
    vx_array example1;
    vx_matrix vxmatrix1;
    /* negative test case for different input and output types */
    /* arrays and matrixes */
    graph = vxCreateGraph(context);
    example1 = vxCreateArray(context, VX_TYPE_UINT8, 4);
    vxmatrix1 = vxCreateMatrix(context, VX_TYPE_UINT8, 4, 4);
    node = vxCopyNode(graph, (vx_reference)example1, (vx_reference)vxmatrix1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleaseArray(&example1));
    VX_CALL(vxReleaseMatrix(&vxmatrix1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* negative test cases for vx_array */
    /* arrays of different sizes */
    vx_array example2;
    graph = vxCreateGraph(context);
    example1 = vxCreateArray(context, VX_TYPE_UINT8, 4);
    example2 = vxCreateArray(context, VX_TYPE_UINT8, 3);
    node = vxCopyNode(graph, (vx_reference)example1, (vx_reference)example2);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleaseArray(&example1));
    VX_CALL(vxReleaseArray(&example2));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* arrays virtual and size 0*/
    graph = vxCreateGraph(context);
    example1 = vxCreateArray(context, VX_TYPE_UINT8, 3);
    example2 = vxCreateVirtualArray(graph, VX_TYPE_UINT8, 0);
    vx_array example3 = vxCreateArray(context, VX_TYPE_UINT8, 3);
    node = vxCopyNode(graph, (vx_reference)example1, (vx_reference)example2);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    vx_node node_1 = vxCopyNode(graph, (vx_reference)example2, (vx_reference)example3);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node_1));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseNode(&node_1));
    VX_CALL(vxReleaseArray(&example1));
    VX_CALL(vxReleaseArray(&example2));
    VX_CALL(vxReleaseArray(&example3));
    VX_CALL(vxReleaseGraph(&graph));
    //array of different types
    graph = vxCreateGraph(context);
    example1 = vxCreateArray(context, VX_TYPE_FLOAT64, 4);
    example2 = vxCreateArray(context, VX_TYPE_UINT8, 4);
    node = vxCopyNode(graph, (vx_reference)example1, (vx_reference)example2);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleaseArray(&example1));
    VX_CALL(vxReleaseArray(&example2));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* negative test cases for vx_array and virtual array*/
    graph = vxCreateGraph(context);
    example1 = vxCreateArray(context, VX_TYPE_UINT8, 4);
    example2 = vxCreateVirtualArray(graph, VX_TYPE_UINT8, 0);
    node = vxCopyNode(graph, (vx_reference)example1, (vx_reference)example2);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseArray(&example1));
    VX_CALL(vxReleaseArray(&example2));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* - negative virtual stuff - different types */
    graph = vxCreateGraph(context);
    example1 = vxCreateArray(context, VX_TYPE_UINT8, 4);
    example2 = vxCreateVirtualArray(graph, VX_TYPE_UINT16, 0);
    node = vxCopyNode(graph, (vx_reference)example1, (vx_reference)example2);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleaseArray(&example1));
    VX_CALL(vxReleaseArray(&example2));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* negative virtual stuff - different capacity */
    graph = vxCreateGraph(context);
    example1 = vxCreateArray(context, VX_TYPE_UINT16, 4);
    example2 = vxCreateVirtualArray(graph, VX_TYPE_UINT16, 1);
    node = vxCopyNode(graph, (vx_reference)example1, (vx_reference)example2);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleaseArray(&example1));
    VX_CALL(vxReleaseArray(&example2));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    /* improve test coverage */
    /* vx_objarray : negative test cases */
    graph = vxCreateGraph(context);
    vx_object_array objarray_0, objarray_1;
    vx_image image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    objarray_0 = vxCreateObjectArray(context, (vx_reference)image, 4);
    objarray_1 = vxCreateObjectArray(context, (vx_reference)image, 2);
    VX_CALL(vxReleaseImage(&image));
    node = vxCopyNode(graph, (vx_reference)objarray_0, (vx_reference)objarray_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleaseObjectArray(&objarray_0));
    VX_CALL(vxReleaseObjectArray(&objarray_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));


    /* improve test coverage */
    vx_image image_0, image_1;
    /* vx_image : negative test cases */
    /* different sizes and virtual image */
    /* width */
    graph = vxCreateGraph(context);
    image_0 = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    image_1 = vxCreateVirtualImage(graph, 15, 16, VX_DF_IMAGE_U8);
    node = vxCopyNode(graph, (vx_reference)image_0, (vx_reference)image_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleaseImage(&image_0));
    VX_CALL(vxReleaseImage(&image_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* height */
    graph = vxCreateGraph(context);
    image_0 = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    image_1 = vxCreateImage(context, 16, 15, VX_DF_IMAGE_U8);
    node = vxCopyNode(graph, (vx_reference)image_0, (vx_reference)image_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleaseImage(&image_0));
    VX_CALL(vxReleaseImage(&image_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    graph = vxCreateGraph(context);
    image_0 = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    image_1 = vxCreateVirtualImage(graph, 16, 1, VX_DF_IMAGE_U8);
    node = vxCopyNode(graph, (vx_reference)image_0, (vx_reference)image_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleaseImage(&image_0));
    VX_CALL(vxReleaseImage(&image_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    graph = vxCreateGraph(context);
    image_0 = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    image_1 = vxCreateVirtualImage(graph, 16, 0, VX_DF_IMAGE_U8);
    node = vxCopyNode(graph, (vx_reference)image_0, (vx_reference)image_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseImage(&image_0));
    VX_CALL(vxReleaseImage(&image_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* formats */
    graph = vxCreateGraph(context);
    image_0 = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    image_1 = vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U16);
    node = vxCopyNode(graph, (vx_reference)image_0, (vx_reference)image_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleaseImage(&image_0));
    VX_CALL(vxReleaseImage(&image_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    graph = vxCreateGraph(context);
    image_0 = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    image_1 = vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_VIRT);
    node = vxCopyNode(graph, (vx_reference)image_0, (vx_reference)image_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseImage(&image_0));
    VX_CALL(vxReleaseImage(&image_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    /* size 0 */
    graph = vxCreateGraph(context);
    image_0 = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    image_1 = vxCreateVirtualImage(graph, 0, 16, VX_DF_IMAGE_U8);
    node = vxCopyNode(graph, (vx_reference)image_0, (vx_reference)image_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseImage(&image_0));    
    VX_CALL(vxReleaseImage(&image_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    /* improve test coverage for pyramids */
    vx_pyramid pyramid_0, pyramid_1, pyramid_2;    
    /* vx_pyramid : negative test cases */
    /* same object for intput and ouput */
    graph = vxCreateGraph(context);
    pyramid_0 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8);
    node = vxCopyNode(graph, (vx_reference)pyramid_0, (vx_reference)pyramid_0);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_FAILURE, vxVerifyGraph(graph));
    VX_CALL(vxReleasePyramid(&pyramid_0));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* different lods */    
    graph = vxCreateGraph(context);
    pyramid_0 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8);
    pyramid_1 = vxCreatePyramid(context, 2, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8);
    node = vxCopyNode(graph, (vx_reference)pyramid_0, (vx_reference)pyramid_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleasePyramid(&pyramid_0));
    VX_CALL(vxReleasePyramid(&pyramid_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* different height */    
    graph = vxCreateGraph(context);
    pyramid_0 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8);
    pyramid_1 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 1, 16, VX_DF_IMAGE_U8);
    node = vxCopyNode(graph, (vx_reference)pyramid_0, (vx_reference)pyramid_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleasePyramid(&pyramid_0));
    VX_CALL(vxReleasePyramid(&pyramid_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* different width */    
    graph = vxCreateGraph(context);
    pyramid_0 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8);
    pyramid_1 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 15, VX_DF_IMAGE_U8);
    node = vxCopyNode(graph, (vx_reference)pyramid_0, (vx_reference)pyramid_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleasePyramid(&pyramid_0));
    VX_CALL(vxReleasePyramid(&pyramid_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* positive and negative tests but with output virtual object */
    /* with width */
    graph = vxCreateGraph(context);
    pyramid_0 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 1, 16, VX_DF_IMAGE_U8);
    pyramid_1 = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 0, 16, VX_DF_IMAGE_U8);
    node = vxCopyNode(graph, (vx_reference)pyramid_0, (vx_reference)pyramid_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleasePyramid(&pyramid_0));
    VX_CALL(vxReleasePyramid(&pyramid_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    graph = vxCreateGraph(context);
    pyramid_0 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 2, 16, VX_DF_IMAGE_U8);
    pyramid_1 = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 1, 16, VX_DF_IMAGE_U8);
    node = vxCopyNode(graph, (vx_reference)pyramid_0, (vx_reference)pyramid_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleasePyramid(&pyramid_0));
    VX_CALL(vxReleasePyramid(&pyramid_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* with height */ 
    graph = vxCreateGraph(context);
    pyramid_0 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 1, VX_DF_IMAGE_U8);
    pyramid_1 = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 16, 0, VX_DF_IMAGE_U8);
    node = vxCopyNode(graph, (vx_reference)pyramid_0, (vx_reference)pyramid_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleasePyramid(&pyramid_0));
    VX_CALL(vxReleasePyramid(&pyramid_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    graph = vxCreateGraph(context);
    pyramid_0 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 2, VX_DF_IMAGE_U8);
    pyramid_1 = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 16, 1, VX_DF_IMAGE_U8);
    node = vxCopyNode(graph, (vx_reference)pyramid_0, (vx_reference)pyramid_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleasePyramid(&pyramid_0));
    VX_CALL(vxReleasePyramid(&pyramid_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* positive and negative tests with formats and virtual object*/
    graph = vxCreateGraph(context);
    pyramid_0 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 2, VX_DF_IMAGE_U8);
    pyramid_1 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 2, VX_DF_IMAGE_S32);
    node = vxCopyNode(graph, (vx_reference)pyramid_0, (vx_reference)pyramid_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleasePyramid(&pyramid_0));
    VX_CALL(vxReleasePyramid(&pyramid_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* with virtual pyramid image format positive and negative tests*/
    graph = vxCreateGraph(context);
    pyramid_0 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8);
    pyramid_1 = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_VIRT);
    node = vxCopyNode(graph, (vx_reference)pyramid_0, (vx_reference)pyramid_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleasePyramid(&pyramid_0));
    VX_CALL(vxReleasePyramid(&pyramid_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /*negative test*/
    graph = vxCreateGraph(context);
    pyramid_0 = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 16, 2, VX_DF_IMAGE_U16);//VX_DF_IMAGE_U8);
    pyramid_1 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 2, VX_DF_IMAGE_U8);//VX_DF_IMAGE_VIRT);
    node = vxCopyNode(graph, (vx_reference)pyramid_0, (vx_reference)pyramid_1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleasePyramid(&pyramid_0));
    VX_CALL(vxReleasePyramid(&pyramid_1));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    /* different objects type*/
    graph = vxCreateGraph(context);
    vx_image img = vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8);
    pyramid_0 = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, 16, 2, VX_DF_IMAGE_U16);
    node = vxCopyNode(graph, (vx_reference)pyramid_0, (vx_reference)img);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleaseImage(&img));
    VX_CALL(vxReleasePyramid(&pyramid_0));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}


/* Test basic performance of Swap kernel for all supported types */
TEST (copySwap, testSwap)
{
    vx_context context = context_->vx_context_;

    int i;
    for (i = 0; i < ( NUM_TYPES ); ++i)
    {
        vx_enum type = type_ids[i];

        if (is_supported(type))
        {
            vx_graph graph = vxCreateGraph(context);
            vx_node node;
            vx_reference example1 = createReference(context, type);
            vx_reference example2 = createReference(context, type);

            EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(example1, type, 4, 3));
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(example2, type, 2, 5));

            node = vxSwapNode(graph, example1, example2);

            /* Add swap node to graph */
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node) );
            /* Swap node verified in graph */
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
            /* Swap node graph executed */
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
            /* Swap executed correctly */
            vx_status status = checkValues(example2, type, 4, 3);
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
            status = checkValues(example1, type, 2, 5);
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);

            VX_CALL(vxReleaseReference(&example1));
            VX_CALL(vxReleaseReference(&example2));
            VX_CALL(vxReleaseNode(&node));
            VX_CALL(vxReleaseGraph(&graph));
        }
    }
}


/* Test basic performance of Move kernel for all supported types */
TEST(copySwap, testMove)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;

    int i;
    for (i = 0; i < ( NUM_TYPES); ++i)
    {
        vx_enum type = type_ids[i];

        if (is_supported(type))
        {
            ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

            vx_node node;
            vx_reference example1 = createReference(context, type);
            vx_reference example2 = createReference(context, type);

            EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(example1, type, 4, 3));
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(example2, type, 2, 5));
            node = vxMoveNode(graph, example1, example2);

            /* Add move node to graph */
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
            /* Move node verified in graph */
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
            /* Move node graph executed */
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
            /* Move executed correctly */
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkValues(example2, type, 4, 3));

            VX_CALL(vxReleaseReference(&example1));
            VX_CALL(vxReleaseReference(&example2));
            VX_CALL(vxReleaseNode(&node));
            VX_CALL(vxReleaseGraph(&graph));
        }
    }

    vx_node node;
    vx_array example1, example2;     
    //different types
    graph = vxCreateGraph(context);
    example1 = vxCreateArray(context, VX_TYPE_FLOAT64, 4);
    example2 = vxCreateArray(context, VX_TYPE_UINT8, 4);
    /* Add copy node to graph */
    node = vxMoveNode(graph, (vx_reference)example1, (vx_reference)example2);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    /* Copy node verified in graph */
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxVerifyGraph(graph));
    VX_CALL(vxReleaseArray(&example1));
    VX_CALL(vxReleaseArray(&example2));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

}


/* Test basic performance of vxuCopy kernel for all supported types */
TEST(copySwap, testVxuCopy)
{
    vx_context context = context_->vx_context_;

    int i;
    for (i = 0; i < NUM_TYPES; ++i)
    {
        vx_enum type = type_ids[i];

        if (is_supported(type))
        {
            vx_reference example1 = createReference(context, type);
            vx_reference example2 = createReference(context, type);

            EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(example1, type, 4, 3));
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(example2, type, 2, 5));
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuCopy(context, example1, example2));
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, (checkValues(example1, type, 4, 3) + checkValues(example2, type, 4, 3)));

            VX_CALL(vxReleaseReference(&example1));
            VX_CALL(vxReleaseReference(&example2));
        }
    }
}

/* Test vxu functions for images. This can be used to cover the kernel
   callback functions, as they are used for vxu implementation
*/
TEST(copySwap, testVxu)
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;

    /* Test immediate mode functions for images */
    const vx_enum type = VX_TYPE_IMAGE;
    vx_reference example1 = createReference(context, type);
    vx_reference example2 = createReference(context, type);

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(example1, type, 4, 3));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(example2, type, 2, 5));
    /* vxuCopy returned VX_SUCCESS */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuCopy(context, example1, example2));
    /* vxuCopy executed correctly */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, (checkValues(example2, type, 4, 3) || checkValues(example1, type, 4, 3)));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(example1, type, 4, 3));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(example2, type, 2, 5));
    /* vxuSwap returned VX_SUCCESS */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuSwap(context, example1, example2));
    /* vxuSwap executed correctly */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, (checkValues(example2, type, 4, 3) || checkValues(example1, type, 2, 5)));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(example1, type, 4, 3));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(example2, type, 2, 5));
    /* vxuMove returned VX_SUCCESS */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuMove(context, example1, example2));
    /* vxuMove executed correctly*/
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, (checkValues(example2, type, 4, 3)));

    /* Sub-images */
    vx_rectangle_t rect = {.start_x = 0, .start_y = 0, .end_x = 16, .end_y = 16};
    vx_rectangle_t wrongrect = {.start_x = 0, .start_y = 0, .end_x = 6, .end_y = 11};
    vx_image roi = vxCreateImageFromROI((vx_image)example1, &rect);
    vx_image wrongsizeroi = vxCreateImageFromROI((vx_image)example1, &wrongrect);

    /* vxuCopy from ROI returned VX_SUCCESS */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuCopy(context, (vx_reference)roi, example2));
    /* vxuCopy to ROI returned VX_SUCCESS */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuCopy(context, example1, (vx_reference)roi));
    /* Expected test fails*/
    /* vxuCopy to unmatched ROI did not return VX_SUCCESS */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxuCopy(context, example1, (vx_reference)wrongsizeroi));
    /* Swap from ROI did not return VX_SUCCESS */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxuSwap(context, (vx_reference)roi, example2));
    /* Move from ROI did not return VX_SUCCESS */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxuMove(context, (vx_reference)roi, example2));

    VX_CALL(vxReleaseImage(&roi));
    VX_CALL(vxReleaseImage(&wrongsizeroi));

    /* Uniform images */
    vx_pixel_value_t pixel1 = {.U8 = 119};
    vx_pixel_value_t pixel2 = {.U8 = 210};
    vx_image uni1 = vxCreateUniformImage(context, 16, 16, VX_DF_IMAGE_U8, &pixel1);
    vx_image uni2 = vxCreateUniformImage(context, 16, 16, VX_DF_IMAGE_U8, &pixel2);

    /* vxuCopy with a uniform image input returned VX_SUCCESS */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuCopy(context, (vx_reference)uni1, example2));
    /* Copied from a uniform to a normal image */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage((vx_image)example2, 119, 119));
    /* Expected Fails */
    /* Cannot copy from a normal to a uniform image */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxuCopy(context, example1, (vx_reference)uni2));
    /* Cannot copy from a uniform to a uniform image */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxuCopy(context, (vx_reference)uni1, (vx_reference)uni2));
    /* Cannot swap uniform images */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxuSwap(context, (vx_reference)uni1, (vx_reference)uni2));
    /* other direction from uniform into one normal image */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxuSwap(context, (vx_reference)uni1, (vx_reference)example1));
    /* Cannot move uniform images */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxuMove(context, (vx_reference)uni1, (vx_reference)uni2));

    VX_CALL(vxReleaseImage(&uni1));
    VX_CALL(vxReleaseImage(&uni2));
    VX_CALL(vxReleaseReference(&example1));
    VX_CALL(vxReleaseReference(&example2));

    /* negative testing for scalar */
    vx_uint32 value = 0;
    example1 = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &value);
    example2 = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT16, &value);
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxuCopy(context, example1, example2));
    VX_CALL(vxReleaseReference(&example1));
    VX_CALL(vxReleaseReference(&example2));

    /* negative test for tensor */
    vx_size dims1[3] = {4, 4, 4};
    example1 = (vx_reference)vxCreateTensor(context, 3, dims1, VX_TYPE_INT32, 0);
    example2 = (vx_reference)vxCreateTensor(context, 3, dims1, VX_TYPE_INT16, 0);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxuSwap(context, example1, example2));
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxuCopy(context, example1, example2));
    VX_CALL(vxReleaseReference(&example1));
    VX_CALL(vxReleaseReference(&example2));

    /* negative tests for pyramid that cannot be reached with a graph */
    /* same object in and out */
    example1 = (vx_reference)vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 16, 2, VX_DF_IMAGE_U16);
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxuCopy(context, example1, example1));
    VX_CALL(vxReleaseReference(&example1));
    /* copy of different generic objects type who are tested with the tivxIsReferenceMetaFormatEqual*/
    example1 = (vx_reference)vxCreateLUT(context, VX_TYPE_UINT8, 4);
    example2 = (vx_reference)vxCreateLUT(context, VX_TYPE_UINT8, 8);
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxuCopy(context, example1, example2));
    VX_CALL(vxReleaseReference(&example1));
    VX_CALL(vxReleaseReference(&example2));
}

/* Check sub-image. This must cover sub-images
   of sub-images as well as covering both from channel and from roi
*/
TEST (copySwap, testSubObjectsOfImages )
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;

    vx_image images[] = {
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_IYUV),
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_IYUV),
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8)
    };

    vx_rectangle_t rect1 = {.start_x = 0, .start_y = 2, .end_x = 10, .end_y = 10};
    vx_rectangle_t rect0 = {.start_x = 0, .start_y = 0, .end_x = 10, .end_y = 8};
    vx_image sub_images[] = {
        vxCreateImageFromChannel(images[0], VX_CHANNEL_Y),
        vxCreateImageFromChannel(images[1], VX_CHANNEL_Y),
        vxCreateImageFromROI(images[2], &rect1),
        vxCreateImageFromROI(images[3], &rect1)
    };

    writeImage(images[2], 0x33, 0x9c);
    writeImage(images[3], 0x42, 0xab);

    writeImage(sub_images[0], 0x15, 0x7e);
    writeImage(sub_images[1], 0x24, 0x8d);
    writeImage(sub_images[2], 0x9c, 0x12);
    writeImage(sub_images[3], 0xab, 0x34);

    vx_image sub_sub_images[] = {
        vxCreateImageFromROI(sub_images[0], &rect1),
        vxCreateImageFromROI(sub_images[1], &rect1),
        vxCreateImageFromROI(sub_images[2], &rect0),
        vxCreateImageFromROI(sub_images[3], &rect0)
    };
    writeImage(sub_sub_images[0], 0x7e, 0x45);
    writeImage(sub_sub_images[1], 0x8d, 0x56);

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuSwap(context, (vx_reference)images[0], (vx_reference)images[1]));
    /* Images from channel correctly Swapped */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_images[0], 0x24, 0x8d) | checkImage(sub_images[1], 0x15, 0x7e));
    /* Images from ROI of Images from channel correctly Swapped */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_sub_images[0], 0x8d, 0x56) | checkImage(sub_sub_images[1], 0x7e, 0x45));

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuSwap(context, (vx_reference)images[2], (vx_reference)images[3]));
    /* Images from ROI correctly Swapped */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_images[2], 0xab, 0x34) | checkImage(sub_images[3], 0x9c, 0x12));
    /* Images from ROI of images from ROI correctly Swapped */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_sub_images[2], 0xab, 0x34) | checkImage(sub_sub_images[3], 0x9c, 0x12));

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuMove(context, (vx_reference)images[0], (vx_reference)images[1]));
    /* Images from channel correctly Moved */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_images[1], 0x24, 0x8d) | checkImage(sub_images[0], 0x15, 0x7e));
    /* Images from ROI of Images from channel correctly Moved */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_sub_images[1], 0x8d, 0x56) | checkImage(sub_sub_images[0], 0x7e, 0x45));

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuMove(context, (vx_reference)images[3], (vx_reference)images[2]));
    /* Images from ROI correctly Moved */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_images[3], 0xab, 0x34) | checkImage(sub_images[2], 0x9c, 0x12));
    /* Images from ROI of images from ROI correctly Moved */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_sub_images[3], 0xab, 0x34) | checkImage(sub_sub_images[2], 0x9c, 0x12));

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuCopy(context, (vx_reference)images[1], (vx_reference)images[0]));
    /* Images from channel correctly Moved */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_images[1], 0x24, 0x8d) | checkImage(sub_images[0], 0x24, 0x8d));
    /* Images from ROI of Images from channel correctly Copied */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_sub_images[1], 0x8d, 0x56) | checkImage(sub_sub_images[0], 0x8d, 0x56));

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuCopy(context, (vx_reference)images[3], (vx_reference)images[2]));
    /* Images from ROI correctly Copied */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_images[3], 0xab, 0x34) | checkImage(sub_images[2], 0xab, 0x34));
    /* Images from ROI of images from ROI correctly Copied */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_sub_images[3], 0xab, 0x34) | checkImage(sub_sub_images[2], 0xab, 0x34));

    vx_uint32 i;
    for (i = 0; i < 4; ++i)
    {
        VX_CALL(vxReleaseImage(&sub_sub_images[i]));
        VX_CALL(vxReleaseImage(&sub_images[i]));
        VX_CALL(vxReleaseImage(&images[i]));
    }
}

/* Check sub-image from tensor created from vxCreateTensorFromROI
*/
TEST (copySwap, testSubObjectsOfTensors )
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;

    vx_image images[] = {
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8)
    };

    vx_rectangle_t rect1 = {.start_x = 0, .start_y = 0, .end_x = 10, .end_y = 10};
    vx_rectangle_t rect0 = {.start_x = 0, .start_y = 0, .end_x = 10, .end_y = 8};

    writeImage(images[0], 0x15, 0x7e);
    writeImage(images[1], 0x24, 0x8d);
    writeImage(images[2], 0x33, 0x9c);
    writeImage(images[3], 0x42, 0xab);

    vx_tensor tensors[2];
    tensors[0] = vxCreateTensorFromROI(images[0], NULL, 0);
    tensors[1] = vxCreateTensorFromROI(images[1], NULL, 0);

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuSwap(context, (vx_reference)images[0], (vx_reference)images[1]));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkValues(vxCastRefFromTensor(tensors[0]), VX_TYPE_TENSOR, 0x24, 0x8d));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkValues(vxCastRefFromTensor(tensors[1]), VX_TYPE_TENSOR, 0x15, 0x7e));

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkValues(vxCastRefFromImage(images[0]), VX_TYPE_IMAGE, 0x24, 0x8d));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkValues(vxCastRefFromImage(images[1]), VX_TYPE_IMAGE, 0x15, 0x7e));      

    VX_CALL(vxReleaseTensor(&tensors[0]));
    VX_CALL(vxReleaseTensor(&tensors[1]));
    vx_uint32 i;
    for (i = 0; i < 4; ++i)
    {
        VX_CALL(vxReleaseImage(&images[i]));
    }
}

/* Check max of sub-image of sub image */
TEST (copySwap, testSubObjectsMaxOfSubImages )
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;
    vx_image parent_image_1, parent_image_2;
    vx_image sub_images_1[TIVX_IMAGE_MAX_SUBIMAGES];
    vx_image sub_images_2[TIVX_IMAGE_MAX_SUBIMAGES];
    vx_image sub_sub_images_1 [TIVX_IMAGE_MAX_SUBIMAGES][TIVX_IMAGE_MAX_SUBIMAGES];
    vx_image sub_sub_images_2 [TIVX_IMAGE_MAX_SUBIMAGES][TIVX_IMAGE_MAX_SUBIMAGES];
    vx_image sub_sub_sub_image_1;
    vx_image yuv_image[TIVX_IMAGE_MAX_SUBIMAGE_DEPTH+1U];

    vx_rectangle_t rect0 = {.start_x = 0, .start_y = 0, .end_x = 10, .end_y = 10};
    vx_rectangle_t rect1 = {.start_x = 0, .start_y = 0, .end_x = 8, .end_y = 8};

    ASSERT_VX_OBJECT(parent_image_1 = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(parent_image_2 = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    writeImage(parent_image_1, 0x33, 0x9c);
    writeImage(parent_image_2, 0x42, 0xab);

    vx_uint32 i;
    for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGES; ++i)
    {
        ASSERT_VX_OBJECT(sub_images_1[i] = vxCreateImageFromROI(parent_image_1, &rect0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(sub_images_2[i] = vxCreateImageFromROI(parent_image_2, &rect0), VX_TYPE_IMAGE);
    }
    
    /* max out the number of possible subimages */
    vx_uint32 j;
    for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGES; i++)
    {
        for (j = 0; j < TIVX_IMAGE_MAX_SUBIMAGES; j++)
        {
            ASSERT_VX_OBJECT(sub_sub_images_1[i][j] = vxCreateImageFromROI(sub_images_1[i], &rect1), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(sub_sub_images_2[i][j] = vxCreateImageFromROI(sub_images_2[i], &rect1), VX_TYPE_IMAGE);
        }
    }

    EXPECT_EQ_VX_STATUS(VX_SUCCESS,vxuSwap(context, vxCastRefFromImage(parent_image_1), vxCastRefFromImage(parent_image_2)));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(parent_image_1, 0x42, 0xab) | checkImage(parent_image_2, 0x33, 0x9c));

    for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGES; i++)
    {
        for (j = 0; j < TIVX_IMAGE_MAX_SUBIMAGES; j++)
        {
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_sub_images_1[i][j], 0x42, 0xab) | checkImage(sub_sub_images_2[i][j], 0x33, 0x9c));
        }
    }

    /* test with a graph and a copy node*/
    vx_graph graph;
    vx_node node;    
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    node = vxCopyNode(graph, vxCastRefFromImage(parent_image_1), vxCastRefFromImage(parent_image_2));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus(vxCastRefFromNode(node)));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(parent_image_2, 0x42, 0xab));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_images_2[15], 0x42, 0xab));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(sub_sub_images_2[10][10], 0x42, 0xab));

    /* add a negative test by adding a last element as a child of the last element
     in fact bigger than TIVX_IMAGE_MAX_SUBIMAGE_DEPTH */
    sub_sub_sub_image_1 = vxCreateImageFromROI(sub_sub_images_1[TIVX_IMAGE_MAX_SUBIMAGES-1][TIVX_IMAGE_MAX_SUBIMAGES-1], &rect1);
    /* get if this is a valid object */
    EXPECT_EQ_VX_STATUS(VX_ERROR_NO_RESOURCES, vxGetStatus(vxCastRefFromImage(sub_sub_sub_image_1)));

    /* release the graph's stuff*/
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGES; i++)
    {
        for (j = 0; j < TIVX_IMAGE_MAX_SUBIMAGES; j++)
        {
            VX_CALL(vxReleaseImage(&sub_sub_images_1[i][j]));
            VX_CALL(vxReleaseImage(&sub_sub_images_2[i][j]));
        }
    }
    for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGES; ++i)
    {
        VX_CALL(vxReleaseImage(&sub_images_1[i]));
        VX_CALL(vxReleaseImage(&sub_images_2[i]));
    }
    VX_CALL(vxReleaseImage(&parent_image_1));
    VX_CALL(vxReleaseImage(&parent_image_2));

    /* add a negative test for creating image from channel*/
    ASSERT_VX_OBJECT(parent_image_1 = vxCreateImage(context, 16, 16, VX_DF_IMAGE_IYUV), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(yuv_image[0U] = vxCreateImageFromChannel(parent_image_1, VX_CHANNEL_Y), VX_TYPE_IMAGE);
    for (i = 1; i < TIVX_IMAGE_MAX_SUBIMAGE_DEPTH; ++i)
    {
        ASSERT_VX_OBJECT(yuv_image[i] = vxCreateImageFromROI(yuv_image[i-1], &rect0), VX_TYPE_IMAGE);
    }
    /* last level cannot be created */
    yuv_image[TIVX_IMAGE_MAX_SUBIMAGE_DEPTH] = vxCreateImageFromROI(yuv_image[TIVX_IMAGE_MAX_SUBIMAGE_DEPTH-1], &rect0);
    EXPECT_EQ_VX_STATUS(VX_ERROR_NO_RESOURCES, vxGetStatus(vxCastRefFromImage(yuv_image[TIVX_IMAGE_MAX_SUBIMAGE_DEPTH])));

    /* release the images */
    VX_CALL(vxReleaseImage(&parent_image_1));
    for (i = 0; i < TIVX_IMAGE_MAX_SUBIMAGE_DEPTH; ++i)
    {
        VX_CALL(vxReleaseImage(&yuv_image[i]));
    }      
}
/* check node removal on verification
    When the node has been disconnected, vxGetParameterByIndex should return an error object,
    and the number of nodes in the graph will have decreased by one.
    This function verifies the given graph and returns VX_SUCCESS if the current number of nodes
    decreases, and the given node subsequently cannot return a parameter from its first index.
    It will return a non-zero value in the case of any error, or if the node was not removed.
*/
vx_status nodeIsOptimised(vx_graph graph, vx_node node)
{
    vx_uint32 original_num_nodes = 0;
    vx_uint32 new_num_nodes = 0;

    vx_status status = VX_SUCCESS;

    vx_parameter parameter = vxGetParameterByIndex(node, 0);
    
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)parameter));
    vxReleaseParameter(&parameter);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryGraph(graph, VX_GRAPH_NUMNODES, &original_num_nodes, sizeof(original_num_nodes)));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryGraph(graph, VX_GRAPH_NUMNODES, &new_num_nodes, sizeof(new_num_nodes)));
    
    if (VX_SUCCESS == status)
    {
        parameter = vxGetParameterByIndex(node, 0);
        status = vxGetStatus((vx_reference)parameter);
        if (VX_SUCCESS == status)
        {
            vxReleaseParameter(&parameter);
            status = VX_FAILURE;
        }
        if (new_num_nodes < original_num_nodes)
        {
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_FAILURE;
        }
//        printf("Nodes optimisation: original nodes: %d, new nodes: %d\n", original_num_nodes, new_num_nodes);
    }
    return status;
}

/*  Test copy node optimisation
    An input copy node should be disconnected if its output is virtual and can't be modified
    An input copy node won't be removed if it's output can be modified
    An output copy node should be disconnected if its input is virtual and not connected to another input
    An output copy node won't be removed if its input is connected to another input
    An embedded copy node should be removed if the input is not connected to another input
    An embedded copy node won't be removed is the input is connected to another input
*/
TEST(copySwap, testCopyRemoval)
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;

#if 1
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    int i;
    vx_image images[] =
    {
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8),
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8),
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8),
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8)
    };

    /* Create a graph with an input copy node with a fan-out of 2*/
    vx_node nodes[] =
    {
        vxCopyNode(graph, (vx_reference)images[0], (vx_reference)images[1]),
        vxNotNode(graph, images[1], images[2]),
        vxNotNode(graph, images[1], images[3]),
        NULL, NULL
    };
    writeImage(images[0], 23, 45);
    addParameterToGraph(graph, nodes[0], 0);

    /* Simple input copy node is removed */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes[0]));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    /* Graph executes correctly after input Copy node removal */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(images[2], 255-23, 255-45) | checkImage(images[3], 255-23, 255-45));

    writeImage(images[6], 12, 34);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetGraphParameterByIndex(graph, 0, (vx_reference)images[6]));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    /* Graph executes correctly after vxSetGraphParameterByIndex to set input parameter */
    status = checkImage(images[2], 255-12, 255-34);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    status = checkImage(images[3], 255-12, 255-34);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);

    VX_CALL(vxReleaseNode(&nodes[0]));
    VX_CALL(vxReleaseNode(&nodes[1]));
    VX_CALL(vxReleaseNode(&nodes[2]));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    nodes[0] = vxCopyNode(graph, (vx_reference)images[0], (vx_reference)images[1]);
    nodes[1] = vxAccumulateWeightedImageNodeX(graph, images[2], 1.0, images[1]);
    nodes[2] = vxNotNode(graph,images[1], images[3]);
    writeImage(images[2], 255, 255);
    /* Input copy node will not be removed if its output may be modified */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes[0]));

    VX_CALL(vxReleaseNode(&nodes[0]));
    VX_CALL(vxReleaseNode(&nodes[1]));
    VX_CALL(vxReleaseNode(&nodes[2]));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    nodes[0] = vxNotNode(graph, images[0], images[1]);
    nodes[1] = vxCopyNode(graph, (vx_reference)images[1], (vx_reference)images[2]);
    writeImage(images[2], 0, 0);
    addParameterToGraph(graph, nodes[1], 1);
    /* Simple output copy node is removed */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes[1]));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    /* Graph executes correctly after output Copy node removal */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(images[2], 255-23, 255-45));

    writeImage(images[6], 0, 0);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetGraphParameterByIndex(graph, 0, (vx_reference)images[6]));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    /* Graph executes correctly after vxSetGraphParameterByIndex to set output parameter */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(images[6], 255-23, 255-45));

    VX_CALL(vxReleaseNode(&nodes[0]));
    VX_CALL(vxReleaseNode(&nodes[1]));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    nodes[0] = vxNotNode(graph,images[0], images[1]);
    nodes[1] = vxCopyNode(graph,(vx_reference)images[1], (vx_reference)images[3]);
    nodes[2] = vxCopyNode(graph, (vx_reference)images[1], (vx_reference)images[2]);

    /* Output copy node will not be removed if its input is connected to another copy node input */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes[2]));
    VX_CALL(vxReleaseNode(&nodes[0]));
    VX_CALL(vxReleaseNode(&nodes[1]));
    VX_CALL(vxReleaseNode(&nodes[2]));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    nodes[0] = vxNotNode(graph, images[0], images[1]);
    nodes[1] = vxCopyNode(graph, (vx_reference)images[1], (vx_reference)images[4]);
    nodes[2] = vxNotNode(graph, images[4], images[2]);
    writeImage(images[2], 0, 0);

    /* Embedded copy node will be removed if its input is not shared with another copy node */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes[1]));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    /* Graph executes correctly after embedded copy node removal */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(images[2], 23, 45));

    VX_CALL(vxReleaseNode(&nodes[0]));
    VX_CALL(vxReleaseNode(&nodes[1]));
    VX_CALL(vxReleaseNode(&nodes[2]));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    nodes[0] = vxNotNode(graph, images[0], images[1]);
    nodes[1] = vxCopyNode(graph, (vx_reference)images[1], (vx_reference)images[4]);     /* Should not be removed */
    nodes[2] = vxNotNode(graph, images[4], images[2]);
    nodes[3] = vxCopyNode(graph, (vx_reference)images[1], (vx_reference)images[5]);     /* Should not be removed */
    nodes[4] = vxNotNode(graph, images[5], images[3]);
    writeImage(images[2], 0, 0);
    writeImage(images[3], 0, 0);

    /* Embedded copy node will not be removed if its input is shared with another copy node */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes[1]));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    /* Graph still runs correctly */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(images[2], 23, 45) | checkImage(images[3], 23, 45));

    for (i = 0; i < dimof(nodes); ++i)
    {
        VX_CALL(vxReleaseNode(&nodes[i]));
    }
    VX_CALL(vxReleaseGraph(&graph));

    for (i = 0; i < dimof(images); ++i)
    {
        VX_CALL(vxReleaseImage(&images[i]));
    }
#endif
}

TEST(copySwap, testNoCopyRemovalSubObjects)
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Test copy node not removed when one image is an ROI */
    vx_rectangle_t full = {.start_x = 0, .start_y = 0, .end_x = 16, .end_y = 16};
    vx_image parent = vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U8);
    vx_image images1[] =
    {
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),
        vxCreateImageFromROI(parent, &full),
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8)
    };
    vx_node nodes1[] = 
    {
        vxNotNode(graph, images1[0], parent),
        vxCopyNode(graph, vxCastRefFromImage(images1[1]), vxCastRefFromImage(images1[2]))
    };
    EXPECT_NE_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes1[1]));
    int i;
    vxReleaseImage(&parent);
    for (i = 0; i < dimof(images1); ++i)
    {
        vxReleaseImage(&images1[i]);
    }
    for (i = 0; i < dimof(nodes1); ++i)
    {
        vxReleaseNode(&nodes1[i]);
    }
    vxReleaseGraph(&graph);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Test copy not removed when one image is from a channel */
    ASSERT_VX_OBJECT(parent = vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_NV21), VX_TYPE_IMAGE);
    vx_image images2[] =
    {
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),
        vxCreateImageFromChannel(parent, VX_CHANNEL_Y),
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8)
    };
    vx_node nodes2[] = 
    {
        vxNotNode(graph, images2[0], parent),
        vxCopyNode(graph, vxCastRefFromImage(images2[1]), vxCastRefFromImage(images2[2]))
    };
    EXPECT_NE_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes2[1]));
    vxReleaseImage(&parent);
    for (i = 0; i < dimof(images2); ++i)
    {
        vxReleaseImage(&images2[i]);
    }
    for (i = 0; i < dimof(nodes2); ++i)
    {
        vxReleaseNode(&nodes2[i]);
    }
    vxReleaseGraph(&graph);
}

TEST(copySwap, testNoCopyRemovalContainersDiffer)
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    int i;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Test copy node not removed when one image is in a pyramid and the other is in an object array */
    vx_image exemplar = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    vx_pyramid parent_pyramid = vxCreateVirtualPyramid(graph, 2, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8);
    vx_object_array parent_array = vxCreateVirtualObjectArray(graph, vxCastRefFromImage(exemplar), 2);
    vx_image images1[] =
    {
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),
        vxGetPyramidLevel(parent_pyramid, 0),
        vxCastRefAsImage(vxGetObjectArrayItem(parent_array, 0), NULL),
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8)
    };
    vx_node nodes1[] = 
    {
        vxNotNode(graph, images1[0], images1[1]),
        vxCopyNode(graph, vxCastRefFromImage(images1[1]), vxCastRefFromImage(images1[2])),
        vxNotNode(graph, images1[2], images1[3])
    };
    EXPECT_NE_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes1[1]));

    /* Now test it the other way around */
    vxSetParameterByIndex(nodes1[0], 1, vxCastRefFromImage(images1[2]));
    vxSetParameterByIndex(nodes1[1], 0, vxCastRefFromImage(images1[2]));
    vxSetParameterByIndex(nodes1[1], 1, vxCastRefFromImage(images1[1]));
    vxSetParameterByIndex(nodes1[2], 0, vxCastRefFromImage(images1[1]));

    EXPECT_NE_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes1[1]));

    vxReleaseImage(&exemplar);
    vxReleasePyramid(&parent_pyramid);
    vxReleaseObjectArray(&parent_array);
    for (i = 0; i < dimof(images1); ++i)
    {
        vxReleaseImage(&images1[i]);
    }
    for (i = 0; i < dimof(nodes1); ++i)
    {
        vxReleaseNode(&nodes1[i]);
    }
    vxReleaseGraph(&graph);
}

TEST(copySwap, testNoCopyRemovalPyramidLevel)
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    int i;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Test copy node not removed when one images are in pyramids but one is not level 0 */
    vx_pyramid parent_pyramid1 = vxCreateVirtualPyramid(graph, 2, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8);
    vx_pyramid parent_pyramid2 = vxCreateVirtualPyramid(graph, 2, VX_SCALE_PYRAMID_HALF, 32, 32, VX_DF_IMAGE_U8);
    vx_image images1[] =
    {
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),
        vxGetPyramidLevel(parent_pyramid1, 0),
        vxGetPyramidLevel(parent_pyramid2, 1),
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8)
    };
    vx_node nodes1[] = 
    {
        vxNotNode(graph, images1[0], images1[1]),
        vxCopyNode(graph, vxCastRefFromImage(images1[1]), vxCastRefFromImage(images1[2])),
        vxNotNode(graph, images1[2], images1[3])
    };
    EXPECT_NE_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes1[1]));

    /* Now test it the other way around */
    vxSetParameterByIndex(nodes1[0], 1, vxCastRefFromImage(images1[2]));
    vxSetParameterByIndex(nodes1[1], 0, vxCastRefFromImage(images1[2]));
    vxSetParameterByIndex(nodes1[1], 1, vxCastRefFromImage(images1[1]));
    vxSetParameterByIndex(nodes1[2], 0, vxCastRefFromImage(images1[1]));

    EXPECT_NE_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes1[1]));

    vxReleasePyramid(&parent_pyramid1);
    vxReleasePyramid(&parent_pyramid2);
    for (i = 0; i < dimof(images1); ++i)
    {
        vxReleaseImage(&images1[i]);
    }
    for (i = 0; i < dimof(nodes1); ++i)
    {
        vxReleaseNode(&nodes1[i]);
    }
    vxReleaseGraph(&graph);
}

TEST(copySwap, testNoCopyRemovalArrayItem)
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    int i;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Test copy node not removed when objects are in arrays but one is not the first item */
    vx_image exemplar = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
    vx_object_array parent_array1 = vxCreateVirtualObjectArray(graph, vxCastRefFromImage(exemplar), 2);
    vx_object_array parent_array2 = vxCreateVirtualObjectArray(graph, vxCastRefFromImage(exemplar), 2);
    vx_image images1[] =
    {
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),
        vxCastRefAsImage(vxGetObjectArrayItem(parent_array1, 0), NULL),
        vxCastRefAsImage(vxGetObjectArrayItem(parent_array2, 1), NULL),
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8)
    };
    vx_node nodes1[] = 
    {
        vxNotNode(graph, images1[0], images1[1]),
        vxCopyNode(graph, vxCastRefFromImage(images1[1]), vxCastRefFromImage(images1[2])),
        vxNotNode(graph, images1[2], images1[3])
    };
    EXPECT_NE_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes1[1]));

    /* Now test it the other way around */
    vxSetParameterByIndex(nodes1[0], 1, vxCastRefFromImage(images1[2]));
    vxSetParameterByIndex(nodes1[1], 0, vxCastRefFromImage(images1[2]));
    vxSetParameterByIndex(nodes1[1], 1, vxCastRefFromImage(images1[1]));
    vxSetParameterByIndex(nodes1[2], 0, vxCastRefFromImage(images1[1]));

    EXPECT_NE_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes1[1]));

    vxReleaseImage(&exemplar);
    vxReleaseObjectArray(&parent_array1);
    vxReleaseObjectArray(&parent_array2);
    for (i = 0; i < dimof(images1); ++i)
    {
        vxReleaseImage(&images1[i]);
    }
    for (i = 0; i < dimof(nodes1); ++i)
    {
        vxReleaseNode(&nodes1[i]);
    }
    vxReleaseGraph(&graph);
}

TEST(copySwap, testNoCopyRemovalBidirectional)
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    int i;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Test copy node is removed when it has no sibling and the ouput is connected to a bidirectional */
    vx_image images1[] =
    {
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U8),
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8)
    };
    vx_node nodes1[] = 
    {
        vxNotNode(graph, images1[0], images1[1]),
        vxCopyNode(graph, vxCastRefFromImage(images1[1]), vxCastRefFromImage(images1[2])),
        vxAccumulateWeightedImageNodeX(graph, images1[0], 1.0, images1[2]),
        vxNotNode(graph, images1[2], images1[3])
    };

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes1[1]));

    for (i = 0; i < dimof(images1); ++i)
    {
        vxReleaseImage(&images1[i]);
    }
    for (i = 0; i < dimof(nodes1); ++i)
    {
        vxReleaseNode(&nodes1[i]);
    }
    vxReleaseGraph(&graph);

    /* Now add a sibling and check that the node is not removed */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    vx_image images2[] =
    {
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U8),
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8),
        vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8)
    };
    vx_node nodes2[] = 
    {
        vxNotNode(graph, images2[0], images2[1]),
        vxCopyNode(graph, vxCastRefFromImage(images2[1]), vxCastRefFromImage(images2[2])),
        vxNotNode(graph, images2[1], images2[4]),
        vxAccumulateWeightedImageNodeX(graph, images2[0], 1.0, images2[2]),
        vxNotNode(graph, images2[2], images2[3])
    };

    EXPECT_NE_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes2[1]));

    for (i = 0; i < dimof(images2); ++i)
    {
        vxReleaseImage(&images2[i]);
    }
    for (i = 0; i < dimof(nodes2); ++i)
    {
        vxReleaseNode(&nodes2[i]);
    }
    vxReleaseGraph(&graph);
    
}

TEST(copySwap, testMaxNodes)
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    vx_image images[16];
    vx_image virtual_images[2];
    int i;
    for (i = 0; i < 16; ++i)
    {
        ASSERT_VX_OBJECT(images[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    for (i = 0; i < 2; ++i)
    {
        ASSERT_VX_OBJECT(virtual_images[i] = vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    vx_node nodes[] = 
    {
        vxNotNode(graph, images[0], images[1]),
        vxNotNode(graph, images[1], images[2]),
        vxNotNode(graph, images[1], images[3]),
        vxNotNode(graph, images[1], images[4]),
        vxNotNode(graph, images[1], images[5]),
        vxNotNode(graph, images[1], images[6]),
        vxNotNode(graph, images[1], images[7]),
        vxNotNode(graph, images[1], images[8]),
        vxCopyNode(graph, vxCastRefFromImage(images[1]), vxCastRefFromImage(virtual_images[0])),
        vxCopyNode(graph, vxCastRefFromImage(virtual_images[0]), vxCastRefFromImage(virtual_images[1])),
        vxNotNode(graph, virtual_images[1], images[9]),
        vxNotNode(graph, virtual_images[1], images[10]),
        vxNotNode(graph, virtual_images[1], images[11]),
        vxNotNode(graph, virtual_images[1], images[12]),
        vxNotNode(graph, virtual_images[1], images[13]),
        vxNotNode(graph, virtual_images[1], images[14]),
        vxNotNode(graph, virtual_images[1], images[15])
    };
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    for (i = 0; i < 16; ++i)
    {
        VX_CALL(vxReleaseImage(&images[i]));
    }
    for (i = 0; i < 2; ++i)
    {
        VX_CALL(vxReleaseImage(&virtual_images[i]));
    }
    for (i = 0; i < dimof(nodes); ++i)
    {
        VX_CALL(vxReleaseNode(&nodes[i]));
    }
    VX_CALL(vxReleaseGraph(&graph));
}
/* Test move node optimisation
    An input move node (with virtual output) should be removed
    An output move node (with virtual input) should be removed
    Embedded move nodes should be removed
    Move nodes where both parameters are non-virtual should not be removed
*/
TEST(copySwap, testMoveRemoval)
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    int i;
    vx_image images[] =
    {
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8),
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8),
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8)
    };

    /* Create a graph with an input move node with a fan-out of 2*/
    vx_node nodes[] =
    {
        vxMoveNode(graph, (vx_reference)images[0], (vx_reference)images[1]),
        vxNotNode(graph, images[1], images[2]),
        vxNotNode(graph, images[1], images[3]),
        NULL, NULL
    };

    /* Simple input move node is removed */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes[0]));

    writeImage(images[0], 23, 45);
    writeImage(images[2], 0, 0);
    writeImage(images[3], 0, 0);

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    /* Graph executes correctly after input move node removal */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(images[2], 255-23, 255-45) | checkImage(images[3], 255-23, 255-45));

    VX_CALL(vxReleaseNode(&nodes[0]));
    VX_CALL(vxReleaseNode(&nodes[1]));
    VX_CALL(vxReleaseNode(&nodes[2]));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    nodes[0] = vxMoveNode(graph, (vx_reference)images[0], (vx_reference)images[1]);
    nodes[1] = vxAccumulateWeightedImageNodeX(graph, images[2], 1.0, images[1]);
    nodes[2] = vxNotNode(graph,images[1], images[3]);
    writeImage(images[2], 255, 255);

    /* Input move node will be removed even if its output may be modified */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes[0]));

    VX_CALL(vxReleaseNode(&nodes[0]));
    VX_CALL(vxReleaseNode(&nodes[1]));
    VX_CALL(vxReleaseNode(&nodes[2]));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    nodes[0] = vxMoveNode(graph, (vx_reference)images[0], (vx_reference)images[1]);
    nodes[1] = vxNotNode(graph, images[1], images[2]);
    nodes[2] = vxNotNode(graph,images[0], images[3]);
    writeImage(images[2], 255, 255);

    /* Input move node must not share its bidirectional parameter with the input of another node */
    status = vxVerifyGraph(graph);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, status);

    VX_CALL(vxReleaseNode(&nodes[0]));
    VX_CALL(vxReleaseNode(&nodes[1]));
    VX_CALL(vxReleaseNode(&nodes[2]));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    nodes[0] = vxNotNode(graph, images[0], images[1]);
    nodes[1] = vxMoveNode(graph, (vx_reference)images[1], (vx_reference)images[2]);
    writeImage(images[2], 0, 0);

    /* Simple output move node is removed */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes[1]));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    /* Graph executes correctly after output move node removal */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(images[2], 255-23, 255-45));

    VX_CALL(vxReleaseNode(&nodes[0]));
    VX_CALL(vxReleaseNode(&nodes[1]));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    nodes[0] = vxNotNode(graph, images[0], images[1]);
    nodes[1] = vxMoveNode(graph,(vx_reference)images[1], (vx_reference)images[2]);
    nodes[2] = vxCopyNode(graph,(vx_reference)images[1], (vx_reference)images[3]);

    /* Output move node must not share its bidirectional parameter with the input of another node */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&nodes[0]));
    VX_CALL(vxReleaseNode(&nodes[1]));
    VX_CALL(vxReleaseNode(&nodes[2]));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    nodes[0] = vxNotNode(graph, images[0], images[1]);
    nodes[1] = vxMoveNode(graph, (vx_reference)images[1], (vx_reference)images[4]);
    nodes[2] = vxNotNode(graph, images[4], images[2]);
    writeImage(images[2], 0, 0);

    /* Embedded move node will be removed if its bidirectional is not shared with another node's input */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes[1]));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    /* Graph executes correctly after embedded move node removal */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(images[2], 23, 45));

    VX_CALL(vxReleaseNode(&nodes[0]));
    VX_CALL(vxReleaseNode(&nodes[1]));
    VX_CALL(vxReleaseNode(&nodes[2]));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    nodes[0] = vxNotNode(graph, images[0], images[1]);
    nodes[1] = vxMoveNode(graph, (vx_reference)images[1], (vx_reference)images[4]);
    nodes[2] = vxNotNode(graph, images[4], images[2]);
    nodes[3] = vxCopyNode(graph, (vx_reference)images[1], (vx_reference)images[5]); /* makes move node illegal */
    nodes[4] = vxNotNode(graph, images[5], images[3]);
    writeImage(images[2], 0, 0);
    writeImage(images[3], 0, 0);

    /* Embedded move node must not share its bidirectional parameter with the input of another node */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    for (i = 0; i < dimof(nodes); ++i)
    {
        VX_CALL(vxReleaseNode(&nodes[i]));
    }
    VX_CALL(vxReleaseGraph(&graph));

    for (i = 0; i < dimof(images); ++i)
    {
        VX_CALL(vxReleaseImage(&images[i]));
    }
}

TEST(copySwap, testMoveRemovalLimits)
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    int i;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    vx_image images_0[] =
    {
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8), //0
        vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8), //1
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8), //2
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8), //3
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8), //4
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8), //5
        vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8), //6
    };

    vx_node nodes_0[] =
    {
        vxMoveNode(graph, (vx_reference)images_0[0], (vx_reference)images_0[1]),
        vxNotNode(graph, images_0[1], images_0[2]),
        vxNotNode(graph, images_0[1], images_0[3]),
        vxMoveNode(graph, (vx_reference)images_0[5], (vx_reference)images_0[6]),
        vxOrNode(graph, images_0[3], images_0[6], images_0[4])
    };
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, nodeIsOptimised(graph, nodes_0[0]));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    
    for (i = 0; i < dimof(images_0); ++i)
    {
        vxReleaseImage(&images_0[i]);
    }
    for (i = 0; i < dimof(nodes_0); ++i)
    {
        vxReleaseNode(&nodes_0[i]);
    }
    vxReleaseGraph(&graph);
}

/* Test for correct sequence of execution even when copy nodes have been removed */
TEST(copySwap, testCopySequence)
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    vx_image images[] = {
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8),
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8),
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8),
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8),
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8),
        vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8)
    };
    vx_node nodes[] = {
        vxAccumulateWeightedImageNodeX(graph, images[0], 0.5f, images[1]),
        vxCopyNode(graph, (vx_reference)images[1], (vx_reference)images[3]),    /* Not removable */
        vxAccumulateWeightedImageNodeX(graph, images[0], 0.5f, images[3]),
        vxCopyNode(graph, (vx_reference)images[3], (vx_reference)images[4]),    /* removable */
        vxNotNode(graph, images[3], images[6]),
        vxAccumulateWeightedImageNodeX(graph, images[1], 0.5f, images[4]),
        vxCopyNode(graph, (vx_reference)images[4], (vx_reference)images[8]),    /* removable */
        vxMoveNode(graph, (vx_reference)images[8], (vx_reference)images[9]),    /* removable */
        vxMoveNode(graph, (vx_reference)images[9], (vx_reference)images[5]),    /* removable **/
        vxNotNode(graph, images[4], images[7]),
        vxAccumulateWeightedImageNodeX(graph, images[0], 0.5f, images[5]),
        vxCopyNode(graph, (vx_reference)images[5], (vx_reference)images[2])     /* removable */
    };
    writeImage(images[0], 96, 160);
    writeImage(images[1], 112, 176);
    writeImage(images[2], 4, 4);
    vx_uint32 old_num_nodes = 0;
    vx_uint32 new_num_nodes = 0;

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryGraph(graph, VX_GRAPH_NUMNODES, &old_num_nodes, sizeof(old_num_nodes)));
    EXPECT_NE_VX_STATUS (VX_SUCCESS, (old_num_nodes == (int)dimof(nodes)));
    /* Expected Accumulate chain graph to verify */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryGraph(graph, VX_GRAPH_NUMNODES, &new_num_nodes, sizeof(new_num_nodes)));
    /* Expected 5 copy or move nodes to be removed from the accumulate chain */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, (new_num_nodes == old_num_nodes - 5));
    EXPECT_EQ_VX_STATUS (VX_SUCCESS, vxProcessGraph(graph));
    /* Graph overall output was correct */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(images[2], 99, 163));
    /* Input value was correct */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(images[1], 104, 168));
    /* First tap output was correct */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(images[6], 255-100, 255-164));
    /* Second tap output was correct */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(images[7], 255-102, 255-166));

    int i;
    for (i = 0; i < dimof(images); ++i)
    {
        VX_CALL(vxReleaseImage(&images[i]));
    }
    for (i = 0; i < dimof(nodes); ++i)
    {
        VX_CALL(vxReleaseNode(&nodes[i]));
    }
    VX_CALL(vxReleaseGraph(&graph));
}

/* Tests for delay:
 * Create a graph with a delay and a move node that
 * pushes an image into the delay. Run manually.
 * Same but with pipelining and auto-aging
 */
TEST(copySwap, testDelays)
{
    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    vx_image image_in;
    vx_image image_out;
    vx_delay delay;
    vx_image image_0;
    vx_image image_1;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image_in = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image_out = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)image_in, 2), VX_TYPE_DELAY);
    ASSERT_VX_OBJECT(image_0 = (vx_image)vxGetReferenceFromDelay(delay, 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image_1 = (vx_image)vxGetReferenceFromDelay(delay, -1), VX_TYPE_IMAGE);

    vx_node nodes[] = {
        vxMoveNode(graph, (vx_reference)image_in, (vx_reference)image_0),
        vxAccumulateWeightedImageNodeX(graph, image_1, 0.5f, image_0),
        vxCopyNode(graph, (vx_reference)image_0, (vx_reference)image_out)
    };
    writeImage(image_in, 0x24, 0x48);
    writeImage(image_1, 0, 0);
    writeImage(image_0, 0xff, 0xff);

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    /* Simple graph with delay executed correctly (pass one) */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image_out, 0x12, 0x24));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxAgeDelay(delay));
    writeImage(image_in, 0x12, 0x44);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    /* Simple graph with delay executed correctly (pass two) */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image_out, 0x12, 0x34));

    VX_CALL(vxReleaseImage(&image_in));
    VX_CALL(vxReleaseImage(&image_out));
    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleaseGraph(&graph));

    /* Now do similar with replicated nodes */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
       vx_object_array array_in = (vx_object_array)createReference(context, VX_TYPE_OBJECT_ARRAY);
    vx_object_array array_out = (vx_object_array)createReference(context, VX_TYPE_OBJECT_ARRAY);
    image_in = (vx_image)vxGetObjectArrayItem(array_in, 0);
    image_out = (vx_image)vxGetObjectArrayItem(array_out, 0);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)array_in, 2), VX_TYPE_DELAY);
    image_0 = (vx_image)vxGetObjectArrayItem((vx_object_array)vxGetReferenceFromDelay(delay, 0), 0);
    image_1 = (vx_image)vxGetObjectArrayItem((vx_object_array)vxGetReferenceFromDelay(delay, -1), 0);
    nodes[0] = vxMoveNode(graph, (vx_reference)image_in, (vx_reference)image_0);
    nodes[1] = vxAccumulateWeightedImageNodeX(graph, image_1, 0.5f, image_0);
    nodes[2] = vxCopyNode(graph, (vx_reference)image_0, (vx_reference)image_out);
    vx_bool replicate[] = {vx_true_e, vx_true_e, vx_false_e, vx_true_e};

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReplicateNode(graph, nodes[0], replicate, 2));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReplicateNode(graph, nodes[1], replicate + 1, 3));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReplicateNode(graph, nodes[2], replicate, 2));

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues((vx_reference)array_in, VX_TYPE_OBJECT_ARRAY, 0x24, 0x48));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(vxGetReferenceFromDelay(delay, -1), VX_TYPE_OBJECT_ARRAY, 0, 0));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(vxGetReferenceFromDelay(delay, 0), VX_TYPE_OBJECT_ARRAY, 0xff, 0xff));

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    /* Replicated graph with image delay executed correctly (pass one) */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkValues((vx_reference)array_out, VX_TYPE_OBJECT_ARRAY, 0x12, 0x24));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxAgeDelay(delay));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues((vx_reference)array_in, VX_TYPE_OBJECT_ARRAY, 0x12, 0x44));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    /* Replicated graph with image delay executed correctly (pass two) */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkValues((vx_reference)array_out, VX_TYPE_OBJECT_ARRAY, 0x12, 0x34));

    VX_CALL(vxReleaseImage(&image_in));
    VX_CALL(vxReleaseImage(&image_out));
    VX_CALL(vxReleaseImage(&image_0));
    VX_CALL(vxReleaseImage(&image_1));
    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleaseObjectArray(&array_in));
    VX_CALL(vxReleaseObjectArray(&array_out));
    VX_CALL(vxReleaseGraph(&graph));

    /* Now execute in a pipeline */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    array_in = (vx_object_array)createReference(context, VX_TYPE_OBJECT_ARRAY);
    array_out = (vx_object_array)createReference(context, VX_TYPE_OBJECT_ARRAY);
    image_in = (vx_image)vxGetObjectArrayItem(array_in, 0);
    image_out = (vx_image)vxGetObjectArrayItem(array_out, 0);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)array_in, 2), VX_TYPE_DELAY);

    image_0 = (vx_image)vxGetObjectArrayItem((vx_object_array)vxGetReferenceFromDelay(delay, 0), 0);
    image_1 = (vx_image)vxGetObjectArrayItem((vx_object_array)vxGetReferenceFromDelay(delay, -1), 0);
    nodes[0] = vxMoveNode(graph, (vx_reference)image_in, (vx_reference)image_0);
    nodes[1] = vxAccumulateWeightedImageNodeX(graph, image_1, 0.5f, image_0);
    nodes[2] = vxCopyNode(graph, (vx_reference)image_0, (vx_reference)image_out);
    addParameterToGraph(graph, nodes[0], 0);
    addParameterToGraph(graph, nodes[2], 1);
    vx_graph_parameter_queue_params_t params[] = {
        {.graph_parameter_index = 0, .refs_list = (vx_reference*)&image_in, .refs_list_size = 1},
        {.graph_parameter_index = 1, .refs_list = (vx_reference*)&image_out, .refs_list_size = 1}
    };

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetGraphScheduleConfig(graph, VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO, 2, params));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReplicateNode(graph, nodes[0], replicate, 2));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReplicateNode(graph, nodes[1], replicate + 1, 3));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReplicateNode(graph, nodes[2], replicate, 2));

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues((vx_reference)array_in, VX_TYPE_OBJECT_ARRAY, 0x24, 0x48));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(vxGetReferenceFromDelay(delay, -1), VX_TYPE_OBJECT_ARRAY, 0, 0));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(vxGetReferenceFromDelay(delay, 0), VX_TYPE_OBJECT_ARRAY, 0xff, 0xff));

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    vx_uint32 num_refs;

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&image_in, 1));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&image_out, 1));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&image_in, 1, &num_refs));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&image_out, 1, &num_refs));
    /* Pipelined replicated graph with image delay executed correctly (pass one) */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkValues((vx_reference)array_out, VX_TYPE_OBJECT_ARRAY, 0x12, 0x24));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues((vx_reference)array_in, VX_TYPE_OBJECT_ARRAY, 0x12, 0x44));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&image_in, 1));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&image_out, 1));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&image_in, 1, &num_refs));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&image_out, 1, &num_refs));
    /* Pipelined replicated graph with image delay executed correctly (pass two) */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkValues((vx_reference)array_out, VX_TYPE_OBJECT_ARRAY, 0x12, 0x34));

    VX_CALL(vxReleaseImage(&image_in));
    VX_CALL(vxReleaseImage(&image_out));
    VX_CALL(vxReleaseImage(&image_0));
    VX_CALL(vxReleaseImage(&image_1));
    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleaseObjectArray(&array_in));
    VX_CALL(vxReleaseObjectArray(&array_out));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Test object arrays of pyramids */
TEST(copySwap, testContainers)
{

    vx_status status = VX_SUCCESS;
    vx_context context = context_->vx_context_;
    vx_object_array array1 = NULL;
    vx_object_array array2 = NULL;

    vx_reference ref = createReference(context, VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(array1 = vxCreateObjectArray(context, ref, 4), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(array2 = vxCreateObjectArray(context, ref, 4), VX_TYPE_OBJECT_ARRAY);

    VX_CALL(vxReleaseReference(&ref));
    vx_uint32 i;
    for (i = 0; i < 4; ++i)
    {
        ref = vxGetObjectArrayItem(array1, i);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(ref, VX_TYPE_PYRAMID, 0x10 + i, 0x20 + i));
        VX_CALL(vxReleaseReference(&ref));
        ref = vxGetObjectArrayItem(array2, i);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(ref, VX_TYPE_PYRAMID, 0x30 + i, 0x40 + i));
        VX_CALL(vxReleaseReference(&ref));
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuSwap(context, (vx_reference)array1, (vx_reference)array2));
    for (i = 0; i < 4; ++i)
    {
        ref = vxGetObjectArrayItem(array2, i);
        vx_status status = checkValues(ref, VX_TYPE_PYRAMID, 0x10 + i, 0x20 + i);
        VX_CALL(vxReleaseReference(&ref));
        ref = vxGetObjectArrayItem(array1, i);
        status |= checkValues(ref, VX_TYPE_PYRAMID, 0x30 + i, 0x40 + i);
        VX_CALL(vxReleaseReference(&ref));
    }
    /* Swap array of pyramids successful */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuMove(context, (vx_reference)array1, (vx_reference)array2));
    for (i = 0; i < 4; ++i)
    {
        ref = vxGetObjectArrayItem(array2, i);
        status = checkValues(ref, VX_TYPE_PYRAMID, 0x30 + i, 0x40 + i);
        VX_CALL(vxReleaseReference(&ref));
    }
    /* "Move array of pyramids successful */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS,status);
    for (i = 0; i < 4; ++i)
    {
        ref = vxGetObjectArrayItem(array1, i);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, writeValues(ref, VX_TYPE_PYRAMID, i, i));
        VX_CALL(vxReleaseReference(&ref));
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuCopy(context, (vx_reference)array2, (vx_reference)array1));
    for (i = 0; i < 4; ++i)
    {
        ref = vxGetObjectArrayItem(array1, i);
        status = checkValues(ref, VX_TYPE_PYRAMID, 0x30 + i, 0x40 + i);
        VX_CALL(vxReleaseReference(&ref));
    }
    /* Copy array of pyramids successful */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    VX_CALL(vxReleaseObjectArray(&array1));
    VX_CALL(vxReleaseObjectArray(&array2));
}

TEST(copySwap, testImportFromHandle)
{
    /*import images from handle and copy or swap it
      the imported image should have the same size and format 
      but the size should be different
    */
    vx_image image;
    vx_imagepatch_addressing_t addr;
    vx_df_image format = VX_DF_IMAGE_U8;
    vx_context context = context_->vx_context_;
    void* ptrs;

    int channel[4] = { 0, 0, 0, 0 };
    channel[0] = VX_CHANNEL_0;
    size_t plane_size = 0;
    addr.dim_x    = 24;
    addr.dim_y    = 24;
    addr.stride_x = 1;
    addr.stride_y = 24;
    addr.scale_x  = VX_SCALE_UNITY;
    addr.scale_y  = VX_SCALE_UNITY;
    addr.step_x = 1;
    addr.step_y = 1;

    plane_size = 24 * 24;
    ptrs = ct_alloc_mem(plane_size);
    /* init memory */
    ct_memset(ptrs, 0xff, plane_size);    

    image = vxCreateImageFromHandle(context, format, &addr, &ptrs, (vx_enum)VX_MEMORY_TYPE_HOST);
    ASSERT_VX_OBJECT(image, VX_TYPE_IMAGE);
    vx_rectangle_t rect = {.start_x = 0, .start_y = 0, .end_x = 24, .end_y = 24};
    /* create a vx_image with the same format and dimensions */
    vx_image image2 = vxCreateImage(context, 24, 24, format);
    ASSERT_VX_OBJECT(image2, VX_TYPE_IMAGE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxuCopy(context, (vx_reference)image, (vx_reference)image2));
    rect.end_x = 12;
    rect.end_y = 12;
    vx_image roi = vxCreateImageFromROI((vx_image)image, &rect);
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxuSwap(context, (vx_reference)image, (vx_reference)image2));

    vxReleaseImage(&roi);
    vxReleaseImage(&image);
    vxReleaseImage(&image2);
}

TESTCASE_TESTS(copySwap,
               testCopy,
               testSwap,
               testMove,
               testVxu,
               testVxuCopy,
               testCopyRemoval,
               testNoCopyRemovalSubObjects,
               testNoCopyRemovalContainersDiffer,
               testNoCopyRemovalPyramidLevel,
               testNoCopyRemovalArrayItem,
               testNoCopyRemovalBidirectional,
               testMaxNodes,
               testMoveRemoval,
               testMoveRemovalLimits,
               testCopySequence,
               testSubObjectsOfImages,
               testSubObjectsOfTensors,
               testSubObjectsMaxOfSubImages,
               testDelays,
               testContainers,
               testImportFromHandle)
