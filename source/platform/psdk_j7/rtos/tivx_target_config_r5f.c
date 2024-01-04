/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <tivx_target_config_r5f.h>

#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J722S)
#define TIVX_TARGET_R5F_MAX            (24U)
#elif defined(SOC_J784S4)
#define TIVX_TARGET_R5F_MAX            (26U)
#elif defined(SOC_AM62A)
#define TIVX_TARGET_R5F_MAX            (5U)
#endif
#define TIVX_TARGET_DEFAULT_STACK_SIZE  (16U*1024U)

#if defined(SAFERTOS)
#define TIVX_TARGET_DEFAULT_STACK_ALIGNMENT  TIVX_TARGET_DEFAULT_STACK_SIZE
#else
#define TIVX_TARGET_DEFAULT_STACK_ALIGNMENT  8192
#endif

/**
 *******************************************************************************
 * \brief Target Stack
 *******************************************************************************
 */
static uint8_t gTarget_tskStack[TIVX_TARGET_R5F_MAX][TIVX_TARGET_DEFAULT_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(TIVX_TARGET_DEFAULT_STACK_ALIGNMENT)))
    ;

void tivxPlatformCreateTargetId(vx_enum target_id, uint32_t i, const char *name, uint32_t task_pri)
{
    vx_status status;
    tivx_target_create_params_t target_create_prms;

    if(ownTargetGetCpuId(target_id) == tivxGetSelfCpuId() )
    {
        char target_name[TIVX_TARGET_MAX_NAME];

        ownTargetSetDefaultCreateParams(&target_create_prms);

        target_create_prms.task_stack_ptr = gTarget_tskStack[i];
        target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
        target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
        target_create_prms.task_priority = task_pri;
        strncpy(target_create_prms.task_name, name,TIVX_TARGET_MAX_TASK_NAME);
        target_create_prms.task_name[TIVX_TARGET_MAX_TASK_NAME-1U] = (char)0;

        status = ownTargetCreate(target_id, &target_create_prms);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Could not Add Target\n");
        }
        ownPlatformGetTargetName(target_id, target_name);
        VX_PRINT(VX_ZONE_INIT, "Added target %s \n", target_name);
    }
}

void tivxPlatformDeleteTargetId(vx_enum target_id)
{
    vx_status status;

    status = ownTargetDelete(target_id);
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Could not Delete Target\n");
    }
}
