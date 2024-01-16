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

#include <vx_context.h>

TESTCASE(tivxInternalUserDataObject, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalUserDataObject, negativeTestUnmapUserDataObjectMapId)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object src_user_data;
    vx_size offset = 0, size = 0;
    tivx_tensor_map_info_t map_addr;
    uint8_t temp; 
    vx_map_id mid[2];
    vx_enum usage = VX_READ_ONLY, user_mem_type = VX_MEMORY_TYPE_NONE;
    vx_uint32 flags = 0, udata = 0, *pdata[2] = {0};
    vx_char test_name[] = {'t', 'e', 's', 't', 'i', 'n', 'g'};
    int i;
    for (i = 0; i < 2; i++)
    {
        pdata[i] = 0;
    }
    vx_size temp_size;
    ASSERT_VX_OBJECT(src_user_data = vxCreateUserDataObject(context, test_name, sizeof(vx_uint32), &udata), VX_TYPE_USER_DATA_OBJECT);
    for (i = 0; i < 2; i++) 
    {
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapUserDataObject(src_user_data, offset, size, &mid[i], (void *)(&pdata[i]), usage, user_mem_type, flags));
    }
    uint8_t *map_addrs = src_user_data->maps[0].map_addr;
    vx_size map_sizes = src_user_data->maps[1].map_size;
    src_user_data->maps[0].map_addr=NULL;
    src_user_data->maps[1].map_size = 0;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxUnmapUserDataObject(src_user_data, mid[0]));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxUnmapUserDataObject(src_user_data, mid[1]));
    src_user_data->maps[0].map_addr = map_addrs;
    src_user_data->maps[1].map_size = map_sizes;
    VX_CALL(vxUnmapUserDataObject(src_user_data, mid[0]));
    VX_CALL(vxUnmapUserDataObject(src_user_data, mid[1]));
    VX_CALL(vxReleaseUserDataObject(&src_user_data));
}

TESTCASE_TESTS(
    tivxInternalUserDataObject,
    negativeTestUnmapUserDataObjectMapId
)