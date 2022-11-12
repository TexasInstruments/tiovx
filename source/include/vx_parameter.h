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
