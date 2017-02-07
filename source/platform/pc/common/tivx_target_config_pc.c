/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_pc.h>

#define TIVX_TARGET_DEFAULT_STACK_SIZE      (64U * 1024U)
#define TIVX_TARGET_DEFAULT_TASK_PRIORITY   (8u)


static void tivxTargetCreateTargetId(vx_enum target_id)
{
    vx_status status;
    tivx_target_create_params_t target_create_prms;

    target_create_prms.task_stack_ptr = NULL;
    target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
    target_create_prms.task_priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY;

    status = tivxTargetCreate(target_id, &target_create_prms);
    if (VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Cound not Add Target\n");
    }
}

static void tivxTargetDeleteTargetId(vx_enum target_id)
{
    vx_status status;

    status = tivxTargetDelete(target_id);
    if (VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Cound not Delete Target\n");
    }
}

void tivxPlatformCreateTargets(void)
{
    tivxTargetCreateTargetId(TIVX_TARGET_ID_CPU1);
    tivxTargetCreateTargetId(TIVX_TARGET_ID_CPU2);
    tivxTargetCreateTargetId(TIVX_TARGET_ID_CPU3);
    tivxTargetCreateTargetId(TIVX_TARGET_ID_CPU4);
}

void tivxPlatformDeleteTargets(void)
{
    tivxTargetDeleteTargetId(TIVX_TARGET_ID_CPU1);
    tivxTargetDeleteTargetId(TIVX_TARGET_ID_CPU2);
    tivxTargetDeleteTargetId(TIVX_TARGET_ID_CPU3);
    tivxTargetDeleteTargetId(TIVX_TARGET_ID_CPU4);
}
