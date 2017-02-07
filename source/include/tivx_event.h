/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_EVENT_H_
#define TIVX_EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to Event APIs
 */


/*!
 * \brief Constant to indicate tivxEventPend() should only
 *        check and not wait for event to arrive
 *
 * \ingroup group_tivx_event
 */
#define TIVX_EVENT_TIMEOUT_NO_WAIT          (0u)

/*!
 * \brief Constant to indicate tivxEventPend() should only
 *        wait forever for the event to arrive
 *
 * \ingroup group_tivx_event
 */
#define TIVX_EVENT_TIMEOUT_WAIT_FOREVER     (0xFFFFFFFFu)


/*!
 * \brief Typedef for a event
 *
 * \ingroup group_tivx_event
 */
typedef struct _tivx_event_t *tivx_event;


/*!
 * \brief Create a event
 *
 * \param event [out] Pointer to event object
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_event
 */
vx_status tivxEventCreate(tivx_event *event);

/*!
 * \brief Delete a event
 *
 * \param event [in] Pointer to event object
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_event
 */
vx_status tivxEventDelete(tivx_event *event);

/*!
 * \brief Post a event
 *
 * \param event [in] event object
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_event
 */
vx_status tivxEventPost(tivx_event event);

/*!
 * \brief Pend on a event
 *
 * \param event [in] event object
 * \param timeout [in] Timeout in units of msecs,
 *                     use TIVX_EVENT_TIMEOUT_WAIT_FOREVER to wait forever
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_event
 */
vx_status tivxEventWait(tivx_event event, uint32_t timeout);

/*!
 * \brief Clear any pending events
 *
 * \param event [in] event object
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_event
 */
vx_status tivxEventClear(tivx_event event);

#ifdef __cplusplus
}
#endif

#endif
