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

/**
 *******************************************************************************
 * \brief Target Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gTarget_tskStack, 32)
#pragma DATA_SECTION(gTarget_tskStack, ".bss:taskStackSection")
uint8_t gTarget_tskStack[TIVX_TARGET_DEFAULT_STACK_SIZE];


void tivxTargetConfig(void)
{
    vx_status status;
    tivx_target_create_params_t target_create_prms;

    target_create_prms.task_stack_ptr = gTarget_tskStack;
    target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
    target_create_prms.task_priority = 8U;

    status = tivxTargetCreate(TIVX_TARGET_ID_DSP1, &target_create_prms);
    if (VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Cound not Add new Target\n");
    }
}
