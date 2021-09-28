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

#define TIVX_TARGET_DEFAULT_STACK_SIZE  (64U*1024U)

static void tivxTargetCreateTargetId(vx_enum target_id, const char *name);
static void tivxTargetDeleteTargetId(vx_enum target_id);

static void tivxTargetCreateTargetId(vx_enum target_id, const char *name)
{
    vx_status status;
    tivx_target_create_params_t target_create_prms;

    tivxTargetSetDefaultCreateParams(&target_create_prms);

    target_create_prms.task_stack_ptr = NULL;
    target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
    target_create_prms.task_priority = 8U;
    strncpy(target_create_prms.task_name, name, TIVX_TARGET_MAX_TASK_NAME);
    target_create_prms.task_name[TIVX_TARGET_MAX_TASK_NAME-1U] = (char)0;

    status = tivxTargetCreate(target_id, &target_create_prms);
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Could not Add Target %s\n", name);
    }
}

static void tivxTargetDeleteTargetId(vx_enum target_id)
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
    tivxTargetCreateTargetId((vx_enum)TIVX_TARGET_ID_A72_0, "TIVX_CPU0");
    tivxTargetCreateTargetId((vx_enum)TIVX_TARGET_ID_A72_1, "TIVX_CPU1");
    tivxTargetCreateTargetId((vx_enum)TIVX_TARGET_ID_A72_2, "TIVX_CPU2");
    tivxTargetCreateTargetId((vx_enum)TIVX_TARGET_ID_A72_3, "TIVX_CPU3");
}

void tivxPlatformDeleteTargets(void)
{
    tivxTargetDeleteTargetId((vx_enum)TIVX_TARGET_ID_A72_0);
    tivxTargetDeleteTargetId((vx_enum)TIVX_TARGET_ID_A72_1);
    tivxTargetDeleteTargetId((vx_enum)TIVX_TARGET_ID_A72_2);
    tivxTargetDeleteTargetId((vx_enum)TIVX_TARGET_ID_A72_3);
}

