/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_KENREL_SOBEL_
#define TIVX_KENREL_SOBEL_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for sobel kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_SOBEL_IN_IMG_IDX        (0U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_SOBEL_OUT0_IMG_IDX      (1U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_SOBEL_OUT1_IMG_IDX      (2U)

/*!
 * \brief Max Params supported by sobel kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_SOBEL_MAX_PARAMS        (3U)

#ifdef __cplusplus
}
#endif

#endif
