/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KERNEL_CNED_
#define _TIVX_KERNEL_CNED_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for canny edge detect kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CNED_IN_IMG_IDX                   (0U)

/*!
 * \brief Index of the strenth threshold
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CNED_IN_THR_IDX                (1U)

/*!
 * \brief Index of The gradient window size to use on the input
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CNED_IN_SC_GS_IDX                 (2U)

/*!
 * \brief Index of The block window size used to compute the Harris Corner score
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CNED_IN_SC_NORM_IDX                 (3U)

/*!
 * \brief Index of the output array corners
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CNED_OUT_IMG_IDX                  (4U)

/*!
 * \brief Max Params supported by canny edge detect kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CNED_MAX_PARAMS            (5U)

/*!
 * \brief Minimu width/height supported by Fast corner
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CNED_MIN_SIZE              (7U)

#ifdef __cplusplus
}
#endif

#endif
