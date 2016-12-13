/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KENREL_ACCUMULATE_
#define _TIVX_KENREL_ACCUMULATE_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for accumulate kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_ACCUMULATE_IN_IMG_IDX      (0U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_ACCUMULATE_OUT_IMG_IDX      (1U)

/*!
 * \brief Max parameters supported by the kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_ACCUMULATE_MAX_PARAMS       (2U)

#ifdef __cplusplus
}
#endif

#endif
