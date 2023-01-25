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

#include <math.h>
#include "test_tiovx.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx.h>

#include "test_engine/test.h"

#define INPUT_NUM_ITEMS 12

#define OUTPUT0_NUM_ITEMS 3
#define OUTPUT1_NUM_ITEMS 3
#define OUTPUT2_NUM_ITEMS 3
#define OUTPUT3_NUM_ITEMS 3

#define TEST_IMAGE_WIDTH  1280U
#define TEST_IMAGE_HEIGHT 720U

static void fillSequence(CT_Image dst, uint32_t seq_init)
{
    uint32_t i, j;
    uint32_t val = seq_init;

    ASSERT(dst);
    ASSERT(dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = val;
}

TESTCASE(tivxObjArraySplit, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxObjArraySplit, testGraph)
{
    vx_object_array in, out0, out1, out2, out3;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node;
    vx_image input_exemplar;
    vx_image output0_exemplar;
    vx_image output1_exemplar;
    vx_image output2_exemplar;
    vx_image output3_exemplar;
    vx_size input_num_items = INPUT_NUM_ITEMS;
    vx_size output0_num_items = OUTPUT0_NUM_ITEMS;
    vx_size output1_num_items = OUTPUT1_NUM_ITEMS;
    vx_size output2_num_items = OUTPUT2_NUM_ITEMS;
    vx_size output3_num_items = OUTPUT3_NUM_ITEMS;
    int i, j, input_idx, input_final;

    /* Prepopulated CT Images */
    CT_Image ref_input[INPUT_NUM_ITEMS];
    CT_Image ref_output0[OUTPUT0_NUM_ITEMS];
    CT_Image ref_output1[OUTPUT1_NUM_ITEMS];
    CT_Image ref_output2[OUTPUT2_NUM_ITEMS];
    CT_Image ref_output3[OUTPUT3_NUM_ITEMS];

    /* Initial vx_images */
    vx_image in_test_image[INPUT_NUM_ITEMS];
    vx_image output0_test_image[OUTPUT0_NUM_ITEMS];
    vx_image output1_test_image[OUTPUT1_NUM_ITEMS];
    vx_image output2_test_image[OUTPUT2_NUM_ITEMS];
    vx_image output3_test_image[OUTPUT3_NUM_ITEMS];

    /* Resulting images from graph execution */
    CT_Image proc_input[INPUT_NUM_ITEMS];
    CT_Image proc_output0[OUTPUT0_NUM_ITEMS];
    CT_Image proc_output1[OUTPUT1_NUM_ITEMS];
    CT_Image proc_output2[OUTPUT2_NUM_ITEMS];
    CT_Image proc_output3[OUTPUT3_NUM_ITEMS];

    /* Processed vx_images */
    vx_image proc_in_test_image[INPUT_NUM_ITEMS];
    vx_image proc_output0_test_image[OUTPUT0_NUM_ITEMS];
    vx_image proc_output1_test_image[OUTPUT1_NUM_ITEMS];
    vx_image proc_output2_test_image[OUTPUT2_NUM_ITEMS];
    vx_image proc_output3_test_image[OUTPUT3_NUM_ITEMS];

    /* Sequence values for prepopulating images */
    uint32_t initial_input_seq = 1, initial_output_seq = 1+INPUT_NUM_ITEMS;
    uint32_t input_seq_init[INPUT_NUM_ITEMS];
    uint32_t output0_seq_init[OUTPUT0_NUM_ITEMS];
    uint32_t output1_seq_init[OUTPUT1_NUM_ITEMS];
    uint32_t output2_seq_init[OUTPUT2_NUM_ITEMS];
    uint32_t output3_seq_init[OUTPUT3_NUM_ITEMS];

    vx_perf_t perf_node, perf_graph;

    tivxHwaLoadKernels(context);

    ASSERT_VX_OBJECT(input_exemplar   = vxCreateImage(context, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output0_exemplar = vxCreateImage(context, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output1_exemplar = vxCreateImage(context, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output2_exemplar = vxCreateImage(context, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output3_exemplar = vxCreateImage(context, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(in   = vxCreateObjectArray(context, (vx_reference)input_exemplar, input_num_items), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(out0 = vxCreateObjectArray(context, (vx_reference)output0_exemplar, output0_num_items), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(out1 = vxCreateObjectArray(context, (vx_reference)output1_exemplar, output1_num_items), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(out2 = vxCreateObjectArray(context, (vx_reference)output2_exemplar, output2_num_items), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(out3 = vxCreateObjectArray(context, (vx_reference)output3_exemplar, output3_num_items), VX_TYPE_OBJECT_ARRAY);

    ASSERT_VX_OBJECT(node = tivxObjArraySplitNode(graph, in, out0, out1, out2, out3), VX_TYPE_NODE);

    /* Pre-loading all of the input/output images */
    {
        for (i = 0; i < INPUT_NUM_ITEMS; i++)
        {
            input_seq_init[i]   = initial_input_seq+i;
            ASSERT_NO_FAILURE({
                ref_input[i] = ct_allocate_image(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, VX_DF_IMAGE_U8);
                fillSequence(ref_input[i], (uint32_t)(input_seq_init[i]));
            });
            ASSERT_VX_OBJECT(in_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)in, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE(ct_image_copyto_vx_image(in_test_image[i], ref_input[i]));
            VX_CALL(vxReleaseImage(&in_test_image[i]));
        }
        for (i = 0; i < OUTPUT0_NUM_ITEMS; i++)
        {
            output0_seq_init[i] = initial_output_seq+i;
            ASSERT_NO_FAILURE({
                ref_output0[i] = ct_allocate_image(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, VX_DF_IMAGE_U8);
                fillSequence(ref_output0[i], (uint32_t)(output0_seq_init[i]));
            });
            ASSERT_VX_OBJECT(output0_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)out0, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE(ct_image_copyto_vx_image(output0_test_image[i], ref_output0[i]));
            VX_CALL(vxReleaseImage(&output0_test_image[i]));
        }
        for (i = 0; i < OUTPUT1_NUM_ITEMS; i++)
        {
            output1_seq_init[i] = OUTPUT0_NUM_ITEMS+i;
            ASSERT_NO_FAILURE({
                ref_output1[i] = ct_allocate_image(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, VX_DF_IMAGE_U8);
                fillSequence(ref_output1[i], (uint32_t)(output1_seq_init[i]));
            });
            ASSERT_VX_OBJECT(output1_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)out1, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE(ct_image_copyto_vx_image(output1_test_image[i], ref_output1[i]));
            VX_CALL(vxReleaseImage(&output1_test_image[i]));
        }
        for (i = 0; i < OUTPUT2_NUM_ITEMS; i++)
        {
            output2_seq_init[i] = OUTPUT0_NUM_ITEMS+OUTPUT1_NUM_ITEMS+i;
            ASSERT_NO_FAILURE({
                ref_output2[i] = ct_allocate_image(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, VX_DF_IMAGE_U8);
                fillSequence(ref_output2[i], (uint32_t)(output2_seq_init[i]));
            });
            ASSERT_VX_OBJECT(output2_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)out2, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE(ct_image_copyto_vx_image(output2_test_image[i], ref_output2[i]));
            VX_CALL(vxReleaseImage(&output2_test_image[i]));
        }
        for (i = 0; i < OUTPUT3_NUM_ITEMS; i++)
        {
            output3_seq_init[i] = OUTPUT0_NUM_ITEMS+OUTPUT1_NUM_ITEMS+OUTPUT2_NUM_ITEMS+i;
            ASSERT_NO_FAILURE({
                ref_output3[i] = ct_allocate_image(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, VX_DF_IMAGE_U8);
                fillSequence(ref_output3[i], (uint32_t)(output3_seq_init[i]));
            });
            ASSERT_VX_OBJECT(output3_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)out3, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE(ct_image_copyto_vx_image(output3_test_image[i], ref_output3[i]));
            VX_CALL(vxReleaseImage(&output3_test_image[i]));
        }
    }

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    /* Validation code for first process to confirm swap functionality */
    {
        for (i = 0; i < INPUT_NUM_ITEMS; i++)
        {
            ASSERT_VX_OBJECT(proc_in_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)in, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE({
                proc_input[i] = ct_image_from_vx_image(proc_in_test_image[i]);
            });
        }

        for (i = 0; i < OUTPUT0_NUM_ITEMS; i++)
        {
            ASSERT_VX_OBJECT(proc_output0_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)out0, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE({
                proc_output0[i] = ct_image_from_vx_image(proc_output0_test_image[i]);
            });
        }

        for (i = 0; i < OUTPUT1_NUM_ITEMS; i++)
        {
            ASSERT_VX_OBJECT(proc_output1_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)out1, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE({
                proc_output1[i] = ct_image_from_vx_image(proc_output1_test_image[i]);
            });
        }

        for (i = 0; i < OUTPUT2_NUM_ITEMS; i++)
        {
            ASSERT_VX_OBJECT(proc_output2_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)out2, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE({
                proc_output2[i] = ct_image_from_vx_image(proc_output2_test_image[i]);
            });
        }

        for (i = 0; i < OUTPUT3_NUM_ITEMS; i++)
        {
            ASSERT_VX_OBJECT(proc_output3_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)out3, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE({
                proc_output3[i] = ct_image_from_vx_image(proc_output3_test_image[i]);
            });
        }

        input_idx = 0;
        input_final = OUTPUT0_NUM_ITEMS;

        /* Validating that the swap of the input/output occurred */
        for (i = input_idx; i < OUTPUT0_NUM_ITEMS; i++)
        {
            ASSERT_EQ_CTIMAGE(ref_input[i], proc_output0[i]);
            ASSERT_EQ_CTIMAGE(ref_output0[i], proc_input[i]);
        }

        j = 0;
        input_idx+=OUTPUT0_NUM_ITEMS;
        input_final+=OUTPUT1_NUM_ITEMS;

        for (i = input_idx; i < input_final; i++)
        {
            ASSERT_EQ_CTIMAGE(ref_input[i], proc_output1[j]);
            ASSERT_EQ_CTIMAGE(ref_output1[j], proc_input[i]);
            j++;
        }

        j = 0;
        input_idx+=OUTPUT1_NUM_ITEMS;
        input_final+=OUTPUT2_NUM_ITEMS;

        for (i = input_idx; i < input_final; i++)
        {
            ASSERT_EQ_CTIMAGE(ref_input[i], proc_output2[j]);
            ASSERT_EQ_CTIMAGE(ref_output2[j], proc_input[i]);
            j++;
        }

        j = 0;
        input_idx+=OUTPUT2_NUM_ITEMS;
        input_final+=OUTPUT3_NUM_ITEMS;

        for (i = input_idx; i < input_final; i++)
        {
            ASSERT_EQ_CTIMAGE(ref_input[i], proc_output3[j]);
            ASSERT_EQ_CTIMAGE(ref_output3[j], proc_input[i]);
            j++;
        }

        for (i = 0; i < INPUT_NUM_ITEMS; i++)
        {
            VX_CALL(vxReleaseImage(&proc_in_test_image[i]));
        }

        for (i = 0; i < OUTPUT0_NUM_ITEMS; i++)
        {
            VX_CALL(vxReleaseImage(&proc_output0_test_image[i]));
        }

        for (i = 0; i < OUTPUT1_NUM_ITEMS; i++)
        {
            VX_CALL(vxReleaseImage(&proc_output1_test_image[i]));
        }

        for (i = 0; i < OUTPUT2_NUM_ITEMS; i++)
        {
            VX_CALL(vxReleaseImage(&proc_output2_test_image[i]));
        }

        for (i = 0; i < OUTPUT3_NUM_ITEMS; i++)
        {
            VX_CALL(vxReleaseImage(&proc_output3_test_image[i]));
        }
    }

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    /* Validation code: Since the graph has now been processed twice, the images should be the same as the initial references */
    {
        for (i = 0; i < INPUT_NUM_ITEMS; i++)
        {
            ASSERT_VX_OBJECT(proc_in_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)in, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE({
                proc_input[i] = ct_image_from_vx_image(proc_in_test_image[i]);
            });
            ASSERT_EQ_CTIMAGE(ref_input[i], proc_input[i]);
        }

        for (i = 0; i < OUTPUT0_NUM_ITEMS; i++)
        {
            ASSERT_VX_OBJECT(proc_output0_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)out0, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE({
                proc_output0[i] = ct_image_from_vx_image(proc_output0_test_image[i]);
            });
            ASSERT_EQ_CTIMAGE(ref_output0[i], proc_output0[i]);
        }

        for (i = 0; i < OUTPUT1_NUM_ITEMS; i++)
        {
            ASSERT_VX_OBJECT(proc_output1_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)out1, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE({
                proc_output1[i] = ct_image_from_vx_image(proc_output1_test_image[i]);
            });
            ASSERT_EQ_CTIMAGE(ref_output1[i], proc_output1[i]);
        }

        for (i = 0; i < OUTPUT2_NUM_ITEMS; i++)
        {
            ASSERT_VX_OBJECT(proc_output2_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)out2, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE({
                proc_output2[i] = ct_image_from_vx_image(proc_output2_test_image[i]);
            });
            ASSERT_EQ_CTIMAGE(ref_output2[i], proc_output2[i]);
        }

        for (i = 0; i < OUTPUT3_NUM_ITEMS; i++)
        {
            ASSERT_VX_OBJECT(proc_output3_test_image[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)out3, i), VX_TYPE_IMAGE);
            ASSERT_NO_FAILURE({
                proc_output3[i] = ct_image_from_vx_image(proc_output3_test_image[i]);
            });
            ASSERT_EQ_CTIMAGE(ref_output3[i], proc_output3[i]);
        }

        for (i = 0; i < INPUT_NUM_ITEMS; i++)
        {
            VX_CALL(vxReleaseImage(&proc_in_test_image[i]));
        }

        for (i = 0; i < OUTPUT0_NUM_ITEMS; i++)
        {
            VX_CALL(vxReleaseImage(&proc_output0_test_image[i]));
        }

        for (i = 0; i < OUTPUT1_NUM_ITEMS; i++)
        {
            VX_CALL(vxReleaseImage(&proc_output1_test_image[i]));
        }

        for (i = 0; i < OUTPUT2_NUM_ITEMS; i++)
        {
            VX_CALL(vxReleaseImage(&proc_output2_test_image[i]));
        }

        for (i = 0; i < OUTPUT3_NUM_ITEMS; i++)
        {
            VX_CALL(vxReleaseImage(&proc_output3_test_image[i]));
        }
    }

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node, sizeof(perf_node));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    printPerformance(perf_node, TEST_IMAGE_WIDTH*TEST_IMAGE_HEIGHT, "tivxObjArraySplit");
    printPerformance(perf_graph, TEST_IMAGE_WIDTH*TEST_IMAGE_HEIGHT, "Graph_tivxObjArraySplit");

    VX_CALL(vxReleaseImage(&input_exemplar));
    VX_CALL(vxReleaseImage(&output0_exemplar));
    VX_CALL(vxReleaseImage(&output1_exemplar));
    VX_CALL(vxReleaseImage(&output2_exemplar));
    VX_CALL(vxReleaseImage(&output3_exemplar));
    VX_CALL(vxReleaseObjectArray(&in));
    VX_CALL(vxReleaseObjectArray(&out0));
    VX_CALL(vxReleaseObjectArray(&out1));
    VX_CALL(vxReleaseObjectArray(&out2));
    VX_CALL(vxReleaseObjectArray(&out3));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    tivxHwaUnLoadKernels(context);
}

TEST(tivxObjArraySplit, negativeTestGraph)
{
    vx_object_array in, out0, out1, out2, out3;
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_node node;
    vx_image exemplar;
    vx_size input_num_items = INPUT_NUM_ITEMS;
    vx_size output0_num_items = OUTPUT0_NUM_ITEMS;
    vx_size output1_num_items = OUTPUT1_NUM_ITEMS;
    vx_size output2_num_items = OUTPUT2_NUM_ITEMS;
    vx_size output3_num_items = 2;

    tivxHwaLoadKernels(context);

    ASSERT_VX_OBJECT(exemplar = vxCreateImage(context, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(in = vxCreateObjectArray(context, (vx_reference)exemplar, input_num_items), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(out0 = vxCreateObjectArray(context, (vx_reference)exemplar, output0_num_items), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(out1 = vxCreateObjectArray(context, (vx_reference)exemplar, output1_num_items), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(out2 = vxCreateObjectArray(context, (vx_reference)exemplar, output2_num_items), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(out3 = vxCreateObjectArray(context, (vx_reference)exemplar, output3_num_items), VX_TYPE_OBJECT_ARRAY);

    ASSERT_VX_OBJECT(node = tivxObjArraySplitNode(graph, in, out0, out1, out2, out3), VX_TYPE_NODE);

    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseImage(&exemplar));
    VX_CALL(vxReleaseObjectArray(&in));
    VX_CALL(vxReleaseObjectArray(&out0));
    VX_CALL(vxReleaseObjectArray(&out1));
    VX_CALL(vxReleaseObjectArray(&out2));
    VX_CALL(vxReleaseObjectArray(&out3));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    tivxHwaUnLoadKernels(context);
}

TESTCASE_TESTS(
    tivxObjArraySplit,
    testGraph,
    negativeTestGraph
)
