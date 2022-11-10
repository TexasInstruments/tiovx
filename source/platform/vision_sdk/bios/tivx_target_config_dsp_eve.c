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

#define TIVX_TARGET_DEFAULT_DSP_STACK_SIZE      (64U * 1024U)
#define TIVX_TARGET_DEFAULT_EVE_STACK_SIZE      (8U * 1024U)

#define TIVX_TARGET_DEFAULT_TASK_PRIORITY   (8u)

/**
 *******************************************************************************
 * \brief Target Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gTarget_dspTskStack, 32)
#pragma DATA_SECTION(gTarget_dspTskStack, ".bss:taskStackSection")
uint8_t gTarget_dspTskStack[TIVX_TARGET_DEFAULT_DSP_STACK_SIZE];

#pragma DATA_ALIGN(gTarget_eveTskStack, 32)
#pragma DATA_SECTION(gTarget_eveTskStack, ".bss:taskStackSection:tiovx")
uint8_t gTarget_eveTskStack[TIVX_TARGET_DEFAULT_EVE_STACK_SIZE];

void ownPlatformCreateTargets(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_target_create_params_t target_create_prms;
    vx_enum self_cpu, target_id;

    self_cpu = tivxGetSelfCpuId();

    target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
    target_create_prms.task_priority = TIVX_TARGET_DEFAULT_TASK_PRIORITY;

    switch (self_cpu)
    {
        case (vx_enum)TIVX_CPU_ID_DSP1:
            target_id = TIVX_TARGET_ID_DSP1;
            target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_DSP_STACK_SIZE;
            target_create_prms.task_stack_ptr = gTarget_dspTskStack;
            break;
        case (vx_enum)TIVX_CPU_ID_DSP2:
            target_id = TIVX_TARGET_ID_DSP2;
            target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_DSP_STACK_SIZE;
            target_create_prms.task_stack_ptr = gTarget_dspTskStack;
            break;
        case (vx_enum)TIVX_CPU_ID_EVE1:
            target_id = TIVX_TARGET_ID_EVE1;
            target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_EVE_STACK_SIZE;
            target_create_prms.task_stack_ptr = gTarget_eveTskStack;
            break;
        case (vx_enum)TIVX_CPU_ID_EVE2:
            target_id = TIVX_TARGET_ID_EVE2;
            target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_EVE_STACK_SIZE;
            target_create_prms.task_stack_ptr = gTarget_eveTskStack;
            break;
        case (vx_enum)TIVX_CPU_ID_EVE3:
            target_id = TIVX_TARGET_ID_EVE3;
            target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_EVE_STACK_SIZE;
            target_create_prms.task_stack_ptr = gTarget_eveTskStack;
            break;
        case (vx_enum)TIVX_CPU_ID_EVE4:
            target_id = TIVX_TARGET_ID_EVE4;
            target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_EVE_STACK_SIZE;
            target_create_prms.task_stack_ptr = gTarget_eveTskStack;
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
            target_id = TIVX_TARGET_ID_DSP1;
            break;
        case (vx_enum)TIVX_CPU_ID_DSP2:
            target_id = TIVX_TARGET_ID_DSP2;
            break;
        case (vx_enum)TIVX_CPU_ID_EVE1:
            target_id = TIVX_TARGET_ID_EVE1;
            break;
        case (vx_enum)TIVX_CPU_ID_EVE2:
            target_id = TIVX_TARGET_ID_EVE2;
            break;
        case (vx_enum)TIVX_CPU_ID_EVE3:
            target_id = TIVX_TARGET_ID_EVE3;
            break;
        case (vx_enum)TIVX_CPU_ID_EVE4:
            target_id = TIVX_TARGET_ID_EVE4;
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
