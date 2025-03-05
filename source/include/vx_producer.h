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

#ifndef VX_PRODUCER_H_
#define VX_PRODUCER_H_

#include <pthread.h>
#include "vx_gw_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief The Producer state enumeration 
 * \ingroup group_vx_producer
 */
typedef enum
{
    VX_PROD_STATE_INIT  = 0x0,
    VX_PROD_STATE_RUN   = 0x1,
    VX_PROD_STATE_WAIT  = 0x2,
    VX_PROD_STATE_FLUSH = 0x3,
} vx_producer_state;

/*! \brief The Producer-Consumer connection state enumeration 
 * \ingroup group_vx_producer
 */
typedef enum
{
    PROD_STATE_CLI_NOT_CONNECTED  = 0x0,
    PROD_STATE_CLI_CONNECTED      = 0x1,
    PROD_STATE_CLI_GRAPH_VERIFIED = 0x2,
    PROD_STATE_CLI_RUNNING        = 0x3,
    PROD_STATE_CLI_FLUSHED        = 0x4,
    PROD_STATE_CLI_FAILED         = 0x5
} producer_client_state_t;

/*! \brief The Producer buffer status enumeration 
 * \ingroup group_vx_producer
 */
typedef enum
{
    IN_GRAPH = 0x00,
    LOCKED   = 0x01,
    FREE     = 0x02
} producer_buffer_status;

/*! \brief The Producer buffer information
 * \ingroup group_vx_producer
 */
typedef struct
{
    /*! \brief Producer reference */
    vx_reference           ovx_ref;
    /*! \brief Number of consumers locking the reference */
    vx_int32               refcount;

    /*! \brief Status of reference in producer */
    producer_buffer_status buffer_status;
    /*! \brief flag to indicate whether the producer is connected to the consumer */
    vx_uint8               attached_to_client[VX_GW_NUM_CLIENTS];

    /*! \brief Indicates the time at which buffer status was set */
    vx_uint64              state_timestamp;

    /*! \brief incremented for every cycle a ref is already locked by a client to keep track of locked duration*/
    vx_uint8               locked_count;
} buffer_info_t;

/*! \brief Backchannel information from consumer
 * \ingroup group_vx_producer
 */
typedef struct 
{
    /*! \brief Indicates the producer-consumer connection status */
    producer_client_state_t state;

    /*! \brief consumer id, used to distinguish consumers on app level */
    vx_uint8                consumer_id;

    /*! \brief Thread to receive backchannel information from consumer */
    pthread_t               bck_thread;

#ifdef IPPC_SHEM_ENABLED
    /*! \brief Contains receiver context */
    SIppcReceiverContext    m_receiver_ctx;
#elif SOCKET_ENABLED
    /*! \brief Socket file descriptor */
    int32_t                 socket_fd;

    /*! \brief Indicates that the first buffer is released */
    int32_t                 first_buffer_released;
#endif
} producer_bckchannel_t;

/*! \brief Producer object internal state
 * \ingroup group_vx_producer
 */
typedef struct _vx_producer
{
    /*! \brief reference object */
    tivx_reference_t       base;
    /*! \brief Indicates the producer state */
    vx_producer_state      graph_state

    /*! \brief Contains number of consumers connected */;
    vx_uint32              nb_consumers;
    /*! \brief Stores consumers backchannel information */;
    producer_bckchannel_t  consumers_list[VX_GW_NUM_CLIENTS];
    /*! \brief Thread to send broadcast information to all consumers */
    pthread_t              broadcast_thread;

    /*! \brief Contains the number of frames that has been enqueued */
    vx_uint32              nbEnqueueFrames;
    /*! \brief Contains the number of frames that has been dequeued */
    vx_uint32              nbDequeueFrames;
    /*! \brief Contains the number of frames that has been dropped */
    vx_uint32              nbDroppedFrames;

    /*! \brief Stores the buffer reference status of the producer */
    buffer_info_t          refs[VX_GW_MAX_NUM_REFS];
    /*! \brief Mutex to prevent conflict during setting of buffer status of multiple consumers */
    pthread_mutex_t        buffer_mutex;
    /*! \brief number of producer buffers */
    vx_uint32              numBuffers;
    /*! \brief number of references to be exported to consumer */
    vx_uint32              numBufferRefsExport;
    /*! \brief maximum number of references allowed to be locked by client before new frame is dropped instead of being sent */
    vx_uint32              maxRefsLockedByClient;
    /*! \brief Flag to indicates that the reference has been exported */
    vx_bool                ref_export_done;

    /*! \brief flag to indicate if this is the last reference to be exchanged with the consumer */
    vx_uint32              last_buffer;
    /*! \brief flag to inform consumer whether previous frame has been dropped by producer */
    vx_uint8               last_frame_dropped;

    /*! \brief name of the producer server */
    vx_char                name[VX_MAX_PRODUCER_NAME];
    /*! \brief name of the access point b/w producer and consumer */
    vx_char                access_point_name[VX_MAX_ACCESS_POINT_NAME];

    /*! \brief pointer to the producer graph object */
    void*                  graph_obj;
    /*! \brief pointer to store producer function callbacks */
    vx_streaming_cb_t      streaming_cb;
    /*! \brief Mutex to prevent conflict during setting of multiple client status */
    pthread_mutex_t        client_mutex;    
#ifdef IPPC_SHEM_ENABLED
    /*! \brief Poll for new clients during startup */
    pthread_t              connection_check_thread;
    /*! \brief rate at which producer polls for new consumer during startup */
    vx_uint32              connection_check_polling_time;
    /*! \brief exit condition for polling connection check thread */
    vx_bool                connection_check_polling_exit;
    /*! \brief Contains shmem context */
    SIppcShmemContext      m_shmem_ctx;
    /*! \brief Contains sender context */
    SIppcSenderContext     m_sender_ctx;
    /*! \brief Contains ippc port configuration */
    SIppcPortMap           ippc_port[IPPC_PORT_COUNT];
#elif SOCKET_ENABLED
    /*! \brief Contains server context */
    server_context         server;
    /*! \brief Contains producer metadata */
    uint8_t metadata_buffer[SOCKET_MAX_MSG_SIZE];
#endif
} tivx_producer_t;

#ifdef __cplusplus
}
#endif

#endif //VX_PRODUCER_H_
