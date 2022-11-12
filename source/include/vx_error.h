/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
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


#ifndef TIVX_ERROR_H_
#define TIVX_ERROR_H_

/*!
 * \file
 * \brief The internal error implementation
 *
 * \brief The Internal Error API.
 */

#ifdef __cplusplus
    extern "C" {
#endif

/*! \brief The internal representation of the error object.
 * \ingroup group_vx_error
 */
typedef struct _vx_error {
    /*! \brief The "base" reference object. */
    tivx_reference_t base;
    /*! \brief The specific error code contained in this object. */
    vx_status status;
} tivx_error_t;

/*! \brief Creates all the status codes as error objects.
 * \ingroup group_vx_error
 */
vx_bool ownCreateConstErrors(vx_context context);

/*! \brief Matches the status code against all known error objects in the
 * context.
 * \param [in] context The pointer to the overall context.
 * \param [in] status The status code to find.
 * \return Returns a matching error object.
 * \ingroup group_vx_error
 */
vx_reference ownGetErrorObject(vx_context context, vx_status status);

#ifdef __cplusplus
}
#endif

#endif

