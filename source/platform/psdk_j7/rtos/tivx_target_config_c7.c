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

#define TIVX_TARGET_C7_MAX                  (8U)

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
static uint8_t gTarget_tskStack[TIVX_TARGET_C7_MAX][TIVX_TARGET_DEFAULT_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection:tiovx")))
__attribute__ ((aligned(8192)))
    ;

static void tivxPlatformCreateTargetId(vx_enum target_id, uint32_t i, const char *name, uint32_t task_pri)
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

static void tivxPlatformDeleteTargetId(vx_enum target_id)
{
    vx_status status;

    status = ownTargetDelete(target_id);
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Could not Delete Target\n");
    }
}

void ownPlatformCreateTargets(void)
{
    /*
     * Note: All CPU tasks should be at a lower priority than APP_IPC_RPMESSAGE_RX_TASK_PRI, otherwise
     *       new messages would be starved from getting enqueued into the various priority
     *       worker threads.
     * */

    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1,       0, "TIVX_C71_P1", 9u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_2, 1, "TIVX_C71_P2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_3, 2, "TIVX_C71_P3", 7u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_4, 3, "TIVX_C71_P4", 6u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_5, 4, "TIVX_C71_P5", 5u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_6, 5, "TIVX_C71_P6", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_7, 6, "TIVX_C71_P7", 3u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_8, 7, "TIVX_C71_P8", 2u);
    #if defined(SOC_J721S2)
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP1,           0, "TIVX_CPU",       8u);
    #endif
    #if defined(SOC_J784S4)
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2,       0, "TIVX_C72_P1", 9u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_2, 1, "TIVX_C72_P2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_3, 2, "TIVX_C72_P3", 7u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_4, 3, "TIVX_C72_P4", 6u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_5, 4, "TIVX_C72_P5", 5u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_6, 5, "TIVX_C72_P6", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_7, 6, "TIVX_C72_P7", 3u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_8, 7, "TIVX_C72_P8", 2u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3,       0, "TIVX_C73_P1", 9u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_2, 1, "TIVX_C73_P2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_3, 2, "TIVX_C73_P3", 7u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_4, 3, "TIVX_C73_P4", 6u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_5, 4, "TIVX_C73_P5", 5u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_6, 5, "TIVX_C73_P6", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_7, 6, "TIVX_C73_P7", 3u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_8, 7, "TIVX_C73_P8", 2u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4,       0, "TIVX_C74_P1", 9u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_2, 1, "TIVX_C74_P2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_3, 2, "TIVX_C74_P3", 7u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_4, 3, "TIVX_C74_P4", 6u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_5, 4, "TIVX_C74_P5", 5u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_6, 5, "TIVX_C74_P6", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_7, 6, "TIVX_C74_P7", 3u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_8, 7, "TIVX_C74_P8", 2u);
    #endif
}

void ownPlatformDeleteTargets(void)
{
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_8);
    #if defined(SOC_J721S2)
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP1);
    #endif
    #if defined(SOC_J784S4)
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_8);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_8);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_8);
    #endif
}
