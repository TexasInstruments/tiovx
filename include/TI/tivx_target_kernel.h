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

/*! \brief Handle to instance of kernel on a target
 *
 * \ingroup group_tivx_target_kernel
 */
typedef struct _tivx_target_kernel_instance *tivx_target_kernel_instance;

/*!
* \brief The target kernel callback
*
*        For create_func, delete_func and process_func callbacks
*        'obj_desc' points to array of data object descriptor parameters
*
*        For control_func,
*        'obj_desc' points to array of objects descriptors where
*             obj_desc[0] points to the node object descriptors
*             obj_desc[1..num_params-1] points to target kernel defined parameters
*
* \param [in] kernel The kernel for which the callback is called
* \param [in] obj_desc Object descriptor passed as input to this callback
*
* \ingroup group_tivx_target_kernel
*/
typedef vx_status(VX_CALLBACK *tivx_target_kernel_f)(tivx_target_kernel_instance kernel, uint32_t target_kernel_instance_handle, tivx_obj_desc_t *obj_desc[], uint16_t num_params);

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
