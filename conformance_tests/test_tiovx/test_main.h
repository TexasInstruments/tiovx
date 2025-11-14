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
/*
 * Copyright (c) 2025 Texas Instruments Incorporated
 */

#if defined(BUILD_CT_TIOVX)
#if defined(BUILD_CORE_KERNELS)
TESTCASE(tivxAccumulate)
TESTCASE(tivxAccumulateSquare)
TESTCASE(tivxAccumulateWeighted)

TESTCASE(tivxAddSub)

TESTCASE(bpExtFramework)

TESTCASE(bpExtStandardNodes)

TESTCASE(tivxBinOp16s)

TESTCASE(tivxBinOp8u)

TESTCASE(tivxBmpRdWr)

TESTCASE(tivxBox3x3)

TESTCASE(tivxCanny)

TESTCASE(tivxChannelCombine)

TESTCASE(tivxChannelExtractCombine)

TESTCASE(tivxContext)

TESTCASE(tivxColorConvert)

TESTCASE(tivxConvertDepth)

TESTCASE(tivxConvolve)

TESTCASE(copySwap)
TESTCASE(tivxCreate)

TESTCASE(tivxDebug)

TESTCASE(tivxDelay)

TESTCASE(tivxDilate3x3)

TESTCASE(tivxDistribution)

TESTCASE(tivxEqualizeHistogram)

TESTCASE(tivxErode3x3)

TESTCASE(tivxEventQueue)

TESTCASE(tivxExemplar)

TESTCASE(tivxFastCorners)

TESTCASE(tivxGaussian3x3)

TESTCASE(tivxGaussianPyramid)

TESTCASE(tivxGraph)
TESTCASE(tivxGraphDelay)
TESTCASE(tivxGraphMultiThreaded)
TESTCASE(tivxGraphPipelineLdra)
TESTCASE(tivxStreamGraph)

TESTCASE(tivxHalfScaleGaussian)

TESTCASE(tivxHarrisCorners)

TESTCASE(tivxHistogram)

TESTCASE(tivxIntegral)

TESTCASE(tivxKernel)

TESTCASE(tivxLaplacianPyramid)
TESTCASE(tivxLaplacianReconstruct)

TESTCASE(tivxLUT)

TESTCASE(tivxMagnitude)

TESTCASE(tivxMaxNodes)

TESTCASE(tivxMeanStdDev)

TESTCASE(tivxMedian3x3)

TESTCASE(tivxMemBoundary)

TESTCASE(tivxMetaFormat)

TESTCASE(tivxMinMaxLoc)

TESTCASE(tivxModule)

TESTCASE(tivxMultiply)

#if !defined(SOC_AM62A)
TESTCASE(tivxNestedUserNode)
#endif

TESTCASE(tivxNode)

TESTCASE(tivxNodeApi)

TESTCASE(tivxNonLinearFilter)

TESTCASE(tivxNot)

TESTCASE(tivxObjDesc)

TESTCASE(tivxOptFlowPyrLK)

TESTCASE(tivxPackedDataFormat)

TESTCASE(tivxParameter)

TESTCASE(tiovxPerformance)
TESTCASE(tiovxPerformance2)

TESTCASE(tivxPhase)

TESTCASE(tivxPymd)

TESTCASE(tivxRawImage)

TESTCASE(tivxReference)

TESTCASE(tivxRemap)

TESTCASE(tivxReplicate)

TESTCASE(tivxSafeCasts)

TESTCASE(tivxScale)

TESTCASE(tivxSobel3x3)

TESTCASE(supplementary_data)

TESTCASE(tivxTgKnl)

TESTCASE(tivxTgKrnlInst)

TESTCASE(tivxTensor)

TESTCASE(tivxThreshold)

TESTCASE(tivxQueue)

TESTCASE(tivxMem)

TESTCASE(tivxUserDataObject)

TESTCASE(tivxWarpAffine)

TESTCASE(tivxWarpPerspective)

TESTCASE(tivxMapImage)
#if defined(BUILD_BAM)
TESTCASE(tiovxSupernodePerformance)
TESTCASE(tiovxSupernodePerformance2)
TESTCASE(tivxSuperNode)
#endif

#if defined(BUILD_DEV)
TESTCASE(tivxBoundary)
TESTCASE(tivxBoundary2)
TESTCASE(tivxBoundaryFrameworkTest)
TESTCASE(tivxNegativeBoundary)
TESTCASE(tivxNegativeBoundary2)

TESTCASE(tivxGraphExportDot)
TESTCASE(tivxGraphPipeline)
TESTCASE(tivxGraphPipeline2)
#endif /* #if defined(BUILD_DEV) */

#endif /* #if defined(BUILD_CORE_KERNELS) */

TESTCASE(tivxArray)

TESTCASE(tivxLog)

TESTCASE(tivxMatx)

TESTCASE(tivxObjArray)

TESTCASE(tivxScalar)

TESTCASE(tivxImage)

TESTCASE(tivxImage2)

TESTCASE(ObjectArrayFromList)

TESTCASE(tivxObjArrayFromList)

#endif
