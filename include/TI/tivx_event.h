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
