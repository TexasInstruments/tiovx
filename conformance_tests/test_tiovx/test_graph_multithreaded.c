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
#include <TI/tivx_task.h>

#define TIVX_TARGET_DEFAULT_STACK_SIZE      (64U * 1024U)
#define TIVX_TARGET_DEFAULT_TASK_PRIORITY1   (8u)
#define TIVX_TARGET_DEFAULT_TASK_PRIORITY2   (7u)
#define TIVX_TARGET_DEFAULT_TASK_PRIORITY3   (6u)

TESTCASE(tivxGraphMultiThreaded, CT_VXContext, ct_setup_vx_context, 0)

static void referenceNot(CT_Image src, CT_Image dst)
{
    uint32_t i, j;

    ASSERT(src && dst);
    ASSERT(src->width == dst->width);
    ASSERT(src->height == dst->height);
    ASSERT(src->format == dst->format && src->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = ~src->data.y[i * src->stride + j];
}

static CT_Image accumulate_weighted_generate_random_8u(int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}


static void accumulate_weighted_reference(CT_Image input, vx_float32 alpha, CT_Image accum)
{
    CT_FILL_IMAGE_8U(return, accum,
            {
                uint8_t* input_data = CT_IMAGE_DATA_PTR_8U(input, x, y);
                vx_float32 res = (1 - alpha) * ((vx_float32)(int32_t)(*dst_data)) + (alpha) * ((vx_float32)(int32_t)(*input_data));
                uint8_t res8 = CT_SATURATE_U8(res);
                *dst_data = res8;
            });
}

static void accumulate_weighted_check(CT_Image input, vx_float32 alpha, CT_Image accum_src, CT_Image accum_dst)
{
    CT_Image accum_ref = NULL;

    ASSERT(input && accum_src && accum_dst);

    ASSERT_NO_FAILURE(accum_ref = ct_image_create_clone(accum_src));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(input, alpha, accum_ref));

    EXPECT_CTIMAGE_NEAR(accum_ref, accum_dst, 1);
}

static void accumulate_multiple_weighted_check(CT_Image input, vx_float32 alpha_intermediate, vx_float32 alpha_final, CT_Image accum_src, CT_Image accum_final, CT_Image accum_dst)
{
    CT_Image accum_ref = NULL, accum_intermediate = NULL;

    ASSERT(input && accum_src && accum_final && accum_dst);

    ASSERT_NO_FAILURE(accum_intermediate = ct_image_create_clone(accum_src));

    ASSERT_NO_FAILURE(accum_ref = ct_image_create_clone(accum_final));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(input, alpha_intermediate, accum_intermediate));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(accum_intermediate, alpha_final, accum_ref));

    EXPECT_CTIMAGE_NEAR(accum_ref, accum_dst, 2); // Changed tolerance to 2 for the amount of nodes
}

static void accumulate_not_multiple_weighted_check(CT_Image input_not, CT_Image input_acc, CT_Image virtual_dummy,
            vx_float32 alpha_intermediate, vx_float32 alpha_final, CT_Image accum_final, CT_Image accum_dst)
{
    CT_Image accum_ref = NULL, accum_intermediate = NULL;

    ASSERT(input_not && input_acc && accum_final && accum_dst);

    ASSERT_NO_FAILURE(accum_ref = ct_image_create_clone(accum_final));

    ASSERT_NO_FAILURE(accum_intermediate = ct_image_create_clone(input_acc));

    ASSERT_NO_FAILURE(referenceNot(input_not, virtual_dummy));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(virtual_dummy, alpha_intermediate, accum_intermediate));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(accum_intermediate, alpha_final, accum_ref));

    EXPECT_CTIMAGE_NEAR(accum_ref, accum_dst, 2); // Changed tolerance to 2
}

static void alternate_node_check(CT_Image input_not, CT_Image input_acc_1, CT_Image input_acc_2, CT_Image virtual_dummy_1,
            CT_Image virtual_dummy_2, CT_Image virtual_dummy_3, vx_float32 alpha_intermediate1, vx_float32 alpha_intermediate2,
            vx_float32 alpha_final, CT_Image accum_final, CT_Image accum_dst)
{
    CT_Image accum_ref = NULL, accum_intermediate_1 = NULL, accum_intermediate_2 = NULL;

    ASSERT(input_not && input_acc_1 && input_acc_2 && accum_final && accum_dst);

    ASSERT_NO_FAILURE(accum_ref = ct_image_create_clone(accum_final));

    ASSERT_NO_FAILURE(accum_intermediate_1 = ct_image_create_clone(input_acc_1));

    ASSERT_NO_FAILURE(accum_intermediate_2 = ct_image_create_clone(input_acc_2));

    ASSERT_NO_FAILURE(referenceNot(input_not, virtual_dummy_1));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(virtual_dummy_1, alpha_intermediate1, accum_intermediate_1));

    ASSERT_NO_FAILURE(referenceNot(accum_intermediate_1, virtual_dummy_2));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(virtual_dummy_2, alpha_intermediate2, accum_intermediate_2));

    ASSERT_NO_FAILURE(referenceNot(accum_intermediate_2, virtual_dummy_3));

    ASSERT_NO_FAILURE(accumulate_weighted_reference(virtual_dummy_3, alpha_final, accum_ref));

    EXPECT_CTIMAGE_NEAR(accum_ref, accum_dst, 2); // Changed tolerance to 2
}

typedef struct {
    const char* testName;
    vx_float32 alpha_intermediate, alpha_final;
    int width, height;
} Arg;


#define PARAMETERS \
    CT_GENERATE_PARAMETERS("random/alphaintermediate0.5f/alphafinal0.25f", ADD_SIZE_18x18, ARG, 0.5f, 0.25f), \
    CT_GENERATE_PARAMETERS("random/alpha0.33f/alphafinal0.67f", ADD_SIZE_644x258, ARG, 0.33f, 0.67f), \
    CT_GENERATE_PARAMETERS("random/alpha0.99f/alphafinal0.8f", ADD_SIZE_1600x1200, ARG, 0.99f, 0.8f)

volatile static int taskFinished1 = 0;
volatile static int taskFinished2 = 0;
volatile static int taskFinished3 = 0;

static void VX_CALLBACK tivxTask1(void *app_var)
{
    vx_graph graph1 = (vx_graph)app_var;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph1));

    taskFinished1 = 1;
}

static void VX_CALLBACK tivxTask2(void *app_var)
{
    vx_graph graph2 = (vx_graph)app_var;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph2));

    taskFinished2 = 1;
}

static void VX_CALLBACK tivxTask3(void *app_var)
{
    vx_graph graph3 = (vx_graph)app_var;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph3));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph3));

    taskFinished3 = 1;
}

TEST_WITH_ARG(tivxGraphMultiThreaded, testParallelGraphsSameTarget, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image1 = 0, input_image2 = 0, accum_image_final1 = 0, accum_image_final2 = 0;
    vx_scalar alpha_scalar1, alpha_scalar2 = 0;
    vx_graph graph1 = 0, graph2 = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph1, perf_graph2;
    tivx_task_create_params_t taskParams1, taskParams2;
    tivx_task taskHandle1, taskHandle2;
    uint32_t wait_loop_cnt = 0;

    taskFinished1 = 0;
    taskFinished2 = 0;

    CT_Image input1 = NULL, input2 = NULL, accum_final1 = NULL, accum_final2 = NULL, accum_dst1 = NULL, accum_dst2 = NULL;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(alpha_scalar1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image1 = ct_image_to_vx_image(input1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image2 = ct_image_to_vx_image(input2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final1 = ct_image_to_vx_image(accum_final1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final2 = ct_image_to_vx_image(accum_final2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxAccumulateWeightedImageNode(graph1, input_image1, alpha_scalar1, accum_image_final1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxAccumulateWeightedImageNode(graph2, input_image2, alpha_scalar2, accum_image_final2), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-1"));

    // Setting up task params for task 1
    tivxTaskSetDefaultCreateParams(&taskParams1);
    taskParams1.task_main = &tivxTask1;
    taskParams1.app_var = graph1;
    taskParams1.stack_ptr = NULL;
    taskParams1.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    taskParams1.core_affinity = TIVX_TASK_AFFINITY_ANY;
    taskParams1.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY1;

    // Setting up task params for task 2
    tivxTaskSetDefaultCreateParams(&taskParams2);
    taskParams2.task_main = &tivxTask2;
    taskParams2.app_var = graph2;
    taskParams2.stack_ptr = NULL;
    taskParams2.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    taskParams2.core_affinity = TIVX_TASK_AFFINITY_ANY;
    taskParams2.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY2;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle1, &taskParams1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle2, &taskParams2));

    wait_loop_cnt = 0;
    do
    {
        tivxTaskWaitMsecs(1000);

        if (taskFinished1 != 0 && taskFinished2 != 0)
        {
            break;
        }
        wait_loop_cnt ++;

        /* Waiting for twenty seconds */
        if (wait_loop_cnt > 20)
        {
            break;
        }
    } while (1);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));

    vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));

    ASSERT_NO_FAILURE(accum_dst1 = ct_image_from_vx_image(accum_image_final1));

    ASSERT_NO_FAILURE(accum_dst2 = ct_image_from_vx_image(accum_image_final2));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input1, arg_->alpha_intermediate, accum_final1, accum_dst1));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input2, arg_->alpha_final, accum_final2, accum_dst2));

    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseGraph(&graph1));

    ASSERT(node2 == 0);
    ASSERT(node1 == 0);
    ASSERT(graph2 == 0);
    ASSERT(graph1 == 0);

    VX_CALL(vxReleaseImage(&accum_image_final2));
    VX_CALL(vxReleaseImage(&accum_image_final1));
    VX_CALL(vxReleaseImage(&input_image2));
    VX_CALL(vxReleaseImage(&input_image1));
    VX_CALL(vxReleaseScalar(&alpha_scalar2));
    VX_CALL(vxReleaseScalar(&alpha_scalar1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle2));

    ASSERT(accum_image_final1 == 0);
    ASSERT(accum_image_final2 == 0);
    ASSERT(input_image1 == 0);
    ASSERT(input_image2 == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_graph1, arg_->width*arg_->height, "G1");
    printPerformance(perf_graph2, arg_->width*arg_->height, "G2");
}

TEST_WITH_ARG(tivxGraphMultiThreaded, testParallelGraphsDifferentTarget, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image1 = 0, input_image2 = 0, accum_image_final1 = 0, accum_image_final2 = 0;
    vx_scalar alpha_scalar1, alpha_scalar2 = 0;
    vx_graph graph1 = 0, graph2 = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph1, perf_graph2;
    tivx_task_create_params_t taskParams1, taskParams2;
    tivx_task taskHandle1, taskHandle2;
    uint32_t wait_loop_cnt = 0;

    taskFinished1 = 0;
    taskFinished2 = 0;

    CT_Image input1 = NULL, input2 = NULL, accum_final1 = NULL, accum_final2 = NULL, accum_dst1 = NULL, accum_dst2 = NULL;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(alpha_scalar1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image1 = ct_image_to_vx_image(input1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image2 = ct_image_to_vx_image(input2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final1 = ct_image_to_vx_image(accum_final1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final2 = ct_image_to_vx_image(accum_final2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxAccumulateWeightedImageNode(graph1, input_image1, alpha_scalar1, accum_image_final1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxAccumulateWeightedImageNode(graph2, input_image2, alpha_scalar2, accum_image_final2), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, "DSP-1"));

    if (vx_true_e == tivxIsTargetEnabled("DSP-2"))
    {
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-2"));
    }
    else
    {
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-1"));
    }

    // Setting up task params for task 1
    tivxTaskSetDefaultCreateParams(&taskParams1);
    taskParams1.task_main = &tivxTask1;
    taskParams1.app_var = graph1;
    taskParams1.stack_ptr = NULL;
    taskParams1.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    taskParams1.core_affinity = TIVX_TASK_AFFINITY_ANY;
    taskParams1.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY1;

    // Setting up task params for task 2
    tivxTaskSetDefaultCreateParams(&taskParams2);
    taskParams2.task_main = &tivxTask2;
    taskParams2.app_var = graph2;
    taskParams2.stack_ptr = NULL;
    taskParams2.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    taskParams2.core_affinity = TIVX_TASK_AFFINITY_ANY;
    taskParams2.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY2;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle1, &taskParams1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle2, &taskParams2));

    wait_loop_cnt = 0;
    do
    {
        tivxTaskWaitMsecs(1000);

        if (taskFinished1 != 0 && taskFinished2 != 0)
        {
            break;
        }
        wait_loop_cnt ++;

        /* Waiting for twenty seconds */
        if (wait_loop_cnt > 20)
        {
            break;
        }
    } while (1);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));

    vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));

    ASSERT_NO_FAILURE(accum_dst1 = ct_image_from_vx_image(accum_image_final1));

    ASSERT_NO_FAILURE(accum_dst2 = ct_image_from_vx_image(accum_image_final2));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input1, arg_->alpha_intermediate, accum_final1, accum_dst1));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input2, arg_->alpha_final, accum_final2, accum_dst2));

    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseGraph(&graph1));

    ASSERT(node2 == 0);
    ASSERT(node1 == 0);
    ASSERT(graph2 == 0);
    ASSERT(graph1 == 0);

    VX_CALL(vxReleaseImage(&accum_image_final2));
    VX_CALL(vxReleaseImage(&accum_image_final1));
    VX_CALL(vxReleaseImage(&input_image2));
    VX_CALL(vxReleaseImage(&input_image1));
    VX_CALL(vxReleaseScalar(&alpha_scalar2));
    VX_CALL(vxReleaseScalar(&alpha_scalar1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle2));

    ASSERT(accum_image_final1 == 0);
    ASSERT(accum_image_final2 == 0);
    ASSERT(input_image1 == 0);
    ASSERT(input_image2 == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_graph1, arg_->width*arg_->height, "G1");
    printPerformance(perf_graph2, arg_->width*arg_->height, "G2");
}

TEST_WITH_ARG(tivxGraphMultiThreaded, testParallelGraphsMultipleNodes, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image_graph1 = 0, accum_image_intermediate_graph1 = 0, accum_image_final_graph1 = 0;
    vx_image input_image_graph2 = 0, accum_image_intermediate_graph2 = 0, accum_image_final_graph2 = 0;
    vx_scalar alpha_scalar_graph1, alpha_scalar_final_graph1 = 0;
    vx_scalar alpha_scalar_graph2, alpha_scalar_final_graph2 = 0;
    vx_graph graph1 = 0, graph2 = 0;
    vx_node node1_graph1 = 0, node2_graph1 = 0, node1_graph2 = 0, node2_graph2 = 0;
    vx_perf_t perf_node1_graph1, perf_node2_graph1, perf_node1_graph2, perf_node2_graph2, perf_graph1, perf_graph2;
    int widthHardCoded = 640, heightHardCoded = 480;

    CT_Image input_graph1 = NULL, accum_src_graph1 = NULL, accum_final_graph1 = NULL, accum_dst_graph1 = NULL;
    CT_Image input_graph2 = NULL, accum_src_graph2 = NULL, accum_final_graph2 = NULL, accum_dst_graph2 = NULL;

    tivx_task_create_params_t taskParams1, taskParams2;
    tivx_task taskHandle1, taskHandle2;
    uint32_t wait_loop_cnt = 0;

    taskFinished1 = 0;
    taskFinished2 = 0;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(alpha_scalar_graph1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar_graph2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar_final_graph1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar_final_graph2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input_graph1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input_graph2 = accumulate_weighted_generate_random_8u(widthHardCoded, heightHardCoded));

    ASSERT_NO_FAILURE(accum_src_graph1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_src_graph2 = accumulate_weighted_generate_random_8u(widthHardCoded, heightHardCoded));

    ASSERT_NO_FAILURE(accum_final_graph1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final_graph2 = accumulate_weighted_generate_random_8u(widthHardCoded, heightHardCoded));

    ASSERT_VX_OBJECT(input_image_graph1 = ct_image_to_vx_image(input_graph1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image_graph2 = ct_image_to_vx_image(input_graph2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_intermediate_graph1 = ct_image_to_vx_image(accum_src_graph1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_intermediate_graph2 = ct_image_to_vx_image(accum_src_graph2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final_graph1 = ct_image_to_vx_image(accum_final_graph1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final_graph2 = ct_image_to_vx_image(accum_final_graph2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1_graph1 = vxAccumulateWeightedImageNode(graph1, input_image_graph1, alpha_scalar_graph1, accum_image_intermediate_graph1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2_graph1 = vxAccumulateWeightedImageNode(graph1, accum_image_intermediate_graph1, alpha_scalar_final_graph1, accum_image_final_graph1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node1_graph2 = vxAccumulateWeightedImageNode(graph2, input_image_graph2, alpha_scalar_graph2, accum_image_intermediate_graph2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2_graph2 = vxAccumulateWeightedImageNode(graph2, accum_image_intermediate_graph2, alpha_scalar_final_graph2, accum_image_final_graph2), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node1_graph1, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node2_graph1, VX_TARGET_STRING, "DSP-1"));

    if (vx_true_e == tivxIsTargetEnabled("DSP-2"))
    {
        VX_CALL(vxSetNodeTarget(node1_graph2, VX_TARGET_STRING, "DSP-2"));
        VX_CALL(vxSetNodeTarget(node2_graph2, VX_TARGET_STRING, "DSP-2"));
    }
    else
    {
        VX_CALL(vxSetNodeTarget(node1_graph2, VX_TARGET_STRING, "DSP-1"));
        VX_CALL(vxSetNodeTarget(node2_graph2, VX_TARGET_STRING, "DSP-1"));
    }

    // Setting up task params for task 1
    tivxTaskSetDefaultCreateParams(&taskParams1);
    taskParams1.task_main = &tivxTask1;
    taskParams1.app_var = graph1;
    taskParams1.stack_ptr = NULL;
    taskParams1.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    taskParams1.core_affinity = TIVX_TASK_AFFINITY_ANY;
    taskParams1.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY1;

    // Setting up task params for task 2
    tivxTaskSetDefaultCreateParams(&taskParams2);
    taskParams2.task_main = &tivxTask2;
    taskParams2.app_var = graph2;
    taskParams2.stack_ptr = NULL;
    taskParams2.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    taskParams2.core_affinity = TIVX_TASK_AFFINITY_ANY;
    taskParams2.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY2;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle1, &taskParams1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle2, &taskParams2));

    wait_loop_cnt = 0;
    do
    {
        tivxTaskWaitMsecs(1000);

        if (taskFinished1 != 0 && taskFinished2 != 0)
        {
            break;
        }
        wait_loop_cnt ++;

        /* Waiting for twenty seconds */
        if (wait_loop_cnt > 20)
        {
            break;
        }
    } while (1);

    vxQueryNode(node1_graph1, VX_NODE_PERFORMANCE, &perf_node1_graph1, sizeof(perf_node1_graph1));
    vxQueryNode(node2_graph1, VX_NODE_PERFORMANCE, &perf_node2_graph1, sizeof(perf_node2_graph1));
    vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));

    vxQueryNode(node1_graph2, VX_NODE_PERFORMANCE, &perf_node1_graph2, sizeof(perf_node1_graph2));
    vxQueryNode(node2_graph2, VX_NODE_PERFORMANCE, &perf_node2_graph2, sizeof(perf_node2_graph2));
    vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));

    ASSERT_NO_FAILURE(accum_dst_graph1 = ct_image_from_vx_image(accum_image_final_graph1));

    ASSERT_NO_FAILURE(accum_dst_graph2 = ct_image_from_vx_image(accum_image_final_graph2));

    ASSERT_NO_FAILURE(accumulate_multiple_weighted_check(input_graph1, arg_->alpha_intermediate, arg_->alpha_final, accum_src_graph1, accum_final_graph1, accum_dst_graph1));

    ASSERT_NO_FAILURE(accumulate_multiple_weighted_check(input_graph2, arg_->alpha_intermediate, arg_->alpha_final, accum_src_graph2, accum_final_graph2, accum_dst_graph2));

    VX_CALL(vxReleaseNode(&node2_graph2));
    VX_CALL(vxReleaseNode(&node1_graph2));
    VX_CALL(vxReleaseNode(&node2_graph1));
    VX_CALL(vxReleaseNode(&node1_graph1));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseGraph(&graph1));

    ASSERT(node2_graph2 == 0);
    ASSERT(node1_graph2 == 0);
    ASSERT(node2_graph1 == 0);
    ASSERT(node1_graph1 == 0);
    ASSERT(graph2 == 0);
    ASSERT(graph1 == 0);

    VX_CALL(vxReleaseImage(&accum_image_final_graph1));
    VX_CALL(vxReleaseImage(&accum_image_final_graph2));
    VX_CALL(vxReleaseImage(&accum_image_intermediate_graph1));
    VX_CALL(vxReleaseImage(&accum_image_intermediate_graph2));
    VX_CALL(vxReleaseImage(&input_image_graph1));
    VX_CALL(vxReleaseImage(&input_image_graph2));
    VX_CALL(vxReleaseScalar(&alpha_scalar_final_graph1));
    VX_CALL(vxReleaseScalar(&alpha_scalar_final_graph2));
    VX_CALL(vxReleaseScalar(&alpha_scalar_graph1));
    VX_CALL(vxReleaseScalar(&alpha_scalar_graph2));

    ASSERT(accum_image_final_graph1 == 0);
    ASSERT(accum_image_final_graph2 == 0);
    ASSERT(accum_image_intermediate_graph1 == 0);
    ASSERT(accum_image_intermediate_graph2 == 0);
    ASSERT(input_image_graph1 == 0);
    ASSERT(input_image_graph2 == 0);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle2));

    printPerformance(perf_node1_graph1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2_graph1, arg_->width*arg_->height, "N2");
    printPerformance(perf_graph1, arg_->width*arg_->height, "G1");
    printPerformance(perf_node1_graph2, widthHardCoded*heightHardCoded, "N1");
    printPerformance(perf_node1_graph2, widthHardCoded*heightHardCoded, "N2");
    printPerformance(perf_node2_graph2, widthHardCoded*heightHardCoded, "G2");
}

TEST_WITH_ARG(tivxGraphMultiThreaded, testThreeParallelGraphs, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image input_image1 = 0, input_image2 = 0, input_image3 = 0, accum_image_final1 = 0, accum_image_final2 = 0, accum_image_final3 = 0;
    vx_scalar alpha_scalar1, alpha_scalar2 = 0, alpha_scalar3 = 0;
    vx_graph graph1 = 0, graph2 = 0, graph3 = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_graph1, perf_graph2, perf_graph3;

    CT_Image input1 = NULL, input2 = NULL, input3 = NULL, accum_final1 = NULL, accum_final2 = NULL, accum_final3 = NULL;
    CT_Image accum_dst1 = NULL, accum_dst2 = NULL, accum_dst3 = NULL;

    tivx_task_create_params_t taskParams1, taskParams2, taskParams3;
    tivx_task taskHandle1, taskHandle2, taskHandle3;
    uint32_t wait_loop_cnt = 0;

    taskFinished1 = 0;
    taskFinished2 = 0;
    taskFinished3 = 0;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(alpha_scalar1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar3 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input3 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final3 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image1 = ct_image_to_vx_image(input1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image2 = ct_image_to_vx_image(input2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image3 = ct_image_to_vx_image(input3, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final1 = ct_image_to_vx_image(accum_final1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final2 = ct_image_to_vx_image(accum_final2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final3 = ct_image_to_vx_image(accum_final3, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(graph3 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node1 = vxAccumulateWeightedImageNode(graph1, input_image1, alpha_scalar1, accum_image_final1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxAccumulateWeightedImageNode(graph2, input_image2, alpha_scalar2, accum_image_final2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxAccumulateWeightedImageNode(graph3, input_image3, alpha_scalar3, accum_image_final3), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, "DSP-1"));

    if (vx_true_e == tivxIsTargetEnabled("DSP-2"))
    {
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-2"));
    }
    else
    {
        VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-1"));
    }

    // Setting up task params for task 1
    tivxTaskSetDefaultCreateParams(&taskParams1);
    taskParams1.task_main = &tivxTask1;
    taskParams1.app_var = graph1;
    taskParams1.stack_ptr = NULL;
    taskParams1.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    taskParams1.core_affinity = TIVX_TASK_AFFINITY_ANY;
    taskParams1.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY1;

    // Setting up task params for task 2
    tivxTaskSetDefaultCreateParams(&taskParams2);
    taskParams2.task_main = &tivxTask2;
    taskParams2.app_var = graph2;
    taskParams2.stack_ptr = NULL;
    taskParams2.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    taskParams2.core_affinity = TIVX_TASK_AFFINITY_ANY;
    taskParams2.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY2;

    // Setting up task params for task 3
    tivxTaskSetDefaultCreateParams(&taskParams3);
    taskParams3.task_main = &tivxTask3;
    taskParams3.app_var = graph3;
    taskParams3.stack_ptr = NULL;
    taskParams3.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    taskParams3.core_affinity = TIVX_TASK_AFFINITY_ANY;
    taskParams3.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY3;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle1, &taskParams1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle2, &taskParams2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle3, &taskParams3));

    wait_loop_cnt = 0;
    do
    {
        tivxTaskWaitMsecs(1000);

        if (taskFinished1 != 0 && taskFinished2 != 0 && taskFinished3 != 0)
        {
            break;
        }
        wait_loop_cnt ++;

        /* Waiting for twenty seconds */
        if (wait_loop_cnt > 20)
        {
            break;
        }
    } while (1);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));
    vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));
    vxQueryGraph(graph3, VX_GRAPH_PERFORMANCE, &perf_graph3, sizeof(perf_graph3));

    ASSERT_NO_FAILURE(accum_dst1 = ct_image_from_vx_image(accum_image_final1));

    ASSERT_NO_FAILURE(accum_dst2 = ct_image_from_vx_image(accum_image_final2));

    ASSERT_NO_FAILURE(accum_dst3 = ct_image_from_vx_image(accum_image_final3));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input1, arg_->alpha_intermediate, accum_final1, accum_dst1));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input2, arg_->alpha_final, accum_final2, accum_dst2));

    ASSERT_NO_FAILURE(accumulate_weighted_check(input3, arg_->alpha_final, accum_final3, accum_dst3));

    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph3));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseGraph(&graph1));

    ASSERT(node3 == 0);
    ASSERT(node2 == 0);
    ASSERT(node1 == 0);
    ASSERT(graph3 == 0);
    ASSERT(graph2 == 0);
    ASSERT(graph1 == 0);

    VX_CALL(vxReleaseImage(&accum_image_final3));
    VX_CALL(vxReleaseImage(&accum_image_final2));
    VX_CALL(vxReleaseImage(&accum_image_final1));
    VX_CALL(vxReleaseImage(&input_image3));
    VX_CALL(vxReleaseImage(&input_image2));
    VX_CALL(vxReleaseImage(&input_image1));
    VX_CALL(vxReleaseScalar(&alpha_scalar3));
    VX_CALL(vxReleaseScalar(&alpha_scalar2));
    VX_CALL(vxReleaseScalar(&alpha_scalar1));

    ASSERT(accum_image_final1 == 0);
    ASSERT(accum_image_final2 == 0);
    ASSERT(accum_image_final3 == 0);
    ASSERT(input_image1 == 0);
    ASSERT(input_image2 == 0);
    ASSERT(input_image3 == 0);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle2));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle3));

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3, arg_->width*arg_->height, "N3");
    printPerformance(perf_graph1, arg_->width*arg_->height, "G1");
    printPerformance(perf_graph2, arg_->width*arg_->height, "G2");
    printPerformance(perf_graph3, arg_->width*arg_->height, "G3");
}

// Testing alternating nodes
// vxNot -> vxAccumulateSquare -> vxNot -> vxAccumulateSquare -> vxNot -> vxAccumulateSquare
TEST_WITH_ARG(tivxGraphMultiThreaded, testAlternatingNodes, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    // Graph1
    vx_image input_image_not = 0, accum_image_final = 0;
    vx_image input_image_acc_1 = 0, input_image_acc_2 = 0;
    vx_image accum_image_virtual_1 = 0, accum_image_virtual_2 = 0, accum_image_virtual_3 = 0;
    vx_scalar alpha_scalar_1, alpha_scalar_2, alpha_scalar_3 = 0;
    vx_float32 sh = 0.5f;
    // Graph2
    vx_image input_image_not_graph2 = 0, accum_image_final_graph2 = 0;
    vx_image input_image_acc_1_graph2 = 0, input_image_acc_2_graph2 = 0;
    vx_image accum_image_virtual_1_graph2 = 0, accum_image_virtual_2_graph2 = 0, accum_image_virtual_3_graph2 = 0;
    vx_scalar alpha_scalar_1_graph2, alpha_scalar_2_graph2, alpha_scalar_3_graph2 = 0;
    vx_float32 sh_graph2 = 0.25f;
    vx_graph graph1 = 0, graph2 = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0, node6 = 0;
    vx_node node7 = 0, node8 = 0, node9 = 0, node10 = 0, node11 = 0, node12 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node3, perf_node4, perf_node5, perf_node6, perf_graph1;
    vx_perf_t perf_node7, perf_node8, perf_node9, perf_node10, perf_node11, perf_node12, perf_graph2;

    // Graph1
    CT_Image input_acc_1 = NULL, input_acc_2 = NULL, input_not = NULL, accum_final = NULL, accum_dst = NULL;
    CT_Image virtual_dummy_1 = NULL, virtual_dummy_2 = NULL, virtual_dummy_3 = NULL;

    // Graph2
    CT_Image input_acc_1_graph2 = NULL, input_acc_2_graph2 = NULL, input_not_graph2 = NULL, accum_final_graph2 = NULL, accum_dst_graph2 = NULL;
    CT_Image virtual_dummy_1_graph2 = NULL, virtual_dummy_2_graph2 = NULL, virtual_dummy_3_graph2 = NULL;

    tivx_task_create_params_t taskParams1, taskParams2, taskParams3;
    tivx_task taskHandle1, taskHandle2, taskHandle3;
    uint32_t wait_loop_cnt = 0;

    taskFinished1 = 0;
    taskFinished2 = 0;

    virtual_dummy_1 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    virtual_dummy_2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    virtual_dummy_3 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    virtual_dummy_1_graph2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    virtual_dummy_2_graph2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    virtual_dummy_3_graph2 = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(alpha_scalar_1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar_2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &sh), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar_3 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input_acc_1 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input_acc_2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input_not = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image_acc_1 = ct_image_to_vx_image(input_acc_1, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image_acc_2 = ct_image_to_vx_image(input_acc_2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image_not = ct_image_to_vx_image(input_not, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final = ct_image_to_vx_image(accum_final, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(alpha_scalar_1_graph2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_intermediate), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar_2_graph2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &sh_graph2), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(alpha_scalar_3_graph2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &arg_->alpha_final), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(input_acc_1_graph2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input_acc_2_graph2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(input_not_graph2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_NO_FAILURE(accum_final_graph2 = accumulate_weighted_generate_random_8u(arg_->width, arg_->height));

    ASSERT_VX_OBJECT(input_image_acc_1_graph2 = ct_image_to_vx_image(input_acc_1_graph2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image_acc_2_graph2 = ct_image_to_vx_image(input_acc_2_graph2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_image_not_graph2 = ct_image_to_vx_image(input_not_graph2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_final_graph2 = ct_image_to_vx_image(accum_final_graph2, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    // Must be after vxCreateGraph
    ASSERT_VX_OBJECT(accum_image_virtual_1 = vxCreateVirtualImage(graph1, 0, 0, VX_DF_IMAGE_VIRT), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_virtual_2 = vxCreateVirtualImage(graph1, 0, 0, VX_DF_IMAGE_VIRT), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_virtual_3 = vxCreateVirtualImage(graph1, 0, 0, VX_DF_IMAGE_VIRT), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_virtual_1_graph2 = vxCreateVirtualImage(graph2, 0, 0, VX_DF_IMAGE_VIRT), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_virtual_2_graph2 = vxCreateVirtualImage(graph2, 0, 0, VX_DF_IMAGE_VIRT), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(accum_image_virtual_3_graph2 = vxCreateVirtualImage(graph2, 0, 0, VX_DF_IMAGE_VIRT), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxNotNode(graph1, input_image_not, accum_image_virtual_1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxAccumulateWeightedImageNode(graph1, accum_image_virtual_1, alpha_scalar_1, input_image_acc_1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxNotNode(graph1, input_image_acc_1, accum_image_virtual_2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node4 = vxAccumulateWeightedImageNode(graph1, accum_image_virtual_2, alpha_scalar_2, input_image_acc_2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node5 = vxNotNode(graph1, input_image_acc_2, accum_image_virtual_3), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node6 = vxAccumulateWeightedImageNode(graph1, accum_image_virtual_3, alpha_scalar_3, accum_image_final), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node7 = vxNotNode(graph2, input_image_not_graph2, accum_image_virtual_1_graph2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node8 = vxAccumulateWeightedImageNode(graph2, accum_image_virtual_1_graph2, alpha_scalar_1_graph2, input_image_acc_1_graph2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node9 = vxNotNode(graph2, input_image_acc_1_graph2, accum_image_virtual_2_graph2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node10 = vxAccumulateWeightedImageNode(graph2, accum_image_virtual_2_graph2, alpha_scalar_2_graph2, input_image_acc_2_graph2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node11 = vxNotNode(graph2, input_image_acc_2_graph2, accum_image_virtual_3_graph2), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node12 = vxAccumulateWeightedImageNode(graph2, accum_image_virtual_3_graph2, alpha_scalar_3_graph2, accum_image_final_graph2), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node1, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node2, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node3, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node4, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node5, VX_TARGET_STRING, "DSP-1"));

    VX_CALL(vxSetNodeTarget(node6, VX_TARGET_STRING, "DSP-1"));

    if (vx_true_e == tivxIsTargetEnabled("DSP-2"))
    {
        VX_CALL(vxSetNodeTarget(node7, VX_TARGET_STRING, "DSP-2"));
        VX_CALL(vxSetNodeTarget(node8, VX_TARGET_STRING, "DSP-2"));
        VX_CALL(vxSetNodeTarget(node9, VX_TARGET_STRING, "DSP-2"));
        VX_CALL(vxSetNodeTarget(node10, VX_TARGET_STRING, "DSP-2"));
        VX_CALL(vxSetNodeTarget(node11, VX_TARGET_STRING, "DSP-2"));
        VX_CALL(vxSetNodeTarget(node12, VX_TARGET_STRING, "DSP-2"));
    }
    else
    {
        VX_CALL(vxSetNodeTarget(node7, VX_TARGET_STRING, "DSP-1"));
        VX_CALL(vxSetNodeTarget(node8, VX_TARGET_STRING, "DSP-1"));
        VX_CALL(vxSetNodeTarget(node9, VX_TARGET_STRING, "DSP-1"));
        VX_CALL(vxSetNodeTarget(node10, VX_TARGET_STRING, "DSP-1"));
        VX_CALL(vxSetNodeTarget(node11, VX_TARGET_STRING, "DSP-1"));
        VX_CALL(vxSetNodeTarget(node12, VX_TARGET_STRING, "DSP-1"));
    }

    // Setting up task params for task 1
    tivxTaskSetDefaultCreateParams(&taskParams1);
    taskParams1.task_main = &tivxTask1;
    taskParams1.app_var = graph1;
    taskParams1.stack_ptr = NULL;
    taskParams1.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    taskParams1.core_affinity = TIVX_TASK_AFFINITY_ANY;
    taskParams1.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY1;

    // Setting up task params for task 2
    tivxTaskSetDefaultCreateParams(&taskParams2);
    taskParams2.task_main = &tivxTask2;
    taskParams2.app_var = graph2;
    taskParams2.stack_ptr = NULL;
    taskParams2.stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    taskParams2.core_affinity = TIVX_TASK_AFFINITY_ANY;
    taskParams2.priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY2;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle1, &taskParams1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskCreate(&taskHandle2, &taskParams2));

    wait_loop_cnt = 0;
    do
    {
        tivxTaskWaitMsecs(1000);

        if (taskFinished1 != 0 && taskFinished2 != 0)
        {
            break;
        }
        wait_loop_cnt ++;

        /* Waiting for twenty seconds */
        if (wait_loop_cnt > 20)
        {
            break;
        }
    } while (1);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryNode(node3, VX_NODE_PERFORMANCE, &perf_node3, sizeof(perf_node3));
    vxQueryNode(node4, VX_NODE_PERFORMANCE, &perf_node4, sizeof(perf_node4));
    vxQueryNode(node5, VX_NODE_PERFORMANCE, &perf_node5, sizeof(perf_node5));
    vxQueryNode(node6, VX_NODE_PERFORMANCE, &perf_node6, sizeof(perf_node6));
    vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));

    vxQueryNode(node7, VX_NODE_PERFORMANCE, &perf_node7, sizeof(perf_node7));
    vxQueryNode(node8, VX_NODE_PERFORMANCE, &perf_node8, sizeof(perf_node8));
    vxQueryNode(node9, VX_NODE_PERFORMANCE, &perf_node9, sizeof(perf_node9));
    vxQueryNode(node10, VX_NODE_PERFORMANCE, &perf_node10, sizeof(perf_node10));
    vxQueryNode(node11, VX_NODE_PERFORMANCE, &perf_node11, sizeof(perf_node11));
    vxQueryNode(node12, VX_NODE_PERFORMANCE, &perf_node12, sizeof(perf_node12));
    vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));

    ASSERT_NO_FAILURE(accum_dst = ct_image_from_vx_image(accum_image_final));

    ASSERT_NO_FAILURE(accum_dst_graph2 = ct_image_from_vx_image(accum_image_final_graph2));

    ASSERT_NO_FAILURE(alternate_node_check(input_not, input_acc_1, input_acc_2, virtual_dummy_1, virtual_dummy_2,
                        virtual_dummy_3, arg_->alpha_intermediate, sh, arg_->alpha_final, accum_final, accum_dst));

    ASSERT_NO_FAILURE(alternate_node_check(input_not_graph2, input_acc_1_graph2, input_acc_2_graph2, virtual_dummy_1_graph2, virtual_dummy_2_graph2,
                        virtual_dummy_3_graph2, arg_->alpha_intermediate, sh_graph2, arg_->alpha_final, accum_final_graph2, accum_dst_graph2));

    VX_CALL(vxReleaseNode(&node12));
    VX_CALL(vxReleaseNode(&node11));
    VX_CALL(vxReleaseNode(&node10));
    VX_CALL(vxReleaseNode(&node9));
    VX_CALL(vxReleaseNode(&node8));
    VX_CALL(vxReleaseNode(&node7));
    VX_CALL(vxReleaseGraph(&graph2));

    VX_CALL(vxReleaseNode(&node6));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph1));

    ASSERT(node12 == 0);
    ASSERT(node11 == 0);
    ASSERT(node10 == 0);
    ASSERT(node9 == 0);
    ASSERT(node8 == 0);
    ASSERT(node7 == 0);
    ASSERT(graph2 == 0);

    ASSERT(node6 == 0);
    ASSERT(node5 == 0);
    ASSERT(node4 == 0);
    ASSERT(node3 == 0);
    ASSERT(node2 == 0);
    ASSERT(node1 == 0);
    ASSERT(graph1 == 0);

    VX_CALL(vxReleaseImage(&accum_image_final_graph2));
    VX_CALL(vxReleaseImage(&accum_image_virtual_3_graph2));
    VX_CALL(vxReleaseImage(&accum_image_virtual_2_graph2));
    VX_CALL(vxReleaseImage(&accum_image_virtual_1_graph2));
    VX_CALL(vxReleaseImage(&input_image_acc_2_graph2));
    VX_CALL(vxReleaseImage(&input_image_acc_1_graph2));
    VX_CALL(vxReleaseImage(&input_image_not_graph2));
    VX_CALL(vxReleaseScalar(&alpha_scalar_3_graph2));
    VX_CALL(vxReleaseScalar(&alpha_scalar_2_graph2));
    VX_CALL(vxReleaseScalar(&alpha_scalar_1_graph2));

    VX_CALL(vxReleaseImage(&accum_image_final));
    VX_CALL(vxReleaseImage(&accum_image_virtual_3));
    VX_CALL(vxReleaseImage(&accum_image_virtual_2));
    VX_CALL(vxReleaseImage(&accum_image_virtual_1));
    VX_CALL(vxReleaseImage(&input_image_acc_2));
    VX_CALL(vxReleaseImage(&input_image_acc_1));
    VX_CALL(vxReleaseImage(&input_image_not));
    VX_CALL(vxReleaseScalar(&alpha_scalar_3));
    VX_CALL(vxReleaseScalar(&alpha_scalar_2));
    VX_CALL(vxReleaseScalar(&alpha_scalar_1));

    ASSERT(accum_image_final_graph2 == 0);
    ASSERT(input_image_acc_2_graph2 == 0);
    ASSERT(input_image_acc_1_graph2 == 0);
    ASSERT(input_image_not_graph2 == 0);

    ASSERT(accum_image_final == 0);
    ASSERT(input_image_acc_2 == 0);
    ASSERT(input_image_acc_1 == 0);
    ASSERT(input_image_not == 0);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle1));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxTaskDelete(&taskHandle2));

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_node3, arg_->width*arg_->height, "N3");
    printPerformance(perf_node4, arg_->width*arg_->height, "N4");
    printPerformance(perf_node5, arg_->width*arg_->height, "N5");
    printPerformance(perf_node6, arg_->width*arg_->height, "N6");
    printPerformance(perf_graph1, arg_->width*arg_->height, "G1");

    printPerformance(perf_node7, arg_->width*arg_->height, "N1");
    printPerformance(perf_node8, arg_->width*arg_->height, "N2");
    printPerformance(perf_node9, arg_->width*arg_->height, "N3");
    printPerformance(perf_node10, arg_->width*arg_->height, "N4");
    printPerformance(perf_node11, arg_->width*arg_->height, "N5");
    printPerformance(perf_node12, arg_->width*arg_->height, "N6");
    printPerformance(perf_graph2, arg_->width*arg_->height, "G2");
}

TESTCASE_TESTS(tivxGraphMultiThreaded,
        testParallelGraphsSameTarget,
        testParallelGraphsDifferentTarget,
        testParallelGraphsMultipleNodes,
        testThreeParallelGraphs,
        testAlternatingNodes
)
