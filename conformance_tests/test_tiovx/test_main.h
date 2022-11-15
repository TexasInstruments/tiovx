/*

 * Copyright (c) 2012-2017 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#if defined(BUILD_CT_TIOVX)
TESTCASE(tiovxPerformance)
TESTCASE(tiovxPerformance2)

#if defined(BUILD_BAM)
TESTCASE(tiovxSupernodePerformance)
TESTCASE(tiovxSupernodePerformance2)
TESTCASE(tivxSuperNode)
#endif

TESTCASE(tivxGraph)
TESTCASE(tivxGraphDelay)
TESTCASE(tivxGraphMultiThreaded)
TESTCASE(tivxAccumulate)
TESTCASE(tivxAccumulateSquare)
TESTCASE(tivxAccumulateWeighted)
TESTCASE(tivxArray)
TESTCASE(tivxAddSub)
TESTCASE(tivxBinOp8u)
TESTCASE(tivxBinOp16s)
TESTCASE(tivxBox3x3)
TESTCASE(tivxChannelCombine)
TESTCASE(tivxChannelExtractCombine)
TESTCASE(tivxColorConvert)
TESTCASE(tivxContext)
TESTCASE(tivxConvertDepth)
TESTCASE(tivxConvolve)
TESTCASE(tivxCreate)
TESTCASE(tivxDebug)
TESTCASE(tivxDelay)
TESTCASE(tivxDilate3x3)
TESTCASE(tivxDistribution)
TESTCASE(tivxEqualizeHistogram)
TESTCASE(tivxErode3x3)
TESTCASE(tivxEventQueue)
TESTCASE(tivxFastCorners)
TESTCASE(tivxGaussian3x3)
TESTCASE(tivxGraphExportDot)
TESTCASE(tivxHalfScaleGaussian)
TESTCASE(tivxHarrisCorners)
TESTCASE(tivxHistogram)
TESTCASE(tivxImage)
TESTCASE(tivxIntegral)
TESTCASE(tivxKernel)
TESTCASE(tivxLog)
TESTCASE(tivxLUT)
TESTCASE(tivxMagnitude)
TESTCASE(tivxMatx)
TESTCASE(tivxMeanStdDev)
TESTCASE(tivxMedian3x3)
TESTCASE(tivxMetaFormat)
TESTCASE(tivxMinMaxLoc)
TESTCASE(tivxModule)
TESTCASE(tivxMultiply)
TESTCASE(tivxNode)
TESTCASE(tivxNodeApi)
TESTCASE(tivxNot)
TESTCASE(tivxObjArray)
TESTCASE(tivxOptFlowPyrLK)
TESTCASE(tivxParameter)
TESTCASE(tivxPhase)
TESTCASE(tivxPymd)
TESTCASE(tivxScalar)
TESTCASE(tivxScale)
TESTCASE(tivxSobel3x3)
TESTCASE(tivxStreamGraph)
TESTCASE(tivxTgKrnlInst)
TESTCASE(tivxTensor)
TESTCASE(tivxThreshold)
TESTCASE(tivxUserDataObject)
TESTCASE(tivxWarpAffine)
TESTCASE(tivxWarpPerspective)
TESTCASE(tivxCanny)
TESTCASE(tivxNonLinearFilter)
TESTCASE(tivxGaussianPyramid)
TESTCASE(tivxBoundary)
TESTCASE(tivxBoundary2)
TESTCASE(tivxNegativeBoundary)
TESTCASE(tivxNegativeBoundary2)
TESTCASE(tivxRemap)
TESTCASE(tivxReplicate)
TESTCASE(tivxLaplacianPyramid)
TESTCASE(tivxLaplacianReconstruct)
TESTCASE(tivxMaxNodes)
TESTCASE(tivxGraphPipeline)
TESTCASE(tivxGraphPipelineLdra)
TESTCASE(tivxPackedDataFormat)
TESTCASE(tivxRawImage)
TESTCASE(tivxReference)
TESTCASE(tivxBmpRdWr)
TESTCASE(tivxMem)
TESTCASE(tivxMapImage)
#if !defined(SOC_AM62A)
TESTCASE(tivxNestedUserNode)
#endif
TESTCASE(tivxTgKnl)
#endif

