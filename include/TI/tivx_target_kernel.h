/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/



#ifndef TIVX_TARGET_KERNEL_H_
#define TIVX_TARGET_KERNEL_H_

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
typedef vx_status(VX_CALLBACK *tivx_target_kernel_f)(tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);

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
                             vx_enum kernel_id,
                             char *target_name,
                             tivx_target_kernel_f process_func,
                             tivx_target_kernel_f create_func,
                             tivx_target_kernel_f delete_func,
                             tivx_target_kernel_f control_func,
                             void *priv_arg);

/*! \brief Allows users to add native kernels implementation to specific targets
 *
 *         Same as tivxAddTargetKernel except that it take a string name as input
 *         instead of kernel_id
 *
 * \ingroup group_tivx_target_kernel
 *
 */
VX_API_ENTRY tivx_target_kernel VX_API_CALL tivxAddTargetKernelByName(
                             char *kernel_name,
                             char *target_name,
                             tivx_target_kernel_f process_func,
                             tivx_target_kernel_f create_func,
                             tivx_target_kernel_f delete_func,
                             tivx_target_kernel_f control_func,
                             void *priv_arg);


/*! \brief Allows users to remove native kernels implementation
 *         to specific targets
 *
 * \ingroup group_tivx_target_kernel
 *
 */
VX_API_ENTRY vx_status VX_API_CALL tivxRemoveTargetKernel(
    tivx_target_kernel target_kernel);

/*!
 * \brief Associate a kernel function context or handle with a target kernel instance
 *
 *        Typically set by the kernel function during create phase
 *
 * \ingroup group_tivx_target_kernel
 */
VX_API_ENTRY vx_status VX_API_CALL tivxSetTargetKernelInstanceContext(
            tivx_target_kernel_instance target_kernel_instance,
            void *kernel_context, uint32_t kernel_context_size);

/*!
 * \brief Get a kernel function context or handle with a target kernel instance
 *
 *        Typically used by the kernel function during run, control, delete phase
 *
 * \ingroup group_tivx_target_kernel
 */
VX_API_ENTRY vx_status VX_API_CALL tivxGetTargetKernelInstanceContext(
            tivx_target_kernel_instance target_kernel_instance,
            void **kernel_context, uint32_t *kernel_context_size);

/*!
 * \brief Get the border mode for the target kernel instance
 *
 *        Used by the kernel implemention to get border mode
 *
 * \ingroup group_tivx_target_kernel
 */
VX_API_ENTRY void tivxGetTargetKernelInstanceBorderMode(
    tivx_target_kernel_instance target_kernel_instance,
    vx_border_t *border_mode);

#ifdef __cplusplus
}
#endif

#endif
