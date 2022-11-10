/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_vision_sdk.h>

#define TIVX_TARGET_DEFAULT_STACK_SIZE  (64U*1024U)
#define TIVX_TARGET_DEFAULT_TASK_PRIORITY   (8u)

void ownPlatformCreateTargets(void)
{
    vx_status status;
    tivx_target_create_params_t target_create_prms;

    target_create_prms.task_stack_ptr = NULL;
    target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
    target_create_prms.task_priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY;

    status = ownTargetCreate(TIVX_TARGET_ID_A15_0, &target_create_prms);
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Could not Add Target\n");
    }
}

void ownPlatformDeleteTargets(void)
{
    vx_status status;

    status = ownTargetDelete(TIVX_TARGET_ID_A15_0);
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Could not Delete Target\n");
    }
}
