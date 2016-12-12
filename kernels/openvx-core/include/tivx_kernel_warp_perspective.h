/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KENREL_WARP_PERSPECTIVE_
#define _TIVX_KENREL_WARP_PERSPECTIVE_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for warp_perspective kernel
 */


/*!
 * \brief Index of the input image for warp_perspective
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_WARP_PERSPECTIVE_IN0_IMG_IDX          (0U)

/*!
 * \brief Index of the matrix for warp_perspective
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_WARP_PERSPECTIVE_IN0_MAT_IDX          (1U)

/*!
 * \brief Index of the scalar interpolation type for warp_perspective
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_WARP_PERSPECTIVE_IN0_SC_IDX          (2U)

/*!
 * \brief Index of the output image for warp_perspective
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_WARP_PERSPECTIVE_OUT_IMG_IDX          (3U)

/*!
 * \brief Max parameters supported by WarpPerspective kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_WARP_PERSPECTIVE_MAX_PARAMS          (4U)


#ifdef __cplusplus
}
#endif

#endif
