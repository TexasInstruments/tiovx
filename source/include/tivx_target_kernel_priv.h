/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_TARGET_KERNEL_PRIV_H_
#define _TIVX_TARGET_KERNEL_PRIV_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <vx_internal.h>

/*!
 * \file
 * \brief Target Kernel implementation APIs
 */


/*!
 * \brief Given a target_kernel_index and kernel_id return the handle to target kernel
 *
 *        Target kernel handle is created during create phase.
 *        During run phase, this API is used to quickly get access to
 *        the target_kernel_handle given target_kernel_index key.
 *
 *        kernel_id is used to confirm the handle matches the required kernel.
 *
 *        NULL is return if target kernel handle is not found.
 *
 * \ingroup group_tivx_target_kernel_priv
 */
tivx_target_kernel_instance tivxTargetKernelInstanceGet(uint16_t target_kernel_index, vx_enum kernel_id);

/*!
 * \brief Execute kernel on the target
 *
 *        'obj_desc' points to parameters object descriptors associated with this kernel execution
 *
 * \ingroup group_tivx_target_kernel_priv
 */
vx_status tivxTargetKernelExecute(tivx_target_kernel_instance target_kernel_instance, tivx_obj_desc_t *obj_desc[], uint16_t num_params);

/*!
 * \brief Create kernel on the target
 *
 *        'obj_desc' points to parameters object descriptors associated with this kernel execution
 *
 * \ingroup group_tivx_target_kernel_priv
 */
vx_status tivxTargetKernelCreate(tivx_target_kernel_instance target_kernel_instance, tivx_obj_desc_t *obj_desc[], uint16_t num_params);

/*!
 * \brief Delete kernel on the target
 *
 *        'obj_desc' points to parameters object descriptors associated with this kernel execution
 *
 * \ingroup group_tivx_target_kernel_priv
 */
vx_status tivxTargetKernelDelete(tivx_target_kernel_instance target_kernel_instance, tivx_obj_desc_t *obj_desc[], uint16_t num_params);

/*!
 * \brief Control kernel on the target
 *
 *        'obj_desc[0]' points to node object descriptor associated with this kernel execution
 *        'obj_desc[1..num_params]' points to kernel specific parameter object descriptors
 *
 * \ingroup group_tivx_target_kernel_priv
 */
vx_status tivxTargetKernelControl(tivx_target_kernel_instance target_kernel_instance, tivx_obj_desc_t *obj_desc[], uint16_t num_params);

/*!
 * \brief Create a target kernel instance for given kernel_id
 *
 * \ingroup group_tivx_target_kernel_priv
 */
tivx_target_kernel_instance tivxTargetKernelInstanceAlloc(vx_enum kernel_id);

/*!
 * \brief Free previously allocate target kernel instance
 *
 * \ingroup group_tivx_target_kernel_priv
 */
vx_status tivxTargetKernelInstanceFree(tivx_target_kernel_instance *target_kernel_instance);

/*!
 * \brief Get fast index key for a given target kernel instance
 *
 * \ingroup group_tivx_target_kernel_priv
 */
uint32_t tivxTargetKernelInstanceGetIndex(tivx_target_kernel_instance target_kernel_instance);

#ifdef __cplusplus
}
#endif

#endif
