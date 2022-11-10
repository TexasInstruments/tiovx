/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
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




#ifndef TIVX_OBJECTS_H_
#define TIVX_OBJECTS_H_

#include <vx_internal.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Structure to hold all framework objects
 *
 * \ingroup group_tivx_obj
 */
typedef struct _tivx_object_t
{
    tivx_meta_format_t      meta_format[TIVX_META_FORMAT_MAX_OBJECTS];
    /**< Meta Format Objects */
    vx_bool                 isMfUse[TIVX_META_FORMAT_MAX_OBJECTS];
    /**< Flag indicating if meta format is in use or not */

    tivx_context_t          context[TIVX_CONTEXT_MAX_OBJECTS];
    /**< Context Objects */
    vx_bool                 isContextUse[TIVX_CONTEXT_MAX_OBJECTS];
    /**< Flag indicating if context object is in use or not */

    tivx_graph_t            graph[TIVX_GRAPH_MAX_OBJECTS];
    /**< Graph Objects */
    vx_bool                 isGraphUse[TIVX_GRAPH_MAX_OBJECTS];
    /**< Flag indicating if graph object is in use or not */

    tivx_node_t             node[TIVX_NODE_MAX_OBJECTS];
    /**< Node Objects */
    vx_bool                 isNodeUse[TIVX_NODE_MAX_OBJECTS];
    /**< Flag indicating if node object is in use or not */

    tivx_kernel_t           kernel[TIVX_KERNEL_MAX_OBJECTS];
    /**< Kernel Objects */
    vx_bool                 isKernelUse[TIVX_KERNEL_MAX_OBJECTS];
    /**< Flag indicating if kernel object is in use or not */

    tivx_array_t            array[TIVX_ARRAY_MAX_OBJECTS];
    /**< Array Objects */
    vx_bool                 isArrayUse[TIVX_ARRAY_MAX_OBJECTS];
    /**< Flag indicating if Array object is in use or not */

    tivx_user_data_object_t user_data_object[TIVX_USER_DATA_OBJECT_MAX_OBJECTS];
    /**< User Data Objects */
    vx_bool                 isUserDataObjectUse[TIVX_USER_DATA_OBJECT_MAX_OBJECTS];
    /**< Flag indicating if User data object is in use or not */

    tivx_raw_image_t raw_image[TIVX_RAW_IMAGE_MAX_OBJECTS];
    /**< Raw Image Objects */
    vx_bool                 isRawImageUse[TIVX_RAW_IMAGE_MAX_OBJECTS];
    /**< Flag indicating if raw image object is in use or not */

    tivx_super_node_t super_node[TIVX_SUPER_NODE_MAX_OBJECTS];
    /**< Super Node Objects */
    vx_bool                 isSuperNodeUse[TIVX_SUPER_NODE_MAX_OBJECTS];
    /**< Flag indicating if super node object is in use or not */

    tivx_convolution_t      convolution[TIVX_CONVOLUTION_MAX_OBJECTS];
    /**< Convolution Objects */
    vx_bool                 isConvolutionUse[TIVX_CONVOLUTION_MAX_OBJECTS];
    /**< Flag indicating if convolution object is in use or not */

    tivx_delay_t            delay[TIVX_DELAY_MAX_OBJECTS];
    /**< Delay Objects */
    vx_bool                 isDelayUse[TIVX_DELAY_MAX_OBJECTS];
    /**< Flag indicating if delay object is in use or not */

    tivx_distribution_t     distribution[TIVX_DISTRIBUTION_MAX_OBJECTS];
    /**< Distribution Objects */
    vx_bool                 isDistributionUse[TIVX_DISTRIBUTION_MAX_OBJECTS];
    /**< Flag indicating if distribution object is in use or not */

    tivx_image_t            image[TIVX_IMAGE_MAX_OBJECTS];
    /**< Image Objects */
    vx_bool                 isImageUse[TIVX_IMAGE_MAX_OBJECTS];
    /**< Flag indicating if image object is in use or not */

    tivx_lut_t              lut[TIVX_LUT_MAX_OBJECTS];
    /**< Lut Objects */
    vx_bool                 isLutUse[TIVX_LUT_MAX_OBJECTS];
    /**< Flag indicating if lut object is in use or not */

    tivx_matrix_t           matrix[TIVX_MATRIX_MAX_OBJECTS];
    /**< Matrix Objects */
    vx_bool                 isMatrixUse[TIVX_MATRIX_MAX_OBJECTS];
    /**< Flag indicating if matrix object is in use or not */

    tivx_pyramid_t          pyramid[TIVX_PYRAMID_MAX_OBJECTS];
    /**< Pyramid Objects */
    vx_bool                 isPyramidUse[TIVX_PYRAMID_MAX_OBJECTS];
    /**< Flag indicating if pyramid object is in use or not */

    tivx_remap_t            remap[TIVX_REMAP_MAX_OBJECTS];
    /**< Remap Objects */
    vx_bool                 isRemapUse[TIVX_REMAP_MAX_OBJECTS];
    /**< Flag indicating if remap object is in use or not */

    tivx_scalar_t           scalar[TIVX_SCALAR_MAX_OBJECTS];
    /**< Scalar Objects */
    vx_bool                 isScalarUse[TIVX_SCALAR_MAX_OBJECTS];
    /**< Flag indicating if scalar object is in use or not */

    tivx_threshold_t        threshold[TIVX_THRESHOLD_MAX_OBJECTS];
    /**< Threshold Objects */
    vx_bool                 isThresholdUse[TIVX_THRESHOLD_MAX_OBJECTS];
    /**< Flag indicating if threshold object is in use or not */

    tivx_error_t            error[TIVX_ERROR_MAX_OBJECTS];
    /**< Error Objects */
    vx_bool                 isErrorUse[TIVX_ERROR_MAX_OBJECTS];
    /**< Flag indicating if error object is in use or not */

    tivx_objarray_t         obj_array[TIVX_OBJ_ARRAY_MAX_OBJECTS];
    /**< Object arrays*/
    vx_bool                 isObjArrUse[TIVX_OBJ_ARRAY_MAX_OBJECTS];
    /**< Flag indicating if object array is in use or not */

    tivx_tensor_t           tensor[TIVX_TENSOR_MAX_OBJECTS];
    /**< Tensor Objects */
    vx_bool                 isTensorUse[TIVX_TENSOR_MAX_OBJECTS];
    /**< Flag indicating if Tensor object is in use or not */

    tivx_parameter_t        parameter[TIVX_PARAMETER_MAX_OBJECTS];
    /**< Parameter objects */
    vx_bool                 isParameterUse[TIVX_PARAMETER_MAX_OBJECTS];
    /**< Flag indicating if Parameter object is use or not */
    tivx_data_ref_queue_t   data_ref_q[TIVX_DATA_REF_Q_MAX_OBJECTS];
    /**< Data ref queue objects */
    vx_bool                 isDataRefQUse[TIVX_DATA_REF_Q_MAX_OBJECTS];
    /**< Flag indicating if Data ref queue object is use or not */
} tivx_object_t;

/*! \brief Alloc memory for a reference of specified type
 * \param [in] reftype The reference type. See \ref tivx_reftype_e
 * \return ref The reference.
 * \ingroup group_tivx_obj
 */
vx_reference ownObjectAlloc(vx_enum reftype);

/*! \brief Free memory for a reference
 * \param [in] ref The reference.
 * \return VX_SUCCESS on success
 * \ingroup group_tivx_obj
 */
vx_status ownObjectFree(vx_reference ref);

/*! \brief Initialize object module
 * \ingroup group_tivx_obj
 */
vx_status ownObjectInit(void);

/*! \brief De-Initialize object module
 * \ingroup group_tivx_obj
 */
vx_status ownObjectDeInit(void);

#ifdef __cplusplus
}
#endif

#endif
