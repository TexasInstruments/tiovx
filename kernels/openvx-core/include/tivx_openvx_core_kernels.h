/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef VX_CORE_KERNEL_H_
#define VX_CORE_KERNEL_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Core Interface file the OpenVX kernels
 */


/*!
 * \brief Function to register core OpenVx Kernels on the Host
 *
 * \ingroup group_tivx_ext
 */
void tivxRegisterOpenVXCoreKernels(void);

/*!
 * \brief Function to un-register core OpenVx Kernels on the Host
 *
 * \ingroup group_tivx_ext
 */
void tivxUnRegisterOpenVXCoreKernels(void);

/*!
 * \brief Function to register OpenVx Kernels on the Target
 *
 * \ingroup group_tivx_ext
 */
void tivxRegisterOpenVXCoreTargetKernels(void);

/*!
 * \brief Function to register OpenVx Kernels on the Target
 *
 * \ingroup group_tivx_ext
 */
void tivxUnRegisterOpenVXCoreTargetKernels(void);


#ifdef __cplusplus
}
#endif

#endif
