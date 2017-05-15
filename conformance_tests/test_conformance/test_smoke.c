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
#include <string.h>

TESTCASE(SmokeTest, CT_VXContext, ct_setup_vx_context, 0)

typedef struct _mystruct {
    vx_uint32 some_uint;
    vx_float64 some_double;
} mystruct;

TEST(SmokeTest, test_vxRegisterUserStruct)
{
    vx_context context = context_->vx_context_;
    vx_enum mytype = 0;
    vx_array array = 0;
    vx_enum type = 0;
    vx_size sz = 0;

    mytype = vxRegisterUserStruct(context, sizeof(mystruct));
    ASSERT(mytype >= VX_TYPE_USER_STRUCT_START);

    ASSERT_VX_OBJECT(array = vxCreateArray(context, mytype, 10), VX_TYPE_ARRAY);

    VX_CALL(vxQueryArray(array, VX_ARRAY_ITEMTYPE, &type, sizeof(type)));
    ASSERT_EQ_INT(mytype, type);

    VX_CALL(vxQueryArray(array, VX_ARRAY_ITEMSIZE, &sz, sizeof(sz)));
    ASSERT_EQ_INT(sizeof(mystruct), sz);

    VX_CALL(vxReleaseArray(&array));
    ASSERT(array == 0);
}


TEST(SmokeTest, test_vxHint)
{
    vx_image image = 0;
    vx_graph graph = 0;
    vx_context context = context_->vx_context_;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

//    VX_CALL(vxHint((vx_reference)image, VX_HINT_SERIALIZE, 0, 0));
//    VX_CALL(vxHint((vx_reference)graph, VX_HINT_SERIALIZE, 0, 0));
//    VX_CALL(vxHint((vx_reference)context, VX_HINT_SERIALIZE, 0, 0));

    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT_EQ_PTR(0, image);
    ASSERT_EQ_PTR(0, graph);
}


TEST(SmokeTest, test_vxReleaseReference)
{
    vx_context context = context_->vx_context_;
    vx_uint32 ref_count0 = 0;
    vx_uint32 ref_count1 = 0;
    vx_reference ref = 0;

    {
        /* test context reference */
        ref = (vx_reference)context;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)context;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);
    }

    {
        /* test graph reference */
        vx_graph graph = 0;
        EXPECT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ref = (vx_reference)graph;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)graph;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseGraph(&graph));
    }

    {
        /* test node reference */
        vx_graph graph = 0;
        vx_node node = 0;
        vx_image src1 = 0;
        vx_image src2 = 0;
        vx_image dst = 0;
        EXPECT_VX_OBJECT(src1 = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        EXPECT_VX_OBJECT(src2 = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        EXPECT_VX_OBJECT(dst  = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        EXPECT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        EXPECT_VX_OBJECT(node = vxAddNode(graph, src1, src2, VX_CONVERT_POLICY_WRAP, dst), VX_TYPE_NODE);
        ref = (vx_reference)node;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)node;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseImage(&src1));
        VX_CALL(vxReleaseImage(&src2));
        VX_CALL(vxReleaseImage(&dst));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
    }

    {
        /* test array reference */
        vx_array array = 0;
        EXPECT_VX_OBJECT(array = vxCreateArray(context, VX_TYPE_KEYPOINT, 32), VX_TYPE_ARRAY);
        ref = (vx_reference)array;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)array;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseArray(&array));
    }

    {
        /* test convolution reference */
        vx_convolution convolution = 0;
        EXPECT_VX_OBJECT(convolution = vxCreateConvolution(context, 5, 5), VX_TYPE_CONVOLUTION);
        ref = (vx_reference)convolution;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)convolution;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseConvolution(&convolution));
    }

    {
        /* test distribution reference */
        vx_distribution distribution = 0;
        EXPECT_VX_OBJECT(distribution = vxCreateDistribution(context, 32, 0, 255), VX_TYPE_DISTRIBUTION);
        ref = (vx_reference)distribution;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)distribution;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseDistribution(&distribution));
    }

    {
        /* test image reference */
        vx_image image = 0;
        EXPECT_VX_OBJECT(image = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ref = (vx_reference)image;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)image;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseImage(&image));
    }

    {
        /* test LUT reference */
        vx_lut lut = 0;
        EXPECT_VX_OBJECT(lut = vxCreateLUT(context, VX_TYPE_UINT8, 127), VX_TYPE_LUT);
        ref = (vx_reference)lut;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)lut;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseLUT(&lut));
    }

    {
        /* test graph reference */
        vx_graph graph = 0;
        EXPECT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ref = (vx_reference)graph;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)graph;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseGraph(&graph));
    }

    {
        /* test matrix reference */
        vx_matrix matrix = 0;
        EXPECT_VX_OBJECT(matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 32, 32), VX_TYPE_MATRIX);
        ref = (vx_reference)matrix;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)matrix;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseMatrix(&matrix));
    }

    {
        /* test pyramid reference */
        vx_pyramid pyramid = 0;
        EXPECT_VX_OBJECT(pyramid = vxCreatePyramid(context, 3, VX_SCALE_PYRAMID_HALF, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ref = (vx_reference)pyramid;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)pyramid;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleasePyramid(&pyramid));
    }

    {
        /* test remap reference */
        vx_remap remap = 0;
        EXPECT_VX_OBJECT(remap = vxCreateRemap(context, 320, 240, 160, 120), VX_TYPE_REMAP);
        ref = (vx_reference)remap;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)remap;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseRemap(&remap));
    }

    {
        /* test scalar reference */
        vx_scalar scalar = 0;
        vx_uint32 val = 5;
        EXPECT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT32, &val), VX_TYPE_SCALAR);
        ref = (vx_reference)scalar;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)scalar;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseScalar(&scalar));
    }

    {
        /* test threshold reference */
        vx_threshold threshold = 0;
        EXPECT_VX_OBJECT(threshold = vxCreateThreshold(context, VX_THRESHOLD_TYPE_BINARY, VX_TYPE_UINT8), VX_TYPE_THRESHOLD);
        ref = (vx_reference)threshold;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)threshold;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseThreshold(&threshold));
    }

    {
        /* test delay reference */
        vx_delay delay = 0;
        vx_image exemplar = 0;
        EXPECT_VX_OBJECT(exemplar = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        EXPECT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)exemplar, 5), VX_TYPE_DELAY);
        ref = (vx_reference)delay;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)delay;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseImage(&exemplar));
        VX_CALL(vxReleaseDelay(&delay));
    }

    {
        /* test kernel reference */
        vx_kernel kernel = 0;
        EXPECT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_ADD), VX_TYPE_KERNEL);
        ref = (vx_reference)kernel;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)kernel;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseKernel(&kernel));
    }

    {
        /* test parameter reference */
        vx_kernel kernel = 0;
        vx_parameter parameter = 0;
        EXPECT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_ADD), VX_TYPE_KERNEL);
        EXPECT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, 0), VX_TYPE_PARAMETER);
        ref = (vx_reference)parameter;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count0, sizeof(ref_count0)), VX_SUCCESS);
        VX_CALL(vxRetainReference(ref));
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 1);
        VX_CALL(vxReleaseReference(&ref));
        ref = (vx_reference)parameter;
        ref_count1 = 0;
        ASSERT_EQ_VX_STATUS(vxQueryReference(ref, VX_REFERENCE_COUNT, (void*)&ref_count1, sizeof(ref_count1)), VX_SUCCESS);
        ASSERT_EQ_INT(ref_count1 - ref_count0, 0);

        VX_CALL(vxReleaseKernel(&kernel));
        VX_CALL(vxReleaseParameter(&parameter));
    }

}


TEST(SmokeTest, test_vxRetainReference)
{
    vx_image image = 0;
    vx_graph graph = 0;
    vx_reference image_ref = 0, graph_ref = 0;
    vx_uint32 image_count = 0, graph_count = 0;
    vx_context context = context_->vx_context_;
    vx_uint32 num_refs1 = 0, num_refs2 = 0, num_refs3 = 0, num_refs4 = 0;

    ASSERT_EQ_VX_STATUS(vxQueryContext(context, VX_CONTEXT_REFERENCES, (void*)&num_refs1, sizeof(num_refs1)), VX_SUCCESS);

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_EQ_VX_STATUS(vxQueryContext(context, VX_CONTEXT_REFERENCES, (void*)&num_refs2, sizeof(num_refs2)), VX_SUCCESS);
    ASSERT_EQ_INT(num_refs2, num_refs1+2);

    image_ref = (vx_reference)image;
    graph_ref = (vx_reference)graph;
    ASSERT_EQ_VX_STATUS(vxQueryReference(image_ref, VX_REFERENCE_COUNT, (void*)&image_count, sizeof(image_count)), VX_SUCCESS);
    ASSERT_EQ_VX_STATUS(vxQueryReference(graph_ref, VX_REFERENCE_COUNT, (void*)&graph_count, sizeof(graph_count)), VX_SUCCESS);
    ASSERT_EQ_INT(image_count, 1);
    ASSERT_EQ_INT(graph_count, 1);

    image_ref = (vx_reference)image;
    graph_ref = (vx_reference)graph;
    VX_CALL(vxRetainReference(image_ref));
    VX_CALL(vxRetainReference(graph_ref));

    image_ref = (vx_reference)image;
    graph_ref = (vx_reference)graph;
    ASSERT_EQ_VX_STATUS(vxQueryReference(image_ref, VX_REFERENCE_COUNT, (void*)&image_count, sizeof(image_count)), VX_SUCCESS);
    ASSERT_EQ_VX_STATUS(vxQueryReference(graph_ref, VX_REFERENCE_COUNT, (void*)&graph_count, sizeof(graph_count)), VX_SUCCESS);
    ASSERT_EQ_INT(image_count, 2);
    ASSERT_EQ_INT(graph_count, 2);

    image_ref = (vx_reference)image;
    graph_ref = (vx_reference)graph;
    VX_CALL(vxReleaseReference(&image_ref));
    VX_CALL(vxReleaseReference(&graph_ref));

    ASSERT_EQ_PTR(0, image_ref);
    ASSERT_EQ_PTR(0, graph_ref);

    image_ref = (vx_reference)image;
    graph_ref = (vx_reference)graph;
    ASSERT_EQ_VX_STATUS(vxQueryReference(image_ref, VX_REFERENCE_COUNT, (void*)&image_count, sizeof(image_count)), VX_SUCCESS);
    ASSERT_EQ_VX_STATUS(vxQueryReference(graph_ref, VX_REFERENCE_COUNT, (void*)&graph_count, sizeof(graph_count)), VX_SUCCESS);
    ASSERT_EQ_INT(image_count, 1);
    ASSERT_EQ_INT(graph_count, 1);

    ASSERT_EQ_VX_STATUS(vxQueryContext(context, VX_CONTEXT_REFERENCES, (void*)&num_refs3, sizeof(num_refs3)), VX_SUCCESS);
    ASSERT_EQ_INT(num_refs3, num_refs1+2);

    image_ref = (vx_reference)image;
    graph_ref = (vx_reference)graph;
    VX_CALL(vxReleaseReference(&image_ref));
    VX_CALL(vxReleaseReference(&graph_ref));

    ASSERT_EQ_PTR(0, image_ref);
    ASSERT_EQ_PTR(0, graph_ref);

    ASSERT_EQ_VX_STATUS(vxQueryContext(context, VX_CONTEXT_REFERENCES, (void*)&num_refs4, sizeof(num_refs4)), VX_SUCCESS);
    ASSERT_EQ_INT(num_refs4, num_refs1);
}

TEST(SmokeTest, test_vxUnloadKernels)
{
    vx_context context = context_->vx_context_;
    vx_kernel kernel = NULL;
    vx_int32 num_modules1;
    vx_int32 num_modules2;
    vx_int32 num_unique_kernels1;
    vx_int32 num_unique_kernels2;

    ASSERT_EQ_VX_STATUS(vxQueryContext(context, VX_CONTEXT_MODULES, (void*)&num_modules1, sizeof(num_modules1)), VX_SUCCESS);
    ASSERT_EQ_VX_STATUS(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, (void*)&num_unique_kernels1, sizeof(num_unique_kernels1)), VX_SUCCESS);
    ASSERT(num_modules1 >= 0u);
    ASSERT(num_unique_kernels1 > 0u);

    kernel = vxGetKernelByName(context, "org.khronos.test.testmodule");
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)kernel));

    VX_CALL(vxLoadKernels(context, "test-testmodule"));
    ASSERT_VX_OBJECT(kernel = vxGetKernelByName(context, "org.khronos.test.testmodule"), VX_TYPE_KERNEL);
    VX_CALL(vxReleaseKernel(&kernel));

    ASSERT_EQ_VX_STATUS(vxQueryContext(context, VX_CONTEXT_MODULES, (void*)&num_modules2, sizeof(num_modules1)), VX_SUCCESS);
    ASSERT_EQ_VX_STATUS(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, (void*)&num_unique_kernels2, sizeof(num_unique_kernels2)), VX_SUCCESS);
    ASSERT(num_modules2 > num_modules1);
    ASSERT(num_unique_kernels2 > num_unique_kernels1);

    VX_CALL(vxUnloadKernels(context, "test-testmodule"));

    kernel = vxGetKernelByName(context, "org.khronos.test.testmodule");
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)kernel));

    ASSERT_EQ_VX_STATUS(vxQueryContext(context, VX_CONTEXT_MODULES, (void*)&num_modules2, sizeof(num_modules1)), VX_SUCCESS);
    ASSERT_EQ_VX_STATUS(vxQueryContext(context, VX_CONTEXT_UNIQUE_KERNELS, (void*)&num_unique_kernels2, sizeof(num_unique_kernels2)), VX_SUCCESS);
    ASSERT(num_modules2 == num_modules1);
    ASSERT(num_unique_kernels2 == num_unique_kernels1);
}

TEST(SmokeTest, test_vxSetReferenceName)
{
    vx_context context = context_->vx_context_;

    vx_image image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8);
    const char* image_name = "Image";
    char* actual_name = NULL;

    VX_CALL(vxSetReferenceName((vx_reference)image, image_name));
    VX_CALL(vxQueryReference((vx_reference)image, VX_REFERENCE_NAME, &actual_name, sizeof(actual_name)));

    ASSERT(0 == strcmp(image_name, actual_name));

    VX_CALL(vxReleaseImage(&image));
}

TEST(SmokeTest, test_vxSetParameterByIndex)
{
    vx_context context = context_->vx_context_;

    vx_enum kernel_id = VX_KERNEL_SOBEL_3x3; /* params: 0:req 1:opt 2:opt */
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;
    vx_node node = NULL;
    vx_uint32 width = 32, height = 32;
    vx_image image = NULL;
    vx_image dy = NULL;
    vx_uint32 node_parameters = 0;
    vx_uint32 i;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, kernel_id), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dy = vxCreateImage(context, width, height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    VX_CALL(vxQueryNode(node, VX_NODE_PARAMETERS, &node_parameters, sizeof(node_parameters)));

    // 1. check that vxCreateGenericNode sets all params to NULL
    for (i = 0; i < node_parameters; ++i)
    {
        vx_parameter param = NULL;
        vx_reference ref = NULL;

        ASSERT_VX_OBJECT(param = vxGetParameterByIndex(node, i), VX_TYPE_PARAMETER);

        VX_CALL(vxQueryParameter(param, VX_PARAMETER_REF, &ref, sizeof(ref)));
        ASSERT(ref == NULL);

        VX_CALL(vxReleaseParameter(&param));
    }

    // 2. check that vxSetParameterByIndex failed to set required params to NULL
    ASSERT(node_parameters == 3);
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxSetParameterByIndex(node, 0, (vx_reference)NULL)); // required parameter
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetParameterByIndex(node, 1, (vx_reference)NULL)); // output images are optional
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetParameterByIndex(node, 2, (vx_reference)dy)); // (but at least one should specified)

    // 3. check that vxVerifyGraph does not allow required params to be NULL
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetParameterByIndex(node, 0, (vx_reference)image));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseImage(&dy));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TESTCASE_TESTS(SmokeTest,
        test_vxRegisterUserStruct,
        test_vxHint,
        test_vxReleaseReference,
        test_vxRetainReference,
        test_vxUnloadKernels,
        test_vxSetReferenceName,
        test_vxSetParameterByIndex
        )
