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




#ifndef TIVX_TARGET_KERNEL_INSTANCE_H_
#define TIVX_TARGET_KERNEL_INSTANCE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <VX/vx.h>

/*!
 * \file
 * \brief Target Kernel Instance implementation APIs
 */

/*!
 * \brief Holds information about a target kernel instance
 * \ingroup group_tivx_target_kernel_instance
 */
typedef struct _tivx_target_kernel_instance {

    /*! target kernel associated with this instance */
    tivx_target_kernel kernel;

    /*! kernel ID */
    vx_enum kernel_id;

    /*! index in target kernel instance table */
    uint32_t index;

    /*! Kernel function context or handle */
    void *kernel_context;

    /*! Kernel function context size */
    uint32_t kernel_context_size;

    /*! \brief border mode */
    vx_border_t border_mode;

    /*! \brief capture state (VX_NODE_STATE_PIPEUP or VX_NODE_STATE_STEADY)*/
    vx_enum state;

    /*! tile width for the kernel */
    uint32_t block_width;

    /*! tile height for the kernel */
    uint32_t block_height;

    /*! flag indicating whether or not the parameter is replicated */
    vx_bool is_kernel_instance_replicated;

    /*! \brief Pointer to object descriptor of corresponding node */
    tivx_obj_desc_node_t *node_obj_desc;

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
tivx_target_kernel_instance ownTargetKernelInstanceGet(uint16_t target_kernel_index, vx_enum kernel_id);

/*!
 * \brief Create a target kernel instance for given kernel_id
 *
 * \ingroup group_tivx_target_kernel_instance
 */
tivx_target_kernel_instance ownTargetKernelInstanceAlloc(vx_enum kernel_id, volatile char *kernel_name, vx_enum target_id);

/*!
 * \brief Free previously allocate target kernel instance
 *
 * \ingroup group_tivx_target_kernel_instance
 */
vx_status ownTargetKernelInstanceFree(tivx_target_kernel_instance *target_kernel_instance);

/*!
 * \brief Get fast index key for a given target kernel instance
 *
 * \ingroup group_tivx_target_kernel_instance
 */
uint32_t ownTargetKernelInstanceGetIndex(tivx_target_kernel_instance target_kernel_instance);



/*!
 * \brief Init Target Kernel Instance Module
 *
 * \ingroup group_tivx_target_kernel_instance
 */
vx_status ownTargetKernelInstanceInit(void);

/*!
 * \brief De-Init Target Kernel Instance Module
 *
 * \ingroup group_tivx_target_kernel_instance
 */
void ownTargetKernelInstanceDeInit(void);

#ifdef __cplusplus
}
#endif

#endif
