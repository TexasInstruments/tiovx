/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
#include <TI/tivx_target_kernel.h>

static tivx_target_kernel vx_absdiff_target_kernel = NULL;

vx_status tivxAbsDiff(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxAbsDiffCreate(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxAbsDiffDelete(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxAbsDiffControl(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params)
{
    vx_status status = VX_SUCCESS;

    return status;
}


void tivxAddTargetKernelAbsDiff()
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_DSP1) || (self_cpu == TIVX_CPU_ID_DSP2))
    {
        if (self_cpu == TIVX_CPU_ID_DSP1)
        {
            strncpy(target_name, TIVX_TARGET_DSP1,
                TIVX_TARGET_MAX_NAME);
        }
        else
        {
            strncpy(target_name, TIVX_TARGET_DSP2,
                TIVX_TARGET_MAX_NAME);
        }

        vx_absdiff_target_kernel = tivxAddTargetKernel(
                    VX_KERNEL_ABSDIFF,
                    target_name,
                    tivxAbsDiff,
                    tivxAbsDiffCreate,
                    tivxAbsDiffDelete,
                    tivxAbsDiffControl);
    }
}

void tivxRemoveTargetKernelAbsDiff()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_absdiff_target_kernel);

    if (VX_SUCCESS == status)
    {
        vx_absdiff_target_kernel = NULL;
    }
}

