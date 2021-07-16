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


#ifndef TIVX_KERNELS_H_
#define TIVX_KERNELS_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Name for TI OpenVX kernel module
 * \ingroup group_tivx_int_common_kernel
 */
#define TIVX_MODULE_NAME_OPENVX_CORE    "openvx-core"

/*! \brief Name for TI OpenVX kernel module
 * \ingroup group_tivx_ext_ivision_kernel
 */
#define TIVX_MODULE_NAME_IVISION    "ivision"

/*!
 * \file
 * \brief The list of supported kernels in the TIOVX.
 */


/*! \brief The list of available libraries in tivx */
enum tivx_library_e {
    /*! \brief The set of kernels supported in ivision. */
   TIVX_LIBRARY_IVISION_BASE = 0,
    /*! \brief TI Extension kernels. */
   TIVX_LIBRARY_EXTENSION_BASE = 1,
};

/*!
 * \brief The list of kernels supported in ivision.
 *
 * \ingroup group_tivx_ext_ivision_kernel
 */
enum tivx_kernel_ivision_e {
    /*! \brief The Harris Corners Kernel.
     * \see group_vision_function_harris
     */
    TIVX_KERNEL_IVISION_HARRIS_CORNERS = VX_KERNEL_BASE(VX_ID_TI, TIVX_LIBRARY_IVISION_BASE) + 0x0,
    TIVX_KERNEL_IVISION_RGB_IR= VX_KERNEL_BASE(VX_ID_TI, TIVX_LIBRARY_IVISION_BASE) + 0x1,
    /* insert new kernels here */
    TIVX_KERNEL_IVISION_MAX_1_0, /*!< \internal Used for bounds checking in the conformance test. */
};

/*!
 * \brief Used for the Application to create the tidl kernel from the context.
 * \ingroup group_vision_function_tidl
 *
 * \param [in]  context             OpenVX context which must be created using \ref vxCreateContext
 * \param [in]  num_input_tensors   Number of input vx_tensor objects to be created
 * \param [in]  num_output_tensors  Number of output vx_tensor objects to be created
 *
 * \returns Handle to vx_kernel object if successful, NULL otherwise
 *
 * \note The caller of this function should check status of the return vx_kernel handle
 *       and if found to be NULL must handle or propagate error appropriately.
 *
 */
vx_kernel tivxAddKernelTIDL(vx_context context,
                            uint32_t num_input_tensors,
                            uint32_t num_output_tensors);



#ifdef __cplusplus
}
#endif

#endif
