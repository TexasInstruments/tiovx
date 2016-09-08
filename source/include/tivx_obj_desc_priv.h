/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_OBJ_DESC_PRIV_H_
#define _TIVX_OBJ_DESC_PRIV_H_

#include <TI/tivx_obj_desc.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Internal private implementation of object descriptor
 */

/*! \brief Max object descriptors than be parameters in a command object
 * \ingroup group_tivx_obj_desc_priv
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


/*!
 * \brief Command object descriptor
 *
 * \ingroup group_tivx_obj_desc_priv
 */
typedef struct _tivx_obj_desc_cmd
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;

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

    /*! \brief Handle of ACK event that is posted when ACK is received
     *     MUST be valid if flags TIVX_CMD_FLAG_SEND_ACK is set
     */
    uint32_t ack_event_handle;

    /*! \brief Number of object descriptor parameters with this command */
    uint32_t num_obj_desc;

    /*! \brief object descriptor ID's of parameters */
    uint16_t obj_desc_id[TIVX_CMD_MAX_OBJ_DESCS];

    /*! \brief command execution status */
    uint32_t cmd_status;

} tivx_obj_desc_cmd_t;

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
    tivx_obj_desc_objarray_t objarray;
    tivx_obj_desc_scalar_t scalar;

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
 * \param type [in] Type of object descriptor to allcoate, see \ref tivx_obj_desc_type_e
 *
 * \return Pointer \ref tivx_obj_desc_t on success
 * \return NULL, if object descriptor could not be allocated
 *
 * \ingroup group_tivx_obj_desc_priv
 */
tivx_obj_desc_t *tivxObjDescAlloc(vx_enum type);

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
void tivxObjDescInit();

#ifdef __cplusplus
}
#endif

#endif
