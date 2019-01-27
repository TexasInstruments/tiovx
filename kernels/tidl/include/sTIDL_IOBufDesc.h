/*
*
* Copyright (c) {2015 - 2019} Texas Instruments Incorporated
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


/**
@file      sTIDL_IOBufDesc.h
@brief     This file defines the structure sTIDL_IOBufDesc_t to maintain cross-compatibility with next version of TI-DL
@version 0.1 Dec 2018 : Initial Code
*/

/** @ingroup    TI_DL */
/*@{*/
#ifndef STIDL_IOBUFDESC_H
#define STIDL_IOBUFDESC_H

#include "itidl_ti.h"

/**
@struct  sTIDL_IOBufDesc_t
@brief   This structure defines the Input and output buffer descriptors
         required for a given Layer group
*/
typedef struct
{
  /** Numbner of Input buffer required by the Layer group  */
  int32_t numInputBuf;
  /** Numbner of Output buffer required by the Layer group  */
  int32_t numOutputBuf;
  /** Feature width of each input buffer */
  int32_t inWidth[TIDL_MAX_ALG_IN_BUFS];
  /** Feature Height of each input buffer */
  int32_t inHeight[TIDL_MAX_ALG_IN_BUFS];
  /** Number of channels in each input buffer */
  int32_t inNumChannels[TIDL_MAX_ALG_IN_BUFS];
  /** Left zero padding required for each input buffer */
  int32_t inPadL[TIDL_MAX_ALG_IN_BUFS];
  /** Top zero padding required for each input buffer */
  int32_t inPadT[TIDL_MAX_ALG_IN_BUFS];
  /** Right zero padding required for each input buffer */
  int32_t inPadR[TIDL_MAX_ALG_IN_BUFS];
  /** Bottom zero padding required for each input buffer */
  int32_t inPadB[TIDL_MAX_ALG_IN_BUFS];
  /** Element type of each input buffer @ref eTIDL_ElementType */
  int32_t inElementType[TIDL_MAX_ALG_IN_BUFS];
  /** Data ID as per Net structure for each input buffer */
  int32_t inDataId[TIDL_MAX_ALG_IN_BUFS];
  /** Feature width of each output buffer */
  int32_t outWidth[TIDL_MAX_ALG_OUT_BUFS];
  /** Feature Height of each output buffer */
  int32_t outHeight[TIDL_MAX_ALG_OUT_BUFS];
  /** Number of channels in each output buffer */
  int32_t outNumChannels[TIDL_MAX_ALG_OUT_BUFS];
  /** Left zero padding required for each output buffer */
  int32_t outPadL[TIDL_MAX_ALG_OUT_BUFS];
  /** top zero padding required for each output buffer */
  int32_t outPadT[TIDL_MAX_ALG_OUT_BUFS];
  /** Right zero padding required for each output buffer */
  int32_t outPadR[TIDL_MAX_ALG_OUT_BUFS];
  /** Bottom zero padding required for each output buffer */
  int32_t outPadB[TIDL_MAX_ALG_OUT_BUFS];
  /** Element type of each output buffer @ref eTIDL_ElementType */
  int32_t outElementType[TIDL_MAX_ALG_OUT_BUFS];
  /** Data ID as per Net structure for each output buffer */
  int32_t outDataId[TIDL_MAX_ALG_OUT_BUFS];
} sTIDL_IOBufDesc_t;

#endif
