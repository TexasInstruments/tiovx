/*
 *
 * Copyright (c) 2021 Texas Instruments Incorporated
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
#if !defined(_TIVX_UTILS_IPC_REF_XFER_H_)
#define _TIVX_UTILS_IPC_REF_XFER_H_

#include <TI/tivx.h>
#include <TI/tivx_obj_desc.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief A utility macro to compute a greater of two values.
 */
#define TIVX_UTILS_MAX(a, b) ((a) > (b) ? (a) : (b))

/*!
 * \brief Max possible planes value to be used in the import/export message.
 *        This is a MAX of the following values:
 *        - TIVX_IMAGE_MAX_PLANES * TIVX_PYRAMID_MAX_LEVEL_OBJECTS
 *        - TIVX_CONTEXT_MAX_TENSOR_DIMS
 *        - TIVX_RAW_IMAGE_MAX_EXPOSURES
 */
#define VX_IPC_MAX_VX_PLANES \
            (TIVX_UTILS_MAX(TIVX_UTILS_MAX(TIVX_IMAGE_MAX_PLANES*\
                                           TIVX_PYRAMID_MAX_LEVEL_OBJECTS,\
                                           TIVX_CONTEXT_MAX_TENSOR_DIMS),\
                            TIVX_RAW_IMAGE_MAX_EXPOSURES))

/*!
 * \brief Meta Format object internal state
 *
 */
typedef struct
{
    /*!< \brief The type of meta data */
    vx_enum type;

    /*!< \brief structure containing information about image
                used when type is set to VX_TYPE_IMAGE */
    struct {
        /*!< \brief The width of the image in pixels */
        vx_uint32 width;
        /*!< \brief The height of the image in pixels */
        vx_uint32 height;
        /*!< \brief The format of the image. */
        vx_df_image format;
        /*!< \brief Number of data planes in image */
        vx_uint32 planes;
    } img;

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
        /*!< \brief Number of data planes in image */
        vx_uint32 planes;
    } pmd;

    /*!< \brief structure containing information about array
                used when type is set to VX_TYPE_ARRAY */
    struct {
        /*!< \brief The type of the Array items */
        vx_enum item_type;
        /*!< \brief The capacity of the Array */
        vx_size capacity;
    } arr;

    /*!< \brief structure containing information about convolution
                used when type is set to VX_TYPE_CONVOLUTION */
    struct {
        /*! The M dimension of the matrix */
        vx_size rows;
        /*! The N dimension of the matrix */
        vx_size cols;
    } conv;

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

} tivx_utils_meta_format_t;

typedef struct
{
    /** Image Meta information. */
    tivx_utils_meta_format_t    meta;

    /** Size of the exported handles.
     * For TIVX_TYPE_PYRAMID type, the sizes are packed for each level. If the image type has multiple
     * planes then all the handles for a given level are packed contiguously before the handles of the next
     * level. For example if the pyramid object has N levels and each level has image with X levels then the
     * packing will look like:
     * Level 0 Plane 0
     * Level 0 Plane 1
     * ...........
     * Level 0 Plane (X-1)
     * ...........
     * Level (N-1) Plane 0
     * Level (N-1) Plane 1
     * ...........
     * Level (N-1) Plane (X-1)
     */
    uint32_t                    handleSizes[VX_IPC_MAX_VX_PLANES];

} tivx_utils_ref_desc_t;

typedef struct
{
    /** vx object reference parameters. */
    tivx_utils_ref_desc_t   refDesc;

    /** Number of valid entries in 'size' and 'fd' arrays below. */
    uint32_t                numFd;

    /** Descriptors associated with the openVX object handles.
     * For TIVX_TYPE_IMAGE, the FD corresponding to only the first plane will be packed.
     *
     * For TIVX_TYPE_PYRAMID type, the FD are packed for the first plane of each level.
     * For example if the pyramid object has N levels and each level has image with X
     * levels then the packing will look like:
     * Level 0 Plane 0
     * Level 1 Plane 0
     * Level 0 Plane (X-1)
     * ...........
     * Level (N-1) Plane (X-1)
     *
     * The receiver should use the fd[] information along with the refDesc->handleSizes[]
     * information to derive the information for subsequent levels/planes.
     */
    uint64_t                fd[VX_IPC_MAX_VX_PLANES];

} tivx_utils_ref_ipc_msg_t;

/**
 * \brief Export the internal handle information of a valid reference as a
 *        buffer descriptor along with meta information so that the information
 *        could be transferred over Linux/QNX IPC mechanism to a remote process.
 *        Only the following reference types are supported.
 *        - VX_TYPE_IMAGE
 *        - VX_TYPE_TENSOR
 *        - VX_TYPE_USER_DATA_OBJECT
 *        - VX_TYPE_ARRAY
 *        - VX_TYPE_DISTRIBUTION
 *        - VX_TYPE_MATRIX
 *        - VX_TYPE_CONVOLUTION
 *        - TIVX_TYPE_RAW_IMAGE
 *        - TIVX_TYPE_PYRAMID
 *
 * \param [in] ref  A valid openVX reference whose information will be exported
 * \param [out] ipcMsg  Exported object information
 *
 * \return VX_SUCCESS on success, else failure
 *
 */
vx_status tivx_utils_export_ref_for_ipc_xfer(const vx_reference         ref,
                                             tivx_utils_ref_ipc_msg_t  *ipcMsg);

/**
 * \brief Import the external handle information of a valid reference as a
 *        buffer descriptor along with meta information exchanged via Linux/
 *        QNX IPC mechanism. Only the following reference types are supported.
 *        - VX_TYPE_IMAGE
 *        - VX_TYPE_TENSOR
 *        - VX_TYPE_USER_DATA_OBJECT
 *        - VX_TYPE_ARRAY
 *        - VX_TYPE_DISTRIBUTION
 *        - VX_TYPE_MATRIX
 *        - VX_TYPE_CONVOLUTION
 *        - TIVX_TYPE_RAW_IMAGE
 *        - TIVX_TYPE_PYRAMID
 *
 * \param [in] context  A valid openVX context
 * \param [in,out] ipcMsg  Exported object information
 * \param [in,out] ref  A valid openVX reference to import. If *ref is NULL,
 *                 then a new object will be allocated with the imported
 *                 handles and returned. If *ref is not NULL then the object
 *                 will be used to import the handles.
 *
 * \return VX_SUCCESS on success, else failure
 *
 */
vx_status tivx_utils_import_ref_from_ipc_xfer(vx_context                context,
                                              tivx_utils_ref_ipc_msg_t *ipcMsg,
                                              vx_reference              *ref);

#ifdef __cplusplus
}
#endif

#endif /* _TIVX_UTILS_IPC_REF_XFER_H_ */

