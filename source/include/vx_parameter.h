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



#ifndef VX_PARAMETER_H_
#define VX_PARAMETER_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Parameter object
 */



/*!
 * \brief Parameter object internal state
 *
 * \ingroup group_vx_parameter
 */
typedef struct _vx_parameter
{
    /*! \brief reference object */
    tivx_reference_t base;

    /*! \brief Index at which this parameter is tracked in both the node references and kernel signatures */
    vx_uint32      index;
    /*! \brief Pointer to the node which this parameter is associated with */
    vx_node        node;
    /*! \brief Pointer to the kernel which this parameter is associated with, if retreived from
     * \ref vxGetKernelParameterByIndex.
     */
    vx_kernel      kernel;

} tivx_parameter_t;


/*! \brief This returns true if the direction is a valid enum
 * \param [in] dir The \ref vx_direction_e enum.
 * \ingroup group_vx_parameter
 */
vx_bool ownIsValidDirection(vx_enum dir);

/*! \brief This returns true if the supplied type matches the expected type with
 * some fuzzy rules.
 * \ingroup group_vx_parameter
 */
vx_bool ownIsValidTypeMatch(vx_enum expected, vx_enum supplied);

/*! \brief This returns true if the supplied state is a valid enum.
 * \ingroup group_vx_parameter
 */
vx_bool ownIsValidState(vx_enum state);





#ifdef __cplusplus
}
#endif

#endif
