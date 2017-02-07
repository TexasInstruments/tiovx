/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_KERNEL_MINMAXLOC_
#define TIVX_KERNEL_MINMAXLOC_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for mml kernel
 */


/*!
 * \brief Index of the input image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MML_IN_IMG_IDX                  (0U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MML_OUT_MIN_SC_IDX              (1U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MML_OUT_MAX_SC_IDX              (2U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MML_OUT_MIN_ARR_IDX             (3U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MML_OUT_MAX_ARR_IDX             (4U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MML_OUT_MIN_SC_C_IDX            (5U)

/*!
 * \brief Index of the output image
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MML_OUT_MAX_SC_C_IDX            (6U)

/*!
 * \brief Max Params supported by mml kernel
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_KERNEL_MML_MAX_PARAMS                  (7U)

#ifdef __cplusplus
}
#endif

#endif
