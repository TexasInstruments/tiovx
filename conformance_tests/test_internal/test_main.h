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


#if defined(BUILD_CT_TIOVX_INTERNAL)

#if defined(BUILD_TEST_KERNELS)

TESTCASE(tivxInternalNode)

TESTCASE(tivxInternalGraphVerify)
#endif /* #if defined(BUILD_TEST_KERNELS) */

TESTCASE(tivxInternalArray)

TESTCASE(tivxObjDescBoundary)

TESTCASE(tivxInternalContext)

TESTCASE(tivxInternalDataRefQueue)

TESTCASE(tivxInternaldelay)

TESTCASE(tivxInternalError)

TESTCASE(tivxInternalEventQueue)

TESTCASE(tivxInternalGraph)

TESTCASE(tivxInternalGraphPipeline)

TESTCASE(tivxInternalGraphSort)

TESTCASE(tivxInternalGraphStream)

TESTCASE(tivxInternalImage)

TESTCASE(tivxInternalApis)

TESTCASE(tivxInternalKernel)

TESTCASE(tivxInternalLUT)

TESTCASE(tivxInternalMatrix)

TESTCASE(tivxInternalMetaFormat)

TESTCASE(tivxInternalObjArray)

TESTCASE(tivxInternalObjects)

TESTCASE(tivxInternalParameter)

TESTCASE(tivxInternalPyramid)

TESTCASE(tivxInternalRawImage)

TESTCASE(tivxInternalReference)

TESTCASE(tivxReferenceLock)

TESTCASE(tivxInternalSafeCasts)

TESTCASE(tivxInternalTensor)

#if defined(A72) || defined(A53) || defined(PC)
TESTCASE(tivxPosixObjects)
#endif

TESTCASE(tivxInternalUserDataObject)

#if defined(LINUX) && !defined(PC)
TESTCASE(tivxRpmsgChar)
#endif

#if defined(A72) || defined(A53)
TESTCASE(tivxDmaHeap)

TESTCASE(tivxTimer)
#endif

#endif
