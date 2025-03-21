/* 

 * Copyright (c) 2024 The Khronos Group Inc.
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

#include "test_engine/test.h"

TESTCASE(tivxSafeCasts, CT_VXContext, ct_setup_vx_context, 0)

#include "TI/tivx.h"
#include "VX/vx_khr_user_data_object.h"
#include "VX/vx_khr_safe_casts.h"
#include <stdio.h>
#include <stdlib.h>

static const vx_enum type_ids[] = 
{
    VX_TYPE_ARRAY,
    VX_TYPE_CONTEXT,
    VX_TYPE_CONVOLUTION,
    VX_TYPE_DELAY,
    VX_TYPE_DISTRIBUTION,
    VX_TYPE_GRAPH,
    VX_TYPE_IMAGE,
    VX_TYPE_KERNEL,
    VX_TYPE_LUT,
    VX_TYPE_MATRIX,
    VX_TYPE_META_FORMAT,
    VX_TYPE_NODE,
    VX_TYPE_OBJECT_ARRAY,
    VX_TYPE_PARAMETER,
    VX_TYPE_PYRAMID,
    VX_TYPE_REMAP,
    VX_TYPE_SCALAR,
    VX_TYPE_TENSOR,
    VX_TYPE_THRESHOLD,
    VX_TYPE_USER_DATA_OBJECT
};

static const vx_enum upcast_type_ids[] = 
{
    VX_TYPE_ARRAY,
    VX_TYPE_CONVOLUTION,
    VX_TYPE_DELAY,
    VX_TYPE_DISTRIBUTION,
    VX_TYPE_GRAPH,
    VX_TYPE_IMAGE,
    VX_TYPE_KERNEL,
    VX_TYPE_LUT,
    VX_TYPE_MATRIX,
    VX_TYPE_NODE,
    VX_TYPE_OBJECT_ARRAY,
    VX_TYPE_PARAMETER,
    VX_TYPE_PYRAMID,
    VX_TYPE_REMAP,
    VX_TYPE_SCALAR,
    VX_TYPE_TENSOR,
    VX_TYPE_THRESHOLD,
    VX_TYPE_USER_DATA_OBJECT
};

#define NUM_TYPES (sizeof(type_ids) / sizeof(vx_enum))
#define UPCAST_NUM_TYPES (sizeof(upcast_type_ids) / sizeof(vx_enum))

enum failtype { SUCCESS, NULLREF, TYPEERROR };

typedef struct _user_data
{
    vx_uint32 numbers[4];
} user_data_t;

/* Create a non-virtual example object of the given type and cast as a reference  */
vx_reference castAsReference(vx_graph graph, vx_enum type)
{
    vx_context context = vxGetContext(vxCastRefFromGraph(graph));
    vx_reference ref = NULL;
    switch (type)
    {
        case VX_TYPE_REFERENCE:
            /* Used for creating the wrong type, just fall through to array */
        case VX_TYPE_ARRAY:
            ref = vxCastRefFromArray(vxCreateArray(context, VX_TYPE_UINT8, 4));
            break;
        case VX_TYPE_CONTEXT:
            ref = vxCastRefFromContext(context);
            vxRetainReference(ref); /* we don't want to lose the context! */
            break;
        case VX_TYPE_CONVOLUTION:
            ref = vxCastRefFromConvolution(vxCreateConvolution(context, 3, 3));
            break;
        case VX_TYPE_DELAY:
        {
            vx_image image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
            ref = vxCastRefFromDelay(vxCreateDelay(context, vxCastRefFromImage(image), 3));
            vxReleaseImage(&image);
            break;
        }
        case VX_TYPE_DISTRIBUTION:
            ref = vxCastRefFromDistribution(vxCreateDistribution(context, 4, 0, 256));
            break;
        case VX_TYPE_GRAPH:
            ref = vxCastRefFromGraph(vxCreateGraph(context));
            break;
        case VX_TYPE_IMAGE:
            ref = vxCastRefFromImage(vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8));
            break;
        case VX_TYPE_KERNEL:
            ref = vxCastRefFromKernel(vxGetKernelByEnum(context, VX_KERNEL_ABSDIFF));
            break;
        case VX_TYPE_LUT:
            ref = vxCastRefFromLUT(vxCreateLUT(context, VX_TYPE_UINT8, 4));
            break;
        case VX_TYPE_MATRIX:
            ref = vxCastRefFromMatrix(vxCreateMatrix(context, VX_TYPE_INT32, 3, 3));
            break;
        case VX_TYPE_NODE:
        {
            vx_kernel kernel = vxGetKernelByEnum(context, VX_KERNEL_ABSDIFF);
            ref = vxCastRefFromNode(vxCreateGenericNode(graph, kernel));
            vxReleaseKernel(&kernel);
            break;
        }
        case VX_TYPE_OBJECT_ARRAY:
        {
            vx_image image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
            ref = vxCastRefFromObjectArray(vxCreateObjectArray(context, vxCastRefFromImage(image), 4));
            vxReleaseImage(&image);
            break;
        }
        case VX_TYPE_PARAMETER:
        {
            vx_kernel kernel = vxGetKernelByEnum(context, VX_KERNEL_ABSDIFF);
            ref = vxCastRefFromParameter(vxGetKernelParameterByIndex(kernel, 0));
            vxReleaseKernel(&kernel);
            break;
        }
        case VX_TYPE_PYRAMID:
            ref = vxCastRefFromPyramid(vxCreatePyramid(context, 2, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8));
            break;
        case VX_TYPE_REMAP:
            ref = vxCastRefFromRemap(vxCreateRemap(context, 16, 16, 16, 16));
            break;
        case VX_TYPE_SCALAR:
        {
            vx_uint32 value = 0;
            ref = vxCastRefFromScalar(vxCreateScalar(context, VX_TYPE_UINT32, &value));
            break;
        }
        case VX_TYPE_TENSOR:
        {
            vx_size dims[3] = {4, 4, 4};
            ref = vxCastRefFromTensor(vxCreateTensor(context, 3, dims, VX_TYPE_INT32, 0));
            break;
        }
        case VX_TYPE_THRESHOLD:
            ref = vxCastRefFromThreshold(vxCreateThreshold(context, VX_THRESHOLD_TYPE_BINARY, VX_TYPE_UINT8));
            break;
        case VX_TYPE_USER_DATA_OBJECT:
        default:    /* All other codes treated here, used for making the wrong type */
        {
            user_data_t user_data = {0};
            ref = vxCastRefFromUserDataObject(vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data));
            break;
        }
    }
    return ref;
}

/* Create a non-virtual example object of the given type and get as a reference  */
vx_reference getAsReference(vx_graph graph, vx_enum type)
{
    vx_context context = vxGetContext(vxCastRefFromGraph(graph));
    vx_reference ref = NULL;
    switch (type)
    {
        case VX_TYPE_REFERENCE:
            /* Used for creating the wrong type, just fall through to array */
        case VX_TYPE_ARRAY:
        {
            vx_array array = vxCreateArray(context, VX_TYPE_UINT8, 4);
            ref = vxGetRefFromArray(&array);
            vxReleaseArray(&array);
            break;
        }
        case VX_TYPE_CONTEXT:
            ref = vxGetRefFromContext(&context);
            break;
        case VX_TYPE_CONVOLUTION:
        {
            vx_convolution convolution = vxCreateConvolution(context, 3, 3);
            ref = vxGetRefFromConvolution(&convolution);
            vxReleaseConvolution(&convolution);
            break;
        }
        case VX_TYPE_DELAY:
        {
            vx_image image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
            vx_delay delay = vxCreateDelay(context, vxCastRefFromImage(image), 3);
            ref = vxGetRefFromDelay(&delay);
            vxReleaseDelay(&delay);
            vxReleaseImage(&image);
            break;
        }
        case VX_TYPE_DISTRIBUTION:
        {
            vx_distribution distribution = vxCreateDistribution(context, 4, 0, 256); 
            ref = vxGetRefFromDistribution(&distribution);
            vxReleaseDistribution(&distribution);
            break;
        }
        case VX_TYPE_GRAPH:
        {
            vx_graph graph = vxCreateGraph(context);
            ref = vxGetRefFromGraph(&graph);
            vxReleaseGraph(&graph);
            break;
        }
        case VX_TYPE_IMAGE:
        {
            vx_image image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
            ref = vxGetRefFromImage(&image);
            vxReleaseImage(&image);
            break;
        }
        case VX_TYPE_KERNEL:
        {
            vx_kernel kernel = vxGetKernelByEnum(context, VX_KERNEL_ABSDIFF);
            ref = vxGetRefFromKernel(&kernel);
            vxReleaseKernel(&kernel);
            break;
        }
        case VX_TYPE_LUT:
        {
            vx_lut lut = vxCreateLUT(context, VX_TYPE_UINT8, 4);
            ref = vxGetRefFromLUT(&lut);
            vxReleaseLUT(&lut);
            break;
        }
        case VX_TYPE_MATRIX:
        {
            vx_matrix matrix = vxCreateMatrix(context, VX_TYPE_INT32, 3, 3);
            ref = vxGetRefFromMatrix(&matrix);
            vxReleaseMatrix(&matrix);
            break;
        }
        case VX_TYPE_NODE:
        {
            vx_kernel kernel = vxGetKernelByEnum(context, VX_KERNEL_ABSDIFF);
            vx_node node = vxCreateGenericNode(graph, kernel);
            ref = vxGetRefFromNode(&node);
            vxReleaseNode(&node);
            vxReleaseKernel(&kernel);
            break;
        }
        case VX_TYPE_OBJECT_ARRAY:
        {
            vx_image image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
            vx_object_array object_array = vxCreateObjectArray(context, vxCastRefFromImage(image), 4);
            ref = vxGetRefFromObjectArray(&object_array);
            vxReleaseImage(&image);
            vxReleaseObjectArray(&object_array);
            break;
        }
        case VX_TYPE_PARAMETER:
        {
            vx_kernel kernel = vxGetKernelByEnum(context, VX_KERNEL_ABSDIFF);
            vx_parameter parameter = vxGetKernelParameterByIndex(kernel, 0);
            ref = vxGetRefFromParameter(&parameter);
            vxReleaseParameter(&parameter);
            vxReleaseKernel(&kernel);
            break;
        }
        case VX_TYPE_PYRAMID:
        {
            vx_pyramid pyramid = vxCreatePyramid(context, 2, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8);
            ref = vxGetRefFromPyramid(&pyramid);
            vxReleasePyramid(&pyramid);
            break;
        }
        case VX_TYPE_REMAP:
        {
            vx_remap remap = vxCreateRemap(context, 16, 16, 16, 16);
            ref = vxGetRefFromRemap(&remap);
            vxReleaseRemap(&remap);
            break;
        }
        case VX_TYPE_SCALAR:
        {
            vx_uint32 value = 0;
            vx_scalar scalar = vxCreateScalar(context, VX_TYPE_UINT32, &value);
            ref = vxGetRefFromScalar(&scalar);
            vxReleaseScalar(&scalar);
            break;
        }
        case VX_TYPE_TENSOR:
        {
            vx_size dims[3] = {4, 4, 4};
            vx_tensor tensor = vxCreateTensor(context, 3, dims, VX_TYPE_INT32, 0);
            ref = vxGetRefFromTensor(&tensor);
            vxReleaseTensor(&tensor);
            break;
        }
        case VX_TYPE_THRESHOLD:
        {
            vx_threshold threshold = vxCreateThreshold(context, VX_THRESHOLD_TYPE_BINARY, VX_TYPE_UINT8);  /* This will need updating!!)*/
            ref = vxGetRefFromThreshold(&threshold);
            vxReleaseThreshold(&threshold);
            break;
        }
        default:    /* All other codes treated here, used for making the wrong type */
        case VX_TYPE_USER_DATA_OBJECT:
        {
            user_data_t user_data = {0};
            vx_user_data_object user_data_object = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data);
            ref = vxGetRefFromUserDataObject(&user_data_object);
            vxReleaseUserDataObject(&user_data_object);
            break;
        }
    }
    return ref;
}

/* We are just getting coverage of functions here...*/
vx_enum retTypeOfConstP(const vx_reference *refp)
{
    vx_enum ret;
    vxQueryReference(*refp, VX_REFERENCE_TYPE, &ret, sizeof(ret));
    return ret;
}

#define typecase(upper_type, lower_type, case_type) case VX_TYPE_##upper_type: \
        {\
            vx_##lower_type lower_type = vxGetRefAs##case_type(&ref, &status);\
            if (!fail)\
            {\
                EXPECT_EQ_INT(retTypeOfConstP(vxCastRefFrom##case_type##ConstP(&lower_type)), type); \
                EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseReference(vxCastRefFrom##case_type##P(& lower_type)));\
            }\
            else\
            {\
                EXPECT_NE_VX_STATUS(VX_SUCCESS, vxGetStatus(vxCastRefFrom##case_type(lower_type)));\
            }\
            break;\
        }

vx_status getAsSomeType(vx_graph graph, vx_enum type, enum failtype fail)
{
    vx_status status = VX_SUCCESS;
    vx_reference ref = NULL;
    if (NULLREF != fail )
    {
        if (TYPEERROR == fail)
        {
            if (VX_TYPE_TENSOR == type)
                ref = castAsReference(graph, VX_TYPE_ARRAY);
            else
                ref = castAsReference(graph, VX_TYPE_TENSOR);
        }
        else
        {
            ref = castAsReference(graph, type);
        }
    }
    switch (type)
    {
        typecase(ARRAY, array, Array)
        typecase(CONVOLUTION, convolution, Convolution)
        typecase(DELAY, delay, Delay)
        typecase(DISTRIBUTION, distribution, Distribution)
        typecase(GRAPH, graph, Graph)
        typecase(IMAGE, image, Image)
        typecase(KERNEL, kernel, Kernel)
        typecase(LUT, lut, LUT)
        typecase(MATRIX, matrix, Matrix)
        typecase (NODE, node, Node)
        typecase(OBJECT_ARRAY, object_array, ObjectArray)
        typecase(PARAMETER, parameter, Parameter)
        typecase(PYRAMID, pyramid, Pyramid)
        typecase(REMAP, remap, Remap)
        typecase(SCALAR, scalar, Scalar)
        typecase(TENSOR, tensor, Tensor)
        typecase(THRESHOLD, threshold, Threshold)
        typecase(USER_DATA_OBJECT, user_data_object, UserDataObject)
        default:
            status = VX_ERROR_INVALID_TYPE;
    }

    if (ref) {EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseReference(&ref));}
    return status;
}

vx_status castAsSomeType(vx_graph graph, vx_enum type, enum failtype fail)
{
    vx_status status = VX_SUCCESS;
    vx_reference ref = NULL;
    if (NULLREF != fail )
    {
        if (TYPEERROR == fail)
        {
            if (VX_TYPE_TENSOR == type)
                ref = castAsReference(graph, VX_TYPE_ARRAY);
            else
                ref = castAsReference(graph, VX_TYPE_TENSOR);
        }
        else
        {
            ref = castAsReference(graph, type);
        }
    }
    switch (type)
    {
        case VX_TYPE_ARRAY:
        {
            vx_array array = vxCastRefAsArray(ref, &status);
            break;
        }
        case VX_TYPE_CONVOLUTION:
        {
            vx_convolution convolution = vxCastRefAsConvolution(ref, &status);
            break;
        }
        case VX_TYPE_DELAY:
        {
            vx_delay delay = vxCastRefAsDelay(ref, &status);
            break;
        }
        case VX_TYPE_DISTRIBUTION:
        {
            vx_distribution distribution = vxCastRefAsDistribution(ref, &status); 
            break;
        }
        case VX_TYPE_GRAPH:
        {
            vx_graph graph = vxCastRefAsGraph(ref, &status);
            break;
        }
        case VX_TYPE_IMAGE:
        {
            vx_image image = vxCastRefAsImage(ref, &status);
            break;
        }
        case VX_TYPE_KERNEL:
        {
            vx_kernel kernel = vxCastRefAsKernel(ref, &status);
            break;
        }
        case VX_TYPE_LUT:
        {
            vx_lut lut = vxCastRefAsLUT(ref, &status);
            break;
        }
        case VX_TYPE_MATRIX:
        {
            vx_matrix matrix = vxCastRefAsMatrix(ref, &status);
            break;
        }
        case VX_TYPE_NODE:
        {
            vx_node node = vxCastRefAsNode(ref, &status);
            break;
        }
        case VX_TYPE_OBJECT_ARRAY:
        {
            vx_object_array object_array = vxCastRefAsObjectArray(ref, &status);
            break;
        }
        case VX_TYPE_PARAMETER:
        {
            vx_parameter parameter = vxCastRefAsParameter(ref, &status);
            break;
        }
        case VX_TYPE_PYRAMID:
        {
            vx_pyramid pyramid = vxCastRefAsPyramid(ref, &status);
            break;
        }
        case VX_TYPE_REMAP:
        {
            vx_remap remap = vxCastRefAsRemap(ref, &status);
            break;
        }
        case VX_TYPE_SCALAR:
        {
            vx_scalar scalar = vxCastRefAsScalar(ref, &status);
            break;
        }
        case VX_TYPE_TENSOR:
        {
            vx_tensor tensor = vxCastRefAsTensor(ref, &status);
            break;
        }
        case VX_TYPE_THRESHOLD:
        {
            vx_threshold threshold = vxCastRefAsThreshold(ref, &status);
            break;
        }
        case VX_TYPE_USER_DATA_OBJECT:
        {
            vx_user_data_object user_data_object = vxCastRefAsUserDataObject(ref, &status);
            break;
        }
        default:
            status = VX_ERROR_INVALID_TYPE;
    }
    if (ref) {EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseReference(&ref));}
    return status;
}

/* Check to see if the type is currently supported */
vx_bool is_supported(vx_enum type)
{
    switch (type)
    {
        case VX_TYPE_ARRAY:
        case VX_TYPE_CONTEXT:
        case VX_TYPE_CONVOLUTION:
        case VX_TYPE_DELAY:
        case VX_TYPE_DISTRIBUTION:
        case VX_TYPE_GRAPH:
        case VX_TYPE_IMAGE:
        case VX_TYPE_KERNEL:
        case VX_TYPE_LUT:
        case VX_TYPE_MATRIX:
        case VX_TYPE_NODE:
        case VX_TYPE_OBJECT_ARRAY:
        case VX_TYPE_PARAMETER:
        case VX_TYPE_PYRAMID:
        case VX_TYPE_REMAP:
        case VX_TYPE_SCALAR:
        case VX_TYPE_TENSOR:
        case VX_TYPE_THRESHOLD:
        case VX_TYPE_USER_DATA_OBJECT:
            return vx_true_e;
        default:
            return vx_false_e;
   }
}

TEST(tivxSafeCasts, testCastGetFromSomeType)
{
    int i;
    vx_graph graph = vxCreateGraph(context_->vx_context_);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    for (i = 0; i < UPCAST_NUM_TYPES; ++i)
    {
        vx_enum type = upcast_type_ids[i];
        if (is_supported(type))
        {
            /* For both operations we are looking for the correct type and a reference count of 1, except for context, where ref count should be 2 */
            vx_enum check_type = VX_ERROR_INVALID_TYPE;
            vx_uint32 ref_count = 0;
            vx_reference ref = castAsReference(graph, type);
            EXPECT_VX_OBJECT(ref, type);
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryReference(ref, VX_REFERENCE_COUNT, &ref_count, sizeof(ref_count)));
            if (VX_TYPE_CONTEXT == type || VX_TYPE_KERNEL == type)
            {
                /* we expect both these to have an extra ref. count */
                --ref_count;
            }
            EXPECT_EQ_INT(ref_count, 1U);
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseReference(&ref));
            ref = getAsReference(graph, type);
            EXPECT_VX_OBJECT(ref, type);
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryReference(ref, VX_REFERENCE_COUNT, &ref_count, sizeof(ref_count)));
            if (VX_TYPE_CONTEXT == type || VX_TYPE_KERNEL == type)
            {
                --ref_count;
            }
            EXPECT_EQ_INT(ref_count, 1U);
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseReference(&ref));
        }
    }
    VX_CALL(vxReleaseGraph(&graph));
}

/* Check basic operation of castAs and GetAs */
TEST(tivxSafeCasts, testCastGetAsSomeType)
{
    int i;
    vx_graph graph = vxCreateGraph(context_->vx_context_);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
    for (i = 0; i < UPCAST_NUM_TYPES; ++i)
    {
        vx_enum type = upcast_type_ids[i];
        if (is_supported(type))
        {
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, getAsSomeType(graph, type, SUCCESS));
            EXPECT_NE_VX_STATUS(VX_SUCCESS, getAsSomeType(graph, type, TYPEERROR));
            EXPECT_NE_VX_STATUS(VX_SUCCESS, getAsSomeType(graph, type, NULLREF));
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, castAsSomeType(graph, type, SUCCESS));
            EXPECT_NE_VX_STATUS(VX_SUCCESS, castAsSomeType(graph, type, TYPEERROR));
            EXPECT_NE_VX_STATUS(VX_SUCCESS, castAsSomeType(graph, type, NULLREF));
        }
    }
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxSafeCasts, testSpecificCast)
{
  tivx_raw_image raw_image = NULL;
  vx_reference ref = NULL;
  vx_context context = context_->vx_context_;
  tivx_raw_image_create_params_t params;
  vx_status status = VX_SUCCESS;
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
  ref = tivxCastRefFromRawImage(raw_image);
  EXPECT_VX_OBJECT(ref, VX_TYPE_REFERENCE);
  /* downcast now the reference */
  tivx_raw_image raw_image_downcast = tivxGetRefAsRawImage(&ref, &status);
  EXPECT_VX_OBJECT(raw_image, (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
  tivxReleaseRawImage(&raw_image);
}

TESTCASE_TESTS(
    tivxSafeCasts,
    testCastGetFromSomeType,
    testCastGetAsSomeType,
    testSpecificCast
)

