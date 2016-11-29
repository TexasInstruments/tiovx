/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KENREL_MULTIPLY_
#define _TIVX_KENREL_MULTIPLY_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for multiply kernel
 */


/*!
 * \brief Index of the input image for multiplication
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MULT_IN0_IMG_IDX          (0U)

/*!
 * \brief Index of the input image for multiplication
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MULT_IN1_IMG_IDX          (1U)

/*!
 * \brief Index of the scalar input for multiplication
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MULT_IN0_SC_IDX          (2U)

/*!
 * \brief Index of the scalar input for overflow policy
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MULT_IN1_SC_IDX          (3U)

/*!
 * \brief Index of the scalar input for rounding policy
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MULT_IN2_SC_IDX          (4U)

/*!
 * \brief Index of the output image for multiplication
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MULT_OUT_IMG_IDX          (5U)

/*!
 * \brief Max parameters supported by Multiply kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MULT_MAX_PARAMS          (6U)


#ifdef __cplusplus
}
#endif

#endif
