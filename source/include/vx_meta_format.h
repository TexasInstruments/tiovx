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


#ifndef _VX_META_FORMAT_H_
#define _VX_META_FORMAT_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Meta Format Object
 */

/*!
 * \brief Meta Format object internal state
 *
 * \ingroup group_vx_meta_format
 */
typedef struct _vx_meta_format
{
    /*! \brief reference object */
    tivx_reference_t base;

    /*!< \brief The size of struct. */
    vx_size size;
    /*!< \brief The type of meta data */
    vx_enum type;

    /*!< \brief structure containing information about image
                used when type is set to VX_TYPE_IMAGE */
    struct image {
        /*!< \brief The width of the image in pixels */
        vx_uint32 width;
        /*!< \brief The height of the image in pixels */
        vx_uint32 height;
        /*!< \brief The format of the image. */
        vx_df_image format;
    } img;

    /*!< \brief structure containing information about pyramid
                used when type is set to VX_TYPE_PYRAMID */
    struct pyramid {
        /*!< \brief The width of the 0th image in pixels. */
        vx_uint32 width;
        /*!< \brief The height of the 0th image in pixels. */
        vx_uint32 height;
        /*!< \brief The <tt>\ref vx_df_image_e</tt> format of the image. */
        vx_df_image format;
        /*!< \brief The number of scale levels */
        vx_size levels;
        /*!< \brief The ratio between each level */
        vx_float32 scale;
    } pmd;

    /*!< \brief structure containing information about scalar
                used when type is set to VX_TYPE_SCALAR */
    struct scalar {
        /*!< \brief The type of the scalar */
        vx_enum type;
    } sc;

    /*!< \brief structure containing information about array
                used when type is set to VX_TYPE_ARRAY */
    struct array {
        /*!< \brief The type of the Array items */
        vx_enum item_type;
        /*!< \brief The capacity of the Array */
        vx_size capacity;
    } arr;

} tivx_meta_format_t;


/*! \brief Releases a meta-format object.
 *
 * \param [in] pmeta
 *
 * \ingroup group_int_meta_format
 */
vx_status vxReleaseMetaFormat(vx_meta_format *meta);

/*! \brief Creates a metaformat object.
 *
 * \param [in] context The overall context object.
 *
 * \return reference to vx_meta_format on success
 *         NULL on error
 *
 * \ingroup group_int_meta_format
 */
vx_meta_format vxCreateMetaFormat(vx_context context);


#ifdef __cplusplus
}
#endif

#endif
