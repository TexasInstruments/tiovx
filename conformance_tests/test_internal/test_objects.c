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
 * Copyright (c) 2023 Texas Instruments Incorporated
 */
#include <vx_internal.h>
#include <tivx_objects.h>

#include "test_tiovx.h"
#include "test_engine/test.h"
#include "test_tiovx/test_tiovx.h"
#include <VX/vx.h>
#include <VX/vxu.h>

TESTCASE(tivxInternalObjects, CT_VXContext, ct_setup_vx_context, 0)
typedef struct
{
    const char *name;
    vx_enum type;
} Arg;
static enum vx_type_e type = VX_TYPE_INVALID;
TEST_WITH_ARG(tivxInternalObjects, negativeTestObjectDeInit, Arg,
              ARG_ENUM(VX_TYPE_META_FORMAT),
              ARG_ENUM(VX_TYPE_CONTEXT),
              ARG_ENUM(VX_TYPE_GRAPH),
              ARG_ENUM(VX_TYPE_NODE),
              ARG_ENUM(VX_TYPE_KERNEL),
              ARG_ENUM(VX_TYPE_ARRAY),
              ARG_ENUM(VX_TYPE_USER_DATA_OBJECT),
              ARG_ENUM(TIVX_TYPE_RAW_IMAGE),
              ARG_ENUM(VX_TYPE_CONVOLUTION),
              ARG_ENUM(VX_TYPE_DELAY),
              ARG_ENUM(VX_TYPE_DISTRIBUTION),
              ARG_ENUM(VX_TYPE_IMAGE),
              ARG_ENUM(VX_TYPE_TENSOR),
              ARG_ENUM(VX_TYPE_LUT),
              ARG_ENUM(VX_TYPE_MATRIX),
              ARG_ENUM(VX_TYPE_PYRAMID),
              ARG_ENUM(VX_TYPE_REMAP),
              ARG_ENUM(VX_TYPE_SCALAR),
              ARG_ENUM(VX_TYPE_THRESHOLD),
              ARG_ENUM(VX_TYPE_ERROR),
              ARG_ENUM(VX_TYPE_OBJECT_ARRAY),
              ARG_ENUM(VX_TYPE_PARAMETER),
              ARG_ENUM(TIVX_TYPE_DATA_REF_Q))
{
    static tivx_object_t g_tivx_objects;
    vx_reference ref = NULL;
    type = (enum vx_type_e)arg_->type;

    ref = ownObjectAlloc(type);
    EXPECT(ref != NULL);
    if (NULL != ref)
    {
        ref->type = type;
        ASSERT_EQ_VX_STATUS(VX_ZONE_ERROR, ownObjectFree(ref));
        //Try to free memory of a ref object when was cleared by previous call,used to trigger -ve case
        ASSERT_EQ_VX_STATUS(VX_FAILURE, ownObjectFree(ref));
        //Trigger -ve condition when de-initializing a ref object
        ASSERT_EQ_VX_STATUS(VX_ZONE_ERROR, ownObjectDeInit());
        VX_CALL(ownObjectInit());
    }
}
TEST_WITH_ARG(tivxInternalObjects, negativeTestObjectDeInit1, Arg,
              ARG_ENUM(VX_TYPE_META_FORMAT),
              ARG_ENUM(VX_TYPE_CONTEXT),
              ARG_ENUM(VX_TYPE_GRAPH),
              ARG_ENUM(VX_TYPE_NODE),
              ARG_ENUM(VX_TYPE_KERNEL),
              ARG_ENUM(VX_TYPE_ARRAY),
              ARG_ENUM(VX_TYPE_USER_DATA_OBJECT),
              ARG_ENUM(TIVX_TYPE_RAW_IMAGE),
              ARG_ENUM(VX_TYPE_CONVOLUTION),
              ARG_ENUM(VX_TYPE_DELAY),
              ARG_ENUM(VX_TYPE_DISTRIBUTION),
              ARG_ENUM(VX_TYPE_IMAGE),
              ARG_ENUM(VX_TYPE_TENSOR),
              ARG_ENUM(VX_TYPE_LUT),
              ARG_ENUM(VX_TYPE_MATRIX),
              ARG_ENUM(VX_TYPE_PYRAMID),
              ARG_ENUM(VX_TYPE_REMAP),
              ARG_ENUM(VX_TYPE_SCALAR),
              ARG_ENUM(VX_TYPE_THRESHOLD),
              ARG_ENUM(VX_TYPE_ERROR),
              ARG_ENUM(VX_TYPE_OBJECT_ARRAY),
              ARG_ENUM(VX_TYPE_PARAMETER))
{
    static tivx_object_t g_tivx_objects;
    vx_reference ref = NULL;
    type = (enum vx_type_e)arg_->type;

    ref = ownObjectAlloc(type);
    EXPECT(ref != NULL);
    if (NULL != ref)
    {
        ref->type = type;
        //Trigger -ve condition when de-initializing a ref object
        ASSERT_EQ_VX_STATUS(VX_ZONE_ERROR, ownObjectDeInit());
        VX_CALL(ownObjectInit());
    }
}
TEST(tivxInternalObjects, negativeTestObjectDeInit2)
{
    static tivx_object_t g_tivx_objects;
    vx_reference ref = NULL,ref1 = NULL;
    ref = ownObjectAlloc(TIVX_TYPE_DATA_REF_Q);
    EXPECT(ref != NULL);
    if (NULL != ref)
    {
        ASSERT_EQ_VX_STATUS(VX_FAILURE, ownObjectDeInit());
        VX_CALL(ownObjectInit());
    }

    ref1 = ownObjectAlloc(VX_TYPE_META_FORMAT);
    EXPECT(ref != NULL);
    VX_CALL(ownObjectDeInit());
    ASSERT_EQ_VX_STATUS(VX_FAILURE, ownObjectDeInit());
    VX_CALL(ownObjectInit());
}
TEST(tivxInternalObjects, negativeTestObjectAlloc)
{
    vx_reference ref = NULL;

    ref = ownObjectAlloc(VX_TYPE_REFERENCE);
    EXPECT(ref == NULL);

    VX_CALL(ownObjectDeInit());
    ref = ownObjectAlloc(VX_TYPE_REFERENCE);
    EXPECT(ref == NULL);
    VX_CALL(ownObjectInit());
}
TEST(tivxInternalObjects, negativeTestObjectFree)
{
    vx_context context = context_->vx_context_;
    vx_reference ref = NULL;
    vx_status status = VX_SUCCESS;

    status = ownObjectFree(ref);
    EXPECT(status == VX_FAILURE);
    //VX_TYPE_LUT element was used to create a error condition any other element 
    //can be used here provided its not part of the switch case of ownObjectAlloc
    ref = ownObjectAlloc(VX_TYPE_LUT);
    EXPECT(ref != NULL);

    ref->type = VX_TYPE_REFERENCE;
    status = ownObjectFree(ref);
    EXPECT(status == VX_ERROR_INVALID_REFERENCE);

    ref->type = VX_TYPE_LUT;
    status = ownObjectFree(ref);
    EXPECT(status == VX_SUCCESS);

    ref = ownObjectAlloc(VX_TYPE_LUT);
    VX_CALL(ownObjectDeInit());
    status = ownObjectFree(ref);
    EXPECT(status == VX_ERROR_INVALID_PARAMETERS);
    VX_CALL(ownObjectInit());
    ref = ownObjectAlloc(VX_TYPE_LUT);
    status = ownObjectFree(ref);
    EXPECT(status == VX_SUCCESS);

}
TESTCASE_TESTS(
    tivxInternalObjects,
    negativeTestObjectDeInit,
    negativeTestObjectAlloc,
    negativeTestObjectFree,
    negativeTestObjectDeInit1,
    negativeTestObjectDeInit2)