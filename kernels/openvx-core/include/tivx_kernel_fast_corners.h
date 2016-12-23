/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_KERNEL_FAST_CORNER_
#define _TIVX_KERNEL_FAST_CORNER_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for fast corners kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_FASTC_IN_IMG_IDX            (0U)

/*!
 * \brief Index of the scalar strenth threshold
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_FASTC_IN_SC_THR_IDX         (1U)

/*!
 * \brief Index of the NMS bool
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_FASTC_IN_NMS_IDX            (2U)

/*!
 * \brief Index of the output array corners
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_FASTC_OUT_ARR_IDX           (3U)

/*!
 * \brief Index of the NMS bool
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_FASTC_OUT_SC_CNT_IDX        (4U)

/*!
 * \brief Max Params supported by fast corners kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_FASTC_MAX_PARAMS            (5U)

/*!
 * \brief Minimu width/height supported by Fast corner
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_FASTC_MIN_SIZE              (7U)

#ifdef __cplusplus
}
#endif

#endif
