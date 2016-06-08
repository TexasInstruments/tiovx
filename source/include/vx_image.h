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
/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _VX_IMAGE_H_
#define _VX_IMAGE_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Image object
 */


/*!
 * \brief Max possible planes of data in an image
 *
 * \ingroup group_vx_image
 */
#define TIVX_IMAGE_MAX_PLANES   (3u)

/*!
 * \brief Max possible sub images from a image
 *
 * \ingroup group_vx_image
 */
#define TIVX_IMAGE_MAX_SUBIMAGES     (16u)

/*!
 * \brief Max possible mapping via vxMapImagePatch supported
 *
 * \ingroup group_vx_image
 */
#define TIVX_IMAGE_MAX_MAPS     (16u)

/*!
 * \brief Method by which image is created
 *
 * \ingroup group_vx_image
 */
typedef enum _tivx_image_create_type_e
{
    /*! \brief Create using vxCreateImage() */
    TIVX_IMAGE_NORMAL,
    /*! \brief Create using vxCreateUniformImage() */
    TIVX_IMAGE_UNIFORM,
    /*! \brief Create using vxCreateVirtualImage() */
    TIVX_IMAGE_VIRTUAL,
    /*! \brief Create using vxCreateImageFromHandle() */
    TIVX_IMAGE_FROM_HANDLE,
    /*! \brief Create using vxCreateImageFromROI() */
    TIVX_IMAGE_FROM_ROI,
    /*! \brief Create using vxCreateImageFromChannel() */
    TIVX_IMAGE_FROM_CHANNEL

} tivx_image_create_type_e;


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
    /*! \brief Type of access being done by user, see \ref vx_accessor_e */
    vx_enum usage;
} tivx_image_map_info_t;

/*!
 * \brief Image object descriptor as placed in shared memory
 *
 * \ingroup group_vx_image
 */
typedef struct _tivx_obj_desc_image
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief Width of image in pixels */
    uint32_t width;
    /*! \brief Height of image in lines */
    uint32_t height;
    /*! \brief Data format of image, see \ref vx_df_image */
    uint32_t format;
    /*! \brief Number of data planes in image */
    uint32_t planes;
    /*! \brief Color space of the image, see \ref vx_color_space_e */
    uint32_t color_space;
    /*! \brief Color range of the image channel, see \ref vx_channel_range_e */
    uint32_t color_range;
    /*! \brief image plane buffer size */
    uint32_t mem_size[TIVX_IMAGE_MAX_PLANES];
    /*! \brief the value to use to fill for a uniform image.
     *
     * bit 0.. 7 - Component 0 - R or Y or U8.
     * bit 8..15 - Component 1 - G or U.
     * bit16..23 - Component 2 - B or V.
     * bit24..31 - Component 3 - X.
     *
     * bit0..15 - U16, S16.
     * bit0..31 - U32, S32.
     */
    uint32_t uniform_image_pixel_value;
    /*! \brief method by which image was created, see \ref tivx_image_create_type_e */
    uint32_t create_type;
    /*! \brief image plane buffer addresses */
    tivx_shared_mem_ptr_t mem_ptr[TIVX_IMAGE_MAX_PLANES];
    /*! \brief image plane addressing parameters */
    vx_imagepatch_addressing_t imagepatch_addr[TIVX_IMAGE_MAX_PLANES];
    /*! \brief valid region of image to use for processing */
    vx_rectangle_t valid_roi;
} tivx_obj_desc_image_t;

/*!
 * \brief Image object internal state
 *
 * \ingroup group_vx_image
 */
typedef struct _vx_image
{
    /*! \brief reference object */
    vx_reference_t base;
    /*! \brief object descriptor */
    tivx_obj_desc_image_t *obj_desc;
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
} vx_image_t;



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

#ifdef __cplusplus
}
#endif

#endif
