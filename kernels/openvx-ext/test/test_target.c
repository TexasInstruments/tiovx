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
#include <TI/tivx_capture.h>
#include <TI/tivx_task.h>
#include <TI/tivx_target_kernel.h>
#include "math.h"
#include <limits.h>

TESTCASE(tivxTargetFinal,  CT_VXContext, ct_setup_vx_context, 0)

typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    char *target_string;

} SetTarget_Arg;

#if defined(SOC_AM62A)
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MCU1_0", __VA_ARGS__, TIVX_TARGET_MCU1_0)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP1", __VA_ARGS__, TIVX_TARGET_DSP1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MPU_0", __VA_ARGS__, TIVX_TARGET_MPU_0))
#else
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MCU2_0", __VA_ARGS__, TIVX_TARGET_MCU2_0)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_DSP1", __VA_ARGS__, TIVX_TARGET_DSP1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_MPU_0", __VA_ARGS__, TIVX_TARGET_MPU_0))
#endif

#define SET_NODE_TARGET_PARAMETERS \
    CT_GENERATE_PARAMETERS("target", ADD_SET_TARGET_PARAMETERS, ARG, NULL)

#define NODE_ERROR_EVENT     (1u)

TEST_WITH_ARG(tivxTargetFinal, testTargetScalar, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_in, scalar_out;
    vx_node n0;
    vx_event_t event;

    tivxTestKernelsLoadKernels(context);

    VX_CALL(vxEnableEvents(context));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_in  = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(scalar_out  = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n0 = tivxTestTargetNode(graph, scalar_in, scalar_out), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n0, "test_target_node");

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)n0, VX_EVENT_NODE_ERROR, 0, NODE_ERROR_EVENT));

    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, arg_->target_string));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    tivxTaskWaitMsecs(10);

    /* Asserting that no errors were produced when running the application */
    ASSERT_EQ_VX_STATUS(VX_FAILURE, vxWaitEvent(context, &event, vx_true_e));

    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseScalar(&scalar_in));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

TESTCASE_TESTS(tivxTargetFinal,
               testTargetScalar)

