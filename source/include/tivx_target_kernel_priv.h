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




#ifndef TIVX_TARGET_KERNEL_PRIV_H_
#define TIVX_TARGET_KERNEL_PRIV_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <vx_internal.h>

#if defined(BUILD_BAM)
#include "tivx_bam_kernel_wrapper.h"
#endif

/*!
 * \file
 * \brief Target Kernel implementation APIs
 */

/*!
 * \brief Used to indicate invalid kernel ID
 * \ingroup group_tivx_target_kernel_priv
 */
#define TIVX_TARGET_KERNEL_ID_INVALID       (0xFFFFu)

/*!
 * \brief Used to indicate kernel ID not used
 *        and kernel name is used instead
 * \ingroup group_tivx_target_kernel_priv
 */
#define TIVX_TARGET_KERNEL_ID_NOT_USED    (0xFFFEu)


/*!
 * \brief Holds information about a target kernel instance
 * \ingroup group_tivx_target_kernel_priv
 */
typedef struct _tivx_target_kernel {

    /*! kernel ID */
    vx_enum kernel_id;
    vx_enum target_id;
    char    kernel_name[VX_MAX_KERNEL_NAME];

    tivx_target_kernel_f process_func;
    tivx_target_kernel_f create_func;
    tivx_target_kernel_f delete_func;
    tivx_target_kernel_control_f control_func;

#if defined(BUILD_BAM)
    tivx_target_kernel_create_in_bam_graph_f   create_in_bam_func;
    tivx_target_kernel_get_node_port_f         get_node_port_func;
    tivx_target_kernel_append_internal_edges_f append_internal_edges_func;
    tivx_target_kernel_pre_post_process_f      preprocess_func;
    tivx_target_kernel_pre_post_process_f      postprocess_func;
    int32_t                                    kernel_params_size;
#endif

    void *caller_priv_arg;

    /*! \brief Number of buffers needed for this kernel */
    vx_uint32      num_pipeup_bufs;

} tivx_target_kernel_t;

/*!
 * \brief Execute kernel on the target
 *
 *        'obj_desc' points to parameters object descriptors associated with this kernel execution
 *
 * \ingroup group_tivx_target_kernel_priv
 */
vx_status ownTargetKernelExecute(tivx_target_kernel_instance target_kernel_instance, tivx_obj_desc_t *obj_desc[], uint16_t num_params);

/*!
 * \brief Create kernel on the target
 *
 *        'obj_desc' points to parameters object descriptors associated with this kernel execution
 *
 * \ingroup group_tivx_target_kernel_priv
 */
vx_status ownTargetKernelCreate(tivx_target_kernel_instance target_kernel_instance, tivx_obj_desc_t *obj_desc[], uint16_t num_params);

/*!
 * \brief Delete kernel on the target
 *
 *        'obj_desc' points to parameters object descriptors associated with this kernel execution
 *
 * \ingroup group_tivx_target_kernel_priv
 */
vx_status ownTargetKernelDelete(tivx_target_kernel_instance target_kernel_instance, tivx_obj_desc_t *obj_desc[], uint16_t num_params);

/*!
 * \brief Control kernel on the target
 *
 *        'obj_desc[0]' points to node object descriptor associated with this kernel execution
 *        'obj_desc[1..num_params]' points to kernel specific parameter object descriptors
 *
 * \ingroup group_tivx_target_kernel_priv
 */
vx_status ownTargetKernelControl(
    tivx_target_kernel_instance target_kernel_instance,
    uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[], uint16_t num_params);

/*!
 * \brief Returns target kernel registered against this kernel ID and target ID
 *
 * \ingroup group_tivx_target_kernel_priv
 */
tivx_target_kernel ownTargetKernelGet(vx_enum kernel_id, volatile char *kernel_name, vx_enum target_id);

/*!
 * \brief Init target kernel module
 *
 * \ingroup group_tivx_target_kernel_priv
 */
vx_status ownTargetKernelInit(void);

/*!
 * \brief DeInit target kernel module
 *
 * \ingroup group_tivx_target_kernel_priv
 */
void ownTargetKernelDeInit(void);


#ifdef __cplusplus
}
#endif

#endif
