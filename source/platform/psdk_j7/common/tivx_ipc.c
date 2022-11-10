/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_psdk.h>

#include <utils/ipc/include/app_ipc.h>

static void tivxIpcHandler(uint32_t src_cpu_id, uint32_t payload);

/*! \brief An array of vision sdk CPU Ids. This is used for mapping
 *   tivx_cpu_id_e to vision sdk cpu id. This mapping is required for
 *   sending notify event using vision sdk API.
 *   vx_enum tivx_cpu_id_e is used as index into this array to get
 *   vision sdk cpu id.
 * \ingroup group_tivx_ipc
 */
#if defined (SOC_J721E)
static uint32_t g_ipc_cpu_id_map[TIVX_CPU_ID_MAX] = {
    APP_IPC_CPU_C6x_1,
    APP_IPC_CPU_C6x_2,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_MCU2_0, /* in j721e, TIOVX CPU IPU1-0 is mapped to vision_apps/pdk CPU mcu2-0 */
    APP_IPC_CPU_MCU2_1, /* in j721e, TIOVX CPU IPU1-1 is mapped to vision_apps/pdk CPU mcu2-1 */
    APP_IPC_CPU_MPU1_0,
    APP_IPC_CPU_MCU3_0,
    APP_IPC_CPU_MCU3_1
};
#endif

#if defined (SOC_J721S2)
static uint32_t g_ipc_cpu_id_map[TIVX_CPU_ID_MAX] = {
    APP_IPC_CPU_C7x_2,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_MCU2_0, /* in j721s2, TIOVX CPU IPU1-0 is mapped to vision_apps/pdk CPU mcu2-0 */
    APP_IPC_CPU_MCU2_1, /* in j721s2, TIOVX CPU IPU1-1 is mapped to vision_apps/pdk CPU mcu2-1 */
    APP_IPC_CPU_MPU1_0,
    APP_IPC_CPU_MCU3_0,
    APP_IPC_CPU_MCU3_1
};
#endif

#if defined (SOC_J784S4)
static uint32_t g_ipc_cpu_id_map[TIVX_CPU_ID_MAX] = {
    APP_IPC_CPU_C7x_2,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_MCU2_0, /* in j784s4, TIOVX CPU IPU1-0 is mapped to vision_apps/pdk CPU mcu2-0 */
    APP_IPC_CPU_MCU2_1, /* in j784s4, TIOVX CPU IPU1-1 is mapped to vision_apps/pdk CPU mcu2-1 */
    APP_IPC_CPU_MPU1_0,
    APP_IPC_CPU_MCU3_0,
    APP_IPC_CPU_MCU3_1,
    APP_IPC_CPU_MCU4_0,
    APP_IPC_CPU_MCU4_1,
    APP_IPC_CPU_C7x_3,
    APP_IPC_CPU_C7x_4
};
#endif

#if defined (SOC_AM62A)
static uint32_t g_ipc_cpu_id_map[TIVX_CPU_ID_MAX] = {
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_MCU1_0, /* in am62a, TIOVX CPU IPU1-0 is mapped to vision_apps/pdk CPU mcu1-0 */
    APP_IPC_CPU_MPU1_0
};
#endif

/*! \brief Pointer to the IPC notify event handler.
 *         It can be registered using #ownIpcRegisterHandler API
 * \ingroup group_tivx_ipc
 */
static tivx_ipc_handler_f g_ipc_handler = NULL;


/*! \brief Global IPC handler
 * \ingroup group_tivx_ipc
 */
static void tivxIpcHandler(uint32_t src_cpu_id, uint32_t payload)
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

        /* if dest CPU and the host CPU is same then send IPC message to
         * the specific host port ID so that the process (Linux) which
         * created the object descriptor gets the response
         *
         * if dest CPU is not host CPU then this is some other RTOS CPU,
         * in this case send to the common global TIOVX port ID used by all
         * RTOS
         */
        if(cpu_id==(vx_enum)host_cpu_id)
        {
            status = appIpcSendNotifyPort(
                vsdk_cpu_id,
                payload,
                host_port_id);
        }
        else
        {
            status = appIpcSendNotify(
                vsdk_cpu_id,
                payload);
        }

        if( status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "OpenVX send notification failed\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "CPU ID invalid\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

vx_enum tivxGetSelfCpuId(void)
{
    vx_enum cpu_id = (vx_enum)TIVX_CPU_ID_INVALID;
    uint32_t i, vsdk_cpu_id;

    vsdk_cpu_id =  appIpcGetSelfCpuId();

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

uint16_t ownIpcGetHostPortId(uint16_t cpu_id)
{
    /* convert OpenVX CPU ID to VSDK CPU ID */
    uint32_t vsdk_cpu_id;
    uint32_t host_port_id = 0;

    if( cpu_id < (vx_enum)TIVX_CPU_ID_MAX)
    {
        vsdk_cpu_id  = g_ipc_cpu_id_map[cpu_id];

        if(vsdk_cpu_id != APP_IPC_CPU_INVALID )
        {
            host_port_id = appIpcGetHostPortId(vsdk_cpu_id);
        }
    }

    /* host port ID is 16b max to this type conversion is ok */
    return (uint16_t)host_port_id;
}

void ownIpcInit(void)
{
    /* Register IPC Handler */
    appIpcRegisterNotifyHandler(tivxIpcHandler);
}

void ownIpcDeInit(void)
{
    /* Un-Register IPC Handler */
    appIpcRegisterNotifyHandler(NULL);
}

vx_bool tivxIsTargetEnabled(char target_name[])
{
    vx_bool isEnabled = (vx_bool)vx_false_e;
    vx_enum target_id = (vx_enum)TIVX_TARGET_ID_INVALID;
    vx_enum cpu_id;
    uint32_t vsdk_cpu_id;
    uint32_t vsdk_isenabled;

    if (NULL != target_name)
    {
        /* Get the targetId */
        target_id = ownPlatformGetTargetId(target_name);
        if (target_id != (vx_enum)TIVX_TARGET_ID_INVALID)
        {
            cpu_id = ownTargetGetCpuId(target_id);
            if( cpu_id < (vx_enum)TIVX_CPU_ID_MAX)
            {
                vsdk_cpu_id  = g_ipc_cpu_id_map[cpu_id];

                vsdk_isenabled = appIpcIsCpuEnabled(vsdk_cpu_id);

                if (1U == vsdk_isenabled)
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
