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
__attribute__ ((section(".bss:taskStackSection:tiovx")))
__attribute__ ((aligned(8192)))
    ;


void ownPlatformCreateTargets(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_target_create_params_t target_create_prms;
    vx_enum self_cpu, target_id;

    self_cpu = tivxGetSelfCpuId();

    ownTargetSetDefaultCreateParams(&target_create_prms);

    target_create_prms.task_stack_ptr = gTarget_tskStack;
    target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
    target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
    target_create_prms.task_priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY;
    strncpy(target_create_prms.task_name, "TIVX_CPU",TIVX_TARGET_MAX_TASK_NAME);
    target_create_prms.task_name[TIVX_TARGET_MAX_TASK_NAME-1U] = (char)0;

    switch (self_cpu)
    {
        case (vx_enum)TIVX_CPU_ID_DSP1:
            target_id = (vx_enum)TIVX_TARGET_ID_DSP1;
            break;
        case (vx_enum)TIVX_CPU_ID_DSP2:
            target_id = (vx_enum)TIVX_TARGET_ID_DSP2;
            break;
        default:
            VX_PRINT(VX_ZONE_ERROR, "Incorrect CPU\n");
            status = (vx_status)VX_FAILURE;
            break;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = ownTargetCreate(target_id, &target_create_prms);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Could not Add Target\n");
        }
    }
}

void ownPlatformDeleteTargets(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum self_cpu, target_id;

    self_cpu = tivxGetSelfCpuId();

    switch (self_cpu)
    {
        case (vx_enum)TIVX_CPU_ID_DSP1:
            target_id = (vx_enum)TIVX_TARGET_ID_DSP1;
            break;
        case (vx_enum)TIVX_CPU_ID_DSP2:
            target_id = (vx_enum)TIVX_TARGET_ID_DSP2;
            break;
        default:
            VX_PRINT(VX_ZONE_ERROR, "Incorrect CPU\n");
            status = (vx_status)VX_FAILURE;
            break;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = ownTargetDelete(target_id);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Could not Delete Target\n");
        }
    }
}
