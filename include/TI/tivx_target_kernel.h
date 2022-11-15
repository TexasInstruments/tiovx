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


/*!
* \brief The target kernel callback for control command
*
*        For create_func, delete_func and process_func callbacks
*        'obj_desc' points to array of data object descriptor parameters
*
*        For control_func,
*        'obj_desc' points to array of objects descriptors
*        for control parameter. It could be any vx_(object)
*
* \param [in] kernel The kernel for which the callback is called
* \param [in] Command ID to be processed in the given node
* \param [in] obj_desc Object descriptor passed as input to this callback
* \param [in] num_params valid entries in object descriptor (obj_desc) array
*
* \ingroup group_tivx_target_kernel
*/
typedef vx_status(VX_CALLBACK *tivx_target_kernel_control_f)(
    tivx_target_kernel_instance kernel, uint32_t node_cmd_id,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);

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
                             const char *target_name,
                             tivx_target_kernel_f process_func,
                             tivx_target_kernel_f create_func,
                             tivx_target_kernel_f delete_func,
                             tivx_target_kernel_control_f control_func,
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
                             const char *kernel_name,
                             const char *target_name,
                             tivx_target_kernel_f process_func,
                             tivx_target_kernel_f create_func,
                             tivx_target_kernel_f delete_func,
                             tivx_target_kernel_control_f control_func,
                             void *priv_arg);


/*! \brief Allows users to remove native kernels implementation
 *         to specific targets
 *
 * \ingroup group_tivx_target_kernel
 *
 */
VX_API_ENTRY vx_status VX_API_CALL tivxRemoveTargetKernel(
    tivx_target_kernel target_kernel);

/*! \brief Queries the framework to determine the number of target kernels
 *         on the target from which it is called
 *
 * \param [out] ptr The location at which to store the resulting value.
 *
 * \ingroup group_tivx_target_kernel
 *
 */
VX_API_ENTRY vx_status VX_API_CALL tivxQueryNumTargetKernel(vx_uint32 *ptr);

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
 * \brief Get a kernel state with a target kernel instance
 *
 *        Typically used by a capture kernel function during process to track state
 *
 * \ingroup group_tivx_target_kernel
 */
VX_API_ENTRY vx_status VX_API_CALL tivxGetTargetKernelInstanceState(
            tivx_target_kernel_instance target_kernel_instance,
            vx_enum *state);

/*!
 * \brief Get a kernel target id with a target kernel instance
 *
 *        Typically used by nodes having multi priority implementation
 *
 * \ingroup group_tivx_target_kernel
 */
VX_API_ENTRY vx_status VX_API_CALL tivxGetTargetKernelTargetId(
            tivx_target_kernel_instance target_kernel_instance,
            uint32_t *targetId);

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

/*!
 * \brief Checks if supplied node is replicated.  Returns vx_true if the given
 *        node is replicated and vx_false if not.
 *
 * \ingroup group_tivx_target_kernel
 */
VX_API_ENTRY vx_bool tivxIsTargetKernelInstanceReplicated(tivx_target_kernel_instance kernel_instance);

/*!
 * \brief Get target kernel for a given target kernel instance
 *
 * \ingroup group_tivx_target_kernel_instance
 */
tivx_target_kernel tivxTargetKernelInstanceGetKernel(tivx_target_kernel_instance target_kernel_instance);

static inline vx_bool tivxFlagIsBitSet(uint32_t flag_var, uint32_t flag_val);
static inline void tivxFlagBitSet(volatile uint32_t *flag_var, uint32_t flag_val);
static inline void tivxFlagBitClear(volatile uint32_t *flag_var, uint32_t flag_val);

/*! \brief Macro to check if flag is set, flag MUST be of bit type
 * \ingroup group_tivx_target_kernel_instance
 */
static inline vx_bool tivxFlagIsBitSet(uint32_t flag_var, uint32_t flag_val)
{
    return (vx_bool)((flag_var & flag_val) == flag_val);
}

/*! \brief Macro to set flag value, flag MUST be of bit type
 * \ingroup group_tivx_target_kernel_instance
 */
static inline void tivxFlagBitSet(volatile uint32_t *flag_var, uint32_t flag_val)
{
    *flag_var |= flag_val;
}

/*! \brief Macro to clear flag value, flag MUST be of bit type
 * \ingroup group_tivx_target_kernel_instance
 */
static inline void tivxFlagBitClear(volatile uint32_t *flag_var, uint32_t flag_val)
{
    uint32_t value = *flag_var;

    value = value & ~flag_val;

    *flag_var = value;
}

/*!
 * \brief Add real-time log event in host side initilize callback for user kernel
 *
 * - This API is intended to be called from within the initialization callback
 *   of a user kernel.
 * - For a given node, multiple such events can be added, therefore each one should be
 *   given a different index used to identify the event within the node.
 * - Since the scope of the indexing is within a single node, it is permissible to reuse
 *   index values across nodes without any issue (for example, event_index of 0 can be used at
 *   most once per node).
 *
 * \param [in] node         The node object that the event is to be asociated with
 * \param [in] event_index  Unique identifier of specific event within node
 * \param [in] *event_name  The node object associated with the event
 *
 * \ingroup group_tivx_log_rt_trace_host
 */
void tivxLogRtTraceKernelInstanceAddEvent(vx_node node, uint16_t event_index, char *event_name);

/*!
 * \brief Remove real-time log event in host side deinitilize callback for user kernel
 *
 * - This API is intended to be called from within the deinitialization callback
 *   of a user kernel.
 *
 * \param [in] node         The node object that the event is to be asociated with
 * \param [in] event_index  Unique identifier of specific event within node
 *
 * \pre \ref tivxLogRtTraceKernelInstanceAddEvent
 * \ingroup group_tivx_log_rt_trace_host
 *
 */
void tivxLogRtTraceKernelInstanceRemoveEvent(vx_node node, uint16_t event_index);

/*!
 * \brief Log trace on target kernel instance execute start
 *
 * - This API is intended to be called from within a target-side callback
 *   of a user kernel (process callback, for example).
 *
 * \param [in] kernel       The target kernel instance object from the argument list of the target side callback
 * \param [in] event_index  Event index to trigger the start trace for
 *
 * \ingroup group_tivx_log_rt_trace_target
 */
void tivxLogRtTraceKernelInstanceExeStart(tivx_target_kernel_instance kernel, uint16_t event_index);

/*!
 * \brief Log trace on target kernel instance execute end
 *
 * - This API is intended to be called from within a target-side callback
 *   of a user kernel (process callback, for example).
 *
 * \param [in] kernel       The target kernel instance object from the argument list of the target side callback
 * \param [in] event_index  Event index to trigger the end trace for
 *
 * \ingroup group_tivx_log_rt_trace_target
 */
void tivxLogRtTraceKernelInstanceExeEnd(tivx_target_kernel_instance kernel, uint16_t event_index);

/*!
 * \brief Utility function to obtain DMA handle
 *
 * \ingroup group_tivx_target_kernel
 */
void *tivxPlatformGetDmaObj(void);

#ifdef __cplusplus
}
#endif

#endif
