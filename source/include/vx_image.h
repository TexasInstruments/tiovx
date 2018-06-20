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
    /*! \brief The array of images derived form this image */
    vx_image       subimages[TIVX_IMAGE_MAX_SUBIMAGES];
    /*! \brief Mapping done via vxMapImagePatch() */
    tivx_image_map_info_t maps[TIVX_IMAGE_MAX_MAPS];
    /*! \brief offset from mem_ptr to reach valid first pixel, in case image is created from ROI */
    uint32_t mem_offset[TIVX_IMAGE_MAX_PLANES];
    /*! \brief channel_plane index of parent in case image is created from channel */
    uint32_t channel_plane;
} tivx_image_t;



/*!
 * \brief Print image patch addressing info
 *
 * \param addr [in] Image patch addressing info
 *
 * \ingroup group_vx_image
 */
void ownPrintImageAddressing(const vx_imagepatch_addressing_t *addr);

/*!
 * \brief Print image info
 *
 * \param image [in] Image
 *
 * \ingroup group_vx_image
 */
void ownPrintImage(vx_image image);

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
 * \ingroup group_tivx_array
 */
vx_status ownInitVirtualImage(
    vx_image img, vx_uint32 width, vx_uint32 height, vx_df_image format);

#ifdef __cplusplus
}
#endif

#endif
