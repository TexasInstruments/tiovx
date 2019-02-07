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

/*
 * Utility API to set trigger node for a graph
 */
static vx_status set_graph_trigger_node(vx_graph graph, vx_node node)
{
    return tivxEnableGraphStreaming(graph, node);
}

#define VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 3)
#define VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE_NAME "org.khronos.openvx.test.user_source"

#define VX_KERNEL_CONFORMANCE_TEST_USER_SINK (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 4)
#define VX_KERNEL_CONFORMANCE_TEST_USER_SINK_NAME "org.khronos.openvx.test.user_sink"
#define PIPEUP_NUM_BUFS 2

typedef enum _own_params_e
{
    OWN_PARAM_OUTPUT = 0
} own_params_e;

static enum vx_type_e type = (enum vx_type_e)VX_TYPE_SCALAR;
static enum vx_type_e objarray_itemtype = VX_TYPE_INVALID;

static vx_bool is_pipeup_entered = vx_false_e;
static vx_bool is_steady_state_entered = vx_false_e;
static vx_reference old_parameters[PIPEUP_NUM_BUFS];
static uint32_t pipeup_frame = 0;
static uint32_t local_value = 0;
static uint32_t copy_value = 0;
static vx_status VX_CALLBACK own_source_Kernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    uint32_t state;
    vx_scalar local_param;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);

    vxQueryNode(node, VX_NODE_STATE, &state, sizeof(state));
    EXPECT(parameters != NULL);
    EXPECT(num == 1);
    if (parameters != NULL && num == 1)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        local_param = (vx_scalar)parameters[0];
        // copy local_value to local_param
        vxCopyScalar(local_param, &local_value, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
        local_value++;

        if (state == VX_NODE_STATE_STEADY)
        {
            vxCopyScalar((vx_scalar)old_parameters[0], &copy_value, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            vxCopyScalar((vx_scalar)parameters[0], &copy_value, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            old_parameters[0] = (vx_reference)local_param;
            is_steady_state_entered = vx_true_e;
        }
        else
        {
            is_pipeup_entered = vx_true_e;
            old_parameters[pipeup_frame] = (vx_reference)local_param;
            pipeup_frame++;
        }
    }

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK own_sink_Kernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    uint32_t state;
    vx_scalar local_param;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);

    vxQueryNode(node, VX_NODE_STATE, &state, sizeof(state));
    EXPECT(parameters != NULL);
    EXPECT(num == 1);
    if (parameters != NULL && num == 1)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        local_param = (vx_scalar)parameters[0];
        // copy local_value to local_param
        vxCopyScalar(local_param, &local_value, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
        local_value++;

        if (state == VX_NODE_STATE_STEADY)
        {
            vxCopyScalar((vx_scalar)old_parameters[0], &copy_value, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            vxCopyScalar((vx_scalar)parameters[0], &copy_value, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            old_parameters[0] = (vx_reference)local_param;
            is_steady_state_entered = vx_true_e;
        }
        else
        {
            is_pipeup_entered = vx_true_e;
            old_parameters[pipeup_frame] = (vx_reference)local_param;
            pipeup_frame++;
        }
    }

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK own_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    void* ptr = NULL;
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
    void* ptr = NULL;
    EXPECT(node != 0);
    EXPECT(parameters != NULL);
    EXPECT(num == 1);
    if (parameters != NULL && num == 1)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
    }

    return VX_SUCCESS;
}

static void own_register_source_kernel(vx_context context)
{
    vx_kernel kernel = 0;

    ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
        context,
        VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE_NAME,
        VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE,
        own_source_Kernel,
        1,
        NULL,
        own_Initialize,
        own_Deinitialize), VX_TYPE_KERNEL);

    vx_uint32 num_bufs = PIPEUP_NUM_BUFS;

    vxSetKernelAttribute(kernel, VX_KERNEL_PIPEUP_DEPTH, &num_bufs, sizeof(num_bufs));

    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_OUTPUT, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_OUTPUT), VX_TYPE_PARAMETER);
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

    vx_uint32 num_bufs = PIPEUP_NUM_BUFS;

    vxSetKernelAttribute(kernel, VX_KERNEL_PIPEUP_DEPTH, &num_bufs, sizeof(num_bufs));

    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_OUTPUT, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_OUTPUT), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxFinalizeKernel(kernel));
    VX_CALL(vxReleaseKernel(&kernel));
}

typedef struct {
    const char* name;
    int stream_time;
} Arg;

#define STREAMING_PARAMETERS \
    CT_GENERATE_PARAMETERS("streaming", ARG, 100), \
    CT_GENERATE_PARAMETERS("streaming", ARG, 1000), \
    CT_GENERATE_PARAMETERS("streaming", ARG, 10000), \

TEST(GraphStreaming, testSourceUserKernel)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_kernel user_kernel = 0;
    vx_node node = 0;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar;

    copy_value = 0;
    pipeup_frame = 0;
    local_value = 0;

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(own_register_source_kernel(context));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE_NAME), VX_TYPE_KERNEL);

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

    ASSERT(is_pipeup_entered == vx_true_e);
    ASSERT(is_steady_state_entered == vx_true_e);

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

TEST_WITH_ARG(GraphStreaming, testSourceUserKernelStreaming, Arg, STREAMING_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_kernel user_kernel = 0;
    vx_node node = 0;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar;

    copy_value = 0;
    pipeup_frame = 0;
    local_value = 0;

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_NO_FAILURE(own_register_source_kernel(context));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_USER_SOURCE_NAME), VX_TYPE_KERNEL);

    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, user_kernel), VX_TYPE_NODE);

    VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)scalar));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, set_graph_trigger_node(graph, node));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    is_pipeup_entered = vx_false_e;
    is_steady_state_entered = vx_false_e;

    VX_CALL(vxStartGraphStreaming(graph));

    tivxTaskWaitMsecs(500);

    VX_CALL(vxStopGraphStreaming(graph));

    ASSERT(is_pipeup_entered == vx_true_e);
    ASSERT(is_steady_state_entered == vx_true_e);

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

TESTCASE_TESTS(GraphStreaming,
               testSourceUserKernel,
               testSourceUserKernelStreaming)

