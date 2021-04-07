/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_psdk_j7.h>


#define TIVX_TARGET_MCU2_0_MAX          (23)
#define TIVX_TARGET_DEFAULT_STACK_SIZE  (16U*1024U)

static void tivxPlatformCreateTargetId(vx_enum target_id, uint32_t i, const char *name, uint32_t task_pri);
static void tivxPlatformDeleteTargetId(vx_enum target_id);

/**
 *******************************************************************************
 * \brief Target Stack
 *******************************************************************************
 */
static uint8_t gTarget_tskStack[TIVX_TARGET_MCU2_0_MAX][TIVX_TARGET_DEFAULT_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(8192)))
    ;


static void tivxPlatformCreateTargetId(vx_enum target_id, uint32_t i, const char *name, uint32_t task_pri)
{
    vx_status status;
    tivx_target_create_params_t target_create_prms;
    
    if(tivxTargetGetCpuId(target_id) == tivxGetSelfCpuId() )
    {
        char target_name[TIVX_TARGET_MAX_NAME];
        
        tivxTargetSetDefaultCreateParams(&target_create_prms);
    
        target_create_prms.task_stack_ptr = gTarget_tskStack[i];
        target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
        target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
        target_create_prms.task_priority = task_pri;
        strncpy(target_create_prms.task_name, name,TIVX_TARGET_MAX_TASK_NAME);
        target_create_prms.task_name[TIVX_TARGET_MAX_TASK_NAME-1U] = (char)0;
    
        status = tivxTargetCreate(target_id, &target_create_prms);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Could not Add Target\n");
        }
        tivxPlatformGetTargetName(target_id, target_name);
        VX_PRINT(VX_ZONE_INIT, "Added target %s \n", target_name);
    }
}

static void tivxPlatformDeleteTargetId(vx_enum target_id)
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
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_IPU1_0, 0, "TIVX_CPU", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_NF, 1, "TIVX_NF", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_LDC1, 2, "TIVX_LDC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC1, 3, "TIVX_MSC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC2, 4, "TIVX_MSC2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_SDE, 5, "TIVX_SDE", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_DOF, 6, "TIVX_DOF", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_VISS1, 7, "TIVX_VISS1", 13u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE1, 8, "TIVX_CAPT1", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE2, 9, "TIVX_CAPT2", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY1, 10, "TIVX_DISP1", 14u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY2, 11, "TIVX_DISP2", 14u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VDEC1, 12, "TIVX_VDEC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VDEC2, 13, "TIVX_VDEC2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VENC1, 14, "TIVX_VENC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VENC2, 15, "TIVX_VENC2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CSITX, 16, "TIVX_CSITX", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE3, 17, "TIVX_CAPT3", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE4, 18, "TIVX_CAPT4", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE5, 19, "TIVX_CAPT5", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE6, 20, "TIVX_CAPT6", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE7, 21, "TIVX_CAPT7", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE8, 22, "TIVX_CAPT8", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M, 23, "TIVX_DISP_M2M", 14u);
}

void tivxPlatformDeleteTargets(void)
{
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_IPU1_0);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_NF);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_LDC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_SDE);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_DOF);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_VISS1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE8);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VDEC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VDEC2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VENC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VENC2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CSITX);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY_M2M);
}
