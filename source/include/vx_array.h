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



#ifndef VX_ARRAY_H_
#define VX_ARRAY_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Array object
 */

/*!
 * \brief Information about a array mapping
 *
 * \ingroup group_vx_array
 */
typedef struct _tivx_array_map_info_t
{
    /*! \brief Address mapped via vxMapArray() */
    uint8_t *map_addr;
    /*! \brief Size of memory region mapped via vxMapArray() */
    vx_size  map_size;
    /*! \brief Type of memory mapped via vxMapArray(), see \ref vx_memory_type_e and \ref tivx_memory_type_e */
    vx_enum  mem_type;
    /*! \brief Type of access being done by user, see \ref vx_accessor_e */
    vx_enum usage;
} tivx_array_map_info_t;

/*!
 * \brief Array object internal state
 *
 * \ingroup group_vx_array
 */
typedef struct _vx_array
{
    /*! \brief reference object */
    tivx_reference_t base;

    /*! \brief Mapping done via vxMapArray() */
    tivx_array_map_info_t maps[TIVX_ARRAY_MAX_MAPS];
} tivx_array_t;



/*!
 * \brief function to initialize virtual array parameters
 *
 * \param arr       [in] virtual array reference
 * \param item_type [in] type of array items
 * \param capacity  [in] array size
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_array
 */
vx_status ownInitVirtualArray(
    vx_array arr, vx_enum item_type, vx_size capacity);

#ifdef __cplusplus
}
#endif

#endif
