/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
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




#ifndef TIVX_OBJ_DESC_PRIV_H_
#define TIVX_OBJ_DESC_PRIV_H_

#include <TI/tivx_obj_desc.h>
#include <TI/tivx_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Internal private implementation of object descriptor
 */

/*! \brief Max nodes that can be blocked on object descriptor queue not empty
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_OBJ_DESC_QUEUE_MAX_BLOCKED_NODES       (8u)

/*! \brief Max elements in a object descriptor queue
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_OBJ_DESC_QUEUE_MAX_DEPTH               (32u)

/*! \brief Max object descriptors than be parameters in a command object
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_CMD_MAX_OBJ_DESCS        (16u)

/*! \brief Flag to indicate if command receiver needs to ACK this command
 * \ingroup group_tivx_obj_desc_priv
 */
#define TIVX_CMD_FLAG_SEND_ACK           (0x00000001u)

/*! \brief Flag to indicate if this is a command or ACK for a command
 * \ingroup group_tivx_obj_desc_priv
 */
#define TIVX_CMD_FLAG_IS_ACK             (0x00000002u)


/*! \brief Data Ref Q flag to determine if ref consumed event should be sent
 * \ingroup group_tivx_obj_desc_priv
 */
#define TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_ENABLE_REF_CONSUMED_EVENT      (0x00000001u)

/*! \brief Data Ref Q flag to determine if ref is acquired for this data ref q
 * \ingroup group_tivx_obj_desc_priv
 */
#define TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_REF_ACQUIRED                   (0x00000002u)

/*! \brief Data Ref Q flag to determine if this obj desc is part of some delay
 * \ingroup group_tivx_obj_desc_priv
 */
#define TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_IN_DELAY                       (0x00000004u)

/*! \brief Data Ref Q flag to determine if delay slot to which
 *          this obj desc belongs should be auto aged
 *
 * Valid only when TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_IN_DELAY is set
 *
 * \ingroup group_tivx_obj_desc_priv
 */
#define TIVX_OBJ_DESC_DATA_REF_Q_FLAG_DELAY_SLOT_AUTO_AGE               (0x00000008u)

/*! \brief Shift for storing Object Descriptor id in 32bit variable
 * \ingroup group_tivx_obj_desc_priv
 */
#define TIVX_OBJ_DESC_ID_SHIFT           (8u)

/*! \brief Mask for storing Object Descriptor id in 32bit variable
 * \ingroup group_tivx_obj_desc_priv
 */
#define TIVX_OBJ_DESC_ID_MASK            (0x0000FFFFu)




/*!
 * \brief Command object descriptor
 *
 * \ingroup group_tivx_obj_desc_priv
 */
typedef struct _tivx_obj_desc_cmd
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;

    /*! \brief Handle of ACK event that is posted when ACK is received
     *     MUST be valid if flags TIVX_CMD_FLAG_SEND_ACK is set
     */
    uint64_t ack_event_handle;

    /*! \brief command to execute */
    uint32_t cmd_id;

    /*! \brief flags associated with this command, see
     *         TIVX_CMD_FLAG_xxx
     */
    uint32_t flags;

    /*! target for which this command is directed */
    uint32_t dst_target_id;

    /*! source target ID to which ACK should be sent
     *  if ACK flag is not set then this field is not used and can be set to
     *  TIVX_TARGET_ID_INVALID
     */
    uint32_t src_target_id;

    /*! \brief Number of object descriptor parameters with this command */
    uint32_t num_obj_desc;

    /*! \brief object descriptor ID's of parameters */
    uint16_t obj_desc_id[TIVX_CMD_MAX_OBJ_DESCS];

    /*! \brief command execution status */
    uint32_t cmd_status;

    /*! \brief time at which this command is sent */
    uint32_t timestamp_h;

    /*! \brief time at which this command is sent */
    uint32_t timestamp_l;

} tivx_obj_desc_cmd_t;

/*!
 * \brief Command object descriptor
 *
 * \ingroup group_tivx_obj_desc_priv
 */
typedef struct _tivx_obj_desc_kernel_name
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;

    /*! \brief kernel name */
    char kernel_name[VX_MAX_KERNEL_NAME];

} tivx_obj_desc_kernel_name_t;

/*!
 * \brief Data structure to get information about node blocked on this object descriptor to be
 *        ready
 * \ingroup group_tivx_obj_desc_priv
 */
typedef struct _tivx_obj_desc_queue_blocked_nodes {

    /*! numbers of blocked nodes */
    uint16_t num_nodes;

    /*! \brief reserved field, to align to 64b */
    uint16_t rsv[7];

    /*! blocked node object descriptor IDs */
    uint16_t node_id[TIVX_OBJ_DESC_QUEUE_MAX_BLOCKED_NODES];

} tivx_obj_desc_queue_blocked_nodes_t;

/*!
 * \brief Object descriptor queue
 *
 * \ingroup group_tivx_obj_desc_priv
 */
typedef struct _tivx_obj_desc_queue
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;

    /*! \brief Current read index */
    uint16_t cur_rd;

    /*! \brief Current write index  */
    uint16_t cur_wr;

    /*! \brief Count of elements in queue  */
    uint16_t count;

    /*! \brief reserved field, to align to 64b */
    uint16_t rsv[5];

    /*! \brief queue memory */
    uint16_t queue_mem[TIVX_OBJ_DESC_QUEUE_MAX_DEPTH];

    /*! \brief node blocked on this object descriptor queue */
    tivx_obj_desc_queue_blocked_nodes_t blocked_nodes;

} tivx_obj_desc_queue_t;

/*!
 * \brief Data reference queue
 *
 * \ingroup group_tivx_obj_desc_priv
 */
typedef struct _tivx_obj_desc_data_ref_q
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;

    /*! \brief object specific flags */
    uint32_t flags;

    /*! \brief command to send when ref is consumed */
    uint16_t ref_consumed_cmd_obj_desc_id;

    /*! \brief queue to use for data ref acquire */
    uint16_t acquire_q_obj_desc_id;

    /*! \brief queue to use for data ref release */
    uint16_t release_q_obj_desc_id;

    /*! \brief acquired ref obj desc ID */
    uint16_t ref_obj_desc_id;

    /*! \brief number of input nodes that have consumed 'ref_obj_desc_id' */
    uint16_t in_node_done_cnt;

    /*! \brief number of nodes take this data ref as input */
    uint16_t num_in_nodes;

    /*! \brief Next obj desc in a delay.
     *
     * Valid only when
     * TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_IN_DELAY is set
     */
    uint16_t next_obj_desc_id_in_delay;

    /*! \brief slot in delay for this obj desc.
     *
     * Valid only when
     * TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_IN_DELAY is set
     */
    uint16_t delay_slot_index;

    /*! \brief Number of slots in delay.
     *
     * Valid only when
     * TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_IN_DELAY is set
     */
    uint16_t delay_slots;

    /*! \brief reserved fields for alignment */
    uint16_t rsv[1];

} tivx_obj_desc_data_ref_q_t;

/*!
 * \brief Graph object descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc_priv
 */
typedef struct _tivx_obj_desc_graph
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;

    /*! \brief pipeline ID of this graph instance */
    uint32_t pipeline_id;

    /*! \brief Graph instance state, running or completed or abandoned */
    uint32_t state;

    /*! \brief number of leaf nodes that have completed so far,
     *  before a graph begins this value is 0
     */
    uint32_t complete_leaf_nodes;

    /*! \brief graph execution time */
    uint32_t exe_time_beg_h;
    /*! \brief graph execution time */
    uint32_t exe_time_beg_l;
    /*! \brief graph execution time */
    uint32_t exe_time_end_h;
    /*! \brief graph execution time */
    uint32_t exe_time_end_l;

} tivx_obj_desc_graph_t;



/*!
 * \brief Object Descriptor Shared memory entry which can hold any of the
 *         supported object descriptor types
 *
 * \ingroup group_tivx_obj_desc_priv
 */
typedef union {

    tivx_obj_desc_cmd_t cmd;
    tivx_obj_desc_node_t node;
    tivx_obj_desc_image_t image;
    tivx_obj_desc_remap_t remap;
    tivx_obj_desc_matrix_t matrix;
    tivx_obj_desc_lut_t lut;
    tivx_obj_desc_pyramid_t pyramid;
    tivx_obj_desc_convolution_t convolution;
    tivx_obj_desc_threshold_t threshold;
    tivx_obj_desc_distribution_t distribution;
    tivx_obj_desc_array_t array;
    tivx_obj_desc_object_array_t objarray;
    tivx_obj_desc_tensor_t tensor;
    tivx_obj_desc_scalar_t scalar;
    tivx_obj_desc_kernel_name_t kernel_name;
    tivx_obj_desc_queue_t obj_desc_queue;
    tivx_obj_desc_data_ref_q_t obj_desc_data_ref_q;
    tivx_obj_desc_graph_t obj_desc_graph;
    
    uint8_t rsv[TIVX_OBJ_DESC_MAX_SHM_ENTRY_SIZE];

} tivx_obj_desc_shm_entry_t;

/*!
 * \brief Data structure to hold info about object descriptor table
 *
 * \ingroup group_tivx_obj_desc_priv
 */
typedef struct {

    /*! \brief Object descriptor table base address */
    tivx_obj_desc_shm_entry_t *table_base;

    /*! \brief Object descriptor table, number of entries */
    uint32_t num_entries;

    /*! \brief Index of last allocated entry, this can be used to optimize
     *  free entry search start index during object descriptor alloc
     */
    uint32_t last_alloc_index;

} tivx_obj_desc_table_info_t;

/*!
 * \brief Allocate a Object descriptor
 *
 * \param type [in] Type of object descriptor to allocate, see \ref tivx_obj_desc_type_e
 * \param ref [in] vx_reference that is associated with this object descriptor.
 *                  If not applicable set to NULL
 *
 * \return Pointer \ref tivx_obj_desc_t on success
 * \return NULL, if object descriptor could not be allocated
 *
 * \ingroup group_tivx_obj_desc_priv
 */
tivx_obj_desc_t *tivxObjDescAlloc(vx_enum type, vx_reference ref);

/*!
 * \brief Free a previously allocated object descriptor
 *
 * \param [in] obj_desc Object descriptor to free
 *
 * \ingroup group_tivx_obj_desc_priv
 */
vx_status tivxObjDescFree(tivx_obj_desc_t **obj_desc);


/*!
 * \brief Sends a object descriptor to specified target
 *
 *        The API does not wait for ACK. ACK handling if any is done by user
 *        The API may result in a IPC is target is on another CPU
 *
 *        Source target ID is not required since ACK is not handled by this API
 *
 * \param [in] dst_target_id Destination target ID
 * \param [in] obj_desc_id   Object descriptor ID
 *
 * \ingroup group_tivx_obj_desc_priv
 */
vx_status tivxObjDescSend(uint32_t dst_target_id, uint16_t obj_desc_id);

/*!
 * \brief Get obj descriptor corresponding to the object descriptor ID
 *
 *        If obj_desc_id is invalid or out of bounds NULL is returned.
 *
 * \param [in] obj_desc_id Object descriptor ID
 *
 * \ingroup group_tivx_obj_desc_priv
 */
tivx_obj_desc_t *tivxObjDescGet(uint16_t obj_desc_id);

/*!
 * \brief Checks if object desc pointer is valid and it is of required type
 *
 * \ingroup group_tivx_obj_desc_priv
 */
vx_bool tivxObjDescIsValidType(tivx_obj_desc_t *obj_desc, tivx_obj_desc_type_e type);

/*!
 * \brief Init object descriptor module
 *
 * \ingroup group_tivx_obj_desc_priv
 */
void tivxObjDescInit(void);

/*!
 * \brief Function to get the descriptor object from the given reference
 *
 * \ingroup group_tivx_obj_desc_priv
 */
uint16_t tivxReferenceGetObjDescId(vx_reference ref);

#ifdef __cplusplus
}
#endif

#endif
