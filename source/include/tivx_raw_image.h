/*
 * Copyright (c) 2018 The Khronos Group Inc.
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



#ifndef TIVX_RAW_IMAGE_H_
#define TIVX_RAW_IMAGE_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Raw Image object
 */

/*!
 * \brief Information about a raw image mapping
 *
 * \ingroup group_tivx_raw_image
 */
typedef struct _tivx_raw_image_map_info_t
{
    /*! \brief Address mapped via tivxMapRawImagePatch() */
    uint8_t *map_addr;
    /*! \brief Size of memory region mapped via tivxMapRawImagePatch() */
    vx_size  map_size;
    /*! \brief Type of memory mapped via tivxMapRawImagePatch(), see \ref vx_memory_type_e and \ref tivx_memory_type_e */
    vx_enum  mem_type;
    /*! \brief Type of access being done by user, see \ref vx_accessor_e */
    vx_enum usage;
} tivx_raw_image_map_info_t;



/*!
 * \brief Image object internal state
 *
 * \ingroup group_tivx_raw_image
 */
typedef struct _tivx_raw_image
{
    /*! \brief reference object */
    tivx_reference_t base;
    /*! \brief A pointer to a parent raw image object. */
    tivx_raw_image       parent;
    /*! \brief The array of raw images derived form this raw image */
    tivx_raw_image       subimages[TIVX_RAW_IMAGE_MAX_SUBIMAGES];
    /*! \brief Mapping done via vxMapImagePatch() */
    tivx_raw_image_map_info_t maps[TIVX_RAW_IMAGE_MAX_MAPS];
    /*! \brief offset from mem_ptr to reach valid first pixel, in case raw image is created from ROI */
    uint32_t mem_offset[TIVX_RAW_IMAGE_MAX_EXPOSURES];
} tivx_raw_image_t;


/*!
 * \brief Function to derive raw image pointers such as pixel buffer,
 *        meta before, meta after from alloc pointer. This function
 *        should be called after freshly allocating raw image buffer
 *        or after importing an external buffer.
 *
 * \param ref [in] reference to tivx_raw_image
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_raw_image
 */
vx_status ownDeriveRawImageBufferPointers(vx_reference ref);

#ifdef __cplusplus
}
#endif

#endif
