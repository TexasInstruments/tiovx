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



#ifndef VX_IMAGE_H_
#define VX_IMAGE_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Image object
 */

/*!
 * \brief Information about a image mapping
 *
 * \ingroup group_vx_image
 */
typedef struct _tivx_image_map_info_t
{
    /*! \brief Address mapped via vxMapImagePatch() */
    uint8_t *map_addr;
    /*! \brief Size of memory region mapped via vxMapImagePatch() */
    vx_size  map_size;
    /*! \brief Type of memory mapped via vxMapImagePatch(), see \ref vx_memory_type_e and \ref tivx_memory_type_e */
    vx_enum  mem_type;
    /*! \brief Type of access being done by user, see \ref vx_accessor_e */
    vx_enum usage;
} tivx_image_map_info_t;



/*!
 * \brief Image object internal state
 *
 * \ingroup group_vx_image
 */
typedef struct _vx_image
{
    /*! \brief reference object */
    tivx_reference_t base;
    /*! \brief A pointer to a parent image object. */
    vx_image       parent;
    /*! \brief The array of images derived from this image */
    vx_image       subimages[TIVX_IMAGE_MAX_SUBIMAGES];
    /*! \brief The array of tensors derived from this image */
    vx_tensor      subtensors[TIVX_IMAGE_MAX_SUBTENSORS];    
    /*! \brief Mapping done via vxMapImagePatch() */
    tivx_image_map_info_t maps[TIVX_IMAGE_MAX_MAPS];
    /*! \brief offset from mem_ptr to reach valid first pixel, in case image is created from ROI */
    uint32_t mem_offset[TIVX_IMAGE_MAX_PLANES];
    /*! \brief channel_plane index of parent in case image is created from channel */
    uint32_t channel_plane;
    /*! \brief Alignment of the stride in the "y" direction.  This defaults to the value \ref TIVX_DEFAULT_STRIDE_Y_ALIGN,
     *   but can be overwitten using the attribute \ref TIVX_IMAGE_STRIDE_Y_ALIGNMENT */
    uint32_t stride_y_alignment;
} tivx_image_t;

/*!
 * \brief function to initialize virtual image's parameters
 *
 * \param img       [in] virtual image reference
 * \param width     [in] image width
 * \param height    [in] image height
 * \param format    [in] image format
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_vx_image
 */
vx_status ownInitVirtualImage(
    vx_image img, vx_uint32 width, vx_uint32 height, vx_df_image format);

/*!
 * \brief function to retrieve the buffer size of an image
 *
 * \param obj_desc       [in] image object descriptor
 *
 * \return size of buffer in bytes
 *
 * \ingroup group_vx_image
 */
uint32_t ownImageGetBufferSize(tivx_obj_desc_image_t *obj_desc);


/*!
 * \brief function determine if a given image is valid or not
 *
 * \param image       [in] OpenVX image object
 *
 * \return boolean of whether or not the given image is valid
 *
 * \ingroup group_vx_image
 */
vx_bool ownIsValidImage(vx_image image);

#ifdef __cplusplus
}
#endif

#endif
