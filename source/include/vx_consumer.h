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

#ifndef VX_CONSUMER_H_
#define VX_CONSUMER_H_

#include <pthread.h>
#include "vx_gw_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief The Consumer state enumeration 
 * \ingroup group_vx_consumer
 */
typedef enum
{
    VX_CONS_STATE_DISCONNECTED = 0x0,
    VX_CONS_STATE_INIT         = 0x1,
    VX_CONS_STATE_RUN          = 0x2,
    VX_CONS_STATE_WAIT         = 0x3,
    VX_CONS_STATE_FLUSH        = 0x4,
    VX_CONS_STATE_FAILED       = 0x5
} vx_consumer_state;

/*! \brief Consumer object internal state
 * \ingroup group_vx_consumer
 */
typedef struct _vx_consumer
{
    /*! \brief reference object */
    tivx_reference_t        base;
    /*! \brief reference to implementation context */
    vx_context              context;

    /*! \brief Contains number of failures in producer-consumer communication */
    vx_uint32               num_failures;
    /*! \brief Flag to indicate that the consumer client initialization is done */
    vx_bool                 init_done;
    /*! \brief Flag to indicate that the consumer reference import is done */
    vx_bool                 ref_import_done;

    /*! \brief name of the consumer client */
    vx_char                 name[VX_MAX_CONSUMER_NAME];
    /*! \brief name of the access point b/w producer and consumer */
    vx_char                 access_point_name[VX_MAX_ACCESS_POINT_NAME];
    /*! \brief pointer to the consumer graph object */
    void*                   graph_obj;
    /*! \brief number of references imported from producer */
    vx_uint32               num_refs;
    /*! \brief Consumer references */
    vx_reference            refs[VX_GW_MAX_NUM_REFS];
    /*! \brief Mutex to prevent conflict during sending back of buffer */
    pthread_mutex_t         buffer_mutex;

    /*! \brief Contains the id of the consumer for which the data will be exchanged */
    vx_uint8                consumer_id;
    /*! \brief Thread to receive broadcasted information from producer */
    pthread_t               receiver_thread;
    /*! \brief Indicates the consumer state */
    vx_consumer_state       state;
    /*! \brief Thread to send backchannel information to producer */
    pthread_t               backchannel_thread;
    /*! \brief pointer to store consumer function callbacks */
    vx_subscriber_cb_t      subscriber_cb;

    /*! \brief flag to indicate that the last reference has been processed */
    vx_uint32               last_buffer;
    /*! \brief Contains the last reference buffer id */
    vx_uint8                last_buffer_id;
    /*! \brief flag to indicate that the last buffer has been transmitted */
    vx_uint8                last_buffer_transmitted;
    /*! \brief flag to inform consumer whether previous frame has been dropped by producer */
    vx_uint8                last_buffer_dropped;

    /*! \brief Array used to store intermediate IPC messages */
    tivx_utils_ref_ipc_msg_t ipcMessageArray[VX_GW_MAX_NUM_REFS];
    /*! \brief Indicates the number of IPC message sent */
    vx_uint32                ipcMessageCount;
    /*! \brief waiting time for producer */
    vx_uint32               connect_polling_time;        
#ifdef IPPC_SHEM_ENABLED
    /*! \brief Contains registry information */
    SIppcRegistry           m_registry;
    /*! \brief Contains receiver context */
    SIppcReceiverContext    m_receiver_ctx;
    /*! \brief Contains sender context */
    SIppcSenderContext      m_sender_ctx;
    /*! \brief Contains ippc port configuration */
    SIppcPortMap            ippc_port[IPPC_PORT_COUNT];
#elif SOCKET_ENABLED
    /*! \brief Socket file descriptor */
    int32_t                 socket_fd;
#endif
}tivx_consumer_t;

#ifdef __cplusplus
}
#endif

#endif // VX_CONSUMER_H_
