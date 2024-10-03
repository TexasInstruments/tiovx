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

    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1,       0u, "TIVX_C71_P1", 9u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_2, 1u, "TIVX_C71_P2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_3, 2u, "TIVX_C71_P3", 7u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_4, 3u, "TIVX_C71_P4", 6u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_5, 4u, "TIVX_C71_P5", 5u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_6, 5u, "TIVX_C71_P6", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_7, 6u, "TIVX_C71_P7", 3u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_8, 7u, "TIVX_C71_P8", 2u);
    #if defined(SOC_J721S2)
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP1,           0u, "TIVX_CPU",       8u);
    #endif
    #if defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2,       0u, "TIVX_C72_P1", 9u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_2, 1u, "TIVX_C72_P2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_3, 2u, "TIVX_C72_P3", 7u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_4, 3u, "TIVX_C72_P4", 6u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_5, 4u, "TIVX_C72_P5", 5u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_6, 5u, "TIVX_C72_P6", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_7, 6u, "TIVX_C72_P7", 3u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_8, 7u, "TIVX_C72_P8", 2u);
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3,       0u, "TIVX_C73_P1", 9u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_2, 1u, "TIVX_C73_P2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_3, 2u, "TIVX_C73_P3", 7u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_4, 3u, "TIVX_C73_P4", 6u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_5, 4u, "TIVX_C73_P5", 5u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_6, 5u, "TIVX_C73_P6", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_7, 6u, "TIVX_C73_P7", 3u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_8, 7u, "TIVX_C73_P8", 2u);
    #endif /* defined(SOC_J784S4) || defined(SOC_J742S2) */
    #if defined(SOC_J784S4)
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4,       0u, "TIVX_C74_P1", 9u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_2, 1u, "TIVX_C74_P2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_3, 2u, "TIVX_C74_P3", 7u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_4, 3u, "TIVX_C74_P4", 6u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_5, 4u, "TIVX_C74_P5", 5u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_6, 5u, "TIVX_C74_P6", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_7, 6u, "TIVX_C74_P7", 3u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_8, 7u, "TIVX_C74_P8", 2u);
    #endif /* defined(SOC_J784S4) */
    #endif /* defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2) */
}

/*LDRA_NOANALYSIS*/
/* TIOVX-1970- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_TARGET_CONFIG_C7x_UM001 */
void ownPlatformDeleteTargets(void)
{
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_8);
    #if defined(SOC_J721S2)
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP1);
    #endif
    #if defined(SOC_J784S4) || defined(SOC_J722S)|| defined(SOC_J742S2)
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_8);
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_8);
    #endif /* defined(SOC_J784S4) || defined(SOC_J742S2) */
    #if defined(SOC_J784S4)
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_8);
    #endif /* defined(SOC_J784S4) */
    #endif /* defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2) */
}
/*LDRA_ANALYSIS*/