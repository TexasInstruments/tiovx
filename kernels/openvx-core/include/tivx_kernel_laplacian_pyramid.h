/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_KERNEL_LAPLACIAN_PYRAMID_
#define TIVX_KERNEL_LAPLACIAN_PYRAMID_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for gaussian pyramid kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_LPL_PMD_IN_IMG_IDX            (0U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_LPL_PMD_OUT_PMD_IDX           (1U)

/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_LPL_PMD_OUT_IMG_IDX            (2U)

/*!
 * \brief Max Params supported by gaussian pyramid kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_LPL_PMD_MAX_PARAMS            (3U)

#ifdef __cplusplus
}
#endif

#endif
