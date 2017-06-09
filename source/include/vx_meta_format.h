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



#ifndef VX_META_FORMAT_H_
#define VX_META_FORMAT_H_


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

    /*! only valid for image and pyramid type */
    vx_kernel_image_valid_rectangle_f valid_rect_callback;

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

    /*!< \brief structure containing information about matrix
                used when type is set to VX_TYPE_MATRIX */
    struct matrix {
        /*! The value type of the matrix */
        vx_enum type;
        /*! The M dimension of the matrix */
        vx_size rows;
        /*! The N dimension of the matrix */
        vx_size cols;
    } mat;

    /*!< \brief structure containing information about distribution
                used when type is set to VX_TYPE_DISTRIBUTION */
    struct distribution {
        /*! Indicates the number of bins. */
        vx_size bins;
        /*! Indicates the start of the values to use (inclusive). */
        vx_int32 offset;
        /*! Indicates the total number of the consecutive values of the distribution interval. */
        vx_uint32 range;
    } dist;

    /*!< \brief structure containing information about remap
                used when type is set to VX_TYPE_REMAP */
    struct _remap {
        /*! The source width */
        vx_uint32 src_width;
        /*! The source height */
        vx_uint32 src_height;
        /*! The destination width */
        vx_uint32 dst_width;
        /*! The destination height */
        vx_uint32 dst_height;
    } remap;

    /*!< \brief structure containing information about lut
                used when type is set to VX_TYPE_LUT */
    struct _lut {
        /*! Indicates the value type of the LUT. */
        vx_enum type;
        /*! Indicates the number of elements in the LUT */
        vx_size count;
    } lut;

    /*!< \brief structure containing information about threshold
                used when type is set to VX_TYPE_THRESHOLD */
    struct threshold {
        /*! The value type of the threshold */
        vx_enum type;
    } thres;

    /*!< \brief structure containing information about object array
                used when type is set to VX_TYPE_OBJECT_ARRAY */
    struct object_array {
        /*! The type of the ObjectArray items */
        vx_enum item_type;
        /*! The number of items in the ObjectArray */
        vx_size num_items;
    } objarr;

} tivx_meta_format_t;


/*! \brief Releases a meta-format object.
 *
 * \param [in] meta meta format object
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
