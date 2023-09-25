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

#include "test_tiovx.h"
#include "test_engine/test.h"
#include "test_tiovx/test_tiovx.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_khr_user_data_object.h>
#include <VX/vx_khr_pipelining.h>
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_config.h>
#include <TI/tivx_mutex.h>
#include <TI/tivx_queue.h>

#include <tivx_event_queue.h>
#include <tivx_obj_desc_priv.h>
#include <vx_reference.h>
#include <vx_context.h>
#include <TI/tivx_capture.h>
#include <TI/tivx_task.h>
#include <TI/tivx_target_kernel.h>
#include "math.h"
#include <limits.h>

TESTCASE(tivxInternalObjArray, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalObjArray, negativeTestGetObjectArrayItem1)
{
    vx_context context = context_->vx_context_;
    vx_object_array vxoa = NULL;
    vx_reference ref = NULL;
    vx_uint32 index = 0;
    vx_image image;
    image = (vx_image)ownCreateReference(context, (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_EXTERNAL, &context->base);
    EXPECT_VX_ERROR(vxoa = vxCreateObjectArray(context, (vx_reference)image, 2),VX_ERROR_NO_RESOURCES);
    VX_CALL(ownReleaseReferenceInt((vx_reference*)&image, (vx_enum)VX_TYPE_IMAGE, (vx_enum)VX_EXTERNAL, NULL));
}

TESTCASE_TESTS(
    tivxInternalObjArray,    
    negativeTestGetObjectArrayItem1
)

