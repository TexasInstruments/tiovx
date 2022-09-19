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

#if defined(SOC_J721E) || defined(SOC_J721S2)
#define TIVX_TARGET_MCU2_MAX            (24U)
#elif defined(SOC_J784S4)
#define TIVX_TARGET_MCU2_MAX            (34U)
#endif
#define TIVX_TARGET_DEFAULT_STACK_SIZE  (16U*1024U)

#if defined(SAFERTOS)
#define TIVX_TARGET_DEFAULT_STACK_ALIGNMENT  TIVX_TARGET_DEFAULT_STACK_SIZE
#else
#define TIVX_TARGET_DEFAULT_STACK_ALIGNMENT  8192
#endif

static void tivxPlatformCreateTargetId(vx_enum target_id, uint32_t i, const char *name, uint32_t task_pri);
static void tivxPlatformDeleteTargetId(vx_enum target_id);

/**
 *******************************************************************************
 * \brief Target Stack
 *******************************************************************************
 */
static uint8_t gTarget_tskStack[TIVX_TARGET_MCU2_MAX][TIVX_TARGET_DEFAULT_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(TIVX_TARGET_DEFAULT_STACK_ALIGNMENT)))
    ;


static void tivxPlatformCreateTargetId(vx_enum target_id, uint32_t i, const char *name, uint32_t task_pri)
{
    vx_status status;
    tivx_target_create_params_t target_create_prms;

    if(tivxTargetGetCpuId(target_id) == tivxGetSelfCpuId() )
    {
        char target_name[TIVX_TARGET_MAX_NAME];

        tivxTargetSetDefaultCreateParams(&target_create_prms);

        target_create_prms.task_stack_ptr = gTarget_tskStack[i];
        target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
        target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
        target_create_prms.task_priority = task_pri;
        strncpy(target_create_prms.task_name, name,TIVX_TARGET_MAX_TASK_NAME);
        target_create_prms.task_name[TIVX_TARGET_MAX_TASK_NAME-1U] = (char)0;

        status = tivxTargetCreate(target_id, &target_create_prms);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Could not Add Target\n");
        }
        tivxPlatformGetTargetName(target_id, target_name);
        VX_PRINT(VX_ZONE_INIT, "Added target %s \n", target_name);
    }
}

static void tivxPlatformDeleteTargetId(vx_enum target_id)
{
    vx_status status;

    status = tivxTargetDelete(target_id);
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Could not Delete Target\n");
    }
}

void tivxPlatformCreateTargets(void)
{
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU2_0, 0, "TIVX_CPU_0", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_NF, 1, "TIVX_VPAC_NF", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_LDC1, 2, "TIVX_VPAC_LDC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC1, 3, "TIVX_VPAC_MSC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC2, 4, "TIVX_VPAC_MSC2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_SDE, 5, "TIVX_SDE", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_DOF, 6, "TIVX_DOF", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_VISS1, 7, "TIVX_VPAC_VISS1", 13u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE1, 8, "TIVX_CAPT1", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE2, 9, "TIVX_CAPT2", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY1, 10, "TIVX_DISP1", 14u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY2, 11, "TIVX_DISP2", 14u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CSITX, 12, "TIVX_CSITX", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE3, 13, "TIVX_CAPT3", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE4, 14, "TIVX_CAPT4", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE5, 15, "TIVX_CAPT5", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE6, 16, "TIVX_CAPT6", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE7, 17, "TIVX_CAPT7", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE8, 18, "TIVX_CAPT8", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M1, 19, "TIVX_DISP_M2M1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M2, 20, "TIVX_DISP_M2M2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M3, 21, "TIVX_DISP_M2M3", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M4, 22, "TIVX_DISP_M2M4", 8u);

    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU2_1, 0, "TIVX_CPU_1", 4u);

    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU3_0, 0, "TIVX_MCU3_0", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU3_1, 1, "TIVX_MCU3_1", 4u);

    #if defined(SOC_J784S4)
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_NF, 23, "TIVX_VPAC2_NF", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_LDC1, 24, "TIVX_VPAC2_LDC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_MSC1, 25, "TIVX_VPAC2_MSC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_MSC2, 26, "TIVX_VPAC2_MSC2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_VISS1, 27, "TIVX_VPAC2_VISS1", 13u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE9, 28, "TIVX_CAPT9", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE10, 29, "TIVX_CAPT10", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE11, 30, "TIVX_CAPT11", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE12, 31, "TIVX_CAPT12", 15u);

    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU4_0, 0, "TIVX_MCU4_0", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU4_1, 1, "TIVX_MCU4_1", 4u);
    #endif
}

void tivxPlatformDeleteTargets(void)
{
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU2_0);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_NF);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_LDC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_SDE);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_DOF);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_VISS1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE8);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CSITX);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU2_1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU3_0);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU3_1);
    #if defined(SOC_J784S4)
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU4_0);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU4_1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_NF);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_LDC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_MSC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_MSC2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC2_VISS1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE9);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE10);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE11);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE12);
    #endif
}
