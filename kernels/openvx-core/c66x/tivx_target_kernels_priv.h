/*
 *******************************************************************************
 *
 * Copyright (C) 2016-2017 Texas Instruments Incorporated - http://www.ti.com/
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

#ifdef BUILD_BAM

void tivxAddTargetKernelBamAbsDiff(void);
void tivxAddTargetKernelBamAccumulate(void);
void tivxAddTargetKernelBamAccumulateSquare(void);
void tivxAddTargetKernelBamAccumulateWeighted(void);
void tivxAddTargetKernelBamAdd(void);
void tivxAddTargetKernelBamSubtract(void);
void tivxAddTargetKernelBamBitwiseNot(void);
void tivxAddTargetKernelBamBitwiseAnd(void);
void tivxAddTargetKernelBamBitwiseOr(void);
void tivxAddTargetKernelBamBitwiseXor(void);
void tivxAddTargetKernelBamBox3x3(void);
void tivxAddTargetKernelBamCannyEd(void);
void tivxAddTargetKernelChannelCombine(void);
void tivxAddTargetKernelBamChannelExtract(void);
void tivxAddTargetKernelBamColorConvert(void);
void tivxAddTargetKernelBamConvertDepth(void);
void tivxAddTargetKernelBamConvolve(void);
void tivxAddTargetKernelBamDilate3x3(void);
void tivxAddTargetKernelBamEqHist(void);
void tivxAddTargetKernelBamErode3x3(void);
void tivxAddTargetKernelFastCorners(void);
void tivxAddTargetKernelBamGaussian3x3(void);
void tivxAddTargetKernelGaussianPyramid(void);
void tivxAddTargetKernelBamHarrisCorners(void);
void tivxAddTargetKernelHalfscaleGaussian(void);
void tivxAddTargetKernelBamHistogram(void);
void tivxAddTargetKernelBamIntegralImage(void);
void tivxAddTargetKernelLaplacianPyramid(void);
void tivxAddTargetKernelLaplacianReconstruct(void);
void tivxAddTargetKernelBamLut(void);
void tivxAddTargetKernelBamMagnitude(void);
void tivxAddTargetKernelMeanStdDev(void);
void tivxAddTargetKernelBamMedian3x3(void);
void tivxAddTargetKernelBamMinMaxLoc(void);
void tivxAddTargetKernelBamMultiply(void);
void tivxAddTargetKernelBamNonLinearFilter(void);
void tivxAddTargetKernelOpticalFlowPyrLk(void);
void tivxAddTargetKernelBamPhase(void);
void tivxAddTargetKernelRemap(void);
void tivxAddTargetKernelScale(void);
void tivxAddTargetKernelBamSobel3x3(void);
void tivxAddTargetKernelBamThreshold(void);
void tivxAddTargetKernelWarpAffine(void);
void tivxAddTargetKernelWarpPerspective(void);

void tivxRemoveTargetKernelBamAbsDiff(void);
void tivxRemoveTargetKernelBamAccumulate(void);
void tivxRemoveTargetKernelBamAccumulateSquare(void);
void tivxRemoveTargetKernelBamAccumulateWeighted(void);
void tivxRemoveTargetKernelBamAdd(void);
void tivxRemoveTargetKernelBamSubtract(void);
void tivxRemoveTargetKernelBamBitwiseNot(void);
void tivxRemoveTargetKernelBamBitwiseAnd(void);
void tivxRemoveTargetKernelBamBitwiseOr(void);
void tivxRemoveTargetKernelBamBitwiseXor(void);
void tivxRemoveTargetKernelBamBox3x3(void);
void tivxRemoveTargetKernelBamCannyEd(void);
void tivxRemoveTargetKernelChannelCombine(void);
void tivxRemoveTargetKernelBamChannelExtract(void);
void tivxRemoveTargetKernelBamColorConvert(void);
void tivxRemoveTargetKernelBamConvertDepth(void);
void tivxRemoveTargetKernelBamConvolve(void);
void tivxRemoveTargetKernelBamDilate3x3(void);
void tivxRemoveTargetKernelBamEqHist(void);
void tivxRemoveTargetKernelBamErode3x3(void);
void tivxRemoveTargetKernelFastCorners(void);
void tivxRemoveTargetKernelBamGaussian3x3(void);
void tivxRemoveTargetKernelGaussianPyramid(void);
void tivxRemoveTargetKernelBamHarrisCorners(void);
void tivxRemoveTargetKernelHalfscaleGaussian(void);
void tivxRemoveTargetKernelBamHistogram(void);
void tivxRemoveTargetKernelBamIntegralImage(void);
void tivxRemoveTargetKernelLaplacianPyramid(void);
void tivxRemoveTargetKernelLaplacianReconstruct(void);
void tivxRemoveTargetKernelBamLut(void);
void tivxRemoveTargetKernelBamMagnitude(void);
void tivxRemoveTargetKernelMeanStdDev(void);
void tivxRemoveTargetKernelBamMedian3x3(void);
void tivxRemoveTargetKernelBamMinMaxLoc(void);
void tivxRemoveTargetKernelBamMultiply(void);
void tivxRemoveTargetKernelBamNonLinearFilter(void);
void tivxRemoveTargetKernelOpticalFlowPyrLk(void);
void tivxRemoveTargetKernelBamPhase(void);
void tivxRemoveTargetKernelRemap(void);
void tivxRemoveTargetKernelScale(void);
void tivxRemoveTargetKernelSobel3x3(void);
void tivxRemoveTargetKernelBamSobel3x3(void);
void tivxRemoveTargetKernelBamThreshold(void);
void tivxRemoveTargetKernelWarpAffine(void);
void tivxRemoveTargetKernelWarpPerspective(void);

#else

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

#endif

#ifdef __cplusplus
}
#endif

#endif

