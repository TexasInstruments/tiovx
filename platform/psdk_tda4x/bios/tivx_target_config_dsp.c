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

#define TIVX_TARGET_DEFAULT_STACK_SIZE      (64U * 1024U)

#define TIVX_TARGET_DEFAULT_TASK_PRIORITY   (8u)

/**
 *******************************************************************************
 * \brief Target Stack
 *******************************************************************************
 */
/* IMPORTANT NOTE: For C7x, 
 * - stack size and stack ptr MUST be 8KB aligned 
 * - AND min stack size MUST be 16KB 
 * - AND stack assigned for task context is "size - 8KB"
 *       - 8KB chunk for the stack area is used for interrupt handling in this task context
 */
static uint8_t gTarget_tskStack[TIVX_TARGET_DEFAULT_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(8192)))
    ;


void tivxPlatformCreateTargets(void)
{
    vx_status status = VX_SUCCESS;
    tivx_target_create_params_t target_create_prms;
    vx_enum self_cpu, target_id;

    self_cpu = tivxGetSelfCpuId();

    target_create_prms.task_stack_ptr = gTarget_tskStack;
    target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
    target_create_prms.task_priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY;

    switch (self_cpu)
    {
        case TIVX_CPU_ID_DSP1:
            target_id = TIVX_TARGET_ID_DSP1;
            break;
        case TIVX_CPU_ID_DSP2:
            target_id = TIVX_TARGET_ID_DSP2;
            break;
        /* Note: this is repurposed to be C7X */
        case TIVX_CPU_ID_EVE1:
            target_id = TIVX_TARGET_ID_EVE1;
            break;
        default:
            VX_PRINT(VX_ZONE_ERROR, "Incorrect CPU\n");
            status = VX_FAILURE;
            break;
    }

    if (VX_SUCCESS == status)
    {
        status = tivxTargetCreate(target_id, &target_create_prms);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Could not Add Target\n");
        }
    }
}

void tivxPlatformDeleteTargets(void)
{
    vx_status status = VX_SUCCESS;
    vx_enum self_cpu, target_id;

    self_cpu = tivxGetSelfCpuId();

    switch (self_cpu)
    {
        case TIVX_CPU_ID_DSP1:
            target_id = TIVX_TARGET_ID_DSP1;
            break;
        case TIVX_CPU_ID_DSP2:
            target_id = TIVX_TARGET_ID_DSP2;
            break;
        /* Note: this is repurposed to be C7X */
        case TIVX_CPU_ID_EVE1:
            target_id = TIVX_TARGET_ID_EVE1;
            break;
        default:
            VX_PRINT(VX_ZONE_ERROR, "Incorrect CPU\n");
            status = VX_FAILURE;
            break;
    }

    if (VX_SUCCESS == status)
    {
        status = tivxTargetDelete(target_id);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Could not Delete Target\n");
        }
    }
}
