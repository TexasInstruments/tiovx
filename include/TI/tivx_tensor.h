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



#ifndef TIVX_TENSOR_H_
#define TIVX_TENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to Tensor APIs (modeled after OpenVX 1.2 tensor support)
 */


 /*! \brief The multidimensional data object (Tensor).
 * \see vxCreateTensor
 * \ingroup group_object_tensor
 * \extends vx_reference
 */
typedef struct _vx_tensor * vx_tensor;

/*! \brief The Object Type Enumeration for Imports.
 * \ingroup group_basic_features
 */
enum vx_ext_type_e {
    VX_TYPE_TENSOR          = 0x815,/*!< \brief A <tt>\ref vx_tensor</tt>. */
};

/*! \brief A list of context attributes.
 * \ingroup group_context
 */
enum vx_ext_context_attribute_e {
    /*! \brief tensor Data maximal number of dimensions supported by the implementation. */
    VX_CONTEXT_MAX_TENSOR_DIMS = VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_CONTEXT) + 0xE,
};

/*! \brief tensor Data attributes.
 * \ingroup group_object_tensor
 */
enum vx_tensor_attribute_e
{
    /*! \brief Number of dimensions. */
    VX_TENSOR_NUMBER_OF_DIMS = VX_ATTRIBUTE_BASE( VX_ID_KHRONOS, VX_TYPE_TENSOR ) + 0x0,
    /*! \brief Dimension sizes. */
    VX_TENSOR_DIMS        = VX_ATTRIBUTE_BASE( VX_ID_KHRONOS, VX_TYPE_TENSOR ) + 0x1,
    /*! \brief tensor Data element data type. <tt>vx_type_e</tt> */
    VX_TENSOR_DATA_TYPE   = VX_ATTRIBUTE_BASE( VX_ID_KHRONOS, VX_TYPE_TENSOR ) + 0x2,
    /*! \brief fixed point position when the input element type is integer.
     * Read-Write.
     */
    VX_TENSOR_FIXED_POINT_POSITION = VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_TENSOR) + 0x3,
    /*! \brief scaling divisor to be applied to each element when the input element type is integer.
     * Read-Write.
     */
    TIVX_TENSOR_SCALING_DIVISOR = VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_TENSOR) + 0x4,
    /*! \brief fixed point position of the scaling divisor
     * Read-Write.
     */
    TIVX_TENSOR_SCALING_DIVISOR_FIXED_POINT_POSITION = VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_TENSOR) + 0x5
};

/*! \brief Creates an opaque reference to a tensor data buffer.
 * \details Not guaranteed to exist until the <tt>\ref vx_graph</tt> containing it has been verified.
 * Since functions using tensors, need to understand the context of each dimension. We describe a layout of the dimensions in each function using tensors.
 * That layout is not mandatory. It is done specifically to explain the functions and not to mandate layout. Different implementation may have different layout.
 * Therefore the layout description is logical and not physical. It refers to the order of dimensions given in this function.
 * \param [in] context The reference to the implementation context.
 * \param [in] number_of_dims The number of dimensions.
 * \param [in] dims Dimensions sizes in elements.
 * \param [in] data_type The <tt>\ref vx_type_e</tt> that represents the data type of the tensor data elements.
 * \param [in] fixed_point_position Specifies the fixed point position when the input element type is integer. if 0, calculations are performed in integer math.
 * \return A tensor data reference. Any possible errors preventing a
 * successful creation should be checked using <tt>\ref vxGetStatus</tt>.
 * \ingroup group_object_tensor
 */
VX_API_ENTRY vx_tensor VX_API_CALL vxCreateTensor(vx_context context, vx_size number_of_dims, const vx_size * dims, vx_enum data_type,vx_int8 fixed_point_position);

/*! \brief Allows the application to copy a view patch from/into an tensor object .
 * \param [in] tensor The reference to the tensor object that is the source or the
 * destination of the copy.
 * \param [in] number_of_dims Number of patch dimension. Error return if 0 or greater than number of
 * tensor dimensions. If smaller than number of tensor dimensions, the lower dimensions are assumed.
 * \param [in] view_start Array of patch start points in each dimension
 * \param [in] view_end Array of patch end points in each dimension
 * \param [in] user_stride Array of user memory strides in each dimension
 * \param [in] user_ptr The address of the memory location where to store the requested data
 * if the copy was requested in read mode, or from where to get the data to store into the tensor
 * object if the copy was requested in write mode. The accessible memory must be large enough
 * to contain the specified patch with the specified layout:\n
 * accessible memory in bytes >= (end[last_dimension] - start[last_dimension]) * stride[last_dimension].\n
 * The layout of the user memory must follow a row major order.
 * \param [in] usage This declares the effect of the copy with regard to the tensor object
 * using the <tt>\ref vx_accessor_e</tt> enumeration. Only <tt>\ref VX_READ_ONLY</tt> and <tt>\ref VX_WRITE_ONLY</tt> are supported:
 * \arg <tt>\ref VX_READ_ONLY</tt> means that data is copied from the tensor object into the application memory
 * \arg <tt>\ref VX_WRITE_ONLY</tt> means that data is copied into the tensor object from the application memory
 * \param [in] user_memory_type A <tt>\ref vx_memory_type_e</tt> enumeration that specifies
 * the memory type of the memory referenced by the user_addr.
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_ERROR_OPTIMIZED_AWAY This is a reference to a virtual tensor that cannot be
 * accessed by the application.
 * \retval VX_ERROR_INVALID_REFERENCE The tensor reference is not actually an tensor reference.
 * \retval VX_ERROR_INVALID_PARAMETERS An other parameter is incorrect.
 * \ingroup group_object_tensor
 */
VX_API_ENTRY vx_status VX_API_CALL vxCopyTensorPatch(vx_tensor tensor, vx_size number_of_dims, const vx_size * view_start, const vx_size * view_end,
        const vx_size * user_stride, void * user_ptr, vx_enum usage, vx_enum user_memory_type);

/*! \brief Retrieves various attributes of a tensor data.
 * \param [in] tensor The reference to the tensor data to query.
 * \param [in] attribute The attribute to query. Use a <tt>\ref vx_tensor_attribute_e</tt>.
 * \param [out] ptr The location at which to store the resulting value.
 * \param [in] size The size of the container to which \a ptr points.
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 * \retval VX_ERROR_INVALID_REFERENCE If data is not a <tt>\ref vx_tensor</tt>.
 * \retval VX_ERROR_INVALID_PARAMETERS If any of the other parameters are incorrect.
 * \ingroup group_object_tensor
 */
VX_API_ENTRY vx_status VX_API_CALL vxQueryTensor(vx_tensor tensor, vx_enum attribute, void *ptr, vx_size size);

/*! \brief Sets attributes of a tensor object.
 * \param [in] tensor The tensor object to set.
 * \param [in] attribute The attribute to modify. Use a <tt>\ref vx_tensor_attribute_e</tt> enumeration.
 * \param [in] ptr The pointer to the value to which to set the attribute.
 * \param [in] size The size of the data pointed to by \a ptr.
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 * \retval VX_ERROR_INVALID_REFERENCE If data is not a <tt>\ref vx_tensor</tt>.
 * \retval VX_ERROR_INVALID_PARAMETERS If any of the other parameters are incorrect.
 * \ingroup group_object_tensor
 */
VX_API_ENTRY vx_status VX_API_CALL vxSetTensorAttribute(
    vx_tensor tensor, vx_enum attribute, const void *ptr, vx_size size);

/*! \brief Releases a reference to a tensor data object.
 * The object may not be garbage collected until its total reference count is zero.
 * \param [in] tensor The pointer to the tensor data to release.
 * \post After returning from this function the reference is zeroed.
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors; all other values indicate failure
 * \retval * An error occurred. See <tt>\ref vx_status_e</tt>.
 * \ingroup group_object_tensor
 */
VX_API_ENTRY vx_status VX_API_CALL vxReleaseTensor(vx_tensor *tensor);


/*! \brief Allows the application to get direct access to a patch of tensor object.
 * \param [in] tensor The reference to the tensor object that is the source or the
 * destination for direct access.
 * \param [in] number_of_dims The number of dimensions. Must be same as tensor number_of_dims.
 * \param [in] view_start Array of patch start points in each dimension. This is optional parameter and will be zero when NULL.
 * \param [in] view_end Array of patch end points in each dimension. This is optional parameter and will be dims[] of tensor when NULL.
 * \param [out] map_id The address of a vx_map_id variable where the function returns a map identifier.
 * \arg (*map_id) must eventually be provided as the map_id parameter of a call to <tt>\ref tivxUnmapTensorPatch</tt>.
 * \param [out] stride An array of stride in all dimensions in bytes. The stride value at index 0 must be size of the tensor data element type.
 * \param [out] ptr The address of a pointer that the function sets to the
 * address where the requested data can be accessed. The returned (*ptr) address
 * is only valid between the call to the function and the corresponding call to
 * <tt>\ref tivxUnmapTensorPatch</tt>.
 * \param [in] usage This declares the access mode for the tensor patch, using
 * the <tt>\ref vx_accessor_e</tt> enumeration.
 * \arg VX_READ_ONLY: after the function call, the content of the memory location
 * pointed by (*ptr) contains the tensor patch data. Writing into this memory location
 * is forbidden and its behavior is undefined.
 * \arg VX_READ_AND_WRITE : after the function call, the content of the memory
 * location pointed by (*ptr) contains the tensor patch data; writing into this memory
 * is allowed only for the location of items and will result in a modification of the
 * affected items in the tensor object once the range is unmapped. Writing into
 * a gap between items (when (*stride) > item size in bytes) is forbidden and its
 * behavior is undefined.
 * \arg VX_WRITE_ONLY: after the function call, the memory location pointed by (*ptr)
 * contains undefined data; writing each item of the range is required prior to
 * unmapping. Items not written by the application before unmap will become
 * undefined after unmap, even if they were well defined before map. Like for
 * VX_READ_AND_WRITE, writing into a gap between items is forbidden and its behavior
 * is undefined.
 * \param [in] mem_type A <tt>\ref vx_memory_type_e</tt> enumeration that
 * specifies the type of the memory where the tensor patch is requested to be mapped.
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_ERROR_OPTIMIZED_AWAY This is a reference to a virtual tensor that cannot be accessed by the application.
 * \retval VX_ERROR_INVALID_REFERENCE The tensor reference is not actually an tensor reference.
 * \retval VX_ERROR_INVALID_PARAMETERS An other parameter is incorrect.
 * \retval VX_ERROR_NO_MEMORY Internal memory allocation failed.
 * \ingroup group_tensor
 * \post <tt>\ref tivxUnmapTensorPatch </tt> with same (*map_id) value.
 */
VX_API_ENTRY vx_status VX_API_CALL tivxMapTensorPatch(
    vx_tensor tensor,
    vx_size number_of_dims,
    const vx_size * view_start,
    const vx_size * view_end,
    vx_map_id * map_id,
    vx_size * stride,
    void ** ptr,
    vx_enum usage,
    vx_enum mem_type);


/*! \brief Unmap and commit potential changes to a tensor object patch that were previously mapped.
* Unmapping a tensor patch invalidates the memory location from which the patch could
* be accessed by the application. Accessing this memory location after the unmap function
* completes has an undefined behavior.
* \param [in] tensor The reference to the tensor object to unmap.
* \param [out] map_id The unique map identifier that was returned by <tt>\ref tivxMapTensorPatch</tt> .
* \return A <tt>\ref vx_status_e</tt> enumeration.
* \retval VX_ERROR_INVALID_REFERENCE The tensor reference is not actually a tensor reference.
* \retval VX_ERROR_INVALID_PARAMETERS An other parameter is incorrect.
* \ingroup group_object_tensor
* \pre <tt>\ref tivxMapTensorPatch</tt> with same map_id value
*/
VX_API_ENTRY vx_status VX_API_CALL tivxUnmapTensorPatch(vx_tensor tensor, vx_map_id map_id);


#ifdef __cplusplus
}
#endif

#endif
