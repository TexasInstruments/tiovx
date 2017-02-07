/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_KERNEL_PHASE_
#define TIVX_KERNEL_PHASE_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for phase kernel
 */


/*!
 * \brief Index of the input image, ie grad_x
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_PHASE_IN0_IMG_IDX       (0U)

/*!
 * \brief Index of the input image, ie grad_y
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_PHASE_IN1_IMG_IDX       (1U)

/*!
 * \brief Index of the output image, ie orientation
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_PHASE_OUT_IMG_IDX       (2U)

/*!
 * \brief Max Params supported by phase kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_PHASE_MAX_PARAMS        (3U)

#ifdef __cplusplus
}
#endif

#endif
