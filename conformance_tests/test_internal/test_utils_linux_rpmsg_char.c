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

#include "test_engine/test.h"
#include <TI/tivx_debug.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include "TI/tivx.h"
#include <VX/vx_types.h>
#include <utils/mem/include/app_mem.h>

#if defined(LINUX) && !defined(PC)
#include <utils/ipc/src/app_ipc_linux_priv.h>

TESTCASE(tivxRpmsgChar, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxRpmsgChar, testappIpcCreateTxCh)
{
    uint32_t remote_app_cpu_id = APP_IPC_CPU_MCU1_0;
    uint32_t remote_endpt = 0;
    uint32_t local_endpt = 0;
    rpmsg_char_dev_t *rcdev = NULL;
    char *eptdev_name = NULL;

    ASSERT((vx_status)VX_FAILURE == appIpcCreateTxCh(remote_app_cpu_id, remote_endpt, &local_endpt, (rpmsg_char_dev_t **)&rcdev, eptdev_name));

    remote_app_cpu_id = APP_IPC_CPU_MCU1_1;
    ASSERT((vx_status)VX_FAILURE == appIpcCreateTxCh(remote_app_cpu_id, remote_endpt, &local_endpt, (rpmsg_char_dev_t **)&rcdev, eptdev_name));

    remote_app_cpu_id = -1u;
    ASSERT((vx_status)VX_FAILURE == appIpcCreateTxCh(remote_app_cpu_id, remote_endpt, &local_endpt, (rpmsg_char_dev_t **)&rcdev, eptdev_name));
}

TEST(tivxRpmsgChar, testappIpcDeleteCh)
{
    rpmsg_char_dev_t rcdev;

    ASSERT((vx_status)VX_SUCCESS != appIpcDeleteCh(&rcdev));
}

TESTCASE_TESTS(
    tivxRpmsgChar,
    testappIpcCreateTxCh,
    testappIpcDeleteCh
    )
#endif