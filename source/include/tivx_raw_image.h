/*
 * Copyright (c) 2018 The Khronos Group Inc.
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


#ifdef __cplusplus
}
#endif

#endif
