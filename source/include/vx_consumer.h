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

    typedef enum
    {
        VX_CONS_STATE_DISCONNECTED = 0x0,
        VX_CONS_STATE_INIT         = 0x1,
        VX_CONS_STATE_RUN          = 0x2,
        VX_CONS_STATE_WAIT         = 0x3,
        VX_CONS_STATE_FLUSH        = 0x4,
        VX_CONS_STATE_FAILED       = 0x5
    } vx_consumer_state;

    typedef struct _vx_consumer{
        tivx_reference_t        base;
        vx_context              context;
        vx_uint32               num_failures;
        vx_bool                 init_done;

        vx_char                 name[VX_MAX_CONSUMER_NAME];
        vx_char                 access_point_name[VX_MAX_ACCESS_POINT_NAME];
        void*                   graph_obj;
        vx_uint32               num_refs;
        vx_reference            refs[OVXGW_MAX_NUM_REFS];
        vx_uint16               consumer_id;

        pthread_t               receiver_thread;
        vx_consumer_state       state;
        pthread_t               backchannel_thread;
        vx_subscriber_cb_t      subscriber_cb;
#ifdef IPPC_SHEM_ENABLED
        SIppcRegistry           m_registry;
        SIppcReceiverContext  m_receiver_ctx;    
        SIppcSenderContext      m_sender_ctx;
        SIppcPortMap            ippc_port[IPPC_PORT_COUNT];
#elif SOCKET_ENABLED
        int32_t                 socket_fd;
        pthread_t               bufferid_thread;
#endif
        vx_uint32               last_buffer;
        vx_uint8                last_buffer_id;
        vx_uint8                last_buffer_transmitted;
        vx_uint8                last_buffer_dropped;

        // Array used to store intermediate IPC messages
        tivx_utils_ref_ipc_msg_t ipcMessageArray[OVXGW_MAX_NUM_REFS];
        vx_uint32                ipcMessageCount;
    }tivx_consumer_t;

#ifdef __cplusplus
}
#endif

#endif // VX_CONSUMER_H_
