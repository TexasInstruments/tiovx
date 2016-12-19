/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KERNEL_MEANSTDDEV_
#define _TIVX_KERNEL_MEANSTDDEV_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for meanstddev kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MSD_IN_IMG_IDX          (0U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MSD_OUT_MEAN_IDX        (1U)

/*!
 * \brief Index of the input Lut
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MSD_OUT_STDDEV_IDX      (2U)

/*!
 * \brief Max Params supported by meanstddev kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MSD_MAX_PARAMS          (3U)

#ifdef __cplusplus
}
#endif

#endif
