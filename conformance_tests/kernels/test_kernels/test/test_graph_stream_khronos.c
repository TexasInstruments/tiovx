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
#include "test_tiovx/test_tiovx.h"

#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_khr_pipelining.h>
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_config.h>
#include <TI/tivx_task.h>
#include <TI/tivx_target_kernel.h>
#include "math.h"
#include <limits.h>

TESTCASE(GraphStreaming,  CT_VXContext, ct_setup_vx_context, 0)

#define VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE1 (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 3)
#define VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE1_NAME "org.khronos.openvx.test.user_source_1"

#define VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE2 (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 4)
#define VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE2_NAME "org.khronos.openvx.test.user_source_2"

#define VX_KERNEL_CONFORMANCE_TEST_USER_SINK (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 5)
#define VX_KERNEL_CONFORMANCE_TEST_USER_SINK_NAME "org.khronos.openvx.test.user_sink"
#define PIPEUP_NUM_BUFS 3

typedef enum _own_source_params_e
{
    OWN_SOURCE_PARAM_OUTPUT = 0
} own_source_params_e;

typedef enum _own_sink_params_e
{
    OWN_SINK_PARAM_INPUT = 0
} own_sink_params_e;

static enum vx_type_e type = (enum vx_type_e)VX_TYPE_SCALAR;

static vx_bool is_pipeup_entered = vx_false_e;
static vx_bool is_steady_state_entered = vx_false_e;
static uint8_t pipeup_frame = 0;
static uint8_t global_value = 0;
static uint8_t copy_value[PIPEUP_NUM_BUFS-1];
static uint8_t golden_sink_value = 0;

static vx_status VX_CALLBACK own_source1_Kernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    uint32_t state;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);
    uint8_t i;

    vxQueryNode(node, VX_NODE_STATE, &state, sizeof(state));
    EXPECT(parameters != NULL);
    EXPECT(num == 1);
    if (parameters != NULL && num == 1)
    {
        EXPECT_VX_OBJECT(parameters[0], type);

        if (255 == global_value)
        {
            global_value = 0;
        }
        else
        {
            global_value++;
        }

        if (state == VX_NODE_STATE_STEADY)
        {
            is_steady_state_entered = vx_true_e;

            vxCopyScalar((vx_scalar)parameters[0], &copy_value[0], VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

            for (i = 0; i < pipeup_frame-1; i++)
            {
                copy_value[i] = copy_value[i+1];
            }

            copy_value[pipeup_frame-1] = global_value;

            /* To give some cycles to test application to end test */
            tivxTaskWaitMsecs(1);
        }
        else
        {
            is_pipeup_entered = vx_true_e;
            copy_value[pipeup_frame] = global_value;
            pipeup_frame++;
        }
    }

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK own_source2_Kernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    uint32_t state;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);

    vxQueryNode(node, VX_NODE_STATE, &state, sizeof(state));
    EXPECT(parameters != NULL);
    EXPECT(num == 1);
    if (parameters != NULL && num == 1)
    {
        EXPECT_VX_OBJECT(parameters[0], type);

        if (255 == global_value)
        {
            global_value = 0;
        }
        else
        {
            global_value++;
        }

        vxCopyScalar((vx_scalar)parameters[0], &global_value, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    }

    /* To give some cycles to test application to end test */
    tivxTaskWaitMsecs(1);

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK own_sink_Kernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    uint8_t local_copy_value = 0;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);

    EXPECT(parameters != NULL);
    EXPECT(num == 1);
    if (parameters != NULL && num == 1)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        vxCopyScalar((vx_scalar)parameters[0], &local_copy_value, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

        if (255 == golden_sink_value)
        {
            golden_sink_value = 0;
        }
        else
        {
            golden_sink_value++;
        }

        EXPECT(local_copy_value == golden_sink_value);
    }

    /* To give some cycles to test application to end test */
    tivxTaskWaitMsecs(1);

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK own_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);
    EXPECT(parameters != NULL);
    EXPECT(num == 1);
    if (parameters != NULL && num == 1)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
    }
    return VX_SUCCESS;
}

static vx_status VX_CALLBACK own_Deinitialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    EXPECT(node != 0);
    EXPECT(parameters != NULL);
    EXPECT(num == 1);
    if (parameters != NULL && num == 1)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
    }

    return VX_SUCCESS;
}

/* Source with buffering */
static void own_register_source1_kernel(vx_context context)
{
    vx_kernel kernel = 0;
    vx_uint32 num_bufs = PIPEUP_NUM_BUFS;

    ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
        context,
        VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE1_NAME,
        VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE1,
        own_source1_Kernel,
        1,
        NULL,
        own_Initialize,
        own_Deinitialize), VX_TYPE_KERNEL);

    vxSetKernelAttribute(kernel, VX_KERNEL_PIPEUP_OUTPUT_DEPTH, &num_bufs, sizeof(num_bufs));

    VX_CALL(vxAddParameterToKernel(kernel, OWN_SOURCE_PARAM_OUTPUT, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_SOURCE_PARAM_OUTPUT), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxFinalizeKernel(kernel));
    VX_CALL(vxReleaseKernel(&kernel));
}

/* Source without buffering */
static void own_register_source2_kernel(vx_context context)
{
    vx_kernel kernel = 0;

    ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
        context,
        VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE2_NAME,
        VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE2,
        own_source2_Kernel,
        1,
        NULL,
        own_Initialize,
        own_Deinitialize), VX_TYPE_KERNEL);

    VX_CALL(vxAddParameterToKernel(kernel, OWN_SOURCE_PARAM_OUTPUT, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_SOURCE_PARAM_OUTPUT), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxFinalizeKernel(kernel));
    VX_CALL(vxReleaseKernel(&kernel));
}

static void own_register_sink_kernel(vx_context context)
{
    vx_kernel kernel = 0;

    ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
        context,
        VX_KERNEL_CONFORMANCE_TEST_USER_SINK_NAME,
        VX_KERNEL_CONFORMANCE_TEST_USER_SINK,
        own_sink_Kernel,
        1,
        NULL,
        own_Initialize,
        own_Deinitialize), VX_TYPE_KERNEL);

    VX_CALL(vxAddParameterToKernel(kernel, OWN_SINK_PARAM_INPUT, VX_INPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_SINK_PARAM_INPUT), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_INPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxFinalizeKernel(kernel));
    VX_CALL(vxReleaseKernel(&kernel));
}

typedef struct {
    const char* name;
    int stream_time;
    int source;
} Arg;

#define STREAMING_PARAMETERS \
    CT_GENERATE_PARAMETERS("streaming_with_buffering", ARG, 100, 1), \
    CT_GENERATE_PARAMETERS("streaming_with_buffering", ARG, 1000, 1), \
    CT_GENERATE_PARAMETERS("streaming_with_buffering", ARG, 10000, 1), \
    CT_GENERATE_PARAMETERS("streaming_with_buffering", ARG, 100, 2), \
    CT_GENERATE_PARAMETERS("streaming_with_buffering", ARG, 1000, 2), \
    CT_GENERATE_PARAMETERS("streaming_with_buffering", ARG, 10000, 2), \

TEST_WITH_ARG(GraphStreaming, testSourceUserKernel, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_kernel user_kernel = 0;
    vx_node node = 0;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar;
    int i;

    for (i = 0; i < (PIPEUP_NUM_BUFS-1); i++)
    {
        copy_value[i] = 0;
    }
    pipeup_frame = 0;
    global_value = 0;

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    if (arg_->source == 1)
    {
        ASSERT_NO_FAILURE(own_register_source1_kernel(context));
    }
    else
    {
        ASSERT_NO_FAILURE(own_register_source2_kernel(context));
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    if (arg_->source == 1)
    {
        ASSERT_VX_OBJECT(user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE1_NAME), VX_TYPE_KERNEL);
    }
    else
    {
        ASSERT_VX_OBJECT(user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE2_NAME), VX_TYPE_KERNEL);
    }

    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, user_kernel), VX_TYPE_NODE);

    VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)scalar));

    is_pipeup_entered = vx_false_e;
    is_steady_state_entered = vx_false_e;

    VX_CALL(vxProcessGraph(graph));

    vxCopyScalar(scalar, &scalar_val, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

    ASSERT(scalar_val == 1);

    VX_CALL(vxProcessGraph(graph));

    vxCopyScalar(scalar, &scalar_val, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

    ASSERT(scalar_val == 2);

    VX_CALL(vxProcessGraph(graph));

    vxCopyScalar(scalar, &scalar_val, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

    ASSERT(scalar_val == 3);

    if (arg_->source == 1)
    {
        ASSERT(is_pipeup_entered == vx_true_e);
        ASSERT(is_steady_state_entered == vx_true_e);
    }

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* user kernel should be removed only after all references to it released */
    /* Note, vxRemoveKernel doesn't zeroing kernel ref */
    VX_CALL(vxRemoveKernel(user_kernel));

    VX_CALL(vxReleaseScalar(&scalar));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(scalar == 0);
}

TEST_WITH_ARG(GraphStreaming, testSourceSinkUserKernel, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_kernel source_user_kernel = 0, sink_user_kernel = 0;
    vx_node node1 = 0, node2 = 0;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar;
    int i;

    for (i = 0; i < (PIPEUP_NUM_BUFS-1); i++)
    {
        copy_value[i] = 0;
    }
    pipeup_frame = 0;
    global_value = 0;
    golden_sink_value = 0;

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    if (arg_->source == 1)
    {
        ASSERT_NO_FAILURE(own_register_source1_kernel(context));
    }
    else
    {
        ASSERT_NO_FAILURE(own_register_source2_kernel(context));
    }

    ASSERT_NO_FAILURE(own_register_sink_kernel(context));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    if (arg_->source == 1)
    {
        ASSERT_VX_OBJECT(source_user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE1_NAME), VX_TYPE_KERNEL);
    }
    else
    {
        ASSERT_VX_OBJECT(source_user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE2_NAME), VX_TYPE_KERNEL);
    }

    ASSERT_VX_OBJECT(sink_user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_USER_SINK_NAME), VX_TYPE_KERNEL);

    ASSERT_VX_OBJECT(node1 = vxCreateGenericNode(graph, source_user_kernel), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxCreateGenericNode(graph, sink_user_kernel), VX_TYPE_NODE);

    VX_CALL(vxSetParameterByIndex(node1, 0, (vx_reference)scalar));

    VX_CALL(vxSetParameterByIndex(node2, 0, (vx_reference)scalar));

    is_pipeup_entered = vx_false_e;
    is_steady_state_entered = vx_false_e;

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxProcessGraph(graph));

    if (arg_->source == 1)
    {
        ASSERT(is_pipeup_entered == vx_true_e);
        ASSERT(is_steady_state_entered == vx_true_e);
    }

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    /* user kernel should be removed only after all references to it released */
    /* Note, vxRemoveKernel doesn't zeroing kernel ref */
    VX_CALL(vxRemoveKernel(source_user_kernel));
    VX_CALL(vxRemoveKernel(sink_user_kernel));

    VX_CALL(vxReleaseScalar(&scalar));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);
    ASSERT(scalar == 0);
}

TEST_WITH_ARG(GraphStreaming, testSourceUserKernelStreaming, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_kernel user_kernel = 0;
    vx_node node = 0;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar;
    int i;

    for (i = 0; i < (PIPEUP_NUM_BUFS-1); i++)
    {
        copy_value[i] = 0;
    }

    pipeup_frame = 0;
    global_value = 0;

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    if (arg_->source == 1)
    {
        ASSERT_NO_FAILURE(own_register_source1_kernel(context));
    }
    else
    {
        ASSERT_NO_FAILURE(own_register_source2_kernel(context));
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    if (arg_->source == 1)
    {
        ASSERT_VX_OBJECT(user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE1_NAME), VX_TYPE_KERNEL);
    }
    else
    {
        ASSERT_VX_OBJECT(user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE2_NAME), VX_TYPE_KERNEL);
    }

    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, user_kernel), VX_TYPE_NODE);

    VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)scalar));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxEnableGraphStreaming(graph, node));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    is_pipeup_entered = vx_false_e;
    is_steady_state_entered = vx_false_e;

    VX_CALL(vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph));

    if (arg_->source == 1)
    {
        ASSERT(is_pipeup_entered == vx_true_e);
        ASSERT(is_steady_state_entered == vx_true_e);
    }

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* user kernel should be removed only after all references to it released */
    /* Note, vxRemoveKernel doesn't zeroing kernel ref */
    VX_CALL(vxRemoveKernel(user_kernel));

    VX_CALL(vxReleaseScalar(&scalar));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(scalar == 0);
}

TEST_WITH_ARG(GraphStreaming, testSourceSinkUserKernelStreaming, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_kernel source_user_kernel = 0, sink_user_kernel = 0;
    vx_node node1 = 0, node2 = 0;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar;
    int i;

    for (i = 0; i < (PIPEUP_NUM_BUFS-1); i++)
    {
        copy_value[i] = 0;
    }

    pipeup_frame = 0;
    global_value = 0;
    golden_sink_value = 0;

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    if (arg_->source == 1)
    {
        ASSERT_NO_FAILURE(own_register_source1_kernel(context));
    }
    else
    {
        ASSERT_NO_FAILURE(own_register_source2_kernel(context));
    }

    ASSERT_NO_FAILURE(own_register_sink_kernel(context));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    if (arg_->source == 1)
    {
        ASSERT_VX_OBJECT(source_user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE1_NAME), VX_TYPE_KERNEL);
    }
    else
    {
        ASSERT_VX_OBJECT(source_user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE2_NAME), VX_TYPE_KERNEL);
    }

    ASSERT_VX_OBJECT(sink_user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_USER_SINK_NAME), VX_TYPE_KERNEL);

    ASSERT_VX_OBJECT(node1 = vxCreateGenericNode(graph, source_user_kernel), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxCreateGenericNode(graph, sink_user_kernel), VX_TYPE_NODE);

    VX_CALL(vxSetParameterByIndex(node1, 0, (vx_reference)scalar));

    VX_CALL(vxSetParameterByIndex(node2, 0, (vx_reference)scalar));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxEnableGraphStreaming(graph, node1));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    is_pipeup_entered = vx_false_e;
    is_steady_state_entered = vx_false_e;

    VX_CALL(vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(arg_->stream_time);

    VX_CALL(vxStopGraphStreaming(graph));

    if (arg_->source == 1)
    {
        ASSERT(is_pipeup_entered == vx_true_e);
        ASSERT(is_steady_state_entered == vx_true_e);
    }

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    VX_CALL(vxRemoveKernel(source_user_kernel));
    VX_CALL(vxRemoveKernel(sink_user_kernel));

    VX_CALL(vxReleaseScalar(&scalar));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);
    ASSERT(scalar == 0);
}

TESTCASE_TESTS(GraphStreaming,
               testSourceUserKernel,
               testSourceSinkUserKernel,
               testSourceUserKernelStreaming,
               testSourceSinkUserKernelStreaming)

