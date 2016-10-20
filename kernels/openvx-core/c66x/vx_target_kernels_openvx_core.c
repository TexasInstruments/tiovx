/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <TI/tivx_target_kernel.h>

void tivxAddTargetKernelAbsDiff();
void tivxAddTargetKernelLut();

void tivxRemoveTargetKernelAbsDiff();
void tivxRemoveTargetKernelLut();


void tivxRegisterOpenVXCoreTargetKernels()
{
    tivxAddTargetKernelAbsDiff();
    tivxAddTargetKernelLut();
}

void tivxUnRegisterOpenVXCoreTargetKernels()
{
    tivxRemoveTargetKernelAbsDiff();
    tivxRemoveTargetKernelLut();
}
