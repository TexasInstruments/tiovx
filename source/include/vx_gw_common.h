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

#ifdef IPPC_SHEM_ENABLED
/*! \brief max number of port configuration for IPPC conatining 1 sender and N receiver*/
  #define IPPC_PORT_COUNT (VX_GW_NUM_CLIENTS + 1U)
#endif

/*! \brief The gateway connector status
 * \ingroup group_vx_gw_common
 */
typedef enum
{
    VX_GW_STATUS_SUCCESS           = 0U,
    VX_GW_STATUS_FAILURE           = 1U,
    VX_GW_STATUS_CONSUMER_REF_DROP = 2U,
    VX_GW_STATUS_CONSUMER_FLUSHED  = 3U,
} vx_gw_status_t;

/*! \brief The producer message content via IPPC
 * \ingroup group_vx_gw_common
 */
typedef struct
{
    /*! \brief Indicates id of the buffer to be exchanged with the consumer */
    int32_t  buffer_id;
    /*! \brief flag to indicate if this is the last reference to be exchanged with the consumer */
    uint32_t last_buffer;
    /*! \brief flag to inform consumer whether previous frame has been dropped by producer */
    uint8_t last_frame_dropped;

    /*! \brief flag set when metadata can be read by consumer */
    uint8_t metadata_valid;
    /*! \brief size of metadata */
    size_t metadata_size;
    /*! \brief Contains producer metadata */
    uint8_t metadata_buffer[VX_GW_MAX_META_SIZE];    

    /*! \brief Used for ippc communication */
    uint32_t backchannel_port;
    /*! \brief Contains the id of the consumer for which the data will be exchanged */
    uint32_t consumer_id;

    /*! \brief number of total object array items; set to zero if reference is not object array */
    uint8_t num_items;
    /*! \brief number of producer references */
    uint8_t num_refs;
    /*! \brief Array used to store intermediate IPC messages */
    tivx_utils_ref_ipc_msg_t ref_export_handle[VX_GW_MAX_NUM_REFS][VX_GW_MAX_NUM_ITEMS];

} vx_prod_msg_content_t;

/*! \brief The consumer message content via IPPC
 * \ingroup group_vx_gw_common
 */
typedef struct
{
    /*! \brief Indicate the id of the buffer to be exchanged with the producer */
    uint32_t buffer_id;
    /*! \brief flag to indicate that the last reference has been processed */
    uint32_t last_buffer;
    /*! \brief Contains the id of the consumer for which the data has been exchanged */
    uint32_t consumer_id;
} vx_cons_msg_content_t;

/*! \brief The message type exchanged b/w producer and consumer 
 * \ingroup group_vx_gw_common
 */
typedef enum
{
    VX_MSGTYPE_HELLO       = 1U,
    VX_MSGTYPE_REF_BUF     = 2U,
    VX_MSGTYPE_BUFID_CMD   = 3U,
    VX_MSGTYPE_BUF_RELEASE = 4U,
    VX_MSGTYPE_COUNT       = 5U
} vx_gw_message_type;

/*! \brief The Structure to specify message type b/w producer and consumer
 * \ingroup group_vx_gw_common
 */
typedef struct
{
    /*! \brief Indicates the type of message */
    vx_gw_message_type msg_type;

    /*! \brief consumer id, used to distinguish consumers on app level */
    uint64_t consumer_id;

} vx_gw_hello_msg;

/*! \brief The initial setup message b/w producer and consumer via SOCKET communication
 * \ingroup group_vx_gw_common
 */
typedef struct
{
    /*! \brief Indicates the type of message */
    vx_gw_message_type msg_type;

    /*! \brief flag to indicate if this is the last reference to be exchanged with the consumer */
    uint8_t last_reference;

    /*! \brief number representing the element index for object array; set to zero if reference is not an object array item */
    uint8_t item_index;
    /*! \brief number of total object array items; set to zero if reference is not object array */
    uint8_t num_items;
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

    /*! \brief number representing the buffer ID */
    uint8_t buffer_id;
    /*! \brief last buffer transmitted */
    uint8_t last_buffer;
    /*! \brief flag to inform consumer whether previous frame has been dropped by producer */
    uint8_t last_frame_dropped;

    /*! \brief flag set when metadata can be read by consumer */
    uint8_t metadata_valid;
    /*! \brief size of metadata, copied after this struct in the buffer ID message */
    uint16_t metadata_size;

} vx_gw_buff_id_msg;

#endif
