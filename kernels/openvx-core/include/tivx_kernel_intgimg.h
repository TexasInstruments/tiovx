/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_KENREL_INTGIMG_
#define TIVX_KENREL_INTGIMG_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for lut kernel
 */


/*!
 * \brief Index of the input image for integral Image operation
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_INTGIMG_IN_IMG_IDX          (0U)

/*!
 * \brief Index of the output image for integral Image operation
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_INTGIMG_OUT_IMG_IDX         (1U)

/*!
 * \brief Max Parameter for bitwise integral Image operation
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_INTGIMG_MAX_PARAMS           (2U)

#ifdef __cplusplus
}
#endif

#endif
