/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <TI/tivx_target_kernel.h>

typedef void (*tivxTargetKernel_Fxn) ();

typedef struct  {
    tivxTargetKernel_Fxn    add_kernel;
    tivxTargetKernel_Fxn    remove_kernel;
} Tivx_Target_Kernel_List;

void tivxAddTargetKernelAbsDiff();
void tivxAddTargetKernelLut();
void tivxAddTargetKernelBitwise();
void tivxAddTargetKernelAdd();
void tivxAddTargetKernelSub();
void tivxAddTargetKernelThreshold();
void tivxAddTargetKernelErode3x3();
void tivxAddTargetKernelMultiply();
void tivxAddTargetKernelConvolve();
void tivxAddTargetKernelWarpAffine();
void tivxAddTargetKernelWarpPerspective();
void tivxAddTargetKernelScale();
void tivxAddTargetKernelAccumulate();
void tivxAddTargetKernelAccumulateSquare();
void tivxAddTargetKernelAccumulateWeighted();
void tivxAddTargetKernelRemap();
void tivxAddTargetKernelIntegralImage();
void tivxAddTargetKernelSobel3x3();
void tivxAddTargetKernelPhase();
void tivxAddTargetKernelMagnitude();
void tivxAddTargetKernelConvertDepth();

void tivxRemoveTargetKernelAbsDiff();
void tivxRemoveTargetKernelLut();
void tivxRemoveTargetKernelBitwise();
void tivxRemoveTargetKernelAdd();
void tivxRemoveTargetKernelSub();
void tivxRemoveTargetKernelThreshold();
void tivxRemoveTargetKernelErode3x3();
void tivxRemoveTargetKernelMultiply();
void tivxRemoveTargetKernelConvolve();
void tivxRemoveTargetKernelWarpAffine();
void tivxRemoveTargetKernelWarpPerspective();
void tivxRemoveTargetKernelScale();
void tivxRemoveTargetKernelAccumulate();
void tivxRemoveTargetKernelAccumulateSquare();
void tivxRemoveTargetKernelAccumulateWeighted();
void tivxRemoveTargetKernelRemap();
void tivxRemoveTargetKernelIntegralImage();
void tivxRemoveTargetKernelSobel3x3();
void tivxRemoveTargetKernelPhase();
void tivxRemoveTargetKernelMagnitude();
void tivxRemoveTargetKernelConvertDepth();

Tivx_Target_Kernel_List gTivx_target_kernel_list[] = {
    {tivxAddTargetKernelAbsDiff, tivxRemoveTargetKernelAbsDiff},
    {tivxAddTargetKernelLut, tivxRemoveTargetKernelLut},
    {tivxAddTargetKernelBitwise, tivxRemoveTargetKernelBitwise},
    {tivxAddTargetKernelAdd, tivxRemoveTargetKernelAdd},
    {tivxAddTargetKernelSub, tivxRemoveTargetKernelSub},
    {tivxAddTargetKernelThreshold, tivxRemoveTargetKernelThreshold},
    {tivxAddTargetKernelErode3x3, tivxRemoveTargetKernelErode3x3},
    {tivxAddTargetKernelMultiply, tivxRemoveTargetKernelMultiply},
    {tivxAddTargetKernelConvolve, tivxRemoveTargetKernelConvolve},
    {tivxAddTargetKernelWarpAffine, tivxRemoveTargetKernelWarpAffine},
    {tivxAddTargetKernelWarpPerspective, tivxRemoveTargetKernelWarpPerspective},
    {tivxAddTargetKernelScale, tivxRemoveTargetKernelScale},
    {tivxAddTargetKernelAccumulate, tivxRemoveTargetKernelAccumulate},
    {tivxAddTargetKernelAccumulateSquare, tivxRemoveTargetKernelAccumulateSquare},
    {tivxAddTargetKernelAccumulateWeighted, tivxRemoveTargetKernelAccumulateWeighted},
    {tivxAddTargetKernelRemap, tivxRemoveTargetKernelRemap},
    {tivxAddTargetKernelIntegralImage, tivxRemoveTargetKernelIntegralImage},
    {tivxAddTargetKernelSobel3x3, tivxRemoveTargetKernelSobel3x3},
    {tivxAddTargetKernelPhase, tivxRemoveTargetKernelPhase},
    {tivxAddTargetKernelMagnitude, tivxRemoveTargetKernelMagnitude},
    {tivxAddTargetKernelConvertDepth, tivxRemoveTargetKernelConvertDepth}
};

void tivxRegisterOpenVXCoreTargetKernels()
{
    vx_uint32 i;

    for (i = 0; i <
        sizeof(gTivx_target_kernel_list)/sizeof(Tivx_Target_Kernel_List); i ++)
    {
        if (gTivx_target_kernel_list[i].add_kernel)
        {
            gTivx_target_kernel_list[i].add_kernel();
        }
    }
}

void tivxUnRegisterOpenVXCoreTargetKernels()
{
    vx_uint32 i;

    for (i = 0; i <
        sizeof(gTivx_target_kernel_list)/sizeof(Tivx_Target_Kernel_List); i ++)
    {
        if (gTivx_target_kernel_list[i].remove_kernel)
        {
            gTivx_target_kernel_list[i].remove_kernel();
        }
    }
}
