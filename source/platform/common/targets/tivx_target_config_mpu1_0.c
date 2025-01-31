/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_target_config.h>

void ownPlatformCreateTargetsMpu(void)
{
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MPU_0, 0u, "TIVX_CPU0", 8U);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MPU_1, 1u, "TIVX_CPU1", 8U);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MPU_2, 2u, "TIVX_CPU2", 8U);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MPU_3, 3u, "TIVX_CPU3", 8U);
    #if defined(SOC_AM62A) && defined(QNX)
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE1, 4u, "TIVX_CAPT1", 8U);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE2, 5u, "TIVX_CAPT2", 8U);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE3, 6u, "TIVX_CAPT3", 8U);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE4, 7u, "TIVX_CAPT4", 8U);
    #endif
}

void ownPlatformDeleteTargetsMpu(void)
{
    #if defined(SOC_AM62A) && defined(QNX)
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE4);
    #endif
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MPU_0);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MPU_1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MPU_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MPU_3);
}

#ifndef PC
void ownPlatformCreateTargets(void)
{
    ownPlatformCreateTargetsMpu();
}

void ownPlatformDeleteTargets(void)
{
    ownPlatformDeleteTargetsMpu();
}
#endif
