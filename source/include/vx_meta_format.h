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
    struct {
        /*!< \brief The width of the image in pixels */
        vx_uint32 width;
        /*!< \brief The height of the image in pixels */
        vx_uint32 height;
        /*!< \brief The format of the image. */
        vx_df_image format;
    } img;

    /*!< \brief structure containing information about pyramid
                used when type is set to VX_TYPE_PYRAMID */
    struct {
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
    struct {
        /*!< \brief The type of the scalar */
        vx_enum type;
    } sc;

    /*!< \brief structure containing information about array
                used when type is set to VX_TYPE_ARRAY */
    struct {
        /*!< \brief The type of the Array items */
        vx_enum item_type;
        /*!< \brief The capacity of the Array */
        vx_size capacity;
    } arr;

    /*!< \brief structure containing information about matrix
                used when type is set to VX_TYPE_MATRIX */
    struct {
        /*! The value type of the matrix */
        vx_enum type;
        /*! The M dimension of the matrix */
        vx_size rows;
        /*! The N dimension of the matrix */
        vx_size cols;
    } mat;

    /*!< \brief structure containing information about distribution
                used when type is set to VX_TYPE_DISTRIBUTION */
    struct {
        /*! Indicates the number of bins. */
        vx_size bins;
        /*! Indicates the start of the values to use (inclusive). */
        vx_int32 offset;
        /*! Indicates the total number of the consecutive values of the distribution interval. */
        vx_uint32 range;
    } dist;

    /*!< \brief structure containing information about remap
                used when type is set to VX_TYPE_REMAP */
    struct {
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
    struct {
        /*! Indicates the value type of the LUT. */
        vx_enum type;
        /*! Indicates the number of elements in the LUT */
        vx_size count;
    } lut;

    /*!< \brief structure containing information about threshold
                used when type is set to VX_TYPE_THRESHOLD */
    struct {
        /*! The value type of the threshold */
        vx_enum type;
    } thres;

    /*!< \brief structure containing information about object array
                used when type is set to VX_TYPE_OBJECT_ARRAY */
    struct {
        /*! The type of the ObjectArray items */
        vx_enum item_type;
        /*! The number of items in the ObjectArray */
        vx_size num_items;
    } objarr;

    /*!< \brief structure containing information about tensor
                used when type is set to VX_TYPE_TENSOR */
    struct {
        /*! \brief The number of dimensions in the tensor */
        vx_size number_of_dimensions;
        /*! \brief The size of all dimensions */
        vx_size dimensions[TIVX_CONTEXT_MAX_TENSOR_DIMS];
        /*! \brief The data type of tensor */
        vx_enum data_type;
        /*! \brief The fixed point precision of the tensor */
        vx_int8 fixed_point_position;
    } tensor;

    /*!< \brief structure containing information about user data object
                used when type is set to VX_TYPE_USER_DATA_OBJECT */
    struct {
        /*!< \brief The type name of the user data object. */
        vx_char type_name[VX_MAX_REFERENCE_NAME];
        /*!< \brief The size in bytes of the user data object */
        vx_size size;
    } user_data_object;

    /*!< \brief structure containing information about raw image
                used when type is set to TIVX_TYPE_RAW_IMAGE */
    tivx_raw_image_create_params_t raw_image;

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

/*!
 * \brief Check for equivalence between two meta format objects
 * \details This API is used to check for equivalence between two meta format
 *          objects. The function returns true if they are equal and false if not.
 *
 * \param [in] meta1 First meta format object to be compared.
 * \param [in] meta2 Second meta format object to be compared.
 * \param [in] type  Type of both meta format objects to be compared.
 *                   Note: this must be the same for each meta object. This information
 *                   cannot be extracted from the meta object because the type gets set
 *                   at verify time and this could be called prior.
 *
 * \return vx_true_e in case of equal meta formats, vx_false else
 *
 * \ingroup group_int_meta_format
 */
vx_bool ownIsMetaFormatEqual(vx_meta_format meta1, vx_meta_format meta2, vx_enum type);

#ifdef __cplusplus
}
#endif

#endif
