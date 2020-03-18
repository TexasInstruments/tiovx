/*

 * Copyright (c) 2012-2018 The Khronos Group Inc.
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
#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_khr_user_data_object.h>

TESTCASE(tivxUserDataObject, CT_VXContext, ct_setup_vx_context, 0)

static const vx_char user_data_object_name[] = "wb_t";

typedef struct
{
    vx_int32 mode;
    vx_int32 gain[4];
    vx_int32 offset[4];
} wb_t;

TEST(tivxUserDataObject, testValidSize)
{
    vx_context context = context_->vx_context_;
    char actual_name[VX_MAX_REFERENCE_NAME];
    vx_size actual_size = 0;
    vx_size valid_size, valid_size_query;
    vx_user_data_object user_data_object = 0;

    ASSERT_VX_OBJECT(user_data_object = vxCreateUserDataObject(context, NULL, sizeof(wb_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* 3. check if user data object actual size corresponds to requested size */
    VX_CALL(vxQueryUserDataObject(user_data_object, VX_USER_DATA_OBJECT_SIZE, &actual_size, sizeof(vx_size)));
    ASSERT_EQ_INT(sizeof(wb_t), actual_size);

    valid_size = actual_size - 4;

    VX_CALL(tivxSetUserDataObjectAttribute(user_data_object, TIVX_USER_DATA_OBJECT_VALID_SIZE, &valid_size, sizeof(vx_size)));

    VX_CALL(vxQueryUserDataObject(user_data_object, TIVX_USER_DATA_OBJECT_VALID_SIZE, &valid_size_query, sizeof(vx_size)));
    ASSERT_EQ_INT(valid_size, valid_size_query);

    VX_CALL(vxReleaseUserDataObject(&user_data_object));
    ASSERT(user_data_object == 0);
}

TESTCASE_TESTS(tivxUserDataObject,
        testValidSize
        )
