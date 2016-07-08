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

static tivx_target_kernel vx_absdiff_target_kernel;


vx_status tivxAbsDiff(tivx_target_kernel kernel, tivx_obj_desc_t *node_desc)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxAbsDiffCreate(tivx_target_kernel kernel, tivx_obj_desc_t *node_desc)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxAbsDiffDelete(tivx_target_kernel kernel, tivx_obj_desc_t *node_desc)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxAbsDiffControl(tivx_target_kernel kernel, tivx_obj_desc_t *node_desc)
{
    vx_status status = VX_SUCCESS;

    return status;
}


vx_status tivxAddTargetKernelAbsDiff()
{
    vx_status status;
    tivx_target_kernel kernel;

    kernel = tivxAddTargetKernel(
                            VX_KERNEL_ABSDIFF,
                            tivxAbsDiff,
                            tivxAbsDiffCreate,
                            tivxAbsDiffDelete,
                            tivxAbsDiffControl
                          );

    if(kernel!=NULL)
    {
        status = tivxAddTargetKernelTarget(kernel, TIVX_TARGET_DSP);
        if(status != VX_SUCCESS)
        {
            /* target does not match actual target's on this CPU */
            tivxRemoveTargetKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        status = VX_FAILURE;
    }

    vx_absdiff_target_kernel = kernel;

    return status;
}

vx_status tivxRemoveTargetKernelAbsDiff()
{
    vx_status status;
    tivx_target_kernel kernel = vx_absdiff_target_kernel;

    status = tivxRemoveTargetKernel(&kernel);

    return status;
}
