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

void ownPlatformCreateTargets(void)
{
    /*
     * Note: All CPU tasks should be at a lower priority than APP_IPC_RPMESSAGE_RX_TASK_PRI, otherwise
     *       new messages would be starved from getting enqueued into the various priority
     *       worker threads.
     * */

    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP1, 0u, "TIVX_DSP1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP2, 0u, "TIVX_DSP2", 8u);
}

void ownPlatformDeleteTargets(void)
{
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP2);
}
