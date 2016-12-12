/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KENREL_CONVOLVE_
#define _TIVX_KENREL_CONVOLVE_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for convolution kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CONVOLVE_IN_IMG_IDX                     (0U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CONVOLVE_OUT_IMG_IDX                    (2U)

/*!
 * \brief Index of the input convolution matrix
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CONVOLVE_IN_CONVOLVE_IDX                (1U)

/*!
 * \brief Max Params supported by threshold kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CONVOLVE_MAX_PARAMS                     (3U)


#define TIVX_KERNEL_CONVOLVE_DIM_H                          (9U)

#define TIVX_KERNEL_CONVOLVE_DIM_V                          (9U)

#ifdef __cplusplus
}
#endif

#endif
