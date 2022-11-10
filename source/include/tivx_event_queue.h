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



#ifndef TIVX_EVENT_QUEUE_H_
#define TIVX_EVENT_QUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to Event Queue APIs
 *
 *        Event Queue can be used to collect framework generated events
 *        into a queue.
 *        When user calls vxWaitEvent the event is returned
 *        to the user in a first-in first out manner
 */

/*! \brief Element inserted into event queue
 * \ingroup group_tivx_event_queue
 */
typedef struct _tivx_event_queue_elem_t
{
    vx_enum   event_id;
    uint64_t  timestamp;
    uint32_t  app_value;
    uintptr_t param1;
    uintptr_t param2;
    uintptr_t param3;
} tivx_event_queue_elem_t;

/*!
 * \brief Event queue object
 *
 * \ingroup group_tivx_event_queue
 */
typedef struct _tivx_event_queue_t
{
    /*! \brief list of events */
    tivx_event_queue_elem_t event_list[TIVX_EVENT_QUEUE_MAX_SIZE];

    /*! \brief handle to free queue holding tivx_event_queue_elem_t's
     * NOTE: queue holds index's to event_list[]
     * */
    tivx_queue free_queue;

    /*! \brief free queue memory */
    uintptr_t free_queue_memory[TIVX_EVENT_QUEUE_MAX_SIZE];

    /*! \brief handle to ready queue holding tivx_event_queue_elem_t's which are ready to be delivered to users
     * NOTE: queue holds index's to event_list[]
     * */
    tivx_queue ready_queue;

    /*! \brief free queue memory */
    uintptr_t ready_queue_memory[TIVX_EVENT_QUEUE_MAX_SIZE];

    /*! \brief flag to control enable/disable of event addition to event queue  */
    vx_bool enable;

} tivx_event_queue_t;


/*! \brief Type of event that can be generated during system execution
 *
 * \ingroup group_tivx_event_queue
 */
enum tivx_queue_type_e {

    /*! \brief Graph event queue
     *
     * The registered event will be used in the graph event queue
     */
    TIVX_EVENT_GRAPH_QUEUE = VX_ATTRIBUTE_BASE(VX_ID_TI, (int32_t)0) + 0x1,

    /*! \brief Context event queue
     *
     * The registered event will be used in the context event queue
     */
    TIVX_EVENT_CONTEXT_QUEUE = VX_ATTRIBUTE_BASE(VX_ID_TI, (int32_t)0) + 0x2
};

/*!
 * \brief Create a event queue
 *
 * \return event queue handle on success, else error
 *
 * \ingroup group_tivx_event_queue
 */
vx_status ownEventQueueCreate(tivx_event_queue_t *event_q);


/*!
 * \brief Delete a event queue
 *
 * \return event queue handle on success, else error
 *
 * \ingroup group_tivx_event_queue
 */
void ownEventQueueDelete(tivx_event_queue_t *event_q);

/*!
 * \brief Add event to event queue
 *
 * When events are disabled, event is not added to the event_q
 *
 * \ingroup group_tivx_event_queue
 */
vx_status ownEventQueueAddEvent(tivx_event_queue_t *event_q,
        vx_enum event_id, uint64_t timestamp, uint32_t app_value, uintptr_t param1, uintptr_t param2, uintptr_t param3);


/*!
 * \brief Enable an event queue
 *
 * \ingroup group_tivx_event_queue
 */
void ownEventQueueEnableEvents(tivx_event_queue_t *event_q, vx_bool enable);

/*!
 * \brief Generic wait event queue API
 *
 * \ingroup group_tivx_event_queue
 */
vx_status vxWaitEventQueue(
                    tivx_event_queue_t *event_q, vx_event_t *event,
                    vx_bool do_not_block);

/*! \brief Registers an event to a given event queue
 *
 * \ingroup group_tivx_event_queue
 */
VX_API_ENTRY vx_status VX_API_CALL ownRegisterEvent(vx_reference ref,
                enum tivx_queue_type_e queue_type, enum vx_event_type_e type,
                vx_uint32 param, vx_uint32 app_value);

#ifdef __cplusplus
}
#endif

#endif
