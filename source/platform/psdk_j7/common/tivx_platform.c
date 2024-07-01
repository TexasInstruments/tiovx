/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <tivx_platform_psdk.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>

const char *tivxPlatformGetEnv(const char *env_var);

uint64_t tivxPlatformGetTimeInUsecs(void)
{
    return appLogGetTimeInUsec();
}

void ownPlatformPrintf(const char *format)
{
    appLogPrintf(format);
}

void ownPlatformActivate(void)
{

}

void ownPlatformDeactivate(void)
{

}

const char *tivxPlatformGetEnv(const char *env_var)
{
    const char *value=" ";

    if(strcmp(env_var, "VX_TEST_DATA_PATH")==0)
    {
        value="/test_data/";
    }
    return (value);
}

#if defined(A72) || defined(A53)
#define TIVX_TARGET_DEFAULT_STACK_SIZE  (128U*1024U)
#elif defined(R5F)
#define TIVX_TARGET_DEFAULT_STACK_SIZE  (8U*1024U)
#elif defined(C7X_FAMILY) || defined(C66)
#define TIVX_TARGET_DEFAULT_STACK_SIZE  (64U*1024U)
#endif


#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)

#if defined(SAFERTOS)
#define TIVX_TARGET_DEFAULT_STACK_ALIGNMENT  TIVX_TARGET_DEFAULT_STACK_SIZE
#else
#define TIVX_TARGET_DEFAULT_STACK_ALIGNMENT  8192
#endif /* SAFERTOS */

#if defined(C7X_FAMILY)
#define TIVX_MAX_TARGET                        8
#elif defined(R5F)
#define TIVX_MAX_TARGET                        TIVX_TARGET_R5F_MAX
#elif defined(C66)
#define TIVX_MAX_TARGET                        1
#endif
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
static uint8_t gTarget_tskStack[TIVX_MAX_TARGET][TIVX_TARGET_DEFAULT_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(TIVX_TARGET_DEFAULT_STACK_ALIGNMENT)))
    ;
#endif /* #if defined(C7X_FAMILY) || defined(R5F) || defined(c66) */

void tivxPlatformCreateTargetId(vx_enum target_id, uint32_t i, const char *name, uint32_t task_pri)
{
    vx_status status;
    tivx_target_create_params_t target_create_prms;

    if(ownTargetGetCpuId(target_id) == tivxGetSelfCpuId() )
    {
        char target_name[TIVX_TARGET_MAX_NAME];

        ownTargetSetDefaultCreateParams(&target_create_prms);

        #if defined(A72) || defined(A53)
        target_create_prms.task_stack_ptr = NULL;
        #else
        target_create_prms.task_stack_ptr = gTarget_tskStack[i];
        #endif
        target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
        target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
        target_create_prms.task_priority = task_pri;
        (void)strncpy(target_create_prms.task_name, name,TIVX_TARGET_MAX_TASK_NAME);
        target_create_prms.task_name[TIVX_TARGET_MAX_TASK_NAME-1U] = (char)0;

        status = ownTargetCreate(target_id, &target_create_prms);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Could not Add Target\n");
        }
        else
        {
            ownPlatformGetTargetName(target_id, target_name);
            VX_PRINT(VX_ZONE_INIT, "Added target %s \n", target_name);
        }
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
