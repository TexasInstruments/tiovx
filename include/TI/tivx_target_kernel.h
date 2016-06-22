/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_TARGET_KERNEL_H_
#define _TIVX_TARGET_KERNEL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to kernel APIs on target
 */

#include <TI/tivx_obj_desc.h>

/*! \brief Handle to kernel on a target
 *
 * \ingroup group_tivx_target_kernel
 */
typedef struct _tivx_target_kernel *tivx_target_kernel;


/*!
* \brief The target kernel callback
*
*        'obj_desc' is of type 'node' for init, deinit and process callbacks
*        'obj_desc' is of type 'command' for control callback
*
*        'node' object descriptor further points to data object descriptors
*        'command' object descriptor further pointer to parameters for this command
*
* \param [in] kernel The kernel for which the callback is called
* \param [in] obj_desc Object descriptor passed as input to this callback
*
* \ingroup group_tivx_target_kernel
*/
typedef vx_status(VX_CALLBACK *tivx_target_kernel_f)(tivx_target_kernel kernel, tivx_obj_desc_t *obj_desc);

/*! \brief Allows users to add native kernels implementation to specific targets
 *
 *         This is different from vxAddUserKernel() in that this is called
 *         on the target CPU. This is a TI prorietary API and not part of
 *         OpenVX or TI OpenVX extention.
 *
 *         This allows users to implement and plugin specific
 *         target optimized kernels on TI platforms
 *
 *         A equivalent  vxAddUserKernel is typically called to pair the target
 *         kernel with OpenVX user kernel.
 *
 * \ingroup group_tivx_target_kernel
 *
 */
VX_API_ENTRY tivx_target_kernel VX_API_CALL tivxAddTargetKernel(
                             vx_enum enumeration,
                             tivx_target_kernel_f process_func,
                             tivx_target_kernel_f create_func,
                             tivx_target_kernel_f delete_func,
                             tivx_target_kernel_f control_func);

/*!
 * \brief Associate a target with a kernel
 *
 *        Call multiple time for each support target or target class
 *
 *        Valid targets, duplicate targets are checked by this API
 *
 * \ingroup group_tivx_target_kernel
 */
VX_API_ENTRY vx_status VX_API_CALL tivxAddTargetKernelTarget(tivx_target_kernel kernel, char *target_name);

/*!
 * \brief Remove kernel for kernel list
 *
 * \ingroup group_tivx_target_kernel
 */
VX_API_ENTRY vx_status VX_API_CALL tivxRemoveTargetKernel(tivx_target_kernel *kernel);

#ifdef __cplusplus
}
#endif

#endif
