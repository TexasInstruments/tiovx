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
    /*! \brief delay for objects within an object array */
    vx_delay obj_arr_delay[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    /*! number of levels in pyramid object */
    vx_size pyr_num_levels;
    /*! number of items in object array object */
    vx_size obj_arr_num_items;
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
