/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KENREL_FILTER_3X3_
#define _TIVX_KENREL_FILTER_3X3_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for Erode3x3 kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_FILT3x3_IN_IMG_IDX        (0U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_FILT3x3_OUT_IMG_IDX       (1U)

/*!
 * \brief Max number of params
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_FILT3x3_MAX_PARAMS        (2U)

#ifdef __cplusplus
}
#endif

#endif
