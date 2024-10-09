/*
 * Copyright (c) 2023 ETAS Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */
#ifndef _OPENVX_RB_REF_DATA_H_
#define _OPENVX_RB_REF_DATA_H_

#include <VX/vx_khr_user_data_object.h>
#include <VX/vx_khr_supplementary_data.h>
#include <TI/tivx_obj_desc.h>
#ifdef  __cplusplus
extern "C" {
#endif

/* TIOVX ONLY */
/*!
 * \brief Return the object descriptor for the supplementary data object associated with the given object descriptor
 * 
 * \param [in] obj_desc -       The object descriptor to be queried.
 * \param [in] type_name        The name of the user data type.          
 *
 * \return a pointer to the object descriptor of the supplementary data, or NULL
 */
VX_API_ENTRY tivx_obj_desc_user_data_object_t * tivxGetSupplementaryDataObjDesc(tivx_obj_desc_t * obj_desc, const char * type_name);

/* TIOVX ONLY */
/*!
 * \brief Set supplementary data for an object descriptor from a user data object object descriptor by copying bytes from one
 * user data object to another. The number of bytes copied is given by the VX_USER_DATA_OBJECT_VALID_SIZE attribute of the source.
 * After the operation the VX_USER_DATA_OBJECT_VALID_SIZE attribute of the destination supplementary data object is set equal
 * to that of the source.
 * \param [in] destination -    The object descriptor whose supplementary data is to be set
 * \param [in] source           From where the data will be copied
 *
 * \return a status code:
 * \retval VX_ERROR_INVALID_REFERENCE: source or destination is NULL
 * \retval VX_ERROR_INVALID_TYPE: type name or memory size of the destination supplementary data and the source do not match
 * \retval VX_FAILURE: the destination does not have any supplementary data
 * \retval VX_SUCCESS: the data has been copied successfully
 */
VX_API_ENTRY vx_status tivxSetSupplementaryDataObjDesc(tivx_obj_desc_t * destination, const tivx_obj_desc_user_data_object_t * source);

/* TIOVX ONLY */
/* \brief Set supplementary data for an object descriptor from a user data object object descriptor by copying bytes from one
* user data object to another, and then extend the data in the user object by copying bytes from another place.
* \details The maximum number of bytes to copy is given by num_bytes.
* \details Data is first copied from the source object up to its valid size (given by the attribute VX_USER_DATA_OBJECT_VALID_SIZE),
* and then bytes are copied from the user_data, starting an equivalent distance in, until num_bytes have been copied.
* Thus the first part of user_data will be ignored if the valid size of the data in the source is greater than zero.
* If the copy operation reaches num_bytes before the bytes in source have been all copied, the operation stops without copying
* any data from user_data.
* If user_data is NULL, then it will copy a number of bytes from source to the destination supplementary data object, given
* by the minimum of num_bytes and the valid size.
* If source is NULL, then it will copy num_bytes from user_data to the destination supplementary data object and set the valid
* size to num_bytes.
* If num_bytes is greater than the VX_USER_DATA_OBJECT_SIZE attribute of the supplementary data then the function will fail and
* return VX_ERROR_INVALID_VALUE.
* If both user_data and source are NULL, the function will fail and return VX_ERROR_INVALID_PARAMETERS
* After the operation, the valid_mem_size of the destination supplementary data is set to the total number of bytes copied.
 * \param [in] destination      The object descriptor to be queried.
 * \param [in] source           Source of the data to be copied first (vx_user_data_object holding a some_user_data_t) or NULL
 * \param [in] user_data        Pointer to data object to be copy rest of bytes (some_user_data_t), or NULL
 * \param [in] num_bytes        Total number of bytes to copy, from source and user_data combined
 * \return a status code:
 * \retval VX_SUCCESS                   Data copied successfully
 * \retval VX_FAILURE                   There is no supplementary data in destination
 * \retval VX_ERROR_INVALID_VALUE       num_bytes is greater than mem_size of source
 * \retval VX_ERROR_INVALID_TYPE        source type does not match destination supplementary data type
 * \retval VX_ERROR_INVALID REFERENCE   destination is NULL
 * \retval VX_ERROR_INVALID_PARAMETERS  source and user_Data are both NULL
 */
VX_API_ENTRY vx_status tivxExtendSupplementaryDataObjDesc(tivx_obj_desc_t * destination, const tivx_obj_desc_user_data_object_t * source, const void *user_data, vx_uint32 num_bytes);
#ifdef  __cplusplus
}
#endif

#endif
