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

vx_status tivxAbsDiff(tivx_target_kernel_instance kernel, uint32_t target_kernel_instance_handle, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxAbsDiffCreate(tivx_target_kernel_instance kernel, uint32_t target_kernel_instance_handle, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxAbsDiffDelete(tivx_target_kernel_instance kernel, uint32_t target_kernel_instance_handle, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxAbsDiffControl(tivx_target_kernel_instance kernel, uint32_t target_kernel_instance_handle, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params)
{
    vx_status status = VX_SUCCESS;

    return status;
}


void tivxAddTargetKernelAbsDiff()
{
    tivxAddTargetKernel(
                VX_KERNEL_ABSDIFF,
                TIVX_TARGET_DSP1,
                tivxAbsDiff,
                tivxAbsDiffCreate,
                tivxAbsDiffDelete,
                tivxAbsDiffControl
        );

    tivxAddTargetKernel(
                VX_KERNEL_ABSDIFF,
                TIVX_TARGET_DSP2,
                tivxAbsDiff,
                tivxAbsDiffCreate,
                tivxAbsDiffDelete,
                tivxAbsDiffControl
        );
}

