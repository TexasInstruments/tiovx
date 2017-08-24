/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#include <TI/tivx.h>
#include <TI/tivx_target_kernel.h>
#include <tivx_target_kernels_priv.h>
#include <tivx_kernels_target_utils.h>

#ifdef BUILD_BAM
static Tivx_Target_Kernel_List gTivx_target_kernel_list[] = {
    {tivxAddTargetKernelBamAbsDiff, tivxRemoveTargetKernelBamAbsDiff},
    {tivxAddTargetKernelBamAccumulate, tivxRemoveTargetKernelBamAccumulate},
    {tivxAddTargetKernelBamAccumulateSquare, tivxRemoveTargetKernelBamAccumulateSquare},
    {tivxAddTargetKernelBamAccumulateWeighted, tivxRemoveTargetKernelBamAccumulateWeighted},
    {tivxAddTargetKernelBamAdd, tivxRemoveTargetKernelBamAdd},
    {tivxAddTargetKernelBamSubtract, tivxRemoveTargetKernelBamSubtract},
    {tivxAddTargetKernelBamBitwiseNot, tivxRemoveTargetKernelBamBitwiseNot},
    {tivxAddTargetKernelBamBitwiseAnd, tivxRemoveTargetKernelBamBitwiseAnd},
    {tivxAddTargetKernelBamBitwiseOr, tivxRemoveTargetKernelBamBitwiseOr},
    {tivxAddTargetKernelBamBitwiseXor, tivxRemoveTargetKernelBamBitwiseXor},
    {tivxAddTargetKernelBamBox3x3, tivxRemoveTargetKernelBamBox3x3},
    {tivxAddTargetKernelBamCannyEd, tivxRemoveTargetKernelBamCannyEd},
    {tivxAddTargetKernelBamChannelCombine, tivxRemoveTargetKernelBamChannelCombine},
    {tivxAddTargetKernelBamChannelExtract, tivxRemoveTargetKernelBamChannelExtract},
    {tivxAddTargetKernelBamColorConvert, tivxRemoveTargetKernelBamColorConvert},
    {tivxAddTargetKernelBamConvertDepth, tivxRemoveTargetKernelBamConvertDepth},
    {tivxAddTargetKernelBamConvolve, tivxRemoveTargetKernelBamConvolve},
    {tivxAddTargetKernelBamDilate3x3, tivxRemoveTargetKernelBamDilate3x3},
    {tivxAddTargetKernelBamEqHist, tivxRemoveTargetKernelBamEqHist},
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
    {tivxAddTargetKernelBamMagnitude, tivxRemoveTargetKernelBamMagnitude},
    {tivxAddTargetKernelBamMeanStdDev, tivxRemoveTargetKernelBamMeanStdDev},
    {tivxAddTargetKernelBamMedian3x3, tivxRemoveTargetKernelBamMedian3x3},
    {tivxAddTargetKernelBamMinMaxLoc, tivxRemoveTargetKernelBamMinMaxLoc},
    {tivxAddTargetKernelBamMultiply, tivxRemoveTargetKernelBamMultiply},
    {tivxAddTargetKernelBamNonLinearFilter, tivxRemoveTargetKernelBamNonLinearFilter},
    {tivxAddTargetKernelOpticalFlowPyrLk, tivxRemoveTargetKernelOpticalFlowPyrLk},
    {tivxAddTargetKernelBamPhase, tivxRemoveTargetKernelBamPhase},
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
    tivxRegisterTargetKernels(gTivx_target_kernel_list, dimof(gTivx_target_kernel_list));

    tivxReserveC66xL2MEM();
}

void tivxUnRegisterOpenVXCoreTargetKernels()
{
    tivxUnRegisterTargetKernels(gTivx_target_kernel_list, dimof(gTivx_target_kernel_list));
}
