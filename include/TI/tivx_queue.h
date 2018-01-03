/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
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



#ifndef TIVX_QUEUE_H_
#define TIVX_QUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to Queue APIs
 */

/*!
 * \brief Flag to indicate queue should block 'put' operation
 *        until free space is available for 'put' to succeed
 *
 * \ingroup group_tivx_queue
 */
#define TIVX_QUEUE_FLAG_BLOCK_ON_PUT        (0x00000001u)

/*!
 * \brief Flag to indicate queue should block 'get' operation
 *        until new element is available in queue to extract
 *
 * \ingroup group_tivx_queue
 */
#define TIVX_QUEUE_FLAG_BLOCK_ON_GET        (0x00000002u)



/*!
 * \brief Typedef for a queue
 *
 * \ingroup group_tivx_queue
 */
typedef struct _tivx_queue_t {

  uint32_t cur_rd;
  /**< Current read index */

  uint32_t cur_wr;
  /**< Current write index  */

  uint32_t count;
  /**< Count of element in queue  */

  uint32_t max_ele;
  /**< Max elements that be present in the queue  */

  uint32_t *queue;
  /**< Address of data area of the queue elements */

  tivx_event block_rd;
  /**< Read semaphore */

  tivx_event block_wr;
  /**< Write semaphore  */

  tivx_mutex lock;
  /**< Queue lock semaphore  */

  void *context;
  /**< Private context of queue handle */

  uint32_t flags;
  /**< Controls how APIs behave internally, i.e blocking wait or non-blocking */

  volatile vx_bool blockedOnGet;
  /**< Flag indicating queue is blocked on get operation */

  volatile vx_bool blockedOnPut;
  /**< Flag indicating queue is blocked on put operation */

} tivx_queue;

/*!
 * \brief Create a queue
 *
 * \param queue [out] Pointer to queue object
 * \param max_elements [in] Max elements in a queue
 * \param queue_memory [in] Pointer to queue memory, queue can only contain elements of size 32bits
 * \param flags [in] Flags to use during queue creation, see TIVX_QUEUE_FLAG_xxx
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_queue
 */
vx_status tivxQueueCreate(tivx_queue *queue, uint32_t max_elements, uint32_t *queue_memory, uint32_t flags);

/*!
 * \brief Delete a queue
 *
 * \param queue [in] Pointer to queue object
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_queue
 */
vx_status tivxQueueDelete(tivx_queue *queue);

/*!
 * \brief Add a element into the queue
 *
 * \param queue [in] Pointer to queue object
 * \param data [in] element to add
 * \param timeout [in] Amount of time to wait,
 *            TIVX_QUEUE_FLAG_BLOCK_ON_PUT msut be set during create,
 *            else function does not wait and returns error if queue is full
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_queue
 */
vx_status tivxQueuePut(tivx_queue *queue, uint32_t data, uint32_t timeout);

/*!
 * \brief Extract a element from the queue
 *
 * \param queue [in] Pointer to queue object
 * \param data [out] element that is extracted
 * \param timeout [in] Amount of time to wait,
 *            TIVX_QUEUE_FLAG_BLOCK_ON_GET msut be set during create,
 *            else function does not wait and returns error if queue is empty
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_queue
 */
vx_status tivxQueueGet(tivx_queue *queue, uint32_t *data, uint32_t timeout);

/*!
 * \brief Peek an element from the queue but dont extract it
 *
 * \param queue [in] Pointer to queue object
 * \param data [out] element that is 'peeked'
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_queue
 */
vx_status tivxQueuePeek(tivx_queue *queue, uint32_t *data);

/*!
 * \brief Check if queue is empty
 *
 * \param queue [in] Pointer to queue object
 *
 * \return vx_true_e if queue is empty, else vx_false_e
 *
 * \ingroup group_tivx_queue
 */
vx_bool tivxQueueIsEmpty(tivx_queue *queue);

#ifdef __cplusplus
}
#endif

#endif
