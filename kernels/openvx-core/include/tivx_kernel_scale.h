/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KENREL_SCALE_
#define _TIVX_KENREL_SCALE_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for image scaling kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_SCALE_IN_IMG_IDX      (0U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_SCALE_OUT_IMG_IDX     (1U)

/*!
 * \brief Index of the input interpolation type
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_SCALE_IN_TYPE_IDX      (2U)

/*!
 * \brief Max Params supported by threshold kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_SCALE_MAX_PARAMS        (3U)

#ifdef __cplusplus
}
#endif

#endif
