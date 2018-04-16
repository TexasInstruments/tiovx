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



#ifndef VX_DELAY_H_
#define VX_DELAY_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Delay object
 */

/*! \brief The internal representation of the delay parameters as a list.
 * \ingroup group_vx_delay
 */
typedef struct _vx_delay_param_t {
    struct _vx_delay_param_t *next;
    vx_node node;
    vx_uint32 index;
} tivx_delay_param_t;


/*!
 * \brief Delay object internal state
 *
 * \ingroup group_vx_delay
 */
typedef struct _vx_delay
{
    /*! \brief reference object */
    tivx_reference_t base;
    /*! \brief reference's of object within delay */
    vx_reference     refs[TIVX_DELAY_MAX_OBJECT];
    /*! \brief information about delay reference's */
    tivx_delay_param_t set[TIVX_DELAY_MAX_OBJECT];
    /*! \brief current delay position */
    vx_uint32 index;
    /*! number of objects within delay */
    vx_uint32 count;
    /*! \brief object type within the delay */
    vx_enum type;
    /*! \brief delay for objects within a pyramid */
    vx_delay pyr_delay[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    /*! number of levels in pyramid object */
    vx_size pyr_num_levels;
    /*! Maximum number of paramter objects that can be associated with a delay */
    tivx_delay_param_t prm_pool[TIVX_DELAY_MAX_PRM_OBJECT];
} tivx_delay_t;


/*! \brief Adds an association to a node to a delay slot object reference.
 * \param [in] value The delay slot object reference.
 * \param [in] n The node reference.
 * \param [in] i The index of the parameter.
 *
 * \ingroup group_vx_delay
 */
vx_bool ownAddAssociationToDelay(vx_reference value,
                                vx_node n, vx_uint32 i);

/*! \brief Removes an association to a node from a delay slot object reference.
 * \param [in] value The delay slot object reference.
 * \param [in] n The node reference.
 * \param [in] i The index of the parameter.
 *
 * \ingroup group_vx_delay
 */
vx_bool ownRemoveAssociationToDelay(vx_reference value,
                                   vx_node n, vx_uint32 i);


#ifdef __cplusplus
}
#endif

#endif
