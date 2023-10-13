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
#include <TI/tivx_obj_desc.h>
#include <TI/tivx_config.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include <string.h>
#include <math.h>
#include <TI/tivx_mutex.h>
#include <TI/tivx_queue.h>

/* The below include files are used for TIVX_TEST_WAIVER_COMPLEXITY_AND_MAINTENANCE_COST_001
 * described below */

#include <tivx_event_queue.h>
#include <tivx_obj_desc_priv.h>
#include <vx_reference.h>
#include <vx_context.h>
#include <tivx_data_ref_queue.h>
#include <vx_node.h>
#include <vx_image.h>

#include "test_engine/test.h"
#include "test_tiovx.h"

#include <TI/tivx_event.h>
#include <TI/tivx_task.h>
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_capture.h>

#include "shared_functions.h"

TESTCASE(tivxInternalApis, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalApis, negativeTestInternalNode)
{
    vx_context context = context_->vx_context_;

    vx_node node = NULL;
    vx_meta_format meta = NULL;
    vx_reference prm_ref[TIVX_KERNEL_MAX_PARAMS] = {NULL};
    vx_graph graph = NULL;
    vx_kernel kernel = NULL;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownResetNodePerf(node));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownUpdateNodePerf(node,0));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownSetNodeImmTarget(node));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownSetNodeAttributeValidRectReset(node,vx_false_e));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownNodeRegisterEvent(node, 0, 0));

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, ownNodeKernelValidate(node, &meta));

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownNodeUserKernelExecute(node, prm_ref));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownNodeUserKernelExecute(node, prm_ref));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxInternalApis, negativeTestInternalReference)
{
    #define VX_TYPE_UNKNOWN 4

    vx_context context = context_->vx_context_;
    vx_image image = NULL;
    vx_uint64 is_invalid, set_is_valid = 10;
    vx_reference ref = NULL;
    vx_bool ret = vx_false_e, ret1 = vx_false_e;
    vx_uint32 result = 0,result1 = 0, result2 = 0;
    vx_threshold src_threshold = NULL;

    ASSERT_VX_OBJECT(image = vxCreateImage(context, 64, 48, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    result = ownDecrementReference((vx_reference)image,VX_EXTERNAL);
    ASSERT(result==0);
    result2 = ownDecrementReference(ref,VX_EXTERNAL);
    ASSERT(result2!=0);
    ownReferenceSetScope(((vx_reference)image),NULL);
    VX_CALL(vxReleaseImage(&image));
    result1 = ownIncrementReference(NULL,VX_TYPE_IMAGE);
    ASSERT(result1==0);
    ret = ownIsValidType(VX_TYPE_KEYPOINT);
    ASSERT(ret != vx_false_e);
    ret = ownIsValidType(VX_TYPE_USER_STRUCT_START);
    ASSERT(ret == vx_false_e);
    ASSERT_VX_OBJECT(ref = ownCreateReference(context, VX_TYPE_IMAGE, VX_TYPE_UNKNOWN, &context->base), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownReferenceAllocMem(NULL));
    VX_CALL(vxReleaseReference(&ref));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownReferenceLock(NULL));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownReferenceUnlock(NULL));
    ownReferenceSetScope(ref,&context->base);

    ASSERT_VX_OBJECT(src_threshold = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8), VX_TYPE_THRESHOLD);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownAllocReferenceBufferGeneric((vx_reference)src_threshold));
    VX_CALL(vxReleaseThreshold(&src_threshold));
}

TEST(tivxInternalApis, negativeTestInternalValidReference)
{
    vx_context context = context_->vx_context_;
    vx_image image;
    vx_reference ref;
    image = (vx_image)ownCreateReference(context, (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_EXTERNAL, &context->base);
    ref = (vx_reference)image;
    ref->context = NULL;
    ASSERT(vx_false_e==ownIsValidReference((vx_reference)image));
    ref->context = context;
    VX_CALL(vxReleaseImage(&image));
}

TESTCASE_TESTS(tivxInternalApis,
        negativeTestInternalNode,
        negativeTestInternalReference,
        negativeTestInternalValidReference
        )