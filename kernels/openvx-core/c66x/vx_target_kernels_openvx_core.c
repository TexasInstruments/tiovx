/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <TI/tivx_target_kernel.h>

vx_status tivxAddTargetKernelAbsDiff();
vx_status tivxRemoveTargetKernelAbsDiff();


void tivxRegisterOpenVXCoreTargetKernels()
{
    vx_status status = VX_SUCCESS;

    if(status == VX_SUCCESS)
    {
        status = tivxAddTargetKernelAbsDiff();
    }
}

void tivxUnRegisterOpenVXCoreTargetKernels()
{
    tivxRemoveTargetKernelAbsDiff();
}
