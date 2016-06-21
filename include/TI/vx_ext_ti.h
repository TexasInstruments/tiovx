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

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to TI extension APIs
 */


/*!
 * \brief Associate a target with a kernel
 *
 *        Call multiple time for each support target or target class
 *
 *        Valid targets, duplicate targets not check by this APIs
 *        Valid targets checked during graph verify
 *
 * \ingroup group_vx_ext_ti
 */
VX_API_ENTRY vx_status VX_API_CALL vxAddKernelTarget(vx_kernel kernel, char *target_name);


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
 *        API is not reentrant, user is recommended to call all vxRegisterModule
 *        during system init before vxCreateContext() from a single thread.
 *
 *        Modules registered against "openvx-core" are called during vxCreateContext
 *        so user MUST ensure vxRegisterModule() is called for "openvx-core" module
 *
 * \ingroup group_vx_ext_ti
 */
VX_API_ENTRY vx_status VX_API_CALL vxRegisterModule(char *name, vx_publish_kernels_f publish, vx_unpublish_kernels_f unpublish);

/*!
 * \brief UnRegister publish and unpublish functions if previously registered
 *
 * \ingroup group_vx_ext_ti
 */
VX_API_ENTRY vx_status VX_API_CALL vxUnRegisterModule(char *name);


#ifdef __cplusplus
}
#endif

#endif
