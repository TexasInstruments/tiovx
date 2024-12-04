/*
 * Copyright (c) 2024 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _VX_KHR_PRODUCER_H_
#define _VX_KHR_PRODUCER_H_

/*!
 * \file
 * \brief The OpenVX producer extension API.
 */

#define OPENVX_KHR_PRODUCER  "vx_khr_producer"

#include <VX/vx.h>
#include <vx_gw_common.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*! \brief The Producer Object. Producer Object is a strongly-typed container for other data structures.
 * \ingroup group_vx_producer
 */
typedef struct _vx_producer *vx_producer;

/*! \brief The object type enumeration for producer object
 * \ingroup group_vx_producer
 */
#define VX_TYPE_PRODUCER 0x819 /*!< \brief A <tt>\ref vx_producer</tt>. */

/*! \brief Defines the length of the producer name string, including the trailing zero.
 * \ingroup group_vx_producer
 */
#define VX_MAX_PRODUCER_NAME 32

/*! \brief Defines the length of the access point name string, including the trailing zero.
 * \ingroup group_vx_producer
 */
#define VX_MAX_ACCESS_POINT_NAME 32

/**
 * \brief Custom callback function for dequeuing references from the producer graph.
 *
 * This function is called by the producer server thread waiting for new references to send to the consumer(s).
 *
 * \param [in]  graph_obj Pointer to the producer graph object (typically a structure with graph information)
 * \param [out] dequeued_refs An array of dequeued references.
 * \param [out] num_dequeued_refs The number of dequeued references.
 *
 * \return A status of the operation. If the dequeue is successful, the function will return VX_SUCCESS.
 *         if the returned status is not VX_SUCCESS, the producer will not send any buffer to the consumer(s)
 * 
 * \ingroup group_vx_producer
 */
typedef vx_status (*vxProducerDequeueCallback)(
    void*        graph_obj,
    vx_reference dequeued_refs[],
    vx_uint32*   num_dequeued_refs);

/**
 * \brief Custom callback function to enqueue references into the producer graph.
 *
 * This function is called by the producer server thread when one (or more) buffer(s) has(have) been released by
 * (all) the consumer(s) and get it returned back to the producer graph.
 *
 * \param [in] graph_obj Pointer to the producer graph object (typically a structure with graph information)
 * \param [in] enqueue_ref the reference(s) to enqueue
 *
 * \return A status of the operation. If the enqueue is successful, the function will return VX_SUCCESS.
 * 
 * \ingroup group_vx_producer
 */
typedef vx_status (*vxProducerEnqueueCallback)(
    void*        graph_obj,
    vx_reference enqueue_ref);

/**
 * \brief Custom callback function to forward meta datas (for eg. supplementary) to the consumer(s).
 *
 * \param [in]  graph_obj Pointer to the producer graph object (typically a structure with graph information)
 * \param [in]  ref the reference which is being sent, usefull if you want to extract the supplementary data
 * \param [out] metadata The metadata array to be filled out by the application.
 * \param [out] size_t* size the size of the metadata array
 * \return if VX_SUCCESS is not returned, the producer will not consider the metadata as valid
 *         and nothing will be forwarded to the consumer(s)
 * 
 * \ingroup group_vx_producer
 */
typedef vx_status (*vxProducerTransmitMetadataCallback)(
    void*     graph_obj,
    vx_reference ref,
    void*    metadata,
    size_t*      size);

/**
 * \brief Custom callback function to notify the producer when a frame is dropped for a corresponding consumer.
 *
 * This function is called by the producer server thread when a frame is dropped for a corresponding consumer.
 * i.e. the number of allowed locked buffers by the consumer is exceeded.
 * example with two consumers running at different speed.
 * this threshold is shared by the consumer when connected to the producer and should correspond to its normal
 * behavior.
 *
 * \param [in] graph A pointer to the OpenVX graph object that the producer is using.
 * \param [in] consumer_id The unique identifier (PID) of the consumer who is retaining too much buffers.
 * \param [in] consumer_name The name of the consumer which is specified in the consumer parameters.
 * \param [in] dequeued_refs An array of references that were dequeued by the consumer and won't be send to the
 * corresponding consumer.
 * \param [in] num_dequeued_refs The number of dequeued references.
 *
 */
typedef void (*vxProducerFrameDropCallback)(
    vx_graph        graph,
    const vx_uint32 consumer_id,
    const char*     consumer_name,
    vx_reference    dequeued_refs[],
    vx_uint32*      num_dequeued_refs);

/**
 * \brief Callback function for notifying when a consumer connects to the producer.
 *
 * \param consumer_id The unique identifier (PID) of the consumer.
 * \param consumer_name The name of the consumer.
 *
 */
typedef void (*vx_producer_connect_notify_f)(const vx_uint32 consumer_id, const char* consumer_name);

/**
 * \brief Callback function for notifying when a consumer disconnects from the producer.
 *
 * \param consumer_id The unique identifier (PID) of the consumer.
 * \param consumer_name The name of the consumer.
 */
typedef void (*vx_producer_disconnect_notify_f)(const vx_uint32 consumer_id, const char* consumer_name);

/*!
 * \brief Producer callbacks
 *
 * \ingroup group_vx_producer
 */
typedef struct _vx_streaming_cb_t
{
    /*! \brief The Dequeue function */
    vxProducerDequeueCallback          dequeueCallback;
    /*! \brief The Enqueue function */
    vxProducerEnqueueCallback          enqueueCallback;
    /*! \brief The Function pointer to export metadata */
    vxProducerTransmitMetadataCallback getMetadataCallback;
} vx_streaming_cb_t;

/*!
 * \brief Parameters for producer
 *
 * \ingroup group_vx_producer
 */
typedef struct _vx_producer_params_t
{
    /*! \brief name of the producer server */
    vx_char             name[VX_MAX_PRODUCER_NAME];
    /*! \brief name of the access point b/w producer and consumer */
    vx_char             access_point_name[VX_MAX_ACCESS_POINT_NAME];

    /*! \brief number of buffers */
    vx_uint32           num_buffers;
    /*! \brief number of references to be exported to consumer */
    vx_uint16           num_buffer_refs_export;
    /*! \brief maximum number of references allowed to be locked by client before new frame is dropped instead of being sent */
    uint32_t max_refs_locked_by_client;
    
    /*! \brief references to be exported to producer */
    vx_reference*       ref_to_export;

    /*! \brief pointer to the producer graph object */
    void*               graph_obj;
    /*! \brief pointer to store producer function callbacks */
    vx_streaming_cb_t   streaming_cb;

#ifdef IPPC_SHEM_ENABLED
    /*! \brief Contains ippc port configuration */
    SIppcPortMap        ippc_port[IPPC_PORT_COUNT];
#endif    
} vx_producer_params_t; 

/**
 * \brief Creates a producer server for inter-process/inter SoC etc. communication.
 *
 * This function creates a producer server that can be used to share OpenVX objects between different consumers
 *
 * \param [in] context The reference to the implementation context.
 * \param [in] params The parameter of consumer to initialize vx_producer object
 *
 * \return this is returning an vx_producer object
 * 
 * \ingroup group_vx_producer
 */
VX_API_ENTRY vx_producer VX_API_CALL vxCreateProducer(vx_context context, const vx_producer_params_t* params);

/**
 * \brief Starts the producer server.
 *
 * This function starts the producer server and prepares it for accepting connections from consumers.
 * Until a consumer connects, the producer will not send any buffers and will enqueue them back to the producer
 * graph.
 *
 * \param producer A pointer to the producer server object.
 *
 * \return A status of the operation. If the operation is successful, the function will return VX_SUCCESS.
 * If the result is not VX_SUCCESS, the producer has not been started and the application should decide what to do
 * (retry, stop, etc.) returns VX_ERROR_NO_RESOURCES if the graph parameter and the reference were not given to the
 * producer
 * 
 * \ingroup group_vx_producer
 */
VX_API_ENTRY vx_status VX_API_CALL vxProducerStart(vx_producer producer);

/**
 * \brief Shuts down the producer server.
 *
 * This function initiates the shutdown process for the producer server. It sends the last buffer to the consumer(s)
 * and waits for all consumer(s) pipelines to be flushed before stopping the producer. This is a blocking interface,
 * it will wait until all the consumers have finished their processing (and returned the last buffer to the producer
 * server) or will time out after a certain time configured in the producer. after this point all the exchanged
 * buffer can be released
 *
 * \param producer A pointer to the producer object.
 *
 * \return A status of the operation. If the shutdown is successful, the function will return VX_SUCCESS.
 * If the result is not VX_SUCCESS, the producer did not received all the buffers back from the consumer(s) and the
 * timeout has been reached. the application can still decide to release the buffer and stop the producer
 * 
 * \ingroup group_vx_producer
 */
VX_API_ENTRY vx_status VX_API_CALL vxProducerShutdown(vx_producer producer);

/**
 * \brief Releases the producer server object.
 *
 * \param producer A pointer to the producer object.
 *
 * \return A status of the operation. If the operation is successful, the function will return VX_SUCCESS.
 * 
 * \ingroup group_vx_producer
 */
VX_API_ENTRY vx_status VX_API_CALL vxReleaseProducer(vx_producer* producer);
    
#ifdef __cplusplus
}
#endif

#endif // _VX_KHR_PRODUCER_H_
