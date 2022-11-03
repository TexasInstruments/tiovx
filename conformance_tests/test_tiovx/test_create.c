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

#define MAX_POINTS 100

TESTCASE(tivxCreate, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* src1_fileName;
    const char* src2_fileName;
    const char* points_fileName;
    vx_size winSize;
    int useReferencePyramid;
} Arg;


#define PARAMETERS \
    ARG("case1/Create", NULL, "optflow_00.bmp", "optflow_01.bmp", "optflow_pyrlk_5x5.txt", 5, 1)

TEST_WITH_ARG(tivxCreate, testCreateMatrix, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_matrix matrix_uint8 = 0, matrix_int8 = 0, matrix_uint16 = 0, matrix_int16 = 0;
    vx_matrix matrix_uint32 = 0, matrix_int32 = 0, matrix_uint64 = 0, matrix_int64 = 0, matrix_float32 = 0, matrix_float64 = 0;

    ASSERT_VX_OBJECT(matrix_uint8 = vxCreateMatrix(context, VX_TYPE_UINT8, 3, 3), VX_TYPE_MATRIX);

    ASSERT_VX_OBJECT(matrix_int8 = vxCreateMatrix(context, VX_TYPE_INT8, 3, 3), VX_TYPE_MATRIX);

    ASSERT_VX_OBJECT(matrix_uint16 = vxCreateMatrix(context, VX_TYPE_UINT16, 3, 3), VX_TYPE_MATRIX);

    ASSERT_VX_OBJECT(matrix_int16 = vxCreateMatrix(context, VX_TYPE_INT16, 3, 3), VX_TYPE_MATRIX);

    ASSERT_VX_OBJECT(matrix_uint32 = vxCreateMatrix(context, VX_TYPE_UINT32, 3, 3), VX_TYPE_MATRIX);

    ASSERT_VX_OBJECT(matrix_int32 = vxCreateMatrix(context, VX_TYPE_INT32, 3, 3), VX_TYPE_MATRIX);

    ASSERT_VX_OBJECT(matrix_uint64 = vxCreateMatrix(context, VX_TYPE_UINT64, 3, 3), VX_TYPE_MATRIX);

    ASSERT_VX_OBJECT(matrix_int64 = vxCreateMatrix(context, VX_TYPE_INT64, 3, 3), VX_TYPE_MATRIX);

    ASSERT_VX_OBJECT(matrix_float32 = vxCreateMatrix(context, VX_TYPE_FLOAT32, 3, 3), VX_TYPE_MATRIX);

    ASSERT_VX_OBJECT(matrix_float64 = vxCreateMatrix(context, VX_TYPE_FLOAT64, 3, 3), VX_TYPE_MATRIX);

    VX_CALL(vxReleaseMatrix(&matrix_float64));
    VX_CALL(vxReleaseMatrix(&matrix_float32));
    VX_CALL(vxReleaseMatrix(&matrix_int64));
    VX_CALL(vxReleaseMatrix(&matrix_uint64));
    VX_CALL(vxReleaseMatrix(&matrix_int32));
    VX_CALL(vxReleaseMatrix(&matrix_uint32));
    VX_CALL(vxReleaseMatrix(&matrix_int16));
    VX_CALL(vxReleaseMatrix(&matrix_uint16));
    VX_CALL(vxReleaseMatrix(&matrix_uint8));
    VX_CALL(vxReleaseMatrix(&matrix_int8));
}

TEST_WITH_ARG(tivxCreate, testCreateLUT, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_object_array src_object_array;
    vx_lut lut_char = 0, lut_uint8 = 0, lut_int8 = 0, lut_uint16 = 0, lut_int16 = 0;
    vx_lut lut_uint32 = 0, lut_int32 = 0, lut_uint64 = 0, lut_int64 = 0, lut_float32 = 0, lut_float64 = 0;

    ASSERT_VX_OBJECT(lut_char = vxCreateLUT(context, VX_TYPE_CHAR, 256), VX_TYPE_LUT);

    ASSERT_VX_OBJECT(lut_int8 = vxCreateLUT(context, VX_TYPE_INT8, 256), VX_TYPE_LUT);

    ASSERT_VX_OBJECT(lut_uint8 = vxCreateLUT(context, VX_TYPE_UINT8, 256), VX_TYPE_LUT);

    ASSERT_VX_OBJECT(lut_uint16 = vxCreateLUT(context, VX_TYPE_UINT16, 256), VX_TYPE_LUT);

    ASSERT_VX_OBJECT(lut_int16 = vxCreateLUT(context, VX_TYPE_INT16, 256), VX_TYPE_LUT);

    ASSERT_VX_OBJECT(lut_uint32 = vxCreateLUT(context, VX_TYPE_UINT32, 256), VX_TYPE_LUT);

    ASSERT_VX_OBJECT(lut_int32 = vxCreateLUT(context, VX_TYPE_INT32, 256), VX_TYPE_LUT);

    ASSERT_VX_OBJECT(lut_uint64 = vxCreateLUT(context, VX_TYPE_UINT64, 256), VX_TYPE_LUT);

    ASSERT_VX_OBJECT(lut_int64 = vxCreateLUT(context, VX_TYPE_INT64, 256), VX_TYPE_LUT);

    ASSERT_VX_OBJECT(lut_float32 = vxCreateLUT(context, VX_TYPE_FLOAT32, 256), VX_TYPE_LUT);

    ASSERT_VX_OBJECT(lut_float64 = vxCreateLUT(context, VX_TYPE_FLOAT64, 256), VX_TYPE_LUT);

    VX_CALL(vxReleaseLUT(&lut_float64));
    VX_CALL(vxReleaseLUT(&lut_float32));
    VX_CALL(vxReleaseLUT(&lut_int64));
    VX_CALL(vxReleaseLUT(&lut_uint64));
    VX_CALL(vxReleaseLUT(&lut_int32));
    VX_CALL(vxReleaseLUT(&lut_uint32));
    VX_CALL(vxReleaseLUT(&lut_int16));
    VX_CALL(vxReleaseLUT(&lut_uint16));
    VX_CALL(vxReleaseLUT(&lut_uint8));
    VX_CALL(vxReleaseLUT(&lut_int8));
    VX_CALL(vxReleaseLUT(&lut_char));
}

TEST_WITH_ARG(tivxCreate, testCreateNodeByEnum, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_node node = NULL;
    vx_graph graph;
    vx_image image_0, image_1, image_2;

    ASSERT_VX_OBJECT(image_0   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image_1   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image_2   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2
    };
    {
        ASSERT_VX_OBJECT(node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_ABSDIFF, params, 3), VX_TYPE_NODE);
    }

    VX_CALL(vxReleaseImage(&image_2));
    VX_CALL(vxReleaseImage(&image_1));
    VX_CALL(vxReleaseImage(&image_0));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST_WITH_ARG(tivxCreate, testCreateNodeByRef, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_node node = NULL;
    vx_graph graph;
    vx_image image_0, image_1, image_2;

    ASSERT_VX_OBJECT(image_0   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image_1   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image_2   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2
    };
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "org.khronos.openvx.absdiff");

        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            ASSERT_VX_OBJECT(node = tivxCreateNodeByKernelRef(graph, kernel, params, 3), VX_TYPE_NODE);
        }

        VX_CALL(vxReleaseKernel(&kernel));
    }

    VX_CALL(vxReleaseImage(&image_2));
    VX_CALL(vxReleaseImage(&image_1));
    VX_CALL(vxReleaseImage(&image_0));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST_WITH_ARG(tivxCreate, testCreateNodeByName, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_node node = NULL;
    vx_graph graph;
    vx_image image_0, image_1, image_2;

    ASSERT_VX_OBJECT(image_0   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image_1   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image_2   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2
    };
    {
        ASSERT_VX_OBJECT(node = tivxCreateNodeByKernelName(graph, "org.khronos.openvx.absdiff", params, 3), VX_TYPE_NODE);
    }

    VX_CALL(vxReleaseImage(&image_2));
    VX_CALL(vxReleaseImage(&image_1));
    VX_CALL(vxReleaseImage(&image_0));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}


TESTCASE_TESTS(tivxCreate,
        testCreateMatrix,
        testCreateLUT,
        testCreateNodeByEnum,
        testCreateNodeByRef,
        testCreateNodeByName
        )

