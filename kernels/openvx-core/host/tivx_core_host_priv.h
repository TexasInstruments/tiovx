/*
 *
 * Copyright (c) 2017-2019 Texas Instruments Incorporated
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

#ifndef TIVX_CORE_HOST_PRIV_H_
#define TIVX_CORE_HOST_PRIV_H_

#include <TI/tivx.h>

vx_status tivxAddKernelAbsdiff(vx_context context);
vx_status tivxAddKernelAccumulate(vx_context context);
vx_status tivxAddKernelAccumulateSquare(vx_context context);
vx_status tivxAddKernelAccumulateWeighted(vx_context context);
vx_status tivxAddKernelAdd(vx_context context);
vx_status tivxAddKernelAnd(vx_context context);
vx_status tivxAddKernelBox3X3(vx_context context);
vx_status tivxAddKernelCanny(vx_context context);
vx_status tivxAddKernelChannelCombine(vx_context context);
vx_status tivxAddKernelChannelExtract(vx_context context);
vx_status tivxAddKernelColorConvert(vx_context context);
vx_status tivxAddKernelConvertDepth(vx_context context);
vx_status tivxAddKernelConvolve(vx_context context);
vx_status tivxAddKernelDilate3X3(vx_context context);
vx_status tivxAddKernelEqualizeHistogram(vx_context context);
vx_status tivxAddKernelErode3X3(vx_context context);
vx_status tivxAddKernelFastCorners(vx_context context);
vx_status tivxAddKernelGaussianPyramid(vx_context context);
vx_status tivxAddKernelGaussian3X3(vx_context context);
vx_status tivxAddKernelHarrisCorners(vx_context context);
vx_status tivxAddKernelHalfscaleGaussian(vx_context context);
vx_status tivxAddKernelHistogram(vx_context context);
vx_status tivxAddKernelIntegralImage(vx_context context);
vx_status tivxAddKernelLaplacianPyramid(vx_context context);
vx_status tivxAddKernelLaplacianReconstruct(vx_context context);
vx_status tivxAddKernelLut(vx_context context);
vx_status tivxAddKernelMagnitude(vx_context context);
vx_status tivxAddKernelMeanStdDev(vx_context context);
vx_status tivxAddKernelMedian3X3(vx_context context);
vx_status tivxAddKernelMinMaxLoc(vx_context context);
vx_status tivxAddKernelMultiply(vx_context context);
vx_status tivxAddKernelNonLinearFilter(vx_context context);
vx_status tivxAddKernelNot(vx_context context);
vx_status tivxAddKernelOpticalFlowPyrLk(vx_context context);
vx_status tivxAddKernelOr(vx_context context);
vx_status tivxAddKernelPhase(vx_context context);
vx_status tivxAddKernelRemap(vx_context context);
vx_status tivxAddKernelScale(vx_context context);
vx_status tivxAddKernelSobel3X3(vx_context context);
vx_status tivxAddKernelSub(vx_context context);
vx_status tivxAddKernelThreshold(vx_context context);
vx_status tivxAddKernelWarpAffine(vx_context context);
vx_status tivxAddKernelWarpPerspective(vx_context context);
vx_status tivxAddKernelXor(vx_context context);
vx_status tivxAddKernelSuperNode(vx_context context);

vx_status tivxRemoveKernelAbsdiff(vx_context context);
vx_status tivxRemoveKernelAccumulate(vx_context context);
vx_status tivxRemoveKernelAccumulateSquare(vx_context context);
vx_status tivxRemoveKernelAccumulateWeighted(vx_context context);
vx_status tivxRemoveKernelAdd(vx_context context);
vx_status tivxRemoveKernelAnd(vx_context context);
vx_status tivxRemoveKernelBox3X3(vx_context context);
vx_status tivxRemoveKernelCanny(vx_context context);
vx_status tivxRemoveKernelChannelCombine(vx_context context);
vx_status tivxRemoveKernelChannelExtract(vx_context context);
vx_status tivxRemoveKernelColorConvert(vx_context context);
vx_status tivxRemoveKernelConvertDepth(vx_context context);
vx_status tivxRemoveKernelConvolve(vx_context context);
vx_status tivxRemoveKernelDilate3X3(vx_context context);
vx_status tivxRemoveKernelEqualizeHistogram(vx_context context);
vx_status tivxRemoveKernelErode3X3(vx_context context);
vx_status tivxRemoveKernelFastCorners(vx_context context);
vx_status tivxRemoveKernelGaussianPyramid(vx_context context);
vx_status tivxRemoveKernelGaussian3X3(vx_context context);
vx_status tivxRemoveKernelHarrisCorners(vx_context context);
vx_status tivxRemoveKernelHalfscaleGaussian(vx_context context);
vx_status tivxRemoveKernelHistogram(vx_context context);
vx_status tivxRemoveKernelIntegralImage(vx_context context);
vx_status tivxRemoveKernelLut(vx_context context);
vx_status tivxRemoveKernelLaplacianPyramid(vx_context context);
vx_status tivxRemoveKernelLaplacianReconstruct(vx_context context);
vx_status tivxRemoveKernelMagnitude(vx_context context);
vx_status tivxRemoveKernelMeanStdDev(vx_context context);
vx_status tivxRemoveKernelMedian3X3(vx_context context);
vx_status tivxRemoveKernelMinMaxLoc(vx_context context);
vx_status tivxRemoveKernelMultiply(vx_context context);
vx_status tivxRemoveKernelNonLinearFilter(vx_context context);
vx_status tivxRemoveKernelNot(vx_context context);
vx_status tivxRemoveKernelOpticalFlowPyrLk(vx_context context);
vx_status tivxRemoveKernelOr(vx_context context);
vx_status tivxRemoveKernelPhase(vx_context context);
vx_status tivxRemoveKernelRemap(vx_context context);
vx_status tivxRemoveKernelScale(vx_context context);
vx_status tivxRemoveKernelSobel3X3(vx_context context);
vx_status tivxRemoveKernelSub(vx_context context);
vx_status tivxRemoveKernelThreshold(vx_context context);
vx_status tivxRemoveKernelWarpAffine(vx_context context);
vx_status tivxRemoveKernelWarpPerspective(vx_context context);
vx_status tivxRemoveKernelXor(vx_context context);
vx_status tivxRemoveKernelSuperNode(vx_context context);

#endif /* TIVX_CORE_HOST_PRIV_H_ */
