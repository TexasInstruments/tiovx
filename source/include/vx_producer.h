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

    typedef enum
    {
        VX_PROD_STATE_INIT  = 0x0,
        VX_PROD_STATE_RUN   = 0x1,
        VX_PROD_STATE_WAIT  = 0x2,
        VX_PROD_STATE_FLUSH = 0x3,
    } vx_producer_state;

    typedef enum
    {
        PROD_STATE_CLI_NOT_CONNECTED  = 0x0,
        PROD_STATE_CLI_CONNECTED      = 0x1,
        PROD_STATE_CLI_RUNNING        = 0x2,
        PROD_STATE_CLI_FLUSHED        = 0x3,
        PROD_STATE_CLI_FAILED         = 0x4
    } producer_client_state_t;

    typedef enum
    {
        IN_GRAPH = 0x00,
        LOCKED   = 0x01,
        FREE     = 0x02
    } producer_buffer_status;

    typedef struct
    {
        producer_buffer_status buffer_status;
        vx_reference           ovx_ref;
        vx_int32               refcount;
        vx_uint8               attached_to_client[OVXGW_NUM_CLIENTS];
        vx_uint64              state_timestamp;
    } buffer_info_t;

    typedef struct 
    {
        producer_client_state_t state;
        uint64_t                consumer_id;
#ifdef IPPC_SHEM_ENABLED
        SIppcReceiverContext  m_receiver_ctx;
#elif SOCKET_ENABLED
        int32_t                 socket_fd;
#endif
        vx_int32                first_buffer_released;
        pthread_t               bck_thread;
    } producer_bckchannel_t;
    
    typedef struct
    {

        pthread_t              broadcast_thread;
        vx_uint32              sequence_num;
        vx_uint32              total_sequences;
    } prod_internal_data_t;

    typedef struct _vx_producer{
        tivx_reference_t       base;
        vx_producer_state      graph_state;
        vx_uint32              nb_consumers;
        producer_bckchannel_t  consumers_list[OVXGW_NUM_CLIENTS];

        pthread_t              broadcast_thread;
        vx_bool                ref_export_done;

        vx_uint32              nbEnqueueFrames;
        vx_uint32              nbDequeueFrames;
        vx_uint32              nbDroppedFrames;
        vx_uint32              enqueuecount;

        vx_uint32              numBufferRefs;
        vx_uint32              numBufferRefsExport;

        buffer_info_t          refs[OVXGW_MAX_NUM_REFS];
        vx_uint32              last_buffer;
        pthread_mutex_t        buffer_mutex;

        vx_char                name[VX_MAX_PRODUCER_NAME];
        vx_char                access_point_name[VX_MAX_ACCESS_POINT_NAME];
        void*                  graph_obj;
        vx_uint8               last_frame_dropped;

        vx_streaming_cb_t      streaming_cb;
#ifdef IPPC_SHEM_ENABLED
        SIppcShmemContext      m_shmem_ctx;
        SIppcSenderContext     m_sender_ctx;
        SIppcPortMap           ippc_port[IPPC_PORT_COUNT];
#elif SOCKET_ENABLED
        pthread_mutex_t client_mutex;
        server_context         server;
        // reference metadata buffer
        uint8_t metadata_buffer[SOCKET_MAX_MSG_SIZE];
#endif
    } tivx_producer_t;

#ifdef __cplusplus
}
#endif

#endif //VX_PRODUCER_H_
