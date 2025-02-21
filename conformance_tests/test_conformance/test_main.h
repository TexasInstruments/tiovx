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

#if defined(BUILD_CT_KHR)
#if defined(BUILD_CORE_KERNELS)
TESTCASE(Graph)
TESTCASE(GraphCallback)
TESTCASE(GraphDelay)
TESTCASE(GraphROI)

TESTCASE(vxCreateImageFromChannel)

TESTCASE(UserNode)
TESTCASE(SmokeTest)
TESTCASE(Target)
TESTCASE(vxuConvertDepth)
TESTCASE(vxConvertDepth)
TESTCASE(ChannelCombine)
TESTCASE(ChannelExtract)
TESTCASE(ColorConvert)
TESTCASE(vxuAddSub)
TESTCASE(vxAddSub)
TESTCASE(vxuNot)
TESTCASE(vxNot)

TESTCASE(vxuBinOp8u)
TESTCASE(vxBinOp8u)

TESTCASE(vxuBinOp16s)
TESTCASE(vxBinOp16s)

TESTCASE(vxuMultiply)
TESTCASE(vxMultiply)
TESTCASE(Histogram)
TESTCASE(EqualizeHistogram)
TESTCASE(MeanStdDev)
TESTCASE(MinMaxLoc)

TESTCASE(Threshold)
TESTCASE(Box3x3)
TESTCASE(Convolve)
TESTCASE(Dilate3x3)
TESTCASE(Erode3x3)

TESTCASE(Gaussian3x3)
TESTCASE(Median3x3)
TESTCASE(Sobel3x3)
TESTCASE(NonLinearFilter)
TESTCASE(Integral)

TESTCASE(Magnitude)
TESTCASE(Phase)
TESTCASE(FastCorners)
TESTCASE(HarrisCorners)
TESTCASE(Scale)
TESTCASE(WarpAffine)
TESTCASE(WarpPerspective)
TESTCASE(Remap)

TESTCASE(GaussianPyramid)
TESTCASE(HalfScaleGaussian)
TESTCASE(LaplacianPyramid)
TESTCASE(LaplacianReconstruct)
TESTCASE(vxuCanny)
TESTCASE(vxCanny)
TESTCASE(OptFlowPyrLK)
TESTCASE(LUT)

TESTCASE(Accumulate)
TESTCASE(AccumulateSquare)
TESTCASE(AccumulateWeighted)
#endif /* #if defined(BUILD_CORE_KERNELS) */

TESTCASE(Array)

TESTCASE(Convolution)

TESTCASE(Logging)

TESTCASE(Matrix)

TESTCASE(ObjectArray)

TESTCASE(Scalar)

TESTCASE(Image)
TESTCASE(vxCopyImagePatch)
TESTCASE(vxMapImagePatch)

#ifdef OPENVX_USE_USER_DATA_OBJECT
TESTCASE(UserDataObject)
#endif

#endif
