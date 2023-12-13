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
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_config.h>
#include <TI/tivx_capture.h>
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

#include "shared_functions.h"

#define MAX_POINTS 100

TESTCASE(tivxObjDescBoundary, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxObjDescBoundary, negativeTestObjDescBoundary)
{
    extern tivx_obj_desc_t *ownObjDescAlloc(vx_enum type, vx_reference ref);
    extern vx_status ownObjDescFree(tivx_obj_desc_t **obj_desc);

    vx_context context = context_->vx_context_;
    int i, j;
    tivx_obj_desc_t *obj_desc[TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST] = {NULL};
    vx_image img = NULL;

    img = (vx_image)ownCreateReference(context, (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_EXTERNAL, &context->base);

    for (i = 0; i < TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST; i++)
    {
        obj_desc[i] = (tivx_obj_desc_t *)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_IMAGE, (vx_reference)img);
        if (NULL != obj_desc[i])
        {
            break;
        }
    }

     for (j = 0; j < i; j++)
    {
        if (NULL == obj_desc[j])
        {
            break;
        }
        VX_CALL(ownObjDescFree((tivx_obj_desc_t**)&obj_desc[j]));
    }

    VX_CALL(ownReleaseReferenceInt((vx_reference*)&img, (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_EXTERNAL, NULL));
}

TEST(tivxObjDescBoundary, negativeBoundaryThreshold)
{
    extern tivx_obj_desc_t *ownObjDescAlloc(vx_enum type, vx_reference ref);
    extern vx_status ownObjDescFree(tivx_obj_desc_t **obj_desc);

    vx_context context = context_->vx_context_;
    int i, j;
    tivx_obj_desc_t *obj_desc[TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST] = {NULL};
    vx_image img;
    vx_image img1;
    vx_threshold vxt=NULL;
    vx_object_array vxoa = NULL;
    vx_node node = NULL;
    tivx_shared_mem_ptr_t tsmp;
    uint32_t size = 1024U;
    vx_object_array src_object_array;

    vx_convolution conv = NULL;
    vx_size rows = 3, cols = 3;

    ASSERT_VX_OBJECT(conv = vxCreateConvolution(context, cols, rows), VX_TYPE_CONVOLUTION);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, ownAllocReferenceBufferGeneric((vx_reference)conv));
    VX_CALL(vxReleaseConvolution(&conv));

    vx_graph graph = NULL;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    img = (vx_image)ownCreateReference(context, (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_EXTERNAL, &context->base);
    for (i = 0; i < TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST-1; i++)
    {
        obj_desc[i] = (tivx_obj_desc_t *)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_IMAGE, (vx_reference)img);
        if (NULL == obj_desc[i])
        {
            break;
        }
    }

    EXPECT_VX_ERROR(vxt = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8), VX_ERROR_NO_RESOURCES);
    EXPECT_VX_ERROR(vxoa = vxCreateObjectArray(context, (vx_reference)img, 2), VX_ERROR_NO_RESOURCES);
    EXPECT_VX_ERROR(src_object_array = vxCreateVirtualObjectArray(graph, (vx_reference)img, 32), VX_ERROR_NO_RESOURCES);
    EXPECT_VX_ERROR(img1 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_ERROR_NO_RESOURCES);

    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownNodeKernelInitKernelName(node));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, ownAllocReferenceBufferGeneric((vx_reference)img));
    for (j = 0; j < i-1; j++)
    {
        if (NULL == obj_desc[j])
        {
            break;
        }
        VX_CALL(ownObjDescFree((tivx_obj_desc_t**)&obj_desc[j]));
    }
    if(NULL!=img)
    {
        VX_CALL(ownReleaseReferenceInt((vx_reference*)&img, (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_EXTERNAL, NULL));
    }
    VX_CALL(vxReleaseGraph(&graph));
}

TESTCASE_TESTS(tivxObjDescBoundary,
        negativeTestObjDescBoundary,
        negativeBoundaryThreshold
        )