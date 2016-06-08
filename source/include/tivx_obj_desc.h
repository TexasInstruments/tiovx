/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_OBJ_DESC_H_
#define _TIVX_OBJ_DESC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to object descriptor
 */

/*!
 * \brief Enum that list all possible object descriptor type's
 *
 * \ingroup group_tivx_obj_desc
 */
typedef enum _tivx_obj_desc_type_e {

    /*! \brief Object desciptor that has information related to image object */
    TIVX_OBJ_DESC_IMAGE,

    /*! \brief Object desciptor that has information related to scalar object */
    TIVX_OBJ_DESC_SCALAR,

    /*! \brief Object desciptor that has information related to remap object */
    TIVX_OBJ_DESC_REMAP,

    /*! \brief Value of a invalid object descriptor */
    TIVX_OBJ_DESC_INVALID = 0xFFFFu

} tivx_obj_desc_type_e;

/*!
 * \brief Object descriptor
 *
 *        Must be first element of any objects descriptors
 *        placed in shared memory
 *
 *        Make sure this structure is multiple of 64b
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_t {

    /*! \brief ID of object in shared memory */
    uint16_t obj_desc_id;

    /*! \brief Type of object descritor, see \ref tivx_obj_desc_type_e */
    uint16_t type;

    /*! \brief reserved field to make this structure a multiple of 64b */
    uint16_t rsv[2];

} tivx_obj_desc_t;


/*!
 * \brief Allocate a Object descriptor
 *
 * \param type [in] Type of object descriptor to allcoate, see \ref tivx_obj_desc_type_e
 *
 * \return Pointer \ref tivx_obj_desc_t on success
 * \return NULL, if object descriptor could not be allocated
 *
 * \ingroup group_tivx_obj_desc
 */
tivx_obj_desc_t *tivxObjDescAlloc(vx_enum type);

/*!
 * \brief Free a previously allocated object descriptor
 *
 * \param [in] obj_desc Object descriptor to free
 *
 * \ingroup group_tivx_obj_desc
 */
vx_status tivxObjDescFree(tivx_obj_desc_t **obj_desc);


#ifdef __cplusplus
}
#endif

#endif
