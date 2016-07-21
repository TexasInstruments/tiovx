/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_TARGET_H_
#define _TIVX_TARGET_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Target Object APIs
 */

/*! \brief Command ID for commands that can be send to a target
 * \ingroup group_tivx_target
 */
typedef enum _tivx_target_cmd_e {

    /*! \brief Command to create a node on target */
    TIVX_CMD_NODE_CREATE  = 0x00000001u,
    /*! \brief Command to delete a node on target */
    TIVX_CMD_NODE_DELETE  = 0x00000002u,
    /*! \brief Command to call a user callback, on node execution complete */
    TIVX_CMD_NODE_USER_CALLBACK  = 0x00000003u,

} tivx_target_cmd_e;

/*! \brief Target ID for supported targets
 * \ingroup group_tivx_target
 */
typedef enum _tivx_target_id_e {

    /*! \brief target ID for DSP1 */
    TIVX_TARGET_ID_DSP1 = 0,

    /*! \brief target ID for DSP2 */
    TIVX_TARGET_ID_DSP2 = 1,

    /*! \brief target ID for EVE1 */
    TIVX_TARGET_ID_EVE1 = 2,

    /*! \brief target ID for EVE2 */
    TIVX_TARGET_ID_EVE2 = 3,

    /*! \brief target ID for EVE3 */
    TIVX_TARGET_ID_EVE3 = 4,

    /*! \brief target ID for EVE4 */
    TIVX_TARGET_ID_EVE4 = 5,

    /*! \brief target ID for IPU1-0 */
    TIVX_TARGET_ID_IPU1_0 = 6,

    /*! \brief target ID for IPU1-1 */
    TIVX_TARGET_ID_IPU1_1 = 7,

    /*! \brief target ID for IPU2 */
    TIVX_TARGET_ID_IPU2_0 = 8,

    /*! \brief target ID for A15-0 */
    TIVX_TARGET_ID_A15_0 = 9,

    /*! \brief target ID for invalid target */
    TIVX_TARGET_ID_INVALID = 0xFFFFFFFFu

} tivx_target_id_e;

/*!
 * \brief Convert a target name or target class to a specific target ID
 *
 * \param target_name [in] Target name
 *
 * \return target ID
 *
 * \ingroup group_tivx_target
 */
tivx_target_id_e ownGetTargetId(const char *target_name);

/*!
 * \brief Match a user specified target_string with kernel suported target name
 *
 * \param kernel_target_name [in] Kernel supported target name
 * \param target_string [in] user specified target string
 *
 * \return vx_true_e if match found, else vx_false_e
 *
 * \ingroup group_tivx_target
 */
vx_bool ownTargetMatch(const char *kernel_target_name, const char *target_string);

#ifdef __cplusplus
}
#endif

#endif
