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

/* this is required for the funtion defined in system_if.h
 * Below typedef is done to avoid pulling in XDC defined types on linux side
 */
typedef uint32_t UInt32;

#include <include/link_api/system_if.h>
#include <include/link_api/system_procId.h>
#include <src/hlos/system/system_priv_openvx.h>

/*! \brief An array of vision sdk CPU Ids. This is used for mapping
 *   tivx_cpu_id_e to vision sdk cpu id. This mapping is required for
 *   sending notify event using vision sdk API.
 *   vx_enum tivx_cpu_id_e is used as index into this array to get
 *   vision sdk cpu id.
 * \ingroup group_tivx_ipc
 */
static uint32_t g_ipc_cpu_id_map[(vx_enum)TIVX_CPU_ID_MAX] = {
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
 *         It can be registered using #ownIpcRegisterHandler API
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

void ownIpcRegisterHandler(tivx_ipc_handler_f notifyCb)
{
    g_ipc_handler = notifyCb;
}

vx_status ownIpcSendMsg(
    vx_enum cpu_id, uint32_t payload, uint32_t host_cpu_id, uint32_t host_port_id)
{
    /* convert OpenVX CPU ID to VSDK CPU ID */
    uint32_t vsdk_cpu_id;
    vx_status status;

    if( cpu_id < (vx_enum)TIVX_CPU_ID_MAX)
    {
        vsdk_cpu_id  = g_ipc_cpu_id_map[cpu_id];

        status = System_openVxSendNotify(
            vsdk_cpu_id,
            payload);

        if( status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "Send notification failed\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid CPU ID\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

uint32_t tivxIpcGetSelfPortId(void)
{
    /* NOT used */
    return 0;
}

vx_enum tivxGetSelfCpuId(void)
{
    vx_enum cpu_id = (vx_enum)TIVX_CPU_ID_INVALID;
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

void ownIpcInit(void)
{
    /* Register IPC Handler */
    System_registerOpenVxNotifyCb(tivxIpcHandler);
}

void ownIpcDeInit(void)
{
    /* Un-Register IPC Handler */
    System_registerOpenVxNotifyCb(NULL);
}

vx_bool tivxIsTargetEnabled(char target_name[])
{
    vx_bool isEnabled = (vx_bool)vx_false_e;
    vx_enum target_id = TIVX_TARGET_ID_INVALID;
    vx_enum cpu_id;
    uint32_t vsdk_cpu_id;
    uint32_t vsdk_isenabled;

    if (NULL != target_name)
    {
        /* Get the targetId */
        target_id = ownPlatformGetTargetId(target_name);
        if (target_id != TIVX_TARGET_ID_INVALID)
        {
            cpu_id = ownTargetGetCpuId(target_id);
            if( cpu_id < (vx_enum)TIVX_CPU_ID_MAX)
            {
                vsdk_cpu_id  = g_ipc_cpu_id_map[cpu_id];
                vsdk_isenabled = System_openvxIsProcEnabled(vsdk_cpu_id);

                if (vsdk_isenabled)
                {
                    isEnabled = (vx_bool)vx_true_e;
                }
                else
                {
                    isEnabled = (vx_bool)vx_false_e;
                }
            }
        }
    }

    return (isEnabled);
}
