/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_TARGET_KERNEL_INSTANCE_H_
#define _TIVX_TARGET_KERNEL_INSTANCE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <vx_internal.h>

/*!
 * \file
 * \brief Target Kernel Instance implementation APIs
 */

/*!
 * \brief Max target kernel instances that will be active on a CPU
 *       at the same time
 * \ingroup group_tivx_target_kernel_instance_cfg
 */
#define TIVX_TARGET_KERNEL_INSTANCE_MAX     (64u)

/*!
 * \brief Holds information about a target kernel instance
 * \ingroup group_tivx_target_kernel_instance
 */
typedef struct _tivx_target_kernel_instance {

    /*! target kernel associated with this instance */
    tivx_target_kernel kernel;

    /*! kernel ID */
    vx_enum kernel_id;

    /*! index in target kernel instance */
    uint32_t index;

    /* Kernel function context or handle */
    void *kernel_context;

    /* Kernel function context size */
    uint32_t kernel_context_size;

    /*! \brief border mode */
    vx_border_t border_mode;

} tivx_target_kernel_instance_t;

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
 * \ingroup group_tivx_target_kernel_instance
 */
tivx_target_kernel_instance tivxTargetKernelInstanceGet(uint16_t target_kernel_index, vx_enum kernel_id);

/*!
 * \brief Create a target kernel instance for given kernel_id
 *
 * \ingroup group_tivx_target_kernel_instance
 */
tivx_target_kernel_instance tivxTargetKernelInstanceAlloc(vx_enum kernel_id, vx_enum target_id);

/*!
 * \brief Free previously allocate target kernel instance
 *
 * \ingroup group_tivx_target_kernel_instance
 */
vx_status tivxTargetKernelInstanceFree(tivx_target_kernel_instance *target_kernel_instance);

/*!
 * \brief Get fast index key for a given target kernel instance
 *
 * \ingroup group_tivx_target_kernel_instance
 */
uint32_t tivxTargetKernelInstanceGetIndex(tivx_target_kernel_instance target_kernel_instance);

/*!
 * \brief Get target kernel for a given target kernel instance
 *
 * \ingroup group_tivx_target_kernel_instance
 */
tivx_target_kernel tivxTargetKernelInstanceGetKernel(tivx_target_kernel_instance target_kernel_instance);


/*!
 * \brief Init Target Kernel Instance Module
 *
 * \ingroup group_tivx_target_kernel_instance
 */
vx_status tivxTargetKernelInstanceInit();

/*!
 * \brief De-Init Target Kernel Instance Module
 *
 * \ingroup group_tivx_target_kernel_instance
 */
void tivxTargetKernelInstanceDeInit();

#ifdef __cplusplus
}
#endif

#endif
