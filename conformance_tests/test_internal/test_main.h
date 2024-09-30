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
TESTCASE(tivxObjDescBoundary)
TESTCASE(tivxInternalObjArray)
TESTCASE(tivxInternalApis)
TESTCASE(tivxInternalObjects)
TESTCASE(tivxInternalContext)
TESTCASE(tivxInternalUserDataObject)
TESTCASE(tivxInternalRawImage)
TESTCASE(tivxInternalError)
TESTCASE(tivxReferenceLock)
TESTCASE(tivxInternalPyramid)
TESTCASE(tivxInternalParameter)
TESTCASE(tivxInternalTensor)
TESTCASE(tivxInternalGraph)
TESTCASE(tivxInternalGraphStream)
TESTCASE(tivxInternalGraphVerify)
TESTCASE(tivxInternalNode)
TESTCASE(tivxInternalImage)
TESTCASE(tivxInternalArray)
TESTCASE(tivxInternalLUT)
TESTCASE(tivxInternalMatrix)
TESTCASE(tivxInternalKernel)
TESTCASE(tivxInternaldelay)
#if defined(A72) || defined(A53) || defined(PC)
TESTCASE(tivxPosixObjects)
#endif
TESTCASE(tivxInternalMetaFormat)
TESTCASE(tivxInternalGraphPipeline)
TESTCASE(tivxInternalDataRefQueue)
TESTCASE(tivxInternalEventQueue)
TESTCASE(tivxInternalReference)

#if defined(A72) || defined(A53)
TESTCASE(tivxDmaHeap)
TESTCASE(tivxTimer)
#endif

#if defined(LINUX) && !defined(PC)
TESTCASE(tivxRpmsgChar)
#endif
TESTCASE(tivxInternalGraphSort)
TESTCASE(tivxInternalSafeCasts)

#endif
