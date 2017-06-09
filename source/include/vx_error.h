/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
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
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
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

