/*
 * Copyright (c) 2024 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http:! \briefwww.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __VX_GW_COMMON_H
#define __VX_GW_COMMON_H

#ifdef IPPC_SHEM_ENABLED
#include <utils/ippc/include/app_ippc.h>
#elif SOCKET_ENABLED
#include <utils/socket/include/app_socket.h>
#endif

#include <tivx_utils_ipc_ref_xfer.h>

/*! \brief max number of references*/
#define VX_GW_MAX_NUM_REFS (10u)
/*! \brief max number of items in the object array*/
#define VX_GW_MAX_NUM_ITEMS (10u)
/*! \brief max size of meta data (supplementary data)*/
#define VX_GW_MAX_META_SIZE (4096u)

/*! \brief max number of consumer for socket or IPPC*/
#define VX_GW_NUM_CLIENTS (4U)

/*! \brief max number of cycles a client is allowed to 
lock a buffer until it is returned by the gateway*/
#define VX_GW_MAX_LOCKED_CNT (10U)

#ifdef IPPC_SHEM_ENABLED
/*! \brief max number of port configuration for IPPC conatining 1 sender and N receiver*/
  #define IPPC_PORT_COUNT (VX_GW_NUM_CLIENTS + 1U)
#endif

/*! \brief The gateway connector status
 * \ingroup group_vx_gw_common
 */
typedef enum
{
    VX_GW_STATUS_SUCCESS                = 0U,
    VX_GW_STATUS_FAILURE                = 1U,
    VX_GW_STATUS_CONSUMER_REF_DROP      = 2U,
    VX_GW_STATUS_CONSUMER_GRAPH_READY   = 3U, 
    VX_GW_STATUS_CONSUMER_FLUSHED       = 4U,
} vx_gw_status_t;

/*! \brief The producer message content via IPPC
 * \ingroup group_vx_gw_common
 */
typedef struct
{
    /*! \brief Indicates id of the buffer to be exchanged with the consumer */
    vx_int32  buffer_id;
    /*! \brief Indicates to receivers whether current frame shall be consumed or not */
    vx_uint32  mask;
    /*! \brief flag to indicate if this is the last reference to be exchanged with the consumer */
    vx_uint32 last_buffer;
    /*! \brief flag to inform consumer whether previous frame has been dropped by producer */
    vx_uint8 last_frame_dropped;

    /*! \brief flag set when metadata can be read by consumer */
    vx_uint8 metadata_valid;
    /*! \brief size of metadata */
    size_t metadata_size;
    /*! \brief Contains producer metadata */
    vx_uint8 metadata_buffer[VX_GW_MAX_META_SIZE];

    /*! \brief number of total object array items; set to zero if reference is not object array */
    vx_uint8 num_items;
    /*! \brief number of producer references */
    vx_uint8 num_refs;
    /*! \brief Array used to store intermediate IPC messages */
    tivx_utils_ref_ipc_msg_t ref_export_handle[VX_GW_MAX_NUM_REFS][VX_GW_MAX_NUM_ITEMS];

} vx_prod_msg_content_t;

/*! \brief The message type exchanged b/w producer and consumer 
 * \ingroup group_vx_gw_common
 */
typedef enum
{
    VX_MSGTYPE_HELLO                = 1U,
    VX_MSGTYPE_REF_BUF              = 2U,
    VX_MSGTYPE_BUFID_CMD            = 3U,
    VX_MSGTYPE_BUF_RELEASE          = 4U,
    VX_MSGTYPE_CONSUMER_CREATE_DONE = 5U,
    VX_MSGTYPE_COUNT                = 6U
} vx_gw_message_type;

/*! \brief The consumer message content via IPPC
 * \ingroup group_vx_gw_common
 */
typedef struct
{
    /*! \brief Indicates the type of message */
    vx_gw_message_type msg_type;
    /*! \brief Indicate the id of the buffer to be exchanged with the producer */
    vx_uint32 buffer_id;
    /*! \brief flag to indicate that the last reference has been processed */
    vx_uint32 last_buffer;
    /*! \brief Contains the id of the consumer for which the data has been exchanged */
    vx_uint8  consumer_id;
} vx_cons_msg_content_t;

/*! \brief The Structure to specify message type b/w producer and consumer
 * \ingroup group_vx_gw_common
 */
typedef struct
{
    /*! \brief Indicates the type of message */
    vx_gw_message_type msg_type;

    /*! \brief consumer id, used to distinguish consumers on app level */
    vx_uint8 consumer_id;

} vx_gw_hello_msg;

/*! \brief The initial setup message b/w producer and consumer via SOCKET communication
 * \ingroup group_vx_gw_common
 */
typedef struct
{
    /*! \brief Indicates the type of message */
    vx_gw_message_type msg_type;

    /*! \brief flag to indicate if this is the last reference to be exchanged with the consumer */
    vx_uint8 last_reference;

    /*! \brief number representing the element index for object array; set to zero if reference is not an object array item */
    vx_uint8 item_index;
    /*! \brief number of total object array items; set to zero if reference is not object array */
    vx_uint8 num_items;
    /*! \brief consumer id, used to distinguish consumers on app level */
    vx_uint8 consumer_id;
    /*! \brief IPC message containing references to be exported to consumer */
    tivx_utils_ref_ipc_msg_t ref_export_handle;

} vx_gw_buff_desc_msg;

/*! \brief The runtime message b/w producer and consumer via SOCKET communication
 * \ingroup group_vx_gw_common
 */
typedef struct
{
    /*! \brief Indicates the type of message */
    vx_gw_message_type msg_type;

    /*! \brief consumer id, used to distinguish consumers on app level */
    vx_uint8 consumer_id;
    /*! \brief number representing the buffer ID */
    vx_uint8 buffer_id;
    /*! \brief last buffer transmitted */
    vx_uint8 last_buffer;
    /*! \brief flag to inform consumer whether previous frame has been dropped by producer */
    vx_uint8 last_frame_dropped;

    /*! \brief flag set when metadata can be read by consumer */
    vx_uint8 metadata_valid;
    /*! \brief size of metadata, copied after this struct in the buffer ID message */
    vx_uint16 metadata_size;

} vx_gw_buff_id_msg;

#endif
