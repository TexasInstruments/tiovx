/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_QUEUE_H_
#define _TIVX_QUEUE_H_

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
typedef struct _tivx_queue_t *tivx_queue;

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
vx_status tivxQueuePut(tivx_queue queue, uint32_t data, uint32_t timeout);

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
vx_status tivxQueueGet(tivx_queue queue, uint32_t *data, uint32_t timeout);






#ifdef __cplusplus
}
#endif

#endif
