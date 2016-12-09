/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KENREL_LUT_
#define _TIVX_KENREL_LUT_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for bitwise kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_BITWISE_IN0_IMG_IDX          (0U)

/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_BITWISE_IN1_IMG_IDX          (1U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_BITWISE_OUT_IMG_IDX          (2U)

/*!
 * \brief Max Parameter for bitwise operation
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_BITWISE_MAX_PARAMS           (3U)

/*!
 * \brief Index of the input image for Not operation
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_BITWISE_NOT_IN_IMG_IDX          (0U)

/*!
 * \brief Index of the output image for Not operation
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_BITWISE_NOT_OUT_IMG_IDX         (1U)

/*!
 * \brief Max Parameter for bitwise Not operation
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_BITWISE_NOT_MAX_PARAMS           (2U)

#ifdef __cplusplus
}
#endif

#endif
