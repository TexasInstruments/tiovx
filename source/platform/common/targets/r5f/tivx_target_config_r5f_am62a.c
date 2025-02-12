/*
 *******************************************************************************
 *
 * Copyright (C) 2023 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <tivx_target_config.h>

void ownPlatformCreateTargetsR5f(void)
{
    /* MCU1-0 targets */
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU1_0, 0u, "TIVX_CPU_0", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_LDC1, 1u, "TIVX_VLDC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC1, 2u, "TIVX_VMSC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC2, 3u, "TIVX_VMSC2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_VISS1, 4u, "TIVX_VVISS1", 13u);
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void ownPlatformDeleteTargetsR5f.* <function end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_CONFIG_R5F_UM001
<justification end> */
void ownPlatformDeleteTargetsR5f(void)
{
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU1_0);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_LDC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_VISS1);
}

#ifndef PC
void ownPlatformCreateTargets(void)
{
    ownPlatformCreateTargetsR5f();
}

void ownPlatformDeleteTargets(void)
{
    ownPlatformDeleteTargetsR5f();
}
#endif
