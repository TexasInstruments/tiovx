/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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

#ifndef J7_KERNELS_H_
#define J7_KERNELS_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The list of supported kernels in this kernel extension.
 */

/*! \brief Name for OpenVX Extension kernel module: hwa
 * \ingroup group_tivx_ext_top
 */
#define TIVX_MODULE_NAME_HWA    "hwa"

/*! \brief Name for OpenVX Extension kernel module: tidl
 * \ingroup group_tivx_ext_top
 */
#define TIVX_MODULE_NAME_TIDL    "tidl"

/*! \brief Name for OpenVX Extension kernel module: tvm
 * \ingroup group_tivx_ext_top
 */
#define TIVX_MODULE_NAME_TVM    "tvm"

/*! \brief dof_visualize kernel name
 *  \ingroup group_vision_function_dmpac_dof
 */
#define TIVX_KERNEL_DOF_VISUALIZE_NAME     "com.ti.hwa.dof_visualize"

/*! \brief Object Array Split Kernel Name
 *  \ingroup group_vision_function_hwa
 */
#define TIVX_KERNEL_OBJ_ARRAY_SPLIT_NAME       "com.ti.hwa.obj_array_split"

/*! \brief tidl kernel name
 *  \ingroup group_vision_function_tidl
 */
#define TIVX_KERNEL_TIDL_NAME          "com.ti.tidl"

/*! \brief tvm kernel name
 *  \ingroup group_vision_function_tvm
 */
#define TIVX_KERNEL_TVM_NAME          "com.ti.tvm"

/*! End of group_vision_function_hwa */


/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Used for the Application to load the hwa kernels into the context.
 *
 * This includes Capture, Display, VPAC, and DMPAC kernels
 *
 * \ingroup group_vision_function_hwa
 */
void tivxHwaLoadKernels(vx_context context);

/*!
 * \brief Used for the Application to unload the hwa kernels from the context.
 *
 * This includes Capture, Display, VPAC, and DMPAC kernels
 *
 * \ingroup group_vision_function_hwa
 */
void tivxHwaUnLoadKernels(vx_context context);

/*!
 * \brief Used for the Application to load the tidl kernels into the context.
 * \ingroup group_vision_function_hwa
 */
void tivxTIDLLoadKernels(vx_context context);

/*!
 * \brief Used for the Application to unload the tidl kernels from the context.
 * \ingroup group_vision_function_hwa
 */
void tivxTIDLUnLoadKernels(vx_context context);

/*!
 * \brief Used for the Application to load the tvm kernels into the context.
 * \ingroup group_vision_function_tvm
 */
void tivxTVMLoadKernels(vx_context context);

/*!
 * \brief Used for the Application to unload the tvm kernels from the context.
 * \ingroup group_vision_function_tvm
 */
void tivxTVMUnLoadKernels(vx_context context);



/*!
 * \brief Function to register TIDL Kernels on the TIDL Target
 * \ingroup group_vision_function_tidl
 */
void tivxRegisterTIDLTargetKernels(void);

/*!
 * \brief Function to un-register TIDL Kernels on the TIDL Target
 * \ingroup group_vision_function_tidl
 */
void tivxUnRegisterTIDLTargetKernels(void);

/*!
 * \brief Function to register TVM Kernels on the TVM Target
 * \ingroup group_vision_function_tvm
 */
void tivxRegisterTVMTargetKernels(void);

/*!
 * \brief Function to un-register TVM Kernels on the TVM Target
 * \ingroup group_vision_function_tvm
 */
void tivxUnRegisterTVMTargetKernels(void);

#ifdef __cplusplus
}
#endif

#endif /* J7_KERNELS_H_ */


