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
#include <VX/vx_khr_supplementary_data.h>
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_config.h>
#include <TI/tivx_capture.h>
#include <TI/tivx_task.h>
#include <TI/tivx_target_kernel.h>
#include "math.h"
#include <limits.h>

TESTCASE(tivxTargetFinal,  CT_VXContext, ct_setup_vx_context, 0)

#if defined(C66_COVERAGE)
#define NODE_TARGET TIVX_TARGET_DSP1
#elif defined(C7X_COVERAGE)
#if defined(SOC_J721E)
#define NODE_TARGET TIVX_TARGET_DSP_C7_1
#else
#define NODE_TARGET TIVX_TARGET_DSP1
#endif /* #if defined(SOC_J721E) */
#elif defined(R5F_COVERAGE)
#if defined(SOC_AM62A)
#define NODE_TARGET TIVX_TARGET_MCU1_0
#else
#define NODE_TARGET TIVX_TARGET_MCU2_0
#endif /* #if defined(SOC_AM62A) */
#else
#define NODE_TARGET TIVX_TARGET_MPU_0
#endif /* #if defined(CORE_COVERAGE) */

#define NODE_ERROR_EVENT     (1u)

typedef struct _user_data
{
    vx_uint32 numbers[4];
} user_data_t;

TEST(tivxTargetFinal, testTargetScalar)
{
    vx_graph graph;
    vx_context context = context_->vx_context_;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar_in, scalar_out;
    vx_node n0;
    vx_event_t event;
    vx_user_data_object exemplar, exemplar2;
    vx_reference test_object, supp_data, supp_data_copy;
    vx_image img;
    user_data_t user_data = {.numbers = {1, 2, 3, 4}};

    tivxTestKernelsLoadKernels(context);
    VX_CALL(vxEnableEvents(context));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(scalar_in  = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(scalar_out  = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(exemplar = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data), VX_TYPE_USER_DATA_OBJECT);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetSupplementaryUserDataObject((vx_reference)scalar_out, exemplar));
    VX_CALL(vxReleaseUserDataObject(&exemplar));

    ASSERT_VX_OBJECT(img = vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(exemplar2 = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data), VX_TYPE_USER_DATA_OBJECT);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetSupplementaryUserDataObject((vx_reference)img, exemplar2));
    VX_CALL(vxReleaseUserDataObject(&exemplar2));

    ASSERT_VX_OBJECT(n0 = tivxTestTargetNode(graph, scalar_in, scalar_out, img), VX_TYPE_NODE);

    vxSetReferenceName((vx_reference)n0, "test_target_node");

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)n0, VX_EVENT_NODE_ERROR, 0, NODE_ERROR_EVENT));

    VX_CALL(vxSetNodeTarget(n0, VX_TARGET_STRING, NODE_TARGET));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    tivxTaskWaitMsecs(10);

    /* Asserting that no errors were produced when running the application */
    ASSERT_EQ_VX_STATUS(VX_FAILURE, vxWaitEvent(context, &event, vx_true_e));

    VX_CALL(vxReleaseImage(&img));
    VX_CALL(vxReleaseNode(&n0));
    VX_CALL(vxReleaseScalar(&scalar_in));
    VX_CALL(vxReleaseScalar(&scalar_out));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

TESTCASE_TESTS(tivxTargetFinal,
               testTargetScalar)
