/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _VX_EXT_TI_H_
#define _VX_EXT_TI_H_

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to TI extension APIs
 */


/*! \brief Name for TI OpenVX kernel module
 * \ingroup group_tivx_ext
 */
#define TIVX_MODULE_NAME    "openvx-core"

/*! \brief Name for DSP target class, instance 1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_DSP1        "DSP-1"

/*! \brief Name for DSP target class, instance 2
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_DSP2        "DSP-2"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_EVE1        "EVE-1"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_EVE2        "EVE-2"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_EVE3        "EVE-3"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_EVE4        "EVE-4"

/*! \brief Name for A15 target class, core 0
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_A15_0       "A15-0"

/*! \brief Name for IPU1 target class, core 0
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_IPU1_0      "IPU1-0"

/*! \brief Name for IPU1 target class, core 1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_IPU1_1      "IPU1-1"

/*! \brief Name for IPU2 target class
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_IPU2        "IPU2"

/*! \brief String to name a OpenVX Host
 *
 *         Host is not a unique target on its own.
 *         At system config "HOST" will map to one
 *         of available targets
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_HOST        "HOST"

/*!
 * \brief Function to initialize host
 *
 * \ingroup group_tivx_ext
 */
void tivxInit(void);

/*!
 * \brief Function to de-initialize host
 *
 * \ingroup group_tivx_ext
 */
void tivxDeInit(void);

/*!
 * \brief Associate a target with a kernel
 *
 *        Call multiple times for each supported target
 *
 *        If given target is not valid on current platform then
 *        error VX_ERROR_NOT_SUPPORTED is returned.
 *
 *        Typically VX_ERROR_NOT_SUPPORTED error should be ignored for this API,
 *        since this code is typically kept same across platforms
 *
 *        During graph verify however if user asks to run the kernel
 *        on a target not supported by this platform it results in a
 *        error and graph cannot execute.
 *
 *
 * \ingroup group_tivx_ext
 */
VX_API_ENTRY vx_status VX_API_CALL tivxAddKernelTarget(vx_kernel kernel, char *target_name);


/*!
 * \brief Register publish and unpublish functions against a module name.
 *
 *        These functions are invoked when vxLoadKernels is called with
 *        the registered name.
 *
 *        This is alternative instead of dynamically loading kernels during vxLoadKernels
 *
 *        Duplicate module names not checked by this API.
 *
 *        API is not reentrant, user is recommended to call all tivxRegisterModule
 *        during system init before vxCreateContext() from a single thread.
 *
 *        Modules registered against TIVX_MODULE_NAME are called during vxCreateContext
 *        so user MUST ensure tivxRegisterModule() is called for TIVX_MODULE_NAME module
 *
 * \ingroup group_tivx_ext
 */
VX_API_ENTRY vx_status VX_API_CALL tivxRegisterModule(char *name, vx_publish_kernels_f publish, vx_unpublish_kernels_f unpublish);

/*!
 * \brief UnRegister publish and unpublish functions if previously registered
 *
 * \ingroup group_tivx_ext
 */
VX_API_ENTRY vx_status VX_API_CALL tivxUnRegisterModule(char *name);

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
