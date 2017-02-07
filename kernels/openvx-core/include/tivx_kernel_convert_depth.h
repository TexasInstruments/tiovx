/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_KENREL_CONVERT_DEPTH_
#define TIVX_KENREL_CONVERT_DEPTH_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for convert_depth kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CONVERT_DEPTH_IN_IMG_IDX      (0U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CONVERT_DEPTH_OUT_IMG_IDX      (1U)

/*!
 * \brief Index of scalar input
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CONVERT_DEPTH_IN0_SCALAR_IDX    (2U)

/*!
 * \brief Index of scalar input
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CONVERT_DEPTH_IN1_SCALAR_IDX    (3U)

/*!
 * \brief Max parameters supported by the kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_CONVERT_DEPTH_MAX_PARAMS       (4U)

#ifdef __cplusplus
}
#endif

#endif
