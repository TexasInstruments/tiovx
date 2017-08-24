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
#include "tivx_kernels_host_utils.h"

static vx_status VX_CALLBACK publishKernels(vx_context context);
static vx_status VX_CALLBACK unPublishKernels(vx_context context);

vx_status tivxAddKernelAbsDiff(vx_context context);
vx_status tivxAddKernelAccumulate(vx_context context);
vx_status tivxAddKernelAccumulateSquare(vx_context context);
vx_status tivxAddKernelAccumulateWeighted(vx_context context);
vx_status tivxAddKernelAdd(vx_context context);
vx_status tivxAddKernelBitwise(vx_context context);
vx_status tivxAddKernelCannyEd(vx_context context);
vx_status tivxAddKernelChannelCombine(vx_context context);
vx_status tivxAddKernelChannelExtract(vx_context context);
vx_status tivxAddKernelColorConvert(vx_context context);
vx_status tivxAddKernelConvertDepth(vx_context context);
vx_status tivxAddKernelConvolve(vx_context context);
vx_status tivxAddKernelEqualizeHistogram(vx_context context);
vx_status tivxAddKernelErode3x3(vx_context context);
vx_status tivxAddKernelFastCorners(vx_context context);
vx_status tivxAddKernelGaussianPyramid(vx_context context);
vx_status tivxAddKernelHarrisCorners(vx_context context);
vx_status tivxAddKernelHalfscaleGaussian(vx_context context);
vx_status tivxAddKernelHistogram(vx_context context);
vx_status tivxAddKernelIntegralImage(vx_context context);
vx_status tivxAddKernelLaplacianPyramid(vx_context context);
vx_status tivxAddKernelLaplacianReconstruct(vx_context context);
vx_status tivxAddKernelLut(vx_context context);
vx_status tivxAddKernelMagnitude(vx_context context);
vx_status tivxAddKernelMeanStdDev(vx_context context);
vx_status tivxAddKernelMinMaxLoc(vx_context context);
vx_status tivxAddKernelMultiply(vx_context context);
vx_status tivxAddKernelNonLinearFilter(vx_context context);
vx_status tivxAddKernelOpticalFlowPyrLk(vx_context context);
vx_status tivxAddKernelPhase(vx_context context);
vx_status tivxAddKernelRemap(vx_context context);
vx_status tivxAddKernelScale(vx_context context);
vx_status tivxAddKernelSobel3x3(vx_context context);
vx_status tivxAddKernelSub(vx_context context);
vx_status tivxAddKernelThreshold(vx_context context);
vx_status tivxAddKernelWarpAffine(vx_context context);
vx_status tivxAddKernelWarpPerspective(vx_context context);

vx_status tivxRemoveKernelAbsDiff(vx_context context);
vx_status tivxRemoveKernelAccumulate(vx_context context);
vx_status tivxRemoveKernelAccumulateSquare(vx_context context);
vx_status tivxRemoveKernelAccumulateWeighted(vx_context context);
vx_status tivxRemoveKernelAdd(vx_context context);
vx_status tivxRemoveKernelBitwise(vx_context context);
vx_status tivxRemoveKernelCannyEd(vx_context context);
vx_status tivxRemoveKernelChannelCombine(vx_context context);
vx_status tivxRemoveKernelChannelExtract(vx_context context);
vx_status tivxRemoveKernelColorConvert(vx_context context);
vx_status tivxRemoveKernelConvertDepth(vx_context context);
vx_status tivxRemoveKernelConvolve(vx_context context);
vx_status tivxRemoveKernelEqualizeHistogram(vx_context context);
vx_status tivxRemoveKernelErode3x3(vx_context context);
vx_status tivxRemoveKernelFastCorners(vx_context context);
vx_status tivxRemoveKernelGaussianPyramid(vx_context context);
vx_status tivxRemoveKernelHarrisCorners(vx_context context);
vx_status tivxRemoveKernelHalfscaleGaussian(vx_context context);
vx_status tivxRemoveKernelHistogram(vx_context context);
vx_status tivxRemoveKernelIntegralImage(vx_context context);
vx_status tivxRemoveKernelLut(vx_context context);
vx_status tivxRemoveKernelLaplacianPyramid(vx_context context);
vx_status tivxRemoveKernelLaplacianReconstruct(vx_context context);
vx_status tivxRemoveKernelMagnitude(vx_context context);
vx_status tivxRemoveKernelMeanStdDev(vx_context context);
vx_status tivxRemoveKernelMinMaxLoc(vx_context context);
vx_status tivxRemoveKernelMultiply(vx_context context);
vx_status tivxRemoveKernelNonLinearFilter(vx_context context);
vx_status tivxRemoveKernelOpticalFlowPyrLk(vx_context context);
vx_status tivxRemoveKernelPhase(vx_context context);
vx_status tivxRemoveKernelRemap(vx_context context);
vx_status tivxRemoveKernelScale(vx_context context);
vx_status tivxRemoveKernelSobel3x3(vx_context context);
vx_status tivxRemoveKernelSub(vx_context context);
vx_status tivxRemoveKernelThreshold(vx_context context);
vx_status tivxRemoveKernelWarpAffine(vx_context context);
vx_status tivxRemoveKernelWarpPerspective(vx_context context);

static Tivx_Host_Kernel_List  gTivx_host_kernel_list[] = {
    {tivxAddKernelAbsDiff, tivxRemoveKernelAbsDiff},
    {tivxAddKernelAccumulate, tivxRemoveKernelAccumulate},
    {tivxAddKernelAccumulateSquare, tivxRemoveKernelAccumulateSquare},
    {tivxAddKernelAccumulateWeighted, tivxRemoveKernelAccumulateWeighted},
    {tivxAddKernelAdd, tivxRemoveKernelAdd},
    {tivxAddKernelBitwise, tivxRemoveKernelBitwise},
    {tivxAddKernelCannyEd, tivxRemoveKernelCannyEd},
    {tivxAddKernelChannelCombine, tivxRemoveKernelChannelCombine},
    {tivxAddKernelChannelExtract, tivxRemoveKernelChannelExtract},
    {tivxAddKernelColorConvert, tivxRemoveKernelColorConvert},
    {tivxAddKernelConvertDepth, tivxRemoveKernelConvertDepth},
    {tivxAddKernelConvolve, tivxRemoveKernelConvolve},
    {tivxAddKernelEqualizeHistogram, tivxRemoveKernelEqualizeHistogram},
    {tivxAddKernelErode3x3, tivxRemoveKernelErode3x3},
    {tivxAddKernelFastCorners, tivxRemoveKernelFastCorners},
    {tivxAddKernelGaussianPyramid, tivxRemoveKernelGaussianPyramid},
    {tivxAddKernelHarrisCorners, tivxRemoveKernelHarrisCorners},
    {tivxAddKernelHalfscaleGaussian, tivxRemoveKernelHalfscaleGaussian},
    {tivxAddKernelHistogram, tivxRemoveKernelHistogram},
    {tivxAddKernelIntegralImage, tivxRemoveKernelIntegralImage},
    {tivxAddKernelLaplacianPyramid, tivxRemoveKernelLaplacianPyramid},
    {tivxAddKernelLaplacianReconstruct, tivxRemoveKernelLaplacianReconstruct},
    {tivxAddKernelLut, tivxRemoveKernelLut},
    {tivxAddKernelMagnitude, tivxRemoveKernelMagnitude},
    {tivxAddKernelMeanStdDev, tivxRemoveKernelMeanStdDev},
    {tivxAddKernelMinMaxLoc, tivxRemoveKernelMinMaxLoc},
    {tivxAddKernelMultiply, tivxRemoveKernelMultiply},
    {tivxAddKernelNonLinearFilter, tivxRemoveKernelNonLinearFilter},
    {tivxAddKernelOpticalFlowPyrLk, tivxRemoveKernelOpticalFlowPyrLk},
    {tivxAddKernelPhase, tivxRemoveKernelPhase},
    {tivxAddKernelRemap, tivxRemoveKernelRemap},
    {tivxAddKernelScale, tivxRemoveKernelScale},
    {tivxAddKernelSobel3x3, tivxRemoveKernelSobel3x3},
    {tivxAddKernelSub, tivxRemoveKernelSub},
    {tivxAddKernelThreshold, tivxRemoveKernelThreshold},
    {tivxAddKernelWarpAffine, tivxRemoveKernelWarpAffine},
    {tivxAddKernelWarpPerspective, tivxRemoveKernelWarpPerspective}
};

static vx_status VX_CALLBACK publishKernels(vx_context context)
{
    return tivxPublishKernels(context, gTivx_host_kernel_list, dimof(gTivx_host_kernel_list));
}

static vx_status VX_CALLBACK unPublishKernels(vx_context context)
{
    return tivxUnPublishKernels(context, gTivx_host_kernel_list, dimof(gTivx_host_kernel_list));
}

void tivxRegisterOpenVXCoreKernels(void)
{
    tivxRegisterModule(TIVX_MODULE_NAME_OPENVX_CORE, publishKernels, unPublishKernels);
}

void tivxUnRegisterOpenVXCoreKernels(void)
{
    tivxUnRegisterModule(TIVX_MODULE_NAME_OPENVX_CORE);
}
