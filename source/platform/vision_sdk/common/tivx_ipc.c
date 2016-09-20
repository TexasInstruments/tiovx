/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */
/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

#include <xdc/std.h>

#include <include/link_api/system_if.h>
#include <include/link_api/system_procId.h>
#include <src/links_common/system/system_priv_openvx.h>
#include "tivx_platform_vision_sdk.h"
#include "tivx_platform_priv.h"

/*! \brief An array of vision sdk CPU Ids. This is used for mapping
 *   tivx_cpu_id_e to vision sdk cpu id. This mapping is required for
 *   sending notify event using vision sdk API.
 *   vx_enum tivx_cpu_id_e is used as index into this array to get
 *   vision sdk cpu id.
 * \ingroup group_tivx_platform
 */
static uint32_t gPlatformCpuIdMap[] = {
    SYSTEM_PROC_DSP1,
    SYSTEM_PROC_DSP2,
    SYSTEM_PROC_EVE1,
    SYSTEM_PROC_EVE2,
    SYSTEM_PROC_EVE3,
    SYSTEM_PROC_EVE4,
    SYSTEM_PROC_IPU1_0,
    SYSTEM_PROC_IPU1_1,
    SYSTEM_PROC_IPU2,
    SYSTEM_PROC_A15_0
};

/*! \brief Pointer to the IPC notify event handler.
 *         It can be registered using #tivxPlatformRegisterIpcHandler API
 * \ingroup group_tivx_platform
 */
static System_openVxNotifyHandler g_ipc_notify = NULL;


/*! \brief Global IPC handler
 * \ingroup group_tivx_platform
 */
void tivxPlatformIpcHandler(uint32_t payload)
{
    if (NULL != g_ipc_notify)
    {
        g_ipc_notify(payload);
    }
}

void tivxPlatformRegisterIpcHandler(System_openVxNotifyHandler notifyCb)
{
    g_ipc_notify = notifyCb;
}

vx_status tivxPlatformSendIpcMsg(
    vx_enum cpu_id, uint32_t payload)
{
    return System_openVxSendNotify(
        gPlatformCpuIdMap[cpu_id],
        payload);
}

vx_enum tivxPlatformGetSelfCpuId()
{
    vx_enum cpu_id = TIVX_INVALID_CPU_ID;
    uint32_t i, vsdk_cpu_id;

    vsdk_cpu_id =  System_getSelfProcId();

    for (i = 0; i < dimof(gPlatformCpuIdMap); i ++)
    {
        if (vsdk_cpu_id == gPlatformCpuIdMap[i])
        {
            cpu_id = (vx_enum)i;
            break;
        }
    }

    return (cpu_id);
}
