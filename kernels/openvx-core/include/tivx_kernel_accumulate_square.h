/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KENREL_ACCUMULATE_SQUARE_
#define _TIVX_KENREL_ACCUMULATE_SQUARE_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for accumulate_square kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_ACCUMULATE_SQUARE_IN_IMG_IDX      (0U)

/*!
 * \brief Index of scalar input
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_ACCUMULATE_SQUARE_IN_SCALAR_IDX    (1U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_ACCUMULATE_SQUARE_OUT_IMG_IDX      (2U)

/*!
 * \brief Max parameters supported by the kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_ACCUMULATE_SQUARE_MAX_PARAMS       (3U)

#ifdef __cplusplus
}
#endif

#endif
