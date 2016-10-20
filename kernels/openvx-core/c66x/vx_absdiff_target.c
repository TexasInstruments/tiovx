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

static tivx_target_kernel vx_absdiff_target_kernel_dsp1 = NULL;
static tivx_target_kernel vx_absdiff_target_kernel_dsp2 = NULL;

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
    vx_absdiff_target_kernel_dsp1 = tivxAddTargetKernel(
                VX_KERNEL_ABSDIFF,
                TIVX_TARGET_DSP1,
                tivxAbsDiff,
                tivxAbsDiffCreate,
                tivxAbsDiffDelete,
                tivxAbsDiffControl
        );

    vx_absdiff_target_kernel_dsp2 = tivxAddTargetKernel(
                VX_KERNEL_ABSDIFF,
                TIVX_TARGET_DSP2,
                tivxAbsDiff,
                tivxAbsDiffCreate,
                tivxAbsDiffDelete,
                tivxAbsDiffControl
        );
}

void tivxRemoveTargetKernelAbsDiff()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_absdiff_target_kernel_dsp1);

    if (VX_SUCCESS == status)
    {
        vx_absdiff_target_kernel_dsp1 = NULL;
    }


    status = tivxRemoveTargetKernel(vx_absdiff_target_kernel_dsp2);

    if (VX_SUCCESS == status)
    {
        vx_absdiff_target_kernel_dsp2 = NULL;
    }
}

