/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KENREL_THRESHOLD_
#define _TIVX_KENREL_THRESHOLD_


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
#define TIVX_KERNEL_THRLD_IN_IMG_IDX      (0U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_THRLD_OUT_IMG_IDX     (2U)

/*!
 * \brief Index of the input Lut
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_THRLD_IN_THR_IDX      (1U)

/*!
 * \brief Max Params supported by threshold kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_THRLD_MAX_PARAMS        (3U)

#ifdef __cplusplus
}
#endif

#endif
