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
void tivxAddTargetKernelAccumulate();
void tivxAddTargetKernelAccumulateSquare();
void tivxAddTargetKernelAccumulateWeighted();
void tivxAddTargetKernelAdd();
void tivxAddTargetKernelBitwise();
void tivxAddTargetKernelChannelCombine();
void tivxAddTargetKernelChannelExtract();
void tivxAddTargetKernelColorConvert();
void tivxAddTargetKernelConvertDepth();
void tivxAddTargetKernelConvolve();
void tivxAddTargetKernelEqualizeHistogram();
void tivxAddTargetKernelErode3x3();
void tivxAddTargetKernelFastCorners();
void tivxAddTargetKernelGaussianPyramid();
void tivxAddTargetKernelHarrisCorners();
void tivxAddTargetKernelHalfscaleGaussian();
void tivxAddTargetKernelHistogram();
void tivxAddTargetKernelIntegralImage();
void tivxAddTargetKernelLut();
void tivxAddTargetKernelMagnitude();
void tivxAddTargetKernelMeanStdDev();
void tivxAddTargetKernelMinMaxLoc();
void tivxAddTargetKernelMultiply();
void tivxAddTargetKernelPhase();
void tivxAddTargetKernelRemap();
void tivxAddTargetKernelScale();
void tivxAddTargetKernelSobel3x3();
void tivxAddTargetKernelSub();
void tivxAddTargetKernelThreshold();
void tivxAddTargetKernelWarpAffine();
void tivxAddTargetKernelWarpPerspective();

void tivxRemoveTargetKernelAbsDiff();
void tivxRemoveTargetKernelAccumulate();
void tivxRemoveTargetKernelAccumulateSquare();
void tivxRemoveTargetKernelAccumulateWeighted();
void tivxRemoveTargetKernelAdd();
void tivxRemoveTargetKernelBitwise();
void tivxRemoveTargetKernelChannelCombine();
void tivxRemoveTargetKernelChannelExtract();
void tivxRemoveTargetKernelColorConvert();
void tivxRemoveTargetKernelConvertDepth();
void tivxRemoveTargetKernelConvolve();
void tivxRemoveTargetKernelEqualizeHistogram();
void tivxRemoveTargetKernelErode3x3();
void tivxRemoveTargetKernelFastCorners();
void tivxRemoveTargetKernelGaussianPyramid();
void tivxRemoveTargetKernelHarrisCorners();
void tivxRemoveTargetKernelHalfscaleGaussian();
void tivxRemoveTargetKernelHistogram();
void tivxRemoveTargetKernelIntegralImage();
void tivxRemoveTargetKernelLut();
void tivxRemoveTargetKernelMagnitude();
void tivxRemoveTargetKernelMeanStdDev();
void tivxRemoveTargetKernelMinMaxLoc();
void tivxRemoveTargetKernelMultiply();
void tivxRemoveTargetKernelPhase();
void tivxRemoveTargetKernelRemap();
void tivxRemoveTargetKernelScale();
void tivxRemoveTargetKernelSobel3x3();
void tivxRemoveTargetKernelSub();
void tivxRemoveTargetKernelThreshold();
void tivxRemoveTargetKernelWarpAffine();
void tivxRemoveTargetKernelWarpPerspective();

Tivx_Target_Kernel_List gTivx_target_kernel_list[] = {
    {tivxAddTargetKernelAbsDiff, tivxRemoveTargetKernelAbsDiff},
    {tivxAddTargetKernelAccumulate, tivxRemoveTargetKernelAccumulate},
    {tivxAddTargetKernelAccumulateSquare, tivxRemoveTargetKernelAccumulateSquare},
    {tivxAddTargetKernelAccumulateWeighted, tivxRemoveTargetKernelAccumulateWeighted},
    {tivxAddTargetKernelAdd, tivxRemoveTargetKernelAdd},
    {tivxAddTargetKernelBitwise, tivxRemoveTargetKernelBitwise},
    {tivxAddTargetKernelChannelCombine, tivxRemoveTargetKernelChannelCombine},
    {tivxAddTargetKernelChannelExtract, tivxRemoveTargetKernelChannelExtract},
    {tivxAddTargetKernelColorConvert, tivxRemoveTargetKernelColorConvert},
    {tivxAddTargetKernelConvertDepth, tivxRemoveTargetKernelConvertDepth},
    {tivxAddTargetKernelConvolve, tivxRemoveTargetKernelConvolve},
    {tivxAddTargetKernelEqualizeHistogram, tivxRemoveTargetKernelEqualizeHistogram},
    {tivxAddTargetKernelErode3x3, tivxRemoveTargetKernelErode3x3},
    {tivxAddTargetKernelFastCorners, tivxRemoveTargetKernelFastCorners},
    {tivxAddTargetKernelGaussianPyramid, tivxAddTargetKernelGaussianPyramid},
    {tivxAddTargetKernelHarrisCorners, tivxRemoveTargetKernelHarrisCorners},
    {tivxAddTargetKernelHalfscaleGaussian, tivxAddTargetKernelHalfscaleGaussian},
    {tivxAddTargetKernelHistogram, tivxRemoveTargetKernelHistogram},
    {tivxAddTargetKernelIntegralImage, tivxRemoveTargetKernelIntegralImage},
    {tivxAddTargetKernelLut, tivxRemoveTargetKernelLut},
    {tivxAddTargetKernelMagnitude, tivxRemoveTargetKernelMagnitude},
    {tivxAddTargetKernelMeanStdDev, tivxRemoveTargetKernelMeanStdDev},
    {tivxAddTargetKernelMinMaxLoc, tivxRemoveTargetKernelMinMaxLoc},
    {tivxAddTargetKernelMultiply, tivxRemoveTargetKernelMultiply},
    {tivxAddTargetKernelPhase, tivxRemoveTargetKernelPhase},
    {tivxAddTargetKernelRemap, tivxRemoveTargetKernelRemap},
    {tivxAddTargetKernelScale, tivxRemoveTargetKernelScale},
    {tivxAddTargetKernelSobel3x3, tivxRemoveTargetKernelSobel3x3},
    {tivxAddTargetKernelSub, tivxRemoveTargetKernelSub},
    {tivxAddTargetKernelThreshold, tivxRemoveTargetKernelThreshold},
    {tivxAddTargetKernelWarpAffine, tivxRemoveTargetKernelWarpAffine},
    {tivxAddTargetKernelWarpPerspective, tivxRemoveTargetKernelWarpPerspective}
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
