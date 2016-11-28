/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KENREL_ADDSUB_
#define _TIVX_KENREL_ADDSUB_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for lut kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_ADDSUB_IN0_IMG_IDX      (0U)

/*!
 * \brief Index of the input Lut
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_ADDSUB_IN1_IMG_IDX      (1U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_ADDSUB_OUT_IMG_IDX      (3U)

/*!
 * \brief Index of scalar input
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_ADDSUB_IN_SCALAR_IDX    (2U)

/*!
 * \brief Max parameters supported by the kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_ADDSUB_MAX_PARAMS       (4U)

#ifdef __cplusplus
}
#endif

#endif
