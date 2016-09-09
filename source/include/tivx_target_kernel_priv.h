/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_TARGET_KERNEL_RPIV_H_
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
 * \brief Used to indicate invalid kernel ID
 * \ingroup group_tivx_target_kernel
 */
#define TIVX_TARGET_KERNEL_ID_INVALID       (0xFFFFu)


/*!
 * \brief Holds information about a target kernel instance
 * \ingroup group_tivx_target_kernel_priv
 */
typedef struct _tivx_target_kernel {

    /*! kernel ID */
    vx_enum kernel_id;
    vx_enum target_id;

    tivx_target_kernel_f process_func;
    tivx_target_kernel_f create_func;
    tivx_target_kernel_f delete_func;
    tivx_target_kernel_f control_func;

} tivx_target_kernel_t;

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
 * \brief Returns target kernel registered against this kernel ID and target ID
 *
 * \ingroup group_tivx_target_kernel_priv
 */
tivx_target_kernel tivxTargetKernelGet(vx_enum kernel_id, vx_enum target_id);

#ifdef __cplusplus
}
#endif

#endif
