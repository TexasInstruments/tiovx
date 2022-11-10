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




#ifndef TIVX_OBJ_DESC_QUEUE_H_
#define TIVX_OBJ_DESC_QUEUE_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Object descriptor Queue Object APIs
 *
 * NOTE: The APIs in this module dont do any locking for Q/DQ and other operations.
 *       User of this API should take local CPU and/or multi-CPU locks
 */

/*!
 * \brief Create a object descriptor queue and return its obj_desc_id
 *
 * \ingroup tivx_obj_desc_queue
 */
vx_status ownObjDescQueueCreate(uint16_t *obj_desc_id);

/*!
 * \brief Release a object descriptor queue
 *
 * \ingroup tivx_obj_desc_queue
 */
vx_status ownObjDescQueueRelease(uint16_t *obj_desc_id);


/*!
 * \brief Enqueue a obj_desc_id into a object descriptor queue
 *
 * \ingroup tivx_obj_desc_queue
 */
vx_status ownObjDescQueueEnqueue(uint16_t obj_desc_q_id, uint16_t obj_desc_id);

/*!
 * \brief Get number of elements in a object descriptor queue
 *
 * \ingroup tivx_obj_desc_queue
 */
vx_status ownObjDescQueueGetCount(uint16_t obj_desc_q_id, uint32_t *count);

/*!
 * \brief Dequeue a obj_desc_id from a object descriptor queue
 *
 * \ingroup tivx_obj_desc_queue
 */
vx_status ownObjDescQueueDequeue(uint16_t obj_desc_q_id, uint16_t *obj_desc_id);

/*!
 * \brief Add 'node_id' to a list of blocked nodes associated with this object descriptor ID
 *
 * \ingroup tivx_obj_desc_queue
 */
vx_status ownObjDescQueueAddBlockedNode(uint16_t obj_desc_q_id, uint16_t node_id);

/*!
 * \brief Extract nodes blocked on this object descritor queue ID
 *
 * Adds to out_blocked_nodes, make sure out_blocked_nodes->num_nodes is set to 0
 * before calling this API
 *
 * \ingroup tivx_obj_desc_queue
 */
vx_status ownObjDescQueueExtractBlockedNodes(uint16_t obj_desc_q_id,
            tivx_obj_desc_queue_blocked_nodes_t *out_blocked_nodes);

#ifdef __cplusplus
}
#endif

#endif
