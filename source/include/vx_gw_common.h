//=============================================================================
//  C O P Y R I G H T
//-----------------------------------------------------------------------------
/// @copyright (c) 2024 by Robert Bosch GmbH. All rights reserved.
//
//  The reproduction, distribution and utilization of this file as
//  well as the communication of its contents to others without express
//  authorization is prohibited. Offenders will be held liable for the
//  payment of damages. All rights reserved in the event of the grant
//  of a patent, utility model or design.
//=============================================================================
//  P R O J E C T   I N F O R M A T I O N
//-----------------------------------------------------------------------------
//     Projectname: IPPC
//  Target systems: cross platform
//       Compilers: ISO C compliant or higher
//=============================================================================
//  I N I T I A L   A U T H O R   I D E N T I T Y
//-----------------------------------------------------------------------------
//        Name:
//  Department: XC-AS/EPO3
//=============================================================================
/// @file  vx_gw_common.h
//=============================================================================

#ifndef __VX_GW_COMMON_H
#define __VX_GW_COMMON_H

#ifdef IPPC_SHEM_ENABLED
#include <utils/ippc/include/app_ippc.h>
#elif SOCKET_ENABLED
#include <utils/socket/include/app_socket.h>
#endif

#include <tivx_utils_ipc_ref_xfer.h>

#define OVXGW_MAX_NUM_REFS (10u)
#define OVXGW_MAX_NUM_ITEMS (10u)
#define OVXGW_MAX_META_SIZE (4096u)

typedef enum
{
    VXGW_STATUS_SUCCESS           = 0U,
    VXGW_STATUS_FAILURE           = 1U,
    VXGW_STATUS_CONSUMER_REF_DROP = 2U,
    VXGW_STATUS_CONSUMER_FLUSHED  = 3U,
} vx_gw_status_t;

//producer->consumers message (1->N)
typedef struct
{
    // used at runtime, release
    int32_t  buffer_id;
    uint32_t supplementary_data;
    uint32_t last_buffer;

    // used at init - meta is toggled, consumer num is checked against our case
    uint32_t backchannel_port; // use SIppcPortMap
    uint32_t consumer_id;
    uint32_t buffer_meta;

    // number of total object array items; set to zero if reference is not object array
    uint8_t num_items;

    // flag to indicate if this is the last reference to be exchanged with the consumer
    uint8_t num_refs;

    tivx_utils_ref_ipc_msg_t ipc_msg[OVXGW_MAX_NUM_REFS][OVXGW_MAX_NUM_ITEMS];

    uint8_t metadata_buffer[OVXGW_MAX_META_SIZE];

    // flag set when metadata can be read by consumer
    uint8_t metadata_valid;

    // flag to inform consumer whether previous frame has been dropped by producer
    uint8_t last_frame_dropped;

    // size of metadata, copied after this struct in the buffer ID message
    size_t metadata_size;
} SProducerContent;

//consumer->producer message (1->1)
typedef struct
{
    uint32_t buffer_id;
    uint32_t last_buffer;
    uint32_t consumer_id;
} SConsumerContent;

typedef enum
{
    VX_MSGTYPE_HELLO       = 1U,
    VX_MSGTYPE_REF_BUF     = 2U,
    VX_MSGTYPE_BUFID_CMD   = 3U,
    VX_MSGTYPE_BUF_RELEASE = 4U,
    VX_MSGTYPE_COUNT       = 5U
} vx_gw_message_type;

typedef struct
{
    vx_gw_message_type msg_type;

    // process id, used to distinguish consumers on app level
    uint64_t consumer_id;

} vx_gw_hello_msg;

typedef struct
{
    vx_gw_message_type msg_type;

    // number of total object array items; set to zero if reference is not object array
    uint8_t num_items;

    // number representing the element index for object array; set to zero if reference is not an object array item
    uint8_t item_index;

    // flag to indicate if this is the last reference to be exchanged with the consumer
    uint8_t last_reference;

    tivx_utils_ref_ipc_msg_t ipc_msg;

} vx_gw_buff_desc_msg;

typedef struct
{
    vx_gw_message_type msg_type;

    // number representing the buffer ID
    uint8_t buffer_id;

    // last buffer transmitted
    uint8_t last_buffer;

    // flag set when metadata can be read by consumer
    uint8_t metadata_valid;

    // flag to inform consumer whether previous frame has been dropped by producer
    uint8_t last_frame_dropped;

    // size of metadata, copied after this struct in the buffer ID message
    uint16_t metadata_size;

} vx_gw_buff_id_msg;

  /* may number of consumer for socket or IPP*/
  #define OVXGW_NUM_CLIENTS (4U)

#ifdef IPPC_SHEM_ENABLED
  #define IPPC_PORT_COUNT (OVXGW_NUM_CLIENTS+1U)
#endif

#endif
