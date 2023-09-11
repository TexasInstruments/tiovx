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
        VX_CALL(ownObjDescFree((tivx_obj_desc_t**)&obj_desc[j]));
    }

    VX_CALL(ownReleaseReferenceInt((vx_reference*)&img, (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_EXTERNAL, NULL));
}

TESTCASE_TESTS(tivxObjDescBoundary,
        negativeTestObjDescBoundary
        )
