/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KENREL_HISTOGRAM_
#define _TIVX_KENREL_HISTOGRAM_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for histogram kernel
 */


/*!
 * \brief Index of the input image for histogram operation
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_HISTOGRAM_IN_IMG_IDX          (0U)

/*!
 * \brief Index of the output image for histogram operation
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_HISTOGRAM_OUT_IMG_IDX         (1U)

/*!
 * \brief Max Parameter for histogram operation
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_HISTOGRAM_MAX_PARAMS           (2U)

#ifdef __cplusplus
}
#endif

#endif
