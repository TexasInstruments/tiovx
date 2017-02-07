/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_KENREL_WARP_AFFINE_
#define TIVX_KENREL_WARP_AFFINE_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for warp_affine kernel
 */


/*!
 * \brief Index of the input image for warp_affine
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_WARP_AFFINE_IN0_IMG_IDX          (0U)

/*!
 * \brief Index of the matrix for warp_affine
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_WARP_AFFINE_IN0_MAT_IDX          (1U)

/*!
 * \brief Index of the scalar interpolation type for warp_affine
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_WARP_AFFINE_IN0_SC_IDX          (2U)

/*!
 * \brief Index of the output image for warp_affine
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_WARP_AFFINE_OUT_IMG_IDX          (3U)

/*!
 * \brief Max parameters supported by WarpAffine kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_WARP_AFFINE_MAX_PARAMS          (4U)


#ifdef __cplusplus
}
#endif

#endif
