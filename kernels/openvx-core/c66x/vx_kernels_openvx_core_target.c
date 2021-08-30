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
#include <tivx_openvx_core_kernels.h>

#ifdef BUILD_BAM

#include "tivx_bam_kernel_database.h"

static Tivx_Target_Kernel_List gTivx_target_kernel_list[] = {
    {tivxAddTargetKernelBamAbsDiff, tivxRemoveTargetKernelBamAbsDiff},
    {tivxAddTargetKernelBamAccumulate, tivxRemoveTargetKernelBamAccumulate},
    {tivxAddTargetKernelBamAccumulateSquare, tivxRemoveTargetKernelBamAccumulateSquare},
    {tivxAddTargetKernelBamAccumulateWeighted, tivxRemoveTargetKernelBamAccumulateWeighted},
    {tivxAddTargetKernelBamAdd, tivxRemoveTargetKernelBamAdd},
    {tivxAddTargetKernelBamSubtract, tivxRemoveTargetKernelBamSubtract},
    {tivxAddTargetKernelBamAnd, tivxRemoveTargetKernelBamAnd},
    {tivxAddTargetKernelBamBox3X3, tivxRemoveTargetKernelBamBox3X3},
    {tivxAddTargetKernelBamCannyEd, tivxRemoveTargetKernelBamCannyEd},
    {tivxAddTargetKernelBamChannelCombine, tivxRemoveTargetKernelBamChannelCombine},
    {tivxAddTargetKernelBamChannelExtract, tivxRemoveTargetKernelBamChannelExtract},
    {tivxAddTargetKernelBamColorConvert, tivxRemoveTargetKernelBamColorConvert},
    {tivxAddTargetKernelBamConvertDepth, tivxRemoveTargetKernelBamConvertDepth},
    {tivxAddTargetKernelBamConvolve, tivxRemoveTargetKernelBamConvolve},
    {tivxAddTargetKernelBamDilate3X3, tivxRemoveTargetKernelBamDilate3X3},
    {tivxAddTargetKernelBamEqHist, tivxRemoveTargetKernelBamEqHist},
    {tivxAddTargetKernelBamErode3X3, tivxRemoveTargetKernelBamErode3X3},
    {tivxAddTargetKernelFastCorners, tivxRemoveTargetKernelFastCorners},
    {tivxAddTargetKernelBamGaussian3X3, tivxRemoveTargetKernelBamGaussian3X3},
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
    {tivxAddTargetKernelBamMedian3X3, tivxRemoveTargetKernelBamMedian3X3},
    {tivxAddTargetKernelBamMinMaxLoc, tivxRemoveTargetKernelBamMinMaxLoc},
    {tivxAddTargetKernelBamMultiply, tivxRemoveTargetKernelBamMultiply},
    {tivxAddTargetKernelBamNonLinearFilter, tivxRemoveTargetKernelBamNonLinearFilter},
    {tivxAddTargetKernelBamNot, tivxRemoveTargetKernelBamNot},
    {tivxAddTargetKernelOpticalFlowPyrLk, tivxRemoveTargetKernelOpticalFlowPyrLk},
    {tivxAddTargetKernelBamOr, tivxRemoveTargetKernelBamOr},
    {tivxAddTargetKernelBamPhase, tivxRemoveTargetKernelBamPhase},
    {tivxAddTargetKernelRemap, tivxRemoveTargetKernelRemap},
    {tivxAddTargetKernelScale, tivxRemoveTargetKernelScale},
    {tivxAddTargetKernelBamSobel3x3, tivxRemoveTargetKernelBamSobel3x3},
    {tivxAddTargetKernelBamThreshold, tivxRemoveTargetKernelBamThreshold},
    {tivxAddTargetKernelWarpAffine, tivxRemoveTargetKernelWarpAffine},
    {tivxAddTargetKernelWarpPerspective, tivxRemoveTargetKernelWarpPerspective},
    {tivxAddTargetKernelBamXor, tivxRemoveTargetKernelBamXor},
    {tivxAddTargetKernelBamSupernode, tivxRemoveTargetKernelBamSupernode}
};

#else

static Tivx_Target_Kernel_List gTivx_target_kernel_list[] = {
    {tivxAddTargetKernelAbsDiff, tivxRemoveTargetKernelAbsDiff},
    {tivxAddTargetKernelAccumulate, tivxRemoveTargetKernelAccumulate},
    {tivxAddTargetKernelAccumulateSquare, tivxRemoveTargetKernelAccumulateSquare},
    {tivxAddTargetKernelAccumulateWeighted, tivxRemoveTargetKernelAccumulateWeighted},
    {tivxAddTargetKernelAdd, tivxRemoveTargetKernelAdd},
    {tivxAddTargetKernelAnd, tivxRemoveTargetKernelAnd},
    {tivxAddTargetKernelBox3X3, tivxRemoveTargetKernelBox3X3},
    {tivxAddTargetKernelCannyEd, tivxRemoveTargetKernelCannyEd},
    {tivxAddTargetKernelChannelCombine, tivxRemoveTargetKernelChannelCombine},
    {tivxAddTargetKernelChannelExtract, tivxRemoveTargetKernelChannelExtract},
    {tivxAddTargetKernelColorConvert, tivxRemoveTargetKernelColorConvert},
    {tivxAddTargetKernelConvertDepth, tivxRemoveTargetKernelConvertDepth},
    {tivxAddTargetKernelConvolve, tivxRemoveTargetKernelConvolve},
    {tivxAddTargetKernelDilate3X3, tivxRemoveTargetKernelDilate3X3},
    {tivxAddTargetKernelEqualizeHistogram, tivxRemoveTargetKernelEqualizeHistogram},
    {tivxAddTargetKernelErode3X3, tivxRemoveTargetKernelErode3X3},
    {tivxAddTargetKernelFastCorners, tivxRemoveTargetKernelFastCorners},
    {tivxAddTargetKernelGaussianPyramid, tivxAddTargetKernelGaussianPyramid},
    {tivxAddTargetKernelGaussian3X3, tivxRemoveTargetKernelGaussian3X3},
    {tivxAddTargetKernelHarrisCorners, tivxRemoveTargetKernelHarrisCorners},
    {tivxAddTargetKernelHalfscaleGaussian, tivxAddTargetKernelHalfscaleGaussian},
    {tivxAddTargetKernelHistogram, tivxRemoveTargetKernelHistogram},
    {tivxAddTargetKernelIntegralImage, tivxRemoveTargetKernelIntegralImage},
    {tivxAddTargetKernelLaplacianPyramid, tivxRemoveTargetKernelLaplacianPyramid},
    {tivxAddTargetKernelLaplacianReconstruct, tivxRemoveTargetKernelLaplacianReconstruct},
    {tivxAddTargetKernelLut, tivxRemoveTargetKernelLut},
    {tivxAddTargetKernelMagnitude, tivxRemoveTargetKernelMagnitude},
    {tivxAddTargetKernelMeanStdDev, tivxRemoveTargetKernelMeanStdDev},
    {tivxAddTargetKernelMedian3X3, tivxRemoveTargetKernelMedian3X3},
    {tivxAddTargetKernelMinMaxLoc, tivxRemoveTargetKernelMinMaxLoc},
    {tivxAddTargetKernelMultiply, tivxRemoveTargetKernelMultiply},
    {tivxAddTargetKernelNonLinearFilter, tivxRemoveTargetKernelNonLinearFilter},
    {tivxAddTargetKernelNot, tivxRemoveTargetKernelNot},
    {tivxAddTargetKernelOpticalFlowPyrLk, tivxRemoveTargetKernelOpticalFlowPyrLk},
    {tivxAddTargetKernelOr, tivxRemoveTargetKernelOr},
    {tivxAddTargetKernelPhase, tivxRemoveTargetKernelPhase},
    {tivxAddTargetKernelRemap, tivxRemoveTargetKernelRemap},
    {tivxAddTargetKernelScale, tivxRemoveTargetKernelScale},
    {tivxAddTargetKernelSobel3x3, tivxRemoveTargetKernelSobel3x3},
    {tivxAddTargetKernelSub, tivxRemoveTargetKernelSub},
    {tivxAddTargetKernelThreshold, tivxRemoveTargetKernelThreshold},
    {tivxAddTargetKernelWarpAffine, tivxRemoveTargetKernelWarpAffine},
    {tivxAddTargetKernelWarpPerspective, tivxRemoveTargetKernelWarpPerspective},
    {tivxAddTargetKernelXor, tivxRemoveTargetKernelXor},
    {tivxAddTargetKernelWarpPerspective, tivxRemoveTargetKernelWarpPerspective}
};

#endif

void tivxRegisterOpenVXCoreTargetKernels(void)
{

#ifdef BUILD_BAM
    tivxRegisterOpenVXCoreBamPlugins();
#endif

    tivxRegisterTargetKernels(gTivx_target_kernel_list, dimof(gTivx_target_kernel_list));

    tivxReserveC66xL2MEM();
}

void tivxUnRegisterOpenVXCoreTargetKernels(void)
{
    tivxUnRegisterTargetKernels(gTivx_target_kernel_list, dimof(gTivx_target_kernel_list));
}

