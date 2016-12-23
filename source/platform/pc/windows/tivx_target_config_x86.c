/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_windows.h>

#define TIVX_TARGET_DEFAULT_STACK_SIZE      (64U * 1024U)
#define TIVX_TARGET_DEFAULT_TASK_PRIORITY   (8u)


static void tivxTargetConfigTargetId(vx_enum target_id)
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
        VX_PRINT(VX_ZONE_ERROR, "Cound not Add new Target\n");
    }
}

void tivxTargetConfig()
{
    tivxTargetConfigTargetId(TIVX_TARGET_ID_CPU1);
    tivxTargetConfigTargetId(TIVX_TARGET_ID_CPU2);
    tivxTargetConfigTargetId(TIVX_TARGET_ID_CPU3);
    tivxTargetConfigTargetId(TIVX_TARGET_ID_CPU4);
}
