/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_TARGET_KERNELS_PRIV_
#define TIVX_TARGET_KERNELS_PRIV_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for all target kernels
 */

void tivxAddTargetKernelAbsDiff(void);
void tivxAddTargetKernelAccumulate(void);
void tivxAddTargetKernelAccumulateSquare(void);
void tivxAddTargetKernelAccumulateWeighted(void);
void tivxAddTargetKernelAdd(void);
void tivxAddTargetKernelBitwise(void);
void tivxAddTargetKernelCannyEd(void);
void tivxAddTargetKernelChannelCombine(void);
void tivxAddTargetKernelChannelExtract(void);
void tivxAddTargetKernelColorConvert(void);
void tivxAddTargetKernelConvertDepth(void);
void tivxAddTargetKernelConvolve(void);
void tivxAddTargetKernelEqualizeHistogram(void);
void tivxAddTargetKernelErode3x3(void);
void tivxAddTargetKernelFastCorners(void);
void tivxAddTargetKernelGaussianPyramid(void);
void tivxAddTargetKernelHarrisCorners(void);
void tivxAddTargetKernelHalfscaleGaussian(void);
void tivxAddTargetKernelHistogram(void);
void tivxAddTargetKernelIntegralImage(void);
void tivxAddTargetKernelLaplacianPyramid(void);
void tivxAddTargetKernelLaplacianReconstruct(void);
void tivxAddTargetKernelLut(void);
void tivxAddTargetKernelMagnitude(void);
void tivxAddTargetKernelMeanStdDev(void);
void tivxAddTargetKernelMinMaxLoc(void);
void tivxAddTargetKernelMultiply(void);
void tivxAddTargetKernelNonLinearFilter(void);
void tivxAddTargetKernelOpticalFlowPyrLk(void);
void tivxAddTargetKernelPhase(void);
void tivxAddTargetKernelRemap(void);
void tivxAddTargetKernelScale(void);
void tivxAddTargetKernelSobel3x3(void);
void tivxAddTargetKernelSub(void);
void tivxAddTargetKernelThreshold(void);
void tivxAddTargetKernelWarpAffine(void);
void tivxAddTargetKernelWarpPerspective(void);

void tivxRemoveTargetKernelAbsDiff(void);
void tivxRemoveTargetKernelAccumulate(void);
void tivxRemoveTargetKernelAccumulateSquare(void);
void tivxRemoveTargetKernelAccumulateWeighted(void);
void tivxRemoveTargetKernelAdd(void);
void tivxRemoveTargetKernelBitwise(void);
void tivxRemoveTargetKernelCannyEd(void);
void tivxRemoveTargetKernelChannelCombine(void);
void tivxRemoveTargetKernelChannelExtract(void);
void tivxRemoveTargetKernelColorConvert(void);
void tivxRemoveTargetKernelConvertDepth(void);
void tivxRemoveTargetKernelConvolve(void);
void tivxRemoveTargetKernelEqualizeHistogram(void);
void tivxRemoveTargetKernelErode3x3(void);
void tivxRemoveTargetKernelFastCorners(void);
void tivxRemoveTargetKernelGaussianPyramid(void);
void tivxRemoveTargetKernelHarrisCorners(void);
void tivxRemoveTargetKernelHalfscaleGaussian(void);
void tivxRemoveTargetKernelHistogram(void);
void tivxRemoveTargetKernelIntegralImage(void);
void tivxRemoveTargetKernelLaplacianPyramid(void);
void tivxRemoveTargetKernelLaplacianReconstruct(void);
void tivxRemoveTargetKernelLut(void);
void tivxRemoveTargetKernelMagnitude(void);
void tivxRemoveTargetKernelMeanStdDev(void);
void tivxRemoveTargetKernelMinMaxLoc(void);
void tivxRemoveTargetKernelMultiply(void);
void tivxRemoveTargetKernelNonLinearFilter(void);
void tivxRemoveTargetKernelOpticalFlowPyrLk(void);
void tivxRemoveTargetKernelPhase(void);
void tivxRemoveTargetKernelRemap(void);
void tivxRemoveTargetKernelScale(void);
void tivxRemoveTargetKernelSobel3x3(void);
void tivxRemoveTargetKernelSub(void);
void tivxRemoveTargetKernelThreshold(void);
void tivxRemoveTargetKernelWarpAffine(void);
void tivxRemoveTargetKernelWarpPerspective(void);


#ifdef __cplusplus
}
#endif

#endif

