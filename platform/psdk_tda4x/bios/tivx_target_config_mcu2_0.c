/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_psdk_tda4x.h>


#define TIVX_TARGET_MCU2_0_MAX          (8)
#define TIVX_TARGET_DEFAULT_STACK_SIZE  (16U*1024U)

/**
 *******************************************************************************
 * \brief Target Stack
 *******************************************************************************
 */
static uint8_t gTarget_tskStack[TIVX_TARGET_MCU2_0_MAX][TIVX_TARGET_DEFAULT_STACK_SIZE]
    __attribute__ ((aligned(4096)))
    ;


void tivxPlatformCreateTargetId(vx_enum target_id, uint32_t i)
{
    vx_status status;
    tivx_target_create_params_t target_create_prms;

    target_create_prms.task_stack_ptr = gTarget_tskStack[i];
    target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
    target_create_prms.task_priority = 8U;

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
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_IPU1_0, 0);
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_VPAC_NF, 1);
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_VPAC_LDC1, 2);
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_VPAC_MSC1, 3);
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_VPAC_MSC2, 4);
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_DMPAC_SDE, 5);
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_DMPAC_DOF, 6);
    tivxPlatformCreateTargetId(TIVX_TARGET_ID_VPAC_VISS1, 7);
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
}
