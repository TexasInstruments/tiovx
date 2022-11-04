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
#include <TI/tivx_config.h>

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

TEST(tivxUserDataObject, negativeTestCreateUserDataObject)
{
    vx_context context = context_->vx_context_;

    vx_user_data_object udobj = NULL;
    vx_char *type_name = NULL;
    vx_size size = 0;
    vx_uint32 udata = 0;

    ASSERT(NULL == (udobj = vxCreateUserDataObject(NULL, type_name, size, &udata)));
    ASSERT(NULL != (udobj = vxCreateUserDataObject(context, type_name, size, &udata)));
}

TEST(tivxUserDataObject, negativeTestQueryUserDataObject)
{
    #define VX_UDO_DEFAULT 0

    vx_context context = context_->vx_context_;

    vx_user_data_object udobj = NULL;
    vx_size size = 0;
    vx_uint32 udata = 0;
    vx_enum attribute = VX_UDO_DEFAULT;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryUserDataObject(udobj, VX_USER_DATA_OBJECT_NAME, &udata, size));
    ASSERT_VX_OBJECT(udobj = vxCreateUserDataObject(context, NULL, 5, NULL), VX_TYPE_USER_DATA_OBJECT);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryUserDataObject(udobj, VX_USER_DATA_OBJECT_NAME, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryUserDataObject(udobj, VX_USER_DATA_OBJECT_SIZE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryUserDataObject(udobj, TIVX_USER_DATA_OBJECT_VALID_SIZE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryUserDataObject(udobj, attribute, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryUserDataObject(udobj, VX_USER_DATA_OBJECT_NAME, NULL, size));
    VX_CALL(vxReleaseUserDataObject(&udobj));
}

TEST(tivxUserDataObject, negativeTestCopyUserDataObject)
{
    vx_context context = context_->vx_context_;

    vx_user_data_object udobj = NULL;
    vx_size offset = 0, size = 0;
    vx_enum usage = VX_READ_ONLY, user_mem_type = VX_MEMORY_TYPE_NONE;
    vx_uint32 udata = 0;
    vx_char tname[] = {'t', 'i', 'o', 'v', 'x'};

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxCopyUserDataObject(udobj, offset, size, &udata, usage, user_mem_type));
    ASSERT_VX_OBJECT(udobj = vxCreateUserDataObject(context, tname, sizeof(vx_uint32), &udata), VX_TYPE_USER_DATA_OBJECT);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxCopyUserDataObject(udobj, offset, size, NULL, usage, user_mem_type));
    VX_CALL(vxReleaseUserDataObject(&udobj));
}

TEST(tivxUserDataObject, negativeTestMapUserDataObject)
{
    vx_context context = context_->vx_context_;

    vx_user_data_object udobj = NULL;
    vx_size offset = 0, size = 0;
    vx_map_id mid = 0;
    vx_enum usage = VX_READ_ONLY, user_mem_type = VX_MEMORY_TYPE_NONE;
    vx_uint32 i = 0, flags = 0, udata = 0, pdata[TIVX_USER_DATA_OBJECT_MAX_MAPS + 1] = {0};
    vx_char tname[] = {'t', 'i', 'o', 'v', 'x'};

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxMapUserDataObject(udobj, offset, size, &mid, (void *)(pdata), usage, user_mem_type, flags));
    ASSERT_VX_OBJECT(udobj = vxCreateUserDataObject(context, tname, sizeof(vx_uint32), &udata), VX_TYPE_USER_DATA_OBJECT);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxMapUserDataObject(udobj, offset, size, NULL, NULL, usage, user_mem_type, flags));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxMapUserDataObject(udobj, 1, sizeof(vx_uint32), &mid, (void *)(pdata), usage, user_mem_type, flags));
    for (i = 0; i < (TIVX_USER_DATA_OBJECT_MAX_MAPS + 1); i++) {
        if (i == TIVX_USER_DATA_OBJECT_MAX_MAPS) {
            ASSERT_EQ_VX_STATUS(VX_ERROR_NO_RESOURCES, vxMapUserDataObject(udobj, offset, size, &mid, (void *)(&pdata[i]), usage, user_mem_type, flags));
        } else {
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapUserDataObject(udobj, offset, size, &mid, (void *)(&pdata[i]), usage, user_mem_type, flags));
        }
    }
    VX_CALL(vxReleaseUserDataObject(&udobj));
}

TEST(tivxUserDataObject, negativeTestUnmapUserDataObject)
{
    vx_context context = context_->vx_context_;

    vx_user_data_object udobj = NULL;
    vx_map_id mid = TIVX_USER_DATA_OBJECT_MAX_MAPS;
    vx_uint32 udata = 0;
    vx_char tname[] = {'t', 'i', 'o', 'v', 'x'};

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxUnmapUserDataObject(udobj, mid));
    ASSERT_VX_OBJECT(udobj = vxCreateUserDataObject(context, tname, sizeof(vx_uint32), &udata), VX_TYPE_USER_DATA_OBJECT);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxUnmapUserDataObject(udobj, mid));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxUnmapUserDataObject(udobj, 1));
    VX_CALL(vxReleaseUserDataObject(&udobj));
}

TEST(tivxUserDataObject, negativeTestSetUserDataObjectAttribute)
{
    #define TIVX_USER_DATA_OBJECT_DEFAULT 0
    vx_context context = context_->vx_context_;

    vx_user_data_object udobj = NULL;
    vx_enum attribute = TIVX_USER_DATA_OBJECT_DEFAULT;
    vx_uint32 udata = 0;
    vx_size size = 0;
    vx_char tname[] = {'t', 'i', 'o', 'v', 'x'};

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, tivxSetUserDataObjectAttribute(udobj, attribute, &udata, size));
    ASSERT_VX_OBJECT(udobj = vxCreateUserDataObject(context, tname, sizeof(vx_uint32), &udata), VX_TYPE_USER_DATA_OBJECT);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxSetUserDataObjectAttribute(udobj, TIVX_USER_DATA_OBJECT_VALID_SIZE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, tivxSetUserDataObjectAttribute(udobj, attribute, &udata, size));
    VX_CALL(vxReleaseUserDataObject(&udobj));
}

TESTCASE_TESTS(
    tivxUserDataObject,
    testValidSize,
    negativeTestCreateUserDataObject,
    negativeTestQueryUserDataObject,
    negativeTestCopyUserDataObject,
    negativeTestMapUserDataObject,
    negativeTestUnmapUserDataObject,
    negativeTestSetUserDataObjectAttribute
)

