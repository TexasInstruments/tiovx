/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_vision_sdk.h>

#include <xdc/std.h>
#include <include/link_api/system_if.h>
#include <include/link_api/system_procId.h>
#include <src/rtos/links_common/system/system_priv_openvx.h>

static void tivxIpcHandler(uint32_t payload);

/*! \brief An array of vision sdk CPU Ids. This is used for mapping
 *   tivx_cpu_id_e to vision sdk cpu id. This mapping is required for
 *   sending notify event using vision sdk API.
 *   vx_enum tivx_cpu_id_e is used as index into this array to get
 *   vision sdk cpu id.
 * \ingroup group_tivx_ipc
 */
static uint32_t g_ipc_cpu_id_map[TIVX_CPU_ID_MAX] = {
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
 *         It can be registered using #tivxIpcRegisterHandler API
 * \ingroup group_tivx_ipc
 */
static tivx_ipc_handler_f g_ipc_handler = NULL;


/*! \brief Global IPC handler
 * \ingroup group_tivx_ipc
 */
static void tivxIpcHandler(uint32_t payload)
{
    if (NULL != g_ipc_handler)
    {
        g_ipc_handler(payload);
    }
}

void tivxIpcRegisterHandler(tivx_ipc_handler_f notifyCb)
{
    g_ipc_handler = notifyCb;
}

vx_status tivxIpcSendMsg(
    vx_enum cpu_id, uint32_t payload)
{
    /* convert OpenVX CPU ID to VSDK CPU ID */
    uint32_t vsdk_cpu_id;
    vx_status status;

    if( cpu_id < TIVX_CPU_ID_MAX)
    {
        vsdk_cpu_id  = g_ipc_cpu_id_map[cpu_id];

        status = System_openVxSendNotify(
            vsdk_cpu_id,
            payload);

        if( status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxIpcSendMsg: OpenVX send notification failed\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxIpcSendMsg: CPU ID invalid\n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

vx_enum tivxGetSelfCpuId(void)
{
    vx_enum cpu_id = TIVX_INVALID_CPU_ID;
    uint32_t i, vsdk_cpu_id;

    vsdk_cpu_id =  System_getSelfProcId();

    for (i = 0; i < dimof(g_ipc_cpu_id_map); i ++)
    {
        if (vsdk_cpu_id == g_ipc_cpu_id_map[i])
        {
            cpu_id = (vx_enum)i;
            break;
        }
    }

    return (cpu_id);
}

void tivxIpcInit(void)
{
    /* Register IPC Handler */
    System_registerOpenVxNotifyCb(tivxIpcHandler);
}

void tivxIpcDeInit(void)
{
    /* Un-Register IPC Handler */
    System_registerOpenVxNotifyCb(NULL);
}

vx_bool tivxIsTargetEnabled(char target_name[])
{
    vx_bool isEnabled = vx_false_e;
    vx_enum target_id = TIVX_TARGET_ID_INVALID;
    vx_enum cpu_id;
    uint32_t vsdk_cpu_id;
    Bool vsdk_isenabled;

    if (NULL != target_name)
    {
        /* Get the targetId */
        target_id = tivxPlatformGetTargetId(target_name);
        if (target_id != TIVX_TARGET_ID_INVALID)
        {
            cpu_id = tivxTargetGetCpuId(target_id);
            if( cpu_id < TIVX_CPU_ID_MAX)
            {
                vsdk_cpu_id  = g_ipc_cpu_id_map[cpu_id];
                vsdk_isenabled = System_openvxIsProcEnabled(vsdk_cpu_id);

                if (vsdk_isenabled)
                {
                    isEnabled = vx_true_e;
                }
                else
                {
                    isEnabled = vx_false_e;
                }
            }
        }
    }
    return (isEnabled);
}
