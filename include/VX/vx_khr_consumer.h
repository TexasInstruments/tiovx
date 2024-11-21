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

#ifndef _VX_KHR_CONSUMER_H_
#define _VX_KHR_CONSUMER_H_

/*!
 * \file
 * \brief The OpenVX consumer extension API.
 */

#define OPENVX_KHR_CONSUMER  "vx_khr_consumer"

#include <VX/vx.h>
#include <vx_gw_common.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*! \brief The Consumer Object. Consumer Object is a strongly-typed container for other data structures.
 * \ingroup group_vx_consumer
 */
typedef struct _vx_consumer *vx_consumer;

/*! \brief The object type enumeration for consumer object
 * \ingroup group_vx_consumer
 */
#define VX_TYPE_CONSUMER 0x820 /*!< \brief A <tt>\ref vx_consumer</tt>. */

/*! \brief Defines the length of the consumer name string, including the trailing zero.
 * \ingroup group_vx_consumer
 */
#define VX_MAX_CONSUMER_NAME 20

/*! \brief Defines the length of the access point name string, including the trailing zero.
 * \ingroup group_vx_consumer
 */
#define VX_MAX_ACCESS_POINT_NAME 20

/**
 * \brief Custom callback function for creating consumer graph.
 *
 * This function is called by the consumer client thread to intialze the exported reference from producer.
 *
 * \param [in]  graph_obj Pointer to the consumer graph object (typically a structure with graph information)
 * \param [out] refs An array of references.
 * \param [out] num_refs The number of references.
 *
 * \return A status of the operation. If the dequeue is successful, the function will return VX_SUCCESS.
 *         if the returned status is not VX_SUCCESS, the consumer will not send any buffer to the producer
 * 
 * \ingroup group_vx_consumer
 */
typedef vx_status (*vxConsumerCreateCallback)(
    void*        graph_obj,
    vx_reference refs[],
    vx_uint32   num_refs);

/**
 * \brief Custom callback function for dequeuing references from the consumer graph.
 *
 * This function is called by the consumer client thread waiting for new references to send to the producer.
 *
 * \param [in]  graph_obj Pointer to the consumer graph object (typically a structure with graph information)
 * \param [out] dequeued_refs An array of dequeued references.
 * \param [out] num_dequeued_refs The number of dequeued references.
 *
 * \return A status of the operation. If the dequeue is successful, the function will return VX_SUCCESS.
 *         if the returned status is not VX_SUCCESS, the consumer will not send any buffer to the producer
 * 
 * \ingroup group_vx_consumer
 */
typedef vx_status (*vxConsumerDequeueCallback)(
    void*        graph_obj,
    vx_reference dequeued_refs[],
    vx_uint32*   num_dequeued_refs);

/**
 * \brief Custom callback function to enqueue references into the consumer graph.
 *
 * This function is called by the consumer client thread when one (or more) buffer(s) has(have) been released by
 * the producer and get it returned back to the consumer graph.
 *
 * \param [in] graph_obj Pointer to the consumer graph object (typically a structure with graph information)
 * \param [in] enqueue_ref the reference(s) to enqueue
 *
 * \return A status of the operation. If the enqueue is successful, the function will return VX_SUCCESS.
 * 
 * \ingroup group_vx_consumer
 */
typedef vx_status (*vxConsumerEnqueueCallback)(
    void*        graph_obj,
    vx_reference enqueue_ref);

/**
 * \brief Custom callback function to store meta datas (for eg. supplementary) from the producer.
 *
 * \param [in]  graph_obj Pointer to the consumer graph object (typically a structure with graph information)
 * \param [in]  ref the reference which is being sent, usefull if you want to extract the supplementary data
 * \param [out] metadata The metadata array to be filled out by the application.
 * \param [out] size_t size the size of the metadata array
 * \return if VX_SUCCESS is not returned, the consumer will not consider the metadata as valid
 * 
 * \ingroup group_vx_consumer
 */
typedef vx_status (*vxConsumerStoreMetadataCallback)(
    void*        graph_obj,
    vx_reference ref,
    void*        metadata,
    size_t       size);

/**
 * \brief Consumer graph recovery callback.
 *
 * This callback is triggered when either of the consumer client sockets closes.
 *
 * \param [in] graph_obj Pointer to the consumer graph object (typically a structure with graph information)
 * 
 * \ingroup group_vx_consumer
 */
typedef void (*vxConsumerRecoveryCallback)(void* graph_obj);

/*!
 * \brief Consumer callbacks
 *
 * \ingroup group_vx_consumer
 */
typedef struct _vx_subscriber_cb_t
{
    /*! \brief The Create function */
    vxConsumerCreateCallback           createCallback;
    /*! \brief The Dequeue function */
    vxConsumerDequeueCallback          dequeueCallback;
    /*! \brief The Enqueue function */
    vxConsumerEnqueueCallback          enqueueCallback;
    /*! \brief The Function pointer to import metadata */
    vxConsumerStoreMetadataCallback    storeMetadataCallback;
    /*! \brief The recovery function */
    vxConsumerRecoveryCallback         recoveryCallback;
} vx_subscriber_cb_t;

/*!
 * \brief Parameters for consumer
 *
 * \ingroup group_vx_consumer
 */
typedef struct _vx_consumer_params_t
{
    /*! \brief name of the consumer client */
    vx_char             name[VX_MAX_CONSUMER_NAME];
    /*! \brief name of the access point b/w producer and consumer */
    vx_char             access_point_name[VX_MAX_ACCESS_POINT_NAME];

    /*! \brief references importeds from producer */
    vx_reference*       ref_to_import;

    /*! \brief pointer to the consumer graph object */
    void*               graph_obj;
    /*! \brief pointer to store consumer function callbacks */
    vx_subscriber_cb_t  subscriber_cb;

#ifdef IPPC_SHEM_ENABLED
    /*! \brief Contains ippc port configuration */
    SIppcPortMap        ippc_port[IPPC_PORT_COUNT];
#endif
} vx_consumer_params_t;

/**
 * \brief Creates a consumer server for inter-process/inter SoC etc. communication.
 *
 * This function creates a consumer client that can be used to share OpenVX objects with producer.
 *
 * \param [in] context The reference to the implementation context.
 * \param [in] params The parameter of consumer to initialize vx_consumer object
 *
 * \return this is returning an vx_consumer object
 * 
 * \ingroup group_vx_consumer
 */
VX_API_ENTRY vx_consumer VX_API_CALL vxCreateConsumer(vx_context context, const vx_consumer_params_t* params);

/**
 * \brief Starts the consumer client.
 *
 * This function starts the consumer client and prepares it for sending connections to producer.
 *
 * \param consumer A pointer to the consumer client object.
 *
 * \return A status of the operation. If the operation is successful, the function will return VX_SUCCESS.
 * 
 * \ingroup group_vx_consumer
 */
VX_API_ENTRY vx_status VX_API_CALL vxConsumerStart(vx_consumer consumer);

/**
 * \brief Shuts down the consumer client.
 *
 * \param consumer A pointer to the consumer object.
 *
 * \return A status of the operation. If the shutdown is successful, the function will return 1.
 */
VX_API_ENTRY vx_uint32 VX_API_CALL vxConsumerShutdownStatus(vx_consumer consumer);

/**
 * \brief Releases the consumer client object.
 *
 * \param consumer A pointer to the consumer object.
 *
 * \return A status of the operation. If the operation is successful, the function will return VX_SUCCESS.
 */
VX_API_ENTRY vx_status VX_API_CALL vxReleaseConsumer(vx_consumer* consumer);
    
#ifdef __cplusplus
}
#endif

#endif // _VX_KHR_CONSUMER_H_
