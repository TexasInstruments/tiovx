/*
 *******************************************************************************
 *
 * Copyright (C) 2023 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <tivx_platform_psdk.h>

void ownPlatformCreateTargets(void)
{
    /* MCU1-0 targets */
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU1_0, 0u, "TIVX_CPU_0", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_LDC1, 1u, "TIVX_VLDC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC1, 2u, "TIVX_VMSC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC2, 3u, "TIVX_VMSC2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_VISS1, 4u, "TIVX_VVISS1", 13u);
}

void ownPlatformDeleteTargets(void)
{
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU1_0);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_LDC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_VISS1);
}
