/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <TI/tivx_target_kernel.h>
#include <tivx_target_kernels_priv.h>

typedef void (*tivxTargetKernel_Fxn) ();

typedef struct  {
    tivxTargetKernel_Fxn    add_kernel;
    tivxTargetKernel_Fxn    remove_kernel;
} Tivx_Target_Kernel_List;

#ifdef USE_BAM
static Tivx_Target_Kernel_List gTivx_target_kernel_list[] = {
    {tivxAddTargetKernelBamAbsDiff, tivxRemoveTargetKernelBamAbsDiff},
    {tivxAddTargetKernelAccumulate, tivxRemoveTargetKernelAccumulate},
    {tivxAddTargetKernelAccumulateSquare, tivxRemoveTargetKernelAccumulateSquare},
    {tivxAddTargetKernelAccumulateWeighted, tivxRemoveTargetKernelAccumulateWeighted},
    {tivxAddTargetKernelBamAdd, tivxRemoveTargetKernelBamAdd},
    {tivxAddTargetKernelBamSubtract, tivxRemoveTargetKernelBamSubtract},
    {tivxAddTargetKernelBamBitwiseNot, tivxRemoveTargetKernelBamBitwiseNot},
    {tivxAddTargetKernelBamBitwiseAnd, tivxRemoveTargetKernelBamBitwiseAnd},
    {tivxAddTargetKernelBamBitwiseOr, tivxRemoveTargetKernelBamBitwiseOr},
    {tivxAddTargetKernelBamBitwiseXor, tivxRemoveTargetKernelBamBitwiseXor},
    {tivxAddTargetKernelBamBox3x3, tivxRemoveTargetKernelBamBox3x3},
    {tivxAddTargetKernelBamCannyEd, tivxRemoveTargetKernelBamCannyEd},
    {tivxAddTargetKernelChannelCombine, tivxRemoveTargetKernelChannelCombine},
    {tivxAddTargetKernelChannelExtract, tivxRemoveTargetKernelChannelExtract},
    {tivxAddTargetKernelColorConvert, tivxRemoveTargetKernelColorConvert},
    {tivxAddTargetKernelConvertDepth, tivxRemoveTargetKernelConvertDepth},
    {tivxAddTargetKernelConvolve, tivxRemoveTargetKernelConvolve},
    {tivxAddTargetKernelBamDilate3x3, tivxRemoveTargetKernelBamDilate3x3},
    {tivxAddTargetKernelEqualizeHistogram, tivxRemoveTargetKernelEqualizeHistogram},
    {tivxAddTargetKernelBamErode3x3, tivxRemoveTargetKernelBamErode3x3},
    {tivxAddTargetKernelFastCorners, tivxRemoveTargetKernelFastCorners},
    {tivxAddTargetKernelBamGaussian3x3, tivxRemoveTargetKernelBamGaussian3x3},
    {tivxAddTargetKernelGaussianPyramid, tivxAddTargetKernelGaussianPyramid},
    {tivxAddTargetKernelBamHarrisCorners, tivxRemoveTargetKernelBamHarrisCorners},
    {tivxAddTargetKernelHalfscaleGaussian, tivxAddTargetKernelHalfscaleGaussian},
    {tivxAddTargetKernelBamHistogram, tivxRemoveTargetKernelBamHistogram},
    {tivxAddTargetKernelBamIntegralImage, tivxRemoveTargetKernelBamIntegralImage},
    {tivxAddTargetKernelLaplacianPyramid, tivxRemoveTargetKernelLaplacianPyramid},
    {tivxAddTargetKernelLaplacianReconstruct, tivxRemoveTargetKernelLaplacianReconstruct},
    {tivxAddTargetKernelBamLut, tivxRemoveTargetKernelBamLut},
    {tivxAddTargetKernelMagnitude, tivxRemoveTargetKernelMagnitude},
    {tivxAddTargetKernelMeanStdDev, tivxRemoveTargetKernelMeanStdDev},
    {tivxAddTargetKernelBamMedian3x3, tivxRemoveTargetKernelBamMedian3x3},
    {tivxAddTargetKernelBamMinMaxLoc, tivxRemoveTargetKernelBamMinMaxLoc},
    {tivxAddTargetKernelBamMultiply, tivxRemoveTargetKernelBamMultiply},
    {tivxAddTargetKernelNonLinearFilter, tivxRemoveTargetKernelNonLinearFilter},
    {tivxAddTargetKernelOpticalFlowPyrLk, tivxRemoveTargetKernelOpticalFlowPyrLk},
    {tivxAddTargetKernelPhase, tivxRemoveTargetKernelPhase},
    {tivxAddTargetKernelRemap, tivxRemoveTargetKernelRemap},
    {tivxAddTargetKernelScale, tivxRemoveTargetKernelScale},
    {tivxAddTargetKernelBamSobel3x3, tivxRemoveTargetKernelBamSobel3x3},
    {tivxAddTargetKernelBamThreshold, tivxRemoveTargetKernelBamThreshold},
    {tivxAddTargetKernelWarpAffine, tivxRemoveTargetKernelWarpAffine},
    {tivxAddTargetKernelWarpPerspective, tivxRemoveTargetKernelWarpPerspective}
};

#else

static Tivx_Target_Kernel_List gTivx_target_kernel_list[] = {
    {tivxAddTargetKernelAbsDiff, tivxRemoveTargetKernelAbsDiff},
    {tivxAddTargetKernelAccumulate, tivxRemoveTargetKernelAccumulate},
    {tivxAddTargetKernelAccumulateSquare, tivxRemoveTargetKernelAccumulateSquare},
    {tivxAddTargetKernelAccumulateWeighted, tivxRemoveTargetKernelAccumulateWeighted},
    {tivxAddTargetKernelAdd, tivxRemoveTargetKernelAdd},
    {tivxAddTargetKernelBitwise, tivxRemoveTargetKernelBitwise},
    {tivxAddTargetKernelCannyEd, tivxRemoveTargetKernelCannyEd},
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
    {tivxAddTargetKernelLaplacianPyramid, tivxRemoveTargetKernelLaplacianPyramid},
    {tivxAddTargetKernelLaplacianReconstruct, tivxRemoveTargetKernelLaplacianReconstruct},
    {tivxAddTargetKernelLut, tivxRemoveTargetKernelLut},
    {tivxAddTargetKernelMagnitude, tivxRemoveTargetKernelMagnitude},
    {tivxAddTargetKernelMeanStdDev, tivxRemoveTargetKernelMeanStdDev},
    {tivxAddTargetKernelMinMaxLoc, tivxRemoveTargetKernelMinMaxLoc},
    {tivxAddTargetKernelMultiply, tivxRemoveTargetKernelMultiply},
    {tivxAddTargetKernelNonLinearFilter, tivxRemoveTargetKernelNonLinearFilter},
    {tivxAddTargetKernelOpticalFlowPyrLk, tivxRemoveTargetKernelOpticalFlowPyrLk},
    {tivxAddTargetKernelPhase, tivxRemoveTargetKernelPhase},
    {tivxAddTargetKernelRemap, tivxRemoveTargetKernelRemap},
    {tivxAddTargetKernelScale, tivxRemoveTargetKernelScale},
    {tivxAddTargetKernelSobel3x3, tivxRemoveTargetKernelSobel3x3},
    {tivxAddTargetKernelSub, tivxRemoveTargetKernelSub},
    {tivxAddTargetKernelThreshold, tivxRemoveTargetKernelThreshold},
    {tivxAddTargetKernelWarpAffine, tivxRemoveTargetKernelWarpAffine},
    {tivxAddTargetKernelWarpPerspective, tivxRemoveTargetKernelWarpPerspective}
};

#endif

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
        (sizeof(gTivx_target_kernel_list)/sizeof(Tivx_Target_Kernel_List));
        i ++)
    {
        if (gTivx_target_kernel_list[i].remove_kernel)
        {
            gTivx_target_kernel_list[i].remove_kernel();
        }
    }
}
