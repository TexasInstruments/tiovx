/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_KENREL_COLOR_CONVERT_
#define TIVX_KENREL_COLOR_CONVERT_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for color_convert kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_COLOR_CONVERT_IN_IMG_IDX      (0U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_COLOR_CONVERT_OUT_IMG_IDX      (1U)

/*!
 * \brief Max parameters supported by the kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS       (2U)

#ifdef __cplusplus
}
#endif

#endif
