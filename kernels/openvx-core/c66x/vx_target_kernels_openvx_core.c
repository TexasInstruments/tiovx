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
void tivxAddTargetKernelBitwise();
void tivxAddTargetKernelAdd();
void tivxAddTargetKernelSub();
void tivxAddTargetKernelThreshold();

void tivxRemoveTargetKernelAbsDiff();
void tivxRemoveTargetKernelLut();
void tivxRemoveTargetKernelBitwise();
void tivxRemoveTargetKernelAdd();
void tivxRemoveTargetKernelSub();
void tivxRemoveTargetKernelThreshold();

void tivxRegisterOpenVXCoreTargetKernels()
{
    tivxAddTargetKernelAbsDiff();
    tivxAddTargetKernelLut();
    tivxAddTargetKernelBitwise();
    tivxAddTargetKernelAdd();
    tivxAddTargetKernelSub();
    tivxAddTargetKernelThreshold();
}

void tivxUnRegisterOpenVXCoreTargetKernels()
{
    tivxRemoveTargetKernelAbsDiff();
    tivxRemoveTargetKernelLut();
    tivxRemoveTargetKernelBitwise();
    tivxRemoveTargetKernelAdd();
    tivxRemoveTargetKernelSub();
    tivxRemoveTargetKernelThreshold();
}
