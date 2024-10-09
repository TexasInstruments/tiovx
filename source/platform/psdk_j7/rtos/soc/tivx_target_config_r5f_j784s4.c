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
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU2_0, 0, "TIVX_CPU_0", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_NF, 1, "TIVX_V2NF", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_LDC1, 2, "TIVXV2LDC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_MSC1, 3, "TIVXV2MSC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_MSC2, 4, "TIVXV2MSC2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_VISS1, 5, "TIVXV2VISS1", 13u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_SDE, 6, "TIVX_SDE", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_DOF, 7, "TIVX_DOF", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY1, 8, "TIVX_DISP1", 14u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY2, 9, "TIVX_DISP2", 14u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M1, 10, "TIVX_DPM2M1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M2, 11, "TIVX_DPM2M2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M3, 12, "TIVX_DPM2M3", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M4, 13, "TIVX_DPM2M4", 8u);

    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU2_1, 0, "TIVX_CPU_1", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_NF, 1, "TIVX_V1NF", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_LDC1, 2, "TIVX_V1LDC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC1, 3, "TIVX_V1SC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC2, 4, "TIVX_V1MSC2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_VISS1, 5, "TIVXVVISS1", 13u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE1, 6, "TIVX_CAPT1", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE2, 7, "TIVX_CAPT2", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CSITX, 8, "TIVX_CSITX", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE3, 9, "TIVX_CAPT3", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE4, 10, "TIVX_CAPT4", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE5, 11, "TIVX_CAPT5", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE6, 12, "TIVX_CAPT6", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE7, 13, "TIVX_CAPT7", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE8, 14, "TIVX_CAPT8", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE9, 15, "TIVX_CAPT9", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE10, 16, "TIVX_CAPT10", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE11, 17, "TIVX_CAPT11", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE12, 18, "TIVX_CAPT12", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CSITX2, 19, "TIVX_CSITX2", 8u);
}

/*LDRA_NOANALYSIS*/
/* TIOVX-1769- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_CONFIG_R5F_J784S4_UM001 */
void ownPlatformDeleteTargets(void)
{
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU2_0);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_NF);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_LDC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_MSC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_MSC2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_VISS1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_SDE);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_DOF);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M4);

    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU2_1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_NF);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_LDC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_VISS1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CSITX);
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
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CSITX2);
}
/*LDRA_ANALYSIS*/
