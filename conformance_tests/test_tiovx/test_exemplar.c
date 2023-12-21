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

#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx_config.h>
#include <TI/tivx_obj_desc.h>
#include "test_engine/test.h"
#include <VX/vx_types.h>
#include <TI/tivx.h>
vx_enum type;

TESTCASE(tivxExemplar, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    vx_enum type;
} type_arg;

TEST_WITH_ARG(tivxExemplar, negativeTestSetExemplarReferenceType, type_arg,
              ARG_ENUM(VX_TYPE_TENSOR),
              ARG_ENUM(VX_TYPE_USER_DATA_OBJECT))
{
    vx_context context = context_->vx_context_;
    vx_reference exemplar;
    vx_size nod = TIVX_CONTEXT_MAX_TENSOR_DIMS;
    vx_size dims[TIVX_CONTEXT_MAX_TENSOR_DIMS] = {0};
    vx_enum dt = VX_TYPE_UINT8;
    vx_int8 fpp = 0;
    type = (enum vx_type_e)arg_->type;
    vx_uint32 udata = 0;
    vx_char test_name[] = {'r'};
    vx_reference ref;
    
    switch ((uint32_t)type)
    {
    case VX_TYPE_TENSOR:
        ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateTensor(context, nod, dims, dt, fpp), (enum vx_type_e)(VX_TYPE_TENSOR));
        ASSERT_VX_OBJECT(ref = tivxCreateReferenceFromExemplar(context, (vx_reference)exemplar),(enum vx_type_e)VX_TYPE_TENSOR);
        break;
    case VX_TYPE_USER_DATA_OBJECT:
        ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateUserDataObject(context, test_name, sizeof(vx_uint32), &udata), VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(ref = tivxCreateReferenceFromExemplar(context, (vx_reference)exemplar), VX_TYPE_USER_DATA_OBJECT);
        break;
    default:
        break;
    }
    VX_CALL(vxReleaseReference(&exemplar));
    VX_CALL(vxReleaseReference(&ref));
}

TEST(tivxExemplar, negativeTestExemplarType)
{
    vx_size count = 1;
    vx_context context = context_->vx_context_;
    vx_reference exemplar;
    vx_reference ref;
    vx_scalar scalar = NULL;
    vx_uint32 tmp_value = 0;
    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT32, &tmp_value), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateDelay(context, (vx_reference)scalar, count), VX_TYPE_DELAY);
    ASSERT_NO_FAILURE(ref = tivxCreateReferenceFromExemplar(context, (vx_reference)exemplar));
    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseReference(&exemplar));
}
TESTCASE_TESTS(
    tivxExemplar,
    negativeTestSetExemplarReferenceType,
    negativeTestExemplarType
    )
