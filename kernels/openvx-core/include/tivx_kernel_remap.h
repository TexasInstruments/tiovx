/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_KENREL_REMAP_
#define TIVX_KENREL_REMAP_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for remap kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_REMAP_IN_IMG_IDX            (0U)

/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_REMAP_IN_TBL_IDX            (1U)

/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_REMAP_IN_POLICY_IDX         (2U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_REMAP_OUT_IMG_IDX           (3U)

/*!
 * \brief Max Params supported by remap kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_REMAP_MAX_PARAMS            (4U)

#ifdef __cplusplus
}
#endif

#endif
