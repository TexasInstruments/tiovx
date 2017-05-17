/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

/*! \brief Pointer to the IPC notify event handler.
 *         It can be registered using #tivxIpcRegisterHandler API
 * \ingroup group_tivx_ipc
 */
static tivx_ipc_handler_f g_ipc_handler = NULL;
static vx_enum g_cpu_id = TIVX_CPU_ID_DSP1;

void tivxIpcRegisterHandler(tivx_ipc_handler_f notifyCb)
{
    g_ipc_handler = notifyCb;
}

vx_status tivxIpcSendMsg(
    vx_enum cpu_id, uint32_t payload)
{
    vx_status status = VX_SUCCESS;

    if( cpu_id < TIVX_CPU_ID_MAX && g_ipc_handler != NULL)
    {
        g_ipc_handler(payload);
    }
    else
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

void tivxSetSelfCpuId(vx_enum cpu_id)
{
    g_cpu_id = cpu_id;
}

vx_enum tivxGetSelfCpuId(void)
{
    return (g_cpu_id);
}

void tivxIpcInit(void)
{
}

void tivxIpcDeInit(void)
{
}
