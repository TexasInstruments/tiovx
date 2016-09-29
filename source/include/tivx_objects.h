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


#ifndef _TIVX_OBJECTS_H_
#define _TIVX_OBJECTS_H_

#include <vx_internal.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Internal header file defining max number of supported for each object
 */

/*!
 * \brief Max number meta format objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_META_FORMAT_MAX_OBJECTS        (32u)

/*!
 * \brief Max number context objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_CONTEXT_MAX_OBJECTS                    (1u)

/*!
 * \brief Max number graph objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_GRAPH_MAX_OBJECTS                      (8u)

/*!
 * \brief Max number node objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_NODE_MAX_OBJECTS                       (32u)

/*!
 * \brief Max number kernel objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_KERNEL_MAX_OBJECTS                       (32u)

/*!
 * \brief Max number array objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_ARRAY_MAX_OBJECTS                      (16u)

/*!
 * \brief Max number convolution objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_CONVOLUTION_MAX_OBJECTS                (8u)

/*!
 * \brief Max number delay objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_DELAY_MAX_OBJECTS                      (8u)

/*!
 * \brief Max number distribution objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_DISTRIBUTION_MAX_OBJECTS               (8u)

/*!
 * \brief Max number image objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_IMAGE_MAX_OBJECTS                      (32u)

/*!
 * \brief Max number lut objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_LUT_MAX_OBJECTS                        (8u)

/*!
 * \brief Max number matrix objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_MATRIX_MAX_OBJECTS                     (8u)

/*!
 * \brief Max number pyramid objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_PYRAMID_MAX_OBJECTS                    (8u)

/*!
 * \brief Max number remap objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_REMAP_MAX_OBJECTS                      (8u)

/*!
 * \brief Max number scalar objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_SCALAR_MAX_OBJECTS                     (32u)

/*!
 * \brief Max number threshold objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_THRESHOLD_MAX_OBJECTS                  (8u)



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

} tivx_object_t;

/*! \brief Alloc memory for a reference of specified type
 * \param [in] reftype The reference type. See \ref tivx_reftype_e
 * \return ref The reference.
 * \ingroup group_tivx_obj
 */
vx_reference tivxObjectAlloc(vx_enum reftype);

/*! \brief Free memory for a reference
 * \param [in] ref The reference.
 * \return VX_SUCCESS on success
 * \ingroup group_tivx_obj
 */
vx_status tivxObjectFree(vx_reference ref);

/*! \brief Initialize object module
 * \ingroup group_tivx_obj
 */
vx_status tivxObjectInit(void);

/*! \brief De-Initialize object module
 * \ingroup group_tivx_obj
 */
vx_status tivxObjectDeInit(void);

#ifdef __cplusplus
}
#endif

#endif
