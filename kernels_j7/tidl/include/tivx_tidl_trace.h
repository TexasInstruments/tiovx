/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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
  * \file tivx_utils_tidl_trace.h Utility APIs to set and get TIDL trace data
  */

#ifndef TIVX_TIDL_TRACE_H_
#define TIVX_TIDL_TRACE_H_

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIVX_TIDL_TRACE_FILE_NAME_SIZE (512)

/*!
 * \brief TIDL trace params structure
 * \ingroup group_vision_function_tidl_trace
 */
typedef struct
{
  /** File name for intermediate layer data */
  char fileName[TIVX_TIDL_TRACE_FILE_NAME_SIZE];
  /** Offset in the trace buffer where data is present */
  uint32_t offset;
  /** Size of the trace data */
  uint32_t size;

} tivxTIDLTraceHeader;

/*!
 * \brief TIDL trace params structure
 * \ingroup group_vision_function_tidl_trace
 */
typedef struct
{
  /** Base pointer of trace buffer */
  uint8_t *base;
  /** Current pointer in trace buffer */
  uint8_t *current;
  /** Total capacity of the buffer */
  uint32_t buffer_capacity;
  /** Current capacity of the buffer */
  uint32_t current_capacity;

} tivxTIDLTraceDataManager;

/*!
 * \brief TIDL trace clear - clears out trace manager variables
 * \ingroup group_vision_function_tidl_trace
 */
static inline void tivxTIDLTraceDataClear(tivxTIDLTraceDataManager *mgr)
 {
     mgr->base = NULL;
     mgr->current = NULL;
     mgr->buffer_capacity = 0;
     mgr->current_capacity = 0;
 }

/*!
 * \brief TIDL trace init - initializes trace manager variables with provided buffer capacity.
 * \ingroup group_vision_function_tidl_trace
 */
 static inline void tivxTIDLTraceDataInit(tivxTIDLTraceDataManager *mgr, uint8_t *base, uint64_t buffer_capacity)
 {
     mgr->base = base;
     mgr->current = base;
     mgr->buffer_capacity = buffer_capacity;
     mgr->current_capacity = 0;
 }

/*!
 * \brief TIDL trace set - writes trace header/payload to the trace buffer.
 * \ingroup group_vision_function_tidl_trace
 */
 static inline void tivxTIDLTraceSetData(tivxTIDLTraceDataManager *mgr, uint8_t *data_ptr, uint64_t data_size)
 {
   if((mgr->current_capacity + data_size) < mgr->buffer_capacity)
   {
     memcpy(mgr->current, data_ptr, data_size);
     mgr->current += data_size;
     mgr->current_capacity += data_size;
   }
 }

/*!
 * \brief TIDL trace set - provides a pointer to the requested header/paylaod by byte offset
 * \ingroup group_vision_function_tidl_trace
 */
 static inline uint8_t *tivxTIDLTraceGetData(tivxTIDLTraceDataManager *mgr, uint64_t offset, uint64_t data_size)
 {
   uint8_t *data_ptr = NULL;

   if((offset + data_size) < mgr->buffer_capacity)
   {
     data_ptr =  mgr->base + offset;
   }

   return data_ptr;
 }

 /*!
  * \brief TIDL trace EOB - indicates End Of Buffer
  * \ingroup group_vision_function_tidl_trace
  */
static inline void tivxTIDLTraceWriteEOB(tivxTIDLTraceDataManager *mgr)
{
 tivxTIDLTraceHeader header;

 strcpy(header.fileName, "EOB");
 header.size   = 0x00000E0B;
 header.offset = 0x00000E0B;

 tivxTIDLTraceSetData(mgr, (uint8_t *)&header, sizeof(tivxTIDLTraceHeader));
}

#ifdef __cplusplus
}
#endif

#endif /* TIVX_TIDL_TRACE_H_ */
