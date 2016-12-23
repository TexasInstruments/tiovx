/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KERNEL_HARRISC_CORNERS_
#define _TIVX_KERNEL_HARRISC_CORNERS_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for fast corners kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_HARRISC_IN_IMG_IDX                   (0U)

/*!
 * \brief Index of the scalar strenth threshold
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_HARRISC_IN_SC_THR_IDX                (1U)

/*!
 * \brief Index of the radial Euclidean distance for non-maximum suppression
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_HARRISC_IN_SC_DIST_IDX               (2U)

/*!
 * \brief Index of the scalar sensitivity threshold k from the
 *        Harris Stephens equation
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_HARRISC_IN_SC_SENS_IDX               (3U)

/*!
 * \brief Index of The gradient window size to use on the input
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_HARRISC_IN_SC_GS_IDX                 (4U)

/*!
 * \brief Index of The block window size used to compute the Harris Corner score
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_HARRISC_IN_SC_BS_IDX                 (5U)

/*!
 * \brief Index of the output array corners
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_HARRISC_OUT_ARR_IDX                  (6U)

/*!
 * \brief Index of the scalar containing array size
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_HARRISC_OUT_SC_CNT_IDX               (7U)

/*!
 * \brief Max Params supported by fast corners kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_HARRISC_MAX_PARAMS            (8U)

/*!
 * \brief Minimu width/height supported by Fast corner
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_HARRISC_MIN_SIZE              (7U)

#ifdef __cplusplus
}
#endif

#endif
