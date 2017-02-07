/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_KENREL_LUT_
#define TIVX_KENREL_LUT_


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
#define TIVX_KERNEL_LUT_IN_IMG_IDX      (0U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_LUT_OUT_IMG_IDX     (2U)

/*!
 * \brief Index of the input Lut
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_LUT_IN_LUT_IDX      (1U)

/*!
 * \brief Max parameters supported by Lut kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_LUT_MAX_PARAMS      (3U)

#ifdef __cplusplus
}
#endif

#endif
