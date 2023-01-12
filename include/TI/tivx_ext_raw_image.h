/*
*
* Copyright (c) 2018 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/
#ifndef _TIVX_EXT_RAW_IMAGE_H_
#define _TIVX_EXT_RAW_IMAGE_H_

/*!
 * \file
 * \brief The TI Raw Image extension.
 */

/*!
 * \defgroup group_raw_image Raw Image Data Type APIs
 * \brief APIs creating and using raw image data type
 * \ingroup group_tivx_ext_host
 */

#define TIVX_RAW_IMAGE  "tivx_raw_image"

#include <VX/vx.h>

#ifdef  __cplusplus
extern "C" {
#endif

/*! \brief The object type enumeration for raw images.
 * \ingroup group_raw_image
 */
#define TIVX_TYPE_RAW_IMAGE          0x817 /*!< \brief A <tt>\ref tivx_raw_image</tt>. */


/*! \brief The enum type enumeration for raw images.
 * \ingroup group_raw_image
 */
#define TIVX_ENUM_RAW_IMAGE_BUFFER_ACCESS      (vx_enum)0x0   /*!< \brief A <tt>\ref tivx_raw_image_buffer_access_e</tt>. */
#define TIVX_ENUM_RAW_IMAGE_PIXEL_CONTAINER    (vx_enum)0x1   /*!< \brief A <tt>\ref tivx_raw_image_pixel_container_e</tt>. */

/*! \brief Maximum number of RAW image exposures that can be contained in a raw image object.
 * \ingroup group_raw_image
 */
#define TIVX_RAW_IMAGE_MAX_EXPOSURES  3

/*! \brief The Raw Image Object. Raw Image is a strongly-typed container for RAW sensor images.
 * \ingroup group_raw_image
 */
typedef struct _tivx_raw_image * tivx_raw_image;

/*! \brief The raw image format structure that is given to the tivxCreateRawImage function.
 * \ingroup group_raw_image
 */
typedef struct _tivx_raw_image_format_t {
    volatile vx_uint32 pixel_container;      /*!< \brief Pixel Container, see \ref tivx_raw_image_pixel_container_e */
    volatile vx_uint32 msb;                  /*!< \brief Most significant bit in pixel container */
} tivx_raw_image_format_t;

/*! \brief The raw image create params structure that is given to the tivxCreateRawImage function.
 * \ingroup group_raw_image
 */
typedef struct _tivx_raw_image_create_params_t {
    volatile vx_uint32 width;                 /*!< \brief The image width in pixels */
    volatile vx_uint32 height;                /*!< \brief The image height in lines (not including meta rows). */
    volatile vx_uint32 num_exposures;         /*!< \brief The number of exposures contained in the sensor readout for a given timestamp.
                                                 Max supported is TIVX_RAW_IMAGE_MAX_EXPOSURES. */
    volatile vx_bool line_interleaved;        /*!< \brief A value of vx_true_e indicates that the exposures are line interleaved
                                                 in a single contiguous buffer. */
    tivx_raw_image_format_t format[TIVX_RAW_IMAGE_MAX_EXPOSURES]; /*!< \brief Array of tivx_raw_image_format_t structures indicating the pixel packing and
        bit alignment format of each exposure.  If line_interleaved == vx_false_e, then the number of
        valid structures in this array should be equal to the value of num_exposures.  If line_interleaved ==
        vx_true_e, then the format should be the same for each exposure in a single buffer, so the
        number of valid structures in this array should equal 1. */
    volatile vx_uint32 meta_height_before;    /*!< \brief Number of lines of meta data at top of sensor readout  (before pixel data)
                                                 (uses the same width as original sensor readout width) */
    volatile vx_uint32 meta_height_after;     /*!< \brief Number of lines of meta data at bottom of sensor readout  (after pixel data)
                                                 (uses the same width as original sensor readout width) */
} tivx_raw_image_create_params_t;

/*! \brief The raw image attributes.
 * \ingroup group_raw_image
 */
enum tivx_raw_image_attribute_e {
    /*! \brief Queries an image for its width. Read-only. Use a <tt>\ref vx_uint32</tt> parameter. */
    TIVX_RAW_IMAGE_WIDTH = VX_ATTRIBUTE_BASE(VX_ID_TI, TIVX_TYPE_RAW_IMAGE) + 0x0,
    /*! \brief Queries an image for its height. Read-only. Use a <tt>\ref vx_uint32</tt> parameter. */
    TIVX_RAW_IMAGE_HEIGHT = VX_ATTRIBUTE_BASE(VX_ID_TI, TIVX_TYPE_RAW_IMAGE) + 0x1,
    /*! \brief Queries an image for its number of exposures. Read-only. Use a <tt>\ref vx_uint32</tt> parameter. */
    TIVX_RAW_IMAGE_NUM_EXPOSURES = VX_ATTRIBUTE_BASE(VX_ID_TI, TIVX_TYPE_RAW_IMAGE) + 0x2,
    /*! \brief Queries an image for if its exposures are line interleaved in memory. Read-only. Use a <tt>\ref vx_bool</tt> parameter. */
    TIVX_RAW_IMAGE_LINE_INTERLEAVED = VX_ATTRIBUTE_BASE(VX_ID_TI, TIVX_TYPE_RAW_IMAGE) + 0x3,
    /*! \brief Queries an image for its format (see <tt>\ref tivx_raw_image_format_t</tt>). Read-only. Use a pointer to a <tt>\ref tivx_raw_image_format_t</tt> array. */
    TIVX_RAW_IMAGE_FORMAT = VX_ATTRIBUTE_BASE(VX_ID_TI, TIVX_TYPE_RAW_IMAGE) + 0x4,
    /*! \brief Queries an image for its meta height at top of readout. Read-only. Use a <tt>\ref vx_uint32</tt> parameter. */
    TIVX_RAW_IMAGE_META_HEIGHT_BEFORE = VX_ATTRIBUTE_BASE(VX_ID_TI, TIVX_TYPE_RAW_IMAGE) + 0x5,
    /*! \brief Queries an image for its meta height at bottom of readout. Read-only. Use a <tt>\ref vx_uint32</tt> parameter. */
    TIVX_RAW_IMAGE_META_HEIGHT_AFTER = VX_ATTRIBUTE_BASE(VX_ID_TI, TIVX_TYPE_RAW_IMAGE) + 0x6,
    /*! \brief Queries an image for its addressing structure. Read-only. Use a <tt>\ref vx_imagepatch_addressing_t[TIVX_RAW_IMAGE_MAX_EXPOSURES]</tt> parameter. */
    TIVX_RAW_IMAGE_IMAGEPATCH_ADDRESSING = VX_ATTRIBUTE_BASE(VX_ID_TI, TIVX_TYPE_RAW_IMAGE) + 0x7
};

/*! \brief The raw image buffer access enum.
 * \ingroup group_raw_image
 */
enum tivx_raw_image_buffer_access_e {
    /*! \brief For accessing pointer to full allocated buffer (pixel buffer + meta buffer). */
    TIVX_RAW_IMAGE_ALLOC_BUFFER = VX_ENUM_BASE(VX_ID_TI, TIVX_ENUM_RAW_IMAGE_BUFFER_ACCESS) + 0x0,
    /*! \brief For accessing pointer to pixel buffer only */
    TIVX_RAW_IMAGE_PIXEL_BUFFER = VX_ENUM_BASE(VX_ID_TI, TIVX_ENUM_RAW_IMAGE_BUFFER_ACCESS) + 0x1,
    /*! \brief For accessing pointer to meta buffer only */
    TIVX_RAW_IMAGE_META_BEFORE_BUFFER = VX_ENUM_BASE(VX_ID_TI, TIVX_ENUM_RAW_IMAGE_BUFFER_ACCESS) + 0x2,
    /*! \brief For accessing pointer to meta buffer only */
    TIVX_RAW_IMAGE_META_AFTER_BUFFER = VX_ENUM_BASE(VX_ID_TI, TIVX_ENUM_RAW_IMAGE_BUFFER_ACCESS) + 0x3
};

/*! \brief The raw image pixel container enum.
 * \ingroup group_raw_image
 */
enum tivx_raw_image_pixel_container_e {
    /*! \brief Two bytes per pixel in memory. */
    TIVX_RAW_IMAGE_16_BIT = VX_ENUM_BASE(VX_ID_TI, TIVX_ENUM_RAW_IMAGE_PIXEL_CONTAINER) + 0x0,
    /*! \brief One byte per pixel in memory. */
    TIVX_RAW_IMAGE_8_BIT = VX_ENUM_BASE(VX_ID_TI, TIVX_ENUM_RAW_IMAGE_PIXEL_CONTAINER) + 0x1,
    /*! \brief Packed 12 bit mode; Three bytes per two pixels in memory. */
    TIVX_RAW_IMAGE_P12_BIT = VX_ENUM_BASE(VX_ID_TI, TIVX_ENUM_RAW_IMAGE_PIXEL_CONTAINER) + 0x2
};

/*! \brief Creates an opaque reference to a raw sensor image (including multi-exposure and metadata).
 * \details Not guaranteed to exist until the <tt>\ref vx_graph</tt> containing it has been verified.
 *
 * \param [in] context         The reference to the implementation context.
 * \param [in] params          The pointer to a \ref tivx_raw_image_create_params_t structure
 *
 * \returns A raw image reference <tt>\ref tivx_raw_image</tt>. Any possible errors preventing a successful
 * creation should be checked using <tt>\ref vxGetStatus</tt>.
 *
 * \see tivxMapRawImagePatch to obtain direct memory access to the image data.
 *
 * \ingroup group_raw_image
 */
VX_API_ENTRY tivx_raw_image VX_API_CALL tivxCreateRawImage(vx_context context,
                                                           tivx_raw_image_create_params_t *params);

/*!
 * \brief Releases a reference of a raw image.
 * The object may not be garbage collected until its total reference count is zero.
 * After returning from this function the reference is zeroed.
 * \param [in] raw_image  The pointer to the Raw Image Object to release.
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 * \retval VX_ERROR_INVALID_REFERENCE If raw_image is not a <tt>\ref tivx_raw_image</tt>.
 * \ingroup group_raw_image
 */
VX_API_ENTRY vx_status VX_API_CALL tivxReleaseRawImage(tivx_raw_image *raw_image);

/*!
 * \brief Queries the raw image for some specific information.
 *
 * \param [in] raw_image         The reference to the raw image.
 * \param [in] attribute         The attribute to query. Use a <tt>\ref tivx_raw_image_attribute_e</tt>.
 * \param [out] ptr              The location at which to store the resulting value.
 * \param [in] size              The size in bytes of the container to which \a ptr points.
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS                   No errors.
 * \retval VX_ERROR_INVALID_REFERENCE   If the \a raw_image is not a <tt>\ref tivx_raw_image</tt>.
 * \retval VX_ERROR_NOT_SUPPORTED       If the \a attribute is not a value supported on this implementation.
 * \retval VX_ERROR_INVALID_PARAMETERS  If any of the other parameters are incorrect.
 *
 * \ingroup group_raw_image
 */
VX_API_ENTRY vx_status VX_API_CALL tivxQueryRawImage (tivx_raw_image raw_image,
                                                      vx_enum attribute,
                                                      volatile void *ptr,
                                                      vx_size size);

/*! \brief Allows the application to copy a rectangular patch from/into a raw image object exposure.
 *
 * \param [in] raw_image            The reference to the raw image object that is the source or the
 *                                  destination of the copy.
 * \param [in] rect                 The coordinates of the image patch. The patch must be within
 *                                  the bounds of the image. (start_x, start_y) gives the coordinates of the topleft
 *                                  pixel inside the patch, while (end_x, end_y) gives the coordinates of the bottomright
 *                                  element out of the patch. Must be 0 <= start < end <= number of pixels in the image
 *                                  dimension.  Only valid when buffer_select is TIVX_RAW_IMAGE_PIXEL_BUFFER.
 * \param [in] exposure_index       The exposure index of the raw image object that is the source or the
 *                                  destination of the patch copy.
 * \param [in] user_addr            The address of a structure describing the layout of the
 *                                  user memory location pointed by user_ptr. In the structure, only dim_x, dim_y,
 *                                  stride_x and stride_y fields must be provided, other fields are ignored by the function.
 *                                  The layout of the user memory must follow a row major order:
 *                                  stride_x == pixel size in bytes, and stride_y >= stride_x * dim_x.
 *                                  When buffer_select is not TIVX_RAW_IMAGE_PIXEL_BUFFER, only dim_x is valid (number of bytes).
 * \param [in] user_ptr             The address of the memory location where to store the requested data
 *                                  if the copy was requested in read mode, or from where to get the data to store into the image
 *                                  object if the copy was requested in write mode. The accessible memory must be large enough
 *                                  to contain the specified patch with the specified layout:
 *                                  accessible memory in bytes >= (end_y - start_y) * stride_y.
 * \param [in] usage                This declares the effect of the copy with regard to the image object
 *                                  using the <tt>\ref vx_accessor_e</tt> enumeration.  Only VX_READ_ONLY and VX_WRITE_ONLY are supported:
 *                                  \arg VX_READ_ONLY means that data is copied from the image object into the application memory
 *                                  \arg VX_WRITE_ONLY means that data is copied into the image object from the application memory
 * \param [in] mem_type             A <tt>\ref vx_memory_type_e</tt> enumeration that specifies
 *                                  the memory type of the memory referenced by the user_addr.
 * \param [in] buffer_select        A <tt>\ref tivx_raw_image_buffer_access_e</tt> enumeration that specifies
 *                                  the buffer to copy to/from.
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_ERROR_OPTIMIZED_AWAY This is a reference to a virtual raw image that cannot be
 * accessed by the application.
 * \retval VX_ERROR_INVALID_REFERENCE The raw image reference is not actually a raw image reference.
 * \retval VX_ERROR_INVALID_PARAMETERS An other parameter is incorrect.
 *
 * \note The application may ask for data outside the bounds of the valid region, but
 * such data has an undefined value.
 *
 * \ingroup group_raw_image
 */
VX_API_ENTRY vx_status VX_API_CALL tivxCopyRawImagePatch(tivx_raw_image raw_image,
                                                         const vx_rectangle_t *rect,
                                                         vx_uint32 exposure_index,
                                                         const vx_imagepatch_addressing_t *user_addr,
                                                         void * user_ptr,
                                                         vx_enum usage,
                                                         vx_enum mem_type,
                                                         vx_enum buffer_select);

/*! \brief Allows the application to get direct access to a rectangular patch of a raw image object exposure.
 *
 * \param [in] raw_image           The reference to the raw image object that contains the patch to map.
 * \param [in] rect                The coordinates of raw image patch. The patch must be within the
 *                                 bounds of the image. (start_x, start_y) gives the coordinate of the topleft
 *                                 element inside the patch, while (end_x, end_y) give the coordinate of
 *                                 the bottomright element out of the patch. Must be 0 <= start < end.
 *                                 Only valid when buffer_select is TIVX_RAW_IMAGE_PIXEL_BUFFER.
 * \param [in] exposure_index      The exposure index of the image object to be accessed.
 * \param [out] map_id             The address of a vx_map_id variable where the function
 *                                 returns a map identifier.
 *                                 \arg (*map_id) must eventually be provided as the map_id parameter of a call to
 *                                 <tt>\ref tivxUnmapRawImagePatch</tt>.
 * \param [out] addr               The address of a structure describing the memory layout of the
 *                                 image patch to access. The function fills the structure pointed by addr with the
 *                                 layout information that the application must consult to access the pixel data
 *                                 at address (*user_ptr). The layout of the mapped memory follows a row-major order:
 *                                 stride_x>0, stride_y>0 and stride_y >= stride_x * dim_x.
 *                                 If the image object being accessed was created via
 *                                 <tt>\ref tivxCreateRawImageFromHandle</tt>, then the returned memory layout will be
 *                                 the identical to that of the addressing structure provided when
 *                                 <tt>\ref tivxCreateRawImageFromHandle</tt> was called.
 *                                 When buffer_select is not TIVX_RAW_IMAGE_PIXEL_BUFFER, only dim_x is valid (number of bytes).
 * \param [out] user_ptr           The address of a pointer that the function sets to the
 *                                 address where the requested data can be accessed. This returned (*ptr) address
 *                                 is only valid between the call to this function and the corresponding call to
 *                                 <tt>\ref tivxUnmapRawImagePatch</tt>.
 *                                 If image was created via <tt>\ref tivxCreateRawImageFromHandle</tt> then the returned
 *                                 address (*ptr) will be the address of the patch in the original pixel buffer
 *                                 provided when image was created.
 * \param [in] usage               This declares the access mode for the image patch, using
 *                                 the <tt>\ref vx_accessor_e</tt> enumeration.
 *                                 \arg VX_READ_ONLY: after the function call, the content of the memory location
 *                                 pointed by (*ptr) contains the image patch data. Writing into this memory location
 *                                 is forbidden and its behavior is undefined.
 *                                 \arg VX_READ_AND_WRITE : after the function call, the content of the memory
 *                                 location pointed by (*ptr) contains the image patch data; writing into this memory
 *                                 is allowed only for the location of pixels only and will result in a modification
 *                                 of the written pixels in the image object once the patch is unmapped. Writing into
 *                                 a gap between lines (when addr->stride_y > addr->stride_x*addr->dim_x)
 *                                 is forbidden and its behavior is undefined.
 *                                 \arg VX_WRITE_ONLY: after the function call, the memory location pointed by (*ptr)
 *                                 contains undefined data; writing each pixel of the patch is required prior to
 *                                 unmapping. Pixels not written by the application before unmap will become
 *                                 undefined after unmap, even if they were well defined before map. Like for
 *                                 VX_READ_AND_WRITE, writing into a gap between pixels is forbidden and its behavior
 *                                 is undefined.
 * \param [in] mem_type            A <tt>\ref vx_memory_type_e</tt> enumeration that
 *                                 specifies the type of the memory where the image patch is requested to be mapped.
 * \param [in] buffer_select       A <tt>\ref tivx_raw_image_buffer_access_e</tt> enumeration that specifies
 *                                 the buffer to map.
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_ERROR_OPTIMIZED_AWAY This is a reference to a virtual raw image that cannot be
 * accessed by the application.
 * \retval VX_ERROR_INVALID_REFERENCE The raw image reference is not actually a raw image
 * reference.
 * \retval VX_ERROR_INVALID_PARAMETERS An other parameter is incorrect.
 * \note The user may ask for data outside the bounds of the valid region, but
 * such data has an undefined value.
 * \ingroup group_raw_image
 * \post <tt>\ref tivxUnmapRawImagePatch </tt> with same (*map_id) value.
 */
VX_API_ENTRY vx_status VX_API_CALL tivxMapRawImagePatch(tivx_raw_image raw_image,
                                                        const vx_rectangle_t *rect,
                                                        vx_uint32 exposure_index,
                                                        vx_map_id *map_id,
                                                        vx_imagepatch_addressing_t *addr,
                                                        void **user_ptr,
                                                        vx_enum usage,
                                                        vx_enum mem_type,
                                                        vx_enum buffer_select);


/*! \brief Unmap and commit potential changes to a raw image object patch that were previously mapped.
 *
 * Unmapping a raw image patch invalidates the memory location from which the patch could
 * be accessed by the application. Accessing this memory location after the unmap function
 * completes has an undefined behavior.
 * \param [in] raw_image The reference to the raw image object to unmap.
 * \param [out] map_id The unique map identifier that was returned by <tt>\ref tivxMapRawImagePatch</tt>
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_ERROR_INVALID_REFERENCE The raw image reference is not actually a raw image reference.
 * \retval VX_ERROR_INVALID_PARAMETERS An other parameter is incorrect.
 *
 * \ingroup group_raw_image
 * \pre <tt>\ref tivxMapRawImagePatch</tt> with same map_id value
 */
VX_API_ENTRY vx_status VX_API_CALL tivxUnmapRawImagePatch(tivx_raw_image raw_image,
                                                          vx_map_id map_id);

#ifdef  __cplusplus
}
#endif

#endif
