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

#define TIVX_TARGET_DEFAULT_STACK_SIZE  (64U*1024U)

/**
 *******************************************************************************
 * \brief Target Stack
 *******************************************************************************
 */
static uint8_t gTarget_tskStack[TIVX_TARGET_DEFAULT_STACK_SIZE]
    __attribute__ ((aligned(4096)))
    ;


void tivxPlatformCreateTargets(void)
{
    vx_status status;
    tivx_target_create_params_t target_create_prms;

    target_create_prms.task_stack_ptr = gTarget_tskStack;
    target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
    target_create_prms.task_priority = 8U;

    status = tivxTargetCreate(TIVX_TARGET_ID_IPU1_0, &target_create_prms);
    if (VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Could not Add Target\n");
    }
}

void tivxPlatformDeleteTargets(void)
{
    vx_status status;

    status = tivxTargetDelete(TIVX_TARGET_ID_IPU1_0);
    if (VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Could not Delete Target\n");
    }
}
