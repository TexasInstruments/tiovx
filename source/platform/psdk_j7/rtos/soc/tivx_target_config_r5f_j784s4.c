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
    /* MCU2-0 Targets */
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU2_0,        0u, "TIVX_CPU_0", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_NF,       1u, "TIVX_V1NF", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_LDC1,     2u, "TIVX_V1LDC", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC1,     3u, "TIVX_V1MSC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC2,     4u, "TIVX_V1MSC2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_VISS1,    5u, "TIVX_V1VISS", 13u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE1,      6u, "TIVX_CAPT1", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE2,      7u, "TIVX_CAPT2", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE3,      8u, "TIVX_CAPT3", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE4,      9u, "TIVX_CAPT4", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE5,     10u, "TIVX_CAPT5", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE6,     11u, "TIVX_CAPT6", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE7,     12u, "TIVX_CAPT7", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE8,     13u, "TIVX_CAPT8", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE9,     14u, "TIVX_CAPT9", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE10,    15u, "TIVX_CAPT10", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE11,    16u, "TIVX_CAPT11", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE12,    17u, "TIVX_CAPT12", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY1,     18u, "TIVX_DISP1", 14u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY2,     19u, "TIVX_DISP2", 14u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CSITX,        20u, "TIVX_CSITX", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CSITX2,       21u, "TIVX_CSITX2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M1, 22u, "TIVX_DPM2M1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M2, 23u, "TIVX_DPM2M2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M3, 24u, "TIVX_DPM2M3", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M4, 25u, "TIVX_DPM2M4", 8u);

    /* MCU2-1 Targets */
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU2_1,    0u, "TIVX_CPU_1", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_SDE, 1u, "TIVX_SDE", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_DOF, 2u, "TIVX_DOF", 8u);

    /* MCU3-0 Targets */
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU3_0, 0u, "TIVX_MCU3_0", 4u);

    /* MCU3-1 Targets */
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU3_1, 0u, "TIVX_MCU3_1", 4u);

    /* MCU4-0 Targets */
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU4_0,      0u, "TIVX_MCU4_0", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_NF,    1u, "TIVX_V2NF", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_LDC1,  2u, "TIVX_V2LDC", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_MSC1,  3u, "TIVX_V2MSC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_MSC2,  4u, "TIVX_V2MSC2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_VISS1, 5u, "TIVX_V2VISS", 13u);

    /* MCU4-1 Targets */
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU4_1, 0u, "TIVX_MCU4_1", 4u);
}

void ownPlatformDeleteTargets(void)
{
    /* MCU2-0 Targets */
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU2_0);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_NF);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_LDC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_VISS1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE8);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE9);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE10);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE11);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE12);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CSITX);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CSITX2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M4);

    /* MCU2-1 targets */
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU2_1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_SDE);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_DOF);

    /* MCU3-0 targets */
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU3_0);

    /* MCU3-1 targets */
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU3_1);

    /* MCU4-0 targets */
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU4_0);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_NF);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_LDC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_MSC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_MSC2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_VISS1);

    /* MCU4-1 targets */
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU4_1);
}
