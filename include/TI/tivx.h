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

/*! \brief Name for DSP target class
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_DSP         "DSP"

/*! \brief Name for DSP target class, instance 1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_DSP1        "DSP-1"

/*! \brief Name for DSP target class, instance 2
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_DSP2        "DSP-2"

/*! \brief Name for EVE target class
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_EVE         "EVE"

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

/*! \brief Name for A15 target class
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_A15         "A15"

/*! \brief Name for A15 target class, core 0
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_A15_0       "A15-0"

/*! \brief Name for IPU1 target class
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_IPU1        "IPU1"

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
 * \brief Associate a target with a kernel
 *
 *        Call multiple time for each support target or target class
 *
 *        Valid targets, duplicate targets not check by this APIs
 *        Valid targets checked during graph verify
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


#ifdef __cplusplus
}
#endif

#endif
