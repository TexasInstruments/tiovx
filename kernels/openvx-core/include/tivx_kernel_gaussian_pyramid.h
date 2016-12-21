/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KERNEL_GAUSSIAN_PYRAMID_
#define _TIVX_KERNEL_GAUSSIAN_PYRAMID_


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
#define TIVX_KERNEL_G_PYD_IN_IMG_IDX            (0U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_G_PYD_OUT_PYT_IDX           (1U)

/*!
 * \brief Max Params supported by gaussian pyramid kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_G_PYD_MAX_PARAMS            (2U)

#ifdef __cplusplus
}
#endif

#endif
