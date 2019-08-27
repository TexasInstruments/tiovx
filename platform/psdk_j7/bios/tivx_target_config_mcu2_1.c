/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_psdk_j7.h>


#define TIVX_TARGET_MCU2_1_MAX          (14)
#define TIVX_TARGET_DEFAULT_STACK_SIZE  (16U*1024U)

/**
 *******************************************************************************
 * \brief Target Stack
 *******************************************************************************
 */
static uint8_t gTarget_tskStack[TIVX_TARGET_MCU2_1_MAX][TIVX_TARGET_DEFAULT_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(8192)))
    ;


void tivxPlatformCreateTargetId(vx_enum target_id, uint32_t i, char *name)
{
    vx_status status;
    tivx_target_create_params_t target_create_prms;

    tivxTargetSetDefaultCreateParams(&target_create_prms);

    target_create_prms.task_stack_ptr = gTarget_tskStack[i];
    target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
    target_create_prms.task_priority = 8U;
    strncpy(target_create_prms.task_name, name,TIVX_TARGET_MAX_TASK_NAME);
    target_create_prms.task_name[TIVX_TARGET_MAX_TASK_NAME-1] = 0;

    status = tivxTargetCreate(target_id, &target_create_prms);
    if (VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Could not Add Target\n");
    }
}

void tivxPlatformDeleteTargetId(vx_enum target_id)
{
    vx_status status;

    status = tivxTargetDelete(target_id);
    if (VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Could not Delete Target\n");
    }
}

void tivxPlatformCreateTargets(void)
{
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_IPU1_0, 0, "TIVX_CPU");
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_VPAC_NF, 1, "TIVX_NF");
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_VPAC_LDC1, 2, "TIVX_LDC1");
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_VPAC_MSC1, 3, "TIVX_MSC1");
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_VPAC_MSC2, 4, "TIVX_MSC2");
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_DMPAC_SDE, 5, "TIVX_SDE");
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_DMPAC_DOF, 6, "TIVX_DOF");
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_VPAC_VISS1, 7, "TIVX_VISS1");
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_CAPTURE1, 8, "TIVX_CAPT1");
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_CAPTURE2, 9, "TIVX_CAPT2");
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_DISPLAY1, 10, "TIVX_DISP1");
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_DISPLAY2, 11, "TIVX_DISP2");
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_VDEC1, 12, "TIVX_VDEC1");
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_VDEC2, 13, "TIVX_VDEC2");
}

void tivxPlatformDeleteTargets(void)
{
    tivxPlatformDeleteTargetId(TIVX_TARGET_ID_IPU1_0);
    tivxPlatformDeleteTargetId(TIVX_TARGET_ID_VPAC_NF);
    tivxPlatformDeleteTargetId(TIVX_TARGET_ID_VPAC_LDC1);
    tivxPlatformDeleteTargetId(TIVX_TARGET_ID_VPAC_MSC1);
    tivxPlatformDeleteTargetId(TIVX_TARGET_ID_VPAC_MSC2);
    tivxPlatformDeleteTargetId(TIVX_TARGET_ID_DMPAC_SDE);
    tivxPlatformDeleteTargetId(TIVX_TARGET_ID_DMPAC_DOF);
    tivxPlatformDeleteTargetId(TIVX_TARGET_ID_VPAC_VISS1);
    tivxPlatformDeleteTargetId(TIVX_TARGET_ID_CAPTURE1);
    tivxPlatformDeleteTargetId(TIVX_TARGET_ID_CAPTURE2);
    tivxPlatformDeleteTargetId(TIVX_TARGET_ID_DISPLAY1);
    tivxPlatformDeleteTargetId(TIVX_TARGET_ID_DISPLAY2);
    tivxPlatformDeleteTargetId(TIVX_TARGET_ID_VDEC1);
    tivxPlatformDeleteTargetId(TIVX_TARGET_ID_VDEC2);
}
