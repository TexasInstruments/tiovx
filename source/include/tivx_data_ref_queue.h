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




#ifndef TIVX_DATA_REF_Q_H_
#define TIVX_DATA_REF_Q_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Data Reference Queue Object APIs
 */


/*!
 *

 */

/*! \brief Data Ref Queue Object
 *
 * \ingroup group_tivx_data_ref_queue
 */
typedef struct _tivx_data_ref_queue *tivx_data_ref_queue;

/*!
 * \brief Data Ref Queue internal state
 *
 * A Data Ref Queue consists of a pair of queues, ready queue and done queue
 * - Ready queue is used to enqueue refs from user to graph
 * - Done queue is used to dequeue processed refs from graph to user
 *
 * \ingroup group_tivx_data_ref_queue
 */
typedef struct _tivx_data_ref_queue {

    /*! \brief The base reference object */
    tivx_reference_t      base;
    /*! \brief object descriptor used for 'READY' queue */
    uint16_t ready_q_obj_desc_id;
    /*! \brief object descriptor used for 'DONE' queue */
    uint16_t done_q_obj_desc_id;
    /*! \brief event triggered when a data ref is inserted into 'DONE' queue */
    tivx_event wait_done_ref_available_event;
    /*! \brief obj descriptor of data ref queue */
    tivx_obj_desc_data_ref_q_t *obj_desc[TIVX_GRAPH_MAX_PIPELINE_DEPTH];
    /*! \brief reference consumed object descriptor */
    tivx_obj_desc_cmd_t *obj_desc_cmd[TIVX_GRAPH_MAX_PIPELINE_DEPTH];
    /*! \brief acquire queue ID */
    uint16_t acquire_q_obj_desc_id;
    /*! \brief release queue ID */
    uint16_t release_q_obj_desc_id;
    /*! \brief pipeline depth */
    uint32_t pipeline_depth;
    /*! \brief send event to user when data ref is consumed */
    vx_bool is_enable_send_ref_consumed_event;
    /*! \brief will user do enqueing/dequeueing or will graph rotate buffers internally */
    vx_bool enable_user_queueing;
    /*! \brief graph parameter index which is associated with this data ref */
    uint32_t graph_parameter_index;
    /*! \brief graph associated with this data reference */
    vx_graph graph;

} tivx_data_ref_queue_t;

/*!
 * \brief Parameters used to create a data ref queue
 *
 * \ingroup group_tivx_data_ref_queue
 */
typedef struct _tivx_data_ref_queue_create_params {

    /*! \brief graph pipeline depth */
    uint32_t pipeline_depth;
    /*! \brief number of nodes that use this data reference as input */
    uint32_t num_in_nodes;
    /*! \brief will user do enqueing/dequeueing or will graph rotate buffers internally */
    vx_bool enable_user_queueing;
    /*! \brief send event to user when data ref is consumed */
    vx_bool is_enable_send_ref_consumed_event;
    /*! \brief graph parameter index which is associated with this data ref */
    uint32_t graph_parameter_index;

} tivx_data_ref_queue_create_params_t;

/*!
 * \brief Create data reference queue
 *
 * \ingroup group_tivx_data_ref_queue
 */
tivx_data_ref_queue tivxDataRefQueueCreate(vx_graph graph, tivx_data_ref_queue_create_params_t *prms);

/*!
 * \brief Release data reference queue
 *
 * \ingroup group_tivx_data_ref_queue
 */
vx_status tivxDataRefQueueRelease(tivx_data_ref_queue *ref);


/*!
 * \brief return object descriptor ID of specific pipeline ID
 *
 * \ingroup group_tivx_data_ref_queue
 */
uint16_t tivxDataRefQueueGetObjDescId(tivx_data_ref_queue ref, uint32_t pipeline_id);

/*!
 * \brief Enqueue 'ref' into 'READY' queue
 *
 * if queue is full, VX_FAILURE is returned.
 *
 * NOTE, this is a non-blocking API.
 *
 * \ingroup group_tivx_data_ref_queue
 */
vx_status tivxDataRefQueueEnqueueReadyRef(tivx_data_ref_queue data_ref_q, vx_reference ref);

/*!
 * \brief Dequeue 'ref' from 'DONE' queue
 *
 * if queue is empty VX_FAILURE is returned and 'ref' is set to NULL
 *
 * NOTE, this is a non-blocking API.
 *
 * \ingroup group_tivx_data_ref_queue
 */
vx_status tivxDataRefQueueDequeueDoneRef(tivx_data_ref_queue data_ref_q, vx_reference *ref);


/*!
 * \brief Wait for 'DONE' queue to be non-empty
 *
 * if 'timeout' units of time elaspes then VX_FAILURE is returned
 *
 * NOTE, this is a blocking API and block for 'timeout' units of time
 *
 * \ingroup group_tivx_data_ref_queue
 */
vx_status tivxDataRefQueueWaitDoneRef(tivx_data_ref_queue data_ref_q, vx_uint32 timeout);


/*!
 * \brief Return number of elements in 'DONE' queue
 *
 * \ingroup group_tivx_data_ref_queue
 */
vx_status tivxDataRefQueueGetDoneQueueCount(tivx_data_ref_queue data_ref_q, vx_uint32 *count);

/*!
 * \brief Return number of elements in 'READY' queue
 *
 * \ingroup group_tivx_data_ref_queue
 */
vx_status tivxDataRefQueueGetReadyQueueCount(tivx_data_ref_queue data_ref_q, vx_uint32 *count);

/*!
 * \brief Send 'ref consumed event' if event send is enabled
 *
 * \ingroup group_tivx_data_ref_queue
 */
vx_status tivxDataRefQueueSendRefConsumedEvent(tivx_data_ref_queue ref, uint64_t timestamp);


/*!
 * \brief Link data ref queues that participate in a delay in a circular manner
 *
 * \ingroup group_tivx_data_ref_queue
 */
vx_status tivxDataRefQueueLinkDelayDataRefQueues(
            tivx_data_ref_queue delay_data_ref_q_list[],
            vx_bool auto_age_delay_slot[],
            uint32_t delay_slots);

#ifdef __cplusplus
}
#endif

#endif
