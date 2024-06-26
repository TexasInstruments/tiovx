/*
*
* Copyright (c) 2018 Texas Instruments Incorporated
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




#ifndef TIVX_LOG_RT_TRACE_H_
#define TIVX_LOG_RT_TRACE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of real-time trace log API
 * \ingroup group_tivx_log_rt_trace
 */

#include <tivx_log_rt_if.h>

/** \brief Event logger queue header, structure size MUST be 64b aligned */
typedef struct {

    uint32_t rd_index;     /**< queue read index within event log */
    uint32_t wr_index;     /**< queue write index within event log */
    uint32_t count;        /**< number of element in queue */
    uint32_t rsv[1];       /**< used to aligned to 64b */

} tivx_log_rt_queue_t;

/** \brief Event logger object */
typedef struct {

    vx_bool is_valid; /**< vx_true_e: event logging is valid and can be used, else event logging is not supported */

    void    *log_rt_shm_base; /**< base address of shared memory */
    uint32_t log_rt_shm_size; /**< size of shared memory */

    tivx_log_rt_queue_t *queue; /**< pointer to queue header within shared memory */
    tivx_log_rt_index_t *index; /**< pointer to identifier index within shared memory */

    tivx_log_rt_entry_t *event_log_base; /**< pointer to event log within shared memory */
    uint32_t event_log_max_entries; /**< max possible events in event log */

} tivx_log_rt_obj_t;

/*!
 * \brief Init run-time logger module
 *
 * - Called internally during tivxInit. Should NOT be called by user
 *
 * \ingroup group_tivx_log_rt_trace
 */
void ownLogRtInit(void);

/*!
 * \brief Reset shared memory used for logging
 *
 * - Called internally during tivxInit. Should NOT be called by user
 *
 * \ingroup group_tivx_log_rt_trace
 */
void ownLogRtResetShm(void *shm_base);

/*!
 * \brief Log trace on node execute start
 *
 * \ingroup group_tivx_log_rt_trace
 */
void ownLogRtTraceNodeExeStart(uint64_t timestamp, const tivx_obj_desc_node_t *node_obj_desc);

/*!
 * \brief Log trace on node execute end
 *
 * \ingroup group_tivx_log_rt_trace
 */
void ownLogRtTraceNodeExeEnd(uint64_t timestamp, const tivx_obj_desc_node_t *node_obj_desc);

/*!
 * \brief Log trace on graph execute start
 *
 * \ingroup group_tivx_log_rt_trace
 */
void ownLogRtTraceGraphExeStart(uint64_t timestamp, const tivx_obj_desc_graph_t *graph_obj_desc);

/*!
 * \brief Log trace on graph execute end
 *
 * \ingroup group_tivx_log_rt_trace
 */
void ownLogRtTraceGraphExeEnd(uint64_t timestamp, const tivx_obj_desc_graph_t *graph_obj_desc);

/*!
 * \brief Log trace on target execute start
 *
 * \ingroup group_tivx_log_rt_trace
 */
void ownLogRtTraceTargetExeStart(tivx_target target, const tivx_obj_desc_t *obj_desc);

/*!
 * \brief Log trace on target execute end
 *
 * \ingroup group_tivx_log_rt_trace
 */
void ownLogRtTraceTargetExeEnd(tivx_target target, const tivx_obj_desc_t *obj_desc);

#ifdef __cplusplus
}
#endif

#endif
